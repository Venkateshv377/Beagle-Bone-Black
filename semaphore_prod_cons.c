#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define BUFF_SIZE 4
#define FULL 0
#define EMPTY 0
char buffer[BUFF_SIZE];
int nextIn = 0;
int nextOut = 0;

sem_t empty_sem_mutex; //producer semaphore
sem_t empty_sem_mutex2; //producer semaphore
sem_t full_sem_mutex; //consumer semaphore

void * Producer1()
{
    int i;
    char item;
    for(i = 0; i < 8; i++)
    {
        sem_wait(&empty_sem_mutex); //get the mutex to fill the buffer
        item = (char)('A'+ i % 26);
        buffer[nextIn] = item;
        nextIn = (nextIn + 1) % BUFF_SIZE;
        printf("%s: Producing %c ...nextIn %d..Ascii=%d\n", __func__, item,nextIn,item);
        if(nextIn==FULL)
        {
            sem_post(&full_sem_mutex);
            sleep(1);
        }
        sem_post(&empty_sem_mutex);
    }
    sem_post(&empty_sem_mutex2);
}

int flag = 0;
void * Producer2()
{
    int i;
    char item;
    for(i = 0; i < 8; i++)
    {
        sem_wait(&empty_sem_mutex2); //get the mutex to fill the buffer
        item = (char)('I'+ i % 26);
        buffer[nextIn] = item;
        nextIn = (nextIn + 1) % BUFF_SIZE;
        printf("%s: Producing %c ...nextIn %d..Ascii=%d\n", __func__, item,nextIn,item);
        if(nextIn==FULL)
        {
            sem_post(&full_sem_mutex);
            sleep(1);
        }
        sem_post(&empty_sem_mutex2);
    }
    flag = 1;
    sem_post(&full_sem_mutex); // gain the mutex to consume from buffer
}

void * Consumer()
{
    int i, item;
    while(1){
        for(i = 0; i < 8; i++)
        {
            sem_wait(&full_sem_mutex); // gain the mutex to consume from buffer

            item = buffer[nextOut];
            nextOut = (nextOut + 1) % BUFF_SIZE;
            printf("\t...Consuming %c ...nextOut %d..Ascii=%d\n",item,nextOut,item);
            if(nextOut==EMPTY) //its empty
            {
                sleep(1);
                sem_wait(&full_sem_mutex); // gain the mutex to consume from buffer
            }
            sem_post(&full_sem_mutex);
        }
        if (flag == 1)  //Once consumer takes out the data from producer2, nothing is left so consumer stops execution
            break;
    }
}

int main()
{
    pthread_t ptid, ptid2,ctid;
    //initialize the semaphores

    sem_init(&empty_sem_mutex,0,1);
    sem_init(&empty_sem_mutex2,0,0);
    sem_init(&full_sem_mutex,0,0);

    //creating producer and consumer threads

    if(pthread_create(&ptid, NULL,Producer1, NULL))
    {
        printf("\n ERROR creating thread 1");
        exit(1);
    }

    if(pthread_create(&ptid2, NULL,Producer2, NULL))
    {
        printf("\n ERROR creating thread 1");
        exit(1);
    }

    if(pthread_create(&ctid, NULL,Consumer, NULL))
    {
        printf("\n ERROR creating thread 2");
        exit(1);
    }

    if(pthread_join(ptid, NULL)) /* wait for the producer to finish */
    {
        printf("\n ERROR joining thread");
        exit(1);
    }

    if(pthread_join(ptid2, NULL)) /* wait for the producer to finish */
    {
        printf("\n ERROR joining thread");
        exit(1);
    }

    if(pthread_join(ctid, NULL)) /* wait for consumer to finish */
    {
        printf("\n ERROR joining thread");
        exit(1);
    }

    sem_destroy(&empty_sem_mutex);
    sem_destroy(&empty_sem_mutex2);
    sem_destroy(&full_sem_mutex);

    //exit the main thread

    pthread_exit(NULL);
    return 1;
}
