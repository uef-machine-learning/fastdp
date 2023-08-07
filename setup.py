#!/usr/bin/env python

import numpy

import setuptools
from setuptools import setup, Extension

__version__ = "1.0"

cargs = ['-O3',  '-std=c++11', '-fopenmp', '-fpermissive',  '-Wall', '-D_PYTHON_LIB']

with open('README.md', 'r', encoding='utf-8') as f:
    long_description = f.read()

module1 = Extension('fastdp', sources=['python/py_interf.cpp','python/dencl_lib.cpp','modules/sort.c','modules/interfc.c','modules/stack.c'], include_dirs=['python','.','modules','dencl',numpy.get_include()], extra_compile_args=cargs)
                        
setup(name = 'fastdp',
        version='1.0',
        setup_requires=['wheel','numpy'],
        python_requires='>=3',
        requires=['numpy'],
        install_requires=["numpy>=1.9"],
        provides=['fastdp'],
        long_description=long_description,
        long_description_content_type='text/markdown',
       
        description='Fast Density Peaks using kNN graph',
        ext_modules = [module1])

