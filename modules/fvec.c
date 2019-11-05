/*
$Revision: 1.3 $
$Date: 2005/06/27 10:58:00 $
$Author: mtuonone $
$Name:  $
$Id: fvec.c,v 1.3 2005/06/27 10:58:00 mtuonone Exp $
*/
 
/*******************************************************************

    FVEC.C      Ismo Karkkainen, Marko Tuononen

    Updated: 17.02.2005  MT

    Version 0.1  

    Operations for float vectors and sets of float vectors.

*******************************************************************/

#include "fvec.h"
#include "textfile.h"
#include <stdlib.h>

#define PRINT_WIDTH 32
#define PRINT_PREC  16


// From https://phoxis.org/2013/05/04/generating-random-numbers-from-normal-distribution-in-c/
// https://en.wikipedia.org/wiki/Marsaglia_polar_method
double
randn (double mu, double sigma)
{
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;
 
  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }
 
  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);
 
  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;
 
  call = !call;
 
  return (mu + sigma * (double) X1);
}


float * fvRand(int dim) {
 float * V = (float*) malloc(sizeof(float)*dim);
 for (int i=0;i<dim;i++) {
 // Uniform:
// V[i] = RAND_FLOAT()*2.0-1.0;
//Gaussian:
V[i] = randn(0,1.0);
 }
 return V;
}


void fvCopy(float* From, float* To, int Dimensionality) {
    while (Dimensionality--) *To++ = *From++;
}

void fvFill(float* V, float Value, int Dimensionality) {
    while (Dimensionality--) *V++ = Value;
}

float fvManhattanDistance(float* V1, float* V2, int Dimensionality) {
    float Dist = 0;
    while (Dimensionality--) Dist += fabs(*V1++ - *V2++);
    return Dist;
}

float fvManhattanLength(float* V, int Dimensionality) {
    float Len = 0;
    while (Dimensionality--) Len += fabs(*V++);
    return Len;
}

float fvDistance(float* V1, float* V2, int Dimensionality) {
    float Dist;
    float* tmp = fvNew(Dimensionality);
    fvSubtract(V1, V2, tmp, Dimensionality);
    Dist = fvLength(tmp, Dimensionality);
    fvDelete(tmp);
    return Dist;
}

float fvScalarProduct(float* V1, float* V2, int Dimensionality) {
    float Prod = 0;
    while (Dimensionality--) Prod += *V1++ * *V2++;
    return Prod;
}

void fvAdd(float* V1, float* V2, float* Result, int Dimensionality) {
    while (Dimensionality--) *Result++ = *V1++ + *V2++;
}

void fvSubtract(float* V1, float* V2, float* Result, int Dimensionality) {
    while (Dimensionality--) *Result++ = *V1++ - *V2++;
}

void fvScale(float* V, float Scale, int Dimensionality) {
    while (Dimensionality--) {
    *V = *V * Scale;
    V++;
    }
}

static int fv_asc = 0;

static int float_compare(const void* a, const void* b) {
    float aa, bb;
    aa = *(float*)a;
    bb = *(float*)b;
    if (fv_asc) {
        if (aa < bb) return -1;
        if (aa > bb) return 1;
    } else {
        if (aa < bb) return 1;
        if (aa > bb) return -1;
    }
    return 0;
}

void fvSort(float* V, int Dimensionality, int Ascending) {
    fv_asc = Ascending;
    qsort(V, Dimensionality, sizeof(float), float_compare);
}

float** fvNewSet(int Count, int Dimensionality) {
    int k;
    float** Set = allocate(sizeof(float*) * Count);
    for (k = 0; k < Count; k++) Set[k] = fvNew(Dimensionality);
    return Set;
}

void fvDeleteSet(float** Set, int Count) {
    while (Count--) fvDelete(Set[Count]);
    deallocate(Set);
}

float** fvCopySet(float** Dest, float** Src, int Count, int Dimensionality) {
    int k;
    for (k = 0; k < Count; k++) fvCopy(Src[k], Dest[k], Dimensionality);
    return Dest;
}

