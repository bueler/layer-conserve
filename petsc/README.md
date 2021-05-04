petsc/
======

The code `layer.c` describes a one-dimensional moving layer with a non-negative-
constrained thickness, and thus a moving boundary.  [PETSc SNESVI](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONRSLS.html) is used to
solve the free boundary problem at each time step.

Building `layer.c` requires recent PETSc; v3.15.0 was used most recently for testing.  Do

    $ make layer

To thoroughly test with exact solution,

    $ ./convtest.sh

For more information build `doclayer.pdf` by

    $ make doclayer.pdf