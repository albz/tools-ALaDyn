#!/usr/bin/python
######################################################################
# Name:         dued_read_frm.py
# Author:       
# Date:			2014-02-18
# Purpose:      reads dued binary frm output
# Source:       python
#####################################################################

### loading shell commands
import os, os.path, glob, sys, shutil, time, datetime, scipy, numpy, pylab, matplotlib
import struct
from scipy import *
import numpy as np
from matplotlib import *
from pylab import *
import matplotlib as plt
from matplotlib.ticker import MultipleLocator, FormatStrFormatter
### --- ###



# - #
#def read_read(dir_path,nframe):

# - #
#dir_path = '/Volumes/Macintosh_HD_2/sims/non_local_electrons/polar_spike_pushing/f_007_00024'
name     = 'Bdenout01.bin'
path     = os.path.join(os.path.join(os.getcwd()),name)


# - #
f = open(path,'rb')


#- vector length -#
struct.unpack('i', f.read(4))
N_param = struct.unpack('i', f.read(4))[0]
print N_param
struct.unpack('i', f.read(4))


struct.unpack('i', f.read(4))
int_param=[]
for i in range(0,N_param):
	int_param.append( struct.unpack('i', f.read(4)) )
struct.unpack('i', f.read(4))
nx= int_param[3][0]
ny= int_param[4][0]
nz= int_param[6][0]
nproc_y = int_param[0][0]
nproc_z = int_param[1][0]

struct.unpack('i', f.read(4))
for i in range(0,N_param):
 	struct.unpack('f', f.read(4))
struct.unpack('i', f.read(4))

#---***---#
r = np.zeros((nx,ny,nz))
print 'n',nx,ny,nz
rr=[]
print 'proc', nproc_z,nproc_y

offsetz = 0
for counter_z in range(0,nproc_z):
	offsety = 0;

	for counter_y in range(0,nproc_y):
		print struct.unpack('i', f.read(4))
		npx= struct.unpack('i', f.read(4))[0]
		npy= struct.unpack('i', f.read(4))[0]
		npz= struct.unpack('i', f.read(4))[0]
		print struct.unpack('i', f.read(4))
		print '>>>', nx, ny, nz
		print '>>>', npx, npy, npz
	
		print struct.unpack('i', f.read(4))
		for k in range(0,npz):
			for j in range(0,npy):
				for i in range(0,npx):
					r[i,j+offsety,k+offsetz] = struct.unpack('f', f.read(4))[0]
					#rr.append(r[i,j,k])
	# 				if np.abs(r[i,j,k]) > 0.0:
	# 					print '***',r[i,j,k]
		print struct.unpack('i', f.read(4))
		offsety += npy
	offsetz += npz;

#r=np.reshape(rr,(nx,ny,nz))
print len(rr),r.shape








np.savetxt('test.txt', r[:,:,64])




xlist = linspace(0.,1.,256.) 
ylist = linspace(0.,1.,128.) 
X, Y = meshgrid (ylist, xlist)
ax  = matplotlib.pyplot.subplot(111) #matplotlib.pyplot.subplot(111)
print 'shape>>',X.shape, Y.shape,r.shape,r[:,:,64].shape
#CP1=pyplot.contour(Y,X,r[:,:,64])
print r.shape,r.min(),r.max()
pyplot.imshow(r[:,:,64].T)

show()

















