
#include <cstring>
#include <cmath>
#include <fstream>
#include <csignal> // Raise
#include <fstream>
#include <vector>
#include <float.h>
#include "util.h"

#include "options.h"
extern struct knng_options g_options;

using namespace std;

#define T_NUMERICAL 1
#define T_STRING 2
#define T_SET 3

// TODO:
#define MEM_ALIGNMENT 32
struct DataSet {
  int size;
  int dimensionality;
  int vector_size;
  float elem_min;
  float elem_max;
  float **data;

  vector<string> *strings;
  int **bigrams;
  int **trigrams;

  int *setSize; // For set data
  int type;
};

#include "stringdata.h"

DataSet *init_DataSet(int size, int dimensionality) {

  DataSet *DS = (DataSet *)safemalloc(sizeof(DataSet));
  DS->size = size;
  DS->dimensionality = dimensionality;
  DS->vector_size = DS->dimensionality * sizeof(float);
  DS->data = (float **)safemalloc(sizeof(float *) * DS->size);
  DS->type = T_NUMERICAL; // numerical
  float **datap = DS->data;
  for (int i = 0; i < DS->size; i++) {
    *datap = (float *)safemalloc(sizeof(float) * DS->dimensionality);
    datap++;
  }

  return DS;
}

void free_DataSet(DataSet *DS) {

  if (DS->type == T_NUMERICAL) { // Numerical
    for (int i = 0; i < DS->size; i++) {
      free(DS->data[i]);
    }
    free(DS->data);
  } else if (DS->type == T_STRING) { // String
    // TODO
  }
  free(DS);
}

inline float *get_vector(DataSet *DS, int idx) { return (float *)*(DS->data + idx); }

inline void set_val(DataSet *DS, int data_i, int elem_i, float val) {
  *(*(DS->data + data_i) + elem_i) = val;
}

float get_val(DataSet *DS, int data_i, int elem_i) {
  return *(*(DS->data + data_i) + elem_i);
  /*return DS->data[data_i][elem_i]; //TODO*/
}

inline int dump_vector(DataSet *DS, int idx) {

  float *v = get_vector(DS, idx);

  for (int i = 0; i < DS->dimensionality; i++) {
    printf(" %e", *v);
    v++;
  }
  printf("\n");

  return 0;
}

inline float minkowskiDist(float *p1_idx, float *p2_idx, int D, float p) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    dist += pow(abs(tmp), p);
    p1_idx++;
    p2_idx++;
  }
  // TODO: take 1/p root? No effect on knn search, only scaling.
  /*printf("dist:%f\n",dist);*/

  return dist;
}

// L0.5 distance, without final 1/p root
inline float L05dist(float *p1_idx, float *p2_idx, int D) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    /*dist += tmp * tmp;*/
    dist += pow(tmp, 0.5);
    p1_idx++;
    p2_idx++;
  }
  // TODO: sqrt?

  return dist;
  /*return sqrt(dist);*/
}

// L1 distance
inline float L1dist(float *p1_idx, float *p2_idx, int D) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    /*dist += tmp * tmp;*/
    dist += abs(tmp);
    p1_idx++;
    p2_idx++;
  }
  // TODO: sqrt?

  return dist;
  /*return sqrt(dist);*/
}

inline float L2dist(float *p1_idx, float *p2_idx, int D) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    dist += tmp * tmp;
    p1_idx++;
    p2_idx++;
  }
  // TODO: sqrt?

  return dist;
  /*return sqrt(dist);*/
}

inline float cosine_dist(float *p1_idx, float *p2_idx, int D) {
  float dist = 0;
  float a_sum = 0;
  float b_sum = 0;
  float product = 0;
  for (int i = 0; i < D; i++) {
    product += (*p1_idx) * (*p2_idx);
    a_sum += (*p1_idx) * (*p1_idx);
    b_sum += (*p2_idx) * (*p2_idx);
    p1_idx++;
    p2_idx++;
  }
  dist = 1 - product * 1.0 / (sqrt(a_sum) * sqrt(b_sum));
  return dist;
}

inline float distance(DataSet *P, int p1, int p2) {
  float ret;
  // printf("type=%d\n",P->type);

  if (P->type == T_STRING) // String data
  {
    if (g_options.distance_type == 10) {
      ret = dice_distance_precalc(P, p1, p2);
    } else {
      int distr = edit_distance(P->strings->at(p1), P->strings->at(p2));
      ret = (float)distr;
    }
    return ret;
  }

  else if (P->type == T_SET) {
    ret = dice_set_distance(P, p1, p2);
    return ret;
  } else if (P->type == T_NUMERICAL) {

    float *p1_idx = get_vector(P, p1);
    float *p2_idx = get_vector(P, p2);

    switch (g_options.distance_type) {
    case 0:
      ret = L2dist(p1_idx, p2_idx, P->dimensionality);
      break;
    case 1:
      ret = minkowskiDist(p1_idx, p2_idx, P->dimensionality, g_options.minkowski_p);
      break;
    case 2:
      ret = cosine_dist(p1_idx, p2_idx, P->dimensionality);
      break;

    default:
      exit(1); // TODO
    }

    return ret;
  } else {
    printf("Unknown DS TYPE:'%d'\n", P->type);
    return 0;
  }
}

