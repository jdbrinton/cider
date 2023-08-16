/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author: 1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

/* Functions to compute the ac admittances of a device. */

#include <math.h>
#include "numglobs.h"
#include "numenum.h"
#include "nummacs.h"
#include "numconst.h"
#include "twodev.h"
#include "twomesh.h"
#include "numcmplx.h"
#include "spmatrix.h"

#include "ifsim.h"
extern IFfrontEnd *SPfrontEnd;

/* Forward declarations. */
BOOLEAN TWOsorSolve();
void TWO_jacLoad(), TWONjacLoad(), TWOPjacLoad();
void storeNewRhs();
complex *contactAdmittance(), *oxideAdmittance();

/* Temporary hack to remove NUMOS gate special case */
#ifdef NORMAL_GATE
#define GateTypeAdmittance oxideAdmittance
#else
#define GateTypeAdmittance contactAdmittance
#endif				/* NORMAL_GATE */

/* AC / PZ debugging flag */
int TWOacDebug = 0;


int
NUMD2admittance(pDevice, omega, yd)
  TWOdevice *pDevice;
  double omega;
  complex *yd;
{
  TWOnode *pNode;
  TWOelem *pElem;
  int index, eIndex;
  double dxdy;
  double *solnReal, *solnImag;
  double *rhsReal, *rhsImag;
  complex yAc, cOmega, *y;
  BOOLEAN deltaVContact = FALSE;
  BOOLEAN SORFailed;
  double startTime;

  /* Each time we call this counts as one AC iteration. */
  pDevice->pStats->numIters[STAT_AC] += 1;

  /*
   * change context names of solution vectors for ac analysis dcDeltaSolution
   * stores the real part and copiedSolution stores the imaginary part of the
   * ac solution vector
   */
  pDevice->solverType = SLV_SMSIG;
  rhsReal = pDevice->rhs;
  rhsImag = pDevice->rhsImag;
  solnReal = pDevice->dcDeltaSolution;
  solnImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  omega *= TNorm;
  CMPLX_ASSIGN_VALUE(cOmega, 0.0, omega);

  if (AcAnalysisMethod IS SOR OR AcAnalysisMethod IS SOR_ONLY) {
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    /* zero the rhsImag */
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    /* store the new rhs vector */
    storeNewRhs(pDevice, pDevice->pLastContact);
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    SORFailed = TWOsorSolve(pDevice, solnReal, solnImag, omega);
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
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    /* solve the system of equations directly */
    if (NOT OneCarrier) {
      TWO_jacLoad(pDevice);
    } else if (OneCarrier IS N_TYPE) {
      TWONjacLoad(pDevice);
    } else if (OneCarrier IS P_TYPE) {
      TWOPjacLoad(pDevice);
    }
    storeNewRhs(pDevice, pDevice->pLastContact);

    spSetComplex(pDevice->matrix);
    for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
      pElem = pDevice->elements[eIndex];
      if (pElem->elemType IS SEMICON) {
	dxdy = 0.25 * pElem->dx * pElem->dy;
	for (index = 0; index <= 3; index++) {
	  pNode = pElem->pNodes[index];
	  if (pNode->nodeType ISNOT CONTACT) {
	    if (NOT OneCarrier) {
	      spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -dxdy * omega);
	      spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, dxdy * omega);
	    } else if (OneCarrier IS N_TYPE) {
	      spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -dxdy * omega);
	    } else if (OneCarrier IS P_TYPE) {
	      spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, dxdy * omega);
	    }
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
    spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;
  }
  /* MISC */
  startTime = SPfrontEnd->IFseconds();
  y = contactAdmittance(pDevice, pDevice->pFirstContact, deltaVContact,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc, -y->real, -y->imag);
  CMPLX_ASSIGN(*yd, yAc);
  CMPLX_MULT_SELF_SCALAR(*yd, GNorm * pDevice->width * LNorm);
  pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

  return (AcAnalysisMethod);
}


