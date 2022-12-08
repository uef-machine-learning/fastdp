
#ifndef DATASET_H
#define DATASET_H
#include <cstring>
#include <cmath>
#include <fstream>
#include <csignal> // Raise
#include <fstream>
#include <vector>
#include <float.h>
#include "util.h"

#include "options.h"

#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>

#include <map>
#include <algorithm>

#include "constants.h"

int *calc_ngrams(const char *str1, int ngram);

extern struct knng_options g_options;

using namespace std;

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
  int type;
  int distance_type;
  int *setSize; // For set data
#ifdef Py_PYTHON_H
  PyObject *pyobj;
  PyObject *pydf; // Distance function defined in python
#endif
};


float dice_set_distance(DataSet *sd, int a, int b);
std::istream &safeGetline(std::istream &is, std::string &t);
DataSet *loadSetData(std::string infname);
DataSet *init_DataSet(int size, int dimensionality);
void free_DataSet(DataSet *DS);
float *get_vector(DataSet *DS, int idx);
void set_val(DataSet *DS, int data_i, int elem_i, float val);
float get_val(DataSet *DS, int data_i, int elem_i);
int dump_vector(DataSet *DS, int idx);
float minkowskiDist(float *p1_idx, float *p2_idx, int D, float p);
float L05dist(float *p1_idx, float *p2_idx, int D);
float L1dist(float *p1_idx, float *p2_idx, int D);
float L2dist(float *p1_idx, float *p2_idx, int D);
float cosine_dist(float *p1_idx, float *p2_idx, int D);
float distance(DataSet *P, int p1, int p2);
DataSet *read_lshkit(const char *fname);
DataSet *read_ascii_dataset(const char *fname);
int cmp(const void *a, const void *b);
DataSet *loadStringData(std::string infname);
int *calc_ngrams(const char *str1, int ngram);
double dice_ngram(int *bg1, int *bg2, int setsize1, int setsize2);
float dice_distance_precalc(DataSet *sd, int a, int b);
double dice(const char *str1, const char *str2, int ngram);
float dice_distance(const std::string &s1, const std::string &s2);
unsigned int edit_distance(const std::string &s1, const std::string &s2);
void debugStringDataset(DataSet *DS);

#endif
