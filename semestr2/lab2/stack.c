#include "stack.h"

void init(struct stack *stk) {
  stk->top = 0;
}

void push(struct stack *stk, char *filename, int len) {
  if (stk->top < NMAX)
  {
    stk->elems[stk->top].filename = calloc(strlen(filename), sizeof(char));
    strcpy(stk->elems[stk->top].filename, filename);
    stk->elems[stk->top].len = len;
    stk->top++;
  } else
    printf("Стек полон!\n");
}

struct elem pop(struct stack *stk) {
  struct elem el;
  if ((stk->top) > 0)
  {
    stk->top--;
    el.filename = calloc(strlen(stk->elems[stk->top].filename), sizeof(char));
    strcpy(el.filename, stk->elems[stk->top].filename);
    free(stk->elems[stk->top].filename);
    el.len = stk->elems[stk->top].len;
    return el;
  }
  else
  {
    printf("Стек пуст!\n");
    return el;
  }
}

int isempty(struct stack *stk) {
  if(stk->top == 0)
    return 1;
  else
    return 0;
}
