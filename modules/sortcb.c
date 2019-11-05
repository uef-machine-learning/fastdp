/*-------------------------------------------------------------------*/
/* CBSORT.C       Timo Kaukoranta                                    */
/*                Pasi Fränti                                        */
/*                Juha Kivijärvi                                     */
/*                                                                   */
/* - Sorting routines for codebook.                                  */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName        "SORTCB"
#define VersionNumber   "Version 0.05"
#define LastUpdated     "2.5.00"

/* ----------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include "cb.h"
#include "interfc.h"
#include "sort.h"
#include "sortcb.h"


/* ----------------------------------------------------------------- */


char* SortModeStr[] = { "DATA_DESCENDING", "FREQ_DESCENDING",
                        "DATA_ASCENDING", "FREQ_ASCENDING",
                        "VECTOR_MEAN" };


/*--------------------------------------------------------------------*/


int cmpFreqDes(const void* a, const void* b, const void* info)
{
  return( ((BOOKNODE*)a)->freq > ((BOOKNODE*)b)->freq ? 1 : 0 );
}


/* ----------------------------------------------------------------- */


static int cmpDataDes(const void* a, const void* b, const void* info)
{
  return( VectorGreaterThan( ((BOOKNODE*)a)->vector,
                             ((BOOKNODE*)b)->vector,
                             *((int*)info)) );
}


/* ----------------------------------------------------------------- */


static int cmpFreqAsc(const void* a, const void* b, const void* info)
{
  return( ((BOOKNODE*)a)->freq < ((BOOKNODE*)b)->freq ? 1 : 0 );
}


/* ----------------------------------------------------------------- */


static int cmpDataAsc(const void* a, const void* b, const void* info)
{
  return( VectorGreaterThan( ((BOOKNODE*)b)->vector,
                             ((BOOKNODE*)a)->vector,
                             *((int*)info)) );
}


/* ----------------------------------------------------------------- */


static int cmpVecMean(const void* a, const void* b, const void* info)
/* Ascending order assumed */
{
  return( ((BOOKNODE*)a)->vmean < ((BOOKNODE*)b)->vmean ? 1 : 0 );
}


/* ----------------------------------------------------------------- */


static int GreaterThan(BOOKNODE* v1, BOOKNODE* v2, int Vsize, int Mode)
{
  switch( Mode )
    {
    case FREQ_DESCENDING:  
      return( v1->freq > v2->freq ? YES : NO );
    case DATA_DESCENDING:  
      return( VectorGreaterThan( v1->vector, v2->vector, Vsize) );
    case FREQ_ASCENDING:
      return( v2->freq > v1->freq ? YES : NO );
    case DATA_ASCENDING:  
      return( VectorGreaterThan( v2->vector, v1->vector, Vsize) );
    case VECTOR_MEAN:
      return( v2->vmean > v1->vmean ? YES : NO );
    default:    
      return( NO );
    }
}


/*-------------------------------------------------------------------*/


static void ValidityOfCodebookSorting(CODEBOOK* CB, int Mode)
{
  int  i;

  for( i = 0; i < BookSize(CB)-1; i++)
    {
    if( GreaterThan(&Node(CB,i+1), &Node(CB, i), VectorSize(CB), Mode) )
      {
      ErrorMessage("ERROR: Sorting CodeBook (mode=%s) was NOT valid.\n",
                    SortModeStr[Mode-1]);
      }
    }
}


/*-------------------------------------------------------------------*/


void SortCodebook(CODEBOOK* CB, int Mode)
{
  int (*cmp)(const void *e1,
             const void *e2,
             const void *info) = NULL;
  int info = VectorSize(CB);

  if( (CB->CodebookSize > 0)  &&  Mode > 0 )
    {
    switch( Mode )
      {
      case FREQ_DESCENDING: cmp = cmpFreqDes; break;
      case DATA_DESCENDING: cmp = cmpDataDes; break;
      case FREQ_ASCENDING:  cmp = cmpFreqAsc; break;
      case DATA_ASCENDING:  cmp = cmpDataAsc; break;
      case VECTOR_MEAN:     cmp = cmpVecMean; break;
      default:              ErrorMessage("ERROR: SortCodebook Mode=%i\n", Mode);
                            ExitProcessing( -1 );
      }

    QuickSort(CB->Book, BookSize(CB), sizeof(BOOKNODE), &info, cmp);

    ValidityOfCodebookSorting(CB, Mode);
    }
}

