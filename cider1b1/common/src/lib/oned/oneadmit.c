/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

/* Functions to compute small-signal parameters of 1D devices */

#include <math.h>
#include "numglobs.h"
#include "numenum.h"
#include "nummacs.h"
#include "numconst.h"
#include "onedev.h"
#include "onemesh.h"
#include "numcmplx.h"
#include "spmatrix.h"

#include "ifsim.h"
extern IFfrontEnd *SPfrontEnd;

int ONEacDebug = FALSE;

/* External Declarations */
extern void ONE_jacLoad();
extern BOOLEAN hasSORConverged();

/* Forward Declarations */
complex *computeAdmittance();
BOOLEAN ONEsorSolve();

int
NUMDadmittance(pDevice, omega, yd)
  ONEdevice *pDevice;
  double omega;
  complex *yd;
{
  ONEnode *pNode;
  ONEelem *pElem;
  ONEedge *pEdge;
  int index, i;
  double yReal, yImag;
  double *solutionReal, *solutionImag;
  complex yAc, cOmega;
  complex *y;
  BOOLEAN SORFailed;
  double startTime;

  /* Each time we call this counts as one AC iteration. */
  pDevice->pStats->numIters[STAT_AC] += 1;

  /*
   * Change context names of solution vectors for ac analysis.
   * dcDeltaSolution stores the real part and copiedSolution stores the
   * imaginary part of the ac solution vector.
   */
  solutionReal = pDevice->dcDeltaSolution;
  solutionImag = pDevice->copiedSolution;
  pDevice->solverType = SLV_SMSIG;

  /* use a normalized radian frequency */
  omega *= TNorm;
  CMPLX_ASSIGN_VALUE(cOmega, 0.0, omega);
  yReal = 0.0;
  yImag = 0.0;

  if (AcAnalysisMethod IS SOR OR AcAnalysisMethod IS SOR_ONLY) {
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    /* zero the rhs before loading in the new rhs */
    for (index = 1; index <= pDevice->numEqns; index++) {
      pDevice->rhs[index] = 0.0;
      pDevice->rhsImag[index] = 0.0;
    }
    /* store the new rhs vector */
    pElem = pDevice->elemArray[pDevice->numNodes - 1];
    pNode = pElem->pLeftNode;
    pDevice->rhs[pNode->psiEqn] = pElem->epsRel * pElem->rDx;
    if (pElem->elemType IS SEMICON) {
      pEdge = pElem->pEdge;
      pDevice->rhs[pNode->nEqn] -= pEdge->dJnDpsiP1;
      pDevice->rhs[pNode->pEqn] -= pEdge->dJpDpsiP1;
    }
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    SORFailed = ONEsorSolve(pDevice, solutionReal, solutionImag, omega);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    if (SORFailed AND AcAnalysisMethod IS SOR) {
      AcAnalysisMethod = DIRECT;
      printf("SOR failed at %g Hz, switching to direct-method ac analysis.\n",
	  omega / (TWO_PI * TNorm) );
    } else if (SORFailed) {	/* Told to only do SOR, so give up. */
      printf("SOR failed at %g Hz, returning null admittance.\n",
	  omega / (TWO_PI * TNorm) );
      CMPLX_ASSIGN_VALUE(*yd, 0.0, 0.0);
      return (AcAnalysisMethod);
    }
  }
  if (AcAnalysisMethod IS DIRECT) {
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    /* solve the system of equations directly */
    for (index = 1; index <= pDevice->numEqns; index++) {
      pDevice->rhs[index] = 0.0;
      pDevice->rhsImag[index] = 0.0;
    }
    pElem = pDevice->elemArray[pDevice->numNodes - 1];
    pNode = pElem->pLeftNode;
    pDevice->rhs[pNode->psiEqn] = pElem->epsRel * pElem->rDx;
    if (pElem->elemType IS SEMICON) {
      pEdge = pElem->pEdge;
      pDevice->rhs[pNode->nEqn] -= pEdge->dJnDpsiP1;
      pDevice->rhs[pNode->pEqn] -= pEdge->dJpDpsiP1;
    }
    ONE_jacLoad(pDevice);
    spSetComplex(pDevice->matrix);
    for (index = 1; index < pDevice->numNodes; index++) {
      pElem = pDevice->elemArray[index];
      if (pElem->elemType IS SEMICON) {
	for (i = 0; i <= 1; i++) {
	  pNode = pElem->pNodes[i];
	  if (pNode->nodeType ISNOT CONTACT) {
	    spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -0.5 * pElem->dx * omega);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, 0.5 * pElem->dx * omega);
	  }
	}
      }
    }
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* FACTOR */
    startTime = SPfrontEnd->IFseconds();
    spFactor(pDevice->matrix);
    pDevice->pStats->factorTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    spSolve(pDevice->matrix, pDevice->rhs, solutionReal,
	pDevice->rhsImag, solutionImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;
  }
  /* MISC */
  startTime = SPfrontEnd->IFseconds();
  pNode = pDevice->elemArray[1]->pLeftNode;
  y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc, -y->real, -y->imag);
  CMPLX_ASSIGN(*yd, yAc);
  CMPLX_MULT_SELF_SCALAR(*yd, GNorm * pDevice->area);
  pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

  return (AcAnalysisMethod);
}


