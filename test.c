#include "athread.h"
#include <stdio.h>
#include <unistd.h>


int testthread(void* arg) {
    int time = 5;
    while(time--) {
        sleep(1);
        printf("Hello I am Child\n");
    }
    athread_exit(NULL);
    return 0;
}

/** #define elem2entry(val_ptr, type, member) (type*)((long int)val_ptr - (long int)(&((type*)0)->member)) */
/** struct list { */
/**     struct list* first; */
/**     struct list* flag; */
/** }; */

int main(int argc, char *argv[])
{
    athread_t th;
    athread_create(&th, NULL, testthread, NULL);
    sleep(10);
    time_action();
    athread_exit(NULL);
    /** struct list temp; */
    /** printf("%p\n", &temp); */
    /** printf("%p\n", elem2entry(&temp.flag, struct list, flag)); */
    /** printf("%d\n", sizeof(long int)); */
}
