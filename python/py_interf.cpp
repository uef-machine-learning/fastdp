
#include <Python.h>
#include <math.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

#include "rknng/rknng_lib.h"
#include "rknng/dataset.hpp"
#include "rknng/knngraph.hpp"
#include "rknng/options.h"
#include "rknng/util.h"

#include "rknng/dataset.cpp"
#include "rknng/knngraph.cpp"
#include "rknng/options.cpp"
#include "rknng/rknng_lib.cpp"
#include "rknng/util.cpp"

#include "dencl.hpp"
#include "dencl.cpp"



PyObject *py_densityPeaks(PyArrayObject *py_v, int num_clusters, int num_neighbors, int W,
                          int max_iter, float endcond, float nndes_start, int dfunc);
PyObject *py_densityPeaksGeneric(PyObject *py_v, int num_clusters, int num_neighbors, int W,  int max_iter, float endcond, float nndes_start, int dfunc);

extern "C" {

static PyObject *fastdp_py(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject *fastdp_generic_py(PyObject *self, PyObject *args, PyObject *kwargs);

// Define python accessible methods
static PyMethodDef FastDPMethods[] = {
    {"fastdp", fastdp_py, METH_VARARGS | METH_KEYWORDS, "Cluster using fast density peaks."},
    {"fastdp_generic", fastdp_generic_py, METH_VARARGS | METH_KEYWORDS, "Cluster using fast density peaks, using python provided distance function."},
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

  ret = py_densityPeaks(py_v, num_clusters, num_neighbors, window, maxiter, endcond, nndes_start,
                        dfunc);

  return ret;
}

static PyObject *fastdp_generic_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();
  PyObject *py_v;
  int num_neighbors = 10, num_clusters = 10, window = 20, maxiter = 100;
  float nndes_start = 0.0, endcond = 0.05;
  char *type = NULL;
  char *distance = NULL;
  int dfunc = D_L2;

  PyObject *ret;
  static char *kwlist[] = {"v",       "num_clusters", "num_neighbors", "window",   "nndes_start",
                           "maxiter", "endcond",      "dtype",         "distance", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi|iififss", kwlist, &py_v,
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

  ret = py_densityPeaksGeneric(py_v, num_clusters, num_neighbors, window, maxiter, endcond, nndes_start,
                        dfunc);

  return ret;
}


} // END extern "C"