int
NBJTadmittance(pDevice, omega, yIeVce, yIcVce, yIeVbe, yIcVbe)
  ONEdevice *pDevice;
  double omega;
  complex *yIeVce, *yIcVce, *yIeVbe, *yIcVbe;
{
  ONEelem *pCollElem = pDevice->elemArray[pDevice->numNodes - 1];
  ONEelem *pBaseElem = pDevice->elemArray[pDevice->baseIndex - 1];
  ONEelem *pElem;
  ONEedge *pEdge;
  ONEnode *pNode;
  int index, i;
  double area = pDevice->area;
  double *solutionReal, *solutionImag;
  BOOLEAN SORFailed;
  complex *y;
  complex cOmega, pIeVce, pIcVce, pIeVbe, pIcVbe;
  double startTime;

  /* Each time we call this counts as one AC iteration. */
  pDevice->pStats->numIters[STAT_AC] += 1;

  /*
   * change context names of solution vectors for ac analysis dcDeltaSolution
   * stores the real part and copiedSolution stores the imaginary part of the
   * ac solution vector
   */
  solutionReal = pDevice->dcDeltaSolution;
  solutionImag = pDevice->copiedSolution;
  pDevice->solverType = SLV_SMSIG;

  /* use a normalized radian frequency */
  omega *= TNorm;
  CMPLX_ASSIGN_VALUE(cOmega, 0.0, omega);

  if (AcAnalysisMethod IS SOR OR AcAnalysisMethod IS SOR_ONLY) {
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    /* zero the rhs before loading in the new rhs */
    for (index = 1; index <= pDevice->numEqns; index++) {
      pDevice->rhs[index] = 0.0;
      pDevice->rhsImag[index] = 0.0;
    }
    /* store the new rhs vector */
    pNode = pCollElem->pLeftNode;
    pDevice->rhs[pNode->psiEqn] = pCollElem->epsRel * pCollElem->rDx;
    if (pCollElem->elemType IS SEMICON) {
      pEdge = pCollElem->pEdge;
      pDevice->rhs[pNode->nEqn] -= pEdge->dJnDpsiP1;
      pDevice->rhs[pNode->pEqn] -= pEdge->dJpDpsiP1;
    }
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    SORFailed = ONEsorSolve(pDevice, solutionReal, solutionImag, omega);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;
    if (SORFailed AND AcAnalysisMethod IS SOR) {
      AcAnalysisMethod = DIRECT;
      printf("SOR failed at %g Hz, switching to direct-method ac analysis.\n",
	  omega / (TWO_PI * TNorm) );
    } else if (SORFailed) {	/* Told to only do SOR, so give up. */
      printf("SOR failed at %g Hz, returning null admittance.\n",
	  omega / (TWO_PI * TNorm) );
      CMPLX_ASSIGN_VALUE(*yIeVce, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(*yIcVce, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(*yIeVbe, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(*yIcVbe, 0.0, 0.0);
      return (AcAnalysisMethod);
    } else {
      /* MISC */
      startTime = SPfrontEnd->IFseconds();
      pElem = pDevice->elemArray[1];
      pNode = pElem->pLeftNode;
      y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
      CMPLX_ASSIGN_VALUE(pIeVce, -y->real, -y->imag);
      pNode = pCollElem->pRightNode;
      y = computeAdmittance(pNode, TRUE, solutionReal, solutionImag, &cOmega);
      CMPLX_ASSIGN_VALUE(pIcVce, -y->real, -y->imag);
      pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

       /* LOAD */
      startTime = SPfrontEnd->IFseconds();
      /* load in the base contribution to the rhs */
      for (index = 1; index <= pDevice->numEqns; index++) {
	pDevice->rhs[index] = 0.0;
      }
      pNode = pBaseElem->pRightNode;
      if (pNode->baseType IS N_TYPE) {
	pDevice->rhs[pNode->nEqn] = pNode->nConc * pNode->eg;
      } else if (pNode->baseType IS P_TYPE) {
	pDevice->rhs[pNode->pEqn] = pNode->pConc * pNode->eg;
      } else {
	printf("projectBJTsolution: unknown base type\n");
      }
      pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

      /* SOLVE */
      startTime = SPfrontEnd->IFseconds();
      SORFailed = ONEsorSolve(pDevice, solutionReal, solutionImag, omega);
      pDevice->pStats->solveTime[STAT_AC] +=
	  SPfrontEnd->IFseconds() - startTime;
      if (SORFailed AND AcAnalysisMethod IS SOR) {
	AcAnalysisMethod = DIRECT;
	printf("SOR failed at %g Hz, switching to direct-method ac analysis.\n",
	    omega / (TWO_PI * TNorm) );
      } else if (SORFailed) {	/* Told to only do SOR, so give up. */
	printf("SOR failed at %g Hz, returning null admittance.\n",
	    omega / (TWO_PI * TNorm) );
	CMPLX_ASSIGN_VALUE(*yIeVce, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(*yIcVce, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(*yIeVbe, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(*yIcVbe, 0.0, 0.0);
	return (AcAnalysisMethod);
      }
    }
  }
  if (AcAnalysisMethod IS DIRECT) {
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    for (index = 1; index <= pDevice->numEqns; index++) {
      pDevice->rhs[index] = 0.0;
      pDevice->rhsImag[index] = 0.0;
    }
    /* solve the system of equations directly */
    ONE_jacLoad(pDevice);
    pNode = pCollElem->pLeftNode;
    pDevice->rhs[pNode->psiEqn] = pCollElem->epsRel * pCollElem->rDx;
    if (pCollElem->elemType IS SEMICON) {
      pEdge = pCollElem->pEdge;
      pDevice->rhs[pNode->nEqn] -= pEdge->dJnDpsiP1;
      pDevice->rhs[pNode->pEqn] -= pEdge->dJpDpsiP1;
    }
    spSetComplex(pDevice->matrix);
    for (index = 1; index < pDevice->numNodes; index++) {
      pElem = pDevice->elemArray[index];
      if (pElem->elemType IS SEMICON) {
	for (i = 0; i <= 1; i++) {
	  pNode = pElem->pNodes[i];
	  if (pNode->nodeType ISNOT CONTACT) {
	    spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -0.5 * pElem->dx * omega);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, 0.5 * pElem->dx * omega);
	  }
	}
      }
    }
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* FACTOR */
    startTime = SPfrontEnd->IFseconds();
    spFactor(pDevice->matrix);
    pDevice->pStats->factorTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    spSolve(pDevice->matrix, pDevice->rhs, solutionReal,
	pDevice->rhsImag, solutionImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* MISC */
    startTime = SPfrontEnd->IFseconds();
    pElem = pDevice->elemArray[1];
    pNode = pElem->pLeftNode;
    y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
    CMPLX_ASSIGN_VALUE(pIeVce, -y->real, -y->imag);
    pNode = pCollElem->pRightNode;
    y = computeAdmittance(pNode, TRUE, solutionReal, solutionImag, &cOmega);
    CMPLX_ASSIGN_VALUE(pIcVce, -y->real, -y->imag);
    pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    /* load in the base contribution in the rhs */
    for (index = 1; index <= pDevice->numEqns; index++) {
      pDevice->rhs[index] = 0.0;
    }
    pNode = pBaseElem->pRightNode;
    if (pNode->baseType IS N_TYPE) {
      pDevice->rhs[pNode->nEqn] = pNode->nConc * pNode->eg;
    } else if (pNode->baseType IS P_TYPE) {
      pDevice->rhs[pNode->pEqn] = pNode->pConc * pNode->eg;
    } else {
      printf("\n BJTadmittance: unknown base type");
    }
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* FACTOR: already done, no need to repeat. */

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    spSolve(pDevice->matrix, pDevice->rhs, solutionReal,
	pDevice->rhsImag, solutionImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;
  }
  /* MISC */
  startTime = SPfrontEnd->IFseconds();
  pElem = pDevice->elemArray[1];
  pNode = pElem->pLeftNode;
  y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIeVbe, -y->real, -y->imag);
  pNode = pCollElem->pRightNode;
  y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIcVbe, -y->real, -y->imag);

  CMPLX_ASSIGN(*yIeVce, pIeVce);
  CMPLX_ASSIGN(*yIcVce, pIcVce);
  CMPLX_ASSIGN(*yIeVbe, pIeVbe);
  CMPLX_ASSIGN(*yIcVbe, pIcVbe);
  CMPLX_MULT_SELF_SCALAR(*yIeVce, GNorm * area);
  CMPLX_MULT_SELF_SCALAR(*yIeVbe, GNorm * area);
  CMPLX_MULT_SELF_SCALAR(*yIcVce, GNorm * area);
  CMPLX_MULT_SELF_SCALAR(*yIcVbe, GNorm * area);
  pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

  return (AcAnalysisMethod);
}

BOOLEAN
ONEsorSolve(pDevice, xReal, xImag, omega)
  ONEdevice *pDevice;
  double *xReal, *xImag, omega;
{
  ONEnode *pNode;
  ONEelem *pElem;
  double wRelax = 1.0;		/* SOR relaxation parameter */
  double *rhsSOR = pDevice->rhsImag;
  int numEqns = pDevice->numEqns;
  int numNodes = pDevice->numNodes;
  BOOLEAN SORConverged = FALSE;
  BOOLEAN SORFailed = FALSE;
  int i, index, indexN, indexP, iterationNum;
  double dx;

  /* clear xReal, xImag arrays */
  for (index = 1; index <= numEqns; index++) {
    xReal[index] = 0.0;
    xImag[index] = 0.0;
  }

  iterationNum = 1;
  for (; (NOT SORConverged) AND(NOT SORFailed); iterationNum++) {
    /* first setup the rhs for the real part */
    for (index = 1; index <= numEqns; index++) {
      rhsSOR[index] = 0.0;
    }
    for (index = 1; index < numNodes; index++) {
      pElem = pDevice->elemArray[index];
      dx = 0.5 * pElem->dx;
      for (i = 0; i <= 1; i++) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT AND
	    pElem->elemType IS SEMICON) {
	  indexN = pNode->nEqn;
	  indexP = pNode->pEqn;
	  rhsSOR[indexN] -= dx * omega * xImag[indexN];
	  rhsSOR[indexP] += dx * omega * xImag[indexP];
	}
      }
    }
    /* now setup the rhs for the SOR equations */
    for (index = 1; index <= numEqns; index++) {
      rhsSOR[index] += pDevice->rhs[index];
    }
    /* compute xReal(k+1). solution stored in rhsSOR */
    spSolve(pDevice->matrix, rhsSOR, rhsSOR, NIL(spREAL), NIL(spREAL));

    /* modify solution when wRelax is not 1 */
    if (wRelax ISNOT 1) {
      for (index = 1; index <= numEqns; index++) {
	rhsSOR[index] = (1 - wRelax) * xReal[index] +
	    wRelax * rhsSOR[index];
      }
    }
    if (iterationNum > 1) {
      SORConverged = hasSORConverged(xReal, rhsSOR, numEqns);
    }
    /* copy real solution into xReal */
    for (index = 1; index <= numEqns; index++) {
      xReal[index] = rhsSOR[index];
    }

    /* now compute the imaginary part of the solution, xImag */
    for (index = 1; index <= numEqns; index++) {
      rhsSOR[index] = 0.0;
    }
    for (index = 1; index < numNodes; index++) {
      pElem = pDevice->elemArray[index];
      dx = 0.5 * pElem->dx;
      for (i = 0; i <= 1; i++) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT AND
	    pElem->elemType IS SEMICON) {
	  indexN = pNode->nEqn;
	  indexP = pNode->pEqn;
	  rhsSOR[indexN] += dx * omega * xReal[indexN];
	  rhsSOR[indexP] -= dx * omega * xReal[indexP];
	}
      }
    }
    /* compute xImag(k+1) */
    spSolve(pDevice->matrix, rhsSOR, rhsSOR,
	NIL(spREAL), NIL(spREAL));
    /* modify solution when wRelax is not 1 */
    if (wRelax ISNOT 1) {
      for (index = 1; index <= numEqns; index++) {
	rhsSOR[index] = (1 - wRelax) * xImag[index] +
	    wRelax * rhsSOR[index];
      }
    }
    if (iterationNum > 1) {
      SORConverged = SORConverged AND hasSORConverged(xImag, rhsSOR, numEqns);
    }
    /* copy imag solution into xImag */
    for (index = 1; index <= numEqns; index++) {
      xImag[index] = rhsSOR[index];
    }
    if (ONEacDebug)
      printf("SOR iteration number = %d\n", iterationNum);
    if (iterationNum > 4) {
      SORFailed = TRUE;
    }
  }
  return (SORFailed);
}