int fvWriteSetWithPrec(float** Set, int Count, int Dimensionality, FILE* f, 
int width, int *prec) {
    int k, n, e;
    for (n = 0; n < Count; n++) {
        for (k = 0; k < Dimensionality; k++)
            fprintf(f, "%*.*f", width, prec[k], Set[n][k]);
        fprintf(f, "\n");
        e = ferror(f);
        if (e) return e;
    }
    return 0;
}

void fvPrint(float* V, int Dimensionality) {
    int k, n, e;
    for (k = 0; k < Dimensionality; k++) {
       printf("%f ", V[k]);
    }
    printf("\n");
    return;
}


int fvWriteSet(float** Set, int Count, int Dimensionality, FILE* f) {
    int k, n, e;
    for (n = 0; n < Count; n++) {
    for (k = 0; k < Dimensionality; k++)
        fprintf(f, "%*.*f ", PRINT_WIDTH, PRINT_PREC, Set[n][k]);
    fprintf(f, "\n");
    e = ferror(f);
    if (e) return e;
    }
    return 0;
}

static void Convert(double* DV, float* FV, int Dim) {
    while (Dim--) *FV++ = (float) *DV++;
}

int fvReadSet(float*** Set, int* Count, int* Dimensionality, FILE* f) {
    double* Vec;
    int Incomplete, Errors, D, Incomp, Err, Stage, DimErrors;    
    fpos_t Beginning;
    fgetpos(f, &Beginning);
    
    *Set = 0;
    *Dimensionality = 0;
    for (Stage = 0; Stage < 2; Stage++) {
        DimErrors = 0;
        *Count = 0;
        if (!feof(f)) 
            Vec = ReadRawVector(f, Dimensionality, &Incomplete, &Errors);
        else 
            return 0;
            
        if (Stage && Vec) Convert(Vec, (*Set)[0], *Dimensionality);
        if (Errors) return 0;
        if (Vec) *Count = 1;
        D = 0;
     
        while (!feof(f)) {
            Vec = ReadRawVector(f, &D, &Incomp, &Err);
            
            if (Err) return 0;
            if (Vec) {
                if (D != *Dimensionality) 
                    DimErrors++;
                    
                else {
                    if (Stage) Convert(Vec, (*Set)[*Count], *Dimensionality);
                    (*Count)++;
                }
            }
        }
        if (!Stage) {
            fsetpos(f, &Beginning);
            *Set = fvNewSet(*Count, *Dimensionality);
        }
    }
    return 1 + DimErrors;
}

void fvFillSet(float** Set, int Count, int Dimensionality, float Value) {
    int k;
    for (k = 0; k < Count; k++) fvFill(Set[k], Value, Dimensionality);
}

float** fvSubSetDimensions(float** Set, int Count, int Dimensionality,
    int* SelectedDimensions, float** SubSet, int* SubsetDimensionality)
{
    int k, n, Dims;
    Dims = 0;
    for (k = 0; k < Dimensionality; k++) if (SelectedDimensions[k]) Dims++;
    if (SubsetDimensionality) *SubsetDimensionality = Dims;
    if (!SubSet) SubSet = fvNewSet(Count, Dims);
    for (n = 0; n < Count; n++) {
    for (k = 0, Dims = 0; k < Dimensionality; ++k, ++Dims)
        if (SelectedDimensions[k]) SubSet[n][Dims] = Set[n][k];
    }
    return SubSet;
}

float fvSetAverage(float** Set, int Count, int Dimension) {
    int k;
    float Sum = 0;
    for (k = 0; k < Count; k++) Sum += Set[k][Dimension];
    return Sum / Count;
}

float fvSetNth(float** Set, int Count, int Dimension, int N) {
    int k;
    float Result;
    float* Values = allocate(sizeof(float) * Count);
    for (k = 0; k < Count; k++) Values[k] = Set[k][Dimension];
    fv_asc = 1;
    qsort(Values, Count, sizeof(float), &float_compare);
    Result = Values[N];
    deallocate(Values);
    return Result;
}

