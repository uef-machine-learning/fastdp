#!/usr/bin/python3

import numpy as np
import fastdp

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

x=np.loadtxt('../data/s1.txt')
dist = DistanceMeasureL2(x)

numclu=15
# (labels,peak_ids) = fastdp.fastdp(x,numclu,distance="l2")
(labels,peak_ids) = fastdp.fastdp(x,numclu,distance="l2",num_neighbors=10,window=30,nndes_start=0.2,maxiter=30,endcond=0.05,dtype="vec")

print(labels)
print(peak_ids)

# Fast version using built in distance functions written in C:
# neighbors=1
# knn = rpdivknng.get_knng(x,neighbors,distance="l2")
# distance must be one of {l2(default),l1,cos}

# Slower version using distance function provided by python:
# (Can work with any kind of distance)
# knn = rpdivknng.get_knng_generic(dist,neighbors)

# For higher quality:
#  - decrease delta (within [0.0..1.0]) 
#  - increase window (minimum 20)
#  - set nndes around 0.2 (higer than delta)

# For example:
# knn = rpdivknng.get_knng(x,neighbors,window=50,nndes=0.2,maxiter=100,delta=0.001)

#Output format:
# knn[i][t][j]
# for t=0: index of j'th neighbor for i'th point
# for t=1: Distance to j'th neighbor for i'th point

# print('Start draving knn graph')
# from matplotlib import pyplot as plt
# for y in x:
	# plt.plot(y[0],y[1], marker=".", color="black")

# for i in range(0,len(knn)):
	# selfcoord = [x[i][0],x[i][1]]
	# for j in range(0,len(knn[i][0])):
		# neighbor = knn[i][0][j]
		# ncoord = [x[neighbor][0],x[neighbor][1]]
		# plt.plot([selfcoord[0],ncoord[0]],[selfcoord[1],ncoord[1]],color="black")
# plt.show()

