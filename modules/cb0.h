#if ! defined(__CB_H)
#define __CB_H

#include "owntypes.h"

/*----------------------- Type definitions ---------------------------*/

typedef  enum { NOTFOUND=0, TSFILE=1, CBFILE=2, PAFILE=3 } CBFILETYPE;
typedef  enum { EUCLIDEANSQ=0, ENTROPYDIST=1,
                JACCARD=11, DICE=12 } DISTANCETYPE;
typedef  enum { MSE=0, SC=1, JACCARDERROR=11, DICEERROR=12,
                                DBI=2 } ERRORFTYPE;
#define  FormatNameCB    "cb"
#define  FormatNameTS    "ts"
#define  FormatNamePA    "pa"
#define  FormatName(ft)  ((ft)==TSFILE ? FormatNameTS : \
                         ((ft)==CBFILE ? FormatNameCB : \
                         ((ft)==PAFILE ? FormatNamePA : "")))

/*---------------------  Vector definitions  -------------------------*/

typedef     int                  VECTORELEMENT;
typedef     VECTORELEMENT*       VECTORTYPE;
typedef     struct { VECTORTYPE  vector;
                     VECTORELEMENT vmean;
                     int         freq;
                     char*       name;
                   } BOOKNODE;
typedef     BOOKNODE*            BOOKTYPE;

/*---------------------  Counter definition  -------------------------*/

typedef     struct { llong*      counter;
                     int         freq;
                   } COUNTER;

/*---------------------  Codebook data structure  --------------------*/

#define     MaxVersionLength     80
#define     MaxGenMethodLength   80
#define     MaxVectorNameLength  80
typedef     struct { char        Versionstr[MaxVersionLength];
                     int         BlockSizeX;
                     int         BlockSizeY;
                     int         CodebookSize;
                     int         TotalFreq;
                     int         BytesPerElement;
                     int         MinValue;
                     int         MaxValue;
                     int         Preprocessing;
                     char        GenerationMethod[MaxGenMethodLength];
                     BOOKTYPE    Book;
                     int         AllocatedSize;
                   } CODEBOOK;

/*---------------------  Training set structure  ---------------------*/

#define    TRAININGSET  CODEBOOK

/*-------------------  Partitioning data structure  ------------------*/

typedef struct { char     Versionstr[MaxVersionLength];
                 int      PartitionCount; /* Number of partitions. */
                 int      TSsize;         /* Number of items to map. */
                 int*     Map;            /* Cursors from TS to CB. */
                 int*     First;   /* Cursor to first element in partition. */
                 int*     Next;           /* Next item in partition. */
                 COUNTER* CC;             /* Centroid counters */
                 int*     Uniques;        /* Number of unique vectors in p */
                 int      Vsize;          /* Vector size (dimension) */
                 char     GenerationMethod[MaxGenMethodLength];
                 int      AllocatedSize;
               } PARTITIONING;

#define ENDPARTITION      -1

/*-------------------  Graph data structure  -------------------------*/

typedef struct GraphVector { int index;     // Vector index in vectors -table
                             int *data;     // Vector components
                             int *kindices; //table of edges as vertex indexes 
} GraphVector;

typedef struct { int nvec;    // Number of vectors
                 int k;              
                 int dim;     // Vector dimensionality
                 int maxcoord;
                 int mincoord;
                 GraphVector **vectors;  // Table of vector pointers
} Graph;




/*-------------------  Vector level routines  ------------------------*/

VECTORTYPE CreateEmptyVector(int Vsize);
void       FreeVector(VECTORTYPE v);
void       CopyVector(VECTORTYPE source, VECTORTYPE dest, int Vsize);
void       PrintVector(VECTORTYPE v, int Xdim, int Ydim);
int        CompareVectors(VECTORTYPE v1, VECTORTYPE v2, int Vsize);
llong      VectorDistance(VECTORTYPE   v1,
                          VECTORTYPE   v2,
                          int          Vsize,
                          llong        maxdist,
                          DISTANCETYPE disttype);

