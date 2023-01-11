
#include <Python.h>
#include <cstring>
#include <cmath>
#include <fstream>
#include <csignal> // Raise
#include <fstream>
#include <vector>
#include <float.h>
#include <math.h>
#include <numpy/arrayobject.h>
// #ifdef Py_PYTHON_H
// #include "rknng_lib.h"
#include <stdio.h>
#include "rknng/rknng_lib.h"
#include "dencl/dencl.hpp"
// #include "rknng/rp_div.h"
// #include "rknng/dataset.hpp"

using namespace std;

// struct DataSet {
// int size;
// int dimensionality;
// int vector_size;
// float elem_min;
// float elem_max;
// float **data;

// vector<string> *strings;
// int **bigrams;
// int **trigrams;

// int *setSize; // For set data
// int type;
// };

// DataSet *init_DataSet(int size, int dimensionality);
// void set_val(DataSet *DS, int data_i, int elem_i, float val);

// #include "constants.h"

// static PyObject *rpdiv_knng(PyObject *self, PyObject *args, PyObject *kwargs);
// static PyObject *rpdiv_knng_generic(PyObject *self, PyObject *args, PyObject *kwargs);

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     (x1)*PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

// extern "C" {

PyObject *array_to_py(int *arr, int N) {
  // Convert c array to python format
  // printf("array_to_py size=%d\n", N);
  PyObject *pyarr = PyList_New(N);
  for (int i = 0; i < N; i++) {
    PyList_SetItem(pyarr, i, Py_BuildValue("i", arr[i]));
  }
  return pyarr;
}

PyObject *py_densityPeaksGeneric(PyObject *py_v, int num_clusters, int num_neighbors, int W,
                                 int max_iter, float endcond, float nndes_start, int dfunc) {

  // PyObject *py_densityPeaks(PyArrayObject *py_v, int k) {
  PyObject *ret;
  PyObject *py_labels;
  PyObject *py_peaks;
  printf("py_densityPeaks\n");

  double *delta;
  int *nearestHighDens;
  double *density;
  Array *neighborhood_peaks;
  int *peaks;
  int *labels;

  PyObject *pysize = PyObject_GetAttrString(py_v, "size");
  DataSet *DS = (DataSet *)safemalloc(sizeof(DataSet));
  DS->size = PyLong_AsLong(pysize);
  DS->dimensionality = 0;
  DS->type = T_CUSTOMDF;
  DS->pydf = PyObject_GetAttrString(py_v, "distance");
  int N = DS->size;

  DS->pyids = (PyObject **)malloc(sizeof(PyObject) * N);
  for (int i = 0; i < N; i++) {
    DS->pyids[i] = PyLong_FromLong(i);
  }

  kNNGraph *knng;

  // int num_clusters = 15;
  // int num_neighbors = 20, W = 40, num_iter = 40;
  // float endcond = 0.02, nndes_start = 0.0;
  // int dfunc = D_L2;
  knng = create_knng(DS, num_neighbors, 0, 0, endcond, nndes_start, W, dfunc, max_iter);
  knnGraphDPstats(knng, 1, &density, &delta, &nearestHighDens, &neighborhood_peaks, 0);

  densityPeaks(
      // INPUT:
      N, num_clusters, density, delta, nearestHighDens,
      // OUTPUT:
      &peaks, &labels);
  py_labels = array_to_py(labels, N);
  py_peaks = array_to_py(peaks, num_clusters);
  // ret = Py_BuildValue("i", 89);
  ret = PyList_New(2);
  PyList_SetItem(ret, 0, py_labels);
  PyList_SetItem(ret, 1, py_peaks);

  return ret;
  // , int w, float nndes, float delta, int maxiter, int dtype) {
}

PyObject *py_densityPeaks(PyArrayObject *py_v, int num_clusters, int num_neighbors, int W,
                          int max_iter, float endcond, float nndes_start, int dfunc) {

  // PyObject *py_densityPeaks(PyArrayObject *py_v, int k) {
  PyObject *ret;
  PyObject *py_labels;
  PyObject *py_peaks;
  printf("py_densityPeaks\n");

  double *delta;
  int *nearestHighDens;
  double *density;
  Array *neighborhood_peaks;
  int *peaks;
  int *labels;

  int N = py_v->dimensions[0];
  int D = py_v->dimensions[1];
  // DataSet *DS = init_DataSet(N, D);
  DataSet *DS = init_DataSet(N, D);
  // DS->distance_type = dtype;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < D; j++) {
      set_val(DS, i, j, v(i, j));
    }
  }

  kNNGraph *knng;

  // int num_clusters = 15;
  // int num_neighbors = 20, W = 40, num_iter = 40;
  // float endcond = 0.02, nndes_start = 0.0;
  // int dfunc = D_L2;
  knng = create_knng(DS, num_neighbors, 0, 0, endcond, nndes_start, W, dfunc, max_iter);
  knnGraphDPstats(knng, 1, &density, &delta, &nearestHighDens, &neighborhood_peaks, 0);

  densityPeaks(
      // INPUT:
      N, num_clusters, density, delta, nearestHighDens,
      // OUTPUT:
      &peaks, &labels);
  py_labels = array_to_py(labels, N);
  py_peaks = array_to_py(peaks, num_clusters);
  // ret = Py_BuildValue("i", 89);
  ret = PyList_New(2);
  PyList_SetItem(ret, 0, py_labels);
  PyList_SetItem(ret, 1, py_peaks);

  return ret;
  // , int w, float nndes, float delta, int maxiter, int dtype) {
}
// }
