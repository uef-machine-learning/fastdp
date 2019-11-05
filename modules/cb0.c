/*-------------------------------------------------------------------*/
/* CB.C            Juha Kivijärvi                                    */
/*                 Pasi Fränti                                       */
/*                 Timo Kaukoranta                                   */
/*                                                                   */
/* Modifications   Marko Tuononen, Ismo Kärkkäinen, Ville Hautamäki  */
/*                 Mikko Malinen, Billy Braithwaite                  */
/*                                                                   */
/* Data structures and interface for:                                */
/*   - Codebook (CB)                                                 */
/*   - Training set (TS)                                             */
/*   - kNN Graph                                                     */
/*                                                                   */
/*-------------------------------------------------------------------*/

#define ProgName           "CB"
#define VersionNumber      "Version 0.68"
#define LastUpdated        "13.2.2013" /* BB */
#define CodebookID         "VQ CODEBOOK"
#define TrainingSetID      "VQ TRAINING SET"
#define PartitioningID     "VQ PARTITIONING"
#define FileFormatVersion  "2.0"

/* ----------------------------------------------------------------- */
/*      Changelog:                                                   */
/*                                                                   */
/* BB   0.68  Return to old graph file format                        */
/* BB   0.67  Fixed AddString                                        */
/* MM   0.66  New graph file format added (graph version 2)          */
/* PF   0.65  added ChangePartitionFast                              */
/* MT   0.64  Fixed a bug in LocalRepartitioningGeneral              */
/* MT   0.63  CreateNewCodebook modified to copy GenerationMethod    */
/* MT   0.62  Fixed error in TrainingSetID (TRAININGOB -> TRAINING)  */
/* VH   0.61  Added kNN Graph routines (API and file IO)             */
/* JK   0.60  Small fix in AverageErrorForSolutionSC in order to     */
/*            get rid of warnings.                                   */
/* TKa  0.59  In PutAllInOwnPartition the call of                    */
/*            IncreaseNumberOfPartitions changed to call of          */
/*            ChangeNumberOfPartitions.                              */
/* TKa  0.58  added SetAllocatedCodebookSize and                     */
/*            SetNumberOfAllocatedPartitions, which change the       */
/*            amount of allocated memory for these structures.       */
/*            Increase and Decrease routines change the logical size */
/*            of the structures.                                     */
/*            modified CreateNewPartitioning so that it initializes  */
/*            GenerationMethod with empty string instead of nothing. */
/*      0.57  added DecreaseNumberOfPartitions, minor changes. (IK)  */
/*      0.56  added functions to change codebook and partition size  */
/*            also added GenerateOptimalCOdebookGeneral              */
/*            adding of "++" in AddCodebook checks if ends in "++"   */
/*            IncreaseNumberOfPartitions has new size checked        */
/*            GenerateOptimalCodebookGeneral has been added          */
/*            AverageErrorForPartitioning uses it now (IK)           */
/*      0.55  fixed WriteCodebook when FileOpen returns NULL (JK)    */
/*            added string length check to AddCodebook               */
/*            removed copying of GenerationMethod in CreateNewP      */
/*      0.54  none                                                   */
/*      0.53  added AddCodebook (PF)                                 */
/*      0.52  added UnrollTrainingSet (JK)                           */
/*                                                                   */
/* ----------------------------------------------------------------- */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  /* Needed for isspace() */
#include <limits.h>
#if !defined(MSVC)
#include <values.h>
#endif

#include "memctrl.h"
#include "file.h"
#include "interfc.h"
#include "cb.h"
#include "owntypes.h"

#define  CURRENTINTSIZE 4

#define  strlength(a) ( (a) == NULL ? 0 : strlen(a) )

#if ! defined(log2)
    #define log2(x) ( log(x) / log(2) )
#endif

/*================  Miscellaneous string routines  ===================*/


static void RemoveLineBreak(char* str)
{
  int slen;

  if( str == NULL )
    {
    ErrorMessage("ERROR: empty header (NULL)\n");
    ExitProcessing( -1 );
    }
  else
    {
    slen = strlen(str);

    if( slen == 0 )
      {
      ErrorMessage("ERROR: empty header (0x00)\n");
      ExitProcessing( -1 );
      }
    else      
      {
      if( str[slen-1] == '\n' )
        {
        str[slen-1] = 0x00;
        }
      }
    }
}


/*-------------------------------------------------------------------*/


void SetVerString(char* str, CBFILETYPE filetype)
{
  switch (filetype)
    {
    case TSFILE:
      {
      sprintf(str, "%s %s", TrainingSetID, FileFormatVersion);
      break;
      }
    case CBFILE:
      {
      sprintf(str, "%s %s", CodebookID, FileFormatVersion);
      break;
      }
    case PAFILE:
      {
      sprintf(str, "%s %s", PartitioningID, FileFormatVersion);
      break;
      }
    default:
      {
      ErrorMessage("ERROR: Unknown file type (SetVersionString)\n");
      ExitProcessing(-1);
      }
    }

}


/*-------------------------------------------------------------------*/


void ClearString(char* str)
{
  strcpy( str, "");
}


/*-------------------------------------------------------------------*/


void AddString(char* str, char* NewName, int maxlength)
{
  char* s;
  s = allocate(2*maxlength);

  strcpy(s, str);
  strcat(s, NewName);

  if ( (int) strlen( s ) > maxlength )
    {
    ErrorMessage( "ERROR: string too long:\n%*s\n", maxlength, s );
    ExitProcessing(-1);
    }

  strncpy(str, s, maxlength);
  deallocate(s);
}



/*====================  Vector level routines  ======================*/


VECTORTYPE  CreateEmptyVector(int Vsize)
{
  VECTORTYPE  v;

  /* Allocate memory for the vector */
  v = (VECTORTYPE) allocate( Vsize * sizeof(VECTORELEMENT));
  if( v == NULL )
    {
    ErrorMessage("ERROR: Not enough memory for new vector element.\n");
    ExitProcessing(-1);
    }
  return( v );
}


/*-------------------------------------------------------------------*/


void FreeVector(VECTORTYPE v)
{
  deallocate( v );
}


/*-------------------------------------------------------------------*/


void CopyVector(VECTORTYPE source, VECTORTYPE dest, int Vsize)
{
  int i;

  if( source == NULL || dest == NULL )
    {
    ErrorMessage("ERROR: CopyVector source=%p dest=%p\n", source, dest);
    ExitProcessing( -1 );
    }
  for( i=0; i<Vsize; i++ )
    {
    dest[i] = source[i];
    }
}


/*-------------------------------------------------------------------*/


void PrintVector(VECTORTYPE v, int Xdim, int Ydim)
{
  int  x=0, y=0, i=0;

  for( y=0; y<Xdim; y++ )
    {
    for( x=0; x<Ydim; x++ )
      {
      PrintMessage("%3i ", v[i++]);
      }
    }
  PrintMessage("\n");
}


/*-------------------------------------------------------------------*/


int CompareVectors( VECTORTYPE v1, VECTORTYPE v2, int Vsize)
{
  int i;

  for( i=0; i<Vsize; i++ )
    {
    if(v1[i] > v2[i]) return(  1 );
    if(v1[i] < v2[i]) return( -1 );
    }
  return( 0 );
}


/*-------------------------------------------------------------------*/


static llong VectorDistanceEuclideanSq( VECTORTYPE v1,
                                        VECTORTYPE v2,
                                        int        Vsize,
                                        llong      maxdist)
/* Precondition: Vsize >= 1 */
/* if VectorDistance > maxdist, returns at least maxdist */
{
  int   i = 0;
  llong diff;
  llong dist = 0;

  do
    {
    diff = (llong)v1[i] - (llong)v2[i];
    dist += diff * diff;
    } while( dist < maxdist && ++i < Vsize );

  return( dist );
}


/*-------------------------------------------------------------------*/


static llong VectorDistanceEntropyDist( VECTORTYPE v1,
                                        VECTORTYPE v2,
                                        int        Vsize,
                                        llong      maxdist)
/* Precondition: Vsize >= 1                              */
/* if VectorDistance > maxdist, returns at least maxdist */
/* v2 should only have values 0 and 1000000              */
{
  int    i = 0;
  double dist = 0.0;
  double vvalue;
  int    EPSILON = 1000;

  do
    {
    if ( v2[i] != 0 && v2[i] != 1000000 )
      {
      ErrorMessage("ERROR: input vector elements should be either 0 or 1000000 (was %d).\n", v2[i]);
      ExitProcessing(-1);
      }

    if ( v2[i] == 0 )
      {
      vvalue = ( v1[i] > 1000000 - EPSILON ) ? 1 - EPSILON / 1000000.0 :
                                      (double) v1[i] / 1000000.0;
      dist -= log2( 1.0 - vvalue );
      }
    else
      {
      vvalue = ( v1[i] < EPSILON ) ? EPSILON / 1000000.0 :
                        (double) v1[i] / 1000000.0;
      dist -= log2( vvalue );
      }
    } while( dist < (double)maxdist/1000000.0 && ++i < Vsize );

  return( (llong) (dist * 1000000 + 0.5) );
}


/*-------------------------------------------------------------------*/


static llong VectorDistanceAsymmetric( VECTORTYPE   v1,
                                       VECTORTYPE   v2,
                                       int          Vsize,
                                       llong        maxdist,
                                       DISTANCETYPE disttype)
/* For binary vectors only!!! */
/* Precondition: Vsize >= 1   */
{
  int    i = 0;
  double dist;
  llong  posmatch = 0;
  llong  mismatch = 0;

  for(i=0; i<Vsize; i++)
     {
     if(v1[i]!=v2[i]) mismatch++;
     else if(v1[i])   posmatch++;
     }

  switch(disttype)
     {
     case JACCARD: dist = (double) mismatch / (mismatch+posmatch);   break;
     case DICE:    dist = (double) mismatch / (mismatch+2*posmatch); break;
     default:      dist = 0; break;
     }
  return( (llong) (1000000*dist + 0.5) );
  /* Result is double between 0..1 multiplied by 1000000 (because of llong) */
}


