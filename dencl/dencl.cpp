
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "rknng/rknng_lib.h"

extern "C" {
#include "stack.h"
#include "sort.h"
}

#include <float.h>

#include "dencl.hpp"

float knng_dist(kNNGraph *knng, int p1, int p2) {
  DataSet *DS = (DataSet *)knng->DS;
  return distance(DS, p1, p2);
}

/*
 * Calculate density, delta and nearest high density pointer values based on
 * knn graph. To be used for density peaks clustering.
 * */
int knnGraphDPstats(
    // INPUT:
    kNNGraph *knng, int check_knn_for_delta,
    // OUTPUT:
    double **density_p, double **delta_p, int **nearestHighDens_p, Array **neighborhood_peaks_p,
    int calcnbprop) {
  int i, j, N, neighbor, deltaFound;
  N = knng->size;
  double *density = calloc(N, sizeof(double));   // Init to zero
  double *delta = calloc(N, sizeof(double));     // Init to zero
  int *nearestHighDens = calloc(N, sizeof(int)); // Init to zero

  Array *delta_not_found = (Array *)malloc(sizeof(Array));
  initArray(delta_not_found, 1000);
  VoidArr **reverse_knn = malloc(sizeof(VoidArr *) * N);
  int delta_not_found_count = 0;
  int found_in_reverse_knn = 0;
  int found_in_knn = 0;
  double eps = 1e-20;
  double maxDelta = 0.0;
  double max_mean_dist = 0.0;

  float nearest_peak_portion = 0;

  for (i = 0; i < N; i++) {
    double dist_sum = 0.0;
    double mean_dist = 0.0;
    for (j = 0; j < knng->k; j++) {
      // density[i] += sqrt(((double)(knng->list[i].items[j].dist)));
      dist_sum += ((double)(knng->list[i].items[j].dist));
    }
    mean_dist = dist_sum / knng->k;
    if (mean_dist < eps) {
      mean_dist = eps;
    }
    if (max_mean_dist < mean_dist) {
      max_mean_dist = mean_dist;
    }
    // density[i] = 1/mean_dist + 1e-10*i;
    // density[i] = pow(2.71828182845905,-mean_dist);
    density[i] = mean_dist;
    if (i < 10) {
      printf("i:%d density:%f\n", i, density[i]);
    }
  }
  for (i = 0; i < N; i++) {
    // density[i]=pow(2.71828182845905,-density[i]/max_mean_dist);
    // 1e-10 * i: Hack to make sure no two identical density values
    // TODO: find better fix
    density[i] = 1 / density[i] + 1e-10 * i; //

    if (i < 10) {
      printf("i:%d density:%f\n", i, density[i]);
    }
  }

  // Construct reverse k nearest neighbors.
  for (i = 0; i < N; i++) {
    initVoidArr(&(reverse_knn[i]), knng->k * 2);
  }
  // TODO: use dynamical instead of k*2??
  for (i = 0; i < N; i++) {
    for (j = 0; j < knng->k; j++) {
      revn *r = malloc(sizeof(revn));
      // r->id = knng->list[i].items[j].id;
      r->id = i;
      r->dist = knng->list[i].items[j].dist;
      int neighbor = knng->list[i].items[j].id;
      insertVoidArr(reverse_knn[neighbor], (void *)r);
    }
  }

  printf("time[density]=%fs\n", get_elapsed_time());

  int found_delta_for = 0;
  for (i = 0; i < N; i++) {
    deltaFound = 0;
    // Check if a point with higher density is among k nearest neighbors
    if (check_knn_for_delta && 1) {
      for (j = 0; j < knng->k; j++) {
        neighbor = knng->list[i].items[j].id;
        if (density[neighbor] > density[i]) {
          found_in_knn++;
          delta[i] = knng->list[i].items[j].dist;
          if (maxDelta < delta[i]) {
            maxDelta = delta[i];
          }
          nearestHighDens[i] = knng->list[i].items[j].id;
          deltaFound = 1;
          break;
        }
      }
    }

    /* Check reverse kNN for points with higher density
    (experimental, disabled)
    */
    if (deltaFound == 0 && 0) {
      revn *r;
      debug("reverse kNN COUNT: %d ", reverse_knn[i]->count);
      for (j = 0; j < reverse_knn[i]->count; j++) {
        r = (revn *)reverse_knn[i]->array[j];
        if (density[r->id] > density[i]) {
          debug("found:[%d d:%f dens:%f]", r->id, r->dist, density[i]);
          // Update if first time or found closer high density point
          if (delta[i] > r->dist || deltaFound == 0) {
            nearestHighDens[i] = r->id;
            delta[i] = r->dist;
            deltaFound = 1;
            debug("u");
          }
          debug(" ");
        }
      }
      debug("\n");
      if (deltaFound == 1) {
        found_in_reverse_knn++;
      }
    }

    if (deltaFound == 0) {
      insertArray(delta_not_found, i);
    }
  }
  found_delta_for = found_in_knn + found_in_reverse_knn;
  printf("Found deltas for %d/%d points among %d neighbors\n", found_delta_for, N, knng->k);
  printf("delta_in_knn=%d delta_in_rev_knn=%d found_delta_total=%d delta_not_found=%d\n",
         found_in_knn, found_in_reverse_knn, found_delta_for, delta_not_found->count);

  printf("time[delta_knn]=%fs\n", get_elapsed_time());

  minFind dlt;
  float dist;

  int highest_delta_id = -1;
  for (i = 0; i < delta_not_found->count; i++) {
  
    init_minFind(&dlt);
    int curid = delta_not_found->array[i];
    for (j = 0; j < N; j++) {
      if (curid == j) {
        continue;
      }
      int candidate = j;
      dist = knng_dist(knng, curid, candidate);

      // Keep track of minimum distance to higher density
      if (density[curid] < density[candidate]) {
        checkMin(&dlt, candidate, dist);
      }
      // printf("[%d] min:%f id:%d\n",i,dlt.min_val,dlt.min_id);
    }
    if (dlt.min_id == -1) {
      // No higher density point found
      delta[curid] = maxDelta; // TODO
      highest_delta_id = curid;
    } else {
      delta[curid] = dlt.min_val;
      if (maxDelta < delta[curid]) {
        maxDelta = delta[curid];
      }
      nearestHighDens[curid] = dlt.min_id;
    }
    // printf("%d ",a->array[i]);

    if (calcnbprop > 0) {
      nearest_peak_portion += calc_nearest_peak_portion(curid, knng, delta_not_found) /
                              ((float)(delta_not_found->count));
    }
  }

  assert(highest_delta_id >= 0);
  delta[highest_delta_id] = maxDelta;

  printf("time[delta_bf]=%fs\n", get_elapsed_time());

  if (calcnbprop > 0) {
    printf("nearest_peak_portion=%f\n", nearest_peak_portion);
  }

  // Return values
  (*delta_p) = delta;
  (*nearestHighDens_p) = nearestHighDens;
  (*density_p) = density;
  (*neighborhood_peaks_p) = delta_not_found;

  return 1;
}

