/*
$Revision: 1.3 $
$Date: 2005/06/27 10:58:00 $
$Author: mtuonone $
$Name:  $
$Id: textfile.c,v 1.3 2005/06/27 10:58:00 mtuonone Exp $
*/

/*********************************************************

    TEXTFILE.C

    Ismo Kärkkäinen

    24.9.2001

    For dealing with ASCII-data.

********************************************************/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>

#include <math.h>
#include <float.h>


#include "textfile.h"
#include "memctrl.h"
#include "fvec.h"

#include "cb.h"
#include "interfc.h"


static int BufSize = 1024;
static char* Buffer = 0;

char* ReadTextLine(FILE* F) {
    fpos_t Position;
    if (feof(F)) return 0;
    if (!Buffer) Buffer = malloc(BufSize);
    fgetpos(F, &Position); /* if need to reread the line */
    do {
    if (!fgets(Buffer, BufSize, F)) return 0;
    if (Buffer[strlen(Buffer) - 1] != '\n' && !feof(F)) {
        fsetpos(F, &Position);
        free(Buffer);
        BufSize *= 2;
        Buffer = malloc(BufSize);
        *Buffer = 0;
    }
    } while (!*Buffer);
    return Buffer;
}

char* ReadTextFile(const char* Filename) {
    FILE* F;
    char* Result;
    struct stat FileInfo;
    if (stat(Filename, &FileInfo)) return 0;
    F = fopen(Filename, "rt");
    if (!F) return 0;
    Result = allocate(FileInfo.st_size + 1);
    Result[FileInfo.st_size] = 0;
    if (!fread(Result, 1, FileInfo.st_size, F)) {
    deallocate(Result);
    fclose(F);
    return 0;
    }
    fclose(F);
    return Result;
}

char** SplitToLines(const char* StringWithNewlines, int* Lines) {
    char** Result;
    int LineCount = 0;
    int LineLength;
    const char* Curr;
    const char* Begin;
    Curr = StringWithNewlines;
    while (*Curr) if (*Curr++ == '\n') LineCount++;
    --Curr;
    if (*Curr != '\n') LineCount++;
    if (Lines) *Lines = LineCount;
    Result = allocate(sizeof(char*) * LineCount + 1);
    Result[LineCount] = 0;
    LineCount = 0;
    Curr = StringWithNewlines;
    while (*Curr) {
    LineLength = 0;
    Begin = Curr;
    while (*Curr && *Curr != '\n') {
        Curr++;
        LineLength++;
    }
    Result[LineCount] = allocate(LineLength + 1);
    strncpy(Result[LineCount], Begin, LineLength);
    Result[LineCount++][LineLength] = 0;
    if (*Curr) Curr++;
    }
    return Result;
}

char** ReadTextFileLines(const char* Filename, int* Lines) {
    char** Result;
    char* Contents = ReadTextFile(Filename);
    if (!Contents) return 0;
    Result = SplitToLines(Contents, Lines);
    deallocate(Contents);
    return Result;
}


static int BufVectorSize = 1024;
static double* BufferVector = 0;

double* ReadRawVector(FILE* F, int* Dimensionality,
              int* Incomplete, int* Errors)
{
    double* Larger;
    int k;
    char* Str, *Buffer;

    *Incomplete = *Errors = 0;
    if (!BufferVector) BufferVector = malloc(BufVectorSize * sizeof(double));
    if (*Dimensionality < 1) *Dimensionality = 1;
    do {
        Buffer = Str = ReadTextLine(F);
        if (!Str) return 0;
        while (*Str && isspace((int)*Str)) Str++; /* move to first non-space */
        if (*Str == '%' || *Str == '#') *Str = 0; /* jump over comments */
    } while (!*Str); /* find line with text */
    Str = Buffer;
    for (k = 0; *Str; ++k) {
        if (k == BufVectorSize) {
            Larger = malloc((BufVectorSize * 2) * sizeof(double));
            memcpy(Larger, BufferVector, BufVectorSize * sizeof(double));
            free(BufferVector);
            BufferVector = Larger;
            BufVectorSize *= 2;
        }
        BufferVector[k] = strtod(Str, &Str);
        if (*Str && !isspace((int)*Str)) {
            *Errors = 1;
            return 0;
        }
        while (*Str && isspace((int)*Str)) Str++;
    }
    *Incomplete = (k < *Dimensionality) || strstr(Buffer, "NaN");
    *Dimensionality = k;
    return BufferVector;
}

