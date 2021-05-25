layer-conserve
==============

This is a project on mass-conserving layer models which have a nonnegativity constraint on the conserved thickness variable.  This situation is especially important for climate applications such as ice sheets, sea ice, subglacial water, shallow water, etc.

This repository includes a paper, several talks, and a 1D example C code which uses [PETSc](http://www.mcs.anl.gov/petsc/).  The paper has been accepted by SIAM J. Applied Math.  It is available in preprint form:

  * E. Bueler (2020). _Conservation laws for free-boundary fluid layers_, [arxiv:2007.05625](https://arxiv.org/abs/2007.05625)

A nontrivial ice sheet example was moved to a [separate repo](https://github.com/bueler/sia-fve) and published:

  * E. Bueler (2016).  _Stable finite volume element schemes for the shallow-ice approximation_, J. Glaciol. 62 (232), 230-242, [doi:10.1017/jog.2015.3](http://dx.doi.org/10.1017/jog.2015.3)

To run the C code see [petsc/README.md](petsc/README.md).