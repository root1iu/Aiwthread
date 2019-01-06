#ifndef athread
#define athread 


typedef unsigned long int athread_t;
typedef int pid_t;

//线程退出函数
void athread_exit(void* arg);

//创建线程
void athread_create(athread_t* th_id, void* en, int (*fn)(void*), void* arg );

//线程取消函数，通过信号来使单个线程执行athread_exit
void athread_cancel(pid_t tid);


#endif /* ifndef sPthread */