#define    VectorDist(v1,v2,Vsize)  \
           VectorDistance(v1,v2,Vsize,MAXLLONG,EUCLIDEANSQ)

#define    EqualVectors(v1,v2,Vsize) ( CompareVectors(v1,v2,Vsize) == 0 )
#define    VectorGreaterThan(v1,v2,Vsize) ( CompareVectors(v1,v2,Vsize) > 0 )

/*--------------------  Node level routines  -------------------------*/

#define    Node(CB,index)                 ((CB)->Book[index])
#define    Vector(CB,index)               ((CB)->Book[index].vector)
#define    VectorScalar(CB,index,scalar)  ((CB)->Book[index].vector[scalar])
#define    VectorMean(CB,index)           ((CB)->Book[index].vmean)
#define    VectorFreq(CB,index)           ((CB)->Book[index].freq)
#define    VectorName(CB,index)           ((CB)->Book[index].name)

#define    VectorSize(CB)  ((int)((CB)->BlockSizeX * (CB)->BlockSizeY))
#define    BookSize(CB)    ((CB)->CodebookSize)
#define    TotalFreq(CB)   ((CB)->TotalFreq)

BOOKNODE   CreateEmptyNode(int Vsize);
void       FreeNode(BOOKNODE node);
void       CopyNode(BOOKNODE* source, BOOKNODE* dest, int Vsize);
void       ChangeVectorName(BOOKNODE* node, char* NewName);

/*--------------------  Codebook interface  --------------------------*/

CBFILETYPE DetermineCBFileTypeConsideringOrder(char* FileName, int order);
#define    DetermineCBFileType(FileName) \
           DetermineCBFileTypeConsideringOrder(FileName, 123)
void       ReadCodebook(char* FileName, CODEBOOK* CB);
void       WriteCodebook(char* FileName, CODEBOOK* CB, int AllowOverWrite);
void       PrintCodebook(CODEBOOK* CB);
void       CreateNewCodebook(CODEBOOK*    CB,
                             int          booksize,
                             TRAININGSET* TS);
void       CreateNewTrainingSet(TRAININGSET* TS,
                                int          booksize,
                                int          BlockSizeX,
                                int          BlockSizeY,
                                int          BytesPerElement,
                                int          MinValue,
                                int          MaxValue,
                                int          Preprocessing,
                                char*        GenerationMethod);
void       FreeCodebook(CODEBOOK* CB);
void       UnrollTrainingSet(TRAININGSET* TS);
int        DuplicatesInCodebook(CODEBOOK* CB);
void       CodebookCentroid(CODEBOOK* CB, VECTORTYPE v);
void       CalculateVectorMeans(CODEBOOK* CB);

void       CopyCodebookHeader(CODEBOOK* sourceCB, CODEBOOK* destCB);
void       CopyCodebook(CODEBOOK* sourceCB, CODEBOOK* destCB);
void       AddCodebook(CODEBOOK *ToCB, CODEBOOK *FromCB);

void       SetAllocatedCodebookSize(CODEBOOK* CB, int newsize);
void       ChangeCodebookSize(CODEBOOK* CB, int newsize);
void       IncreaseCodebookSize(CODEBOOK* CB, int newsize);
void       DecreaseCodebookSize(CODEBOOK* CB, int newsize);

/*------------------  String handling routines  ----------------------*/

/* these macros accept also a partitioning as the codebook parameter  */

/* use these:    SetVersionString(CB*|P*, filetype)                   */
/*               ClearGenerationMethod(CB*|P*)                        */
/*               AddGenerationMethod(CB*|P*, NewName)                 */
/*               RecognizeCBFileType(CB*|P*)                          */

#define SetVersionString(CB, filetype) \
        SetVerString( (CB)->Versionstr, filetype)

#define ClearGenerationMethod(CB) \
        ClearString( (CB)->GenerationMethod )

