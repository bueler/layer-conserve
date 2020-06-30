#!/usr/bin/env python3

from sympy import *
#init_printing(use_unicode=True)

s, t, p = symbols('s t p')

f = (1 - (t**(p-1)+t)*s + t**p) / (1 - 2*s*t + t**2)**(p/2)

dfds = diff(f,s)

y = dfds / (t * (1 - 2*s*t + t**2)**(-1-p/2))

g = powsimp( collect(simplify(y), s) )
print(g)
gshouldbe = s*(2-p)*t*(t**(p-2)+1) + (p-1)*(t**p+1) - t**(p-2) - t**2
print(simplify(g - gshouldbe))  # get 0

mydgds = (2-p)*t*(t**(p-2)+1)
print(simplify(diff(g,s) - mydgds))

G = g.subs(s,1)
print(G)

