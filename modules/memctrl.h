#ifndef __MEMCTRL_H
#define __MEMCTRL_H

/* ----------------------------------------------------------------- */

/* Uncomment the following #define if you want to use this
   memorycontroller module or define it with -D for compiler.
 */
/* #define MEMORYCONTROLLER */

/* Uncomment the following #define if you want to exit when out of
   memory occurs. Otherwise 'allocate' and 'reallocate' return NULL in
   the case of failure.
   Note: You need to define MEMORYCONTROLLER if you want to use this.
 */
/* #define EXIT_OUTOFMEMORY */

/* Uncomment the following line if you want to use "Amount Control".
   Note:You need to define MEMORYCONTROLLER if you want to use this.
 */
/* #define AMOUNTCONTROL */

/* Uncomment the following line if you want to perform thorough checks
   on memory.
   Note: You need to define MEMORYCONTROLLER and AMOUNTCONTROL if you
   want to use this.
 */
/* #define CHECKMEMORYDEALLOCATIONS */

/* ----------------------------------------------------------------- */

#if defined(MEMORYCONTROLLER)

#include <stdlib.h>

/* Do NOT use these five functions directly!
   Instead, use macros allocate, deallocate etc.
*/
void* allocatememory(size_t nbytes, char* file, int line);
void  deallocatememory(void* block, char* file, int line);
void* reallocatememory(void* block, size_t nbytes, char* file, int line);
int   checkmemoryconsistency(char* file, int line);  /* Returns -1 if problems (and outputs an error message), 0 otherwise. */
void  outputmemoryreport(char* file, int line);

#define allocate(nbytes)          allocatememory(nbytes,__FILE__,__LINE__)
#define deallocate(block)         deallocatememory(block,__FILE__,__LINE__)
#define reallocate(block,nbytes)  reallocatememory(block,nbytes,__FILE__,__LINE__)
#define checkconsistency()        checkmemoryconsistency(__FILE__,__LINE__)
#define memoryreport()            outputmemoryreport(__FILE__,__LINE__)

/* Use 'checkmemory' at end of main to check that all memory is
   released. Returns -1 if problems (and outputs an error message),
   0 otherwise. */
int   checkmemory(void);

/* Information retrieval. */
unsigned long maxusedmemory(void);       /* In bytes. */
unsigned long currentlyusedmemory(void); /* In bytes. */
unsigned long nallocations(void);        /* Number of allocation calls. */
unsigned long ndeallocations(void);      /* Number of deallocation calls. */

#else /* ! defined(MEMORYCONTROLLER) */

#include <stdlib.h>

#define allocate(nbytes)          malloc(nbytes)
#define deallocate(block)         free(block)
#define reallocate(block,nbytes)  realloc(block,nbytes)
#define checkconsistency()
#define memoryreport()

#define checkmemory()

#define maxusedmemory();
#define currentlyusedmemory();
#define nallocations();
#define ndeallocations();

#endif /* defined(MEMORYCONTROLLER) */

/* ----------------------------------------------------------------- */

#endif /* __MEMCTRL_H */

