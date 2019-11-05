/*-------------------------------------------------------------------*/
/* RANDOM.C       Timo Kaukoranta                                    */
/*                                                                   */
/* - Random functions.                                               */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName        "RANDOM"
#define VersionNumber   "Version 0.08a"
#define LastUpdated     "16.10.2000"

/* ----------------------------------------------------------------- */

#include <stdio.h>
/************ random.c . Random numbers  ***********/
/* Tausworthes generator. This file contains C-version. Assembler
 * version (HP 720) is in the file randoma.s
 */

/* S. Tezuka and P.L Equyer: Efficient portable combined Tausworthe
 * random number generators. ACM. Transactions on Modelling and
 * Computer Simulation 1 (1991) 99-112.
 * Time in HP 720 (Tarzan): 1.36 microsec.
 *
 * Contents:
 * Long random integers in [0,2^31-1]:       BRAND() (Macro)
 * Long random integers in [ia,ib] (long):   irand(ia,ib)
 * Float random numbers in (0,1):            frand()
 * Double random numbers in (0,1):           drand()
 * Set seeds to ia and ib (unsigned long):   setrangen2(ia,ib)
 * Store seeds to ia and ib (unsigned long): getseeds2(&ia,&ib)
 */
#include <stdlib.h>
#include <math.h>
#include "random.h"

unsigned long I1 = 12345UL, I2 = 67890UL; /* Seeds. Global */

/* Definitions of constants */
#define  Mask1 0x7fffffffUL
#define  Mask2 0x1fffffffUL
#define  Todoub 4.656612873e-10
#define  Tofloat 4.656612e-10f

/* Basic random sequence. C-version */
long CombTaus(void)
{
unsigned long b;
b = ((I1 << 13) ^ I1) & Mask1;
I1 = ((I1 << 12) ^ (b >> 19)) & Mask1;
b = ((I2 << 2) ^ I2) & Mask2;
I2 = ((I2 << 17) ^ (b >> 12)) & Mask2;
return I1 ^ (I2 << 2);
}

/*+++ Random integer in ia..ib +++*/
long irand (long ia, long ib)
{
long range = ib-ia+1,rn;
do {rn = BRAND()/(Mask1/range); } while (rn >= range);
return ia + rn;
}


/*+++ Random double in [0,1) +++*/
double drand(void)
{ return Todoub*BRAND(); }


/*+++ Random float in [0,1) +++*/
float frand(void)
{ return Tofloat*BRAND(); }


/*+++ Set seeds of the random number generator +++*/
void setrangen2(unsigned long seed1, unsigned long seed2)
{ I1 = seed1 & Mask1; I2 = seed2 & Mask2; }


/*+++ Get current seeds from the random number generator +++*/
void getseeds2(unsigned long *pia, unsigned long *pib)
{ *pia = I1; *pib = I2; }


/* ----------------------------------------------------------- */
/* Extensions by Timo Kaukoranta                               */

#include <time.h>
/* #include <sys/timeb.h> */
#ifndef MSVC
#include <unistd.h>
#else
#include <process.h>
#define getpid() _getpid()
#endif
/*+++ Set seeds using time-function or given seed +++*/
void initrandom(unsigned long seed)
{
    int i;
    unsigned long t;

    if( seed == 0L )
    {
        t = time(NULL) ^ (getpid() << 2);
    }
    else
    {
        t = seed;
    }

    //printf("t: %lu %d\n",t,getpid());

    I1 = t & Mask1;
    I2 = (t ^ Mask2) & Mask2;


    // [SS]: With some seeds, the first random numbers don't seem to be very random.
    // Fix by generating 5000 random numbers.
    // TODO: Should find a better fix for this
    for(i=0;i<5000;i++)
    {
        CombTaus();
    }

}


/* ----------------------------------------------------------------- */


static void SwapElems(char* a, char* b, int elemsize)
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


void ShuffleMemory(void* base, int nelem, int elemsize)
{
  int   i;

  for( i = 0; i < nelem - 1; i++ )
    {
    SwapElems((char*) base + i * elemsize,
              (char*) base + (int)irand(i, nelem - 1) * elemsize,
              elemsize);
    }
}


/*-------------------------------------------------------------------*/

double dgauss() {
	return sqrt(-2 * log(drand())) * cos(6.283185307 * drand());
}

// Returns numSample different (SELECT * DISTINCT from N) random integers in the range [0,rangeEnd];
int* getRandomSampleInts(int rangeEnd, int numSample) {
    int* isPointSelected;
    int* randSample = calloc(numSample,sizeof(int)); // Init with zeros
    //assert(numSample < rangeEnd);
    int randId,i;

    // Full search, no random sampling
    if(numSample -1 >= rangeEnd) {
        for(i=0;i<=rangeEnd;i++){
            randSample[i] = i; // Not random
        }
    }
    else {
        isPointSelected = calloc(rangeEnd+1,sizeof(int)); // Init with zeros
        // Select numSample different random points.
        for(i=0;i<numSample;){
            randId = IRZ(rangeEnd+1);
            if(isPointSelected[randId] == 1) {continue;}
            else {
                isPointSelected[randId] = 1;
                randSample[i] = randId;
                i++;
            }
        }
        free(isPointSelected);
    }
    return randSample;
}