void
NUMDys(pDevice, s, yd)
  ONEdevice *pDevice;
  complex *s;
  complex *yd;
{
  ONEnode *pNode;
  ONEelem *pElem;
  ONEedge *pEdge;
  int index, i;
  double *solutionReal, *solutionImag;
  complex temp, cOmega;
  complex *y;

  /*
   * change context names of solution vectors for ac analysis dcDeltaSolution
   * stores the real part and copiedSolution stores the imaginary part of the
   * ac solution vector
   */
  solutionReal = pDevice->dcDeltaSolution;
  solutionImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  CMPLX_MULT_SCALAR(cOmega, *s, TNorm);

  /* solve the system of equations directly */
  for (index = 1; index <= pDevice->numEqns; index++) {
    pDevice->rhs[index] = 0.0;
    pDevice->rhsImag[index] = 0.0;
  }
  ONE_jacLoad(pDevice);
  pElem = pDevice->elemArray[pDevice->numNodes - 1];
  pNode = pElem->pLeftNode;
  pDevice->rhs[pNode->psiEqn] = pElem->epsRel * pElem->rDx;
  if (pElem->elemType IS SEMICON) {
    pEdge = pElem->pEdge;
    pDevice->rhs[pNode->nEqn] -= pEdge->dJnDpsiP1;
    pDevice->rhs[pNode->pEqn] -= pEdge->dJpDpsiP1;
  }
  spSetComplex(pDevice->matrix);
  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    if (pElem->elemType IS SEMICON) {
      for (i = 0; i <= 1; i++) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT) {
	  CMPLX_MULT_SCALAR(temp, cOmega, 0.5 * pElem->dx);
	  spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	  spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	}
      }
    }
  }


  spFactor(pDevice->matrix);
  spSolve(pDevice->matrix, pDevice->rhs, solutionReal,
      pDevice->rhsImag, solutionImag);

  pElem = pDevice->elemArray[1];
  pNode = pElem->pLeftNode;
  y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(*yd, -y->real, -y->imag);
  CMPLX_MULT_SELF_SCALAR(*yd, GNorm * pDevice->area);
}


