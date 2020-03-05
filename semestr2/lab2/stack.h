#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMAX 100000

struct elem {
  char *filename;
  int len;
};

struct stack {
  struct elem elems[NMAX];
  int top;
};

void init(struct stack *stk);
void push(struct stack *stk, char *filename, int len);
struct elem pop(struct stack *stk);
int isempty(struct stack *stk);

#endif
