/*-------------------------------------------------------------------*/
/* EVI.C        Mohammad Rezaei                                      */
/*                                                                   */
/* implementation of External Validity Indexes                       */
/*                                                                   */
/*  List of indexes:                                                 */
/*                 RI, ARI                                           */
/*                 NMI, NVI, NVD, JACCARD                            */
/*                 FM, CI, CI*                                       */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
/* HISTORY                                                           */
/* --------                                                          */
/* 0.05 SS  Partition based CI index                                 */
/* 0.04 MR  Adding Criterion H to indexes                            */
/* 0.03 MR  modifying CI* and using CI2 instead of CI1 (16.4.13)     */
/* 0.02 MR  implementation of indexes (21.2.13)                      */
/* 0.01 MR  preparing the main functions (29.9.12)                   */
/*-------------------------------------------------------------------*/

#define ProgName       "EVI"
#define VersionNumber  "Version 0.05"
#define LastUpdated    "17.8.2017"  /* SS */

/* converts ObjectiveFunction values to MSE values */
#define CALC_MSE(val) (double) (val) / (TotalFreq(pTS) * VectorSize(pTS))

#define AUTOMATIC_MAX_ITER  50000
#define AUTOMATIC_MIN_SPEED 1e-5
#define min(a,b) ((a) < (b) ? (a) : (b))

/*-------------------------------------------------------------------*/
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <string.h>

#include <math.h>
#include "cb.h"
#include "random.h"
#include "interfc.h"
#include "file.h"
#include "evi.h"


/* ========================== PROTOTYPES ============================= */

int PerformEVI(TRAININGSET   *TS, PARTITIONING *pP1, PARTITIONING *pP2, CODEBOOK *pCB1, CODEBOOK *pCB2, int indexType, int quietLevel, int useInitial)
{
  double        c, indexeValue = 0.0;

  SetClock(&c);

  switch (indexType) {
    case INDEX_RI:
      indexeValue = EVI_RI(pP1, pP2, quietLevel);
      break;
    case INDEX_ARI:
      indexeValue = EVI_ARI(pP1, pP2, quietLevel);
      break;
    case INDEX_NMI:
      indexeValue = EVI_NMI(pP1, pP2, quietLevel);
      break;
    case INDEX_NVI:
      indexeValue = EVI_NVI(pP1, pP2, quietLevel);
      break;
    case INDEX_MI:
      indexeValue = EVI_MI(pP1, pP2, quietLevel);
      break;
    case INDEX_VI:
      indexeValue = EVI_VI(pP1, pP2, quietLevel);
      break;
    case INDEX_NVD:
      indexeValue = EVI_NVD(pP1, pP2, quietLevel);
      break;
    case INDEX_JACCARD:
      indexeValue = EVI_JACCARD(pP1, pP2, quietLevel);
      break;
    case INDEX_FM:
      indexeValue = EVI_FM(pP1, pP2, quietLevel);
      break;
    case INDEX_CI:
      indexeValue = EVI_CI(pCB1, pCB2, quietLevel);
      break;
    case INDEX_CIstar:
      indexeValue = EVI_CISTAR(TS, pCB1, pCB2, quietLevel);
      break;
    case INDEX_CIpart:
      indexeValue = EVI_CIpart(pP1, pP2, quietLevel);
      break;
    case INDEX_CH:
      indexeValue = EVI_CH(pP1, pP2, quietLevel);
      break;
    default:
      break;
  }

  PrintResultsEVI(quietLevel, indexeValue, indexType, GetClock(c));
  return 0;

}  /* PerformEVI2() */



int CIpartGetOrphanCount(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int n, n1, n2, size,i1,i2;
  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  int *contigencyTable;
  int *counts = (int *)calloc(n2,sizeof(int));
  int zeros=0;
  contigencyTable = (int *)malloc(sizeof(int)*(n1*n2));
  Contingency(pP1, pP2, contigencyTable);

  for ( i1 = 0 ; i1 < n1 ; i1++ )
  {
      int max,maxInd,tmp;
      max=0;

      for ( i2 = 0 ; i2 < n2 ; i2++ )
      {
          tmp = contigencyTable[i2*n2+i1];

          if (tmp > max)
          {
              max = tmp;
              maxInd=i2;
          }
      }

      counts[maxInd]++;
  }


  for ( i2 = 0 ; i2 < n2 ; i2++ )
  {
      if(counts[i2]==0) { zeros++; }
  }

  free(contigencyTable);
  return zeros;

}
// CI indexd based on partitions
double EVI_CIpart(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int CI,zeros;

  //printf("From P1 to P2:\n");
  zeros = CIpartGetOrphanCount(pP1, pP2, quietLevel);
  CI = zeros;

  //printf("From P2 to P1:\n");
  zeros = CIpartGetOrphanCount(pP2, pP1, quietLevel);
  if(zeros > CI) { CI = zeros; }

  return (double) CI;
}


/*============  External validity indexes algorithms  ===============*/


double EVI_RI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int n, size;
  llong nis, njs, nt, nijs, avg, A;
  int *contigencyTable;
  double indexeValue = 0.0;

  size = pP1->TSsize;
  n = (pP1->PartitionCount)*(pP2->PartitionCount);
  contigencyTable = (int *)malloc(sizeof(int)*n);
  getSumSquares(pP1, pP2, contigencyTable, &nis, &njs, &nijs);

  avg = (nis+njs)/2;

  nt = (llong)size * (llong)(size-1)/2; // total number of pairs of entities
  A = nt + nijs - avg; // agreements
  indexeValue = (double)A/nt;	// Rand 1971
/*
  if ( 0 ) { // other indexes
    MIRKIN=D/(double)nt;
    HI=(A-D)/(double)nt;  // Hubert 1977
    F_M = 2*precision*recall/(precision+recall); // F measure
  } */

  free(contigencyTable);

  return indexeValue;
}


