/*-------------------------------------------------------------------*/
/* memctrl.c       Timo Kaukoranta                                   */
/*                                                                   */
/* Memory control; allocation and deallocation                       */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName        "memctrl"
#define VersionNumber   "Version 0.11"
#define LastUpdated     "4.10.00" /* TKa */

/* ----------------------------------------------------------------- */
/* Changelog:                                                        */
/*                                                                   */
/* 0.11 - bugfix: '#include "memctrl.h"' moved to right position.    */
/*      - new routine (macro): 'checkconsistency' checks linkage of  */
/*        memory chunks, etc. Outputs an error message if problems.  */
/*      - added internal property: Abstraction of system (library)   */
/*        memory handling routines by directives.                    */
/*      - added internal property: Abstraction of error message      */
/*        routine by a directive.                                    */
/*      - added few comments.                                        */
/*                                                                   */
/* ----------------------------------------------------------------- */

#include "memctrl.h"

/* Do we use this stuff anyway? */
#if defined(MEMORYCONTROLLER)

#include <assert.h>
/* #include <stdio.h> */
/* #include <stdlib.h> */

#include "interfc.h"

/* Abstraction of error message routine. */
#define memctrlErrorMessage ErrorMessage

/*========================= Memory handling =========================*/

/* to disallow different threads make changes at the same time */
#if (defined(WIN32) && !defined(BATCH)) || defined(SWI)

#include <windows.h>

static CRITICAL_SECTION MEMCS;
static int Init = 0;

#define mcEnterCritical() if (!Init) { Init = 1; InitializeCriticalSection(&MEMCS); } EnterCriticalSection(&MEMCS)
#define mcLeaveCritical() LeaveCriticalSection(&MEMCS)

#else

#define mcEnterCritical()
#define mcLeaveCritical()

#endif /* Windows stuff */

struct chunkheader
  {
  size_t              size;
#if defined(AMOUNTCONTROL)
  unsigned long       index;
  int                 sourceline;
  char*               sourcefile;
  struct chunkheader* prev;
  struct chunkheader* next;
  int                 guard;
#else
  size_t foo; /* Handles word alignment problems. Really? */
#endif
  };

#if defined(AMOUNTCONTROL)
static unsigned long _allocated          = 0L; /* # of bytes that are currently allocated. */
static unsigned long _maxallocated       = 0L; /* Max. # of bytes that has been allocated */
static struct chunkheader* head          = NULL;
static struct chunkheader* tail          = NULL;
#endif
static unsigned long _nallocated         = 0L; /* # of allocation calls */
static unsigned long _ndeallocated       = 0L; /* # of deallocation calls */


#if defined(EXIT_OUTOFMEMORY)
#define outofmemoryexit()    exit(EXIT_FAILURE)
#else
#define outofmemoryexit()    return(NULL)
#endif

/* Names of the 'real' memory allocation and deallocation routines. */
#define systemalloc   malloc
#define systemrealloc realloc
#define systemdealloc free

/* If real memory routines are macro names then undef them. Because here
   we want to use functions with these names.
*/
#ifdef malloc
#undef malloc
#endif
#ifdef realloc
#undef realloc
#endif
#ifdef free
#undef free
#endif


/* ----------------------------------------------------------------- */

#if defined(AMOUNTCONTROL)
static void printchunklist(void)
{
  struct chunkheader* ch = head;

  assert( (head == NULL && tail == NULL) || (head != NULL && tail != NULL) );

  memctrlErrorMessage("Allocated chunks:\n");
  while( ch != NULL )
    {
    memctrlErrorMessage(
      "call# %lu : %li bytes in %s at %i Guards: 0x%08x & 0x%08x\n",
      ch->index,
      (long)ch->size,
	    ch->sourcefile,
      ch->sourceline,
      ch->guard,
      *((int*)(((char*)ch) + sizeof(struct chunkheader) + ch->size)));
    ch = ch->next;
    }
}
#endif /* AMOUTCONTROL */

/* ----------------------------------------------------------------- */


