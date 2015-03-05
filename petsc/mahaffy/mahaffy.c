/* (C) 2015 Ed Bueler */

static const char help[] =
"Solves steady ice sheet problem in 2d:\n"
"    div (q^x,q^y) = m,\n"
"    (q^x,q^y) = - Gamma H^{n+2} |grad s|^{n-1} grad s,\n"
"where  H(x,y)  is the ice thickness,  b(x,y)  is the bed elevation,\n"
"and  s(x,y) = H(x,y) + b(x,y)  is surface elevation.\n"
"Note  n > 1  and  Gamma = 2 A (rho g)^n / (n+2).\n"
"Domain is  -Lx < x < Lx,  -Ly < y < Ly,  with periodic boundary conditions.\n\n"
"Computed by Q1 FVE method with FD evaluation of Jacobian (i.e. no analytical yet).\n"
"Compares Mahaffy* (default) and true Mahaffy schemes.\n"
"Uses SNESVI (-snes_type vinewtonrsls) with constraint  H(x,y) >= 0.\n\n"
"Two cases: (1) flat bed case where analytical solution is known,\n"
"           (2) -mah_read foo.dat reads b and m from PETSc binary file.\n\n"
"For case (1) usage read usage comment at start of mahaffy.c.\n"
"For case (2) usage see README.md.\n\n";

/* Usage:
   ./mahaffy -help |grep mah_  # get help

   ./mahaffy

   ./mahaffy -mah_true         # use true Mahaffy
   ./mahaffy -mah_Neps 4       # don't go all the way on continuation

   ./mahaffy -da_refine 1 -snes_vi_monitor  # widen screen to see SNESVI monitor output

Divergence and error cases:
   ./mahaffy -da_refine 2      # divergence, but no crash
   ./mahaffy -da_refine 3      # crash at SNESSolve_VINEWTONRSLS() line 505
                               #     in /home/ed/petsc-maint/src/snes/impls/vi/rs/virs.c
   ./mahaffy -da_refine 1 -pc_type mg  # error at line 506 of virs.c

Compare methods:
   mpiexec -n 6 ./mahaffy -da_refine 4 -mah_Neps 10
   mpiexec -n 6 ./mahaffy -da_refine 4 -mah_Neps 10 -mah_true   # again, line 505 of virs.c

High-res success (also -da_refine 7 works):
   mpiexec -n 6 ./mahaffy -da_refine 6 -pc_type asm -sub_pc_type lu -snes_max_it 200

Generate .png figs:
   mkdir foo/
   ./mahaffy -mah_dump foo/
   cd foo/
   python ../figsmahaffy.py
*/

#include <math.h>
#include <petscdmda.h>
#include <petscsnes.h>

#include "mahaffyctx.h"
#include "exactsia.h"
#include "binaryio.h"


extern PetscErrorCode FormBounds(SNES,Vec,Vec);
extern PetscErrorCode FormFunctionLocal(DMDALocalInfo*,PetscScalar**,PetscScalar**,AppCtx*);
// extern PetscErrorCode FormJacobianLocal(DMDALocalInfo*,PetscScalar*,Mat,Mat,AppCtx*); //see layer.c

extern PetscErrorCode SNESboot(SNES*,AppCtx*);
extern PetscErrorCode SNESAttempt(SNES,Vec,PetscInt*,SNESConvergedReason*);

extern PetscErrorCode ProcessOptions(AppCtx*);
extern PetscErrorCode ShowFields(DMDALocalInfo *info, AppCtx *user);
extern PetscErrorCode ErrorReport(Vec,DMDALocalInfo*,AppCtx*);