int GetRawFileInfo(char* Filename,
                   int IgnoreIncomplete, int FrequencyLocation,
                   int QuietLevel, double* Minimum, double* Maximum,
                   int* Dimensionality, int* Count, int* HasIncomplete,
                   int* HasErrors, int* FrequenciesOK, int* HasHeader,
                   int* TotalFrequency)
{
    double* Vector;
    int MinMaxSet = 0;
    int k, LatestDim, Incomplete, Errors;
    FILE* F = fopen(Filename, "rt");
    *Minimum = *Maximum = 0;
    LatestDim = *Dimensionality = *Count = 0;
    *HasIncomplete = *HasErrors = *TotalFrequency = 0;
    *FrequenciesOK = 1;
    if (!F) return 0;
    *HasHeader = 0;
    while (!feof(F)) {
        Vector = ReadRawVector(F, &LatestDim, &Incomplete, &Errors);
        (*Count)++;
        if (LatestDim > *Dimensionality) {
            *Dimensionality = LatestDim;
            MinMaxSet = 0;
            *Count = 1;
        }
        *HasIncomplete = *HasIncomplete || Incomplete;
        if (!Vector) {
            fclose(F);
            if (FrequencyLocation) *Dimensionality -= 1;
            *HasErrors = *HasErrors || Errors;
            if (Errors) return 0;
            return 1;
        }
        if (Errors) {
            *HasErrors = 1;
            return 0;
        }
        if (Incomplete && !IgnoreIncomplete) return 0;
        if (!Incomplete) for (k = 0; k < LatestDim; k++) {
            if (((k == 0) && (FrequencyLocation == 1)) ||
                ((k == LatestDim - 1) && (FrequencyLocation == 2)))
            {
                if (((int)Vector[k]) - Vector[k]) {
                    *FrequenciesOK = 0;
                    return 0;
                } else *TotalFrequency += (int)Vector[k];
            } else {
                if (!MinMaxSet) {
                    *Minimum = *Maximum = Vector[k];
                    MinMaxSet = 1;
                } else if (Vector[k] < *Minimum) *Minimum = Vector[k];
                else if (Vector[k] > *Maximum) *Maximum = Vector[k];
            }
        } else (*Count)--;
    }
    fclose(F);
    if (FrequencyLocation) *Dimensionality -= 1;
    return 1;
}

int DetermineMaxval(float **Data, int count, int dim, int *bytes) {
    int maxval;
    float val = 0;

    switch (*bytes) {
        case 1: maxval = MAX_1BYTE; break;
        case 2: maxval = MAX_2BYTE; break;
        case 3: maxval = MAX_3BYTE; break;

        default:    /* automatic choice */
            val = fvSetTotalMaximum(Data, count, dim);
            val = val - fvSetTotalMinimum(Data, count, dim);

            if (val <= MAX_1BYTE) {
                maxval = MAX_1BYTE;
                *bytes = 1;
            } else if (val <= MAX_2BYTE) {
                maxval = MAX_2BYTE;
                *bytes = 2;
            } else {
                maxval = MAX_3BYTE;
                *bytes = 3;
            }

            break;
    }
    return maxval;
}


void ReadCodebookTXT(char* FileName, CODEBOOK* CB)
{
  int   i;
  FILE* f;
  int   totalfreq; //TODO:?

  int dim = 0; int ok = 0; int count = 0;
  float **Data;

  //ok = ReadInputData(&Data, &count, &dim, FileName, Value(QuietLevel));
  ok = ReadInputData(&Data, &count, &dim, FileName, 4); //TODO: quietlevel

  if (!ok)
  {
      ErrorMessage("\nERROR: Failed to read input text file: %s\n\n", FileName);
      ExitProcessing(FATAL_ERROR);
  }
  CB->MinMax = FindMinMax(Data, count, dim, 4);

  Data2CB(Data, CB, count, dim, CB->MinMax,
          //0 /*automatic*/, 1 /* scaling*/, 4 /*ql*/);
          3 /* 3 byte */, 1 /* scaling*/, 4 /*ql*/);
          // 9 /* INT_MAX */, 1 /* scaling*/, 4 /*ql*/);

  CB->InputFormat=TXT;
}


