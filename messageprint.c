
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CV = PTHREAD_COND_INITIALIZER;
int number_prime_finished = 0;
int numberPrimes;

void *threaded_function(void *arg);
int prime(unsigned long number);


int main(int argc, char *argv[]){
    if(argv[1] == NULL){
        printf("Number of threads not specified\n");
        exit(0);
    }
    //Quick and dirty way of doing this
    int threadNum = atoi(argv[1]);

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
    }


    for(int i = 0; i < threadNum; i++){
        pthread_join(arrThread[i], NULL);
    }

}

void *threaded_function(void *arg){
    int threadID = *(int *)arg;
    pthread_t thread_id = pthread_self();

    pthread_mutex_lock(&lock);
    if((prime(threadID)) == 0){
        printf("This is a prime thread, ID: %d\n", threadID);
        number_prime_finished++;
        if(number_prime_finished == numberPrimes){
            pthread_cond_broadcast(&CV);
        }
        pthread_mutex_unlock(&lock);
    }
    else{
        while(number_prime_finished < numberPrimes){
            pthread_cond_wait(&CV, &lock);
        }
        printf("This is a composite thread, ID: %d\n", threadID);
        pthread_mutex_unlock(&lock);
    }

    return 0;
}

int prime(unsigned long number){
    if((number == 0) | (number == 1)){
        return 1;
    }
    for(unsigned long i = 2; i < number; i++){
        if(number % i == 0){
            return 1;
        }
    }
    return 0;
}

