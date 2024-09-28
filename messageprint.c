
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CV = PTHREAD_MUTEX_INITIALIZER;
int number_prime_finished = 0;
void *threaded_function(void *arg);


int main(int argc, char *argv[]){
    //Quick and dirty way of doing this
    int threadNum = atoi(argv[1]);

    printf("%d\n", threadNum);

    pthread_t arrThread[threadNum];
    int arrID[threadNum];

    for(int i = 0; i < threadNum; i++){
        arrID[i] = i;
        pthread_create(&arrThread[i],NULL,threaded_function,&arrID[i]);
        pthread_join(arrThread[i], NULL);
    }

}

void *threaded_function(void *arg){
    int threadID = *(int *)arg;
    printf("This is a thread\n");
    
    return 0;
}
