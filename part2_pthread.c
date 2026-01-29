#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NUM_THREADS 10

int total_amount = 0; /* this data is shared by the thread(s) */
void *deposit(void *param); //threads call this function
void *withdrawal(void *param); //threads call this function
pthread_mutex_t mutex; //declaring mutex
sem_t sem_can_deposit;
sem_t sem_can_withdrawal;

int main(int argc, char *argv[])
{
    pthread_t tid[NUM_THREADS];        /* Creating thread identifiers for 6 threads */
    pthread_attr_t attr;                /* set of thread attributes */
    pthread_attr_init(&attr);     /* set the default attributes of the thread */

    //checking if mutex initialized properly
    if (pthread_mutex_init(&mutex, NULL) != 0)
        printf("Error in initializing mutex \n");

    //check if semaphore initialized properly
    if (sem_init(&sem_can_deposit, 0, 4) != 0)
        printf("Error in initializing semaphore \n");
    if (sem_init(&sem_can_withdrawal, 0, 0) != 0)
        printf("Error in initializing semaphore \n");

    /* set the default attributes of the thread */
    pthread_attr_init(&attr);

    /* create the thread */
    // 3 threads for deposit and 7 threads for withdrawal
    for (int i = 0; i < 3; i++)
        pthread_create(&tid[i], &attr, withdrawal, argv[1]);
    
    for (int i = 3; i < 10; i++)
        pthread_create(&tid[i], &attr, deposit, argv[1]);

    /* wait for the thread to exit */
    // parent thread join again, can create for loop to join more than 1 threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(tid[i], NULL);

    printf("total_amount = %d\n", total_amount);
}

void *deposit(void *param)
{
    printf("Executing deposit function\n");
    int deposit_amount = atoi(param);

    //acquire deposit semaphore
    sem_wait(&sem_can_deposit);

    //acquire mutex lock
    pthread_mutex_lock(&mutex);

    total_amount += deposit_amount;
    printf("Amount after deposit = %d\n", total_amount);

    //Release mutex locks
    pthread_mutex_unlock(&mutex);

    //Release withdraw semaphore
    sem_post(&sem_can_withdrawal);

    pthread_exit(0);

    return NULL;

}

void *withdrawal(void *param)
{
    printf("Executing Withdraw function\n");
    int withdrawal_amount = atoi(param);

    //acquire semaphore
    sem_wait(&sem_can_withdrawal);

    //acquire mutex lock
    pthread_mutex_lock(&mutex);
    
    total_amount -= withdrawal_amount;
    printf("Amount after Withdrawal = %d\n", total_amount);

    //Release mutex locks
    pthread_mutex_unlock(&mutex);

    //Release semaphore
    sem_post(&sem_can_deposit);

    pthread_exit(0);

    return NULL;
}