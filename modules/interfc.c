/*--------------------------------------------------------------------*/
/* INTERFC.C   Eugene Ageenko                                         */
/*             Ismo Karkkainen                                        */
/*                                                                    */
/* This module provides User interface functions called from MODULES: */
/* it is information output, error messages, progress indicator, etc. */
/*--------------------------------------------------------------------*/

#define ProgName        "INTERFC"
#define VersionNumber   "Version 0.10"
#define LastUpdated     "6.4.2001"  /* jl */

/*----------------------------------------------------------------------*/
/*  Changes:                                                            */
/*    22.1.2000 Removed windowing stuff and some minor clean up. (iak)  */
/*     6.4.2001 stdout and stderr output initialization modified. (jl)  */
/*                                                                      */
/*----------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <interfc.h>
#include <string.h> // strlen

/*--------------------- DOS Specials -------------------------*/

#if defined(__MSDOS__)
#include <conio.h>
#endif

/* --------------------------------------------------------------------*/

static int   __ShowProgress    = 1;   /* Controlled by ProgressVisibility */
static int   __ShowProgressBar = 1;   /* controlled by FreezeProgrssBar   */
static int   __ProgressCreated = 0;

static FILE * __ModuleInfoFlow = NULL;   /* module output  */
static FILE * __ProgramInfoFlow = NULL;  /* program output */

/* ---------------------- INFORMATION OUTPUT --------------------------*/


int PrintMessage(char *format, ...)
{
    va_list ap;             /* points to each unnamed arg in turn */
    int retval=0;
    char msg[10000];

    if (!__ModuleInfoFlow) __ModuleInfoFlow = stdout;
    if (!__ProgramInfoFlow) __ProgramInfoFlow = stdout;

    va_start(ap, format);   /* make ap point to 1st unnamed arg */
    vsprintf(msg, format, ap);
    va_end(ap);             /* clean up when done */
    retval = strlen(msg);
    retval = fprintf (__ModuleInfoFlow, "%s", msg);
    fflush(__ModuleInfoFlow);
    return(retval);
}


/* --------------------------------------------------------------------*/


int PrintProgramMessage(char *format, ...)
{
    va_list ap;             /* points to each unnamed arg in turn */
    int retval=0;
    char msg[256];

    if (!__ModuleInfoFlow) __ModuleInfoFlow = stdout;
    if (!__ProgramInfoFlow) __ProgramInfoFlow = stdout;

    va_start(ap, format);   /* make ap point to 1st unnamed arg */
    vsprintf(msg, format, ap);
    va_end(ap);             /* clean up when done */
    retval = fprintf (__ProgramInfoFlow, "%s", msg);
    fflush(__ProgramInfoFlow);
    return(retval);
}


/* --------------------------------------------------------------------*/


int ErrorMessage(char *format, ...)
{
    va_list	ap;		/* points to each unnamed arg in turn */
    char msg[1024];

    va_start(ap, format);	/* make ap point to 1st unnamed arg */
    vsprintf(msg, format, ap);
    va_end(ap);		/* clean up when done */
    return fprintf(stderr,"%s", msg);
}


/* --------------------------------------------------------------------*/


int OpenInfoFlow(int id)
{
    if (!__ModuleInfoFlow) __ModuleInfoFlow = stdout;
    if (!__ProgramInfoFlow) __ProgramInfoFlow = stdout;

/* CONSOLE: 0 - all to stdout, 1 - PrintMessage to stderr */
    fflush(__ModuleInfoFlow);
    if (id) __ModuleInfoFlow = stderr;
    else    __ModuleInfoFlow = stdout;
    return(0);
}


/* --------------------------------------------------------------------*/


int CloseInfoFlow(void)
{
    if (__ModuleInfoFlow) fflush(__ModuleInfoFlow);
    if (__ProgramInfoFlow) fflush(__ProgramInfoFlow);
    return(0);
}


/* --------------------- PROGRAM TERMINATION --------------------------*/

int ExitRequestHonoredStatus = 0;

/* returns non-zero, if program calls ExitRequested, zero otherwise */
int ExitRequestHonored() {
    return ExitRequestHonoredStatus;
}


/* --------------------------------------------------------------------*/


/* tells UI if program calls ExitRequested, default is not honored */
void SetExitRequestHonored(int Honored) {
    ExitRequestHonoredStatus = Honored;
}


/* --------------------------------------------------------------------*/


/* checks if stopping of process has been requested, returns non-zero if yes */
int ExitRequested()
{
  #if defined(__MSDOS__)
     if (kbhit()) return(getch()==27);
  #else
     return(0);
  #endif
}


/* --------------------------------------------------------------------*/


/* exits the program or thread in a proper manner */
void ExitProcessing(int ReturnValue)
{
    exit(ReturnValue);
}



/* ---------------------- PROGRESS INDICATOR --------------------------*/


/* iak: I presume this will be called before progress bar is created */
int SetProgressVisibility(int value)
{
    __ShowProgress = value;
    return(value);
}


/* --------------------------------------------------------------------*/


int OpenProgressWindow(void)
{
  if (__ShowProgress && !__ProgressCreated)
    {
    __ProgressCreated = 1;
    fflush(stderr);
    }
  return(0);
}


/* --------------------------------------------------------------------*/


int CloseProgressWindow(void)
{
  if (__ShowProgress && __ProgressCreated)
    {
    UpdateProgressLabel("");
    fprintf(stderr,"\r");
    fflush(stderr);
    }
  __ProgressCreated = 0;
  return(0);
}


/* --------------------------------------------------------------------*/


int UpdateProgressBar(int state, int total)
{
  if (__ShowProgress && __ShowProgressBar)
    {
        if (total)
        {
            fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b%3i %% done  ",
		(int)((state*100L)/total));
            fflush(stderr);
        }
    }
  return(0);
}


/* --------------------------------------------------------------------*/


int UpdateFancyProgressBar(int state, int total)
{
  static int x=0;
  char c;

  if (__ShowProgress && __ShowProgressBar)
    {
        if (total)
        {
           if (state)
           {
              switch (x)
              {
                 case 0: c='-'; x++; break;
                 case 1: c='\\'; x++; break;
                 case 2: c='|'; x++; break;
                 default: c='/'; x=0; break;
              }
              fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b\b(%c)%3i %% done",
		  c,(int)((state*100L)/total));
              fflush(stderr);
           }
           else fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b  wait...     ");
        }
        else fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b stage complete");
    }
  return(0);
}


/* --------------------------------------------------------------------*/


int UpdateProgressLabel(char *format, ...)
{
        va_list ap;             /* points to each unnamed arg in turn */
        int retval=0;
        char msg[255];

  if (__ProgressCreated)
    {
        va_start(ap, format);   /* make ap point to 1st unnamed arg */
        vsprintf(msg, format, ap);
        va_end(ap);             /* clean up when done */
        fprintf(stderr,"\r                                                                      \r");
        retval = fprintf (stderr, "%s", msg);
        fprintf(stderr,"            ");
        /* fprintf(stderr,"\r"); */
        fflush(stderr);
    }
  return(retval);
}


/* --------------------------------------------------------------------*/


int FreezeProgressBar(int value)
{
  __ShowProgressBar = !value;
  return(value);
}


/*                                                                    */
/* ============================= END ================================ */
