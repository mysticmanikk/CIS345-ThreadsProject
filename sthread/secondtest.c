
#define _REENTRANT
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include "sthread.h"


int balance;
sthread_rwlock_t mylock;


int slowreader(void *arg) {
  int threadno = (int)arg;
  sthread_read_lock(&mylock);
  printf("thread%d ===> This reader thread is very slow\n", threadno);
  sleep(10);
  sthread_read_unlock(&mylock);
  printf("thread%d ===> I am finally done! #############\n", threadno);
  return 0;
}


int bcheck(void *arg) {
    int i;
    int trubal;
    int threadno = (int)arg;
    sthread_read_lock(&mylock);
    trubal = balance;
    printf("thread%d ===> Reader thread iteration %d, balance = %d\n", threadno, i, trubal);
    fflush(stdout);
    sthread_read_unlock(&mylock);
    //sleep(1);
    return 0;
}

int deposit(void *arg) {
    int i, tmp;
    //k = 0;
  sleep(3);
    int threadno = (int)arg;
    sthread_write_lock(&mylock);
    balance += 1024;
    sthread_write_unlock(&mylock);
    printf("thread%d ===> Deposit thread iteration %d\n", threadno, i);
    fflush(stdout);
    return 0;
}

int withdraw(void *arg) {
  int i;
  sleep(3);
  int threadno = (int)arg;
    sthread_write_lock(&mylock);
    balance -= 100;
    sthread_write_unlock(&mylock);
    printf("thread%d ===> Withdrawal thread iteration %d\n", threadno, i);
    fflush(stdout);
  return 0;
}

int main(int argc, char *argv[])
{
  int i;
  balance = 10000;
  int destroyed = 0;

  sthread_t test[10];
  sthread_t desp[10];
  sthread_t with[10];
  sthread_t bch[10];
  sthread_rwlock_init(&mylock);

  if (sthread_init() == -1)
    fprintf(stderr, "%s: sthread_init: %s\n", argv[0], strerror(errno));



  for(int i = 0; i<10;i++){

      if((sthread_create(&test[i], slowreader, (void *)1) == -1)){
          printf("Something went wrong\n");
      }
      if((sthread_create(&desp[i], deposit, (void *)1) == -1)){
          printf("Something went wrong\n");
      }
      if((sthread_create(&with[i], withdraw, (void *)1) == -1)){
          printf("Something went wrong\n");
      }
  }

  while (1) {

    sleep(1);
    printf("the balnce is %d\n", balance);
    if (destroyed == 0) {
      sthread_rwlock_destroy(&mylock);
      destroyed = 1;
    }
  }

  return 0;
}

