/*-------------------------------------------------------------------*/
/* PARAMETR.C      Pasi Fränti                                       */
/*                 Timo Kaukoranta                                   */
/* C-consultation given by:                                          */
/*                 Harri Hakonen                                     */
/* ParseParameters() etc:                                            */
/*                 Juha Kivijärvi                                    */
/* modified by:    Eugene Ageenko                                    */
/*                                                                   */
/* - Command line parameter parsing; options                         */
/*                                                                   */
/*-------------------------------------------------------------------*/

/*
#define ProgName        "PARAMETR"
#define VersionNumber   "Version 0.31b"
#define LastUpdated     "24.4.2001" iak
*/

/* ----------------------------------------------------------------- */

#if !(defined(_WIN32) && !defined(BATCH))
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file.h"
#else
#define NULL 0L
#endif

#include "parametr.h"

/* --------------------------- Data structure ---------------------------- */


enum    ParameterType    { BOOL_=0, INT_, ENUM_, STRING_ };
enum    Special          { NOT_DEFINED=-1 };

typedef struct
        {
        char*  Documentation;
        char   Key;
        int    Number;
        int    Type;
        int    Value;
        int    Min;
        int    Max;
        int    Default;
        int    SaveToFile;
        char*  Name[10];
        int    PrintingCondition;
        } ParameterList;


extern void PrintInfo();


/* -------------------- Boolean definition ---------------------------- */

#if ! defined(YES)
#if defined(NO)
#error ERROR: NO defined, YES undefined
#endif
#define YES 1
#define NO 0
#define UNDEF_YES_AND_NO
#endif

/* --------------------------- Local constant --------------------------- */


#define MAX_PARAMETER  (Pseudo_Last + 1)
#define BytesPerValue  2


/* ---------------- Parameter data structure definition ----------------- */


#define Fact( id, doc, a1, a2, a3, a4, a5, a6, a7, a8, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, pr ) \
            { doc, a1, a2, a3##_, a4, a5, a6, a7, a8,     \
            { n1, n2, n3, n4, n5, n6, n7, n8, n9, n10 },  \
              NO },

ParameterList Parameter[ MAX_PARAMETER ] =
     {
       { " ", ' ', 0, 0, 0, 0, 0, 0, 0, { NULL }, NO },
       #include FACTFILE
       { " ", ' ', 0, 0, 0, 0, 0, 0, 0, { NULL }, NO }
     };

#undef Fact


/* ---------------- Printing condition initialization  ----------------- */


