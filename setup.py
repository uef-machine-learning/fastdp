#!/usr/bin/env python

from distutils.core import setup, Extension

# module1 = Extension('rpdivknng', sources=['py_interf.c','rknng_lib.cpp','options.c'],
                        # include_dirs=['/usr/local/lib'])
                        
# OBJS=modules/file.o  dencl.o  modules/evi.o rknng/rknng_lib.o rknng/options.o modules/cb.o modules/interfc.o  modules/stack.o modules/sort.o modules/random.o modules/textfile.o modules/fvec.o modules/argtable3.o

cargs = ['-O3',  '-std=c++11', '-fopenmp', '-fpermissive',  '-Wall', '-D_PYTHON_LIB']


# module1 = Extension('fastdp', sources=['python/py_interf.cpp','python/dencl_lib.cpp','dencl/dencl.cpp','rknng/rknng_lib.cpp','rknng/options.cpp','modules/sort.c','modules/interfc.c','modules/stack.c' ,'rknng/dataset.cpp','rknng/knngraph.cpp','rknng/util.cpp'], include_dirs=['python','.','modules'], extra_compile_args=cargs)


#include "rknng/dataset.cpp"
#include "rknng/knngraph.cpp"
#include "rknng/options.cpp"
#include "rknng/rknng_lib.cpp"
#include "rknng/util.cpp"

#include "dencl.hpp"
#include "dencl.cpp"


module1 = Extension('fastdp', sources=['python/py_interf.cpp','python/dencl_lib.cpp','modules/sort.c','modules/interfc.c','modules/stack.c'], include_dirs=['python','.','modules','dencl'], extra_compile_args=cargs)
                        
setup(name = 'fastdp',
        version='1.0',
        setup_requires=['wheel'],
        python_requires='>=3',
        requires=['numpy'],
#        py_modules=['fastdp'],
        install_requires=["numpy>=1.9"],
        provides=['fastdp'],
       
        description='Fast Density Peaks using kNN graph',
        ext_modules = [module1])

