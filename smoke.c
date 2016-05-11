#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create (agent->mutex);
  agent->match   = uthread_cond_create (agent->mutex);
  agent->tobacco = uthread_cond_create (agent->mutex);
  agent->smoke   = uthread_cond_create (agent->mutex);
  return agent;
}

struct table{
  int paper,match,tobacco;
    uthread_mutex_t mutex;
  uthread_cond_t pama,pato,mato,done_smoke;
  struct Agent* a;
};
struct table* creattable(struct Agent* ag){
  struct table* t = malloc(sizeof(struct table));
  t->a = ag;
  t->mutex = uthread_mutex_create();
  t->done_smoke = uthread_cond_create(t->mutex);
  t->pama = uthread_cond_create(t->mutex);
  t->pato = uthread_cond_create(t->mutex);
  t->mato = uthread_cond_create(t->mutex);
  t->paper = 0;
  t->match=0;
  t->tobacco=0;
  return t;
}

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked
void* mm_m(void* t){
  struct table* ta=t;
  uthread_cond_wait(ta->a->match);
  ta->match++;signal_count[1]++;
  for (int i=0;i<10;i++){
    if(ta->paper==1)
      uthread_cond_signal(ta->pama);
    if(ta->tobacco==1)
      uthread_cond_signal(ta->mato);
    uthread_cond_wait(ta->done_smoke);
    uthread_cond_signal(ta->a->smoke);
  }
  return NULL;
//
//  while(1) {
//    ta->match = 1;
//
//    if (ta->paper == 1)
//      uthread_cond_signal(ta->pama);
//    if (ta->tobacco == 1)
//      uthread_cond_signal(ta->mato);
//  }
}
void* mm_t(void* t) {
  struct table *ta = t;
  uthread_cond_wait(ta->a->tobacco);
  ta->tobacco++;
  signal_count[4]++;
  for (int i = 0; i < 10; i++) {
    if (ta->match == 1)
      uthread_cond_signal(ta->mato);
    if (ta->paper == 1)
      uthread_cond_signal(ta->pama);
    uthread_cond_wait(ta->done_smoke);
    uthread_cond_signal(ta->a->smoke);
  }
  return NULL;
}
//  while(1) {
//    printf("mm_t\n");
//    ta->tobacco = 1;
//    printf("try t");
//
//    if (ta->paper == 1)
//      uthread_cond_signal(ta->pato);
//    if (ta->match == 1)
//      uthread_cond_signal(ta->mato);
//  }

void* mm_p(void* t){
  struct table* ta=t;
  uthread_cond_wait(ta->a->paper);
  ta->paper++;
    ta->paper = 1;
    signal_count[2]++;
  for (int i = 0; i < 10; ++i) {
    if (ta->match == 1)
      uthread_cond_signal(ta->pama);
    if (ta->tobacco == 1)
      uthread_cond_signal(ta->pato);
    uthread_cond_wait(ta->done_smoke);
    uthread_cond_signal(ta->a->smoke);
  }
  return  NULL;
}
void* tobaccosmkr (void* ts){
  struct table* t = ts;
    uthread_cond_wait(t->pama);
    t->paper = 0;
    t->match = 0;
    smoke_count[4]++;
    uthread_cond_signal(t->done_smoke);
  return NULL;
}
void* papersmkr (void* ps){
  struct table* p=ps;
    uthread_cond_wait(p->mato);
    p->tobacco=0;
    p->match=0;
    smoke_count[2]++;
    uthread_cond_signal(p->done_smoke);
  return NULL;
}
void* matchsmkr (void* ms){
  struct table* m=ms;
  uthread_cond_wait(m->pato);
    m->paper=0;
    m->tobacco=0;
    smoke_count[1]++;
    uthread_cond_signal(m->done_smoke);
  return NULL;
}
/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};


/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        uthread_cond_signal (a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        uthread_cond_signal (a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        uthread_cond_signal (a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      uthread_cond_wait (a->smoke);
    }
  uthread_mutex_unlock (a->mutex);
  return NULL;
}

int main (int argc, char** argv) {
  uthread_init (7);
  struct Agent*  a = createAgent();
  struct table* t = creattable(a);
  uthread_join(uthread_create(tobaccosmkr,t),0);
  uthread_join(uthread_create(papersmkr, t),0);
  uthread_join(uthread_create(matchsmkr,t),0);
  uthread_join(uthread_create(mm_p,t),0);
  uthread_join(uthread_create(mm_t,t),0);
  uthread_join(uthread_create(mm_m,t),0);
  uthread_join (uthread_create (agent, t), 0);
  printf("may be\n");
  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}