/*-------------------------------------------------------------------*/
/* STACK.C        Timo Kaukoranta                                    */
/*                                                                   */
/*                                                                   */
/* - Implementation of stack                                         */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName        "STACK"
#define VersionNumber   "Version 0.01"
#define LastUpdated     "11.8.98"

/* ----------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>

#include "memctrl.h"
#include "stack.h"


/*====================  N O D E   S T A C K  =========================*/


STACK* S_make(void)
{
  STACK* tmp = allocate(sizeof(STACK));

  tmp->top = NULL;

  return( tmp );
}


/* ----------------------------------------------------------------- */



void S_free(STACK* S)
{
  assert( S != NULL );

  while( ! S_empty(S) )
    {
    S_pop(S);
    }
  deallocate(S);
}


/* ----------------------------------------------------------------- */


void S_push(STACK* S, void* item)
{
  STACKNODE* tmp = (STACKNODE*)allocate(sizeof(STACKNODE));

  assert( S != NULL );

  tmp->item = item;
  tmp->next = S->top;
  S->top    = tmp;
}


/*-------------------------------------------------------------------*/


void* S_pop(STACK* S)
{
  void*      item;
  STACKNODE* tmp;

  assert( S != NULL );

  if( S_empty(S) )
    {
    printf("\nERROR: Stack empty, cannot POP.\n");
    exit(-1);
    }
  tmp    = S->top;
  item   = tmp->item;
  S->top = tmp->next;
  deallocate(tmp);

  return( item );
}

void* S_peek(STACK* S)
{
  void*      item;
  STACKNODE* tmp;

  assert( S != NULL );

  if( S_empty(S) )
    {
    printf("\nERROR: Stack empty, cannot Peek.\n");
    exit(-1);
    }
  tmp    = S->top;
  item   = tmp->item;

  return( item );
}




