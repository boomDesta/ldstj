#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

#define MAX_OCCUPANCY      3
#define NUM_ITERATIONS     100
#define NUM_PEOPLE         20
#define FAIR_WAITING_COUNT 4

/**
 * You might find these declarations useful.
 */
enum Sex {MALE = 0, FEMALE = 1};
const static enum Sex otherSex [] = {FEMALE, MALE};

struct Washroom {
    uthread_mutex_t mutex;
    uthread_cond_t male_can_enter;
    uthread_cond_t female_can_enter;
    int fairness;
    int number_inside;
    int gender_inside;
};

struct Washroom* createWashroom() {
  struct Washroom* washroom = malloc (sizeof (struct Washroom));
  washroom->mutex = uthread_mutex_create();
  washroom->female_can_enter = uthread_cond_create(washroom->mutex);
  washroom->male_can_enter = uthread_cond_create(washroom->mutex);
  washroom->fairness = 0;
  washroom->number_inside=0;
  washroom->gender_inside = FEMALE;
  return washroom;
}

#define WAITING_HISTOGRAM_SIZE (NUM_ITERATIONS * NUM_PEOPLE)
int             entryTicker;  // incremented with each entry
int             waitingHistogram         [WAITING_HISTOGRAM_SIZE];
int             waitingHistogramOverflow;
uthread_mutex_t waitingHistogrammutex;
int             occupancyHistogram       [2] [MAX_OCCUPANCY + 1];

void enterWashroom (struct Washroom* washroom, enum Sex Sex) {
  for(int i=0;i<NUM_ITERATIONS;i++) {
    uthread_mutex_lock(washroom->mutex);
    int n = washroom->number_inside;
    int g = washroom->gender_inside;
    if (n == 3) {
      if (Sex == 1)uthread_cond_wait(washroom->female_can_enter);
      else uthread_cond_wait(washroom->male_can_enter);
    }
     if (n<3 && g != Sex) {
      if (Sex == 1) uthread_cond_wait(washroom->female_can_enter);
      else uthread_cond_wait(washroom->male_can_enter);
    }
    if (washroom->fairness == 4) {
      if (Sex == 1)uthread_cond_wait(washroom->female_can_enter);
      else uthread_cond_wait(washroom->male_can_enter);
    }
    washroom->number_inside++;
    washroom->fairness++;
    entryTicker++;
    occupancyHistogram[washroom->number_inside];
    uthread_mutex_unlock(washroom->mutex);
    // TODO
  }
}

void leaveWashroom (struct Washroom* washroom, enum Sex sex) {
  uthread_mutex_lock(washroom->mutex);
  if(washroom->number_inside <3 && washroom->fairness<4) {
    if(washroom->gender_inside==1) uthread_cond_signal(washroom->female_can_enter);
    else uthread_cond_signal(washroom->male_can_enter);
  }
  if(washroom->fairness==4 && washroom->number_inside==0) {
    washroom->gender_inside == otherSex[washroom->gender_inside];
    washroom->fairness = 0;
  }
  washroom->number_inside--;
  uthread_mutex_unlock(washroom->mutex);
  // TODO
}

void recordWaitingTime (int waitingTime) {
  for(int i=0;i<NUM_ITERATIONS;i++) {
    uthread_mutex_lock(waitingHistogrammutex);
    if (waitingTime < WAITING_HISTOGRAM_SIZE)
      waitingHistogram[waitingTime]++;
    else
      waitingHistogramOverflow++;
    uthread_mutex_unlock(waitingHistogrammutex);
  }
}

//
// TODO
// You will probably need to create some additional produres etc.
//
void* person (struct Washroom* washroom){
  enum Sex your_gender = rand()%2;
  for(int i=0;i<NUM_ITERATIONS;i++) {
    enterWashroom(washroom, your_gender);
    recordWaitingTime(entryTicker);
    for(int i=0;i<NUM_PEOPLE;i++){
      uthread_yield();
    }
    leaveWashroom(washroom,your_gender);
    for(int j=0;j<NUM_PEOPLE;j++)
      uthread_yield();
  }
}

int main (int argc, char** argv) {
  uthread_init (1);
  struct Washroom* washroom = createWashroom();
  uthread_t        pt [NUM_PEOPLE];
  waitingHistogrammutex = uthread_mutex_create ();
  for(int i=0;i<NUM_PEOPLE;i++){
    pt[i] = uthread_create(person,washroom);
  }
  // TODO
  for(int j=0;j<NUM_PEOPLE;j++)
    uthread_join(pt[j],0);
  printf ("Times with 1 male    %d\n", occupancyHistogram [MALE]   [1]);
  printf ("Times with 2 males   %d\n", occupancyHistogram [MALE]   [2]);
  printf ("Times with 3 males   %d\n", occupancyHistogram [MALE]   [3]);
  printf ("Times with 1 female  %d\n", occupancyHistogram [FEMALE] [1]);
  printf ("Times with 2 females %d\n", occupancyHistogram [FEMALE] [2]);
  printf ("Times with 3 females %d\n", occupancyHistogram [FEMALE] [3]);
  printf ("Waiting Histogram\n");
  for (int i=0; i<WAITING_HISTOGRAM_SIZE; i++)
    if (waitingHistogram [i])
      printf ("  Number of times people waited for %d %s to enter: %d\n", i, i==1?"person":"people", waitingHistogram [i]);
  if (waitingHistogramOverflow)
    printf ("  Number of times people waited more than %d entries: %d\n", WAITING_HISTOGRAM_SIZE, waitingHistogramOverflow);
}