/*-------------------------------------------------------------------*/


llong VectorDistance( VECTORTYPE   v1,
                      VECTORTYPE   v2,
                      int          Vsize,
                      llong        maxdist,
                      DISTANCETYPE disttype)
/* Precondition: Vsize >= 1 */
/* if VectorDistance > maxdist, returns at least maxdist */
{

  switch( disttype )
    {
    case EUCLIDEANSQ:
      {
      return VectorDistanceEuclideanSq(v1,v2,Vsize,maxdist);
      }
    case ENTROPYDIST:
      {
      return VectorDistanceEntropyDist(v1,v2,Vsize,maxdist);
      }
    case JACCARD:
    case DICE:
      {
      return VectorDistanceAsymmetric(v1,v2,Vsize,maxdist,disttype);
      }
    default:
      {
      ErrorMessage("ERROR: unsupported vector distance metric %d\n", disttype);
      ExitProcessing(-1);
      }
    }

  return(0);
}


/*=================  Codebook memory control  =======================*/


static void AllocateMemoryForBook(CODEBOOK* CB)
{
  int i;

  CB->Book = (BOOKTYPE) allocate(BookSize(CB) * sizeof(BOOKNODE));

  if( CB->Book == NULL )
    {
    ErrorMessage("ERROR: allocate CB->Book\n");
    ExitProcessing(-1);
    }
  for(i=0; i<BookSize(CB); i++)
    {
    CB->Book[i].vector = CreateEmptyVector(VectorSize(CB));
    CB->Book[i].vmean = 0;
    CB->Book[i].name = NULL;
    }

  CB->AllocatedSize = BookSize(CB);
}


/*-------------------------------------------------------------------*/


void CreateNewCodebook(CODEBOOK* CB,
                       int booksize,
                       TRAININGSET* TS)
{
  SetVersionString(CB, CBFILE);

  CB->BlockSizeX         = TS->BlockSizeX;
  CB->BlockSizeY         = TS->BlockSizeY;
  CB->BytesPerElement    = TS->BytesPerElement;
  CB->MinValue           = TS->MinValue;
  CB->MaxValue           = TS->MaxValue;
  CB->Preprocessing      = TS->Preprocessing;
  CB->TotalFreq          = TS->TotalFreq;

  /* name of minmax file is stored in GenerationMethod-field 
     and should be copied (this is because of CB2TXT program) */
  if (CB->Preprocessing >= 2) 
    {       
    strcpy( CB->GenerationMethod, TS->GenerationMethod );
    }
  else
    {
    strcpy( CB->GenerationMethod, "" );
    }

  BookSize(CB) = booksize;

  AllocateMemoryForBook(CB);

}


/*-------------------------------------------------------------------*/


void CreateNewTrainingSet(TRAININGSET* TS,
                          int          booksize,
                          int          BlockSizeX,
                          int          BlockSizeY,
                          int          BytesPerElement,
                          int          MinValue,
                          int          MaxValue,
                          int          Preprocessing,
                          char*        GenerationMethod)

{
  SetVersionString(TS, TSFILE);

  TS->BlockSizeX         = BlockSizeX;
  TS->BlockSizeY         = BlockSizeY;
  TS->BytesPerElement    = BytesPerElement;
  TS->MinValue           = MinValue;
  TS->MaxValue           = MaxValue;
  TS->Preprocessing      = Preprocessing;
  TS->TotalFreq          = 0;

  strcpy( TS->GenerationMethod, GenerationMethod );

  BookSize(TS) = booksize;

  AllocateMemoryForBook(TS);

}


/*-------------------------------------------------------------------*/


void FreeCodebook(CODEBOOK* CB)
{
  int i;

  for(i=0; i<CB->AllocatedSize; i++)
    {
    FreeNode(Node(CB,i));
    }
  deallocate(CB->Book);
}


/*-------------------------------------------------------------------*/


void SetAllocatedCodebookSize(CODEBOOK* CB, int newsize)
{
  CODEBOOK tempCB;
  BOOKTYPE tempBook;

  if (newsize <= BookSize(CB))
    {
    ErrorMessage("CB.C::SetAllocatedCodebookSize: "
      "New allocated size (%i) <= logical booksize (%i).\n",
      newsize, BookSize(CB));
    ExitProcessing(FATAL_ERROR);
    }

  if (CB->AllocatedSize < newsize)
    {
    CreateNewCodebook(&tempCB, newsize, CB);
    CopyCodebook(CB, &tempCB);
    tempBook    = CB->Book; /* swap books */
    CB->Book    = tempCB.Book;
    tempCB.Book = tempBook;
    /* swap allocated sizes */
    tempCB.AllocatedSize = CB->AllocatedSize;
    CB->AllocatedSize    = newsize;
    FreeCodebook(&tempCB);
    }
}


/*-------------------------------------------------------------------*/


void IncreaseCodebookSize(CODEBOOK* CB, int newsize)
{
  if (newsize > BookSize(CB))
    {
    if (newsize > CB->AllocatedSize)
      {
      SetAllocatedCodebookSize(CB, newsize);
      }
    BookSize(CB) = newsize;
    }
  else
    {
    ExitProcessing(FATAL_ERROR);
    }
}


/*-------------------------------------------------------------------*/


void DecreaseCodebookSize(CODEBOOK* CB, int newsize)
{
  if ((newsize >= BookSize(CB)) || (newsize < 0))
    ExitProcessing(FATAL_ERROR);
  BookSize(CB) = newsize;
}


/*-------------------------------------------------------------------*/


void ChangeCodebookSize(CODEBOOK* CB, int newsize)
{
  if (newsize < BookSize(CB)) DecreaseCodebookSize(CB, newsize);
  else if (newsize > BookSize(CB)) IncreaseCodebookSize(CB, newsize);
}


/*========================  Codebook  ================================*/


BOOKNODE CreateEmptyNode(int Vsize)
{
  BOOKNODE node;

  node.vector = CreateEmptyVector(Vsize);
  node.vmean  = 0;
  node.freq   = 0;
  node.name   = NULL;

  return( node );
}


/*-------------------------------------------------------------------*/


void FreeNode(BOOKNODE node)
{
  FreeVector( node.vector );
  deallocate( node.name );
}


/*-------------------------------------------------------------------*/


void CopyNode(BOOKNODE* source, BOOKNODE* dest, int Vsize)
{
   if ( source == dest ) return;

   CopyVector( source->vector, dest->vector, Vsize );
   dest->freq = source->freq;
   dest->vmean = source->vmean;

   deallocate(dest->name);

   if ( source->name )
     {
     dest->name = (char*) allocate( (strlen(source->name)+1) * sizeof(char) );
     strcpy(dest->name, source->name);
     }
   else
     {
     dest->name = NULL;
     }
}


/*-------------------------------------------------------------------*/


void ChangeVectorName( BOOKNODE* node, char* NewName)
{
  deallocate(node->name);
  if ( NewName == NULL)
    {
    node->name = NULL;
    }
  else
    {
    node->name = (char*) allocate((strlen(NewName)+1) * sizeof(char));
    strcpy(node->name, NewName);
    }
}


/*-------------------------------------------------------------------*/


static void CalculateTotalFreq(CODEBOOK* CB)
{
  int i;
  int totalfreq = 0;

  for (i=0; i<BookSize(CB); i++)
    totalfreq += VectorFreq(CB, i);

  TotalFreq(CB) = totalfreq;

}


/*-------------------------------------------------------------------*/


void CopyCodebookHeader(CODEBOOK *sourceCB, CODEBOOK *destCB)
{
  strcpy(destCB->Versionstr, sourceCB->Versionstr);
  destCB->BlockSizeX         = sourceCB->BlockSizeX;
  destCB->BlockSizeY         = sourceCB->BlockSizeY;
  destCB->CodebookSize       = sourceCB->CodebookSize;
  destCB->BytesPerElement    = sourceCB->BytesPerElement;
  destCB->MinValue           = sourceCB->MinValue;
  destCB->MaxValue           = sourceCB->MaxValue;
  destCB->Preprocessing      = sourceCB->Preprocessing;
  destCB->TotalFreq          = sourceCB->TotalFreq;
  strcpy( destCB->GenerationMethod, sourceCB->GenerationMethod );
/*   destCB->AllocatedSize       = sourceCB->AllocatedSize; */
}


/*-------------------------------------------------------------------*/


void CopyCodebook(CODEBOOK *sourceCB, CODEBOOK *destCB)
{
  int i;

  if (destCB->AllocatedSize < sourceCB->CodebookSize)
    {
    ErrorMessage("ERROR: Destination codebook is too small (%d -> %d).\n",
                 sourceCB->CodebookSize, destCB->AllocatedSize);
    ExitProcessing(-1);
    }

  CopyCodebookHeader(sourceCB, destCB);

  for (i=0; i<sourceCB->CodebookSize; i++)
    {
    CopyNode(&Node(sourceCB,i), &Node(destCB,i), VectorSize(sourceCB));
    }
}


/*-------------------------------------------------------------------*/


void AddCodebook(CODEBOOK *ToCB, CODEBOOK *FromCB)
{
  int  i;
  int  booksize = BookSize(ToCB);
  int  newsize  = BookSize(FromCB)+BookSize(ToCB);

  IncreaseCodebookSize(ToCB, newsize);
  BookSize(ToCB) = newsize;
  for(i=0; i<BookSize(FromCB); i++)
      {
      CopyNode(&Node(FromCB,i), &Node(ToCB,booksize+i), VectorSize(ToCB));
      }
  ToCB->TotalFreq += FromCB->TotalFreq;

  /* "++" is added to the generation method name */
  i = strlen(ToCB->GenerationMethod) - 1;
  if (i >= MaxGenMethodLength - 3
      || ((ToCB->GenerationMethod[i - 1] == '+')
      && (ToCB->GenerationMethod[i] == '+'))) return;
  strcat(ToCB->GenerationMethod, "++");
}