void getSumSquares(PARTITIONING *pP1, PARTITIONING *pP2, int *contigencyTable, llong *nis, llong *njs, llong *nijs)
{
  int i, j, n1, n2;

  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  Contingency(pP1, pP2, contigencyTable);

  // sum of squares of sums of rows
  (*nis) = SumSqRowsOrColumns(contigencyTable, n1, n2, 0);
  // sum of squares of sums of columns
  (*njs) = SumSqRowsOrColumns(contigencyTable, n1, n2, 1);

  // sum of squares of rows and columns
  (*nijs) = 0;
  for ( i = 0 ; i < n1 ; i++ )
    for ( j = 0 ; j < n2 ; j++ )
      (*nijs) += (llong)contigencyTable[i*n2+j]*contigencyTable[i*n2+j];
}


/*-------------------------------------------------------------------*/


double EVI_ARI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int n, size;
  llong nis, njs, nt, nijs, avg, A;
  int *contigencyTable;
  double indexeValue = 0.0, nc;

  size = pP1->TSsize;
  n = (pP1->PartitionCount)*(pP2->PartitionCount);
  contigencyTable = (int *)malloc(sizeof(int)*n);
  getSumSquares(pP1, pP2, contigencyTable, &nis, &njs, &nijs);

  avg = (nis+njs)/2;

  nt = (llong)size * (llong)(size-1)/2; // total number of pairs of entities
  A = nt + nijs - avg; // agreements

  // Expected index
  nc = (size*((double)size*size+1)-(size+1)*nis-((double)size+1)*njs+2*((double)nis*njs)/size)/(2*((double)size-1));

  if (nt == nc)
    indexeValue = 0;
  else
    indexeValue = (double)(A-nc)/(nt-nc);  // Hubert & Arabie 1985

  free(contigencyTable);

  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_NMI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  double indexeValue = 0.0, H1, H2, MI, NMI1, NMI2;

  getMutualInformation(pP1, pP2, &H1, &H2, &MI);

  NMI1 = 2*MI/(H1+H2);
  NMI2 = MI/sqrt(H1*H2);
  NMI2 = NMI2;

  indexeValue = NMI1;

  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_NVI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  double indexeValue = 0.0, H1, H2, MI, VI, NVI;

  getMutualInformation(pP1, pP2, &H1, &H2, &MI);

  // VI
  VI = H1+H2-2*MI;
  // NVI
  NVI = VI/(H1+H2);

  indexeValue = NVI;

  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_MI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  double indexeValue = 0.0, H1, H2, MI;

  getMutualInformation(pP1, pP2, &H1, &H2, &MI);
  indexeValue = MI;

  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_VI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  double indexeValue = 0.0, H1, H2, MI, VI;

  getMutualInformation(pP1, pP2, &H1, &H2, &MI);

  VI = H1+H2-2*MI;
  indexeValue = VI;

  return indexeValue;
}


/*-------------------------------------------------------------------*/


void getMutualInformation(PARTITIONING *pP1, PARTITIONING *pP2, double *H1, double*H2, double *MI)
{
  int i, j, n1, n2, n, size, iTemp;

  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  size = pP1->TSsize;
  n = n1*n2;

  int *clusterSize1 = (int *)malloc(sizeof(int)*n1);
  int *clusterSize2 = (int *)malloc(sizeof(int)*n2);
  int *contigencyTable = (int *)malloc(sizeof(int)*n);
  Contingency(pP1, pP2, contigencyTable);

  // size of clusters in clustering 1 and 2
  for ( i = 0 ; i < n1 ; i++ ) {
    clusterSize1[i] = 0;
    for ( j = 0 ; j < n2 ; j++ )
      clusterSize1[i] += contigencyTable[i*n2+j];
  }

  for ( j = 0 ; j < n2 ; j++ ) {
    clusterSize2[j] = 0;
    for ( i = 0 ; i < n1 ; i++ )
      clusterSize2[j] += contigencyTable[i*n2+j];
  }

  // Entropies
  (*H1) = 0;
  for ( i = 0 ; i < n1 ; i++ )
    (*H1) -= ((double)clusterSize1[i]/size)*log2((double)clusterSize1[i]/size);

  (*H2) = 0;
  for ( j = 0 ; j < n2 ; j++ )
    (*H2) -= ((double)clusterSize2[j]/size)*log2((double)clusterSize2[j]/size);

  (*MI)=0;
  for ( i = 0 ; i < n1 ; i++ )
    for ( j = 0 ; j < n2 ; j++ )
    {
      iTemp = contigencyTable[i*n2+j];
      if ( iTemp > 0)
        (*MI) += iTemp*log2(iTemp*size/((double)clusterSize1[i]*clusterSize2[j]));
    }

  (*MI) = (*MI)/size;

  free(clusterSize1);
  free(clusterSize2);
  free(contigencyTable);
}


/*-------------------------------------------------------------------*/