int checkmemoryconsistency(char* file, int line)
{
#if defined(AMOUNTCONTROL)
  int corruptionoccured = 0;
#endif

  assert( 0 <= _nallocated );
  assert( 0 <= _ndeallocated && _ndeallocated <= _nallocated );

#if defined(AMOUNTCONTROL)
  assert( 0 <= _maxallocated );
  assert( 0 <= _allocated && _allocated <= _maxallocated );

  if( (head == NULL) ^ (tail == NULL) ) /* Assumes that '==' returns 0 or 1. */
    {
    corruptionoccured = 1;
    }
  else
    {
    struct chunkheader* Curr = tail;
    int corrupted = 0;

    while( Curr )
      {
      corrupted = 0;

      if( Curr->size <= 0 )
        {
        memctrlErrorMessage("Size corrupted!\n");
        corrupted = 1;
        }
      if( Curr->guard != 0xdeadbeef )
        {
        memctrlErrorMessage("Start guard corrupted!\n");
        corrupted = 1;
        }
      if( *((int*)(((char*)Curr) + sizeof(struct chunkheader) + Curr->size)) != 0xdeadbeef )
        {
        memctrlErrorMessage("End guard corrupted!\n");
        corrupted = 1;
        }

      if( corrupted )
        {
        memctrlErrorMessage(
          "chunkheader at 0x%08lx, index=%lu, size=%lu (?)\n"
          "start guard=0x%08x, end guard=0x%08x (?)\n"
          "prev chunk=%p, next chunk=%p (?)\n"
          "allocation   at line %i in file %s (?)\n"
          "deallocation at line %i in file %s\n",
          (unsigned long)Curr, Curr->index, (unsigned long)Curr->size,
          Curr->guard, *((int*)(((char*)Curr) + sizeof(struct chunkheader) + Curr->size)),
          Curr->prev, Curr->next,
          Curr->sourceline, Curr->sourcefile,
          line, file);
        }
      corruptionoccured |= corrupted;

      Curr = Curr->prev;
      }
    }

  if( corruptionoccured )
    {
    memctrlErrorMessage("Linkage corrupted: head=%p, tail=%p\n",head, tail);
    memctrlErrorMessage("Checked from line %i in file %s\n\n",line, file);
    return( -1 );
    }
  else
#endif /* AMOUTCONTROL */
    {
    return( 0 );
    }
}


/* ----------------------------------------------------------------- */


void outputmemoryreport(char* file, int line)
{
  memctrlErrorMessage("Memory report from %s at line %i\n", file, line);
#if defined(AMOUNTCONTROL)
  printchunklist();
  memctrlErrorMessage("Currently allocated:%lu\n",_allocated);
  memctrlErrorMessage("Max. allocated:     %lu\n",_maxallocated);
#endif
  memctrlErrorMessage("Allocation calls:   %lu\n",_nallocated);
  memctrlErrorMessage("Deallocation calls: %lu\n",_ndeallocated);
  memctrlErrorMessage("Difference in calls:%lu\n",_nallocated - _ndeallocated);
}


/* ----------------------------------------------------------------- */


int checkmemory(void)
{
  if( _nallocated != _ndeallocated
#if defined(AMOUNTCONTROL)
      || _allocated != 0L 
      || head != NULL 
      || tail != NULL
#endif
     )
    {
    memctrlErrorMessage("Memory handling stinks:");
    memoryreport();
    return( -1 );
    }
  else
    {
    return( 0 );
    }
}


/* ----------------------------------------------------------------- */


unsigned long maxusedmemory(void)
{
#if defined(AMOUNTCONTROL)
  return( _maxallocated );
#else
  return( 0 );
#endif
}


/* ----------------------------------------------------------------- */


unsigned longcurrentlyusedmemory(void)
{
#if defined(AMOUNTCONTROL)
  return( _allocated );
#else
  return( 0 );
#endif
}


/* ----------------------------------------------------------------- */


unsigned long nallocations(void)        /* Number of allocation calls. */
{
  return( _nallocated );
}


/* ----------------------------------------------------------------- */


unsigned long ndeallocations(void)      /* Number of deallocation calls. */
{
  return( _ndeallocated );
}


/*-------------------------------------------------------------------*/


