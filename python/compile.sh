#!/bin/bash
rm build/*/*.o
rm build/*/*.so
rm -f lib/*.so
rm -f *.so
python3 ./setup.py build_ext --inplace