static int cmpDoubleDesc(const void *a, const void *b, const void *info) {
  int id1 = *((int *)a);
  int id2 = *((int *)b);
  return (((double *)info)[id1] > ((double *)info)[id2] ? 1 : 0);
}

int densityPeaks(
    // INPUT:
    int N, int K, double *density, double *delta, int *nearestHighDens,
    // OUTPUT:
    int **peaks_out, int **label_out) {
  int i, j;
  int *idx = malloc(sizeof(int) * N);
  int *deltaOrder = malloc(sizeof(int) * N);
  int *peaks = malloc(sizeof(int) * K);
  double *gamma = malloc(sizeof(double) * N);
  int *visited = calloc(N, sizeof(int)); // Init to zero
  // int* isPeak = calloc(N,sizeof(int)); // Init to zero
  int *label = calloc(N, sizeof(int)); // Init to zero
  STACK *S = S_make();

  printf("time=%fs\n", get_elapsed_time());

  for (i = 0; i < N; i++) {
    idx[i] = i;
    deltaOrder[i] = i;
    gamma[i] = delta[i] * density[i];
  }

  delta = gamma;

  QuickSort(deltaOrder, N, sizeof(int), delta, cmpDoubleDesc);
  printf("First %d peaks:", K);

  for (i = 0; i < K; i++) {
    // printf(" %d %f\n",deltaOrder[i],delta[deltaOrder[i]]);
    printf("%d ", deltaOrder[i], delta[deltaOrder[i]]);
    peaks[i] = deltaOrder[i];
    // isPeak[deltaOrder[i]] = i+1;
    label[deltaOrder[i]] = i + 1;
  }
  printf("\n");

  Array trail;
  initArray(&trail, 1000);

  // printf("labels=[");
  // Set cluster labels based on nearest higher density point.
  int curnode = 2;
  for (i = 0; i < N; i++) {
    // if(visited[i] == 1 || label[i] > 0) {continue;}
    // Label already set
    if (label[i] > 0) {
      // printf(" %d",label[i]);
      continue;
    }
    curnode = i;
    while (1) {
      insertArray(&trail, curnode);
      curnode = nearestHighDens[curnode];
      if (label[curnode] > 0) {
        for (j = 0; j < trail.count; j++) {
          label[trail.array[j]] = label[curnode];
          visited[trail.array[j]] = 1;
        }
        trail.count = 0; // empty array
        break;
      }
    }
    // printf(" %d",label[i]);
  }
  // printf("];\n====================================\n");

  free(idx);
  free(deltaOrder);
  free(visited);
  // free(isPeak);

  // Return
  (*label_out) = label;
  (*peaks_out) = peaks;

  // free(peaks);
  // free(label);

  return 1;
}

