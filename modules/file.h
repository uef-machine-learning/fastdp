#if ! defined(__FILE_H)
#define __FILE_H

#include <stdio.h>
#include "owntypes.h"

/*--------------------  Basic type definitions ----------------------*/

typedef enum { OUTPUT, INPUT, INTERNAL }  FILEMODE;

/*--------------------  Bit stream definitions ----------------------*/

typedef struct { FILE*  File;
                 int    BitsInQueue;
                 int    BitQueue;
               } BITSTREAM;

/* ------------------------ I/O general ---------------------------- */

int     ExistFile(char* File);
FILE*   FileOpen(char* File, int Mode, int AllowOverWrite);
int     ReadIntegerFromFile(FILE* f, int* value, int bytecount);
int     WriteIntegerToFile(FILE* f, int value, int bytecount);

           /* for little endian files (e.g. VQ 1.0 files) */
int     ReadLittleEndianIntegerFromFile(FILE* f, int* value, int bytecount);


/* ----------------------- Bit stream I/O -------------------------- */

void    InitializeBitStream(BITSTREAM* bs, FILE* f);
int     InputBit(BITSTREAM* bs);
void    OutputBit(BITSTREAM* bs, int bit);
int     InputValue(BITSTREAM* bs, int bits);
void    OutputValue(BITSTREAM* bs, int x, int bits);
void    FlushInput(BITSTREAM *bs);
void    FlushOutput(BITSTREAM* bs);

/* --------------------- File name routines ------------------------ */

void    PickFileName(char* Source, char* Destination);
void    CheckFileName(char* FileName, char* Extension);
YESNO   CheckExtention(char*FileName, char* Extention);

/* ---------------- Miscellaneous string routines ------------------ */

YESNO EqualFileNames(char* f1, char* f2);

/* ----------------------------------------------------------------- */

/* ------------------- For backward compatibility ------------------ */
/* ----------------------  (Not for Windows)  ---------------------- */
#if !defined(_WINDOWS)
FILE*   OpenFile(char* File, int Mode, int AllowOverWrite);
#endif  /* _WINDOWS */

#endif /* __FILE_H */
