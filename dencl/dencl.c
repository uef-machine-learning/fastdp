#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "rknng/rknng_lib.h"

#include "cb.h"
#include "file.h"
#include "stack.h"
#include "cbevi/evi.h"

#include <float.h>

#include "argtable3.h"
//#include "contrib/libstrcmp/src/distance/levenshtein.c"
#include "distance/levenshtein.c"
#include "dencl.h"






/*
 * Calculate density, delta and nearest high density pointer values based on
 * knn graph. To be used for density peaks clustering.
 * */
int knnGraphDPstats(
    // INPUT:
    kNNGraph *knng, int check_knn_for_delta,
    // OUTPUT:
    double **density_p, double **delta_p, double **nearestHighDens_p, Array **neighborhood_peaks_p,
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
      dist_sum += sqrt(((double)(knng->list[i].items[j].dist)));
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
      r->dist = sqrt(knng->list[i].items[j].dist);
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
          delta[i] = sqrt(knng->list[i].items[j].dist);
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
      dist = sqrt(knng_dist(knng, curid, candidate));

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

  f = FileOpen(FileName, OUTPUT, 1);
  for (i = 0; i < N; i++) {
    fprintf(f, "%d %f %f\n", nearestHighDens[i], dens[i], delta[i]);
    // printf("%i %i\n",i, list[i]);
  }
  fclose(f);
}

// TODO:
// extern struct knng_options g_options;