double EVI_NVD(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int i, j, n1, n2, n, size, iTemp;
  llong nij12, nij21;
  int *contigencyTable;
  double indexeValue = 0.0, VD, NVD, purity;

  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  size = pP1->TSsize;
  n = n1*n2;
  contigencyTable = (int *)malloc(sizeof(int)*n);
  Contingency(pP1, pP2, contigencyTable);


  nij12 = 0;
  for ( i=0 ; i < n1 ; i++ ) {
    iTemp = 0;
    for ( j=0 ; j < n2 ; j++ )
      if ( contigencyTable[i*n2+j] > iTemp )
        iTemp = contigencyTable[i*n2+j];
    nij12 += iTemp;
  }

  /* n1max = 0; do not delete // used for another type of NVD
      for ( i=0 ; i < n1 ; i++ ) {
        temp = 0;
        for ( j=0 ; j < n2 ; j++ )
          temp += contigencyTable[i*n2+j];
        if ( temp > n1max )
          n1max = temp;
      }  */
  nij21 = 0;
  for ( j=0 ; j < n2 ; j++ ) {
    iTemp = 0;
    for ( i=0 ; i < n1 ; i++ )
      if ( contigencyTable[i*n2+j] > iTemp )
        iTemp = contigencyTable[i*n2+j];
    nij21 += iTemp;
  }
/*  n2max = 0;  do not delete
      for ( j=0 ; j < n2 ; j++ ) {
        temp = 0;
        for ( i=0 ; i < n1 ; i++ )
          temp += contigencyTable[i*n2+j];
        if ( temp > n2max )
          n2max = temp;
      } */
  VD = (double)(2*size-nij12-nij21);
  NVD = VD / (2*size);
  indexeValue = NVD;
  purity = (double)nij12 / size;
  purity =  purity;

  free(contigencyTable);
  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_JACCARD(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int n, n1, n2, size, i, j;
  llong nis, njs, nijs, avg, D, type1, type2;
  int *contigencyTable;
  double indexeValue = 0.0;

  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  size = pP1->TSsize;
  n = n1*n2;
  contigencyTable = (int *)malloc(sizeof(int)*n);
  getSumSquares(pP1, pP2, contigencyTable, &nis, &njs, &nijs);

  avg = (nis+njs)/2;
  D = -nijs + avg;     // disagreements

  type1 = 0;
  // type one of agreements
  for ( i = 0 ; i < n1 ; i++ )
    for ( j = 0 ; j < n2 ; j++ )
      type1 = type1 + (llong)contigencyTable[i*n2+j]*(contigencyTable[i*n2+j]-1);
  type1 = type1 / 2;

  type2 = nijs - nis - njs;
  type2 = type2 + (size*size);
  type2 = type2 / 2;

  indexeValue = (double)type1/(type1+D);

  free(contigencyTable);

  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_FM(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int n, n1, n2, i, j;
  llong nis, njs, nijs, type1, type3, type4;
  int *contigencyTable;
  double indexeValue = 0.0, precision, recall;

  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  n = n1*n2;
  contigencyTable = (int *)malloc(sizeof(int)*n);
  getSumSquares(pP1, pP2, contigencyTable, &nis, &njs, &nijs);

  type1 = 0;
  // type one of agreements
  for ( i = 0 ; i < n1 ; i++ )
    for ( j = 0 ; j < n2 ; j++ )
      type1 = type1 + (llong)contigencyTable[i*n2+j]*(contigencyTable[i*n2+j]-1);
  type1 = type1 / 2;

  type3 = (njs - nijs)/2;
  type4 = (nis - nijs)/2;

  precision = (double)type1/(type1+type3);
  recall = (double)type1/(type1+type4);

  indexeValue = sqrt(precision*recall);

  free(contigencyTable);

  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_CH(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel)
{
  int i, j, i1, j1, n1, n2, n, size, iTemp;
  llong nij12;
  int *contigencyTable, *contigencyTableSort, *indexSort, *valid1, *valid2, nPairs;
  double indexeValue = 0.0, CH;

  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  size = pP1->TSsize;
  n = n1*n2;
  contigencyTable = (int *)malloc(sizeof(int)*n);
  valid1 = (int *)malloc(sizeof(int)*n1);
  valid2 = (int *)malloc(sizeof(int)*n2);
  contigencyTableSort = (int *)malloc(sizeof(int)*n);
  indexSort = (int *)malloc(sizeof(int)*n);
  Contingency(pP1, pP2, contigencyTable);

  for ( i=0 ; i < n1 ; i++ )
    valid1[i] = 1;
  for ( i=0 ; i < n2 ; i++ )
    valid2[i] = 1;
  // Sort values in contigency table
  for ( i=0 ; i < n ; i++ ) {
    contigencyTableSort[i] = contigencyTable[i];
    indexSort[i] = i;
  }

  // bubble sort
  for ( i=0 ; i < n ; i++ )
    for ( j=i+1 ; j < n ; j++ )
      if ( contigencyTableSort[j] > contigencyTableSort[i] ) {
        iTemp = contigencyTableSort[j];
        contigencyTableSort[j] = contigencyTableSort[i];
        contigencyTableSort[i] = iTemp;

        iTemp = indexSort[j];
        indexSort[j] = indexSort[i];
        indexSort[i] = iTemp;
      }
  nij12 = 0;
  nPairs = 0;
  for ( i=0 ; i < n ; i++ ) {  // check pairs of clusters
    j = indexSort[i];
    j1 = j%n2;
    i1 = j/n2;
    if ( valid1[i1] && valid2[j1] && contigencyTable[i1*n2+j1] > 0 ) {
      // make pair
      nij12 += contigencyTable[i1*n2+j1];
      // make the row and the column used (invalid for the rest)
      nPairs++;
      valid1[i1] = 0;
      valid2[j1] = 0;
    }

    if ( nPairs >= n1 || nPairs >= n2 )
      break;
  }

//printf("Number of pairs: %d\n", nPairs);
  CH = 1.0 - ((double)nij12 / size);
  indexeValue = CH;

  free(valid1);
  free(valid2);
  free(contigencyTableSort);
  free(indexSort);
  free(contigencyTable);
  return indexeValue;
}


/*-------------------------------------------------------------------*/


double EVI_CI_testClusterSizes(TRAININGSET   *TS, CODEBOOK* pCB1, CODEBOOK* pCB2, int quietLevel)
{
  double MSE1 = 0.0, MSE2 = 0.0;
  double MAE1 = 0.0, MAE2 = 0.0;
  int    nMatches1 = 0, nMatches2 = 0;
  int    CI12, CI21, CI2;

  PARTITIONING P1;
  PARTITIONING P2;
  PARTITIONING P1x;
  PARTITIONING P2x;

  CreateNewPartitioning(&P1x, TS, BookSize(pCB1));
  MSE1 = GenerateOptimalPartitioning(TS, pCB1, &P1x);
  CreateNewPartitioning(&P2x, TS, BookSize(pCB2));
  MSE2 = GenerateOptimalPartitioning(TS, pCB2, &P2x);

  // pairing centroids
  CreateNewPartitioning(&P1, pCB1, BookSize(pCB2));
  GenerateOptimalPartitioning(pCB1, pCB2, &P1);
  CalculateMAEandNMatches(pCB1, pCB2, &P1, &MAE1, &nMatches1);

  CreateNewPartitioning(&P2, pCB2, BookSize(pCB1));
  GenerateOptimalPartitioning(pCB2, pCB1, &P2);
  CalculateMAEandNMatches(pCB2, pCB1, &P2, &MAE2, &nMatches2);

  if ( quietLevel >= 1 )
    printf("\nComparing clustering 1 with 2:\n");
  CI12 = GlobalAllocations_testClusterSizes(pCB1, pCB2, &P1, quietLevel, &P1x, &P2x);
  if ( quietLevel >= 1 )
    printf("\nComparing clustering 2 with 1:\n");
  CI21 = GlobalAllocations_testClusterSizes(pCB2, pCB1, &P2, quietLevel, &P2x, &P1x);

  if ( quietLevel >= 1 )
    printf("\n");

  CI2 = CI12;
  if ( CI21 > CI2 )
    CI2 = CI21;

  return (double)CI2;
}


/*-------------------------------------------------------------------*/


double EVI_CI(CODEBOOK* pCB1, CODEBOOK* pCB2, int quietLevel)
{
  double MSE1 = 0.0, MSE2 = 0.0;
  double MAE1 = 0.0, MAE2 = 0.0;
  int    nMatches1 = 0, nMatches2 = 0;
  int    CI12, CI21, CI2;
  int i;

  PARTITIONING P1;
  PARTITIONING P2;

  CreateNewPartitioning(&P1, pCB1, BookSize(pCB2));
  MSE1 = GenerateOptimalPartitioning(pCB1, pCB2, &P1);
  MSE1 = MSE1;
  CalculateMAEandNMatches(pCB1, pCB2, &P1, &MAE1, &nMatches1);

  CreateNewPartitioning(&P2, pCB2, BookSize(pCB1));
  MSE2 = GenerateOptimalPartitioning(pCB2, pCB1, &P2);
  MSE2 = MSE2;
  CalculateMAEandNMatches(pCB2, pCB1, &P2, &MAE2, &nMatches2);


  //printf("p1:\n");
  //PrintPartitioning(&P1);
  //printf("p2:\n");
  //PrintPartitioning(&P2);


  if (quietLevel >= 2) {
      printf("Partition vectors (1 to 2): %d",UniqueVectors(&P1,0));
      for( i=1; i<PartitionCount(&P1); i++ ) {
          printf(":%d",UniqueVectors(&P1,i));
      }
      printf("\n");

      printf("Partition vectors (2 to 1): %d",UniqueVectors(&P2,0));
      for( i=1; i<PartitionCount(&P2); i++ ) {
          printf(":%d",UniqueVectors(&P2,i));
      }
      printf("\n");
  }

/*
  if( quietLevel >= 2 ) {
    printf("                  clustering1->clustering2    clustering2->clustering1    Average\n");
    printf("MSE Distortion = %9.4f %9.4f %9.4f\n",
            PrintableError(MSE1,pCB2),
            PrintableError(MSE2,pCB2),
            PrintableError((MSE1+MSE2)/2.0,pCB2));
    printf("MAE Distortion = %9.4f %9.4f %9.4f\n",
            PrintableError(MAE1,pCB2),
            PrintableError(MAE2,pCB2),
            PrintableError((MAE1+MAE2)/2.0,pCB2));
    printf("Equal vectors  = %9d %9d\n", nMatches1, nMatches2);
  }
  else if( quietLevel >= 1 ) {
    printf("%9.4f %9.4f %9.4f\n", PrintableError(MSE1,pCB2),
                                  PrintableError(MSE2,pCB2),
                                  PrintableError((MSE1+MSE2)/2.0,pCB2));

  }
  else
    printf("%9.4f  ", PrintableError((MSE1+MSE2)/2.0,pCB2));
*/
  if ( quietLevel >= 2 )
    printf("\nComparing clustering 1 with 2:\n");
  CI12 = GlobalAllocations(pCB1, pCB2, &P1, quietLevel);
  if ( quietLevel >= 2 )
    printf("\nComparing clustering 2 with 1:\n");
  CI21 = GlobalAllocations(pCB2, pCB1, &P2, quietLevel);

  if ( quietLevel >= 2 )
    printf("\n");

  CI2 = CI12;
  if ( CI21 > CI2 )
    CI2 = CI21;

  return (double)CI2;
}


/*-------------------------------------------------------------------*/


double EVI_CISTAR(TRAININGSET   *TS, CODEBOOK* pCB1, CODEBOOK* pCB2, int quietLevel)
{
  double MSE1 = 0.0, MSE2 = 0.0;
  double MAE1 = 0.0, MAE2 = 0.0;
  double MSEx = 0.0;
  int    nMatches1 = 0, nMatches2 = 0;
  int    i, j, iTemp, n1, n2, size;
  llong nij12, nij21;
  double VD, NVD, s12, s21, S2, S;
  int *contigencyTable;
  int *mapTemp1;
  int *mapTemp2;
  int *mapIdx1;
  int *mapIdx2;
  int indexType = 3;
  PARTITIONING P1x; // this is used just for paring of centroids, not real partitions of data
  PARTITIONING P2x; // this is used just for paring of centroids, not real partitions of data
  PARTITIONING P1;
  PARTITIONING P2;

  S = 0.0;

  // check TS is valid?
  if ( VectorSize(TS) != VectorSize(pCB1) ) {
    ErrorMessage("\nERROR: Needs valid training set!\n\n");
    ExitProcessing(FATAL_ERROR);
  }

  CreateNewPartitioning(&P1x, pCB1, BookSize(pCB2)); // for pairing
  CreateNewPartitioning(&P1, TS, BookSize(pCB1));
  MSE1 = GenerateOptimalPartitioning(pCB1, pCB2, &P1x);
  MSEx = GenerateOptimalPartitioning(TS, pCB1, &P1);
  MSE1 = MSE1; MSEx = MSEx;
  CalculateMAEandNMatches(pCB1, pCB2, &P1x, &MAE1, &nMatches1);

  CreateNewPartitioning(&P2x, pCB2, BookSize(pCB1));
  CreateNewPartitioning(&P2, TS, BookSize(pCB2));
  MSE2 = GenerateOptimalPartitioning(pCB2, pCB1, &P2x);
  MSE2 = MSE2;
  MSEx = GenerateOptimalPartitioning(TS, pCB2, &P2);
  CalculateMAEandNMatches(pCB2, pCB1, &P2x, &MAE2, &nMatches2);
/*
  if( quietLevel >= 2 ) {
    printf("                  clustering1->clustering2    clustering2->clustering1    Average\n");
    printf("MSE Distortion = %9.4f %9.4f %9.4f\n",
            PrintableError(MSE1,pCB2),
            PrintableError(MSE2,pCB2),
            PrintableError((MSE1+MSE2)/2.0,pCB2));
    printf("MAE Distortion = %9.4f %9.4f %9.4f\n",
            PrintableError(MAE1,pCB2),
            PrintableError(MAE2,pCB2),
            PrintableError((MAE1+MAE2)/2.0,pCB2));
    printf("Equal vectors  = %9d %9d\n", nMatches1, nMatches2);
  }
  else if( quietLevel >= 1 ) {
    printf("%9.4f %9.4f %9.4f\n", PrintableError(MSE1,pCB2),
                                  PrintableError(MSE2,pCB2),
                                  PrintableError((MSE1+MSE2)/2.0,pCB2));

  }
  else
    printf("%9.4f  ", PrintableError((MSE1+MSE2)/2.0,pCB2));
*/
  n1 = (&P1)->PartitionCount;
  n2 = (&P2)->PartitionCount;
  size = (&P1)->TSsize;
  contigencyTable = (int *)malloc(sizeof(int)*n1*n2);
  Contingency(&P1, &P2, contigencyTable);
  // Van Dongen according to CI pairing
  if ( (indexType == 1) || (indexType == 2) ) {// according to NVD and CI
    nij12 = 0;
    for ( i=0 ; i < n1 ; i++ ) {
      j = Map(&P1x,i); // pair of the cluster i in the other clustering
      iTemp = contigencyTable[i*n2+j];
      nij12 += (llong)iTemp;
    }

    nij21 = 0;
    for ( j=0 ; j < n2 ; j++ ) {
      i = Map(&P2x,j); // pair of the cluter j in the other clustering
      iTemp = contigencyTable[i*n2+j];
      nij21 += (llong)iTemp;
    }

    VD = (double)(2*size-nij12-nij21);
    NVD = VD / (2*size);

    // second type of point wise measure according to CI pairing (Pasi's idea)
    s12 = nij12/size;
    s21 = nij21/size;
    S2 = (s12+s21)/2; // make the index symmetric

    if ( indexType == 1 )
      S = NVD;
    if ( indexType == 2 )
      S = S2;
  }

  // third and fourth types of point wise measure according to CI pairing and general pairing (Pasi's idea)
  // comparing clustering 1 with 2
  if ( indexType == 3 || indexType == 4 ) {
    mapTemp1 = (int *)malloc(sizeof(int)*n1);
    mapTemp2 = (int *)malloc(sizeof(int)*n2);
    mapIdx1 = (int *)malloc(sizeof(int)*n1);
    mapIdx2 = (int *)malloc(sizeof(int)*n2);

    if ( quietLevel >= 3 )
      printf("\nDataset size = %d\n\n", size);

    for ( j=0 ; j < n2 ; j++ )
      mapTemp2[j] = -1;
    for ( i=0 ; i < n1 ; i++ )
      mapTemp1[i] = -1;

    if ( indexType == 4 ) { // mapping
      for ( i=0 ; i < n1 ; i++ ) {
        iTemp = 0;
        for ( j=0 ; j < n2 ; j++ )
         if ( contigencyTable[i*n2+j] > iTemp ) {
	       iTemp = contigencyTable[i*n2+j];
	       mapIdx1[i] = j;
        }
      }

	  for ( j=0 ; j < n2 ; j++ ) {
        iTemp = 0;
        for ( i=0 ; i < n1 ; i++ )
         if ( contigencyTable[i*n2+j] > iTemp ) {
	       iTemp = contigencyTable[i*n2+j];
	       mapIdx2[j] = i;
	     }
      }
    }
    else {
      for ( i=0 ; i < n1 ; i++ )
        mapIdx1[i] = Map(&P1x,i);
      for ( j=0 ; j < n2 ; j++ )
        mapIdx2[j] = Map(&P2x,j);
    }

    for ( i=0 ; i < n1 ; i++ ) {
      j = mapIdx1[i]; // pair of the cluster i in the other clustering
      iTemp = contigencyTable[i*n2+j];
      if ( mapTemp2[j] != -1 ) { // take the mapped cluster with maximum shared objects only
        if ( iTemp > contigencyTable[mapTemp2[j]*n2+j] )
          mapTemp2[j] = i;
      }
      else
        mapTemp2[j] = i;
    }

    if ( quietLevel >= 3 )
      printClustersInfo(mapTemp1, mapTemp2, n1, n2, contigencyTable, size);

    s12 = 0;
    for ( j=0 ; j < n2 ; j++ ) {
      if ( mapTemp2[j] != -1 ) {
        i = mapTemp2[j];
        iTemp = contigencyTable[i*n2+j];
        s12 += (double)iTemp;
      }
    }
    s12 /= size;

    // comparing clustering 2 with 1
    for ( i=0 ; i < n1 ; i++ )
      mapTemp1[i] = -1;
    for ( j=0 ; j < n2 ; j++ )
      mapTemp1[j] = -1;

    for ( j=0 ; j < n2 ; j++ ) {
      i = mapIdx2[j]; // pair of the cluster i in the other clustering
      iTemp = contigencyTable[i*n2+j];
      if ( mapTemp1[i] != -1 ) { // take the mapped cluster with maximum shared objects only
        if ( iTemp > contigencyTable[i*n2+mapTemp1[i]] )
          mapTemp1[i] = j;
      }
      else
        mapTemp1[i] = j;
    }

    if ( quietLevel >= 3 )
      printClustersInfo(mapTemp2, mapTemp1, n2, n1, contigencyTable, size);

    s21 = 0;
    for ( i=0 ; i < n1 ; i++ ) {
	  if ( mapTemp1[i] != -1 ) {
	    j = mapTemp1[i];
	    iTemp = contigencyTable[i*n2+j];
	    s21 += (double)iTemp;
	  }
    }
    s21 /= size;

    S = (s12+s21)/2.0; // make the index symmetric
  }

  free(contigencyTable);
/*  free(&P1x);
  free(&P1);
  free(&P2x);
  free(&P2);*/
  free(mapTemp1);
  free(mapTemp2);
  free(mapIdx1);
  free(mapIdx2);

  return S;
}


/*-------------------------------------------------------------------*/


int Contingency(PARTITIONING *pP1, PARTITIONING *pP2, int *contigencyTable)
{
  int i1, i2, j, n1, n2, size;

  n1 = pP1->PartitionCount;
  n2 = pP2->PartitionCount;
  size = pP1->TSsize;

  for ( i1 = 0 ; i1 < n1 ; i1++ )
    for ( i2 = 0 ; i2 < n2 ; i2++ )
      contigencyTable[i1*n2+i2] = 0;

  for ( j = 0 ; j < size ; j++ ) {
    i1 = Map(pP1,j);
    i2 = Map(pP2,j);
    contigencyTable[i1*n2+i2] += 1;
  }

  return 0;
}


/*-------------------------------------------------------------------*/


llong SumSqRowsOrColumns(int *contigencyTable, int n1, int n2, int rowOrColumn)
{
  int i, j;
  llong ns, temp;

  ns = 0;
  if ( rowOrColumn == 0 ) { // for rows
    for ( i = 0 ; i < n1 ; i++ )
    {
      temp = 0;
      for ( j = 0 ; j < n2 ; j++ )
        temp += (llong)contigencyTable[i*n2+j];

      temp *= temp;
      ns += temp;
    }
  }
  else { // for columns
    for ( j = 0 ; j < n2 ; j++ )
    {
      temp = 0;
      for ( i = 0 ; i < n1 ; i++ )
        temp += (llong)contigencyTable[i*n2+j];

      temp *= temp;
      ns += temp;
    }
  }

  return ns;
}


/*-------------------------------------------------------------------*/


char* EVIInfo(void)
{
  char* p;
  int len;

  len = strlen(ProgName)+strlen(VersionNumber)+strlen(LastUpdated)+4;
  p   = (char*) malloc(len*sizeof(char));

  if (!p)
    {
    ErrorMessage("ERROR: Allocating memory failed!\n");
    ExitProcessing(FATAL_ERROR);
    }

  sprintf(p, "%s\t%s\t%s", ProgName, VersionNumber, LastUpdated);

  return p;
}


/*-------------------------------------------------------------------*/


int getPartitionsFromFiles(char *InName1, char *InName2, char *InName3, TRAININGSET   *TS, PARTITIONING *pP1, PARTITIONING *pP2)
{
  int numFiles = 0;
  int type1, type2, type3;
  CODEBOOK      CB1;
  CODEBOOK      CB2;

  if ( (type1 = DetermineFileType(InName1)) != NOTFOUND )
    numFiles++;
  if ( (type2 = DetermineFileType(InName2)) != NOTFOUND )
    numFiles++;
  if ( (type3 = DetermineFileType(InName3)) != NOTFOUND )
    numFiles++;

  if ( numFiles == 2 ) {
    ReadPartitioningFile(InName1, pP1);
    ReadPartitioningFile(InName2, pP2);
  }

  if ( numFiles == 3 ) {
    ReadTrainingSet(InName1, TS);
    if ( type2 == PAFILE )
      ReadPartitioningFile(InName2, pP1);
    else {

      ReadCodebook(InName2, &CB1);
      CreateNewPartitioning(pP1, TS, BookSize(&CB1));
      GenerateOptimalPartitioning(TS, &CB1, pP1);
    }

    if ( type3 == PAFILE )
      ReadPartitioningFile(InName3, pP2);
    else {
      ReadCodebook(InName3, &CB2);
      CreateNewPartitioning(pP2, TS, BookSize(&CB2));
      GenerateOptimalPartitioning(TS, &CB2, pP2);
    }
  }

    // check size of clusterings
  if ( pP1->TSsize != pP2->TSsize ) {
    ErrorMessage("\nERROR: Partitionings should have the same size!\n\n");
    ExitProcessing(FATAL_ERROR);
  }

  return 0;
}


/*-------------------------------------------------------------------*/


int getCentroidsFromFiles(char *InName1, char *InName2, char *InName3, TRAININGSET   *TS, CODEBOOK *pCB1, CODEBOOK *pCB2)
{
  int useInitial = 0;
  int numFiles = 0;
  int type1, type2, type3;
  PARTITIONING  P1;
  PARTITIONING  P2;

  if ( (type1 = DetermineFileType(InName1)) != NOTFOUND )
    numFiles++;
  if ( (type2 = DetermineFileType(InName2)) != NOTFOUND )
    numFiles++;
  if ( (type3 = DetermineFileType(InName3)) != NOTFOUND )
    numFiles++;

  if ( numFiles == 2 ) {
    ReadCodebook(InName1, pCB1);
    ReadCodebook(InName2, pCB2);
    useInitial = 0;
  }

  if ( numFiles == 3 ) {
    ReadTrainingSet(InName1, TS);
    if ( type2 == CBFILE ) {
      ReadCodebook(InName2, pCB1);
    }
    else {
      ReadPartitioning(InName2, &P1, TS);
      CreateNewCodebook(pCB1, (&P1)->PartitionCount, TS);
      GenerateOptimalCodebook(TS, pCB1, &P1);
    }

    if ( type3 == CBFILE )
      ReadCodebook(InName3, pCB2);
    else {
      ReadPartitioning(InName3, &P2, TS);
      CreateNewCodebook(pCB2, (&P2)->PartitionCount, TS);
      GenerateOptimalCodebook(TS, pCB2, &P2);
    }

    useInitial = 0;
  }

  return useInitial;

}


/*-------------------------------------------------------------------*/


int GlobalAllocations_testClusterSizes(TRAININGSET* pCB1, CODEBOOK* pCB2, PARTITIONING* P, int quietLevel, PARTITIONING *P1, PARTITIONING *P2)
{
  int  i, count, CI;
  int  zeros=0, nonzeros=0;
  int clusterSize;
  int t1, t2;
  double temp;

  t1 = 0;
  printf(" Cluster sizes for unmatched clusters:\n");
  printf("Cluster no.\t\tSize\n", i, clusterSize);
  for (i=0; i<BookSize(pCB2); i++) {
    count = UniqueVectors(P,i);
    if (count==0) { // orphan
      zeros++;
      clusterSize = UniqueVectors(P2,i);
      printf("%i\t\t%i\n", i, clusterSize);
      t1 += clusterSize;
    }
  }

  printf("total unmatched: %i\n", t1);

  temp = (double)t1;
  if ( zeros != 0 )
    temp /= zeros;
  t1 = floor(temp+0.5);
  printf("Average Cluster size for unmatched clusters: %i\n", t1);

  t2 = 0;
  printf(" Cluster sizes for matched clusters:\n");
  printf("Cluster no.\t\tSize\n", i, clusterSize);
  for (i=0; i<BookSize(pCB2); i++) {
    count = UniqueVectors(P,i);
    if (count>0) {
      nonzeros++;
      clusterSize = UniqueVectors(P2,i);
      printf("%i\t\t%i\n", i, clusterSize);
      t2 += clusterSize;
    }
  }

  printf("total matched: %i\n", t2);

  temp = (double)t2;
  if ( nonzeros != 0 )
    temp /= nonzeros;
  t2 = floor(temp+0.5);
  printf("Average Cluster size for matched clusters: %i\n", t2);

  if( quietLevel >= 1 ) {
    printf("  No matches: %i ", zeros);
    printf("\n");
  }

  CI = zeros;

  return CI;
}



/*-------------------------------------------------------------------*/


int GlobalAllocations(TRAININGSET* pCB1, CODEBOOK* pCB2, PARTITIONING* P, int quietLevel)
{
  int  i, count, CI;
  int  total=0;
  int  zeros=0, ones=0, plenty=0;

  /* First sketch to see if the partition counts give the answer directly. */
  for (i=0; i<BookSize(pCB2); i++) {
    count = UniqueVectors(P,i);
    if(count==0) zeros++;
    if(count==1) ones++;
    if(count>=2) plenty++;
//    if(quietLevel>=3) printf("%i ", count);
    total += count;
  }

  if(quietLevel>=10) printf("\n");
  if( quietLevel >= 10 ) {
    printf("\nGlobal matching result:\n");
    printf("  No matches:   %i \n", zeros);
    printf("  One match:    %i \n", ones);
    printf("  Two or more:  %i \n", plenty);
    if(ones==BookSize(pCB2))
       printf("CORRECT MATCH. \n");
    else
       printf("The first file needs %i swaps to match the second one.\n", zeros);
  }
  else if( quietLevel >= 2 ) {
    printf("  No matches: %i ", zeros);
//    if(ones==BookSize(pCB2))  printf(" CORRECT MATCH.");
    printf("\n");
  }

  /* Validity checkings. Note: Frequencies are ignored!!! */
/*  if((zeros+ones+plenty)!=BookSize(pCB2)) printf("Warning: incorrect counts in Global Allocation!\n");
  if( total != BookSize(pCB2)) {
    printf("\nNOTE!!! Cluster count %i dosen't match ", total);
    printf("the size of the second file %i. ", BookSize(pCB2));
    printf("\nThe analysis might therefore be meaningless.\n");
  } */

  CI = zeros;

  return CI;
}


/*-------------------------------------------------------------------*/


void CalculateMAEandNMatches(CODEBOOK* pCB1,
                                    CODEBOOK* pCB2,
                                    PARTITIONING* P,
                                    double* MAE,
                                    int* nMatches)
{
  int    i,j;
  int    MAEdiff;
  llong  MAEsum    = 0LL;

  *nMatches = 0;
  for (i=0; i<BookSize(pCB1); i++) {
    MAEdiff = 0;
    for (j=0; j<VectorSize(pCB1); j++) {
      MAEdiff += VectorFreq(pCB1,i) * abs( VectorScalar(pCB1,i,j) -
                                         VectorScalar(pCB2,Map(P,i),j) );
    }
    if (MAEdiff == 0) (*nMatches)++;
    MAEsum += MAEdiff;
  }
  *MAE = (double) MAEsum / ((double)TotalFreq(pCB1) * (double)VectorSize(pCB1));
}

int DetermineFileType(char *fileName)
{
  int type;
  type = NOTFOUND;

  if (*fileName) {
    type = DetermineCBFileType(fileName);
    if ( type == NOTFOUND ) {
        ErrorMessage("\nERROR: Type of the file "
            "%s is unidentified!\n\n", fileName);
        ExitProcessing(FATAL_ERROR);
    }
  }

  return type;
}

/* ------------------------------------------------------------------ */


int CheckParameters(char *InName1, char *InName2, char *InName3, int indexType) {

  int numFiles = 0;
  int type1, type2, type3, flag;
  type1 = NOTFOUND;
  type2 = NOTFOUND;
  type3 = NOTFOUND;

  if ( (type1 = DetermineFileType(InName1)) != NOTFOUND )
    numFiles++;
  if ( (type2 = DetermineFileType(InName2)) != NOTFOUND )
    numFiles++;
  if ( (type3 = DetermineFileType(InName3)) != NOTFOUND )
    numFiles++;

  flag = 0;
  if ( numFiles == 2 ) {
    if ( (type1 == CBFILE || type1 == TSFILE) && (type2 == CBFILE || type2 == TSFILE) && (indexType == INDEX_CI) ) // CI index
      flag = 1;
    if ( (type1 == PAFILE) && (type2 == PAFILE) && ((indexType != INDEX_CI) && (indexType != INDEX_CIstar)) ) // other index rather than CI
      flag = 1;

    if ( flag == 0 ) {
      if ( indexType == INDEX_CI )
        ErrorMessage("\nERROR: To claculate CI, input files should be centroids (CB) or\nyou need to give the training set as well!\n\n");
      else if ( indexType == INDEX_CIstar )
        ErrorMessage("\nERROR: To claculate CI*, partitions or centroids from two\n clusterings in addition to the training set are needed!\n\n");
      else
        ErrorMessage("\nERROR: To claculate indexes rather than CI and CI*, partitions from two\n clusterings are needed or you need to give the training set as well!\n\n");
	}
  }

  if ( numFiles == 3 ) {
    if ( type1 == TSFILE )
      flag = 1;
	else
	  ErrorMessage("\nERROR: Invalid input files\n\n");
  }

  if ( !flag ) {
    ExitProcessing(FATAL_ERROR);
  }

  return 0;
}  /* CheckParameters() */


/* ------------------------------------------------------------------ */


int DetermineFileName(char *name)
{
  char newName[MAXFILENAME], suffix[MAXFILENAME];
  int  i;

  /* Without extension */
  if (ExistFile(name))
  {
    return 1;
  }

  for (i = 0; i < 3; i++)
  {
    if (i == 0)      /* Try TS-file extension */
      strcpy(suffix, FormatNameTS);

    else if (i == 1)  /* Try CB-file extension */
      strcpy(suffix, FormatNameCB);

    else             /* Try PA-file extension */
      strcpy(suffix, FormatNamePA);

    if (strlen(name) < MAXFILENAME-strlen(suffix)-1)
    {
      strcpy(newName, name);
      CheckFileName(newName, suffix);
      if (ExistFile(newName))
      {
        strcpy(name, newName);
        return 1;
      }
    }
  }

  /* No luck this time */
  return 0;
}  /* DetermineFileName() */

double SetClock(double* start)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return( *start = (double)tv.tv_sec + ((double)tv.tv_usec /1e6) );
}  /* SetClock() */


/* ------------------------------------------------------------------ */


double GetClock(double start)
{
  struct timeval tv;
  double elapsed;

  gettimeofday(&tv, NULL);
  elapsed = (double)tv.tv_sec + ((double)tv.tv_usec /1e6) - start;

  return elapsed;
}  /* GetClock() */


/* ------------------------------------------------------------------ */


void PrintHeader(int quietLevel)
{
  if ((quietLevel >= 2))
  {
    PrintMessage("\n");
    PrintMessage("Iteration\tMSE\t\tTime\n");
  }
}  /* PrintHeader() */


/* ------------------------------------------------------------------ */


void PrintResultsEVI(int quietLevel, double indexValue, int indexType, double time)
{
  if ( quietLevel == 0 ) {
    if ( indexType == INDEX_CI )
  	  PrintMessage("%d\n", (int)indexValue);
    else
  	  PrintMessage("%f\n", indexValue);
  }
  if ( quietLevel == 1 ) {
    if ( indexType == INDEX_CI )
      PrintMessage("%d\t%f\n", (int)indexValue, time);
    else
      PrintMessage("%f\t%f\n", indexValue, time);
  }

  if (quietLevel >= 2) {
	if ( indexType == INDEX_CI )
      PrintMessage("%s = %d  time = %f", Name(indexType), (int)indexValue, time);
    else
	  PrintMessage("\n\n%s = %f  time = \t%f\n", Name(indexType), indexValue, time);

    PrintMessage("\n");
  }

}  /* PrintIterationEVI() */


/* ------------------------------------------------------------------ */


void PrintFooterEVI(int quietLevel, int iter, double error, double time)
{
  if (quietLevel >= 2)
  {
    PrintMessage("\nmse = %f  time = %f  %d iterations\n\n", error, time, iter);
  }
  else if (quietLevel == 1)
  {
    PrintMessage("%f\t%f\n", error, time);
  }
}  /* PrintFooterEVI() */


/* ------------------------------------------------------------------ */

void printClustersInfo(int *mapTemp1, int *mapTemp2, int n1, int n2, int *contigencyTable, int size)
{
  int i, j;

  printf("\nComparing clustering 1 with 2 -- paired clusters and number of shared objects.\n\n");
  for ( j=0 ; j < n2 ; j++ )
    if ( mapTemp2[j] != -1 )
      mapTemp1[mapTemp2[j]] = j;

  for ( i=0 ; i < n1 ; i++ )
    if ( mapTemp1[i] != -1 )
      printf("%12d , %d", i+1, mapTemp1[i]+1);
    else
      printf("%12d , no match", i+1);
  printf("\n");
  for ( i=0 ; i < n1 ; i++ )
    if ( mapTemp1[i] != -1 )
      printf("%15d  ", contigencyTable[i*n2+mapTemp1[i]]);
    else
      printf("%15d  ", 0);

  printf("\n");
}


/* ------------------------------------------------------------------ */


char * Name(int indexType)
{
  char *str;
  str = (char*) malloc(16*sizeof(char));

  switch (indexType) {
    case INDEX_RI:
      sprintf(str, "RI");
      break;
    case INDEX_ARI:
      sprintf(str, "ARI");
      break;
    case INDEX_NMI:
      sprintf(str, "NMI");
      break;
    case INDEX_NVI:
      sprintf(str, "NVI");
      break;
    case INDEX_MI:
      sprintf(str, "MI");
      break;
    case INDEX_VI:
      sprintf(str, "VI");
      break;
    case INDEX_NVD:
      sprintf(str, "NVD");
      break;
    case INDEX_JACCARD:
      sprintf(str, "JACCARD");
      break;
    case INDEX_FM:
      sprintf(str, "FM");
      break;
    case INDEX_CI:
      sprintf(str, "CI");
      break;
    case INDEX_CIpart:
      sprintf(str, "CI");
      break;
    case INDEX_CIstar:
      sprintf(str, "CI*");
      break;
    case INDEX_CH:
      sprintf(str, "Criterion H");
      break;
    default:
      sprintf(str, "Invalid index");
      break;
  }

  return str;
}
