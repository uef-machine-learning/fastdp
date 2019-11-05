/*
$Revision: 1.3 $
$Date: 2005/06/27 10:58:00 $
$Author: mtuonone $
$Name:  $
$Id: fvec.h,v 1.3 2005/06/27 10:58:00 mtuonone Exp $
*/
 
#ifndef FVEC_H
#define FVEC_H

#if defined(__cplusplus)
extern "C" {
#endif

#define RAND_FLOAT()   (float)rand()/(float)RAND_MAX

#include <math.h>
#include "memctrl.h"
#include <stdio.h>

#define fvNew(Dimensionality) allocate(sizeof(float) * (Dimensionality)) 
#define fvDelete(V) deallocate(V)

void fvCopy(float* From, float* To, int Dimensionality);
void fvFill(float* V, float Value, int Dimensionality);
/*void fvRandom(float* V, int Dimensionality);*/


double randn (double mu, double sigma);
float * fvRand(int dim);

float fvManhattanDistance(float* V1, float* V2, int Dimensionality);
float fvManhattanLength(float* V, int Dimensionality);

float fvDistance(float* V1, float* V2, int Dimensionality);
#define fvLength(V, D) sqrt(fvScalarProduct((V), (V), (D)))
#define fvNormalize(V, Dimensionality) \
    fvScale((V), 1.0 / fvLength((V), (Dimensionality)), Dimensionality)

void fvPrint(float* V, int Dimensionality);

float fvScalarProduct(float* V1, float* V2, int Dimensionality);
void fvAdd(float* V1, float* V2, float* Result, int Dimensionality);
void fvSubtract(float* V1, float* V2, float* Result, int Dimensionality);
void fvScale(float* V, float Scale, int Dimensionality);

void fvSort(float* V, int Dimensionality, int Ascending);

float** fvNewSet(int Count, int Dimensionality);
void fvDeleteSet(float** Set, int Count);
float** fvCopySet(float** Dest, float** Src, int Count, int Dimensionality);
int fvWriteSetWithPrec(float** Set, int Count, int Dimensionality, FILE* f, 
    int width, int *prec);
int fvWriteSet(float** Set, int Count, int Dimensionality, FILE* f);
int fvReadSet(float*** Set, int* Count, int* Dimensionality, FILE* f);

void fvFillSet(float** Set, int Count, int Dimensionality, float Value);

float** fvSubSetDimensions(float** Set, int Count, int Dimensionality,
    int* SelectedDimensions, float** SubSet, int* SubsetDimensionality);
float fvSetAverage(float** Set, int Count, int Dimension);
float fvSetNth(float** Set, int Count, int Dimension, int N);
float fvSetMinimum(float** Set, int Count, int Dimension);
float fvSetMaximum(float** Set, int Count, int Dimension);
float fvSetTotalMinimum(float** Set, int Count, int Dimensionality);
float fvSetTotalMaximum(float** Set, int Count, int Dimensionality);
int fvSetIsBinary(float** Set, int Count, int Dimensionality,
    float V1, float V2);

enum { fvAscendingOrder, fvDescendingOrder };

void fvSortSet(float** Set, int Count, int Dimensionality,
    int* DimensionOrder, int* ValueOrder, int Order);
float* fvSetUnique(float** Set, int Count, int Dimension, int* FoundCount);

#if defined(__cplusplus)
}
#endif

#endif