/*-------------------------------------------------------------------*/


int DuplicatesInCodebook(CODEBOOK *CB)
{
  int  i,j;
  int  ndups = 0;

  for( i=0; i<BookSize(CB)-1; i++ )
    for( j=i+1; j<BookSize(CB); j++ )
      {
      if( EqualVectors( Vector(CB,i), Vector(CB,j), VectorSize(CB)))
        {
        ndups++;
        j = BookSize(CB);
        }
      }
  return( ndups );
}


/*-------------------------------------------------------------------*/


void CodebookCentroid(CODEBOOK* CB, VECTORTYPE v)
{
  llong*   SumCounter;
  int      i,k;

  if( TotalFreq(CB)==0 ) return;

  SumCounter = allocate( VectorSize(CB) * sizeof(llong) );
  for( k=0; k<VectorSize(CB); k++ )
     SumCounter[k] = 0;

  for( i=0; i<BookSize(CB); i++ )
    for( k=0; k<VectorSize(CB); k++ )
      {
      SumCounter[k] += (llong) VectorScalar(CB,i,k) * VectorFreq(CB,i);
      }

  for( k=0; k<VectorSize(CB); k++ )
     {
     v[k] = (VECTORELEMENT) ( (double) SumCounter[k] / TotalFreq(CB) + 0.5);
     }
  deallocate(SumCounter);
}


/*-------------------------------------------------------------------*/


void CalculateVectorMeans(CODEBOOK* CB)
{
  llong vsum;
  int   i,k;

  for( i=0; i<BookSize(CB); i++ )
    {
    vsum = 0;
    for( k=0; k<VectorSize(CB); k++ )
       {
       vsum += (llong) VectorScalar(CB,i,k);
       }
    VectorMean(CB,i) = (VECTORELEMENT) ( (double) vsum/VectorSize(CB) + 0.5);
    }
}


/*-------------------------------------------------------------------*/


void UnrollTrainingSet(TRAININGSET* TS)
{
  int         i, j, l;
  TRAININGSET tmpTS;
  BOOKTYPE    tmpBook;
  int         tmpAsize;

  CreateNewTrainingSet(&tmpTS,
                       TotalFreq(TS),
                       TS->BlockSizeX,
                       TS->BlockSizeY,
                       TS->BytesPerElement,
                       TS->MinValue,
                       TS->MaxValue,
                       TS->Preprocessing,
                       TS->GenerationMethod);

  for( i = 0, l = 0; i < BookSize(TS); i++ )
    {
    for( j = 0; j < VectorFreq(TS, i); j++ )
      {
      CopyNode(&Node(TS,i), &Node(&tmpTS,l), VectorSize(TS));
      VectorFreq(&tmpTS, l) = 1;
      l++;
      }
    }
  BookSize(&tmpTS) = BookSize(TS);
  BookSize(TS) = TotalFreq(TS);

  tmpBook    = TS->Book;              /* swap books */
  TS->Book   = tmpTS.Book;
  tmpTS.Book = tmpBook;

  tmpAsize = TS->AllocatedSize;
  TS->AllocatedSize = tmpTS.AllocatedSize;
  tmpTS.AllocatedSize = tmpAsize;

  FreeCodebook(&tmpTS);
}


/*========================  Codebook I/O  ===========================*/


static void ReadOldCodebookFileHeader(FILE* f, CODEBOOK* CB)
{
  fscanf(f, "%i\n", &(CB->BlockSizeX));
  fscanf(f, "%i\n", &(CB->BlockSizeY));
  fscanf(f, "%i\n", &(CB->CodebookSize));
  fscanf(f, "%i\n", &(CB->Preprocessing));
  fscanf(f, "%*i\n");
  fscanf(f, "%*i\n");
  fscanf(f, "%*i\n");
  fscanf(f, "%i\n", &(CB->MaxValue));
  if( strncmp(CodebookID, CB->Versionstr, strlen(CodebookID)) == 0 )
    {
    fgets(CB->GenerationMethod, MaxGenMethodLength, f);
    RemoveLineBreak(CB->GenerationMethod);
    }
  else
    {
    strcpy(CB->GenerationMethod, "");
    }
  while( getc(f) == '-' );       /* Read dashes and '\n' */

  TotalFreq(CB)          = 0;
  CB->BytesPerElement    = 1;
  CB->MinValue           = 0;

}


/*-------------------------------------------------------------------*/


static void ReadNewCodebookFileHeader(FILE* f, CODEBOOK* CB)
{
  char tempchar;

  fscanf(f, "%i\n", &(CB->BlockSizeX));
  fscanf(f, "%i\n", &(CB->BlockSizeY));
  fscanf(f, "%i\n", &(CB->CodebookSize));
  fscanf(f, "%i\n", &(CB->TotalFreq));
  fscanf(f, "%i\n", &(CB->BytesPerElement));
  fscanf(f, "%i\n", &(CB->MinValue));
  fscanf(f, "%i\n", &(CB->MaxValue));
  fscanf(f, "%i\n", &(CB->Preprocessing));

  tempchar = getc(f);
  ungetc(tempchar,f);
  if (tempchar != '-')
    {
    fgets(CB->GenerationMethod, MaxGenMethodLength, f);
    RemoveLineBreak(CB->GenerationMethod);
    }
  else
    {
    strcpy(CB->GenerationMethod, "");
    }

  while( getc(f) == '-' );       /* Read dashes and '\n' */

}


/*-------------------------------------------------------------------*/


static int ReadCodebookFileHeader(FILE* f, CODEBOOK* CB)
{
  int versionnumber, i=0;

  fgets(CB->Versionstr, MaxVersionLength, f);
  RemoveLineBreak(CB->Versionstr);

  /* find the first number in Versionstr */
  while ( i < (int) strlen(CB->Versionstr) &&
        (CB->Versionstr[i]<'0' || CB->Versionstr[i]>'9'))  i++;

  if (i == (int) strlen(CB->Versionstr))
    {
    ErrorMessage("ERROR: No version number found in header.\n");
    ExitProcessing(-1);
    }
  versionnumber = CB->Versionstr[i] - '0';

  switch (versionnumber)
    {
    case 0:
    case 1:
      {
      ReadOldCodebookFileHeader(f,CB);
      break;;
      }
    case 2:
      {
      ReadNewCodebookFileHeader(f,CB);
      break;
      }
    default:
      {
      ErrorMessage("ERROR: Unknown file version number (%d).\n", versionnumber);
      ExitProcessing(-1);
      }
    }

  return (versionnumber);
}

/*-------------------------------------------------------------------*/


static void WriteCodebookFileHeader(FILE* f, CODEBOOK *CB)
{
  fprintf(f, "%s\n", CB->Versionstr);
  fprintf(f, "%i\n", CB->BlockSizeX);
  fprintf(f, "%i\n", CB->BlockSizeY);
  fprintf(f, "%i\n", CB->CodebookSize);
  fprintf(f, "%i\n", CB->TotalFreq);
  fprintf(f, "%i\n", CB->BytesPerElement);
  fprintf(f, "%i\n", CB->MinValue);
  fprintf(f, "%i\n", CB->MaxValue);
  fprintf(f, "%i\n", CB->Preprocessing);

  if ( strlen(CB->GenerationMethod) > 0 )
    fprintf(f, "%s\n", CB->GenerationMethod);

  fprintf(f, "-------------------------------------");
  putc(10, f);
}



/*-------------------------------------------------------------------*/


static void ReadVectorNameFromFile(FILE* f, char** name)
{
  int  i = 0;
  char n[MaxVectorNameLength]="";
  int  c;

  do
    {
    c = getc(f);
    if (c == EOF)
      {
      ErrorMessage("ERROR: premature end of file\n");
      ExitProcessing(-1);
      }
    n[i] = (char) c;
    i++;
    }
  while ( c != '\0' && i<MaxVectorNameLength);

  if (c != '\0')
    {
    ErrorMessage("ERROR: Too long vector name:\n%*s\nMaximum is %d characters.\n",
                 MaxVectorNameLength, n, MaxVectorNameLength);
    }

  if ( i > 1 )
    {
    *name = (char*) allocate( i*sizeof(char) );
    strcpy(*name,n);
    }
  else
    {
    *name = NULL;
    }
}


/*-------------------------------------------------------------------*/


static void ReadVectorFromFile(FILE* f,
                               BOOKNODE* v,
                               int Vsize,
                               int VEsize,
                               int versionnumber)

{
  llong vsum=0;
  int  i,x;

  /* read vector name */
  if (versionnumber >= 2) ReadVectorNameFromFile(f, &(v->name));
  else            v->name = NULL;

  /* read vector elements */
  for( i=0; i<Vsize; i++ )
   {
     if ( versionnumber >= 2 )  ReadIntegerFromFile(f, &x, VEsize);
   else           ReadLittleEndianIntegerFromFile(f, &x, VEsize);
   v->vector[i] = (VECTORELEMENT) x;
   vsum += v->vector[i];
   }

  /* read vector frequency */
  if ( versionnumber >= 2 )  ReadIntegerFromFile(f, &x, CURRENTINTSIZE);
  else             ReadLittleEndianIntegerFromFile(f, &x, 2);
  v->freq = x;

  /* calculate vector mean */
  v->vmean = (VECTORELEMENT) ( (double) vsum / Vsize + 0.5);
}


/*-------------------------------------------------------------------*/


