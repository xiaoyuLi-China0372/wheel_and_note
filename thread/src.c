#include<stdio.h>  
#include<pthread.h>  
#include<string.h>  
#include<sys/types.h>  
#include<semaphore.h>
#include<unistd.h>  
pthread_t thread_1;  
sem_t sem;
pthread_mutex_t mutex;

void process(int *buf)
{
    printf("customer buffer...\n");
}

int buffer[100];

void *thread1(void *arg)  
{  
    int buf[100];
    while (1) {
        sem_wait(&sem);
        pthread_mutex_lock(&mutex); 
        memcpy(buf, buffer, sizeof(buf));
        pthread_mutex_unlock(&mutex);
        process(buf);
    }
}

int main()  
{  
    int err;  
    pthread_mutex_init(&mutex, NULL);
    err = sem_init(&sem, 0, 0);
    if(err != 0){  
        printf("create sem error: %s/n",strerror(err));  
        return 1;  
    }  
    err = pthread_create(&thread_1, NULL, thread1, NULL); 
    if(err != 0){  
        printf("create thread error: %s/n",strerror(err));  
        return 1;  
    }  
    while (1) {
        pthread_mutex_lock(&mutex); 
        for (int i = 0; i < 100; i++) {
            buffer[i] = i;
        }
        pthread_mutex_unlock(&mutex);
        printf("produce buffer...\n");
        sem_post(&sem);
        sleep(1);
    }
    return 0;  
}