int main(int argc,char **argv) {
  PetscErrorCode      ierr;
  SNES                snes;
  Vec                 H, Htry;
  AppCtx              user;
  SerialReadVecs      readvecs;
  DMDALocalInfo       info;

  PetscInitialize(&argc,&argv,(char*)0,help);

  user.Nx     = -12;        // so DMDACreate2d() defaults to 12x12
  user.Ny     = -12;
  user.Lx     = 900.0e3;    // m
  user.Ly     = 900.0e3;    // m

  user.n      = 3.0;
  user.g      = 9.81;       // m/s^2
  user.rho    = 910.0;      // kg/m^3
  user.secpera= 31556926.0;
  user.A      = 1.0e-16/user.secpera; // = 3.17e-24  1/(Pa^3 s); EISMINT I value
  user.Gamma  = 2.0 * PetscPowReal(user.rho*user.g,user.n) * user.A / (user.n+2.0);

  user.eps    = 0.0;
  user.D0     = 1.0;        // m^2 / s
  user.Neps   = 13;
  user.mtrue  = PETSC_FALSE;

  user.dome   = PETSC_TRUE;  // defaults to this case
  user.read   = PETSC_FALSE;
  user.jsa    = PETSC_FALSE;

  user.showdata= PETSC_FALSE;
  user.dump   = PETSC_FALSE;
  strcpy(user.figsprefix,"PREFIX/");  // dummies improve "mahaffy -help" appearance
  strcpy(user.readname,"FILENAME");

  ierr = ProcessOptions(&user); CHKERRQ(ierr);

  if (user.read == PETSC_TRUE) {
      ierr = PetscPrintf(PETSC_COMM_WORLD,
          "reading grid, bed topography, climatic mass balance, and observed thickness from %s ...\n",
          user.readname); CHKERRQ(ierr);
      // sets user.[Nx,Ny,Lx,Ly,dx] and reads serial Vecs user.[topgread,cmbread,thkobsread]
      ierr = CreateReadVecs(&readvecs,&user); CHKERRQ(ierr);
  }

  // this DMDA is used for scalar fields on nodes; cell-centered grid
  ierr = DMDACreate2d(PETSC_COMM_WORLD,
                      DM_BOUNDARY_PERIODIC,DM_BOUNDARY_PERIODIC,
                      DMDA_STENCIL_BOX,
                      user.Nx,user.Ny,PETSC_DECIDE,PETSC_DECIDE,  // default grid if Nx<0, Ny<0
                      1, 1,                               // dof=1,  stencilwidth=1
                      NULL,NULL,&user.da);
  ierr = DMSetApplicationContext(user.da, &user);CHKERRQ(ierr);
  ierr = DMDAGetLocalInfo(user.da,&info); CHKERRQ(ierr);
  if (user.read == PETSC_FALSE) {
      user.dx = 2.0 * user.Lx / (PetscReal)(info.mx);
  }
  ierr = DMDASetUniformCoordinates(user.da, -user.Lx+user.dx/2, user.Lx+user.dx/2,
                                            -user.Ly+user.dx/2, user.Ly+user.dx/2,
                                   0.0,1.0); CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,
      "solving on [-Lx,Lx]x[-Ly,Ly] with Lx=%.3f km and Ly=%.3f,  %d x %d grid,  spacing dx = %.6f km ...\n",
      user.Lx/1000.0,user.Ly/1000.0,info.mx,info.my,user.dx/1000.0); CHKERRQ(ierr);

  // this DMDA is used for evaluating flux components at quad points on elements
  ierr = DMDACreate2d(PETSC_COMM_WORLD,
                      DM_BOUNDARY_PERIODIC,DM_BOUNDARY_PERIODIC,
                      DMDA_STENCIL_BOX,
                      info.mx,info.my,PETSC_DECIDE,PETSC_DECIDE,
                      4, 1,                               // dof=4,  stencilwidth=1
                      NULL,NULL,&user.quadda);
  ierr = DMSetApplicationContext(user.quadda, &user);CHKERRQ(ierr);

  ierr = DMCreateGlobalVector(user.da,&H);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)H,"thickness solution H"); CHKERRQ(ierr);

  ierr = VecDuplicate(H,&Htry); CHKERRQ(ierr);

  ierr = VecDuplicate(H,&user.b); CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)(user.b),"bed elevation b"); CHKERRQ(ierr);
  ierr = VecDuplicate(H,&user.m); CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)(user.m),"surface mass balance m"); CHKERRQ(ierr);
  ierr = VecDuplicate(H,&user.Hexact); CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)H,"exact/observed thickness H"); CHKERRQ(ierr);

  // fill user.[b,m,Hexact] according to 3 choices: data, dome exact soln, JSA exact soln
  if (user.read == PETSC_TRUE) {
      ierr = ReshapeAndDestroyReadVecs(&readvecs,&user); CHKERRQ(ierr);
  } else if (user.dome == PETSC_TRUE) {
      ierr = VecSet(user.b,0.0); CHKERRQ(ierr);
      ierr = SetToDomeSMB(user.m,PETSC_FALSE,&user); CHKERRQ(ierr);
      ierr = SetToDomeExactThickness(user.Hexact,&user); CHKERRQ(ierr);
  } else if (user.jsa == PETSC_TRUE) {
      SETERRQ(PETSC_COMM_WORLD,2,"ERROR: JSA exact solution not implemented ...\n");
/*FIXME
      ierr = SetToJSABed(user.b,&user); CHKERRQ(ierr);
      ierr = SetToJSASMB(user.m,&user); CHKERRQ(ierr);
      ierr = SetToJSAExactThickness(user.Hexact,&user); CHKERRQ(ierr);
*/
  } else {
      SETERRQ(PETSC_COMM_WORLD,1,"ERROR: one of user.[read,dome,jsa] must be PETSC_TRUE ...\n");
  }

  if (user.showdata == PETSC_TRUE) {
      ierr = ShowFields(&info,&user); CHKERRQ(ierr);
  }

  // initialize by chop & scale SMB
  ierr = VecCopy(user.m,H); CHKERRQ(ierr);
  ierr = VecScale(H,1500.0*user.secpera); CHKERRQ(ierr);  // FIXME make user.initializemagic
  // alternatives:
  //ierr = SetToVerifExactThickness(H,&user);CHKERRQ(ierr);
  //ierr = VecSet(H,0.0); CHKERRQ(ierr);

  ierr = SNESboot(&snes,&user); CHKERRQ(ierr);

  KSP        ksp;
  PetscInt   its, kspits, m;
  PetscReal  eps_sched[13] = {1.0,    0.5,    0.2,     0.1,     0.05,
                                       0.02,   0.01,   0.005,   0.002,   0.001,
                                       0.0005, 0.0002, 0.0000};
  SNESConvergedReason reason;
  for (m = 0; m<user.Neps; m++) {
      user.eps = eps_sched[m];
      ierr = VecCopy(H,Htry); CHKERRQ(ierr);
      ierr = SNESAttempt(snes,Htry,&its,&reason);CHKERRQ(ierr);
      ierr = SNESGetKSP(snes,&ksp); CHKERRQ(ierr);
      ierr = KSPGetIterationNumber(ksp,&kspits); CHKERRQ(ierr);
      if (reason < 0) {
          ierr = PetscPrintf(PETSC_COMM_WORLD,
                     "%3d. DIVERGED   with eps=%.2e ... %3d KSP (last) iters and %3d Newton iters (%s)\n",
                     m+1,kspits,its,user.eps,SNESConvergedReasons[reason]);CHKERRQ(ierr);
          if (user.eps > 0.0) {
              ierr = PetscPrintf(PETSC_COMM_WORLD,
                         "         ... try again w eps *= 1.5\n");CHKERRQ(ierr);
              //ierr = SNESDestroy(&snes); CHKERRQ(ierr);
              //ierr = SNESboot(&snes,&user); CHKERRQ(ierr);
              user.eps = 1.5 * eps_sched[m];
              ierr = VecCopy(H,Htry); CHKERRQ(ierr);
              ierr = SNESAttempt(snes,Htry,&its,&reason);CHKERRQ(ierr);
              ierr = SNESGetKSP(snes,&ksp); CHKERRQ(ierr);
              ierr = KSPGetIterationNumber(ksp,&kspits); CHKERRQ(ierr);
              if (reason < 0) {
                  ierr = PetscPrintf(PETSC_COMM_WORLD,
                         "     DIVERGED AGAIN  eps=%.2e ... %3d KSP (last) iters and %3d Newton iters (%s)\n",
                         user.eps,kspits,its,SNESConvergedReasons[reason]);CHKERRQ(ierr);
                  break;
              }
          }
      }
      if (reason >= 0) {
          ierr = PetscPrintf(PETSC_COMM_WORLD,
                     "%3d. converged  with eps=%.2e ... %3d KSP (last) iters and %3d Newton iters (%s)\n",
                     m+1,kspits,its,user.eps,SNESConvergedReasons[reason]);CHKERRQ(ierr);
          ierr = VecCopy(Htry,H); CHKERRQ(ierr);
          ierr = ErrorReport(H,&info,&user); CHKERRQ(ierr);
      }
  }

  if (user.dump == PETSC_TRUE) {
      ierr = DumpToFiles(H,&user); CHKERRQ(ierr);
  }

  ierr = VecDestroy(&user.m);CHKERRQ(ierr);
  ierr = VecDestroy(&user.b);CHKERRQ(ierr);
  ierr = VecDestroy(&user.Hexact);CHKERRQ(ierr);
  ierr = VecDestroy(&Htry);CHKERRQ(ierr);
  ierr = VecDestroy(&H);CHKERRQ(ierr);
  ierr = SNESDestroy(&snes);CHKERRQ(ierr);
  ierr = DMDestroy(&user.quadda);CHKERRQ(ierr);
  ierr = DMDestroy(&user.da);CHKERRQ(ierr);

  PetscFinalize();
  return 0;
}


