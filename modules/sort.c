/*-------------------------------------------------------------------*/
/* SORT.C         Pasi Fr„nti                                        */
/*                Timo Kaukoranta                                    */
/*                Juha Kivij„rvi                                     */
/*                                                                   */
/* - Sorting routines for codebook.                                  */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName        "SORT"
#define VersionNumber   "Version 0.09a"
#define LastUpdated     "27.7.99"

/* ----------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interfc.h"
#include "memctrl.h"
#include "sort.h"
#include "cb.h"

/*--------------------  Basic type definitions -----------------------*/

typedef  signed int   I32;

#define  Elem(i)      (((char*)base) + (i) * elemsize)

/*-----------------------  Stack strucutre  -------------------------*/

typedef struct STACKSTRUCT
          { I32                 data;
            struct STACKSTRUCT* next;
          } STACKNODE;
typedef STACKNODE*  STACK;

#define StackInit(s)     ( (*(s)) = NULL )
#define StackEmpty(s)    ( (*(s)) == NULL )


/*=========================  S T A C K  =============================*/


static void Push(I32 x, STACK* s)
{
  STACK tmp;

  tmp = (STACK)allocate(sizeof(STACKNODE));
  if( tmp == NULL )
    {
    ErrorMessage("\nERROR: allocate Push\n");
    ExitProcessing( -1 );
    }
  tmp->data = x;
  tmp->next = (*s);
  (*s)      = tmp;
}


/*-------------------------------------------------------------------*/


static I32 Pop(STACK* s)
{
  I32   x;
  STACK tmp;

  if( StackEmpty(s) )
    {
    ErrorMessage("\nERROR: Stack empty, cannot POP.\n");
    ExitProcessing(-1);
    }
  tmp   = (*s);
  x     = (*s)->data;
  (*s)  = (*s)->next;
  deallocate(tmp);
  return( x );
}


/*=========================  S O R T  ===============================*/


static void SwapCells(char* a, char* b, int elemsize)
{
  char tmp;
  int  k;

  for( k = 0; k < elemsize; k++ )
    {
    tmp = *a;
    *a  = *b;
    *b  = tmp;
    a++;
    b++;
    }
}


/*-------------------------------------------------------------------*/


static void BubbleSort(void* base,
                       int   elemsize,
                       void* info,
                       int (*cmp)(const void *e1,
                                  const void *e2,
                                  const void *info),
                       I32   First,
                       I32   Last)
{
  I32 i, j;

  for( i = First; i < Last; i++ )
    {
    for( j = i+1; j <= Last; j++ )     /* What if: Last==2^31 ??? */
      {
      if( cmp(Elem(j), Elem(i), info) )
        {
        SwapCells(Elem(i), Elem(j), elemsize);
        }
      }
    }
}


/* ----------------------------------------------------------------- */


void InsertSort(void* base,
                int   nelem,
                int   elemsize,
                void* info,
                int (*cmp)(const void *e1,
                           const void *e2,
                           const void *info))
{
  I32 i, j;

  for( i = 0; i < nelem - 1; i++ )
    {
    j = i + 1;
    while( j > 0 && cmp(Elem(j), Elem(j-1), info) )
      {
      SwapCells(Elem(j), Elem(j-1), elemsize);
      j--;
      }
    }
}

/*
static void InsertSort(void* base,
                       int   elemsize,
                       void* info,
                       int (*cmp)(const void *e1,
                                  const void *e2,
                                  const void *info),
                       I32   First,
                       I32   Last)
{
  I32 i, j;

  for( i = First; i < Last; i++ )
    {
    j = i + 1;
    while( j > First && cmp(Elem(j), Elem(j-1), info) )
      {
      SwapCells(Elem(j), Elem(j-1), elemsize);
      j--;
      }
    }
}
*/

/*-------------------------------------------------------------------*/


static I32 MakePartition(void* base,
                         int   elemsize,
                         void* info,
                         int (*cmp)(const void *e1,
                                    const void *e2,
                                    const void *info),
                         I32       First,
                         I32       Last)
{
  I32   left  = First - 1;
  I32   right = Last  + 1;
  I32   Half  = First + (Last - First) / 2;
  void* Pivot = allocate(elemsize * sizeof(char));

  memcpy(Pivot, Elem(Half), elemsize);

  while( 1 )
    {
    do
      {
      left++;
      if( left >= right + 2 ) { ErrorMessage("ERROR: left=%i right=%i when left++\n",left,right); ExitProcessing( -1 ); }
      } while( cmp(Elem(left), Pivot, info));
    do
      {
      right--;
      if( left >= right + 2 ) { ErrorMessage("ERROR: left=%i right=%i when right++\n",left,right); ExitProcessing( -1 ); }
      } while( cmp(Pivot, Elem(right), info));
    if( left < right )
      {
      SwapCells(Elem(left), Elem(right), elemsize);
      }
    else
      {
      deallocate(Pivot);
      return( right );
      }
    }
  return( 0 );
}


/*-------------------------------------------------------------------*/


void QuickSort(void* base,
               int   nelem,
               int   elemsize,
               void* info,
               int (*cmp)(const void *e1,
                          const void *e2,
                          const void *info))
{
  I32    BreakPoint;
  I32    first, last;
  STACK  s;

  StackInit(&s);
  Push(0, &s);
  Push(nelem-1, &s);

  while( ! StackEmpty(&s) )
    {
    last  = Pop(&s);
    first = Pop(&s);
    if( first > last ) { ErrorMessage("ERROR: SORT; first>last (%i>%i)\n", first, last); ExitProcessing(-1); }
    if( last-first < 5 )
      {
      InsertSort(Elem(first), last - first + 1, elemsize, info, cmp);
      }
    else
      {
      BreakPoint = MakePartition(base, elemsize, info, cmp, first, last);
      Push(first, &s);
      Push(BreakPoint, &s);
      Push(BreakPoint+1, &s);
      Push(last, &s);
      }
    } /* end while */
}