int
NBJT2admittance(pDevice, omega, yIeVce, yIcVce, yIeVbe, yIcVbe)
  TWOdevice *pDevice;
  double omega;
  complex *yIeVce, *yIcVce, *yIeVbe, *yIcVbe;
{
  TWOcontact *pEmitContact = pDevice->pLastContact;
  TWOcontact *pColContact = pDevice->pFirstContact;
  TWOcontact *pBaseContact = pDevice->pFirstContact->next;
  TWOnode *pNode;
  TWOelem *pElem;
  int index, eIndex;
  double width = pDevice->width;
  double dxdy;
  double *solnReal, *solnImag;
  double *rhsReal, *rhsImag;
  BOOLEAN SORFailed;
  complex *y;
  complex pIeVce, pIcVce, pIeVbe, pIcVbe;
  complex cOmega;
  double startTime;

  /* Each time we call this counts as one AC iteration. */
  pDevice->pStats->numIters[STAT_AC] += 1;

  pDevice->solverType = SLV_SMSIG;
  rhsReal = pDevice->rhs;
  rhsImag = pDevice->rhsImag;
  solnReal = pDevice->dcDeltaSolution;
  solnImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  omega *= TNorm;
  CMPLX_ASSIGN_VALUE(cOmega, 0.0, omega);

  if (AcAnalysisMethod IS SOR OR AcAnalysisMethod IS SOR_ONLY) {
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    /* zero the rhs before loading in the new rhs */
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    storeNewRhs(pDevice, pColContact);
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    SORFailed = TWOsorSolve(pDevice, solnReal, solnImag, omega);
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
      y = contactAdmittance(pDevice, pEmitContact, FALSE,
	  solnReal, solnImag, &cOmega);
      CMPLX_ASSIGN_VALUE(pIeVce, y->real, y->imag);
      y = contactAdmittance(pDevice, pColContact, TRUE,
	  solnReal, solnImag, &cOmega);
      CMPLX_ASSIGN_VALUE(pIcVce, y->real, y->imag);
      pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

      /* LOAD */
      startTime = SPfrontEnd->IFseconds();
      /* load in the base contribution to the rhs */
      for (index = 1; index <= pDevice->numEqns; index++) {
	rhsImag[index] = 0.0;
      }
      storeNewRhs(pDevice, pBaseContact);
      pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

      /* SOLVE */
      startTime = SPfrontEnd->IFseconds();
      SORFailed = TWOsorSolve(pDevice, solnReal, solnImag, omega);
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
      rhsImag[index] = 0.0;
    }
    /* solve the system of equations directly */
    if (NOT OneCarrier) {
      TWO_jacLoad(pDevice);
    } else if (OneCarrier IS N_TYPE) {
      TWONjacLoad(pDevice);
    } else if (OneCarrier IS P_TYPE) {
      TWOPjacLoad(pDevice);
    }
    storeNewRhs(pDevice, pColContact);
    spSetComplex(pDevice->matrix);
    for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
      pElem = pDevice->elements[eIndex];
      if (pElem->elemType IS SEMICON) {
	dxdy = 0.25 * pElem->dx * pElem->dy;
	for (index = 0; index <= 3; index++) {
	  pNode = pElem->pNodes[index];
	  if (pNode->nodeType ISNOT CONTACT) {
	    if (NOT OneCarrier) {
	      spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -dxdy * omega);
	      spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, dxdy * omega);
	    } else if (OneCarrier IS N_TYPE) {
	      spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -dxdy * omega);
	    } else if (OneCarrier IS P_TYPE) {
	      spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, dxdy * omega);
	    }
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
    spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* MISC */
    startTime = SPfrontEnd->IFseconds();
    y = contactAdmittance(pDevice, pEmitContact, FALSE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(pIeVce, y->real, y->imag);
    y = contactAdmittance(pDevice, pColContact, TRUE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(pIcVce, y->real, y->imag);
    pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    storeNewRhs(pDevice, pBaseContact);
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* FACTOR: already done, no need to repeat. */

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;
  }
  /* MISC */
  startTime = SPfrontEnd->IFseconds();
  y = contactAdmittance(pDevice, pEmitContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIeVbe, y->real, y->imag);
  y = contactAdmittance(pDevice, pColContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIcVbe, y->real, y->imag);

  CMPLX_ASSIGN(*yIeVce, pIeVce);
  CMPLX_ASSIGN(*yIeVbe, pIeVbe);
  CMPLX_ASSIGN(*yIcVce, pIcVce);
  CMPLX_ASSIGN(*yIcVbe, pIcVbe);
  CMPLX_MULT_SELF_SCALAR(*yIeVce, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(*yIeVbe, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(*yIcVce, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(*yIcVbe, GNorm * width * LNorm);
  pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

  return (AcAnalysisMethod);
}

int
NUMOSadmittance(pDevice, omega, yAc)
  TWOdevice *pDevice;
  double omega;
  struct mosAdmittances *yAc;
{
  TWOcontact *pDContact = pDevice->pFirstContact;
  TWOcontact *pGContact = pDevice->pFirstContact->next;
  TWOcontact *pSContact = pDevice->pFirstContact->next->next;
  TWOcontact *pBContact = pDevice->pLastContact;
  TWOnode *pNode;
  TWOelem *pElem;
  int index, eIndex;
  double width = pDevice->width;
  double dxdy;
  double *solnReal, *solnImag;
  double *rhsReal, *rhsImag;
  BOOLEAN SORFailed;
  complex *y, cOmega;
  double startTime;

  /* Each time we call this counts as one AC iteration. */
  pDevice->pStats->numIters[STAT_AC] += 1;

  pDevice->solverType = SLV_SMSIG;
  rhsReal = pDevice->rhs;
  rhsImag = pDevice->rhsImag;
  solnReal = pDevice->dcDeltaSolution;
  solnImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  omega *= TNorm;
  CMPLX_ASSIGN_VALUE(cOmega, 0.0, omega);

  if (AcAnalysisMethod IS SOR OR AcAnalysisMethod IS SOR_ONLY) {
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    /* zero the rhs before loading in the new rhs */
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    storeNewRhs(pDevice, pDContact);
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    SORFailed = TWOsorSolve(pDevice, solnReal, solnImag, omega);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;
    if (SORFailed AND AcAnalysisMethod IS SOR) {
      AcAnalysisMethod = DIRECT;
      printf("SOR failed at %g Hz, switching to direct-method ac analysis.\n",
	  omega / (TWO_PI * TNorm) );
    } else if (SORFailed) {	/* Told to only do SOR, so give up. */
      printf("SOR failed at %g Hz, returning null admittance.\n",
	  omega / (TWO_PI * TNorm) );
      CMPLX_ASSIGN_VALUE(yAc->yIdVdb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIdVsb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIdVgb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIsVdb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIsVsb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIsVgb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIgVdb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIgVsb, 0.0, 0.0);
      CMPLX_ASSIGN_VALUE(yAc->yIgVgb, 0.0, 0.0);
      return (AcAnalysisMethod);
    } else {
      /* MISC */
      startTime = SPfrontEnd->IFseconds();
      y = contactAdmittance(pDevice, pDContact, TRUE,
	  solnReal, solnImag, &cOmega);
      CMPLX_ASSIGN_VALUE(yAc->yIdVdb, y->real, y->imag);
      y = contactAdmittance(pDevice, pSContact, FALSE,
	  solnReal, solnImag, &cOmega);
      CMPLX_ASSIGN_VALUE(yAc->yIsVdb, y->real, y->imag);
      y = GateTypeAdmittance(pDevice, pGContact, FALSE,
	  solnReal, solnImag, &cOmega);
      CMPLX_ASSIGN_VALUE(yAc->yIgVdb, y->real, y->imag);
      pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

      /* LOAD */
      startTime = SPfrontEnd->IFseconds();
      /* load in the source contribution to the rhs */
      for (index = 1; index <= pDevice->numEqns; index++) {
	rhsImag[index] = 0.0;
      }
      storeNewRhs(pDevice, pSContact);
      pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

      /* SOLVE */
      startTime = SPfrontEnd->IFseconds();
      SORFailed = TWOsorSolve(pDevice, solnReal, solnImag, omega);
      pDevice->pStats->solveTime[STAT_AC] +=
	  SPfrontEnd->IFseconds() - startTime;
      if (SORFailed AND AcAnalysisMethod IS SOR) {
	AcAnalysisMethod = DIRECT;
	printf("SOR failed at %g Hz, switching to direct-method ac analysis.\n",
	    omega / (TWO_PI * TNorm) );
      } else if (SORFailed) {	/* Told to only do SOR, so give up. */
	printf("SOR failed at %g Hz, returning null admittance.\n",
	    omega / (TWO_PI * TNorm) );
	CMPLX_ASSIGN_VALUE(yAc->yIdVdb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIdVsb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIdVgb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIsVdb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIsVsb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIsVgb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIgVdb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIgVsb, 0.0, 0.0);
	CMPLX_ASSIGN_VALUE(yAc->yIgVgb, 0.0, 0.0);
	return (AcAnalysisMethod);
      } else {
	/* MISC */
	startTime = SPfrontEnd->IFseconds();
	y = contactAdmittance(pDevice, pDContact, FALSE,
	    solnReal, solnImag, &cOmega);
	CMPLX_ASSIGN_VALUE(yAc->yIdVsb, y->real, y->imag);
	y = contactAdmittance(pDevice, pSContact, TRUE,
	    solnReal, solnImag, &cOmega);
	CMPLX_ASSIGN_VALUE(yAc->yIsVsb, y->real, y->imag);
	y = GateTypeAdmittance(pDevice, pGContact, FALSE,
	    solnReal, solnImag, &cOmega);
	CMPLX_ASSIGN_VALUE(yAc->yIgVsb, y->real, y->imag);
	pDevice->pStats->miscTime[STAT_AC] +=
	    SPfrontEnd->IFseconds() - startTime;

	/* LOAD */
	startTime = SPfrontEnd->IFseconds();
	/* load in the gate contribution to the rhs */
	for (index = 1; index <= pDevice->numEqns; index++) {
	  rhsImag[index] = 0.0;
	}
	storeNewRhs(pDevice, pGContact);
	pDevice->pStats->loadTime[STAT_AC] +=
	    SPfrontEnd->IFseconds() - startTime;

	/* SOLVE */
	startTime = SPfrontEnd->IFseconds();
	SORFailed = TWOsorSolve(pDevice, solnReal, solnImag, omega);
	pDevice->pStats->solveTime[STAT_AC] +=
	    SPfrontEnd->IFseconds() - startTime;
	if (SORFailed AND AcAnalysisMethod IS SOR) {
	  AcAnalysisMethod = DIRECT;
	  printf("SOR failed at %g Hz, switching to direct-method ac analysis.\n",
	      omega / (TWO_PI * TNorm) );
	} else if (SORFailed) {	/* Told to only do SOR, so give up. */
	  printf("SOR failed at %g Hz, returning null admittance.\n",
	      omega / (TWO_PI * TNorm) );
	  CMPLX_ASSIGN_VALUE(yAc->yIdVdb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIdVsb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIdVgb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIsVdb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIsVsb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIsVgb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIgVdb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIgVsb, 0.0, 0.0);
	  CMPLX_ASSIGN_VALUE(yAc->yIgVgb, 0.0, 0.0);
	  return (AcAnalysisMethod);
	}
      }
    }
  }
  if (AcAnalysisMethod IS DIRECT) {
    /* solve the system of equations directly */
    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    storeNewRhs(pDevice, pDContact);

    /* Need to load & factor jacobian once. */
    if (NOT OneCarrier) {
      TWO_jacLoad(pDevice);
    } else if (OneCarrier IS N_TYPE) {
      TWONjacLoad(pDevice);
    } else if (OneCarrier IS P_TYPE) {
      TWOPjacLoad(pDevice);
    }
    spSetComplex(pDevice->matrix);
    for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
      pElem = pDevice->elements[eIndex];
      if (pElem->elemType IS SEMICON) {
	dxdy = 0.25 * pElem->dx * pElem->dy;
	for (index = 0; index <= 3; index++) {
	  pNode = pElem->pNodes[index];
	  if (pNode->nodeType ISNOT CONTACT) {
	    if (NOT OneCarrier) {
	      spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -dxdy * omega);
	      spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, dxdy * omega);
	    } else if (OneCarrier IS N_TYPE) {
	      spADD_COMPLEX_ELEMENT(pNode->fNN, 0.0, -dxdy * omega);
	    } else if (OneCarrier IS P_TYPE) {
	      spADD_COMPLEX_ELEMENT(pNode->fPP, 0.0, dxdy * omega);
	    }
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
    spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* MISC */
    startTime = SPfrontEnd->IFseconds();
    y = contactAdmittance(pDevice, pDContact, TRUE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(yAc->yIdVdb, y->real, y->imag);
    y = contactAdmittance(pDevice, pSContact, FALSE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(yAc->yIsVdb, y->real, y->imag);
    y = GateTypeAdmittance(pDevice, pGContact, FALSE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(yAc->yIgVdb, y->real, y->imag);
    pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    storeNewRhs(pDevice, pSContact);
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* FACTOR: already done, no need to repeat. */

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* MISC */
    startTime = SPfrontEnd->IFseconds();
    y = contactAdmittance(pDevice, pDContact, FALSE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(yAc->yIdVsb, y->real, y->imag);
    y = contactAdmittance(pDevice, pSContact, TRUE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(yAc->yIsVsb, y->real, y->imag);
    y = GateTypeAdmittance(pDevice, pGContact, FALSE,
	solnReal, solnImag, &cOmega);
    CMPLX_ASSIGN_VALUE(yAc->yIgVsb, y->real, y->imag);
    pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* LOAD */
    startTime = SPfrontEnd->IFseconds();
    for (index = 1; index <= pDevice->numEqns; index++) {
      rhsImag[index] = 0.0;
    }
    storeNewRhs(pDevice, pGContact);
    pDevice->pStats->loadTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

    /* FACTOR: already done, no need to repeat. */

    /* SOLVE */
    startTime = SPfrontEnd->IFseconds();
    spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
    pDevice->pStats->solveTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;
  }
  /* MISC */
  startTime = SPfrontEnd->IFseconds();
  y = contactAdmittance(pDevice, pDContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIdVgb, y->real, y->imag);
  y = contactAdmittance(pDevice, pSContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIsVgb, y->real, y->imag);
  y = GateTypeAdmittance(pDevice, pGContact, TRUE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIgVgb, y->real, y->imag);

  CMPLX_MULT_SELF_SCALAR(yAc->yIdVdb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIdVsb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIdVgb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIsVdb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIsVsb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIsVgb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIgVdb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIgVsb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIgVgb, GNorm * width * LNorm);
  pDevice->pStats->miscTime[STAT_AC] += SPfrontEnd->IFseconds() - startTime;

  return (AcAnalysisMethod);
}

BOOLEAN 
TWOsorSolve(pDevice, xReal, xImag, omega)
  TWOdevice *pDevice;
  double *xReal, *xImag, omega;
{
  double dxdy;
  double wRelax = 1.0;		/* SOR relaxation parameter */
  double *rhsReal = pDevice->rhs;
  double *rhsSOR = pDevice->rhsImag;
  BOOLEAN SORConverged = FALSE;
  BOOLEAN SORFailed = FALSE;
  BOOLEAN hasSORConverged();
  int numEqns = pDevice->numEqns;
  int iterationNum;
  int indexN, indexP;
  int index, eIndex;
  TWOnode *pNode;
  TWOelem *pElem;

  /* clear xReal and xImag arrays */
  for (index = 1; index <= numEqns; index++) {
    xReal[index] = 0.0;
    xImag[index] = 0.0;
  }

  iterationNum = 1;
  for (; (NOT SORConverged) AND(NOT SORFailed); iterationNum++) {
    for (index = 1; index <= numEqns; index++) {
      rhsSOR[index] = 0.0;
    }
    for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
      pElem = pDevice->elements[eIndex];
      dxdy = 0.25 * pElem->dx * pElem->dy;
      for (index = 0; index <= 3; index++) {
	pNode = pElem->pNodes[index];
	if (pNode->nodeType ISNOT CONTACT AND pElem->elemType IS SEMICON) {
	  if (NOT OneCarrier) {
	    indexN = pNode->nEqn;
	    indexP = pNode->pEqn;
	    rhsSOR[indexN] -= dxdy * omega * xImag[indexN];
	    rhsSOR[indexP] += dxdy * omega * xImag[indexP];
	  } else if (OneCarrier IS N_TYPE) {
	    indexN = pNode->nEqn;
	    rhsSOR[indexN] -= dxdy * omega * xImag[indexN];
	  } else if (OneCarrier IS P_TYPE) {
	    indexP = pNode->pEqn;
	    rhsSOR[indexP] += dxdy * omega * xImag[indexP];
	  }
	}
      }
    }

    /* now add the terms from rhs to rhsImag */
    for (index = 1; index <= numEqns; index++) {
      rhsSOR[index] += rhsReal[index];
    }

    /* compute xReal(k+1). solution stored in rhsImag */
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

    /* now compute the imaginary part of the solution xImag */
    for (index = 1; index <= numEqns; index++) {
      rhsSOR[index] = 0.0;
    }
    for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
      pElem = pDevice->elements[eIndex];
      dxdy = 0.25 * pElem->dx * pElem->dy;
      for (index = 0; index <= 3; index++) {
	pNode = pElem->pNodes[index];
	if (pNode->nodeType ISNOT CONTACT AND pElem->elemType IS SEMICON) {
	  if (NOT OneCarrier) {
	    indexN = pNode->nEqn;
	    indexP = pNode->pEqn;
	    rhsSOR[indexN] += dxdy * omega * xReal[indexN];
	    rhsSOR[indexP] -= dxdy * omega * xReal[indexP];
	  } else if (OneCarrier IS N_TYPE) {
	    indexN = pNode->nEqn;
	    rhsSOR[indexN] += dxdy * omega * xReal[indexN];
	  } else if (OneCarrier IS P_TYPE) {
	    indexP = pNode->pEqn;
	    rhsSOR[indexP] -= dxdy * omega * xReal[indexP];
	  }
	}
      }
    }
    /* compute xImag(k+1) */
    spSolve(pDevice->matrix, rhsSOR, rhsSOR, NIL(spREAL), NIL(spREAL));
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
    if ((iterationNum > 4) AND NOT SORConverged) {
      SORFailed = TRUE;
    }
    if (TWOacDebug)
      printf("SOR iteration number = %d\n", iterationNum);
  }
  return (SORFailed);
}


complex *
contactAdmittance(pDevice, pContact, delVContact, xReal, xImag, cOmega)
  TWOdevice *pDevice;
  TWOcontact *pContact;
  BOOLEAN delVContact;
  double *xReal, *xImag;
  complex *cOmega;
{
  TWOnode *pNode, *pHNode, *pVNode;
  TWOedge *pHEdge, *pVEdge;
  int index, i, indexPsi, indexN, indexP, numContactNodes;
  TWOelem *pElem;
  complex psiAc, nAc, pAc;
  complex prod1, prod2, sum;
  double yReal, yImag;
  double temp;
  complex yTotal;

  CMPLX_ASSIGN_VALUE(yTotal, 0.0, 0.0);

  numContactNodes = pContact->numNodes;
  for (index = 0; index < numContactNodes; index++) {
    pNode = pContact->pNodes[index];
    for (i = 0; i <= 3; i++) {
      pElem = pNode->pElems[i];
      if (pElem ISNOT NIL(TWOelem)) {
	switch (i) {
	case 0:
	  /* the TL element */
	  pHNode = pElem->pBLNode;
	  pVNode = pElem->pTRNode;
	  pHEdge = pElem->pBotEdge;
	  pVEdge = pElem->pRightEdge;
	  if (pElem->elemType IS SEMICON) {
	    /* compute the derivatives with n,p */
	    if (pHNode->nodeType ISNOT CONTACT) {
	      indexN = pHNode->nEqn;
	      indexP = pHNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pHEdge->dJnDn);
	      CMPLX_MULT_SCALAR(prod2, pAc, pHEdge->dJpDp);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dy);
	      CMPLX_SUBT_ASSIGN(yTotal, prod1);
	    }
	    if (pVNode->nodeType ISNOT CONTACT) {
	      indexN = pVNode->nEqn;
	      indexP = pVNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pVEdge->dJnDn);
	      CMPLX_MULT_SCALAR(prod2, pAc, pVEdge->dJpDp);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dx);
	      CMPLX_SUBT_ASSIGN(yTotal, prod1);
	    }
	  }
	  break;
	case 1:
	  /* the TR element */
	  pHNode = pElem->pBRNode;
	  pVNode = pElem->pTLNode;
	  pHEdge = pElem->pBotEdge;
	  pVEdge = pElem->pLeftEdge;
	  if (pElem->elemType IS SEMICON) {
	    /* compute the derivatives with n,p */
	    if (pHNode->nodeType ISNOT CONTACT) {
	      indexN = pHNode->nEqn;
	      indexP = pHNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pHEdge->dJnDnP1);
	      CMPLX_MULT_SCALAR(prod2, pAc, pHEdge->dJpDpP1);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dy);
	      CMPLX_ADD_ASSIGN(yTotal, prod1);
	    }
	    if (pVNode->nodeType ISNOT CONTACT) {
	      indexN = pVNode->nEqn;
	      indexP = pVNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pVEdge->dJnDn);
	      CMPLX_MULT_SCALAR(prod2, pAc, pVEdge->dJpDp);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dx);
	      CMPLX_SUBT_ASSIGN(yTotal, prod1);
	    }
	  }
	  break;
	case 2:
	  /* the BR element */
	  pHNode = pElem->pTRNode;
	  pVNode = pElem->pBLNode;
	  pHEdge = pElem->pTopEdge;
	  pVEdge = pElem->pLeftEdge;
	  if (pElem->elemType IS SEMICON) {
	    /* compute the derivatives with n,p */
	    if (pHNode->nodeType ISNOT CONTACT) {
	      indexN = pHNode->nEqn;
	      indexP = pHNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pHEdge->dJnDnP1);
	      CMPLX_MULT_SCALAR(prod2, pAc, pHEdge->dJpDpP1);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dy);
	      CMPLX_ADD_ASSIGN(yTotal, prod1);
	    }
	    if (pVNode->nodeType ISNOT CONTACT) {
	      indexN = pVNode->nEqn;
	      indexP = pVNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pVEdge->dJnDnP1);
	      CMPLX_MULT_SCALAR(prod2, pAc, pVEdge->dJpDpP1);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dx);
	      CMPLX_ADD_ASSIGN(yTotal, prod1);
	    }
	  }
	  break;
	case 3:
	  /* the BL element */
	  pHNode = pElem->pTLNode;
	  pVNode = pElem->pBRNode;
	  pHEdge = pElem->pTopEdge;
	  pVEdge = pElem->pRightEdge;
	  if (pElem->elemType IS SEMICON) {
	    /* compute the derivatives with n,p */
	    if (pHNode->nodeType ISNOT CONTACT) {
	      indexN = pHNode->nEqn;
	      indexP = pHNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pHEdge->dJnDn);
	      CMPLX_MULT_SCALAR(prod2, pAc, pHEdge->dJpDp);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dy);
	      CMPLX_SUBT_ASSIGN(yTotal, prod1);
	    }
	    if (pVNode->nodeType ISNOT CONTACT) {
	      indexN = pVNode->nEqn;
	      indexP = pVNode->pEqn;
	      CMPLX_ASSIGN_VALUE(nAc, xReal[indexN], xImag[indexN]);
	      CMPLX_ASSIGN_VALUE(pAc, xReal[indexP], xImag[indexP]);
	      CMPLX_MULT_SCALAR(prod1, nAc, pVEdge->dJnDnP1);
	      CMPLX_MULT_SCALAR(prod2, pAc, pVEdge->dJpDpP1);
	      CMPLX_ADD(sum, prod1, prod2);
	      CMPLX_MULT_SCALAR(prod1, sum, 0.5 * pElem->dx);
	      CMPLX_ADD_ASSIGN(yTotal, prod1);
	    }
	  }
	  break;
	}
	if (pElem->elemType IS SEMICON) {
	  if (pHNode->nodeType ISNOT CONTACT) {
	    indexPsi = pHNode->psiEqn;
	    CMPLX_ASSIGN_VALUE(psiAc, xReal[indexPsi], xImag[indexPsi]);
	    temp = 0.5 * pElem->dy * (pHEdge->dJnDpsiP1 + pHEdge->dJpDpsiP1);
	    CMPLX_MULT_SCALAR(prod1, psiAc, temp);
	    CMPLX_ADD_ASSIGN(yTotal, prod1);
	    if (delVContact) {
	      CMPLX_ADD_SELF_SCALAR(yTotal, -temp);
	    }
	  }
	  if (pVNode->nodeType ISNOT CONTACT) {
	    indexPsi = pVNode->psiEqn;
	    CMPLX_ASSIGN_VALUE(psiAc, xReal[indexPsi], xImag[indexPsi]);
	    temp = 0.5 * pElem->dx * (pVEdge->dJnDpsiP1 + pVEdge->dJpDpsiP1);
	    CMPLX_MULT_SCALAR(prod1, psiAc, temp);
	    CMPLX_ADD_ASSIGN(yTotal, prod1);
	    if (delVContact) {
	      CMPLX_ADD_SELF_SCALAR(yTotal, -temp);
	    }
	  }
	}
	/* displacement current terms */
	if (pHNode->nodeType ISNOT CONTACT) {
	  indexPsi = pHNode->psiEqn;
	  CMPLX_ASSIGN_VALUE(psiAc, xReal[indexPsi], xImag[indexPsi]);
	  CMPLX_MULT_SCALAR(prod1, *cOmega, pElem->epsRel * 0.5 * pElem->dyOverDx);
	  CMPLX_MULT(prod2, prod1, psiAc);
	  CMPLX_SUBT_ASSIGN(yTotal, prod2);
	  if (delVContact) {
	    CMPLX_ADD_ASSIGN(yTotal, prod1);
	  }
	}
	if (pVNode->nodeType ISNOT CONTACT) {
	  indexPsi = pVNode->psiEqn;
	  CMPLX_ASSIGN_VALUE(psiAc, xReal[indexPsi], xImag[indexPsi]);
	  CMPLX_MULT_SCALAR(prod1, *cOmega, pElem->epsRel * 0.5 * pElem->dxOverDy);
	  CMPLX_MULT(prod2, prod1, psiAc);
	  CMPLX_SUBT_ASSIGN(yTotal, prod2);
	  if (delVContact) {
	    CMPLX_ADD_ASSIGN(yTotal, prod1);
	  }
	}
      }
    }
  }
  return (&yTotal);
}


