#ifndef __STACK_H
#define __STACK_H

/*----------------------------  Stack  ------------------------------*/

struct STACKNODESTRUCT
  { void*                   item;
    struct STACKNODESTRUCT* next;
  };

typedef struct STACKNODESTRUCT      STACKNODE;
typedef struct { STACKNODE* top; }  STACK;

/* ----------------------------------------------------------------- */

STACK* S_make(void);
void   S_free(STACK* S);
void   S_push(STACK* S, void* item);
void*  S_pop(STACK* S);
void*  S_peek(STACK* S);

#define S_empty(S)  ((S)->top == NULL)

/* ----------------------------------------------------------------- */

#endif /* __STACK_H */