void
NBJTys(pDevice, s, yIeVce, yIcVce, yIeVbe, yIcVbe)
  ONEdevice *pDevice;
  complex *s;
  complex *yIeVce, *yIcVce, *yIeVbe, *yIcVbe;
{
  ONEelem *pCollElem = pDevice->elemArray[pDevice->numNodes - 1];
  ONEelem *pBaseElem = pDevice->elemArray[pDevice->baseIndex - 1];
  ONEelem *pElem;
  ONEnode *pNode;
  ONEedge *pEdge;
  int index, i;
  complex *y;
  double area = pDevice->area;
  double *solutionReal, *solutionImag;
  void ONE_jacLoad(), spSetComplex();
  complex temp, cOmega;
  complex pIeVce, pIcVce, pIeVbe, pIcVbe;

  /*
   * change context names of solution vectors for ac analysis dcDeltaSolution
   * stores the real part and copiedSolution stores the imaginary part of the
   * ac solution vector
   */

  solutionReal = pDevice->dcDeltaSolution;
  solutionImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  CMPLX_MULT_SCALAR(cOmega, *s, TNorm);
  for (index = 1; index <= pDevice->numEqns; index++) {
    pDevice->rhs[index] = 0.0;
    pDevice->rhsImag[index] = 0.0;
  }
  /* solve the system of equations directly */
  ONE_jacLoad(pDevice);
  pNode = pCollElem->pLeftNode;
  pDevice->rhs[pNode->psiEqn] = pCollElem->epsRel * pCollElem->rDx;
  if (pCollElem->elemType IS SEMICON) {
    pEdge = pCollElem->pEdge;
    pDevice->rhs[pNode->nEqn] -= pEdge->dJnDpsiP1;
    pDevice->rhs[pNode->pEqn] -= pEdge->dJpDpsiP1;
  }
  spSetComplex(pDevice->matrix);
  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    if (pElem->elemType IS SEMICON) {
      for (i = 0; i <= 1; i++) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT) {
	  CMPLX_MULT_SCALAR(temp, cOmega, 0.5 * pElem->dx);
	  spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	  spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	}
      }
    }
  }

  spFactor(pDevice->matrix);
  spSolve(pDevice->matrix, pDevice->rhs, solutionReal,
      pDevice->rhsImag, solutionImag);
  pElem = pDevice->elemArray[1];
  pNode = pElem->pLeftNode;
  y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIeVce, -y->real, -y->imag);
  pNode = pCollElem->pRightNode;
  y = computeAdmittance(pNode, TRUE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIcVce, -y->real, -y->imag);

  /* load in the base contribution in the rhs */
  for (index = 1; index <= pDevice->numEqns; index++) {
    pDevice->rhs[index] = 0.0;
  }
  pNode = pBaseElem->pRightNode;
  if (pNode->baseType IS N_TYPE) {
    pDevice->rhs[pNode->nEqn] = pNode->nConc * pNode->eg;
  } else if (pNode->baseType IS P_TYPE) {
    pDevice->rhs[pNode->pEqn] = pNode->pConc * pNode->eg;
  } else {
    printf("\n BJTadmittance: unknown base type");
  }

  /* don't need to LU factor the jacobian since it exists */
  spSolve(pDevice->matrix, pDevice->rhs, solutionReal,
      pDevice->rhsImag, solutionImag);
  pElem = pDevice->elemArray[1];
  pNode = pElem->pLeftNode;
  y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIeVbe, -y->real, -y->imag);
  pNode = pCollElem->pRightNode;
  y = computeAdmittance(pNode, FALSE, solutionReal, solutionImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIcVbe, -y->real, -y->imag);


  CMPLX_ASSIGN(*yIeVce, pIeVce);
  CMPLX_ASSIGN(*yIcVce, pIcVce);
  CMPLX_ASSIGN(*yIeVbe, pIeVbe);
  CMPLX_ASSIGN(*yIcVbe, pIcVbe);
  CMPLX_MULT_SELF_SCALAR(*yIeVce, GNorm * area);
  CMPLX_MULT_SELF_SCALAR(*yIeVbe, GNorm * area);
  CMPLX_MULT_SELF_SCALAR(*yIcVce, GNorm * area);
  CMPLX_MULT_SELF_SCALAR(*yIcVbe, GNorm * area);
}

