#define _GNU_SOURCE
#include <linux/futex.h>    //for FUTEX_WAIT FUTEX_WAKE
#include <sys/time.h>       //for struct timespec
#include <sys/syscall.h>    //for SYS_futex
#include <unistd.h>         //for syscall
#include <sys/mman.h>       //for mmap
#include <stdint.h>         //for uint32_t
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "athread_futex.h"

#define error_handle(msg) do{perror(msg); exit(EXIT_FAILURE);} while(0)

static int futex(uint32_t* uaddr, int futex_op, int val, const struct timespec *timeout,
        int *uaddr2, int val3) 
{
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

void athread_mutex_wait(uint32_t *futexp)
{
    while(1) {
        //通过一个循环来避免多个线程占用同一个futex的情况
        if(__sync_bool_compare_and_swap(futexp, 0, -1)){
            break;
        }
        /** __sync_sub_and_fetch(futexp, 1); */

        int s = futex(futexp, FUTEX_WAIT, -1, NULL, NULL, 0);
        //判断被唤醒后,再占有futex，此时futexp是否会被futex改为0
        /** printf("futexp = %d\n", *futexp); */
        if(s == -1) {
            /** error_handle("wait futex: "); */
            continue;
        } 
    }
}

void athread_mutex_post(uint32_t* futexp) 
{
    /** if(__sync_bool_compare_and_swap(futexp, 0, 1)) {
     *     //这里有一个情况是，将futex值加1后，还没进行下一步唤醒，有新线程过来
     *     //则新线程会获得futex的所有权
     *     int s = futex(futexp, FUTEX_WAKE, 1, NULL , NULL, 0);
     *     if(s == -1) {
     *         error_handle("futex FUTEX_WAKE:");
     *     }
     * } */
    if(__sync_bool_compare_and_swap(futexp, -1, 0)){
        int s = futex(futexp, FUTEX_WAKE, 0, NULL, NULL, 0);
        if(s == -1) {
            error_handle("wake futex: ");
        }
    }
}

int athread_init_mutex(uint32_t* futexp) 
{
    /** if(val != 1 && val != 0) {
     *     fprintf(stderr, "The value of mutex must be 0 or 1\n");
     *     return -1;
     * } */
    *futexp = 0;
    return 0;
}

void athread_sem_wait(sem_t* sem)
{
}

void athread_cond_wait(cond_t *cond, mutex_t *mutex)
{
    athread_mutex_post(mutex);

    do {
        int futex_val = *cond;
        int s = futex(cond, FUTEX_WAIT, futex_val, NULL, NULL, 0);
        if(s == -1) {
            error_handle("cond_wait futex: ");
        }
    } while(0);

    return athread_mutex_wait(mutex);
}

void athread_cond_signal(cond_t *cond) 
{
    futex(cond, FUTEX_WAKE, 1, NULL, NULL, 0);
}


void athread_init_cond(cond_t *cond) 
{
    *cond = 0;
}
