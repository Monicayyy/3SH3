#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define CHAIRS 3

int num_students; // type in an integer as an input
int wait_list = 0;

pthread_mutex_t mutex; //declaring mutex used for protect wait list
sem_t sem_student;
sem_t sem_ta;

void *students(void *param); //threads call this function
void *ta(void *param); //threads call this function

int main(int argc, char *argv[])
{   
    if (argc != 2)
    {
        printf("Need to type: output file name & student number as inputs");
        return 1;
    }

    num_students = atoi(argv[1]);
    if (num_students <= 0)
    {
        printf("num_students must be > 0\n");
        return 1;
    }

    srand((unsigned)time(NULL)); // random time

    pthread_t ta_tid;
    pthread_t student_tid[num_students];
    pthread_attr_t attr;          /* set of thread attributes */
    pthread_attr_init(&attr);     /* set the default attributes of the thread */
    int ids[num_students];

    // checking if mutex initialized properly
    if (pthread_mutex_init(&mutex, NULL) != 0)
        printf("Error in initializing mutex \n");

    // initialize and check if semaphore initialized properly
    if (sem_init(&sem_student, 0, 0) != 0)
        printf("Error in initializing semaphore \n");
    if (sem_init(&sem_ta, 0, 0) != 0)
        printf("Error in initializing semaphore \n");

    // set the default attributes of the thread
    pthread_attr_init(&attr);

    // create TA thread
    pthread_create(&ta_tid, &attr, ta, NULL);

    // create student thread
    for (int i = 0; i < num_students; i++)
    {
        ids[i] = i + 1; // starting from 1
        pthread_create(&student_tid[i], &attr, students, &ids[i]);
    }

    // TA and student thread join
    pthread_join(ta_tid, NULL);
    for (int i = 0; i < num_students; i++)
        pthread_join(student_tid[i], NULL);

    return 0;
}

void *ta(void *param)
{
    while (1)
    {
        // ta sleep until at least one student arrives
        sem_wait(&sem_student);

        // one student leaves the chair(wait list) to get help
        pthread_mutex_lock(&mutex);
        wait_list--;
        printf("TA starts helping. Number of students waiting in hallway: %d\n", wait_list);
        pthread_mutex_unlock(&mutex);

        // simulate helping time
        sleep(rand() % 3 + 1);

        // tell ONE student I am ready to help
        sem_post(&sem_ta);
    }
    return NULL;
}

void *students(void *param)
{
    int id = *(int *)param;

    while (1)
    {
        // simulate programming time
        sleep(rand() % 5 + 1);

        // try to get help
        pthread_mutex_lock(&mutex);

        if (wait_list < CHAIRS)
        {
            wait_list++;
            printf("Student %d sits down. Number of students waiting in hallway: %d\n", id, wait_list);

            // notify TA a student is waiting and wake TA
            sem_post(&sem_student);

            pthread_mutex_unlock(&mutex);

            // wait until TA is ready to help
            sem_wait(&sem_ta);

            printf("Student %d is getting help now.\n", id);
        }
        else
        {
            printf("Student %d leaves (no chair).\n", id);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}
