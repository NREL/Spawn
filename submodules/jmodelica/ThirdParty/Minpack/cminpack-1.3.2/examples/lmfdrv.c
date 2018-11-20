/* Usage: lmfdrv < lm.data */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cminpack.h"
#include "ssq.h"
#define real __cminpack_real__

/*     ********** */

/*     this program tests codes for the least-squares solution of */
/*     m nonlinear equations in n variables. it consists of a driver */
/*     and an interface subroutine fcn. the driver reads in data, */
/*     calls the nonlinear least-squares solver, and finally prints */
/*     out information on the performance of the solver. this is */
/*     only a sample driver, many other drivers are possible. the */
/*     interface subroutine fcn is necessary to take into account the */
/*     forms of calling sequences used by the function and jacobian */
/*     subroutines in the various nonlinear least-squares solvers. */

/*     subprograms called */

/*       user-supplied ...... fcn */

/*       minpack-supplied ... dpmpar,enorm,initpt,lmdif1,ssqfcn */

/*       fortran-supplied ... dsqrt */

/*     argonne national laboratory. minpack project. march 1980. */
/*     burton s. garbow, kenneth e. hillstrom, jorge j. more */

/*     ********** */

int fcn(void *p, int m, int n, const real *x, real *fvec, int iflag);

struct refnum {
    int nprob, nfev, njev;
};

static void printvec(int n, const real *x)
{
    int i, num5, ilow, numleft;
    num5 = n/5;

    for (i = 0; i < num5; ++i) {
        ilow = i*5;
        printf("     %15.7e%15.7e%15.7e%15.7e%15.7e\n",
               (double)x[ilow+0], (double)x[ilow+1], (double)x[ilow+2], (double)x[ilow+3], (double)x[ilow+4]);
    }
    
    numleft = n%5;
    ilow = n - numleft;

    switch (numleft) {
        case 1:
            printf("     %15.7e\n",
                   (double)x[ilow+0]);
            break;
        case 2:
            printf("     %15.7e%15.7e\n",
                   (double)x[ilow+0], (double)x[ilow+1]);
            break;
        case 3:
            printf("     %15.7e%15.7e%15.7e\n",
                   (double)x[ilow+0], (double)x[ilow+1], (double)x[ilow+2]);
            break;
        case 4:
            printf("     %15.7e%15.7e%15.7e%15.7e\n",
                   (double)x[ilow+0], (double)x[ilow+1], (double)x[ilow+2], (double)x[ilow+3]);
            break;
    }
}

/* Main program */
int main(int argc, char **argv)
{

    int i,ic,k,m,n,ntries;
    struct refnum lmdiftest;
    int info;

    int ma[60];
    int na[60];
    int nf[60];
    int nj[60];
    int np[60];
    int nx[60];

    real factor,fnorm1,fnorm2,tol;

    real fnm[60];
    real fvec[65];
    real x[40];

    int iwa[40];
    real wa[65*40+5*40+65];
    const int lwa = 65*40+5*40+65;

    tol = sqrt(__cminpack_func__(dpmpar)(1));

    ic = 0;

    for (;;) {
        scanf("%5d%5d%5d%5d\n", &lmdiftest.nprob, &n, &m, &ntries);
/*
         read (nread,50) nprob,n,m,ntries
   50 format (4i5)
*/
        if (lmdiftest.nprob <= 0.)
            break;
        factor = 1.;

        for (k = 0; k < ntries; ++k, ++ic) {
            lmdipt(n,x,lmdiftest.nprob,factor);

            ssqfcn(m,n,x,fvec,lmdiftest.nprob);

            fnorm1 = __cminpack_func__(enorm)(m,fvec);

            printf("\n\n\n\n      problem%5d      dimensions%5d%5d\n\n", lmdiftest.nprob, n, m);
/*
            write (nwrite,60) nprob,n,m
   60 format ( //// 5x, 8h problem, i5, 5x, 11h dimensions, 2i5, 5x //
     *         )
*/

            lmdiftest.nfev = 0;
            lmdiftest.njev = 0;

            info = __cminpack_func__(lmdif1)(fcn,&lmdiftest,m,n,x,fvec,tol,iwa,wa,lwa);

            ssqfcn(m,n,x,fvec,lmdiftest.nprob);

            fnorm2 = __cminpack_func__(enorm)(m,fvec);

            np[ic] = lmdiftest.nprob;
            na[ic] = n;
            ma[ic] = m;
            nf[ic] = lmdiftest.nfev;
            lmdiftest.njev /= n;
            nj[ic] = lmdiftest.njev;
            nx[ic] = info;

            fnm[ic] = fnorm2;

            printf("\n      initial l2 norm of the residuals%15.7e\n"
                   "\n      final l2 norm of the residuals  %15.7e\n"
                   "\n      number of function evaluations  %10d\n"
                   "\n      number of Jacobian evaluations  %10d\n"
                   "\n      exit parameter                  %10d\n"
                   "\n      final approximate solution\n\n",
                   (double)fnorm1, (double)fnorm2, lmdiftest.nfev, lmdiftest.njev, info);
            printvec(n, x);
/*
            write (nwrite,70)
     *            fnorm1,fnorm2,nfev,njev,info,(x(i), i = 1, n)
   70 format (5x, 33h initial l2 norm of the residuals, d15.7 // 5x,
     *        33h final l2 norm of the residuals  , d15.7 // 5x,
     *        33h number of function evaluations  , i10 // 5x,
     *        33h number of jacobian evaluations  , i10 // 5x,
     *        15h exit parameter, 18x, i10 // 5x,
     *        27h final approximate solution // (5x, 5d15.7))
*/

            factor *= 10.;

        }

    }

    printf("\f summary of %d calls to lmdif1: \n\n", ic);
/*
      write (nwrite,80) ic
   80 format (12h1summary of , i3, 16h calls to lmdif1 /)
*/
    printf("\n\n nprob   n    m   nfev  njev  info  final L2 norm \n\n");
/*
      write (nwrite,90)
   90 format (49h nprob   n    m   nfev  njev  info  final l2 norm /)
*/

    for (i = 0; i < ic; ++i) {
        printf("%5d%5d%5d%6d%6d%6d%16.7e\n",
               np[i], na[i], ma[i], nf[i], nj[i], nx[i], (double)fnm[i]);
/*
         write (nwrite,100) np(i),na(i),ma(i),nf(i),nj(i),nx(i),fnm(i)
  100 format (3i5, 3i6, 1x, d15.7)
*/

    }
    exit(0);
}


int fcn(void *p, int m, int n, const real *x, real *fvec, int iflag)
{
/*     ********** */

/*     the calling sequence of fcn should be identical to the */
/*     calling sequence of the function subroutine in the nonlinear */
/*     least-squares solver. fcn should only call the testing */
/*     function and jacobian subroutines ssqfcn and ssqjac with */
/*     the appropriate value of problem number (nprob). */

/*     subprograms called */

/*       minpack-supplied ... ssqfcn,ssqjac */

/*     argonne national laboratory. minpack project. march 1980. */
/*     burton s. garbow, kenneth e. hillstrom, jorge j. more */

/*     ********** */
    struct refnum *lmdiftest = (struct refnum *)p;
    ssqfcn(m,n,x,fvec,lmdiftest->nprob);
    if (iflag == 1) {
        lmdiftest->nfev++;
    }
    if (iflag == 2) {
        lmdiftest->njev++;
    }

    return 0;
} /* fcn_ */
