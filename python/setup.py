#!/usr/bin/env python

from distutils.core import setup, Extension

# module1 = Extension('rpdivknng', sources=['py_interf.c','rknng_lib.cpp','options.c'],
                        # include_dirs=['/usr/local/lib'])
                        
# OBJS=modules/file.o  dencl.o  modules/evi.o rknng/rknng_lib.o rknng/options.o modules/cb.o modules/interfc.o  modules/stack.o modules/sort.o modules/random.o modules/textfile.o modules/fvec.o modules/argtable3.o

cargs = ['-O3',  '-std=c++11', '-fopenmp', '-fpermissive',  '-Wall', '-D_PYTHON_LIB']


# module1 = Extension('fastdp', sources=['py_interf.c','dencl_lib.cpp','../dencl/dencl.cpp','../rknng/rknng_lib.cpp','../rknng/options.c','../modules/sort.c','../modules/interfc.c','../modules/stack.c'], include_dirs=['/usr/local/lib','../','../modules'], extra_compile_args=cargs)
module1 = Extension('fastdp', sources=['py_interf.cpp','dencl_lib.cpp','../dencl/dencl.cpp','../rknng/rknng_lib.cpp','../rknng/options.c','../modules/sort.c','../modules/interfc.c','../modules/stack.c' ,'../rknng/dataset.cpp','../rknng/knngraph.cpp','../rknng/util.cpp'], include_dirs=['/usr/local/lib','../','../modules'], extra_compile_args=cargs)
                        
# ,'../rknng/.cpp'                       

setup(name = 'fastdp',
        version='1.0',
        description='Fast Density Peaks using kNN graph',
        ext_modules = [module1])

