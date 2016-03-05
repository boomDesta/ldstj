#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Element {
  char   name[200];
  struct Element *next;
};

struct Element *top = 0;

void push (char* aName) {
  struct Element* e = malloc (sizeof (*e));    // Not the bug: sizeof (*e) == sizeof(struct Element)
  strncpy (e->name, aName, sizeof (e->name));  // Not the bug: sizeof (e->name) == 200
  e->next  = top;
  top = e;
}

void pop(char *c) {
  struct Element* e = top;
  top = e -> next;
  strncpy(c,e->name, 200);
  free (e);
}

int main (int argc, char** argv) {
  push ("A");
  push ("B");
  char w[200];
  pop(w);
  push ("C");
  push ("D");
  char x[200];
  pop(x);
  char y[200];
  pop(y);
  char z[200];
  pop(z);
  printf ("%s %s %s %s\n", w, x, y, z);
}