complex *
oxideAdmittance(pDevice, pContact, delVContact, xReal, xImag, cOmega)
  TWOdevice *pDevice;
  TWOcontact *pContact;
  BOOLEAN delVContact;
  double *xReal, *xImag;
  complex *cOmega;
{
  TWOnode *pNode, *pHNode, *pVNode;
  TWOedge *pHEdge, *pVEdge;
  int index, i, indexPsi, numContactNodes;
  TWOelem *pElem;
  complex psiAc, nAc, pAc;
  complex prod1, prod2, sum;
  double yReal, yImag;
  complex yTotal;

  CMPLX_ASSIGN_VALUE(yTotal, 0.0, 0.0);

  numContactNodes = pContact->numNodes;
  for (index = 0; index < numContactNodes; index++) {
    pNode = pContact->pNodes[index];
    for (i = 0; i <= 3; i++) {
      pElem = pNode->pElems[i];
      if (pElem ISNOT NIL(TWOelem)) {
	switch (i) {
	case 0:
	  /* the TL element */
	  pHNode = pElem->pBLNode;
	  pVNode = pElem->pTRNode;
	  pHEdge = pElem->pBotEdge;
	  pVEdge = pElem->pRightEdge;
	  break;
	case 1:
	  /* the TR element */
	  pHNode = pElem->pBRNode;
	  pVNode = pElem->pTLNode;
	  pHEdge = pElem->pBotEdge;
	  pVEdge = pElem->pLeftEdge;
	  break;
	case 2:
	  /* the BR element */
	  pHNode = pElem->pTRNode;
	  pVNode = pElem->pBLNode;
	  pHEdge = pElem->pTopEdge;
	  pVEdge = pElem->pLeftEdge;
	  break;
	case 3:
	  /* the BL element */
	  pHNode = pElem->pTLNode;
	  pVNode = pElem->pBRNode;
	  pHEdge = pElem->pTopEdge;
	  pVEdge = pElem->pRightEdge;
	  break;
	}
	/* displacement current terms */
	if (pHNode->nodeType ISNOT CONTACT) {
	  indexPsi = pHNode->psiEqn;
	  CMPLX_ASSIGN_VALUE(psiAc, xReal[indexPsi], xImag[indexPsi]);
	  CMPLX_MULT_SCALAR(prod1, *cOmega, pElem->epsRel * 0.5 * pElem->dyOverDx);
	  CMPLX_MULT(prod2, prod1, psiAc);
	  CMPLX_SUBT_ASSIGN(yTotal, prod2);
	  if (delVContact) {
	    CMPLX_ADD_ASSIGN(yTotal, prod1);
	  }
	}
	if (pVNode->nodeType ISNOT CONTACT) {
	  indexPsi = pVNode->psiEqn;
	  CMPLX_ASSIGN_VALUE(psiAc, xReal[indexPsi], xImag[indexPsi]);
	  CMPLX_MULT_SCALAR(prod1, *cOmega, pElem->epsRel * 0.5 * pElem->dxOverDy);
	  CMPLX_MULT(prod2, prod1, psiAc);
	  CMPLX_SUBT_ASSIGN(yTotal, prod2);
	  if (delVContact) {
	    CMPLX_ADD_ASSIGN(yTotal, prod1);
	  }
	}
      }
    }
  }
  return (&yTotal);
}