static void WriteVectorToFile(FILE* f, BOOKNODE* v, int Vsize, int VEsize)
{
  int  i;

  if ( v->name )
    fprintf(f, "%s", v->name);

  putc(0, f);

  for( i=0; i<Vsize; i++ )
    {
    WriteIntegerToFile(f, (int) v->vector[i], VEsize);
    }

  WriteIntegerToFile(f, v->freq, CURRENTINTSIZE);

}


/*-------------------------------------------------------------------*/


void ReadCodebook(char* FileName, CODEBOOK* CB)
{
  int   i;
  FILE* f;
  int   totalfreq;
  int   versionnumber;

  f = FileOpen(FileName, INPUT, NO);
  versionnumber = ReadCodebookFileHeader(f, CB);

  AllocateMemoryForBook(CB);

  totalfreq = 0;

  for(i=0; i<BookSize(CB); i++)
    {
    ReadVectorFromFile(f,
                       &Node(CB,i),
                       VectorSize(CB),
                       CB->BytesPerElement,
                       versionnumber);
    totalfreq += VectorFreq(CB,i);
    }

  if ( totalfreq != TotalFreq(CB) && versionnumber > 1 )
    {
    PrintMessage("WARNING: TotalFreq header field = %d but ", TotalFreq(CB));
    PrintMessage("calculated totalfreq = %d.\n", totalfreq);
    }

  TotalFreq(CB) = totalfreq;
  fclose(f);
}

/* ----------------------------------------------------------------- */

void WriteCodebook(char* FileName, CODEBOOK *CB, int AllowOverWrite)
{
  int   i;
  FILE *f;

  CalculateTotalFreq(CB);

  f = FileOpen(FileName, OUTPUT, AllowOverWrite);
  if (f == NULL) return;
  WriteCodebookFileHeader(f, CB);

  for(i=0; i<CB->CodebookSize; i++)
    {
    WriteVectorToFile(f, &Node(CB,i), VectorSize(CB), CB->BytesPerElement);
    }

  fclose(f);
}


/* ----------------------------------------------------------------- */


void PrintCodebook(CODEBOOK* CB)
{
  int i;

  PrintMessage("Codebook generated: %s\n",CB->GenerationMethod);
  PrintMessage("Vectors=%i, Totalfreq=%i \n",BookSize(CB),TotalFreq(CB));
  for(i=0; i<BookSize(CB); i++)
     {
     PrintMessage("%-3i: (f=%-4i, vm=%-3i)  ",
       i, VectorFreq(CB,i), VectorMean(CB,i));
     PrintVector(Vector(CB,i),VectorSize(CB),1);
     }
}


/*=======================  Partitioning  =============================*/


static void AddToCC(TRAININGSET*  TS,
                    PARTITIONING* P,
                    int           TSvec,
                    int           Pindex)
{
  int k;
  BOOKNODE* v = &Node(TS, TSvec);

  for( k = 0; k < P->Vsize; k++ )
    {
    CCScalar(P, Pindex, k) += v->freq * (llong)v->vector[k];
    }
  CCFreq(P, Pindex) += v->freq;
  UniqueVectors(P, Pindex)++;
}


/*-------------------------------------------------------------------*/


static void SubtractFromCC(TRAININGSET*  TS,
                           PARTITIONING* P,
                           int           TSvec,
                           int           Pindex)
{
  int k;
  BOOKNODE* v = &Node(TS, TSvec);

  for( k = 0; k < P->Vsize; k++ )
    {
    CCScalar(P, Pindex, k) -= v->freq * (llong)v->vector[k];
    }
  CCFreq(P, Pindex) -= v->freq;
  UniqueVectors(P, Pindex)--;
}


/* ----------------------------------------------------------------- */


static void PutAllInFirstPartition(TRAININGSET* TS, PARTITIONING* P)
{
  int i, j, k;

  for( i = 0; i < P->PartitionCount; i++ )
    {
    FirstVector(P, i) = ENDPARTITION;

    for( k = 0; k < P->Vsize; k++ )
      {
      CCScalar(P, i, k) = 0;
      }
    CCFreq(P, i)        = 0;
    UniqueVectors(P, i) = 0;
    }
  FirstVector(P, 0) = 0;

  for( j = 0; j < P->TSsize; j++ )
    {
    Map(P, j)  = 0;
    NextVector(P, j) = j + 1;

    AddToCC(TS, P, j, 0);
    }
  NextVector(P, P->TSsize - 1) = ENDPARTITION;
}


/* ----------------------------------------------------------------- */


void SetNumberOfAllocatedPartitions(PARTITIONING* P, int newsize)
{
  int*     oldfirst  = P->First;
  int*     oldunique = P->Uniques;
  COUNTER* oldCC     = P->CC;
  int      i, k;

  if (P->PartitionCount > newsize)
    {
    ErrorMessage("CB.C::SetNumberOfAllocatedPartitions: "
      "New allocated size of partitions (%i) < logical partition count (%i).\n",
      newsize, P->PartitionCount);
    ExitProcessing(FATAL_ERROR);
    }
  if (P->AllocatedSize < newsize)
    {
    /* Allocate memory for new 'codebook' */
    P->First   = (int*) allocate(newsize * sizeof(int));
    P->Uniques = (int*) allocate(newsize * sizeof(int));
    P->CC      = (COUNTER*)allocate(newsize * sizeof(COUNTER));

    /* Copy old values. */
    for(i = 0; i < P->PartitionCount; i++)
      {
      FirstVector(P, i)   = oldfirst[i];
      P->CC[i].counter    = oldCC[i].counter;
      CCFreq(P, i)        = oldCC[i].freq;
      UniqueVectors(P, i) = oldunique[i];
      }

    /* Allocate memory for new vectors. */
    for(i = P->PartitionCount; i < newsize; i++)
      {
      FirstVector(P, i) = ENDPARTITION;
      P->CC[i].counter  = (llong*)allocate(P->Vsize * sizeof(llong));
      for( k = 0; k < P->Vsize; k++ )
        {
        CCScalar(P, i, k) = 0;
        }
      CCFreq(P, i)        = 0;
      UniqueVectors(P, i) = 0;
      }

    P->AllocatedSize  = newsize;

    deallocate(oldfirst);
    deallocate(oldunique);
    deallocate(oldCC);
    }
}

/* ----------------------------------------------------------------- */


void IncreaseNumberOfPartitions(PARTITIONING* P, int newsize)
{
  if (P->PartitionCount >= newsize)
    {
    ErrorMessage("CB.C::IncreaseNumberOfPartitions: "
      "New partition count (%i) less or equal to old (%i).\n",
      newsize, P->PartitionCount);
    ExitProcessing(FATAL_ERROR);
    }
  if( P->AllocatedSize < newsize )
    {
    SetNumberOfAllocatedPartitions(P, newsize);
    }
  PartitionCount(P) = newsize;
}


/* ----------------------------------------------------------------- */


void DecreaseNumberOfPartitions(PARTITIONING* P, int newsize)
{
  int k;
  if (P->PartitionCount <= newsize)
    {
    ErrorMessage("CB.C::DecreaseNumberOfPartitions: "
      "New partition count (%i) greater or equal to old (%i).\n",
      newsize, P->PartitionCount);
    ExitProcessing(FATAL_ERROR);
    }
  else if (newsize <= 0)
    {
    ErrorMessage("CB.C::DecreaseNumberOfPartitions: "
      "New partition count (%i) less than or equal to zero.\n",
      newsize);
    }
  for (k = newsize; k < PartitionCount(P); k++)
    if (FirstVector(P, k) != ENDPARTITION)
      {
      ErrorMessage("CB.C::DecreaseNumberOfPartitions: "
        "Partition %i not empty.\n", k);
      ExitProcessing(FATAL_ERROR);
      }
  PartitionCount(P) = newsize;
}


/* ----------------------------------------------------------------- */


void ChangeNumberOfPartitions(PARTITIONING* P, int newsize)
{
  if (newsize < P->PartitionCount) DecreaseNumberOfPartitions(P, newsize);
  else if (newsize > P->PartitionCount)
    IncreaseNumberOfPartitions(P, newsize);
}


/* ----------------------------------------------------------------- */


void PutAllInOwnPartition(TRAININGSET* TS, PARTITIONING* P)
{
  int i, k;

  ChangeNumberOfPartitions(P, BookSize(TS));

  for( i = 0; i < P->PartitionCount; i++ )
    {
    Map(P, i)         = i;
    FirstVector(P, i) = i;
    NextVector(P, i)  = ENDPARTITION;

    for( k = 0; k < P->Vsize; k++ )
      {
      CCScalar(P, i, k) = (llong)VectorScalar(TS, i, k) * VectorFreq(TS, i);
      }
    CCFreq(P, i)        = VectorFreq(TS, i);
    UniqueVectors(P, i) = 1;
    }
}


/* ----------------------------------------------------------------- */


static void AddToPartition(TRAININGSET* TS,
                           PARTITIONING* P,
                           int Pindex,
                           int item)
{
  Map(P, item)           = Pindex;
  NextVector(P, item)    = FirstVector(P, Pindex);
  FirstVector(P, Pindex) = item;

  AddToCC(TS, P, item, Pindex);
}


/* ----------------------------------------------------------------- */


static void RemoveFromPartition(TRAININGSET* TS, PARTITIONING* P, int item)
{
  int Pindex = Map(P, item);
  int tmp;

  if( Pindex != ENDPARTITION )
    {
    if( FirstVector(P, Pindex) == item )
      {
      Map(P, item)           = ENDPARTITION;
      FirstVector(P, Pindex) = NextVector(P, item);
      NextVector(P, item)    = ENDPARTITION;
      }
    else
      {
      for( tmp = FirstVector(P, Pindex);
           NextVector(P, tmp) != item;
           tmp = NextVector(P, tmp) )
        ;
      Map(P, item)        = ENDPARTITION;
      NextVector(P, tmp)  = NextVector(P, item);
      NextVector(P, item) = ENDPARTITION;
      }
    SubtractFromCC(TS, P, item, Pindex);
    }
}

