#!/usr/bin/env python3
#
# (C) 2015, 2021 Ed Bueler
#
# Generate .png figures from ASCII time-series mass files with t_n,M_n,R_n, as
# written by layer.c.
#
# Example of usage:
#   $ cd ../petsc/
#   $ make layer
# Generate time-series in ASCII files:
#   $ ./layer -lay_dt 0.02 -lay_steps 500 -lay_timedependentsource -lay_massfile foo1.txt
#   $ ./layer -lay_dt 0.01 -lay_steps 1000 -lay_timedependentsource -lay_massfile foo2.txt
# Make image file from multiple time-series files:
#   # mv foo?.txt ../refinemass
#   $ cd ../refinemass
#   $ ./fig.py -o bar.pdf foo?.txt

import argparse
import sys
import numpy as np
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(description='Generate figures from time-series mass files written by layer.c.')
parser.add_argument('files', metavar='FILE', nargs='+',
                    help='an ASCII file with these numbers in its first three columns: t_n M_n R_n')
parser.add_argument('-o', metavar='OUTFILE', default='',
                    help='name of output image file (.png,.pdf,...); show() if not given')
args = parser.parse_args()

def readtimeseries(filename):
    S = np.loadtxt(filename)
    return S[:,0], S[:,1], S[:,2]

for name in args.files:
    t, M, R = readtimeseries(name)
    plt.semilogy(t,M,'k')
    plt.semilogy(t,R,label='dt=%.3f' % (t[1]-t[0]))

plt.axis([0.0,t.max(),1.0e-10,1.0])
plt.legend(loc='lower right',fontsize=14.0)
plt.text(4.0,0.2,r'$M^\ell$',fontsize=18.0)
plt.text(4.9,1.0e-5,r'$R^\ell$',fontsize=18.0)
plt.xlabel('t',fontsize=12.0)
plt.ylabel('volume',fontsize=12.0)

if len(args.o) > 0:
    plt.savefig(args.o,bbox_inches='tight')
else:
    plt.show()