void
NUMD2ys(pDevice, s, yIn)
  TWOdevice *pDevice;
  complex *s;
  complex *yIn;
{
  TWOnode *pNode;
  TWOelem *pElem;
  int index, eIndex;
  double dxdy;
  double *solnReal, *solnImag;
  double *rhsReal, *rhsImag;
  complex yAc, *y;
  BOOLEAN deltaVContact = FALSE;
  complex temp, cOmega;

  /*
   * change context names of solution vectors for ac analysis dcDeltaSolution
   * stores the real part and copiedSolution stores the imaginary part of the
   * ac solution vector
   */
  pDevice->solverType = SLV_SMSIG;
  rhsReal = pDevice->rhs;
  rhsImag = pDevice->rhsImag;
  solnReal = pDevice->dcDeltaSolution;
  solnImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  CMPLX_MULT_SCALAR(cOmega, *s, TNorm);
  for (index = 1; index <= pDevice->numEqns; index++) {
    rhsImag[index] = 0.0;
  }
  /* solve the system of equations directly */
  if (NOT OneCarrier) {
    TWO_jacLoad(pDevice);
  } else if (OneCarrier IS N_TYPE) {
    TWONjacLoad(pDevice);
  } else if (OneCarrier IS P_TYPE) {
    TWOPjacLoad(pDevice);
  }
  storeNewRhs(pDevice, pDevice->pLastContact);

  spSetComplex(pDevice->matrix);
  for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
    pElem = pDevice->elements[eIndex];
    if (pElem->elemType IS SEMICON) {
      dxdy = 0.25 * pElem->dx * pElem->dy;
      for (index = 0; index <= 3; index++) {
	pNode = pElem->pNodes[index];
	if (pNode->nodeType ISNOT CONTACT) {
	  if (NOT OneCarrier) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	  } else if (OneCarrier IS N_TYPE) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	  } else if (OneCarrier IS P_TYPE) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	  }
	}
      }
    }
  }

  spFactor(pDevice->matrix);
  spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
  y = contactAdmittance(pDevice, pDevice->pFirstContact, deltaVContact,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc, y->real, y->imag);
  CMPLX_ASSIGN(*yIn, yAc);
  CMPLX_NEGATE_SELF(*yIn);
  CMPLX_MULT_SELF_SCALAR(*yIn, GNorm * pDevice->width * LNorm);
}