// From txt2cb.c:WriteData2CB
void Data2CB(float **Data, CODEBOOK* CB,
        int count, int dim, float **MinMax,  int bytes,
        int scaling, int ql) {
    int maxval, i, j;
    float scale, totalMin, totalMax;
    //char GenMethod[MAXFILENAME+1] = "\0";
    char GenMethod[1000] = "\0"; //TODO: constants.h ?

    //TODO: needed?:
    maxval = DetermineMaxval(Data, count, dim, &bytes);
    //maxval = 2147483;

    //maxval = GetMaxval(CB);

    /* is it possible not to scale? */
    if (!scaling) {
        totalMin = fvSetTotalMinimum(Data, count, dim);
        totalMax = fvSetTotalMaximum(Data, count, dim);

        if ((totalMax > maxval) || (totalMin < 0)) {
            ErrorMessage("ERROR: Cannot save vectors without scaling!\n");
            ExitProcessing(FATAL_ERROR);
        }
    }

    /* saving minmax name to GenerationMethod-field, if needed */
    if (scaling >= 2) {
        //strcpy(GenMethod, MinMaxName);
        //GenMethod[strlen(GenMethod)]   = MINMAX_FILENAME_SEPARATOR;
        //GenMethod[strlen(GenMethod)+1] = '\0';
        //strcat(GenMethod, ProgName);
    } else {
        //strcpy(GenMethod, ProgName);
    }

    CreateNewTrainingSet(CB, count, dim, 1, bytes, 0, maxval, 0, GenMethod);

    /* set all vector frequencies to 1 */
    for (i = 0; i < count; i++) {
        VectorFreq(CB, i) = 1;
    }

    for (j = 0; j < dim; j++) {
        if (scaling) {
            if (MinMax[j][1] - MinMax[j][0] < FLT_EPSILON)
                scale = 0.0F;
            else
                scale = maxval / (MinMax[j][1] - MinMax[j][0]);

            for (i = 0; i < count; i++) {
                /* scaling attributes to interval [0,maxval] */
                VectorScalar(CB, i, j) = ROUND((Data[i][j] - MinMax[j][0]) * scale);
            }

        } else {
            for (i = 0; i < count; i++) {
                /* no scaling at all; just rounding, if needed */
                VectorScalar(CB, i, j) = ROUND(Data[i][j]);
            }
        }
    }
    CB->Preprocessing = scaling;

    TotalFreq(CB) = count;
    //WriteCodebook(OutName, &CB, 1);
    //FreeCodebook(&CB);

    if (ql) {
        //PrintMessage("\nVectors were saved to output codebook %s\n", OutName);
        if (scaling) {
            PrintMessage("All attributes were scaled to range [0,%d]\n", maxval);
        } else {
            PrintMessage("All attributes were rounded to integers and no scaling was done.\n");
        }
    }
    //return CB;
}

float **FindMinMax(float **Data, int count, int dim, int ql) {
    int j;
    float **MinMax;

    MinMax = fvNewSet(dim, 2);

    for (j = 0; j < dim; j++) {
        MinMax[j][0] = fvSetMinimum(Data, count, j);
        MinMax[j][1] = fvSetMaximum(Data, count, j);

        //if (Value(QuietLevel) > 1)
        PrintMessage("Range for %d. attribute is [%g,%g]\n",
                j+1, MinMax[j][0], MinMax[j][1]);
    }

    return MinMax;
}

