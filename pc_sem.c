#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

struct Pool {
  uthread_sem_t mutex;
    uthread_sem_t notEmpty;
    uthread_sem_t notFull;
  int           items;
};

struct Pool* createPool() {
  struct Pool* pool = malloc (sizeof (struct Pool));
  // TODO
  pool->mutex = uthread_sem_create(1);
  pool->notEmpty = uthread_sem_create(0);
  pool->notFull = uthread_sem_create(0);
  pool->items     = 0;
  return pool;
}

void* producer (void* pv) {
  struct Pool* p = pv;
  for (int i=0;i<NUM_ITERATIONS;i++){
    uthread_sem_wait(p->mutex);
    while (p->items>=MAX_ITEMS) {
      producer_wait_count++;
      uthread_sem_signal(p->mutex);
      uthread_sem_wait(p->notFull);
      uthread_sem_wait(p->mutex);
    }
    assert(0<=p->items<=MAX_ITEMS);
    p->items++;
    histogram[p->items]+=1;
    uthread_sem_signal(p->notEmpty);

    uthread_sem_signal(p->mutex);
  }
  // TODO
  return NULL;
}

void* consumer (void* pv) {
  struct Pool* p = pv;
  for(int i=0;i<NUM_ITERATIONS;i++){
    uthread_sem_wait(p->mutex);
    while(p->items<=0) {
      consumer_wait_count++;
      uthread_sem_signal(p->mutex);
      uthread_sem_wait(p->notEmpty);
      uthread_sem_wait(p->mutex);
    }
    assert(0<=p->items<=MAX_ITEMS);
    p->items--;
    uthread_sem_signal(p->notFull);
    histogram[p->items]+=1;
    uthread_sem_signal(p->mutex);
  }
  // TODO
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
  struct Pool* p = createPool();
  t[0] = uthread_create(producer,p);
  t[1] = uthread_create(producer,p);
  t[2] = uthread_create(consumer,p);
  t[3]= uthread_create(consumer,p);
  uthread_join(t[0], 0);
  uthread_join(t[1], 0);
  uthread_join(t[2], 0);
  uthread_join(t[3], 0);
  // TODO
  
  printf ("producer_wait_count=%d, consumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}