#define AddGenerationMethod(CB, NewName) \
        AddString( (CB)->GenerationMethod, NewName, MaxGenMethodLength)

#define RecognizeCBFileType(CB) \
        RecognizeVersionstr( (CB)->Versionstr)

/*------------------  Training set interface  ------------------------*/

#define    ReadTrainingSet(name, CB)        ReadCodebook(name, CB)
#define    WriteTrainingSet(name, CB, ow)   WriteCodebook(name, CB, ow)

/*------------------  Partitioning interface  ------------------------*/

void CreateNewPartitioning(PARTITIONING* P,
                           TRAININGSET*  TS,
                           int           PartitionCount);
void FreePartitioning(PARTITIONING* P);
void SetNumberOfAllocatedPartitions(PARTITIONING* P, int newsize);
void IncreaseNumberOfPartitions(PARTITIONING* P, int newsize);
void DecreaseNumberOfPartitions(PARTITIONING* P, int newsize);
void ChangeNumberOfPartitions(PARTITIONING* P, int newsize);
void PutAllInOwnPartition(TRAININGSET* TS, PARTITIONING* P);
void ChangePartitionFast(TRAININGSET* TS, PARTITIONING* P, int Vnew, int item, int prev);
void ChangePartition(TRAININGSET* TS, PARTITIONING* P, int Pindex, int item);
void ChangePartitionFast(TRAININGSET* TS, PARTITIONING* P, int Vnew, int item, int prev);
void JoinPartitions(TRAININGSET* TS, PARTITIONING* P, int To, int From);
void PartitionCentroid(PARTITIONING* P, int Pindex, BOOKNODE* centroid);
void FillEmptyPartitions(TRAININGSET* TS, CODEBOOK* CB, PARTITIONING* P);
void ReadPartitioning(char* FileName, PARTITIONING* P, TRAININGSET* TS);
void WritePartitioning(char*         FileName,
                       PARTITIONING* P,
                       TRAININGSET*  TS,
                       int           AllowOverWrite);

void CopyPartitioning(PARTITIONING* sourceP, PARTITIONING* destP);

#define Map(P,item)              ((P)->Map[item])
#define UniqueVectors(P,Pindex)  ((P)->Uniques[Pindex])

#define CCFreq(P,Pindex)     ((P)->CC[Pindex].freq)
#define CCScalar(P,Pindex,k) ((P)->CC[Pindex].counter[k])

#define PartitionCount(P)        ((P)->PartitionCount)

/*-----------------  Partitioning iterator routines  -----------------*/

#define FirstVector(P,Pindex)    ((P)->First[Pindex])
#define NextVector(P,item)       ((P)->Next[item])

#define EndOfPartition(item)     ((item) == ENDPARTITION)
#define IsLast(P,item)           EndOfPartition((P)->Next[item])

/*---------------  GLA & error related routines  ---------------------*/

double  GenerateOptimalPartitioningGeneral(TRAININGSET*  TS,
                                           CODEBOOK*     CB,
                                           PARTITIONING* P,
                                           ERRORFTYPE    errorf);

void    GenerateOptimalCodebookGeneral    (TRAININGSET*  TS,
                                           CODEBOOK*     CB,
                                           PARTITIONING* P,
                                           ERRORFTYPE    errorf);

void    LocalRepartitioningGeneral        (TRAININGSET*  TS,
                                           CODEBOOK*     CB,
                                           PARTITIONING* P,
                                           int           clusterindex,
                                           DISTANCETYPE  disttype);

void    RepartitionDueToNewVectorsGeneral (TRAININGSET*  TS,
                                           CODEBOOK*     CB,
                                           PARTITIONING* P,
                                           int*          codevectorindices,
                                           int           codevectorcount,
                                           DISTANCETYPE  disttype);