/* ----------------------------------------------------------------- */


void ChangePartitionFast(TRAININGSET*    TS,
                         PARTITIONING*   P,
                         int             Vnew,
                         int             item,
                         int             prev)
{
  int Pindex = Map(P, item);

  /* Remove vector item from its current partition */
  if( Pindex != ENDPARTITION )
    {
    if( FirstVector(P, Pindex) == item )
      {
      Map(P, item)           = ENDPARTITION;
      FirstVector(P, Pindex) = NextVector(P, item);
      NextVector(P, item)    = ENDPARTITION;
      }
    else
      {
      Map(P, item)         = ENDPARTITION;
      NextVector(P, prev)  = NextVector(P, item);
      NextVector(P, item)  = ENDPARTITION;
      }
    SubtractFromCC(TS, P, item, Pindex);
    }

  /* Add vector item to partition Vnew */
  if (Pindex != ENDPARTITION)
     {
     AddToPartition(TS, P, Vnew, item);
     }
}
     
/* ----------------------------------------------------------------- */


void ChangePartition(TRAININGSET* TS, PARTITIONING* P, int Pindex, int item)
{
  RemoveFromPartition(TS, P, item);
  if (Pindex != ENDPARTITION)
    {
    AddToPartition(TS, P, Pindex, item);
    }
}


/* ----------------------------------------------------------------- */


void JoinPartitions(TRAININGSET* TS, PARTITIONING* P, int To, int From)
{
  int j = FirstVector(P, To);

  if( EndOfPartition(j) )
    {
    FirstVector(P, To) = FirstVector(P, From);
    }
  else
    {
    while( ! IsLast(P, j) )
      {
      j = NextVector(P, j);
      }
    NextVector(P, j) = FirstVector(P, From);
    }
  for( j = FirstVector(P, From); ! EndOfPartition(j); j = NextVector(P, j) )
    {
    Map(P, j) = To;
    SubtractFromCC(TS, P, j, From);
    AddToCC(TS, P, j, To);
    }
  FirstVector(P, From) = ENDPARTITION;

  /* move vectors from the last partition to the empty one */

  if ( From != P->PartitionCount-1 )
    {
    while ( ! EndOfPartition(FirstVector(P,P->PartitionCount-1)) )
      {
      ChangePartition(TS, P, From, FirstVector(P, P->PartitionCount-1));
      }
    }

  P->PartitionCount--;
}


/* ----------------------------------------------------------------- */


void PartitionCentroid(PARTITIONING* P,
                       int           Pindex,
                       BOOKNODE*     centroid)
{
  llong vsum=0;
  int   k;

  if( CCFreq(P, Pindex) > 0 )
    {
    for( k = 0; k < P->Vsize; k++ )
      {
      centroid->vector[k] = (VECTORELEMENT)
                             roundint( (double) CCScalar(P, Pindex, k) /
                                       (double) CCFreq(P, Pindex) );
      vsum += centroid->vector[k];
      }
    centroid->freq  = CCFreq(P, Pindex);
    centroid->vmean = (VECTORELEMENT) ( (double) vsum / P->Vsize + 0.5);
    }
  else
    {
    ErrorMessage("ERROR: PartitionCentroid Pindex=%i CCFreq=%i\n",
           Pindex, CCFreq(P, Pindex));
    ExitProcessing( -1 );
    }
}


/* ----------------------------------------------------------------- */


void FillEmptyPartitions(TRAININGSET* TS, CODEBOOK* CB, PARTITIONING* P)
{
  int    i, j;
  int    n;
  int    NextNew;
  int*   NewIndex;   /* indices of CB */
  llong* error;
  llong  newerror;
  int    nEmpties = 0;
  int    OldPartition;

  for (i=0; i<P->PartitionCount; i++)
    {
    if( CCFreq(P, i) == 0 )  /* Is this an empty partition? */
      nEmpties++;
    }

  if (nEmpties == 0) return;

  /* Allocate temporary memory */
  NewIndex  = (int*)allocate((nEmpties + 1) * sizeof(int));
  error     = (llong*)allocate((nEmpties + 1) * sizeof(llong));

  /* Find new vectors */
  NextNew = 0;
  for( j = 0; j < BookSize(TS); j++ )
    {
    newerror =
    error[NextNew] = VectorDist(Vector(CB, Map(P, j)), Vector(TS, j),
                                VectorSize(TS)) * VectorFreq(TS, j);
    n = NextNew;
    while( n > 0 && error[n] > error[n - 1] )
      {
      NewIndex[n] = NewIndex[n - 1];
      error[n] = error[n - 1];
      n--;
      }
    NewIndex[n] = j;
    error[n] = newerror;

    if( NextNew < nEmpties  )
      {
      NextNew++;
      }
    }

  /* Allocate new codevectors to empty partitions */
  for( i = 0, n = 0; n < nEmpties && i < P->PartitionCount; i++ )
    {
    if( CCFreq(P, i) == 0 )  /* Is this an empty partition? */
      {
      /* Move the vector 'NewIndex[n]' from its partition
         to the current empty partition. */
      OldPartition = Map(P, NewIndex[n]);
      CopyNode(&Node(TS, NewIndex[n]), &Node(CB, i), VectorSize(TS));

      ChangePartition(TS, P, i, NewIndex[n]);
      if( CCFreq(P, OldPartition) > 0 )
        {
        PartitionCentroid(P, OldPartition, &Node(CB, OldPartition));
        }
      else
        {
        VectorFreq(CB, OldPartition) = 0;
        }

      n++;
      }
    }

  /* Free temporary memory */
  deallocate(NewIndex);
  deallocate(error);
}


/*-------------------------------------------------------------------*/


static void CopyPartitioningHeader(PARTITIONING* sourceP, PARTITIONING* destP)
{
  strcpy(destP->Versionstr, sourceP->Versionstr);
  destP->PartitionCount = sourceP->PartitionCount;
  strcpy( destP->GenerationMethod, sourceP->GenerationMethod );
}


/*-------------------------------------------------------------------*/


static void CopyCounter(COUNTER* sourceC, COUNTER* destC, int Vsize)
{
  int i;

  if (sourceC == NULL)
    {
    for (i=0; i<Vsize; i++)
      {
      destC->counter[i] = 0;
      }
    destC->freq = 0;
    }
  else
    {
    for (i=0; i<Vsize; i++)
      {
      destC->counter[i] = sourceC->counter[i];
      }
    destC->freq = sourceC->freq;
    }
}


/*-------------------------------------------------------------------*/


void CopyPartitioning(PARTITIONING* sourceP, PARTITIONING* destP)
{
  int i;

  if (destP->AllocatedSize < sourceP->PartitionCount)
    {
    ErrorMessage("ERROR: Destination partitioning is too small (%d -> %d).\n",
            sourceP->PartitionCount, destP->AllocatedSize);
    ExitProcessing(-1);
    }

  if (destP->TSsize != sourceP->TSsize ||
      destP->Vsize  != sourceP->Vsize)
    {
    ErrorMessage("ERROR: Partitionings are not for same training set.\n");
    ExitProcessing(-1);
    }

  CopyPartitioningHeader(sourceP, destP);

  for (i=0; i<sourceP->TSsize; i++)
    {
    Map(destP, i)        = Map(sourceP, i);
    NextVector(destP, i) = NextVector(sourceP, i);
    }

  for (i=0; i<sourceP->PartitionCount; i++)
    {
    FirstVector(destP, i)   = FirstVector(sourceP, i);
    CopyCounter(&(sourceP->CC[i]), &(destP->CC[i]), sourceP->Vsize);
    UniqueVectors(destP, i) = UniqueVectors(sourceP, i);
    }

  for (i=sourceP->PartitionCount; i<destP->AllocatedSize; i++)
    {
    FirstVector(destP, i) = ENDPARTITION;
    CopyCounter(NULL, &(destP->CC[i]), destP->Vsize);
    UniqueVectors(destP, i) = 0;
    }
}


/*==================  Partitioning memory control  ==================*/


void CreateNewPartitioning(PARTITIONING* P,
                           TRAININGSET*  TS,
                           int           PartitionCount)
{
  int i;

  SetVersionString(P, PAFILE);

  P->PartitionCount =  PartitionCount;
  P->TSsize         =  BookSize(TS);
  P->Map            =  (int*)allocate(BookSize(TS) * sizeof(int));
  P->First          =  (int*)allocate(PartitionCount * sizeof(int));
  P->Next           =  (int*)allocate(BookSize(TS) * sizeof(int));
  P->Uniques        =  (int*)allocate(PartitionCount * sizeof(int));
  P->Vsize          =  VectorSize(TS);
  P->AllocatedSize  =  PartitionCount;

  P->CC = (COUNTER*)allocate(P->PartitionCount * sizeof(COUNTER));
  for( i = 0; i < P->PartitionCount; i++ )
    {
    P->CC[i].counter = (llong*)allocate(P->Vsize * sizeof(llong));
    }
  /*
  strcpy( P->GenerationMethod, TS->GenerationMethod );
  */
  strcpy(P->GenerationMethod, "");
  PutAllInFirstPartition(TS, P);

}


/* ----------------------------------------------------------------- */


void FreePartitioning(PARTITIONING* P)
{
  int i;

  deallocate(P->Map);
  deallocate(P->First);
  deallocate(P->Next);
  deallocate(P->Uniques);
  for( i = 0; i < P->AllocatedSize; i++ )
    {
    deallocate(P->CC[i].counter);
    }
  deallocate(P->CC);
}


/*=====================  Partitioning I/O  ==========================*/


static void ReadPartitioningFileHeader(FILE* f,
                                       PARTITIONING* P,
                                       TRAININGSET* TS)
