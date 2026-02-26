#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define CHAIRS 3

int num_students;
int wait_list = 0;

pthread_mutex_t mutex; //declaring mutex
sem_t sem_student; // counting sem
sem_t sem_ta; // counting sem

void *students(void *param); //threads call this function
void *ta(void *param); //threads call this function

int main(int argc, char *argv[])
{   
    num_students = atoi(argv[1]);
    if (num_students <= 0)
    {
        printf("num_students must be > 0\n");
        return 1;
    }

    pthread_t ta_tid;
    pthread_t student_tid[num_students];
    pthread_attr_t attr;                /* set of thread attributes */
    pthread_attr_init(&attr);     /* set the default attributes of the thread */
    int ids[num_students];

    //checking if mutex initialized properly
    if (pthread_mutex_init(&mutex, NULL) != 0)
        printf("Error in initializing mutex \n");

    //check if semaphore initialized properly
    if (sem_init(&sem_student, 0, 0) != 0)
        printf("Error in initializing semaphore \n");
    if (sem_init(&sem_ta, 0, 0) != 0)
        printf("Error in initializing semaphore \n");

    /* set the default attributes of the thread */
    pthread_attr_init(&attr);

    /* create the thread */
    pthread_create(&ta_tid, &attr, ta, NULL);

    for (int i = 0; i < num_students; i++)
    {
        ids[i] = i + 1;
        pthread_create(&student_tid[i], &attr, students, &ids[i]);
    }

    // Join (will run forever; ok unless you add a stop condition)
    pthread_join(ta_tid, NULL);
    for (int i = 0; i < num_students; i++)
        pthread_join(student_tid[i], NULL);

    return 0;
}

void *ta(void *param)
{
    (void)param;

    while (1)
    {
        // Sleep until at least one student arrives
        sem_wait(&sem_student);

        // One student leaves the hallway chairs to get help
        pthread_mutex_lock(&mutex);
        wait_list--;
        printf("TA starts helping. Waiting in hallway: %d\n", wait_list);
        pthread_mutex_unlock(&mutex);

        // Simulate helping time
        sleep(rand() % 3 + 1);

        // Tell ONE student: I'm ready / you're being helped
        sem_post(&sem_ta);
    }
    return NULL;
}

void *students(void *param)
{
    int id = *(int *)param;

    while (1)
    {
        // Simulate programming time
        sleep(rand() % 5 + 1);

        // Try to get help
        pthread_mutex_lock(&mutex);

        if (wait_list < CHAIRS)
        {
            wait_list++;
            printf("Student %d sits down. Waiting in hallway: %d\n", id, wait_list);

            // Notify TA a student is waiting (may wake TA)
            sem_post(&sem_student);

            pthread_mutex_unlock(&mutex);

            // Wait until TA is ready to help me
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