int main(int argc, char *argv[]) {
  int i;
  TRAININGSET *TS = (TRAININGSET *)malloc(sizeof(TRAININGSET));
  kNNGraph *knng = NULL;
  int neighbors = 20;
  int clusters = 256;
  int data_type = 1;
  int verbose = 1;
  int knng_algo = 0;
  int num_iter = 100; // Run RP-div for maxknum_iter iterations
  int W = 50;
  float endcond = 0.01;    // For RP-div knng algorithm
  float start_nndes = 0.1; // For RP-div knng algorithm

  struct arg_file *infn;
  struct arg_file *out_pa_fn;
  struct arg_file *out_cb_fn;
  struct arg_file *gt_pa_fn;

  struct arg_file *out_densdelta_fn;

  struct arg_int *numNeighbors;
  struct arg_int *bfsize;
  struct arg_int *numClusters;
  struct arg_str *dtype;
  struct arg_int *rngSeed;
  struct arg_int *verboseArg;
  struct arg_int *calcnbprop;
  struct arg_int *a_num_iter;

  struct arg_str *distfunc;
  struct arg_str *algo;
  struct arg_dbl *knng_endc;
  struct arg_dbl *knng_start_nndes;

  struct arg_lit *help;
  struct arg_end *end;

  // printf("ll:%d\n", levenshtein("abcde", "a2zde"));

  void *argtable[] = {
      help = arg_litn(NULL, "help", 0, 1, "display this help and exit"),

      numNeighbors = arg_intn(NULL, "neighbors", "<n>", 0, 1, "knn neighbors"),
      bfsize = arg_intn("W", "bfsize", "W", 0, 1, "use bf when < W"),
      a_num_iter = arg_intn("I", "num_iter", "<n>", 0, 1, "End condition: Number of RKNNG iterations  "),
      numClusters = arg_intn(NULL, "clusters", "<n>", 0, 1, "number of clusters"),
      rngSeed = arg_intn(NULL, "seed", "<n>", 0, 1, "random number seed"),
      verboseArg = arg_intn("Q", "verbose", "<n>", 0, 10, "Verbose level"),
      calcnbprop = arg_intn("", "calcnbprop", "<n>", 0, 10,
                            "Calculate neighborhood peak proportion (diagnostic)"),
      distfunc = arg_str0(NULL, "dfunc", "<FUNC>",
                          "Distance function:\n"
                          "     l2 = euclidean distance (vectorial, default)\n"
                          "     mnkw = Minkowski distance (vectorial)\n"
                          "     lev = Levenshtein distance (for strings, default)\n"
                          "     dice = Dice coefficient / bigrams (for strings)\n"),
      out_pa_fn = arg_filen(NULL, "out-pa", "<file>", 0, 1, "output partition file"),
      out_densdelta_fn = arg_filen(NULL, "out-dd", "<file>", 0, 1, "output density/delta filename"),
      gt_pa_fn = arg_filen(NULL, "gt-pa", "<file>", 0, 1, "Ground truth partition file"),
      out_cb_fn = arg_filen(NULL, "out-cb", "<file>", 0, 1, "output centroids file"),
      dtype = arg_str0(NULL, "type", "<vec|txt>", "Input data type: vectorial or text."),
      algo = arg_str0(NULL, "knng-algo", "<rpdiv|brutef>",
                      "k-nearest neighbor graph algorithm: RP-Div or Bruteforce"),
      knng_endc =
          arg_dbln(NULL, "knng-delta", "<FLOAT>", 0, 1, "Stop when delta < STOP (knng/RP-div)"),
      knng_start_nndes =
          arg_dbln(NULL, "knng-nndes", "START", 0, 1, "Start using nndes when delta < START"),
      infn = arg_filen(NULL, NULL, "<file>", 1, 1, "input files"), end = arg_end(20),
  };

  int ok = 1;
  int nerrors = arg_parse(argc, argv, argtable);

  if (rngSeed->count > 0) {
    printf("Set RNG seed: %d\n", rngSeed->ival[0]);
    srand(rngSeed->ival[0]);
  } else {
    srand(time(NULL));
  }

  if (numNeighbors->count > 0) {
    neighbors = numNeighbors->ival[0];
  }
  W = (int)neighbors * 2.5;
  if (bfsize->count > 0) {
    W = bfsize->ival[0];
  }
  
  if (a_num_iter->count > 0) {
    num_iter = a_num_iter->ival[0];
  }
  // debug("num_iter=%d\n", num_iter);

  if (knng_endc->count > 0) {
    endcond = (float)knng_endc->dval[0];
  }
  if (knng_start_nndes->count > 0) {
    start_nndes = (float)knng_start_nndes->dval[0];
  }

  if (verboseArg->count > 0) {
    verbose = verboseArg->ival[0];
  }

  if (numClusters->count > 0) {
    clusters = numClusters->ival[0];
  }

  int dfunc = 0;
  if (distfunc->count > 0) {
    if (strcmp(distfunc->sval[0], "l2") == 0) {
      printf("Distance function: %s\n", distfunc->sval[0]);
      dfunc = 0;
    } else if (strcmp(distfunc->sval[0], "mnkw") == 0) {
      dfunc = 1;
      //      printf("Distance function: minkowski (p=%f)\n", g_options.minkowski_p);
    }
    if (strcmp(distfunc->sval[0], "dice") == 0) {
      dfunc = 10;
      printf("Distance function: Dice\n");
    }
  }

  if (infn->count <= 0) {
    ok = 0;
  }
  if (dtype->count <= 0) {
    ok = 0;
  }
  if (dtype->count > 0 && strcmp(dtype->sval[0], "txt") == 0) {
    data_type = 2;
  } else if (dtype->count > 0 && strcmp(dtype->sval[0], "vec") == 0) {
    data_type = 1;
   } else if (dtype->count > 0 && strcmp(dtype->sval[0], "set") == 0) {
    data_type = 3;
   
  } else {
    printf("Must specify data type: vec|txt\n");
  }

  if (help->count > 0 || ok == 0) {
    printf("Density peaks clustering using knn-graph\n\ndencl");
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    return 0;
  }

  printf("Fast Density Peaks clustering with kNN graph\n");
  printf("infn:%s neighbors:%d clusters:%d data type:%s\n", infn->filename[0], neighbors, clusters,
         dtype->sval[0]);
  double *delta;
  int *nearestHighDens;
  double *density;
  Array *neighborhood_peaks;
  int N = 0;

  int *peaks;
  int *labels;
  // DataSet* DS;

  if (algo->count > 0 && strcmp(algo->sval[0], "brutef") == 0) {
    knng_algo = 9;
  } else if (algo->count == 0 || strcmp(algo->sval[0], "rpdiv") == 0) {
    knng_algo = 0;
  }

  printf("clusters=%d knng_algo=%d knng_endc:%f start_nndes:%f data_type=%d\n", clusters, knng_algo, endcond,
         start_nndes, data_type);
  printf("time[start]=%fs\n", 0.0); // TODO: output inside get_knng??
  knng =
      get_knng(infn->filename[0], neighbors, data_type, knng_algo, endcond, start_nndes, W, dfunc, num_iter);
  printf("time[graph]=%fs\n", get_elapsed_time());
  fflush(stdout);

  N = knng->size;
  if (knng_algo == 0) {
    knnGraphDPstats(knng, 1, &density, &delta, &nearestHighDens, &neighborhood_peaks,
                    calcnbprop->count);
  } else {
    knnGraphDPstats(knng, 0, &density, &delta, &nearestHighDens, &neighborhood_peaks,
                    calcnbprop->count);
  }
  printf("time[deltadens]=%fs\n", get_elapsed_time());
  densityPeaks(
      // INPUT:
      N, clusters, density, delta, nearestHighDens,
      // OUTPUT:
      &peaks, &labels);
  printf("time[total]=%fs\n", get_elapsed_time());
  printf("total_time=%fs", get_elapsed_time());

  PARTITIONING P;
  CreateNewPartitioningWithoutTS(&P, clusters, N); // TODO: Free
  for (i = 0; i < N; i++) {
    Map(&P, i) = labels[i] - 1;
  }

  // WriteIntegerList("npointers.txt",nearestHighDens,N);
  // WriteDoubleList("densities.txt",density,N);
  // WriteDoubleList("deltas.txt",delta,N);

  PARTITIONING gtPart;

  if (gt_pa_fn->count > 0) {
    ReadPartitioningFile(gt_pa_fn->filename[0], &gtPart);
    printf(" ari=%f ri=%f", EVI_ARI(&P, &gtPart, 1), EVI_RI(&P, &gtPart, 1));
    printf("  cipart=%f", EVI_CIpart(&P, &gtPart, 2));
    printf("  nmi=%f", EVI_NMI(&P, &gtPart, 2));
    //"data/countries_sub_K12_n50_tp0100.pa"
  }
  printf(" neighborhood_peaks=%d", neighborhood_peaks->count);
  printf(" num_iter=%d", num_iter);
  
  printf("\n");

  if (neighborhood_peaks->count < 1000) {
    printf("NPEAKS_LIST");
    printArray(neighborhood_peaks);
  }
  freeArray(neighborhood_peaks);

  if (out_densdelta_fn->count > 0) {
    printf("Writing density/delta info to file: %s\n", out_densdelta_fn->filename[0]);

    WriteDensityPeaksInfo(out_densdelta_fn->filename[0], nearestHighDens, density, delta, N);
  }
  if (out_pa_fn->count > 0) {
    printf("Writing Partition info to file: %s\n", out_pa_fn->filename[0]);
    WritePartitioning2(out_pa_fn->filename[0], &P, NULL, 1 /*=AllowOverWrite*/, 1 /*=writeHeader*/);
  }

  free(peaks);
  free(labels);

  printf("END\n");
  return 0;
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