/* allocates also the memory needed */
{
  char tempchar;

  fgets(P->Versionstr, MaxVersionLength, f);
  RemoveLineBreak(P->Versionstr);

  fscanf(f, "%i\n", &(P->PartitionCount));

  CreateNewPartitioning(P, TS, P->PartitionCount);

  fscanf(f, "%i\n", &(P->TSsize));

  if (P->TSsize != BookSize(TS))
    {
    ErrorMessage("ERROR: Partitioning is not for this training set (%d != %d).\n",
            P->TSsize, BookSize(TS));
    ExitProcessing(-1);
    }

  tempchar = getc(f);
  ungetc(tempchar,f);
  if (tempchar != '-')
    {
    fgets(P->GenerationMethod, MaxGenMethodLength, f);
    RemoveLineBreak(P->GenerationMethod);
    }
  else
    {
    strcpy(P->GenerationMethod, "");
    }

  while( getc(f) == '-' );       /* Read dashes and '\n' */

}


/*-------------------------------------------------------------------*/


static void WritePartitioningFileHeader(FILE* f, PARTITIONING* P)
{
  fprintf(f, "%s\n", P->Versionstr);
  fprintf(f, "%i\n", P->PartitionCount);
  fprintf(f, "%i\n", P->TSsize);

  if ( strlen(P->GenerationMethod) > 0 )
    fprintf(f, "%s\n", P->GenerationMethod);

  fprintf(f, "-------------------------------------");
  putc(10, f);
}



/*-------------------------------------------------------------------*/


void ReadPartitioning(char* FileName, PARTITIONING* P, TRAININGSET* TS)
{
  int   i;
  FILE* f;
  int   index;
  int   c;

  f = FileOpen(FileName, INPUT, NO);
  ReadPartitioningFileHeader(f, P, TS);

  for(i=0; i<BookSize(TS); i++)
    {
    c = fscanf(f, "%i", &index);
    if (c == 0)
      {
      ErrorMessage("ERROR reading partitioning.\n");
      ExitProcessing(-1);
      }

    do
      c = getc(f);
    while (c != 10 && c != EOF);

    if (index > P->PartitionCount || index < 1)
      {
      ErrorMessage("ERROR: Invalid partition index (%d) found.\n", index);
      ExitProcessing(-1);
      }
    ChangePartition(TS, P, index-1, i);
    }

  fclose(f);
}


/* ----------------------------------------------------------------- */


void WritePartitioning(char* FileName,
                       PARTITIONING* P,
                       TRAININGSET* TS,
                       int AllowOverWrite)
{
  int   i;
  FILE *f;

  f = FileOpen(FileName, OUTPUT, AllowOverWrite);
  WritePartitioningFileHeader(f, P);

  if (P->TSsize != BookSize(TS))
    {
    ErrorMessage("ERROR: Partitioning is not for this training set (%d != %d).\n",
            P->TSsize, BookSize(TS));
    ExitProcessing(-1);
    }

  for(i=0; i<P->TSsize; i++)
    {
    fprintf(f, "%i", Map(P, i)+1);
    if ( strlength(VectorName(TS,i)) > 0)
      fprintf(f, " %s", VectorName(TS,i));
    putc(10,f);
    }

  fclose(f);
}


/*==========================  GLA  ===================================*/


int FindNearestVector(BOOKNODE*     v,
                      CODEBOOK*     CB,
                      llong*        error,
                      int           guess,
                      DISTANCETYPE  disttype)
{
  int   i;
  int   MinIndex = guess;
  llong e;

  *error = VectorDistance(Vector(CB, guess),
                          v->vector,
                          VectorSize(CB),
                          MAXLLONG,
                          disttype);

  for(i = 0; i < BookSize(CB); i++)
    {
    e = VectorDistance(Vector(CB, i),
                       v->vector,
                       VectorSize(CB),
                       *error,
                       disttype);
    if( e < *error )
      {
      *error   = e;
      MinIndex = i;
      if( e == 0 )
        {
        return( MinIndex );
        }
      }
    }

  return( MinIndex );
}


/*-------------------------------------------------------------------*/


static double GenerateOptimalPartitioningMeanError(TRAININGSET*  TS,
                                                   CODEBOOK*     CB,
                                                   PARTITIONING* P,
                                                   DISTANCETYPE  disttype)

/* Generates an optimal partitioning from a codebook; returns mean error. */
{
  llong     error;
  llong     totalerror = 0;
  int       i;
  int       nearest;

  /* Find mapping from training vector to code vector */
  for(i = 0; i < BookSize(TS); i++)
    {
    nearest = FindNearestVector( &Node(TS,i),
                                 CB,
                                 &error,
                                 Map(P,i),
                                 disttype);
    if( nearest != Map(P, i) )
      {
      ChangePartition(TS, P, nearest, i);
      }

    totalerror += error * VectorFreq(TS, i);
    }

  return (double) totalerror / (double) (TotalFreq(TS) * VectorSize(TS));

}


/*-------------------------------------------------------------------*/


static double GenerateOptimalPartitioningSC(TRAININGSET*  TS,
                                            CODEBOOK*     CB,
                                            PARTITIONING* P)

/* Generates optimal partitioning from a codebook; returns error. */
{
  GenerateOptimalPartitioningMeanError(TS, CB, P, ENTROPYDIST);

  return AverageErrorForSolution(TS, CB, P, SC);
}


/*-------------------------------------------------------------------*/


double GenerateOptimalPartitioningGeneral(TRAININGSET*  TS,
                                          CODEBOOK*     CB,
                                          PARTITIONING* P,
                                          ERRORFTYPE    errorf)

/* Calculates the error of partitioning using given codevectors. */
{
  switch (errorf)
    {
    case MSE:
      {
      return GenerateOptimalPartitioningMeanError(TS, CB, P, EUCLIDEANSQ);
      }
    case SC:
      {
      return GenerateOptimalPartitioningSC(TS, CB, P);
      }
    case JACCARDERROR:
      {
      return GenerateOptimalPartitioningMeanError(TS, CB, P, JACCARD);
      }
    case DICEERROR:
      {
      return GenerateOptimalPartitioningMeanError(TS, CB, P, DICE);
      }
    default:
      {
      ErrorMessage("ERROR: Unknown error function.\n");
      ExitProcessing(-1);
      }
    }
  return(0);
}


/*-------------------------------------------------------------------*/


static void GenerateOptimalCodebookMSE(TRAININGSET*  TS,
                                CODEBOOK*     CB,
                                PARTITIONING* P)

/* Generates optimal codebook from a partitioning. */
{
  int i;

  for( i = 0; i < BookSize(CB); i++ )
    {
    if( CCFreq(P, i) > 0 )
      {
      /* Calculate mean values for centroid */
      PartitionCentroid(P, i, &Node(CB, i));
      }
    else
      {
      VectorFreq(CB, i) = 0;
      }
    }
}


/*-------------------------------------------------------------------*/


void GenerateOptimalCodebookGeneral(TRAININGSET*  TS,
                                    CODEBOOK*     CB,
                                    PARTITIONING* P,
                                    ERRORFTYPE    errorf)
/* Generates optimal codebook from a partitioning. */
{
    switch(errorf)
      {
      case MSE:
  {
  GenerateOptimalCodebookMSE(TS, CB, P);
  break;
  }
      case SC:
      case JACCARDERROR:
      case DICEERROR:
  {
  GenerateOptimalCodebookMSE(TS, CB, P);
  break;
  }
      case DBI:
  {
  GenerateOptimalCodebookMSE(TS, CB, P);
  break;
  }
      default:
  {
        ErrorMessage(
          "cb.c::GenerateOptimalCodebookGeneral: Unknown error function.\n");
        ExitProcessing(FATAL_ERROR);
  }
      }
}

/*-------------------------------------------------------------------*/


void LocalRepartitioningGeneral(TRAININGSET*  TS,
                                CODEBOOK*     CB,
                                PARTITIONING* P,
                                int           clusterindex,
                                DISTANCETYPE  disttype)
{
  int   i,j;
  llong error;
  int   new;

  i = FirstVector(P,clusterindex);
  while (!EndOfPartition(i))
    {
    new = FindNearestVector(&Node(TS,i), CB, &error, Map(P,i),disttype);
    j   = i;
    i   = NextVector(P,i);

    if (new != clusterindex)
        ChangePartition(TS, P, new, j);
    }
}


/*-------------------------------------------------------------------*/


void RepartitionDueToNewVectorsGeneral(TRAININGSET*  TS,
                                       CODEBOOK*     CB,
                                       PARTITIONING* P,
                                       int*          codevectorindices,
                                       int           codevectorcount,
                                       DISTANCETYPE  disttype)
{
  int   i, j, best;
  llong olddist, newdist;

  for ( i=0; i<BookSize(TS); i++ )
    {
    best = -1;
    olddist = VectorDistance(Vector(CB,Map(P,i)),
                             Vector(TS,i),
                             VectorSize(TS),
                             MAXLLONG,
                             disttype);

    for ( j=0; j<codevectorcount; j++)
      {
      newdist = VectorDistance(Vector(CB,codevectorindices[j]),
                               Vector(TS,i),
                               VectorSize(TS),
                               olddist,
                               disttype);
      if ( newdist < olddist )
        {
        best = codevectorindices[j];
        olddist = newdist;
        }
      }

    if ( best > -1 )
      ChangePartition(TS, P, best, i);
    }
}


/*-------------------------------------------------------------------*/


void RepartitionDueToNewVectorGeneral(TRAININGSET*  TS,
                                      CODEBOOK*     CB,
                                      PARTITIONING* P,
                                      int           codevectorindex,
                                      DISTANCETYPE  disttype)
{
  int index;

  index = codevectorindex;

  RepartitionDueToNewVectorsGeneral(TS, CB, P, &index, 1, disttype);
}