void    RepartitionDueToNewVectorGeneral  (TRAININGSET*  TS,
                                           CODEBOOK*     CB,
                                           PARTITIONING* P,
                                           int           codevectorindex,
                                           DISTANCETYPE  disttype);

int     FindNearestVector                 (BOOKNODE*     v,
                                           CODEBOOK*     CB,
                                           llong*        error,
                                           int           guess,
                                           DISTANCETYPE  disttype);

double  AverageErrorForSolution           (TRAININGSET*  TS,
                                           CODEBOOK*     CB,
                                           PARTITIONING* P,
                                           ERRORFTYPE    errorf);

double  AverageErrorForPartitioning       (TRAININGSET*  TS,
                                           PARTITIONING* P,
                                           ERRORFTYPE    errorf);

double  AverageErrorCBFast                (TRAININGSET*  TS,
                                           CODEBOOK*     CB,
                                           PARTITIONING* P,
                                           ERRORFTYPE    errorf);

#define GenerateOptimalPartitioning(TS, CB, P) \
        GenerateOptimalPartitioningGeneral(TS, CB, P, MSE)

#define GenerateOptimalCodebook(TS, CB, P) \
        GenerateOptimalCodebookGeneral(TS, CB, P, MSE)

#define LocalRepartitioning(TS, CB, P, ind) \
        LocalRepartitioningGeneral(TS, CB, P, ind, EUCLIDEANSQ)

#define RepartitionDueToNewVector(TS, CB, P, ind) \
        RepartitionDueToNewVectorGeneral(TS, CB, P, ind, EUCLIDEANSQ)

#define RepartitionDueToNewVectors(TS, CB, P, inds, c) \
        RepartitionDueToNewVectorsGeneral(TS, CB, P, inds, c, EUCLIDEANSQ)

#define AverageErrorCB(TS,CB,errorf) AverageErrorCBFast(TS,CB,NULL,errorf)

double  PrintableError(double error, CODEBOOK* CB);

DISTANCETYPE DistType(ERRORFTYPE errorf);

/*------------------  General string routines  -----------------------*/

void       SetVerString(char* str, CBFILETYPE filetype);
void       ClearString(char* str);
void       AddString(char* str, char* NewName, int maxlength);
CBFILETYPE RecognizeVersionstr(char* str);

/*------------------  This one requires random module  ---------------*/

#define ShuffleTS(TS) \
  ShuffleMemory( (void*) (TS)->Book, BookSize(TS), sizeof(BOOKNODE) )

/*--------------------------------------------------------------------*/

/*------------------  Graph API  -------------------------------------*/


int GraphPutNN(Graph *g, GraphVector *v, int index, int i);
int GraphPutVector(Graph *g, int ith_vector, int *data);
int GraphPutVectorElement(GraphVector *v, int jth_element, int data);
int GraphGetVectorIndex(GraphVector *v);
int GraphGetMaxCoord(Graph *g);
int GraphGetMinCoord(Graph *g);
int FreeGraphVector(Graph *g, int i);
int FreeGraph(Graph *g);

Graph *AllocateMemoryForGraph(int number_vectors, int k, int dim);
GraphVector *AllocateMemoryForGraphVector(Graph *g, int index); 

int IsGraphFile(char* FileName);
Graph *GraphRead(char* FileName);
void GraphWrite(char* FileName, Graph *g, int AllowOverWrite);

int GraphPrint(Graph *g);

int GraphGetK(Graph *g);
int GraphPutK(Graph *g, int k);
int GraphGetNumberVectors(Graph *g);
int GraphPutNumberVectors(Graph *g, int nvec);
int GraphGetDim(Graph *g);
int GraphPutDim(Graph *g, int dim);

int *GraphGetVectorCoord(GraphVector *v);
GraphVector *GraphGetVector(Graph *g, int i);
GraphVector *GraphGetNNVector(Graph *g, GraphVector *v, int i); 
int IsGraphEdge(Graph *g, GraphVector *src, GraphVector *dest); 



#endif /* __CB_H */
