#define _GNU_SOURCE 
#include <sched.h>
#include "athread.h"
#include <stdio.h> // for perror
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <unistd.h>
#include "alist.h"


#define STACK_LEN 4096 * 1024
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE);} while(0)
#define gettid()    syscall(SYS_gettid)

#define elem2entry(val_ptr, type, member) (type*)((long int)val_ptr - (long int)(&((type*)0)->member))

/** #define max_thread 1024 */

bool first_create = true;

struct list alive_stack_list;
struct list wait_rm_stack_list;

struct stk_list_elem {
    struct list_elem flag;
    void*  stk_addr;
    int    stk_size;
    pid_t thid;
};

void init_stk_elem(struct stk_list_elem* elem, void* addr, int size, athread_t id) {
    elem->stk_addr = addr;
    elem->stk_size = size;
    elem->thid = id;
}

//通过tid来找结点的list回调函数
bool find_stk_addr_by_tid(struct list_elem* elem, pid_t tid) {
    struct stk_list_elem* temp = elem2entry(elem, struct stk_list_elem, flag);
    if(temp->thid == tid) {
        return true;
    }
    return false;
}

//通过wait链表释放未释放的mmap映射内存的回调函数
bool release_mem(struct list_elem* elem, pid_t tid) {
    struct stk_list_elem* temp = elem2entry(elem, struct stk_list_elem, flag);
    printf("release memory in %p\n", temp->stk_addr);
    munmap(temp->stk_addr, temp->stk_size);
    printf("malloc memory in %p\n", temp);
    remove_elem(&wait_rm_stack_list, &temp->flag);
    free(temp);
    return false;
}

void time_action() {
    list_traversal(&wait_rm_stack_list, release_mem, 0);
}

//static void* stack_array[max_thread];
//static int thid2stack[max_thread];
//static int index = 0;

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
        first_create = false;
    }

    //加入alive队列
    struct stk_list_elem* temp = (struct stk_list_elem*)malloc(sizeof(struct stk_list_elem));
    if(temp == NULL) {
        handle_error("malloc");
    }
    printf("malloc memory in %p\n", temp);
    init_stk_elem(temp, start_stack, STACK_LEN, *th_id);
    push_list(&alive_stack_list, &temp->flag);
    
}

void athread_exit(void* arg) 
{
    int stack_index = -1;
    pid_t tid = gettid();
    printf("tid = %d\n", tid);
    pid_t pid = getpid();
    printf("pid = %d\n", pid);

    if(pid == tid)  {   //main thread do not need munmap
        printf("main thread exit\n");
        syscall(SYS_exit, 0);
        printf("main thread exit\n");
    }

    //通过tid找到链表flag
    struct list_elem* temp = list_traversal(&alive_stack_list, find_stk_addr_by_tid, tid);
    remove_elem(&alive_stack_list, temp);    //从alive列表中移除
    /** struct stk_list_elem* stk_elem = elem2entry(temp, struct stk_list_elem, flag); */
    push_list(&wait_rm_stack_list, temp);    //加入删除列表
    //for(int i = 0; i < max_thread; ++i) {
    //    if(tid == thid2stack[i]) {
    //        stack_index = i;
    //        break;
    //    }
    //}
    //if(stack_index == -1) {  //说明没有这个线程或者create时间片在赋值前断了
    //    //something wrong
    //    handle_error("terrible wrong");

    //加入回收队列

    //}

    //release stack memory from mmap
    //printf("munmap addr %p\n", stack_array[stack_index]);
    //munmap(stack_array[stack_index], STACK_LEN);
    //stack_array[stack_index] = NULL;
    //thid2stack[stack_index] = 0;
    //printf("tid = %d\n", tid);

    //exit thread;
    syscall(SYS_exit, 0);
    /** syscall(SYS_exit, *(int*)arg); */
}
