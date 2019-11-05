#ifndef DENCL_H
#define DENCL_H

typedef struct Array Array;
typedef struct VoidArr VoidArr;
typedef struct minFind minFind;

struct minFind {
  int min_id;
  float min_val;
};

struct Array {
  int *array;
  size_t count;
  size_t size;
};

struct VoidArr {
  int *array;
  size_t count;
  size_t size;
};

typedef struct {
  int id;
  float dist;
} revn;


float calc_nearest_peak_portion(int itemid, kNNGraph *knng, Array *neighborhood_peaks);
int knnGraphDPstats(
    // INPUT:
    kNNGraph *knng, int check_knn_for_delta,
    // OUTPUT:
    double **density_p, double **delta_p, double **nearestHighDens_p, Array **neighborhood_peaks_p,
    int calcnbprop);
static int cmpDoubleDesc(const void *a, const void *b, const void *info);
int densityPeaks(
    // INPUT:
    int N, int K, double *density, double *delta, int *nearestHighDens,
    // OUTPUT:
    int **peaks_out, int **label_out);
void WriteDensityPeaksInfo(char *FileName, int *nearestHighDens, double *dens, double *delta,
                           int *N);
int main(int argc, char *argv[]);

void debug(const char *format, ...);
void initArray(Array *a, size_t initialSize);
void printArray(Array *a);
void insertArray(Array *a, int element);
void freeArray(Array *a);
void initVoidArr(VoidArr **_a, size_t initialSize);
void insertVoidArr(VoidArr *a, int element);
void freeVoidArr(Array *a);
void init_minFind(minFind *mf);
void checkMin(minFind *mf, int id, float val);

#endif
