/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include <math.h>
#include "numglobs.h"
#include "numenum.h"
#include "nummacs.h"
#include "onemesh.h"
#include "onedev.h"

#define MIN_DELV 1e-3

/*
 * Functions for projecting the next solution point for modified two-level
 * newton scheme
 */

void 
NUMDproject(pDevice, delV)
  ONEdevice *pDevice;
  double delV;
{
  ONEelem *pElem = pDevice->elemArray[pDevice->numNodes - 1];
  ONEnode *pNode;
  ONEedge *pEdge;
  double delPsi, delN, delP, newN, newP, guessNewConc();
  double *incVpn;
  void ONEstoreInitialGuess();
  int i, index;

  delV = - delV / VNorm;
  pElem->pRightNode->psi += delV;

  if (ABS(delV) < MIN_DELV) {
    ONEstoreInitialGuess(pDevice);
    return;
  }
  /* zero the rhs before loading in the new rhs */
  for (index = 1; index <= pDevice->numEqns; index++) {
    pDevice->rhs[index] = 0.0;
  }
  /* compute incremental changes due to N contact */
  pNode = pElem->pLeftNode;
  pDevice->rhs[pNode->psiEqn] = pElem->epsRel * pElem->rDx;
  if (pElem->elemType IS SEMICON) {
    pEdge = pElem->pEdge;
    pDevice->rhs[pNode->nEqn] = -pEdge->dJnDpsiP1;
    pDevice->rhs[pNode->pEqn] = -pEdge->dJpDpsiP1;
  }
  incVpn = pDevice->dcDeltaSolution;
  spSolve(pDevice->matrix, pDevice->rhs, incVpn);

  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    for (i = 0; i <= 1; i++) {
      if (pElem->evalNodes[i]) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT) {
	  delPsi = incVpn[pNode->psiEqn] * delV;
	  pDevice->dcSolution[pNode->psiEqn] = pNode->psi + delPsi;
	  if (pElem->elemType IS SEMICON) {
	    delN = incVpn[pNode->nEqn] * delV;
	    delP = incVpn[pNode->pEqn] * delV;
	    newN = pNode->nConc + delN;
	    newP = pNode->pConc + delP;
	    /* if new conc less than 0.0 then use a fraction of the guess */
	    if (newN <= 0.0) {
	      pDevice->dcSolution[pNode->nEqn] =
		  guessNewConc(pNode->nConc, delN);
	    } else {
	      pDevice->dcSolution[pNode->nEqn] = newN;
	    }
	    if (newP <= 0.0) {
	      pDevice->dcSolution[pNode->pEqn] =
		  guessNewConc(pNode->pConc, delP);
	    } else {
	      pDevice->dcSolution[pNode->pEqn] = newP;
	    }
	  }
	}
      }
    }
  }
}


