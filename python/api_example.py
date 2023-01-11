#!/usr/bin/python3

import numpy as np
from fastdp import fastdp,fastdp_generic


# x=np.loadtxt('../data/s1.txt')
# x=np.loadtxt('data/s1.txt')
x=np.loadtxt('data/b2.txt')

# Fast version using built in distance functions written in C:
numclu=15
# (labels,peak_ids) = fastdp(x,numclu,distance="l2",num_neighbors=30)

# For higher quality (more accurate kNN):
#  - decrease endcond (within [0.0..1.0]) 
#  - increase window (minimum 20)
#  - set nndes around 0.2 (higer than delta)

#Example:
# (labels,peak_ids) = fastdp(x,numclu,distance="l2",num_neighbors=20,window=50,nndes_start=0.2,maxiter=30,endcond=0.001,dtype="vec")

# Slower version using distance function provided by python:
# (Can work with any kind of distance)

# Class implementing distance measure needs to have following properties:
# - attribute 'size' that reflects the number of data objects
# - function distance(a,b) where parameters a,b are integers between 0..(size-1)
# Name of class is arbitrary
class DistanceMeasureL2:
	def __init__(self,x):
		self.tmp = 1
		self.x = x
		self.size = len(x)
	def distance(self,a,b):
		dist = np.linalg.norm(self.x[a]-self.x[b])
		return dist

dist = DistanceMeasureL2(x)
(labels,peak_ids) = fastdp_generic(dist,numclu,num_neighbors=20,window=50,nndes_start=0.0,maxiter=30,endcond=0.03,dtype="vec")
# Takes around 13 minutes for 100k (2D) dataset