/*====================  Error calculations  ==========================*/


static double AverageErrorForSolutionMeanError(TRAININGSET*  TS,
                                               CODEBOOK*     CB,
                                               PARTITIONING* P,
                                               DISTANCETYPE  disttype)
{
  int   i;
  llong totalerror = 0;

  for ( i=0; i<BookSize(TS); i++ )
      {
      totalerror+=VectorDistance(Vector(CB,Map(P,i)),
                                 Vector(TS,i),
                                 VectorSize(TS),
                                 MAXLLONG,
                                 disttype)   * VectorFreq(TS,i);
      }

  return (double) totalerror / (double) (TotalFreq(TS) * VectorSize(TS));

}


/*-------------------------------------------------------------------*/


static int SCcs(PARTITIONING* P, int i)
{
  return CCFreq(P, i-1);
}


/*-------------------------------------------------------------------*/


static int SCfr(TRAININGSET* TS, PARTITIONING* P, int i, int j)
{
  int k;
  int res = 0;

  for ( k = FirstVector(P, i-1); ! EndOfPartition(k); k = NextVector(P, k) )
    {
    if ( VectorScalar(TS, k, j-1) > 0)
      res += VectorFreq(TS, k);
    }


  return res;
}



/*-------------------------------------------------------------------*/


static double AverageErrorForSolutionSC(TRAININGSET*  TS,
                                        CODEBOOK*     CB,
                                        PARTITIONING* P)
{
  int    i, j, m;
  double SC1 = 0.0;
  double SC2 = 0.0;
  int    limit;
  int SCcsPi, SCfrTSPij;

  /* SC1 */

  for ( j=2; j <= TotalFreq(TS); j++ )
    SC1 += log2(j);

  for ( i=1; i <= BookSize(CB); i++ )
    {
    limit = SCcs(P,i);
    for ( j=2; j <= limit; j++ )
      SC1 -= log2(j);
    }

  for ( i=TotalFreq(TS); i <= TotalFreq(TS)+BookSize(CB)-1; i++ )
    SC1 += log2(i);

  for ( i=2; i <= BookSize(CB)-1; i++ )
    SC1 -= log2(i);

  /* SC2 */

  for ( i=1; i <= BookSize(CB); i++ )
    {
    SCcsPi = SCcs(P,i);
    for ( j=1; j <= VectorSize(TS); j++ )
      {
      SCfrTSPij = SCfr(TS,P,i,j);
      limit = SCcsPi+1;
      for ( m=2; m <= limit; m++ )
        SC2 += log2(m);

      limit = SCfrTSPij;
      for ( m=2; m <= limit; m++ )
        SC2 -= log2(m);

      limit = SCcsPi-SCfrTSPij;
      for ( m=2; m <= limit; m++ )
        SC2 -= log2(m);
      }
    }

  /* SC */

  return (SC1 + SC2) / TotalFreq(TS);

}


/*-------------------------------------------------------------------*/


double AverageErrorForSolution(TRAININGSET*  TS,
                               CODEBOOK*     CB,
                               PARTITIONING* P,
                               ERRORFTYPE    errorf)

/* Calculates the error of partitioning using given codevectors.
   It might be a good idea to call GenerateOptimalCodebook first. */
{
  switch (errorf)
    {
    case MSE:
      {
      return AverageErrorForSolutionMeanError(TS, CB, P, EUCLIDEANSQ);
      }
    case SC:
      {
      return AverageErrorForSolutionSC(TS, CB, P);
      }
    case JACCARDERROR:
      {
      return AverageErrorForSolutionMeanError(TS, CB, P, JACCARD);
      }
    case DICEERROR:
      {
      return AverageErrorForSolutionMeanError(TS, CB, P, DICE);
      }
    default:
      {
      ErrorMessage("ERROR: Unknown error function.\n");
      ExitProcessing(-1);
      }
    }
  return(0);
}


/*-------------------------------------------------------------------*/


double AverageErrorForPartitioning(TRAININGSET*  TS,
                                   PARTITIONING* P,
                                   ERRORFTYPE    errorf)
{
  CODEBOOK cb;
  double   error;

  CreateNewCodebook(&cb, P->PartitionCount, TS);
  GenerateOptimalCodebookGeneral(TS, &cb, P, errorf);

  error = AverageErrorForSolution(TS, &cb, P, errorf);

  FreeCodebook(&cb);

  return error;

}


/*-------------------------------------------------------------------*/


static double AverageErrorCBFastMeanError(TRAININGSET*  TS,
                                          CODEBOOK*     CB,
                                          PARTITIONING* P,
                                          DISTANCETYPE  disttype)
{
  llong     error;
  llong     totalerror = 0;
  int       i;
  YESNO     usefreqs = ( RecognizeCBFileType(TS) != RecognizeCBFileType(CB) );

  /* Find mapping from training vector to code vector */
  for(i = 0; i < BookSize(TS); i++)
    {
    FindNearestVector( &Node(TS,i),
                       CB,
                       &error,
                       (P==NULL)? 0 : Map(P,i),
                       disttype);
    totalerror += error * (usefreqs? VectorFreq(TS,i) : 1);
    }

  return (double) totalerror /
         (double) ( (usefreqs? TotalFreq(TS) : BookSize(TS)) * VectorSize(TS));

}


/*-------------------------------------------------------------------*/


static double AverageErrorCBFastSC(TRAININGSET*  TS,
                                   CODEBOOK*     CB)
{
  /* not really fast, though */

  double res;

  PARTITIONING P;

  CreateNewPartitioning(&P, TS, BookSize(CB));
  res = GenerateOptimalPartitioningGeneral(TS, CB, &P, SC);

  FreePartitioning(&P);

  return res;
}


/*-------------------------------------------------------------------*/


double AverageErrorCBFast(TRAININGSET*  TS,
                          CODEBOOK*     CB,
                          PARTITIONING* P,
                          ERRORFTYPE    errorf)
{
  switch (errorf)
    {
    case MSE:
      {
      return AverageErrorCBFastMeanError(TS, CB, P, EUCLIDEANSQ);
      }
    case SC:
      {
      return AverageErrorCBFastSC(TS, CB);
      }
    case JACCARDERROR:
      {
      return AverageErrorCBFastMeanError(TS, CB, P, JACCARD);
      }
    case DICEERROR:
      {
      return AverageErrorCBFastMeanError(TS, CB, P, DICE);
      }
    default:
      {
      ErrorMessage("ERROR: Unknown error function.\n");
      ExitProcessing(-1);
      }
    }
  return(0);
}


/*-------------------------------------------------------------------*/


double PrintableError(double error, CODEBOOK* CB)
{
  if (CB->MaxValue == 1)
    {
    return  error * VectorSize(CB);
    }
  else
    {
    return  error;
    }
}


/*-------------------------------------------------------------------*/


DISTANCETYPE DistType(ERRORFTYPE errorf)
{
  switch (errorf)
    {
    case MSE:
      {
      return EUCLIDEANSQ;
      }
    case SC:
      {
      return ENTROPYDIST;
      }
    case JACCARDERROR:
      {
      return JACCARD;
      }
    case DICEERROR:
      {
      return DICE;
      }
  case DBI: return EUCLIDEANSQ;
    default:
      {
      ErrorMessage("ERROR: Unknown error function %d (DistType).\n", (int) errorf);
      ExitProcessing(-1);
      }
    }
  return(0);
}


/*====================  File type determination  ====================*/


CBFILETYPE RecognizeVersionstr(char* s)
{
  if( strncmp(CodebookID, s, strlen(CodebookID)) == 0 )
    {
    return(CBFILE);
    }
  if( strncmp(TrainingSetID, s, strlen(TrainingSetID)) == 0 )
    {
    return(TSFILE);
    }
  if( strncmp(PartitioningID, s, strlen(PartitioningID)) == 0 )
    {
    return(PAFILE);
    }
  return(NOTFOUND);
}


/*-------------------------------------------------------------------*/


static CBFILETYPE AnalyzeFileType(char* FileName)
{
  FILE*  f;
  char   s[MaxVersionLength+1];

  if(!ExistFile(FileName))
    {
    return(NOTFOUND);
    }
  f = FileOpen(FileName, INPUT, NO);
  fgets(s, MaxVersionLength, f);
  fclose(f);

  return RecognizeVersionstr(s);
}


/*-------------------------------------------------------------------*/


CBFILETYPE DetermineCBFileTypeConsideringOrder(char* FileName, int order)
/* Determines the type of the input file (TS/CB/PA) and returns
   the result. If no extension is given in the file name, the
   routine tries each possibility and chooses the one that matches. */
{
  CBFILETYPE filetype;
  char       s[MaxVersionLength+1];
  int        i = 0;
  char       orderstring[10];

  /* Without extension */
  strcpy(s, FileName);
  filetype = AnalyzeFileType(s);
  if(filetype!=NOTFOUND)
    {
    return(filetype);
    }

  sprintf(orderstring,"%d",order);

  while ( orderstring[i] )
    {
    switch (orderstring[i] - '0')
      {
      case 1:
        {
        /* Try TS-file extension */
        strcpy(s, FileName);
        CheckFileName(s, FormatNameTS);
        filetype = AnalyzeFileType(s);
        if(filetype!=NOTFOUND)
          {
          strcpy(FileName, s);
          return(filetype);
          }
        break;
        }
      case 2:
        {
        /* Try CB-file extension */
        strcpy(s, FileName);
        CheckFileName(s, FormatNameCB);
        filetype = AnalyzeFileType(s);
        if(filetype!=NOTFOUND)
          {
          strcpy(FileName, s);
          return(filetype);
          }
        break;
        }
      case 3:
        {
        /* Try PA-file extension */
        strcpy(s, FileName);
        CheckFileName(s, FormatNamePA);
        filetype = AnalyzeFileType(s);
        if(filetype!=NOTFOUND)
          {
          strcpy(FileName, s);
          return(filetype);
          }
        break;
        }
      default:
        {
        ErrorMessage("ERROR: invalid priority order in ",
                     "DetermineCBFileTypeConsideringOrder: %d\n", order);
        ExitProcessing(-1);
        }
      }
    i++;
    }

  /* No luck */
  return(NOTFOUND);
}

