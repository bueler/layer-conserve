#!/bin/bash

MPI="mpiexec --map-by socket --bind-to core"
for SCHEME in 0 1 2 ; do
    for NN in 1 4 ; do
        echo "testing advection scheme $SCHEME on $NN processes:" ;
        for LEV in 0 2 4 6 8 10 12 ; do
            $MPI -n $NN ./layer -lay_exactinit -lay_noshow -lay_dt 0.01 -lay_steps 10 -lay_adscheme $SCHEME -da_refine $LEV -lay_jac | 'grep' error
        done
        echo
    done
done
