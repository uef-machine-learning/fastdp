
#include <Python.h>
#include <math.h>
#include <numpy/arrayobject.h>
// #ifdef Py_PYTHON_H
// #include "rknng_lib.h"
#include <stdio.h>
#include "rknng/rknng_lib.h"
#include "dencl/dencl.hpp"

// #include "constants.h"

// static PyObject *rpdiv_knng(PyObject *self, PyObject *args, PyObject *kwargs);
// static PyObject *rpdiv_knng_generic(PyObject *self, PyObject *args, PyObject *kwargs);

// PyObject *py_densityPeaks(PyArrayObject *py_v, int k);
PyObject *py_densityPeaks(PyArrayObject *py_v, int num_clusters, int num_neighbors, int W,
                          int max_iter, float endcond, float nndes_start, int dfunc);

extern "C" {

static PyObject *fastdp_py(PyObject *self, PyObject *args, PyObject *kwargs);

// Define python accessible methods
static PyMethodDef FastDPMethods[] = {
    {"fastdp", fastdp_py, METH_VARARGS | METH_KEYWORDS, "Cluster using fast density peaks"},
    {NULL, NULL, 0, NULL}};

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     (x1)*PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

/* This initiates the module using the above definitions. */
#if PY_VERSION_HEX >= 0x03000000
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT, "fastdp", NULL, -1, FastDPMethods, NULL, NULL, NULL, NULL};

PyMODINIT_FUNC PyInit_fastdp(void) {
  PyObject *m;
  m = PyModule_Create(&moduledef);
  if (!m) {
    return NULL;
  }
  return m;
}
#else
PyMODINIT_FUNC initfastdp(void) {
  PyObject *m;

  m = Py_InitModule("fastdp", FastDPMethods);
  if (m == NULL) {
    return;
  }
}
#endif

static PyObject *fastdp_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();
  PyArrayObject *py_v;
  int num_neighbors = 10, num_clusters = 10, window = 20, maxiter = 100;
  float nndes_start = 0.0, endcond = 0.05;
  char *type = NULL;
  char *distance = NULL;
  int dfunc = D_L2;

  PyObject *ret;
  static char *kwlist[] = {"v",       "num_clusters", "num_neighbors", "window",   "nndes_start",
                           "maxiter", "endcond",      "dtype",         "distance", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!i|iififss", kwlist, &PyArray_Type, &py_v,
                                   &num_clusters, &num_neighbors, &window, &nndes_start, &maxiter,
                                   &endcond, &type, &distance)) {
    return NULL;
  }

  if (window <= 0) {
    window = 2 * num_neighbors;
  }
  if (window <= 20) {
    window = 20;
  }

  int dtype = D_L2;
  if (distance != NULL) {
    if (strcmp("l2", distance) == 0) {
      dtype = D_L2;
    } else if (strcmp("l1", distance) == 0) {
      dtype = D_L1;
    } else if (strcmp("cos", distance) == 0) {
      dtype = D_COS;
    } else {
      PyErr_SetString(PyExc_ValueError, "Distance must be one for {l2(default),l1,cos}");
      return NULL;
    }
  }

  // DataSet *DS = init_DataSet(N, D);
  // ret = py_densityPeaks(py_v,15);
  ret = py_densityPeaks(py_v, num_clusters, num_neighbors, window, maxiter, endcond, nndes_start,
                        dfunc);

  // ret = Py_BuildValue("i", 99);
  return ret;
}
}

#ifdef DISABLED00
// For generic distance functions implemented in python
static PyObject *rpdiv_knng_generic(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();

  PyObject *py_v;
  int k, w = 0, maxiter = 100;
  float nndes = 0.0, delta = 0.05;
  char *type = NULL;
  // char *distance = NULL;

  PyObject *ret;
  static char *kwlist[] = {"v", "k", "window", "nndes", "maxiter", "delta", "dtype", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi|ififss", kwlist, &py_v, &k, &w, &nndes,
                                   &maxiter, &delta, &type)) {

    return NULL;
  }
  if (w <= 0) {
    w = 2 * k;
  }
  if (w <= 20) {
    w = 20;
  }

  printf("DELTA=%f\n", delta);

  ret = __rpdiv_knng_generic(py_v, k, w, nndes, delta, maxiter);
  return ret;
}

// For distance function implemented in C code in dataset.h
static PyObject *rpdiv_knng(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();

  PyArrayObject *py_v;
  int k, w = 0, maxiter = 100;
  float nndes = 0.0, delta = 0.05;
  char *type = NULL;
  char *distance = NULL;

  PyObject *ret;
  static char *kwlist[] = {"v",     "k",     "window",   "nndes", "maxiter",
                           "delta", "dtype", "distance", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!i|ififss", kwlist, &PyArray_Type, &py_v, &k, &w,
                                   &nndes, &maxiter, &delta, &type, &distance)) {
    return NULL;
  }
  if (w <= 0) {
    w = 2 * k;
  }
  if (w <= 20) {
    w = 20;
  }

  int dtype = D_L2;
  if (distance != NULL) {
    if (strcmp("l2", distance) == 0) {
      dtype = D_L2;
    } else if (strcmp("l1", distance) == 0) {
      dtype = D_L1;
    } else if (strcmp("cos", distance) == 0) {
      dtype = D_COS;
    } else {
      PyErr_SetString(PyExc_ValueError, "Distance must be one for {l2(default),l1,cos}");
      return NULL;
    }
  }

  // if (type != NULL && distance != NULL) {
  // printf(":%s %s\n", type, distance);
  // }

  ret = __rpdiv_knng(py_v, k, w, nndes, delta, maxiter, dtype);
  return ret;
}

#endif