/* function to compute the admittance of a one-D device */
/* cOmega is the complex frequency */

complex *
computeAdmittance(pNode, delVContact, xReal, xImag, cOmega)
  ONEnode *pNode;
  BOOLEAN delVContact;
  double *xReal, *xImag;
  complex *cOmega;
{
  ONEnode *pHNode;
  ONEedge *pEdge;
  ONEelem *pElem;
  complex psi, n, p;
  complex sum, prod1, prod2;
  complex yAc;
  double temp;
  int i;

  CMPLX_ASSIGN_VALUE(yAc, 0.0, 0.0);

  for (i = 0; i <= 1; i++) {
    pElem = pNode->pElems[i];
    if (pElem ISNOT NIL(ONEelem)) {
      switch (i) {
      case 0:
	/* the right node of the element */
	pHNode = pElem->pLeftNode;
	pEdge = pElem->pEdge;
	CMPLX_ASSIGN_VALUE(psi, xReal[pHNode->psiEqn],
	    xImag[pHNode->psiEqn]);
	if (pElem->elemType IS SEMICON) {
	  CMPLX_ASSIGN_VALUE(n, xReal[pHNode->nEqn],
	      xImag[pHNode->nEqn]);
	  CMPLX_ASSIGN_VALUE(p, xReal[pHNode->pEqn],
	      xImag[pHNode->pEqn]);

	  CMPLX_MULT_SCALAR(prod1, psi, -pEdge->dJnDpsiP1);
	  CMPLX_MULT_SCALAR(prod2, n, pEdge->dJnDn);
	  CMPLX_ADD(yAc, prod1, prod2);
	  CMPLX_MULT_SCALAR(prod1, psi, -pEdge->dJpDpsiP1);
	  CMPLX_MULT_SCALAR(prod2, p, pEdge->dJpDp);
	  CMPLX_ADD(sum, prod1, prod2);
	  CMPLX_ADD_ASSIGN(yAc, sum);
	  if (delVContact) {
	    CMPLX_ADD_SELF_SCALAR(yAc, pEdge->dJnDpsiP1 + pEdge->dJpDpsiP1);
	  }
	}
	CMPLX_MULT_SCALAR(prod1, *cOmega, pElem->epsRel * pElem->rDx);
	CMPLX_MULT(prod2, prod1, psi)
	    CMPLX_ADD_ASSIGN(yAc, prod2);
	if (delVContact) {
	  CMPLX_SUBT_ASSIGN(yAc, prod1);
	}
	break;

      case 1:
	/* the left node of the element */
	pHNode = pElem->pRightNode;
	pEdge = pElem->pEdge;
	CMPLX_ASSIGN_VALUE(psi, xReal[pHNode->psiEqn],
	    xImag[pHNode->psiEqn]);
	if (pElem->elemType IS SEMICON) {
	  CMPLX_ASSIGN_VALUE(n, xReal[pHNode->nEqn],
	      xImag[pHNode->nEqn]);
	  CMPLX_ASSIGN_VALUE(p, xReal[pHNode->pEqn],
	      xImag[pHNode->pEqn]);
	  CMPLX_MULT_SCALAR(prod1, psi, pEdge->dJnDpsiP1);
	  CMPLX_MULT_SCALAR(prod2, n, pEdge->dJnDnP1);
	  CMPLX_ADD(yAc, prod1, prod2);
	  CMPLX_MULT_SCALAR(prod1, psi, pEdge->dJpDpsiP1);
	  CMPLX_MULT_SCALAR(prod2, p, pEdge->dJpDpP1);
	  CMPLX_ADD(sum, prod1, prod2);
	  CMPLX_ADD_ASSIGN(yAc, sum);

	  if (delVContact) {
	    CMPLX_ADD_SELF_SCALAR(yAc, -(pEdge->dJnDpsiP1 + pEdge->dJpDpsiP1));
	  }
	}
	CMPLX_MULT_SCALAR(prod1, *cOmega, pElem->epsRel * pElem->rDx);
	CMPLX_MULT(prod2, prod1, psi);
	CMPLX_SUBT_ASSIGN(yAc, prod2);
	if (delVContact) {
	  CMPLX_ADD_ASSIGN(yAc, prod1);
	}
	break;
      default:
	/* should never be here. Error */
	printf("computeAdmittance: Error - unknown element\n");
      }
    }
  }
  return (&yAc);
}
