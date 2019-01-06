#define _GNU_SOURCE 
#include <sched.h>
#include "athread.h"
#include <stdio.h> // for perror
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include "alist.h"
#include "sptimer.h"


#define STACK_LEN 4096 * 1024
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE);} while(0)
#define gettid()    syscall(SYS_gettid)
#define tkill(tid, sig) syscall(SYS_tkill, tid, sig)

#define RELEASE_MEM_TIME 1
#define ATHREAD_CANCEL 60

#define elem2entry(val_ptr, type, member) (type*)((long int)val_ptr - (long int)(&((type*)0)->member))

typedef void(*SIGNALFN)(int);
/** #define max_thread 1024 */

bool first_create = true;
static int thread_num = 1;
static int timefd;

struct list alive_stack_list;
struct list wait_rm_stack_list;

//自定义栈结构
struct stk_list_elem {
    struct list_elem flag;  //加入通用链表的标记
    void*  stk_addr;        //mmap申请的栈地址
    int    stk_size;        //mmap申请的栈大小
    pid_t thid;             //线程号为thid的线程申请的栈
};

//初始化自定义结构stk_list_elem
static void init_stk_elem(struct stk_list_elem* elem, void* addr, int size, athread_t id) {
    elem->stk_addr = addr;
    elem->stk_size = size;
    elem->thid = id;
}

//通过tid来找结点的list回调函数
static bool find_stk_addr_by_tid(struct list_elem* elem, pid_t tid) {
    struct stk_list_elem* temp = elem2entry(elem, struct stk_list_elem, flag);
    if(temp->thid == tid) {
        return true;
    }
    return false;
}

//通过wait链表释放未释放的mmap映射内存的回调函数
static bool release_mem(struct list_elem* elem, pid_t tid) {
    struct stk_list_elem* temp = elem2entry(elem, struct stk_list_elem, flag);
    printf("release memory in %p\n", temp->stk_addr);
    munmap(temp->stk_addr, temp->stk_size);
    printf("after munmap\n");
    printf("malloc memory in %p\n", temp);
    remove_elem(&wait_rm_stack_list, &temp->flag);
    free(temp);
    return false;
}


//定时执行内存释放函数
static void time_action(int arg) {
    list_traversal(&wait_rm_stack_list, release_mem, 0);
    signal(SIGALRM, time_action);
    alarm(RELEASE_MEM_TIME);
}

//创建线程
void athread_create(athread_t* th_id, void* en, int (*fn)(void*), void* arg ) {
    void* start_stack = mmap(NULL, STACK_LEN,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    printf("mmap memory in %p\n", start_stack);

    if(start_stack == MAP_FAILED) {
        handle_error("mmap");
    }
    pid_t tid = clone(fn, 
            (void*)((unsigned char*) start_stack + STACK_LEN),
            CLONE_THREAD    //同一个进程组
            | CLONE_PARENT  //同一个父进程
            | CLONE_SIGHAND //一样的信号处理器
            | CLONE_VM      //同一个虚拟空间
            | CLONE_FS      //共享文件系统信息
            | CLONE_FILES   //共享文件描述符表
            | CLONE_IO,     //共享IO上下文
            (void*) NULL);

    if(tid == -1) {
        munmap(start_stack, STACK_LEN);
        handle_error("clone");
    }

    *th_id = tid;

    if(first_create) {
        init_list(&alive_stack_list);
        init_list(&wait_rm_stack_list);
        /** atexit(time_action); */
        signal(SIGALRM, time_action);
        alarm(RELEASE_MEM_TIME);

        signal(ATHREAD_CANCEL, (SIGNALFN)athread_exit);

        printf("install time_action\n");
        first_create = false;
    }

    //加入alive队列
    struct stk_list_elem* temp = (struct stk_list_elem*)malloc(sizeof(struct stk_list_elem));
    thread_num++;
    if(temp == NULL) {
        handle_error("malloc");
    }
    printf("malloc memory in %p\n", temp);
    init_stk_elem(temp, start_stack, STACK_LEN, *th_id);
    push_list(&alive_stack_list, &temp->flag);

}


//线程取消函数，通过信号来使单个线程执行athread_exit
void athread_cancel(pid_t tid) 
{
    printf("cancel %d\n", tid);
    tkill(tid, 60);
}

//线程退出函数
void athread_exit(void* arg) 
{
    int stack_index = -1;
    pid_t tid = gettid();
    printf("tid = %d\n", tid);
    pid_t pid = getpid();
    printf("pid = %d\n", pid);

    if(pid == tid)  {   //main thread do not need munmap
        thread_num--;
        if(thread_num == 0) {
            printf("exit\n");
            /** exit(0); */
        }
        printf("main thread exit\n");
        syscall(SYS_exit, 0);
        printf("main thread exit\n");
    }

    //通过tid找到链表flag
    struct list_elem* temp = list_traversal(&alive_stack_list, find_stk_addr_by_tid, tid);
    if(temp == NULL) printf("temp == nULL\n");
    remove_elem(&alive_stack_list, temp);    //从alive列表中移除
    /** struct stk_list_elem* stk_elem = elem2entry(temp, struct stk_list_elem, flag); */
    push_list(&wait_rm_stack_list, temp);    //加入删除列表

    syscall(SYS_exit, 0);
}
