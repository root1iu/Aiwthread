#ifndef ATHREAD_FUTEX_H
#define ATHREAD_FUTEX_H
#include <stdint.h>


typedef uint32_t mutex_t;
typedef uint32_t sem_t;
typedef uint32_t cond_t;

void athread_mutex_wait(uint32_t *futexp);
void athread_mutex_post(uint32_t *futexp);
int  athread_init_mutex(uint32_t* futexp);
void athread_cond_wait(cond_t *cond, mutex_t *mutex);
void athread_cond_signal(cond_t *cond) ;
void athread_init_cond(cond_t *cond);


#endif /* ifndef SPFUTEX */
