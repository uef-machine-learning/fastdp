
#ifndef RKNNG_LIB_H
#define RKNNG_LIB_H
#include "dataset.hpp"
#include "knngraph.hpp"

typedef int BOOL;
#define true 1
#define false 0

extern "C" {
void test_rknn_lib();
void printDSVec(kNNGraph *knng, int id);
float knng_dist(kNNGraph *knng, int p1, int p2);
kNNGraph *create_knng(DataSet *DS, int k, int data_type, int algo, float endcond, float nndes_start,
                      int W, int dfunc, int num_iter);
kNNGraph *get_knng(const char *infn, int k, int data_type, int algo, float endcond,
                   float nndes_start, int W, int dfunc, int num_iter);
float get_elapsed_time();
}

#endif
