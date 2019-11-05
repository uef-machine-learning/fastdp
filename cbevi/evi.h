/*
$Revision: 0.05 $
$Date: 17.8.2017 12:45:25 $
$Author: Sami Sieranoja $
$Name:  $
$Id: evi.h,v 0.01 8.7.2015 12:45:25 Mohammad Rezaei Exp $
*/

#if ! defined(__EVI_H)
#define __EVI_H

#define MAXFILENAME     1024    /* maximum length of filename */

#define INDEX_RI       1      // Rand Index
#define INDEX_ARI      2      // Adjusted Rand Index
#define INDEX_NMI      3      // Normalized Mutual Information
#define INDEX_NVI      4      // Normalized Variation of Information
#define INDEX_NVD      5      // Normalized Van Dongen
#define INDEX_JACCARD  6      // JACCARD
#define INDEX_FM       7      // Fowlkes and Mallows
#define INDEX_CI       8      // Centroid Index
#define INDEX_CIstar   9      // Point-wise Centroid Index
#define INDEX_CH       10     // Criterion H
#define INDEX_MI       11     // Mutual Information
#define INDEX_VI       12     // Variation of Information
#define INDEX_CIpart   13    // Centroid Index (based on partitions)


int PerformEVI(TRAININGSET   *TS, PARTITIONING *pP1, PARTITIONING *pP2, CODEBOOK *pCB1, CODEBOOK *pCB2, int indexType, int quietLevel, int useInitial);
double EVI_RI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_ARI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_NMI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_NVI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_MI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_VI(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_NVD(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_JACCARD(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_FM(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_CH(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_CI(CODEBOOK *pCB1, CODEBOOK *pCB2, int quietLevel);
double EVI_CIpart(PARTITIONING *pP1, PARTITIONING *pP2, int quietLevel);
double EVI_CI_testClusterSizes(TRAININGSET   *TS, CODEBOOK* pCB1, CODEBOOK* pCB2, int quietLevel);
double EVI_CISTAR(TRAININGSET   *TS, CODEBOOK *pCB1, CODEBOOK *pCB2, int quietLevel);

int Contingency(PARTITIONING *pP1, PARTITIONING *pP2, int *contigencyTable);
void getSumSquares(PARTITIONING *pP1, PARTITIONING *pP2, int *contigencyTable, llong *nis, llong *njs, llong *nijs);
void getMutualInformation(PARTITIONING *pP1, PARTITIONING *pP2, double *H1, double*H2, double *MI);
llong SumSqRowsOrColumns(int *contigencyTable, int n1, int n2, int rowOrColumn);

int DetermineFileType(char *fileName);
char* EVIInfo(void);
char * Name(int indexType);

int getPartitionsFromFiles(char *InName1, char *InName2, char *InName3, TRAININGSET   *TS, PARTITIONING *pP1, PARTITIONING *pP2);
int getCentroidsFromFiles(char *InName1, char *InName2, char *InName3, TRAININGSET   *TS, CODEBOOK *pCB1, CODEBOOK *pCB2);
int GlobalAllocations(TRAININGSET* pCB1, CODEBOOK* pCB2, PARTITIONING* P, int quietLevel);
int GlobalAllocations_testClusterSizes(TRAININGSET* pCB1, CODEBOOK* pCB2, PARTITIONING* P, int quietLevel, PARTITIONING *P1, PARTITIONING *P2);
double EVI_CI(CODEBOOK* pCB1, CODEBOOK* pCB2, int quietLevel);
double EVI_CISTAR(TRAININGSET   *TS, CODEBOOK* pCB1, CODEBOOK* pCB2, int quietLevel);
void CalculateMAEandNMatches(CODEBOOK* pCB1, CODEBOOK* pCB2, PARTITIONING* P, double* MAE, int* nMatches);

int CheckParameters(char *InName1, char *InName2, char *InName3, int indexType);

int DetermineFileName(char *name);
double SetClock(double* start);
double GetClock(double start);
void PrintHeader(int quietLevel);
void PrintResultsEVI(int quietLevel, double indexeValue, int indexType, double time);
void PrintFooterKM(int quietLevel, double error, int repeats,
    double totalTime, int totalIter);
void PrintFooterEVI(int quietLevel, int iter, double error, double time);
void printClustersInfo(int *mapTemp1, int *mapTemp2, int n1, int n2, int *contigencyTable, int size);

#endif /* __EVI_H */