int ReadInputData(float ***Data, int *count, int *dim, char *InName, int ql) {
    FILE *f;
    int ok;

    f = fopen(InName, "rt");
    ok = fvReadSet(Data, count, dim, f);
    fclose(f);

    if (ok && ql) {
        if (ok-1) {
            PrintMessage("WARNING: %d vectors had missing ", ok-1);
            PrintMessage("attributes and they were ignored.\n\n");
        }
        PrintMessage("Input text file %s contained ", InName);
        PrintMessage("%i vectors of dimensionality %i\n", *count, *dim);
    }

    return ok;
}

int SaveCB2TXT(CODEBOOK *CB, char *OutName,
        float **MinMax, int freq, int prec) {
    int i, j, k, n, ok, maxval, maxlen, maxprec;
    int precArr[VectorSize(CB)];
    float **tmp;
    FILE *f;

    if (freq) {
        tmp = fvNewSet(TotalFreq(CB), VectorSize(CB));

    } else {
        tmp = fvNewSet(BookSize(CB), VectorSize(CB));
    }

    maxval  = GetMaxval(CB);
    maxprec = CalcPrec(precArr, MinMax, VectorSize(CB), maxval, prec);
    maxlen  = CalcMaxlen(MinMax, VectorSize(CB), maxval, maxprec);

    n = 0;
    for (i = 0; i < BookSize(CB); i++) {
        k = 0;
        do {
            for (j = 0; j < VectorSize(CB); j++) {
                /* we do descaling and denormalizing
                   (if original range information given) */
                //tmp[n][j] = DeScale(CB, i, j, MinMaxName, MinMax, maxval);
                tmp[n][j] = DeScale(CB, i, j, MinMax, maxval);
                /* PrintMessage("%d %d: %f\n", n, j, tmp[n][j]); */
            }
            n++;
            k++;
        } while ((k < VectorFreq(CB, i)) && freq);
    }

    /* we save data to text file */
    f = fopen(OutName, "w");

    ok = !fvWriteSetWithPrec(tmp, n, VectorSize(CB), f, maxlen, precArr);
    //ok = !fvWriteSetWithPrec(tmp, n, VectorSize(CB), f, maxlen, NULL);
    fclose(f);
    fvDeleteSet(tmp, n);

    return ok;
}

int GetMaxval(CODEBOOK *CB) {
    int maxval;

    //return 2147483; //TODO
    switch (CB->BytesPerElement) {
        //TODO
        case 9 : maxval = INT_MAX; break;
        case 1 : maxval = MAX_1BYTE; break;
        case 2 : maxval = MAX_2BYTE; break;
        case 3 : maxval = MAX_3BYTE; break;
        default:
            maxval = 0;
            ErrorMessage("\nERROR: CB->BytesPerElement value unsupported.\n\n");
            ExitProcessing(FATAL_ERROR);
            break;
    }

    return maxval;
}


int CalcPrec(int *precArr, float **MinMax, int dim,
int maxval, int prec) {
    int i, tmp, maxprec = -1;
    double resolution;

    if (MinMax && !prec) {
        for (i = 0; i < dim; i++) {
            resolution  = (MinMax[i][1] - MinMax[i][0]) / maxval;
            tmp         = (int)log10(resolution);

            if (tmp < 0)
                precArr[i] = 1 - tmp;
            else
                precArr[i] = 0;

            if (precArr[i] > maxprec)
                maxprec = precArr[i];
        }

    } else {
        maxprec = prec;
        for (i = 0; i < dim; i++)
            precArr[i] = prec;
    }

    return maxprec;
}


int CalcMaxlen(float **MinMax, int dim, int maxval,
int prec) {
    int i;
    float max = FLT_MIN;

    if (MinMax) {
        for (i = 0; i < dim; i++) {
            if (MinMax[i][1] > max)
                max = MinMax[i][1];
        }
    } else {
        max = maxval;
    }

    return (int)log10((double)max) + prec + 4;
}




float DeScale(CODEBOOK *CB, int i, int j,  float **MinMax, int maxval) {
    float val, scale;

    if (MinMax) {
        scale = (MinMax[j][1] - MinMax[j][0]) / maxval;
        val   = MinMax[j][0] + VectorScalar(CB, i, j)*scale;

    } else {
        val   = VectorScalar(CB, i, j);
    }

    return val;
}