void* allocatememory(size_t nbytes, char* file, int line)
{
#if defined(AMOUNTCONTROL)
  size_t* temp;
  struct chunkheader* ch;

  assert( nbytes >= 0 );
  assert( line > 0 );
  assert( file != NULL );
  assert( strlen(file) > 0 );

  mcEnterCritical();

  temp = (size_t*)systemalloc(nbytes + sizeof(struct chunkheader) + sizeof(int));
  if( temp == NULL )
    {
    memctrlErrorMessage("\nallocate: Out of memory in file `%s' in line %i. "
                        "Requiring %li\n", file, line, (long)nbytes);
    checkmemory();
    outofmemoryexit();
    }

  ch = (struct chunkheader*)temp;
  ch->size = nbytes;
  ch->index = _nallocated;
  ch->sourceline = line;
  ch->sourcefile = file;
  ch->guard = 0xdeadbeef;
  *((int*)(((char*)ch) + sizeof(struct chunkheader) + ch->size)) = 0xdeadbeef;

  /* Linkage */
  if( tail == NULL )
    {
    assert( head == NULL );
    head = tail = ch;
    ch->next = ch->prev = NULL;
    }
  else
    {
    tail->next = ch;
    ch->prev = tail;
    ch->next = NULL;
    tail = ch;
    }

  _allocated += (unsigned long)nbytes;
  if( _allocated > _maxallocated )
    {
    _maxallocated = _allocated;
    }
  _nallocated++;

  mcLeaveCritical();

  return( (void*)(ch + 1) );

#else /* ! defined(AMOUNTCONTROL) */

  size_t* temp = (size_t*)systemalloc(nbytes);

  if( temp == NULL )
    {
    memctrlErrorMessage("\nallocate: Out of memory in file `%s' in line %i. "
                        "Requiring %li\n", file, line, (long)nbytes);
    checkmemory();
    outofmemoryexit();
    }

  mcEnterCritical();
  _nallocated++;
  mcLeaveCritical();
  return( (void*)(temp) );

#endif
}


/*-------------------------------------------------------------------*/

#if defined(AMOUNTCONTROL)
#if defined(CHECKMEMORYDEALLOCATIONS)

static int iscorrectlylinkaged(struct chunkheader* ch,
                               void* block,
                               char* file,
                               int   line)
{
  struct chunkheader* Curr = tail;

  while( Curr )
    {
    if( Curr == ch )
      {
      break;
      }
    Curr = Curr->prev;
    }
  if( !Curr )
    {
    memctrlErrorMessage(
      "Tried to deallocate unallocated or freed memory at 0x%08lx\n"
      "chunkheader at 0x%08lx (?)\n"
      "Called from %s at line %i",
      (unsigned long)block, (unsigned long)ch, file, line);
    return 0;
    }
  else
    {
    return 1;
    }
}

#else /* ! defined(CHECKMEMORYDEALLOCATIONS) */

/* Because we are not interested in linkage checking,
   everything is alright (return non-zero). */
#define iscorrectlylinkaged(ch,block,file,line)  1

#endif /* CHECKMEMORYDEALLOCATIONS */
#endif /* AMOUNTCONTROL */

/*-------------------------------------------------------------------*/


