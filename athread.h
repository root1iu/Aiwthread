#ifndef athread
#define athread 


typedef unsigned long int athread_t;

void athread_exit(void* arg);
void athread_create(athread_t* th_id, void* en, int (*fn)(void*), void* arg );

void time_action();

#endif /* ifndef sPthread */