void WriteDensityPeaksInfo(char *FileName, int *nearestHighDens, double *dens, double *delta,
                           int *N) {
  int i;
  FILE *f;

  // f = FileOpen(FileName, OUTPUT, 1);
  f = fopen(FileName, "w");

  for (i = 0; i < N; i++) {
    fprintf(f, "%d %f %f\n", nearestHighDens[i], dens[i], delta[i]);
    // printf("%i %i\n",i, list[i]);
  }
  fclose(f);
}

/************************************************************/
// Based on
// https://stackoverflow.com/questions/3536153/c-dynamically-growing-array

void initArray(Array *a, size_t initialSize) {
  a->array = (int *)malloc(initialSize * sizeof(int));
  a->count = 0;
  a->size = initialSize;
}

void printArray(Array *a) {
  int i;
  printf("[%d]: ", a->count);
  for (i = 0; i < a->count; i++) {
    printf("%d ", a->array[i]);
  }
  printf("\n");
}

void insertArray(Array *a, int element) {
  // a->count is the number of count entries, because a->array[a->count++] updates a->count only
  // *after* the array has been accessed. Therefore a->count can go up to a->size
  if (a->count == a->size) {
    a->size *= 2;
    a->array = (int *)realloc(a->array, a->size * sizeof(int));
    if (a->array == NULL) {
      printf("realloc error\n");
      exit(0);
    }
  }
  a->array[a->count++] = element;
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->count = a->size = 0;
}
/*****/
// struct VoidArr {
// int *array;
// size_t count;
// size_t size;
// };

void initVoidArr(VoidArr **_a, size_t initialSize) {
  (*_a) = (VoidArr *)malloc(sizeof(VoidArr));
  VoidArr *a = *_a;

  a->array = (int *)malloc(initialSize * sizeof(void *));
  a->count = 0;
  a->size = initialSize;
}

void insertVoidArr(VoidArr *a, int element) {
  // a->count is the number of count entries, because a->array[a->count++] updates a->count only
  // *after* the array has been accessed. Therefore a->count can go up to a->size
  if (a->count == a->size) {
    a->size *= 2;
    a->array = (void *)realloc(a->array, a->size * sizeof(void *));
    if (a->array == NULL) {
      printf("realloc error\n");
      exit(0);
    }
  }
  a->array[a->count++] = element;
}

void freeVoidArr(Array *a) {
  free(a->array);
  a->array = NULL;
  a->count = a->size = 0;
}

/************************************************************/

void init_minFind(minFind *mf) {
  mf->min_id = -1;
  mf->min_val = FLT_MAX;
  // mf->min_val=1e10; //TODO
}

void checkMin(minFind *mf, int id, float val) {
  if (val < mf->min_val) {
    mf->min_val = val;
    mf->min_id = id;
  }
}

// Calculate portion of neighbors for which this peak is the nearest neighborhood peak
// Expected to be around 1.0
float calc_nearest_peak_portion(int itemid, kNNGraph *knng, Array *neighborhood_peaks) {
  int i, j, success;
  float portion, dist, dist_alt;
  success = 0;

  for (i = 0; i < knng->k; i++) {
    int neighbor = knng->list[itemid].items[i].id;
    dist = knng_dist(knng, itemid, neighbor);

    // Calc distances to all other neighborhood peaks
    for (j = 0; j < neighborhood_peaks->count; j++) {
      int peak = neighborhood_peaks->array[j];
      if (peak == itemid) {
        continue;
      }
      dist_alt = knng_dist(knng, peak, neighbor);
      if (dist_alt < dist) {
        // If distance from neighbor to some other peak is smaller than
        // distance to this peak
        success--;
        break;
      }
    }
    success++;
  }
  portion = ((float)success) / (knng->k);
  printf("itemid=%d success=%d portion:%f\n", itemid, success, portion);

  return portion;
}

// From
// http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=/com.qnx.doc.neutrino_lib_ref/v/vprintf.html
int quiet_level = 0;
void debug(const char *format, ...) {
  va_list arglist;

  if (quiet_level > 0) {
    // printf( "[DEBUG]:");
    va_start(arglist, format);
    vprintf(format, arglist);
    va_end(arglist);
  }
}
