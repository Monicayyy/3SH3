#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 6

int total_amount = 0; /* this data is shared by the thread(s) */
void *deposit(void *param); //threads call this function
void *withdrawal(void *param); //threads call this function
pthread_mutex_t mutex; //declaring mutex

int main(int argc, char *argv[])
{
    pthread_t tid[NUM_THREADS];        /* Creating thread identifiers for 6 threads */
    pthread_attr_t attr;                /* set of thread attributes */

    //checking if mutex intialized properly
    if (pthread_mutex_init(&mutex, NULL) != 0)
        printf("Error in initializing mutex \n");

    /* set the default attributes of the thread */
    pthread_attr_init(&attr);

    /* create the thread */
    // deposit is argv[1] and argv[2] is withdrawal
    pthread_create(&tid[0], &attr, deposit, argv[1]);
    pthread_create(&tid[1], &attr, deposit, argv[1]);
    pthread_create(&tid[2], &attr, deposit, argv[1]);
    pthread_create(&tid[3], &attr, withdrawal, argv[2]);
    pthread_create(&tid[4], &attr, withdrawal, argv[2]);
    pthread_create(&tid[5], &attr, withdrawal, argv[2]);

    /* wait for the thread to exit */
    // parent thread join again, can create for loop to join more than 1 threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(tid[i], NULL);

    printf("total_amount = %d\n", total_amount);
}

void *deposit(void *param)
{
    int deposit_amount = atoi(param);

    //acquire mutex lock
    pthread_mutex_lock(&mutex);

    total_amount += deposit_amount;
    printf("Deposit amount = %d\n", total_amount);

    //Release mutex locks
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

void *withdrawal(void *param)
{
    int withdrawal_amount = atoi(param);

    //acquire mutex lock
    pthread_mutex_lock(&mutex);
    
    total_amount -= withdrawal_amount;
    printf("Withdrawal amount = %d\n", total_amount);

    //Release mutex locks
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}