void 
NBJTproject(pDevice, delVce, delVbe, vbe)
  ONEdevice *pDevice;
  double delVce, delVbe, vbe;
{
  ONEelem *pLastElem = pDevice->elemArray[pDevice->numNodes - 1];
  ONEelem *pBaseElem = pDevice->elemArray[pDevice->baseIndex - 1];
  ONEelem *pElem;
  ONEnode *pNode;
  ONEedge *pEdge;
  double delPsi, delN, delP, newN, newP, guessNewConc(), *incVce, *incVbe;
  double baseConc;
  int i, index;
  double nConc, pConc;
  void ONEstoreInitialGuess();

  /* normalize the voltages for calculations */
  delVce = delVce / VNorm;
  delVbe = delVbe / VNorm;
  pLastElem->pRightNode->psi += delVce;
  pBaseElem->pRightNode->vbe = vbe / VNorm + pBaseElem->matlInfo->refPsi;
  pNode = pBaseElem->pRightNode;
  if (pNode->baseType IS N_TYPE) {
    baseConc = pNode->nConc;
  } else if (pNode->baseType IS P_TYPE) {
    baseConc = pNode->pConc;
  }
  if (ABS(delVce) > MIN_DELV) {

    /* zero the rhs before loading in the new rhs */
    for (index = 1; index <= pDevice->numEqns; index++) {
      pDevice->rhs[index] = 0.0;
    }
    /* store the new rhs for computing the incremental quantities */
    pNode = pLastElem->pLeftNode;
    pDevice->rhs[pNode->psiEqn] = pLastElem->epsRel * pLastElem->rDx;
    if (pLastElem->elemType IS SEMICON) {
      pEdge = pLastElem->pEdge;
      pDevice->rhs[pNode->nEqn] = -pEdge->dJnDpsiP1;
      pDevice->rhs[pNode->pEqn] = -pEdge->dJpDpsiP1;
    }
    incVce = pDevice->dcDeltaSolution;
    spSolve(pDevice->matrix, pDevice->rhs, incVce);

    for (index = 1; index < pDevice->numNodes; index++) {
      pElem = pDevice->elemArray[index];
      for (i = 0; i <= 1; i++) {
	if (pElem->evalNodes[i]) {
	  pNode = pElem->pNodes[i];
	  if (pNode->nodeType ISNOT CONTACT) {
	    delPsi = incVce[pNode->psiEqn] * delVce;
	    pDevice->dcSolution[pNode->psiEqn] = pNode->psi + delPsi;
	    if (pElem->elemType IS SEMICON) {
	      delN = incVce[pNode->nEqn] * delVce;
	      delP = incVce[pNode->pEqn] * delVce;
	      newN = pNode->nConc + delN;
	      newP = pNode->pConc + delP;
	      /* if new conc less than 0.0 then use a fraction of the guess */
	      if (newN <= 0.0) {
		pDevice->dcSolution[pNode->nEqn] =
		    guessNewConc(pNode->nConc, delN);
	      } else {
		pDevice->dcSolution[pNode->nEqn] = newN;
	      }
	      if (newP <= 0.0) {
		pDevice->dcSolution[pNode->pEqn] =
		    guessNewConc(pNode->pConc, delP);
	      } else {
		pDevice->dcSolution[pNode->pEqn] = newP;
	      }
	    }
	  }
	}
      }
    }
  } else {
    ONEstoreInitialGuess(pDevice);
  }

  if (ABS(delVbe) > MIN_DELV) {

    /* zero the rhs before loading in the new rhs base contribution */
    for (index = 1; index <= pDevice->numEqns; index++) {
      pDevice->rhs[index] = 0.0;
    }
    pNode = pBaseElem->pRightNode;
    if (pNode->baseType IS N_TYPE) {
      pDevice->rhs[pNode->nEqn] = baseConc * pNode->eg;
    } else if (pNode->baseType IS P_TYPE) {
      pDevice->rhs[pNode->pEqn] = baseConc * pNode->eg;
    }

    incVbe = pDevice->copiedSolution;
    spSolve(pDevice->matrix, pDevice->rhs, incVbe);

    for (index = 1; index < pDevice->numNodes; index++) {
      pElem = pDevice->elemArray[index];
      for (i = 0; i <= 1; i++) {
	if (pElem->evalNodes[i]) {
	  pNode = pElem->pNodes[i];
	  if (pNode->nodeType ISNOT CONTACT) {
	    delPsi = incVbe[pNode->psiEqn] * delVbe;
	    pDevice->dcSolution[pNode->psiEqn] += delPsi;
	    if (pElem->elemType IS SEMICON) {
	      delN = incVbe[pNode->nEqn] * delVbe;
	      delP = incVbe[pNode->pEqn] * delVbe;
	      nConc = pDevice->dcSolution[pNode->nEqn];
	      pConc = pDevice->dcSolution[pNode->pEqn];
	      newN = nConc + delN;
	      newP = pConc + delP;
	      /* if new conc less than 0.0 then use a fraction of the guess */
	      if (newN <= 0.0) {
		pDevice->dcSolution[pNode->nEqn] =
		    guessNewConc(nConc, delN);
	      } else {
		pDevice->dcSolution[pNode->nEqn] = newN;
	      }
	      if (newP <= 0.0) {
		pDevice->dcSolution[pNode->pEqn] =
		    guessNewConc(pConc, delP);
	      } else {
		pDevice->dcSolution[pNode->pEqn] = newP;
	      }
	    }
	  }
	}
      }
    }
  }
}

/* functions to update device states for full-LU algorithm */

void 
NUMDupdate(pDevice, delV, updateBoundary)
  ONEdevice *pDevice;
  double delV;
  BOOLEAN updateBoundary;
{
  ONEelem *pElem = pDevice->elemArray[pDevice->numNodes - 1];
  ONEnode *pNode;
  ONEedge *pEdge;
  double delPsi, delN, delP;
  int i, index;

  delV = - delV / VNorm;
  if (updateBoundary) {
    pElem->pRightNode->psi += delV;
  }
  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    for (i = 0; i <= 1; i++) {
      if (pElem->evalNodes[i]) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT) {
	  delPsi = pDevice->dcDeltaSolution[pNode->psiEqn] * delV;
	  pDevice->dcSolution[pNode->psiEqn] = pNode->psi + delPsi;
	  if (pElem->elemType IS SEMICON) {
	    delN = pDevice->dcDeltaSolution[pNode->nEqn] * delV;
	    delP = pDevice->dcDeltaSolution[pNode->pEqn] * delV;
	    pDevice->dcSolution[pNode->nEqn] = pNode->nConc + delN;
	    pDevice->dcSolution[pNode->pEqn] = pNode->pConc + delP;
	  }
	}
      }
    }
  }
}

