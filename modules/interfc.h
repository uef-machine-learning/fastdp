#if ! defined(__INTERFC_H)
#define __INTERFC_H

/* ---------------------- INFORMATION OUTPUT --------------------------*/

int PrintMessage(char *format, ...); 
int PrintProgramMessage(char *format, ...); 
int ErrorMessage(char *format, ...);
int MessageWithQuestion(char *format, ...);

/* CONSOLE: 0 - all Print* to stdout, 1 - PrintMessage to stderr */
int OpenInfoFlow(int id);
int CloseInfoFlow(void);

/* --------------------- PROGRAM TERMINATION --------------------------*/

typedef enum { EVERYTHING_OK=0,
		FATAL_ERROR=-1,
		EXIT_REQUEST_HONOURED=-2,
		RECOVERABLE_ERROR=-3,
		RECOVERABLE_ERROR_UNCONTROLLED_EXIT=-4
		} ReservedExitProcessReturnValues;

extern void ExitProcessing(int ReturnValue);
extern int ExitRequestHonored();
extern void SetExitRequestHonored(int Honoured);
extern int ExitRequested();

/* ---------------------- PROGRESS INDICATOR --------------------------*/

int SetProgressVisibility(int true_false);  /* its quiet level */
int OpenProgressWindow(void);
int CloseProgressWindow(void);
int UpdateProgressBar(int processedsamples, int samplestotal);
int UpdateFancyProgressBar(int processedsamples, int samplestotal);
int UpdateProgressLabel(char *format, ...);
int FreezeProgress(int true_false);

/*--------------------------------------------------------------------*/

#endif /* __INTERFC_H */