#define Fact( id, doc, a1, a2, a3, a4, a5, a6, a7, a8, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, pr ) \
              Parameter[ _##id ].PrintingCondition = (pr);

void InitializePrintingConditions(void)
{
#include FACTFILE
}

#undef Fact


/* ========================  I N T E R F A C E  ========================= */


char* Document_(int x)
{
return( Parameter[ x ].Documentation );
}


char Key_(int x)
{
return( Parameter[ x ].Key );
}


int Number_(int x)
{
return( Parameter[ x ].Number );
}


int Type_(int x)
{
return( Parameter[ x ].Type );
}


int Value_(int x)
{
return( Parameter[ x ].Value );
}


void SetValue_(int x, int v)
{
Parameter[ x ].Value = v;
}


int Max_(int x)
{
return( Parameter[ x ].Max );
}


int Min_(int x)
{
return( Parameter[ x ].Min );
}


int Default_(int x)
{
return( Parameter[ x ].Default );
}


int SaveToFile_(int x)
{
return( Parameter[ x ].SaveToFile );
}


char* NameOfValue_(int x, int v)
{
return( Parameter[ x ].Name[ v ] );
}


int ShallWePrint_(int x)
{
return( Parameter[ x ].PrintingCondition );
}


/* ==============  G E N E R A L   S U B R O U T I N E S  =============== */


static int FindParameterID(char key, int number)
{
  int id;

  for(id=First_Parameter; id<=Last_Parameter; id++)
    {
    if( Key_(id) == key   &&   Number_(id) == number )
      {
      return( id );
      }
    }
  return(0);
}


/*-------------------------------------------------------------------*/


static int ValidKey(char key)
{
  int id;

  for(id=First_Parameter; id<=Last_Parameter; id++)
    {
    if( Key_(id) == key )
      {
      return( YES );
      }
    }
  return( NO );
}


/*-------------------------------------------------------------------*/


static int NumberOfSubParameters(char key)
{
  int id,result=0;

  for(id=First_Parameter; id<=Last_Parameter; id++)
    {
    if( (Key_(id)==key)  &&  (Number_(id)>0) )
      {
      result++;
      }
    }
  return( result );
}


/*-------------------------------------------------------------------*/


void SetDefaultValuesForParameters()
{
  int id;

  for(id=First_Parameter; id<=Last_Parameter; id++)
    {
    SetValue_( id, Default_(id) );
    }
}


#if !(defined(_WIN32) && !defined(BATCH))
/* Printing routines cannot be included in Windows applications */


/* ==============  P R I N T I N G   R O U T I N E S  =============== */


static void PrintValue(int id, int value)
{
  char s[80];

  switch(Type_(id))
    {
    case BOOL_: strcpy(s, value ? "YES" : "NO" );   break;
    case ENUM_: strcpy(s, NameOfValue_(id,value));  break;
    case INT_:  if( value == NOT_DEFINED )
                  {
                  strcpy(s, "NOT DEFINED");
                  }
                else
                  {
          sprintf(s, "%i", value);
                  }
                break;
    }
  printf("%s", s);
}


/*-------------------------------------------------------------------*/


static void PrintEnumList(int id)
{
  int i;
  int def = Default_(id);

  printf("\n");
  for(i=Min_(id); i<=Max_(id); i++)
    {
    printf("                            ");
    printf("%1i = ", i);
    PrintValue(id,i);
    if( def==i )
      {
      printf(" (default)");
      }
    printf("\n");
    }
}


/*-------------------------------------------------------------------*/


static void PrintIntDefault(int id)
{
  printf(" (");
  PrintValue(id, Min_(id));
  printf("..");
  PrintValue(id, Max_(id));
  printf(", default=");
  PrintValue(id, Default_(id));
  printf(")\n");
}


/*-------------------------------------------------------------------*/


static void PrintDefault(int id)
{
  printf(" (default=");
  PrintValue(id, Default_(id));
  printf(")\n");
}


/*-------------------------------------------------------------------*/


static void PrintDocumentation(char ch, int id, int number, int NumberOfSubs)
{
  if( NumberOfSubs==1 && FindParameterID(ch,0) )
    {
    printf("                      n: ");
    }
  if( NumberOfSubs>1 )
    {
    printf("                      n%1i: ", number);
    }
  printf("%s", Document_(id));
  switch( Type_(id) )
    {
    case BOOL_: PrintDefault(id);    break;
    case ENUM_: PrintEnumList(id);   break;
    case INT_:  PrintIntDefault(id); break;
    }

}


/*-------------------------------------------------------------------*/


static void PrintSyntax(char ch, int NumberOfSubs)
{
  int id;

  if( !ValidKey( ch ))
    {
    return;
    }
  switch( NumberOfSubs )
    {
    case 0:  printf("  %c%c               = ", OPTION_SYMBOL, ch); break;
    case 1:  printf("  %c%cn              = ", OPTION_SYMBOL, ch); break;
    case 2:  printf("  %c%cn1,n2          = ", OPTION_SYMBOL, ch); break;
    case 3:  printf("  %c%cn1,n2,n3       = ", OPTION_SYMBOL, ch); break;
    default: printf("  %c%c[n1,n2,..n%i]   = ", OPTION_SYMBOL, ch, NumberOfSubs); break;
    }
  id = FindParameterID(ch,0);
  if(id)
    {
    PrintDocumentation(ch,id,0,0);         /* Parameter document header */
    }
  else if( NumberOfSubs > 1 )
    {                                      /* Several subparameters     */
    printf("\n");                          /* yes => no document header */
    }
}


/*-------------------------------------------------------------------*/


void PrintOptions()
{
  char ch;
  int  i, id;
  int  NumberOfSubs;

  for(ch='A'; ch<='Z'; ch++)
    {
    NumberOfSubs = NumberOfSubParameters(ch);
    PrintSyntax( ch, NumberOfSubs );
    for(i=1; i<=10; i++)
      {
      id = FindParameterID( ch, i );  /* Non-parametric option */
      if( id )
        {
        PrintDocumentation( ch, id, i, NumberOfSubs );
        }
      }
    }
}


/*-------------------------------------------------------------------*/


void PrintSelectedOptions()
{
  int id;

  for(id=First_Parameter; id<=Last_Parameter; id++)
    {
    if(ShallWePrint_(id))
      {
      printf("%-25s = ", Document_(id));
      PrintValue( id, Value_(id) );
      printf("\n");
      }
    }
}



/* ================  F I L E   O P E R A T I O N S  ================= */


void ReadOptions(FILE* f)
{
  int id,value;

  for(id=First_Parameter; id<=Last_Parameter; id++)
    {
    if( SaveToFile_(id) )
      {
      ReadIntegerFromFile(f,&value,BytesPerValue);
      SetValue_(id,value);
      }
    }
}


/*-------------------------------------------------------------------*/


void WriteOptions(FILE* f)
{
  int id;

  for(id=First_Parameter; id<=Last_Parameter; id++)
    {
    if( SaveToFile_(id) )
      {
      WriteIntegerToFile(f, Value_(id), BytesPerValue);
      }
    }
}

/* ==============  P A R A M E T E R   P A R S I N G  =============== */


static void ParseSubParameters(char* s,
                               int   limit,
                               int*  parsed,
                               int   values[])
{
  *parsed = 0;
  while( s && *s && *parsed < limit-1 )
    {
    switch( *s )
      {
      case ',': values[++(*parsed)] = NOT_DEFINED;
                s++;
                break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': values[++(*parsed)] = abs(atoi(s));
                if( (s = strchr(s, ',')) != NULL )
                  {
                  s++;
                  }
                break;
      default:  s++;
                break;
      }
    }
}


/*-------------------------------------------------------------------*/


static void SetParameterValue(char key, int number, int id, int value)
{
/*printf("SetValue: id=%i, value=%i, type=%i.\n", id, value, Type_(id));*/
  switch( Type_( id ))
    {
    case BOOL_:
      {
      SetValue_(id, YES);
      break;
      }
    case ENUM_:
    case INT_:
      {
      if( (value >= Min_(id))  &&  (value <= Max_(id)) )
         {
         SetValue_(id, value);
         }
      else
         {
         printf("ERROR: Parameter %c(%i) = %i is out of range [%i,%i].\n", key, number, value, Min_(id), Max_(id));
         exit(-1);
         }
      break;
      }
    } /* end switch */
}


/*-------------------------------------------------------------------*/


void ParseOption(char* s)
{
  int SubPar[20] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
  int   parsed = 0;
  int   found  = NO;
  int   value;
  int   i, id;
  char  key;

  key = toupper( s[1] );
  ParseSubParameters(s + 2, 20, &parsed, SubPar);

  /* Non-parametric option */
  id = FindParameterID( key, 0 );
  if( id )
    {
    SetParameterValue( key, 0, id, YES );
    found = YES;
    }

  /* Parametric option */
  for(i=1; i<=parsed; i++)
    {
    value = SubPar[i];
    id = FindParameterID( key, i );
    if( id )
      {
      SetParameterValue( key, i, id, value );
      found = YES;
      }
    }
if( !found )
  {
  printf("ERROR: Unknown option %c%c found.\n", OPTION_SYMBOL, key);
  exit(-1);
  }
}


/*-------------------------------------------------------------------*/

static void CheckForDuplicateFileNames(int FileNameCount,
                                       ParameterInfo PInfo[])

{
  int i,j;

  for (i=0; i<FileNameCount; i++)
      for (j=0; j<i; j++)
          if ( !( (PInfo[i].FileType==INFILE && PInfo[j].FileType==INFILE) ||
                   PInfo[i].FileType==DONTCARE || PInfo[j].FileType==DONTCARE))
              if (*(PInfo[i].FileName) && *(PInfo[j].FileName)
	          && EqualFileNames(PInfo[i].FileName,PInfo[j].FileName))
                  {
                  printf("ERROR: Input and output filenames should be "
		      "different. (%s)\n", PInfo[i].FileName);
                  exit(-1);
                  }
}



/*-------------------------------------------------------------------*/


int ParseOptions(int ArgC, char* ArgV[])
{
  int i,FileNamesGiven=0;

  for(i=1; i<ArgC; i++)
  {
     if( ArgV[i][0] == OPTION_SYMBOL )
          ParseOption(ArgV[i]);
     else
          FileNamesGiven++;
  }
  return (FileNamesGiven);
}


/*-------------------------------------------------------------------*/


void ParseNames     (int          ArgC,
                    char*         ArgV[],
                    int           FileNameCount,
                    int           FileNamesGiven,
                    ParameterInfo PInfo[])
{
  int i;
  int NumberOfObligatoryFileNames = 0;
  int Highest;
  int CurrentParameter = 0;

  if (FileNamesGiven > FileNameCount)
      {
      printf("ERROR: Too many parameters\n\n");
      PrintInfo();
      exit(-1);
      }

  for (i=0; i<FileNameCount; i++)
      if (PInfo[i].Priority == 0)
          NumberOfObligatoryFileNames++;

  if (FileNamesGiven < NumberOfObligatoryFileNames)
      {
      if (FileNamesGiven > 0)
          printf("ERROR: Too few parameters\n\n");
      PrintInfo();
      exit(-1);
      }

  Highest = FileNamesGiven - NumberOfObligatoryFileNames;

  for (i=1; i<ArgC; i++)
      if (ArgV[i][0] != OPTION_SYMBOL)
          {
          while (PInfo[CurrentParameter].Priority > Highest)
              {
              strcpy(PInfo[CurrentParameter].FileName, "\0");
              CurrentParameter++;
              }
          strcpy(PInfo[CurrentParameter].FileName,ArgV[i]);
          if (strlen(PInfo[CurrentParameter].Extension) > 0)
              CheckFileName(PInfo[CurrentParameter].FileName,
                            PInfo[CurrentParameter].Extension);
          CurrentParameter++;
          }

  while (CurrentParameter < FileNameCount)
      {
      strcpy(PInfo[CurrentParameter].FileName, "\0");
      CurrentParameter++;
      }

  InitializePrintingConditions();
  CheckForDuplicateFileNames(FileNameCount, PInfo);

}


/*-------------------------------------------------------------------*/


int ParseParameters(int           ArgC,
                    char*         ArgV[],
                    int           FileNameCount,
                    ParameterInfo PInfo[])
{
  int FileNamesGiven = 0;

  SetDefaultValuesForParameters();
  FileNamesGiven=ParseOptions(ArgC,ArgV);
  ParseNames(ArgC,ArgV,FileNameCount,FileNamesGiven,PInfo);
  return (FileNamesGiven);
}

#endif /* !_WIN32 || BATCH */

/*-------------- Boolean undefinition (if necessary) ----------------*/

#if defined(UNDEF_YES_AND_NO)
#undef YES
#undef NO
#undef UNDEF_YES_AND_NO
#endif
