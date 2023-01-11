/*******************************************************************************
 *
 * This file is part of RKNNG software.
 * Copyright (C) 2015-2018 Sami Sieranoja
 * <samisi@uef.fi>, <sami.sieranoja@gmail.com>
 *
 * RKNNG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. You should have received a copy
 * of the GNU Lesser General Public License along with RKNNG.
 * If not, see <http://www.gnu.org/licenses/lgpl.html>.
 *******************************************************************************/

#include <stdio.h>


//#include "contrib/argtable3.h"



#include "timer.h"
#include "util.h"
#include "globals.h"

#include "rknng_lib.h"

kNNGraph *g_ground_truth;

#include "recall.h"

#include "rp_div.h"
#include "brute_force.h"

extern "C" {
void test_rknn_lib() {
  printf("333333\n");
  DataSet *DS = loadStringData("data/birkbeckU.txt");
  debugStringDataset(DS);
}

void printDSVec(kNNGraph *knng, int id) {
  DataSet *DS = (DataSet *)knng->DS;
  printf("id:%d [%f, %f]\n", id, DS->data[id][0], DS->data[id][1]);
}


kNNGraph *create_knng(DataSet *DS, int k, int data_type, int algo, float endcond,
                   float nndes_start, int W, int dfunc, int num_iter) {

  g_options.distance_type = dfunc;
  kNNGraph *knng;

  g_timer.tick();

  if (algo == 0) {
    knng = rpdiv_create_knng(DS, DS, k, W, endcond, nndes_start, num_iter);
  } else if (algo == 9) {
    knng = brute_force_search(DS, k);
  }
  
  DS->distance_type = dfunc;

  // debugVecGraph(DS,knng,0);
  // debugStringGraph(DS,knng,0);
  // knng->list[10].items[0].id = 23;
  knng->DS = (void *)DS;

  printf("time=%fs\n", g_timer.get_time());

  return knng;
}



kNNGraph *get_knng(const char *infn, int k, int data_type, int algo, float endcond,
                   float nndes_start, int W, int dfunc, int num_iter) {

  g_options.distance_type = dfunc;
  DataSet *DS;
  if (data_type == 1) {
    DS = read_ascii_dataset(infn);
  } else if (data_type == 2) {
    DS = loadStringData(infn);
  } else {
    printf("Incorrect data type:%d\n", data_type);
  }
  kNNGraph *knng;
  DS->type = data_type;
  DS->distance_type = dfunc;

  g_timer.tick();

  if (algo == 0) {
    knng = rpdiv_create_knng(DS, DS, k, W, endcond, nndes_start, num_iter);
  } else if (algo == 9) {
    knng = brute_force_search(DS, k);
  }

  // debugVecGraph(DS,knng,0);
  // debugStringGraph(DS,knng,0);
  // knng->list[10].items[0].id = 23;
  knng->DS = (void *)DS;

  printf("time=%fs\n", g_timer.get_time());

  return knng;
}

float get_elapsed_time() { return g_timer.get_time(); }
}
