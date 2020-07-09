#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

th = np.linspace(0.0,2.0*np.pi,1001)

pert = 0.3*np.exp(-(th-4.3)**2/0.3)
#pert = 0.6*exp(-(th-4.3)**2/0.3)
xOm = np.cos(th) + pert
yOm = np.sin(th) + pert

pertnx = 0.05*np.sin(6*th)
pertny = 0.05*np.sin(6*th+13.0)
xn = 0.7*np.cos(th) + pertnx
yn = 0.5*np.sin(th) + pertny

xr = xn[50:500]
yr = yn[50:500] + 0.3*np.exp(-(th[50:500]-1.5)**2/0.2)

plt.figure(figsize=(6,6))
plt.plot(xOm, yOm, '-k', lw=2.0)
plt.plot(xn, yn, '-k', lw=1.5)
plt.plot(xr, yr, '--k', lw=1.5)

myfontsize=18.0
#plt.text(0.7,0.9,r'$\Omega$',fontsize=20.0)
plt.text(0.5,-0.6,r'$\Omega_n^{00}$',fontsize=myfontsize)
plt.text(0.0,0.6,r'$\Omega_n^r$',fontsize=myfontsize)
plt.text(-0.2,-0.1,r'$\Omega_n$',fontsize=myfontsize)

plt.axis('off')
plt.savefig('domains-fig.pdf',bbox_inches='tight')
#plt.show()  # debug