/* ----------------------------------------------------------------- */
/*            Graph API here                                         */
/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */



#define LINEBUF 1024

/* ----------------------------------------------------------------- */

int GraphGetK(Graph *g)
{
  return g->k;
}

/* ----------------------------------------------------------------- */

int GraphPutK(Graph *g, int k)
{
  g->k = k;

  return 0;
}

/* ----------------------------------------------------------------- */

int GraphGetNumberVectors(Graph *g)
{
  return g->nvec;
}

/* ----------------------------------------------------------------- */

int GraphPutNumberVectors(Graph *g, int nvec)
{
  g->nvec = nvec; 

  return 0;
}

/* ----------------------------------------------------------------- */

int GraphGetDim(Graph *g)
{
  return g->dim;
}

/* ----------------------------------------------------------------- */

int GraphPutDim(Graph *g, int dim)
{
  g->dim = dim; 

  return 0;
}

/* ----------------------------------------------------------------- */

int GraphGetVectorIndex(GraphVector *v)
{
  return v->index;
}

/* ----------------------------------------------------------------- */

int *GraphGetVectorCoord(GraphVector *v)
{
  return v->data;
}

/* ----------------------------------------------------------------- */

GraphVector *GraphGetVector(Graph *g, int i)
{
  return g->vectors[i];
}

/* ----------------------------------------------------------------- */

GraphVector *GraphGetNNVector(Graph *g, GraphVector *v, int i)
{
  if ((i < g->k) && (GraphGetVector(g,v->kindices[i]) != NULL))
    return GraphGetVector(g, v->kindices[i]);
  else 
    return NULL;
}

/* ----------------------------------------------------------------- */

int GraphPutNN(Graph *g, GraphVector *v, int index, int i)
{
  if (i < g->k) {
    v->kindices[i] = index;
    return 0;
 } else 
    return 1;
}

/* ----------------------------------------------------------------- */

int IsGraphEdge(Graph *g, GraphVector *src, GraphVector *dest)
{
  int i;

  for (i = 0; i < g->k; i++) {
    if (src->kindices[i] == dest->index)
      return 1;
  }

  return 0;
}

/* ----------------------------------------------------------------- */

int GraphPutVector(Graph *g, int i, int *data)
{
  GraphVector *v;

  v = (GraphVector *) allocate(sizeof(GraphVector));
  if (v == NULL)
    return 1;
  v->data = (int *) allocate(sizeof(int)*g->dim);
  if (v->data == NULL)
    return 1;
  v->kindices = (int *) allocate(sizeof(int)*g->k);
  if (v->kindices == NULL)
    return 1;

  v->data = memcpy(v->data, data, sizeof(int)*g->dim);

  g->vectors[i] = v;
  v->index = i; 

  return 0;
}

/* ----------------------------------------------------------------- */

int GraphPutVectorElement(GraphVector *v, int j, int data)
{
  v->data[j] = data;

  return 0;
}

/* ----------------------------------------------------------------- */

int FreeGraphVector(Graph *g, int i)
{
  if (g->vectors[i] == NULL)
    return 1;

  free(g->vectors[i]->data);
  free(g->vectors[i]->kindices);
  free(g->vectors[i]);

  return 0;
}

/* ----------------------------------------------------------------- */

int FreeGraph(Graph *g)
{
  int i;

  for (i = 0; i < g->nvec; i++) {
    if (FreeGraphVector(g, i) == 1) {
      free(g->vectors);
      free(g);
    }      
  }

  free(g->vectors);
  free(g);
  
  return 0;
}

/* ----------------------------------------------------------------- */

Graph *ReadGraphFileHeader(FILE *f)
{
  int nvec, k, dim, i;
  Graph *g;

  fscanf(f, "Graph v. %i", &i);  // Read version
  fscanf(f, "%i\n", &nvec);
  fscanf(f, "%i\n", &k);
  fscanf(f, "%i\n", &dim);

  g = AllocateMemoryForGraph(nvec, k, dim); 

  while (getc(f) == '-' )             // Read dashes and '\n'
    ;       

  return g;
}

/* ----------------------------------------------------------------- */

int WriteGraphkFileHeader(FILE *f, Graph *g)
{

  fprintf(f, "Graph v. %d\n", 1);
  fprintf(f, "%d\n", g->nvec);
  fprintf(f, "%d\n", g->k);
  fprintf(f, "%d\n", g->dim);

  fprintf(f, "-------------------------------------");
  putc(10, f);

  return 0;
}

/* ----------------------------------------------------------------- */

Graph *AllocateMemoryForGraph(int number_vectors, int k, int dim) 
{
  Graph *g;

  g = (Graph *) allocate(sizeof(Graph));
  if (g == NULL)
    return NULL;

  g->nvec = number_vectors;
  g->k = k;
  g->dim = dim;
  g->maxcoord = 0;
  g->maxcoord = 0;

  g->vectors = (GraphVector **) allocate(sizeof(GraphVector) * 
                       GraphGetNumberVectors(g));
  if (g->vectors == NULL) {
    ErrorMessage("ERROR: Allocate error: AllocateMemoryForGraph(Graph)\n");
    return NULL;
  }

  return g;
}

/* ----------------------------------------------------------------- */

GraphVector *AllocateMemoryForGraphVector(Graph *g, int index) 
{
  GraphVector *v;

  v = (GraphVector *) allocate(sizeof(GraphVector));
  v->data = (int *) allocate(sizeof(int)*GraphGetDim(g));
  v->kindices = (int *) allocate(sizeof(int)*GraphGetK(g));
  v->index = index;

  if (v == NULL || v->data == NULL || v->kindices == NULL) {
    ErrorMessage("ERROR: Allocate error: AllocateMemoryForGraphVector()\n");
    return NULL;
  }

  return v;
}

/* ----------------------------------------------------------------- */

int readline(FILE *infile, char *buf)
{
  if ((fgets(buf, LINEBUF, infile)) == NULL) {
    return 1;  
  }

 return feof(infile);
}

/* ----------------------------------------------------------------- */

GraphVector *ReadGraphVectorFromFile(FILE *f, Graph *g, int index)
{
  int i = 0, j = 0;
  char *buffer, *temp;
  GraphVector *v;

  buffer = (char *) allocate(sizeof(char)*LINEBUF);

  temp = buffer;

  if (readline(f, buffer)) {
    free(temp);
    return 0;
  }

  v = AllocateMemoryForGraphVector(g, index);
  
  while (*buffer) {
    while (*buffer && isspace(*buffer)) buffer++;
    if (*buffer) {
      if (i < GraphGetDim(g)) {
	v->data[i++] = (int) strtol(buffer, &buffer, 10);
      } else 
	v->kindices[j++] = (int) strtol(buffer, &buffer, 10);
      buffer++;
    }
  }

  free(temp);

  return v;
}

/* ----------------------------------------------------------------- */

int WriteGraphVectorToFile(FILE *f, Graph *g, GraphVector *v)
{
  int i; 


  for (i = 0; i < g->dim; i++) {
    fprintf(f, "%d ",v->data[i]); 
  }
  for (i = 0; i < g->k; i++) {
    fprintf(f, "%d ",v->kindices[i]); 
  }
  fprintf(f, "\n");

  return 0; 
}

/* ----------------------------------------------------------------- */

int IsGraphFile(char* FileName)
{
  FILE *f;

  f = FileOpen(FileName, INPUT, NO);

  if (ReadGraphFileHeader(f)) {
    fclose(f);
    return 1;
  }

   fclose(f);
   return 0;
}

/* ----------------------------------------------------------------- */

Graph *GraphRead(char* FileName)
{
  int   i, j, min = INT_MAX, max = 0;
  FILE *f;
  Graph *g;
  int *data; 

  f = FileOpen(FileName, INPUT, NO);
  g = ReadGraphFileHeader(f);
  if (g == NULL)
    return NULL; 

  for (i=0; i< GraphGetNumberVectors(g); i++) {
    g->vectors[i] = ReadGraphVectorFromFile(f, g, i);
  }

  fclose(f);

  for (i=0; i< GraphGetNumberVectors(g); i++) {
    data = GraphGetVectorCoord(g->vectors[i]);
    for (j = 0; j < GraphGetDim(g); j++) {
      if (data[j] < min)
    min = data[j];
      if (data[j] > max)
    max = data[j];
    }
  }

  g->maxcoord = max;
  g->mincoord = min;

  return g;
}

/* ----------------------------------------------------------------- */

void GraphWrite(char* FileName, Graph *g,  int AllowOverWrite)
{
  int   i;
  FILE* f;

  f = FileOpen(FileName, OUTPUT, AllowOverWrite);
  WriteGraphkFileHeader(f, g);
 
  for(i=0; i<g->nvec; i++) {
    WriteGraphVectorToFile(f, g, GraphGetVector(g, i));
  }

  fclose(f);
}

/* ----------------------------------------------------------------- */

int GraphPrint(Graph *g)
{
  int i; 

  for (i = 0; i < g->nvec; i++) 
    WriteGraphVectorToFile(stdout, g, GraphGetVector(g, i));
 

  return 0;
}

/* ----------------------------------------------------------------- */

int GraphGetMaxCoord(Graph *g)
{
  return g->maxcoord;
}

/* ----------------------------------------------------------------- */

int GraphGetMinCoord(Graph *g)
{
  return g->mincoord;
}
/* ----------------------------------------------------------------- */


