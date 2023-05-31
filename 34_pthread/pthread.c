#include <stdio.h>
#include <pthread.h>
#define ITER_SIZE 100000
#define PTHREADS_SUM 10

long int global = 0;
long int* global_ptr = &global;

void* thread_calc ()
{   
    for(int j = 0; j < ITER_SIZE; j++)
    {
        (*global_ptr)++;
    }
    
    printf("%ld\n", *global_ptr);

    return NULL;
}

int main(void)
{
    int i;
    int a[PTHREADS_SUM];
    int* s;
    
    pthread_t thread[PTHREADS_SUM];

    for(i = 0; i < PTHREADS_SUM; i++)
    {
        a[i] = i;
        pthread_create(&thread[i], NULL, thread_calc, (void*) &a[i]);
    }
    
    for(i = 0; i < PTHREADS_SUM; i++)
    {
        pthread_join(thread[i], (void**) &s);
    }

    return 0;
}