float fvSetMinimum(float** Set, int Count, int Dimension) {
    int k;
    float Result = Set[0][Dimension];
    for (k = 1; k < Count; k++)
    if (Result > Set[k][Dimension]) Result = Set[k][Dimension];
    return Result;
}

float fvSetMaximum(float** Set, int Count, int Dimension) {
    int k;
    float Result = Set[0][Dimension];
    for (k = 1; k < Count; k++)
    if (Result < Set[k][Dimension]) Result = Set[k][Dimension];
    return Result;
}

float fvSetTotalMinimum(float** Set, int Count, int Dimensionality) {
    int k;
    float Minimum, tmp;
    Minimum = fvSetMinimum(Set, Count, 0);
    for (k = 1; k < Dimensionality; k++) {
    tmp = fvSetMinimum(Set, Count, k);
    if (tmp < Minimum) Minimum = tmp;
    }
    return Minimum;
}

float fvSetTotalMaximum(float** Set, int Count, int Dimensionality) {
    int k;
    float Maximum, tmp;
    Maximum = fvSetMaximum(Set, Count, 0);
    for (k = 1; k < Dimensionality; k++) {
    tmp = fvSetMaximum(Set, Count, k);
    if (tmp > Maximum) Maximum = tmp;
    }
    return Maximum;
}

int fvSetIsBinary(float** Set, int Count, int Dimensionality, 
    float V1, float V2)
{
    int k, n;
    for (n = 0; n < Count; n++) for (k = 0; k < Dimensionality; k++)
    if (Set[n][k] != V1 || Set[n][k] != V2) return 0;
    return 1;
}

static int fv_dim = 0;
static int* fv_dord = 0;
static int* fv_vord = 0;
static int fv_ord = 0;

static int fv_gen_compare(const void* aa, const void* bb) {
    int k;
    float* a = *(float**)aa;
    float* b = *(float**)bb;
    if (!fv_dord || !fv_dord) for (k = 0; k < fv_dim; k++) {
    if (fv_ord) {
        if (a[k] < b[k]) return 1;
        if (a[k] > b[k]) return -1;
    } else {
        if (a[k] < b[k]) return -1;
        if (a[k] > b[k]) return 1;
    }
    } else for (k = 0; k < fv_dim; k++) {
    if (fv_vord[k]) {
        if (a[fv_dord[k]] < b[fv_dord[k]]) return 1;
        if (a[fv_dord[k]] > b[fv_dord[k]]) return -1;
    } else {
        if (a[fv_dord[k]] < b[fv_dord[k]]) return -1;
        if (a[fv_dord[k]] > b[fv_dord[k]]) return 1;
    }
    }
    return 0;
}

void fvSortSet(float** Set, int Count, int Dimensionality,
    int* DimensionOrder, int* ValueOrder, int Order)
{
    fv_dord = DimensionOrder;
    fv_vord = ValueOrder;
    fv_dim = Dimensionality;
    fv_ord = Order;
    qsort(Set, Count, sizeof(float*), &fv_gen_compare);
}

float* fvSetUnique(float** Set, int Count, int Dimension, int* FoundCount) {
    int k, n, Sorted;
    float* List, *Result;
    if (!Count) {
    *FoundCount = 0;
    return 0;
    }
    Sorted = 1;
    List = allocate(sizeof(float) * Count);
    List[0] = Set[0][Dimension];
    for (k = 1; k < Count; k++) {
    List[k] = Set[k][Dimension];
    if (List[k - 1] > List[k]) Sorted = 0;
    }
    if (!Sorted) fvSort(List, Count, 1);
    *FoundCount = 1;
    for (k = 1; k < Count; k++) if (List[k - 1] != List[k]) (*FoundCount)++;
    Result = allocate(sizeof(float) * *FoundCount);
    k = 0;
    for (n = 0; n < *FoundCount; n++) {
    Result[n] = List[k++];
    while (k < Count && List[k] == List[k - 1]) k++;
    }
    deallocate(List);
    return Result;
}
