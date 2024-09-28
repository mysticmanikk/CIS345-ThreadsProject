
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CV = PTHREAD_MUTEX_INITIALIZER;
int number_prime_finished = 0;

void *threaded_function(void *arg);
int prime(int number);


int main(int argc, char *argv[]){
    if(argv[1] == NULL){
        printf("Number of threads not specified\n");
        exit(0);
    }
    //Quick and dirty way of doing this
    int threadNum = atoi(argv[1]);
    int numberPrimes;

    //Prime Number finder
    for(int i = 0; i < threadNum;i++){
        if(prime(i) == 0){
            numberPrimes++;
        }
    }
    printf("Number of primes: %d\n", numberPrimes);

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

int prime(int number){
    if((number == 0) | (number == 1)){
        return 1;
    }
    for(int i = 2; i < number; i++){
        if(number % i == 0){
            return 1;
        }
    }
    return 0;
}