void 
NBJTupdate(pDevice, delVce, delVbe, vbe, updateBoundary)
  ONEdevice *pDevice;
  double delVce, delVbe, vbe;
  BOOLEAN updateBoundary;
{
  ONEelem *pLastElem = pDevice->elemArray[pDevice->numNodes - 1];
  ONEelem *pBaseElem = pDevice->elemArray[pDevice->baseIndex - 1];
  ONEelem *pElem;
  ONEnode *pNode;
  double delPsi, delN, delP, *incVce, *incVbe;
  int i, index;

  /* normalize the voltages for calculations */
  delVce = delVce / VNorm;
  delVbe = delVbe / VNorm;
  if (updateBoundary) {
    pLastElem->pRightNode->psi += delVce;
    pBaseElem->pRightNode->vbe = vbe / VNorm + pBaseElem->matlInfo->refPsi;
  }
  /*
   * The incremental quantities are available from NBJTconductance. incVce
   * (dcDeltaSolution) and incVbe (copiedSolution) are used to store the
   * incremental quantities associated with Vce and Vbe
   */

  /* set incVce = dcDeltaSolution; incVbe = copiedSolution */
  incVce = pDevice->dcDeltaSolution;
  incVbe = pDevice->copiedSolution;

  for (index = 1; index < pDevice->numNodes; index++) {
    pElem = pDevice->elemArray[index];
    for (i = 0; i <= 1; i++) {
      if (pElem->evalNodes[i]) {
	pNode = pElem->pNodes[i];
	if (pNode->nodeType ISNOT CONTACT) {
	  delPsi = incVce[pNode->psiEqn] * delVce
	      + incVbe[pNode->psiEqn] * delVbe;
	  pDevice->dcSolution[pNode->psiEqn] = pNode->psi + delPsi;
	  if (pElem->elemType IS SEMICON) {
	    delN = incVce[pNode->nEqn] * delVce
		+ incVbe[pNode->nEqn] * delVbe;
	    delP = incVce[pNode->pEqn] * delVce
		+ incVbe[pNode->pEqn] * delVbe;
	    pDevice->dcSolution[pNode->nEqn] = pNode->nConc + delN;
	    pDevice->dcSolution[pNode->pEqn] = pNode->pConc + delP;
	  }
	}
      }
    }
  }
}

void 
NUMDsetBCs(pDevice, vpn)
  ONEdevice *pDevice;
  double vpn;
{
  ONEelem *pElem = pDevice->elemArray[pDevice->numNodes - 1];

  /* normalize the voltage */
  vpn =  - vpn / VNorm;

  /* set the boundary conditions */
  pElem->pRightNode->psi = vpn + pElem->pRightNode->psi0;
}

void 
NBJTsetBCs(pDevice, vce, vbe)
  ONEdevice *pDevice;
  double vce, vbe;
{
  ONEelem *pLastElem = pDevice->elemArray[pDevice->numNodes - 1];
  ONEelem *pBaseElem = pDevice->elemArray[pDevice->baseIndex - 1];
  ONEnode *pNode;
  double psi, conc, sign, absConc;
  double nie, ni, pi;

  /* normalize the voltages */
  vce = vce / VNorm;
  vbe = vbe / VNorm;

  /* set the boundary conditions */
  pBaseElem->pRightNode->vbe = vbe + pBaseElem->matlInfo->refPsi;
  pLastElem->pRightNode->psi = vce + pLastElem->pRightNode->psi0;

  /*
   * if (pLastElem->elemType IS INSULATOR) { pNode->psi = RefPsi -
   * pNode->eaff; pNode->nConc = 0.0; pNode->pConc = 0.0; } else if
   * (pLastElem->elemType IS SEMICON) { nie = pNode->nie; conc =
   * pNode->netConc / pNode->nie; psi = 0.0; ni = nie; pi = nie; sign = SGN(
   * conc ); absConc = ABS( conc ); if ( conc ISNOT 0.0 ) { psi = sign * log(
   * 0.5 * absConc + sqrt( 1.0 + 0.25*absConc*absConc )); ni = nie * exp( psi
   * ); pi = nie * exp( -psi ); } pNode->psi = pLastElem->matlInfo->refPsi +
   * psi; pNode->nConc = ni; pNode->pConc = pi; } pNode->psi = pNode->psi0;
   * pNode->psi += vce;
   */
}