//  for call-back: tell SNESVI (variational inequality) that we want
//    0.0 <= H < +infinity
PetscErrorCode FormBounds(SNES snes, Vec Xl, Vec Xu) {
  PetscErrorCode ierr;
  PetscFunctionBeginUser;
  ierr = VecSet(Xl,0.0); CHKERRQ(ierr);
  ierr = VecSet(Xu,PETSC_INFINITY); CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


// the single nontrivial formula for SIA
PetscReal getD(const PetscReal H, const PetscReal sx, const PetscReal sy, const AppCtx *user) {
    const PetscReal eps = user->eps,
                    n   = (1.0 - eps) * user->n + eps * 1.0;
    return user->Gamma * PetscPowReal(H,n+2.0) * PetscPowReal(sx*sx + sy*sy,(n-1.0)/2);
}


typedef struct {
    PetscReal x,y;
} Grad;


/* first of two nontrival operations with Q1 interpolants on an element */
/* evaluate the gradient of the surface elevation at any point (x,y) on element, using corner values of H and b */
PetscErrorCode gradsatpt(PetscInt j, PetscInt k,         // (j,k) is the element (by lower-left corner)
                         PetscReal locx, PetscReal locy, // = (x,y) coords in element
                         PetscReal **H, PetscReal **b,   // H[k][j] and b[k][j] are node values
                         const AppCtx *user, Grad *grads) {

  const PetscReal dx = user->dx, dy = dx,
                  x[4]  = {1.0 - locx / dx, locx / dx,       1.0 - locx / dx, locx / dx},
                  gx[4] = {- 1.0 / dx,      1.0 / dx,        - 1.0 / dx,      1.0 / dx},
                  y[4]  = {1.0 - locy / dy, 1.0 - locy / dy, locy / dy,       locy / dy},
                  gy[4] = {- 1.0 / dy,      - 1.0 / dy,      1.0 / dy,        1.0 / dy};
  PetscReal s[4];

  if ((grads == NULL) || (H == NULL) || (b == NULL)) {
      SETERRQ(PETSC_COMM_WORLD,1,"ERROR: illegal NULL ptr in gradsatpt() ...\n");
  }
  s[0] = H[k][j]     + b[k][j];
  s[1] = H[k][j+1]   + b[k][j+1];
  s[2] = H[k+1][j]   + b[k+1][j];
  s[3] = H[k+1][j+1] + b[k+1][j+1];
  grads->x = gx[0] * y[0] * s[0] + gx[1] * y[1] * s[1] + gx[2] * y[2] * s[2] + gx[3] * y[3] * s[3];
  grads->y =  x[0] *gy[0] * s[0] +  x[1] *gy[1] * s[1] +  x[2] *gy[2] * s[2] +  x[3] *gy[3] * s[3];
  PetscFunctionReturn(0);
}


Grad gradav(Grad g1, Grad g2) {
  Grad gav;
  gav.x = (g1.x + g2.x) / 2.0;
  gav.y = (g1.y + g2.y) / 2.0;
  return gav;
}


typedef struct {
    PetscReal x,y;
} Flux;


/* second of two nontrival operations with Q1 interpolants on an element */
/* evaluate the flux at any point (x,y) on element, using corner values of H and b */
PetscErrorCode fluxatpt(PetscInt j, PetscInt k,         // (j,k) is the element (by lower-left corner)
                        PetscReal locx, PetscReal locy, // = (x,y) coords in element
                        Grad gs,                        // compute with gradsatpt() first
                        PetscReal **H,                  // H[k][j] are node values
                        const AppCtx *user, Flux *q) {
  const PetscReal eps = user->eps,  dx = user->dx,  dy = dx,
                  x[4]  = {1.0 - locx / dx, locx / dx,       1.0 - locx / dx, locx / dx},
                  y[4]  = {1.0 - locy / dy, 1.0 - locy / dy, locy / dy,       locy / dy};
  PetscReal HH, DD;
  if ((q == NULL) || (H == NULL)) {
      SETERRQ(PETSC_COMM_WORLD,1,"ERROR: illegal NULL ptr in fluxatpt() ...\n");
  }
  HH = x[0] * y[0] * H[k][j] + x[1] * y[1] * H[k][j+1] + x[2] * y[2] * H[k+1][j] + x[3] * y[3] * H[k+1][j+1];
  DD = (1.0 - eps) * getD(HH,gs.x,gs.y,user) + eps * user->D0;
  q->x = - DD * gs.x;
  q->y = - DD * gs.y;
  PetscFunctionReturn(0);
}


/* on element shown, indexed by (j,k) node at lower-left corner @,
 FluxQuad holds x-components at * and y-components at %
   ---------------
  |       |       |
  |       *xn     |
  |  yw   |  ye   |
  |---%--- ---%---|
  |       |       |
  |       *xs     |
  |       |       |
  @---------------
(j,k)
*/
typedef struct {
    PetscReal xn, xs, ye, yw;
} FluxQuad;


/* for call-back: evaluate residual FF(x) on local process patch */
PetscErrorCode FormFunctionLocal(DMDALocalInfo *info,PetscScalar **H,PetscScalar **FF,
                                 AppCtx *user) {
  PetscErrorCode  ierr;
  const PetscReal dx = user->dx, dy = dx;
  PetscInt        j, k;
  PetscReal       **am, **ab;
  FluxQuad        **aq;
  Grad            gsn, gss, gse, gsw;
  Flux            qn, qs, qe, qw;
  Vec             bloc, qloc;

  PetscFunctionBeginUser;
  ierr = DMCreateLocalVector(user->da,&bloc);CHKERRQ(ierr);
  ierr = DMGlobalToLocalBegin(user->da,user->b,INSERT_VALUES,bloc);CHKERRQ(ierr);
  ierr = DMGlobalToLocalEnd(user->da,user->b,INSERT_VALUES,bloc);CHKERRQ(ierr);

  ierr = DMCreateLocalVector(user->quadda,&qloc);CHKERRQ(ierr);

  ierr = DMDAVecGetArray(user->da, bloc, &ab);CHKERRQ(ierr);
  ierr = DMDAVecGetArray(user->da, user->m, &am);CHKERRQ(ierr);
  ierr = DMDAVecGetArray(user->quadda, qloc, &aq);CHKERRQ(ierr);
  // loop over locally-owned elements, including ghosts, to get fluxes
  // note start at (xs-1,ys-1)
  for (k = info->ys-1; k < info->ys + info->ym; k++) {
      for (j = info->xs-1; j < info->xs + info->xm; j++) {
          if (user->mtrue == PETSC_FALSE) {  // default Mahaffy* method
              ierr = gradsatpt(j,k,     dx/2.0, 3.0*dy/4.0,      H,ab, user,&gsn); CHKERRQ(ierr);
              ierr =  fluxatpt(j,k,     dx/2.0, 3.0*dy/4.0, gsn, H,    user,&qn ); CHKERRQ(ierr);
              ierr = gradsatpt(j,k,     dx/2.0,     dy/4.0,      H,ab, user,&gss); CHKERRQ(ierr);
              ierr =  fluxatpt(j,k,     dx/2.0,     dy/4.0, gss, H,    user,&qs ); CHKERRQ(ierr);
              ierr = gradsatpt(j,k, 3.0*dx/4.0,     dy/2.0,      H,ab, user,&gse); CHKERRQ(ierr);
              ierr =  fluxatpt(j,k, 3.0*dx/4.0,     dy/2.0, gse, H,    user,&qe ); CHKERRQ(ierr);
              ierr = gradsatpt(j,k,     dx/4.0,     dy/2.0,      H,ab, user,&gsw); CHKERRQ(ierr);
              ierr =  fluxatpt(j,k,     dx/4.0,     dy/2.0, gsw, H,    user,&qw ); CHKERRQ(ierr);
          } else {  // true Mahaffy method
                    // this implementation is at least a factor of two inefficient
              Grad  gsn_nbr, gss_nbr, gse_nbr, gsw_nbr;
              // center-top point in element
              ierr =   gradsatpt(j,k,   dx/2.0, dy,   H,ab, user,&gsn); CHKERRQ(ierr);
              if (k < info->ys + info->ym - 1) {
                ierr = gradsatpt(j,k+1, dx/2.0, 0.0,  H,ab, user,&gsn_nbr); CHKERRQ(ierr);
                gsn  = gradav(gsn,gsn_nbr);
              }
              ierr =    fluxatpt(j,k,   dx/2.0, dy, gsn, H, user,&qn ); CHKERRQ(ierr);
              // center-bottom point in element
              ierr =   gradsatpt(j,k,   dx/2.0, 0.0,  H,ab, user,&gss); CHKERRQ(ierr);
              if (k > info->ys - 1) {
                ierr = gradsatpt(j,k-1, dx/2.0, dy,   H,ab, user,&gss_nbr); CHKERRQ(ierr);
                gss  = gradav(gss,gss_nbr);
              }
              ierr =    fluxatpt(j,k,   dx/2.0, 0.0,gss, H, user,&qs ); CHKERRQ(ierr);
              // center-right point in element
              ierr =   gradsatpt(j,k,   dx,  dy/2.0,  H,ab, user,&gse); CHKERRQ(ierr);
              if (j < info->xs + info->xm - 1) {
                ierr = gradsatpt(j+1,k, 0.0, dy/2.0,  H,ab, user,&gse_nbr); CHKERRQ(ierr);
                gse  = gradav(gse,gse_nbr);
              }
              ierr =    fluxatpt(j,k,   dx,  dy/2.0,gse, H, user,&qe ); CHKERRQ(ierr);
              // center-left point in element
              ierr =   gradsatpt(j,k,   0.0, dy/2.0,  H,ab, user,&gsw); CHKERRQ(ierr);
              if (j > info->xs - 1) {
                ierr = gradsatpt(j-1,k, dx,  dy/2.0,  H,ab, user,&gsw_nbr); CHKERRQ(ierr);
                gsw  = gradav(gsw,gsw_nbr);
              }
              ierr =    fluxatpt(j,k,   0.0, dy/2.0,gsw, H, user,&qw ); CHKERRQ(ierr);
          }
          aq[k][j] = (FluxQuad){qn.x,qs.x,qe.y,qw.y};
      }
  }
  // loop over nodes to get residual
  // This is the integral over boundary of control volume using two quadrature
  // points on each side of control volume.  For Mahaffy* it is two instances of
  // midpoint rule on each side.  For Mahaffy it is just one, but with averaged
  // gradient calculation at the midpoint of the side.
  for (k=info->ys; k<info->ys+info->ym; k++) {
      for (j=info->xs; j<info->xs+info->xm; j++) {
          FF[k][j] =  (aq[k][j].xs     + aq[k-1][j].xn  ) * dy/2.0
                    + (aq[k][j-1].ye   + aq[k][j].yw    ) * dx/2.0
                    - (aq[k][j-1].xs   + aq[k-1][j-1].xn) * dy/2.0
                    - (aq[k-1][j-1].ye + aq[k-1][j].yw  ) * dx/2.0
                    - am[k][j] * dx * dy;
      }
  }
  ierr = DMDAVecRestoreArray(user->da, user->m, &am);CHKERRQ(ierr);
  ierr = DMDAVecRestoreArray(user->da, bloc, &ab);CHKERRQ(ierr);
  ierr = DMDAVecRestoreArray(user->quadda, qloc, &aq);CHKERRQ(ierr);

  ierr = VecDestroy(&bloc); CHKERRQ(ierr);
  ierr = VecDestroy(&qloc); CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


PetscErrorCode ProcessOptions(AppCtx *user) {
  PetscErrorCode ierr;

  ierr = PetscOptionsBegin(PETSC_COMM_WORLD,"mah_","options to mahaffy","");CHKERRQ(ierr);
  ierr = PetscOptionsString(
      "-dump", "dump fields into PETSc binary files [x,y,b,m,H,Herror].dat with this prefix",
      NULL,user->figsprefix,user->figsprefix,512,&user->dump); CHKERRQ(ierr);
  ierr = PetscOptionsReal(
      "-D0", "representative value in m^2/s of diffusivity: D0 ~= D(H,|grad s|)",
      NULL,user->D0,&user->D0,NULL);CHKERRQ(ierr);
  ierr = PetscOptionsBool(
      "-jsa", "use bedrock-step exact solution by Jarosh, Schoof, Anslow (2013)",
      NULL,user->jsa,&user->jsa,NULL);CHKERRQ(ierr);
  ierr = PetscOptionsInt(
      "-Neps", "levels in schedule of eps regularization/continuation",
      NULL,user->Neps,&user->Neps,NULL);CHKERRQ(ierr);
  ierr = PetscOptionsString(
      "-read", "read grid and data from special-format PETSc binary file; see README.md",
      NULL,user->readname,user->readname,512,&user->read); CHKERRQ(ierr);
  ierr = PetscOptionsBool(
      "-showdata", "use PETSc X viewers to show bed elevation, climatic mass balance, and observed thickness data",
      NULL,user->showdata,&user->showdata,NULL);CHKERRQ(ierr);
  ierr = PetscOptionsBool(
      "-true", "use true Mahaffy method, not default Mahaffy*",
      NULL,user->mtrue,&user->mtrue,NULL);CHKERRQ(ierr);
  ierr = PetscOptionsEnd();CHKERRQ(ierr);
  // enforce consistency of cases
  if ((user->jsa == PETSC_TRUE) && (user->read == PETSC_TRUE)) {
      SETERRQ(PETSC_COMM_WORLD,1,"ERROR: option conflict: both user.jsa and user.read are true\n");
  }
  if ((user->jsa == PETSC_TRUE) || (user->read == PETSC_TRUE)) {
      user->dome = PETSC_FALSE;
  }
  PetscFunctionReturn(0);
}


// start a SNESVI
PetscErrorCode SNESboot(SNES *s, AppCtx* user) {
  PetscErrorCode ierr;
  ierr = SNESCreate(PETSC_COMM_WORLD,s);CHKERRQ(ierr);
  ierr = SNESSetDM(*s,user->da);CHKERRQ(ierr);
  ierr = DMDASNESSetFunctionLocal(user->da,INSERT_VALUES,
              (DMDASNESFunction)FormFunctionLocal,user); CHKERRQ(ierr);
  ierr = SNESVISetComputeVariableBounds(*s,&FormBounds);CHKERRQ(ierr);
  ierr = SNESSetType(*s,SNESVINEWTONRSLS);CHKERRQ(ierr);
  ierr = SNESSetFromOptions(*s);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


// try a SNES solve; H is modified
PetscErrorCode SNESAttempt(SNES snes, Vec H, PetscInt *its, SNESConvergedReason *reason) {
  PetscErrorCode ierr;
  PetscFunctionBeginUser;
  ierr = SNESSolve(snes, NULL, H); CHKERRQ(ierr);
  ierr = SNESGetIterationNumber(snes,its);CHKERRQ(ierr);
  ierr = SNESGetConvergedReason(snes,reason);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


// use X viewers to show b,m,Hexact
PetscErrorCode ShowFields(DMDALocalInfo *info, AppCtx *user) {
  PetscErrorCode ierr;
  PetscViewer  graphical;
  PetscInt     xdim = PetscMax(200,PetscMin(300,info->mx)),
               ydim = PetscMax(200,PetscMin(561,info->my));
  PetscFunctionBeginUser;
  ierr = PetscViewerDrawOpen(PETSC_COMM_WORLD,NULL,"bed topography (m)",
                             PETSC_DECIDE,PETSC_DECIDE,xdim,ydim,&graphical); CHKERRQ(ierr);
  ierr = VecView(user->b,graphical); CHKERRQ(ierr);
  ierr = PetscViewerDestroy(&graphical); CHKERRQ(ierr);
  ierr = PetscViewerDrawOpen(PETSC_COMM_WORLD,NULL,"climatic mass balance (m s-1)",
                             PETSC_DECIDE,PETSC_DECIDE,xdim,ydim,&graphical); CHKERRQ(ierr);
  ierr = VecView(user->m,graphical); CHKERRQ(ierr);
  ierr = PetscViewerDestroy(&graphical); CHKERRQ(ierr);
  ierr = PetscViewerDrawOpen(PETSC_COMM_WORLD,NULL,"exact or observed thickness (m)",
                             PETSC_DECIDE,PETSC_DECIDE,xdim,ydim,&graphical); CHKERRQ(ierr);
  ierr = VecView(user->Hexact,graphical); CHKERRQ(ierr);
  ierr = PetscViewerDestroy(&graphical); CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


// prints the |.|_inf = (max error) and |.|_1/(dim) = (av error) norms
PetscErrorCode ErrorReport(Vec H, DMDALocalInfo *info, AppCtx *user) {
  PetscErrorCode ierr;
  PetscScalar enorminf,enorm1;
  Vec dH;
  ierr = VecDuplicate(H,&dH); CHKERRQ(ierr);
  ierr = VecWAXPY(dH,-1.0,user->Hexact,H); CHKERRQ(ierr);    // dH := (-1.0) Hexact + H = H - Hexact
  ierr = VecNorm(dH,NORM_INFINITY,&enorminf); CHKERRQ(ierr);
  ierr = VecNorm(dH,NORM_1,&enorm1); CHKERRQ(ierr);
  ierr = VecDestroy(&dH); CHKERRQ(ierr);
  enorm1 /= info->mx * info->my;
  ierr = PetscPrintf(PETSC_COMM_WORLD,
             "     errors:  max = %8.2f,  av = %8.2f\n",enorminf,enorm1); CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

