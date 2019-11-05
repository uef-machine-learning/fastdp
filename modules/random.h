#if ! defined(__RANDOM_H)
#define __RANDOM_H

/* random.h */

#define BRAND CombTaus /* Basic random numbers in [1..2**31-1] */

/* Random integers in interval [a,b) (b-a < 100000) */
#define IRI(ia,ib) ((ia) + CombTaus() % ((ib) - (ia)))

/* Random integers in interval [0,z) (z < 100000) */
#define IRZ(z) (CombTaus() % (z))

extern long  CombTaus(void);
extern long  irand(long,long);
extern double  drand(void);
extern float  frand(void);
extern void  setrangen2(unsigned long,unsigned long);
extern void  getseeds2(unsigned long*,unsigned long*);

/* Initializes random number generator from clock if seed == 0. */
extern void  initrandom(unsigned long seed);

void ShuffleMemory(void* base, int nelem, int elemsize);

/*------- Random numbers with Gaussian distribution -------*/

/***
	For 10^6 numbers generated:
	Mean == 0.0 +- 10^-4
	Standard Deviation == 1.0 +- 10^-4
	Values appear to be within range [-5.5, 5.5]
***/
extern double dgauss();

int* getRandomSampleInts(int rangeEnd, int numSample);

#endif /* __RANDOM_H */