void deallocatememory(void* block, char* file, int line)
{
#if defined(AMOUNTCONTROL)
  struct chunkheader* ch;
#endif

  if( block != NULL )
    {
    mcEnterCritical();

#if defined(AMOUNTCONTROL)
    ch = ((struct chunkheader*)block) - 1;

    /* Check that 'ch' is linkaged. */
    if( ! iscorrectlylinkaged(ch, block, file, line) )
      {
      return;
      }

    if( ch->guard != 0xdeadbeef )
      {
      memctrlErrorMessage(
        "\nProgram has written past the start of memory block, "
        "possibly corrupted info:\n"
        "block at 0x%08lx, chunkheader at 0x%08lx, size is %lu (?)\n"
        "allocation at line %i (?) in file %s (?)\n"
        "deallocation at line %i in file %s\n",
        (unsigned long) block, (unsigned long)ch, (unsigned long)ch->size,
        ch->sourceline, ch->sourcefile, line, file);
      }

    if( *((int*)(((char*)ch) + sizeof(struct chunkheader) + ch->size))
        != 0xdeadbeef )
      {
      memctrlErrorMessage(
        "\nProgram has written past the end of memory block, "
        "possibly corrupted info:\n"
        "block at 0x%08lx, chunkheader at 0x%08lx, size is %lu (?)\n"
        "allocation at line %i in file %s (?)\n"
        "deallocation at line %i in file %s\n",
        (unsigned long) block, (unsigned long)ch, (unsigned long)ch->size,
        ch->sourceline, ch->sourcefile, line, file);
      }

    /* Decrease the chance to hit invalid 'deadbeef' labels
       by clearing them. */
    ch->guard = 0x00000000;
    *((int*)(((char*)ch) + sizeof(struct chunkheader) + ch->size)) = 0x00000000;

    assert( ch->index < _nallocated );

    _allocated -= (unsigned long)ch->size;
    if( ch->next == NULL )
      {
      assert( tail == ch );
      tail = ch->prev;
      }
    else
      {
      ch->next->prev = ch->prev;
      }
    if( ch->prev == NULL )
      {
      assert( head == ch );
      head = ch->next;
      }
    else
      {
      ch->prev->next = ch->next;
      }
    systemdealloc(ch);

#else /* ! defined(AMOUNTCONTROL) */
    systemdealloc(block);
#endif

    _ndeallocated++;
    mcLeaveCritical();
    } /* if(block != NULL) */
}


/*-------------------------------------------------------------------*/


void* reallocatememory(void* block, size_t nbytes, char* file, int line)
{
#if defined(AMOUNTCONTROL)
  struct chunkheader* oldch;
  struct chunkheader* ch;
  struct chunkheader* prev = NULL;
  struct chunkheader* next = NULL;
#else
  size_t* temp;
#endif

  assert( nbytes >= 0 );
  assert( line > 0 );
  assert( file != NULL );
  assert( strlen(file) > 0 );

  if( nbytes == 0 )
    {
    if( block != NULL )
      {
      deallocatememory(block, file, line);
      }
    return( NULL );  
    }

#if defined(AMOUNTCONTROL)

  if( block == NULL )
    {
    return( allocatememory(nbytes, file, line) );
    }

  mcEnterCritical();
  oldch = ((struct chunkheader*)block) - 1;
  _allocated -= (unsigned long)oldch->size;
  prev = oldch->prev;
  next = oldch->next;
  ch = (struct chunkheader*)systemrealloc(oldch, nbytes + sizeof(struct chunkheader));
  if( ch == NULL )
    {
    memctrlErrorMessage("\nreallocate: Out of memory in file `%s' in line %i. "
                        "Requiring %li\n", file, line, (long)nbytes);
    checkmemory();
    outofmemoryexit();
    }

  ch->size = nbytes;
  ch->index = _nallocated;
  ch->sourceline = line;
  ch->sourcefile = file;

  if( ch != oldch ) /* Is the new chunk in the same place than previous? */
    {
    /* No, it is not. */
    assert( oldch != NULL );
    ch->prev = prev;
    ch->next = next;
    if( prev == NULL )
      {
      head = ch;
      }
    else
      {
      ch->prev->next = ch;
      }
    if( next == NULL )
      {
      tail = ch;
      }
    else
      {
      ch->next->prev = ch;
      }
    }

  _allocated += (unsigned long)nbytes;
  if( _allocated > _maxallocated )
    {
    _maxallocated = _allocated;
    }

  mcLeaveCritical();
  return( (void*)(ch + 1) );

#else /* ! defined(AMOUNTCONTROL) */

  temp = (size_t*)realloc(block, nbytes);

  if( temp == NULL )
    {
    memctrlErrorMessage("\nreallocate: Out of memory in file `%s' in line %i. "
                        "Requiring %li\n", file, line, (long)nbytes);
    checkmemory();
    outofmemoryexit();
    }

  mcEnterCritical();
  _nallocated++;
  mcLeaveCritical();

  return( (void*)(temp) );
#endif
}


/*-------------------------------------------------------------------*/

#endif  /* defined(MEMORYCONTROLLER) */

