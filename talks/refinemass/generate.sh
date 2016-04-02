#!/bin/bash

NN=1

# do 100, 1000, 10000 steps for mass0.txt, mass1.txt, mass2.txt, respectively
for MM in 0 1 2 ; do
  STEPS=$(bc -l <<< "100*10^$MM")
  DT=$(bc -l <<< "scale = 6; 10.0/$STEPS")
  cmd="mpiexec -n $NN ../../petsc/layer -da_refine 4 -lay_dt ${DT} -lay_steps ${STEPS} -lay_adscheme 1 -lay_jac -lay_noshow -lay_timedependentsource -lay_massfile mass${MM}.txt -snes_max_it 1000"
  echo $cmd
  $cmd
done

# now do something like
#   ./fig.py -o foo.pdf mass?.txt

