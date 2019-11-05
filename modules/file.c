/*-------------------------------------------------------------------*/
/* FILE.C       Juha Kivijarvi                                       */
/*              Pasi Franti                                          */
/*              Timo Kaukoranta                                      */
/*                                                                   */
/* General I/O routines                                              */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName        "FILE"
#define VersionNumber   "Version 0.16"
#define LastUpdated     "15.11.2001"

/* ----------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "file.h"
#include "owntypes.h"
#include "interfc.h"


/*========================  I/O  GENERAL  ============================*/


int ExistFile(char* File)
{
  FILE* f;

  if( (f = fopen(File, "rb")) != NULL )
    {
    fclose(f);
    return( YES );
    }
  else
    {
    return( NO );
    }
}


/*-------------------------------------------------------------------*/


FILE* FileOpen(char* File, int Mode, int AllowOverWrite)
{
  FILE* f;

  switch( Mode )
    {
    case INPUT:
      {
      f = fopen(File, "rb");
      if( ! f )
        {
        ErrorMessage("ERROR: Cannot find the file: %s", File);
        return(NULL); /* exit( -1 ); */
        }
      break;
      }
    case OUTPUT:
      {
      if( ExistFile(File) )
        {
        if( AllowOverWrite == YES )
          {
          remove(File);
          }
        else
          {
          ErrorMessage("ERROR: File already exists: %s", File);
          return(NULL); /* exit( -1 ); */
          }
        }
      f = fopen(File,"wb+");
      if( ! f )
        {
        ErrorMessage("ERROR: Couldn't open the file: %s", File);
        return(NULL); /* exit( -1 ); */
        }
      break;
      }
    default:
      {
      ErrorMessage("ERROR: Invalid file mode:%d", Mode);
      return(NULL); /* exit( -1 ); */
      }
    }
  return( f );
}


/*-------------------------------------------------------------------*/

#ifndef _WINDOWS

FILE* OpenFile(char* File, int Mode, int AllowOverWrite)
{
FILE* f;

  f = FileOpen(File, Mode, AllowOverWrite);
  if ( f == NULL )
    ExitProcessing(-1);
  return f;
}  
  
#endif

/*-------------------------------------------------------------------*/


int ReadIntegerFromFile(FILE* f, int* value, int bytecount)
{                          /* byte order is big endian */
  int  i;
  int  c;
  int  res = 0;

  for( i=0; i<bytecount; i++ )
    {
    res <<= 8;
    c = getc(f);
    if (c == EOF)
      {
      ErrorMessage("ERROR: unable to read from file.");
      break;
      }
    res |= c;
    }

  *value = res;
  return i;
}

/*-------------------------------------------------------------------*/


int ReadLittleEndianIntegerFromFile(FILE* f, int* value, int bytecount)
{                          /* byte order is little endian */
  int  i;
  int  c;
  int  res = 0;

  for( i=0; i<bytecount; i++ )
    {
    c = getc(f);
    if (c == EOF)
      {
      ErrorMessage("ERROR: unable to read from file.");
      break;
      }
    c <<= (8*i);
    res |= c;
    }

  *value = res;
  return i;
}


/*-------------------------------------------------------------------*/


int WriteIntegerToFile(FILE* f, int value, int bytecount)
{                       /* byte order is big endian */
  int  i;
  int  c;

  if (bytecount < sizeof(int) && (value >> (8*bytecount)) > 0)
    {
    ErrorMessage("ERROR: Vector element value is too big (%d)."
                 " Increase bytes/elem ratio.", value);
	return(0);
    }

  for( i=0; i<bytecount; i++ )
    {
    c = ( value >> (8*(bytecount-1-i))) % 256;
    if (putc(c,f)==EOF)
       { ErrorMessage ("ERROR: unable to write to file."); break; }
    }

  return (i);
}


/*========================  I/O  SPECIAL  ============================*/


void InitializeBitStream(BITSTREAM* bs, FILE* f)
/* f should already be opened */
{
  bs->File        = f;
  bs->BitsInQueue = 0;
  bs->BitQueue    = 0;
}


/*-------------------------------------------------------------------*/


int InputBit(BITSTREAM* bs)
{
  int bit;

  if( bs->BitsInQueue == 0 )
    {
    bs->BitQueue = getc(bs->File);
    if( bs->BitQueue == EOF )
      {
      ErrorMessage("Unexpected EOF in input file.");
      return( EOF );
      }
    bs->BitsInQueue = 8;
    }
  bit = (bs->BitQueue & (0x80)) >> 7;
  bs->BitQueue <<= 1;
  (bs->BitsInQueue)--;
  return( bit );
}


/*-------------------------------------------------------------------*/


void OutputBit(BITSTREAM* bs, int bit)
{
  assert( bit == 0 || bit == 1 );

  bs->BitQueue <<= 1;
  bs->BitQueue |= bit;
  if( ++(bs->BitsInQueue) == 8 )
    {
    putc(bs->BitQueue, bs->File);
    bs->BitsInQueue = 0;
    bs->BitQueue = 0;
    }
}


/*-------------------------------------------------------------------*/


void FlushInput(BITSTREAM* bs)
{
  bs->BitsInQueue = 0;
}


/*-------------------------------------------------------------------*/


void FlushOutput(BITSTREAM* bs)
{
  if(bs->BitsInQueue==0)
     {
     return;
     }
  while((bs->BitsInQueue)++ < 8)
     {
     bs->BitQueue <<= 1;
     }
  putc(bs->BitQueue, bs->File);
  bs->BitsInQueue = 0;
  bs->BitQueue = 0;
}


/*-------------------------------------------------------------------*/


int InputValue(BITSTREAM* bs, int bits)
{
  int i;
  int x    = 0;
  int mask = 0x01;

  for(i=1; i<=bits; i++)
     {
     if(InputBit(bs))  x |= mask;
     mask <<= 1;
     }
  return( x );
}


/*-------------------------------------------------------------------*/


void OutputValue(BITSTREAM* bs, int x, int bits)
{
  int i;

  for(i=1; i<=bits; i++)
    {
    OutputBit( bs, x & 0x01 );
    x >>= 1;
    }
}


/*=====================  File name routines  =========================*/


void PickFileName(char* Source, char* Destination)
{
  char* ch;

  strcpy(Destination, Source);
  ch = strchr(Destination, '.');
  if( ch )
    {
    (*ch) = 0x00;
    }
}


/*-------------------------------------------------------------------*/


void CheckFileName(char* FileName, char* Extension)
/* Checks whether the file has extension. If no extension found,
   then the file name is catenated by 'Extension'. */
{
  int i = strlen(FileName);

  while( i >= 0 && FileName[i] != '.' && FileName[i] != '/' &&
                                         FileName[i] != '\\' )
    {
    i--;
    }
  if( i < 0 || FileName[i] == '/' || FileName[i] == '\\' )
    {
    strcat(FileName, ".");
    strcat(FileName, Extension);
    }
}


/* ---------------------------------------------------------- */
YESNO CheckExtention(char* FileName, char* Extention)
{
  char* ch;

  ch = strchr(FileName, '.');
  ch ++;
  if(ch)
  {
    if (strcmp(ch, Extention)==0) return YES;
  }

  return NO;
}

/*================  Miscellaneous STRING-routines  ===================*/


YESNO EqualFileNames(char* f1, char *f2)
{
  int i=0;

  while (f1[i] && f2[i])
    {
    if (toupper(f1[i])!=toupper(f2[i]))
        return NO;
    i++;
    }
  return (YESNO) (f1[i]==f2[i]);
}