void
NBJT2ys(pDevice, s, yIeVce, yIcVce, yIeVbe, yIcVbe)
  TWOdevice *pDevice;
  complex *s;
  complex *yIeVce, *yIcVce, *yIeVbe, *yIcVbe;
{
  TWOcontact *pEmitContact = pDevice->pLastContact;
  TWOcontact *pColContact = pDevice->pFirstContact;
  TWOcontact *pBaseContact = pDevice->pFirstContact->next;
  TWOnode *pNode;
  TWOelem *pElem;
  int index, eIndex;
  double width = pDevice->width;
  double dxdy;
  double *solnReal, *solnImag;
  double *rhsReal, *rhsImag;
  complex *y;
  complex pIeVce, pIcVce, pIeVbe, pIcVbe;
  complex temp, cOmega;

  pDevice->solverType = SLV_SMSIG;
  rhsReal = pDevice->rhs;
  rhsImag = pDevice->rhsImag;
  solnReal = pDevice->dcDeltaSolution;
  solnImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  CMPLX_MULT_SCALAR(cOmega, *s, TNorm);

  for (index = 1; index <= pDevice->numEqns; index++) {
    rhsImag[index] = 0.0;
  }
  /* solve the system of equations directly */
  if (NOT OneCarrier) {
    TWO_jacLoad(pDevice);
  } else if (OneCarrier IS N_TYPE) {
    TWONjacLoad(pDevice);
  } else if (OneCarrier IS P_TYPE) {
    TWOPjacLoad(pDevice);
  }
  storeNewRhs(pDevice, pColContact);
  spSetComplex(pDevice->matrix);
  for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
    pElem = pDevice->elements[eIndex];
    if (pElem->elemType IS SEMICON) {
      dxdy = 0.25 * pElem->dx * pElem->dy;
      for (index = 0; index <= 3; index++) {
	pNode = pElem->pNodes[index];
	if (pNode->nodeType ISNOT CONTACT) {
	  if (NOT OneCarrier) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	  } else if (OneCarrier IS N_TYPE) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	  } else if (OneCarrier IS P_TYPE) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	  }
	}
      }
    }
  }
  spFactor(pDevice->matrix);
  spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);

  y = contactAdmittance(pDevice, pEmitContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIeVce, y->real, y->imag);
  y = contactAdmittance(pDevice, pColContact, TRUE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIcVce, y->real, y->imag);
  for (index = 1; index <= pDevice->numEqns; index++) {
    rhsImag[index] = 0.0;
  }
  storeNewRhs(pDevice, pBaseContact);
  /* don't need to LU factor the jacobian since it exists */
  spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
  y = contactAdmittance(pDevice, pEmitContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIeVbe, y->real, y->imag);
  y = contactAdmittance(pDevice, pColContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(pIcVbe, y->real, y->imag);


  CMPLX_ASSIGN(*yIeVce, pIeVce);
  CMPLX_ASSIGN(*yIeVbe, pIeVbe);
  CMPLX_ASSIGN(*yIcVce, pIcVce);
  CMPLX_ASSIGN(*yIcVbe, pIcVbe);
  CMPLX_MULT_SELF_SCALAR(*yIeVce, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(*yIeVbe, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(*yIcVce, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(*yIcVbe, GNorm * width * LNorm);
}

void
NUMOSys(pDevice, s, yAc)
  TWOdevice *pDevice;
  complex *s;
  struct mosAdmittances *yAc;
{
  TWOcontact *pDContact = pDevice->pFirstContact;
  TWOcontact *pGContact = pDevice->pFirstContact->next;
  TWOcontact *pSContact = pDevice->pFirstContact->next->next;
  TWOcontact *pBContact = pDevice->pLastContact;
  TWOnode *pNode;
  TWOelem *pElem;
  int index, eIndex;
  double width = pDevice->width;
  double dxdy;
  double *rhsReal, *rhsImag;
  double *solnReal, *solnImag;
  complex *y;
  complex temp, cOmega;

  pDevice->solverType = SLV_SMSIG;
  rhsReal = pDevice->rhs;
  rhsImag = pDevice->rhsImag;
  solnReal = pDevice->dcDeltaSolution;
  solnImag = pDevice->copiedSolution;

  /* use a normalized radian frequency */
  CMPLX_MULT_SCALAR(cOmega, *s, TNorm);
  for (index = 1; index <= pDevice->numEqns; index++) {
    rhsImag[index] = 0.0;
  }
  /* solve the system of equations directly */
  if (NOT OneCarrier) {
    TWO_jacLoad(pDevice);
  } else if (OneCarrier IS N_TYPE) {
    TWONjacLoad(pDevice);
  } else if (OneCarrier IS P_TYPE) {
    TWOPjacLoad(pDevice);
  }
  storeNewRhs(pDevice, pDContact);
  spSetComplex(pDevice->matrix);

  for (eIndex = 1; eIndex <= pDevice->numElems; eIndex++) {
    pElem = pDevice->elements[eIndex];
    if (pElem->elemType IS SEMICON) {
      dxdy = 0.25 * pElem->dx * pElem->dy;
      for (index = 0; index <= 3; index++) {
	pNode = pElem->pNodes[index];
	if (pNode->nodeType ISNOT CONTACT) {
	  if (NOT OneCarrier) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	  } else if (OneCarrier IS N_TYPE) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fNN, -temp.real, -temp.imag);
	  } else if (OneCarrier IS P_TYPE) {
	    CMPLX_MULT_SCALAR(temp, cOmega, dxdy);
	    spADD_COMPLEX_ELEMENT(pNode->fPP, temp.real, temp.imag);
	  }
	}
      }
    }
  }

  spFactor(pDevice->matrix);
  spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);

  y = contactAdmittance(pDevice, pDContact, TRUE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIdVdb, y->real, y->imag);
  y = contactAdmittance(pDevice, pSContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIsVdb, y->real, y->imag);
  y = GateTypeAdmittance(pDevice, pGContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIgVdb, y->real, y->imag);

  for (index = 1; index <= pDevice->numEqns; index++) {
    rhsImag[index] = 0.0;
  }
  storeNewRhs(pDevice, pSContact);
  /* don't need to LU factor the jacobian since it exists */
  spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
  y = contactAdmittance(pDevice, pDContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIdVsb, y->real, y->imag);
  y = contactAdmittance(pDevice, pSContact, TRUE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIsVsb, y->real, y->imag);
  y = GateTypeAdmittance(pDevice, pGContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIgVsb, y->real, y->imag);
  for (index = 1; index <= pDevice->numEqns; index++) {
    rhsImag[index] = 0.0;
  }
  storeNewRhs(pDevice, pGContact);
  spSolve(pDevice->matrix, rhsReal, solnReal, rhsImag, solnImag);
  y = contactAdmittance(pDevice, pDContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIdVgb, y->real, y->imag);
  y = contactAdmittance(pDevice, pSContact, FALSE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIsVgb, y->real, y->imag);
  y = GateTypeAdmittance(pDevice, pGContact, TRUE,
      solnReal, solnImag, &cOmega);
  CMPLX_ASSIGN_VALUE(yAc->yIgVgb, y->real, y->imag);

  CMPLX_MULT_SELF_SCALAR(yAc->yIdVdb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIdVsb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIdVgb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIsVdb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIsVsb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIsVgb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIgVdb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIgVsb, GNorm * width * LNorm);
  CMPLX_MULT_SELF_SCALAR(yAc->yIgVgb, GNorm * width * LNorm);
}