DataSet *read_lshkit(const char *fname) {

  // From http://lshkit.sourceforge.net/dc/d46/matrix_8h.html :
  // " In the beginning of the file are three 32-bit integers: ELEM-SIZE, SIZE, DIM. ELEM-SIZE is
  // the size of the element, and currently the only valid value is 4, which is the size of float.
  // SIZE is the number of vectors in the file and DIM is the dimension. The rest of the file is
  // SIZE vectors stored consecutively, and the total size is SIZE * DIM * 4 bytes."

  int header[3];
  DataSet *DS = (DataSet *)safemalloc(sizeof(DataSet));

  ifstream infile(fname, ios::binary);
  infile.read((char *)header, sizeof(int) * 3);
  if (header[0] != sizeof(float)) {
    printf("%d %zu", header[0], sizeof(float));
    terminal_error("Error reading input file. Format unknown.");
  }
  printf("Reading file with %d vectors of dimensionality %d\n", header[1], header[2]);

  DS->size = header[1];
  DS->dimensionality = header[2];
  DS->vector_size = DS->dimensionality * sizeof(float);
  DS->data = (float **)safemalloc(sizeof(float *) * DS->size);
  float **datap = DS->data;
  for (int i = 0; i < DS->size; i++) {
    /**datap = (float*) safemalloc(sizeof(float)*DS->dimensionality);*/
    // https://linux.die.net/man/3/memalign
    // TODO
    /**datap = (float*)memalign(MEM_ALIGNMENT, DS->dimensionality * sizeof(float));*/
    *datap = (float *)malloc(DS->dimensionality * sizeof(float));
    if (!*datap) {
      printf("mem align failed: i=%d\n", i);
      exit(1);
    }
    infile.read((char *)(*datap), DS->vector_size);
    datap++;
  }
  printf("Done reading.\n");

  return DS;
}

DataSet *read_ascii_dataset(const char *fname) {

  int N = 0;
  float buf;
  FILE *fp;
  int max_chars = 100000;
  char line[max_chars + 1];
  char *pbuf;
  int i_elem = 0;
  int dim = 0;
  printf("Reading ascii dataset file %s\n", fname);
  fp = fopen(fname, "r");
  if (!fp) {
    terminal_error("File does not exist\n");
  }

  N = count_lines(fp);
  printf("lines=%d\n", N);

  // Get number of elements
  char *ok = fgets(line, max_chars, fp);
  if (ok == NULL) {
    terminal_error("");
  }
  pbuf = line;
  for (i_elem = 0;; i_elem++) {
    if (*pbuf == '\n')
      break;
    buf = strtof(pbuf, &pbuf);
    printf(" %f", buf);
  }
  dim = i_elem;
  printf("\nnum_vectors=%d D=%d\n", N, dim);

  DataSet *DS = (DataSet *)safemalloc(sizeof(DataSet));
  DS->size = N;
  DS->dimensionality = dim;
  DS->vector_size = DS->dimensionality * sizeof(float);
  DS->data = (float **)safemalloc(sizeof(float *) * DS->size);
  DS->type = T_NUMERICAL;

  fseek(fp, 0L, SEEK_SET);
  float **datap = DS->data;
  for (int i_vector = 0; i_vector < N; i_vector++) {
    // TODO: mem alignment needed?
    /**datap = (float*)memalign(MEM_ALIGNMENT, DS->dimensionality * sizeof(float));*/
    *datap = (float *)malloc(DS->dimensionality * sizeof(float));
    if (!*datap) {
      printf("mem align failed: i=%d\n", i_vector);
      exit(1);
    }

    float *col_p = *datap;

    char *ok = fgets(line, max_chars, fp);
    if (ok == NULL) {
      terminal_error("premature end of file");
    }
    /*printf("|%s |\n",line);*/
    pbuf = line;

    for (i_elem = 0;; i_elem++) {
      if (*pbuf == '\n' && i_elem == dim) {
        break;
      } else if (*pbuf == '\n') {
        terminal_error("Got too few elements");
      }
      buf = strtof(pbuf, &pbuf);
      if (i_elem <= dim) {
        *col_p = buf;
        /*if(i_vector==0) {printf("%f ",buf);}*/
      } else {
        terminal_error("");
      }
      col_p++;
    }

    datap++;
  }

  // DEBUG:
  /*for(int i=0;i<3;i++) {*/
  /*for(int j=0;j<DS->dimensionality;j++) {*/
  /*printf("%f ",get_val(DS, i, j));*/
  /*}*/
  /*printf("\n");*/
  /*}*/

  return DS;
}
