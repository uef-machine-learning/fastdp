/*
$Revision: 1.3 $
$Date: 2005/06/27 10:58:00 $
$Author: mtuonone $
$Name:  $
$Id: textfile.h,v 1.3 2005/06/27 10:58:00 mtuonone Exp $
*/

#ifndef TEXTFILE_H
#define TEXTFILE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "cb.h"

char* ReadTextLine(FILE* F);
char* ReadTextFile(const char* Filename);
char** SplitToLines(const char* EntireFile, int* Lines);
char** ReadTextFileLines(const char* Filename, int* Lines);

double* ReadRawVector(FILE* F, int* Dimensionality,
              int* Incomplete, int* Errors);

int GetRawFileInfo(char* Filename,
           int IgnoreIncomplete, int FrequencyLocation,
           int QuietLevel, double* Minimum, double* Maximum,
           int* Dimensionality, int* Count, int* HasIncomplete,
           int* HasErrors, int* FrequenciesOK, int* HasHeader,
           int* TotalFrequency);

int DetermineMaxval(float **Data, int count, int dim, int *bytes);

void ReadCodebookTXT(char* FileName, CODEBOOK* CB);
int ReadInputData(float ***Data, int *count, int *dim, char *InName, int ql);
float **FindMinMax(float **Data, int count, int dim, int ql);
void Data2CB(float **Data, CODEBOOK* CB, int count, int dim, float **MinMax,  int bytes, int scaling, int ql);
int SaveCB2TXT(CODEBOOK *CB, char *OutName, float **MinMax, int freq, int prec);

int GetMaxval(CODEBOOK *CB);
int CalcPrec(int *precArr, float **MinMax, int dim, int maxval, int prec);
int CalcMaxlen(float **MinMax, int dim, int maxval, int prec);
float DeScale(CODEBOOK *CB, int i, int j,  float **MinMax, int maxval);




#define MAXFILENAME 1024
#define MAX_1BYTE 255
#define MAX_2BYTE 65535
#define MAX_3BYTE 1000000
#define DEFAULT_MINMAX_FILE "minmax.txt"
#define MINMAX_FILENAME_SEPARATOR ';'

#define FormatNameTXT "txt"
#define FormatNameSC  "sc"

#define ROUND(a) ( ((a) - floor(a)) < 0.5 ? (floor(a)) : (floor(a)+1) )

#if defined(__cplusplus)
}
#endif

#endif
