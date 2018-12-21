#!/usr/bin/env python3

import matplotlib.font_manager as font_manager
import numpy as np
import matplotlib.pyplot as plt
from sys import exit

def genbasicfig():
    x = np.linspace(0.0,10.0,1001)
    b = 0.07*(x-3.0)**2 + 0.2*np.sin(2.0*x)

    h0 = 3.0
    L = 3.0
    h = np.maximum(0.0, h0*(-0.2 + np.sqrt(np.maximum(0.0,1.0 - (x-5)**2/L**2))) )
    s = b + h

    plt.plot(x, b, '--k', lw=3.0)
    plt.plot(x, s, 'k', lw=3.0)

    plt.arrow(x[600],b[600],0.0,h[600],lw=1.0,head_width=0.1,color='k',
              length_includes_head=True)
    plt.arrow(x[600],s[600],0.0,-h[600],lw=1.0,head_width=0.1,color='k',
              length_includes_head=True)
    plt.text(x[600]-0.4,b[600]+0.4*h[600],r'$u$',fontsize=24.0)
    plt.arrow(x[450],b[600]+0.3*h[600],-1.0,0.0,
              lw=2.0,head_width=0.2,color='k',length_includes_head=True)
    plt.text(x[400],b[600]+0.45*h[600],r'$\mathbf{q}$',fontsize=24.0)

    plt.axis([0.0,10.0,-0.5,4.5])
    plt.axis('off')
    return x, s, b

def drawclimate(x,s,mycolor):
    plt.text(x[0]-0.5,s.max()+0.1,r'$f$',fontsize=24.0)
    for j in range(10):
        xarr = x[50+100*j]
        if j>0:
            magarr = 0.6*np.sin(np.pi/2 + 0.6*xarr)
        else:
            magarr = 0.05
        plt.arrow(xarr,s.max()+0.2,0.0,magarr,lw=1.5,head_width=0.1,color=mycolor)

figdebug = False
def figsave(name):
    if figdebug:
        plt.show()  # debug
    else:
        plt.savefig(name,bbox_inches='tight')

# basic figure
plt.figure(figsize=(10,4))
x, s, _ = genbasicfig()
drawclimate(x,s,'k')
figsave('cartoon.pdf')

