/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include <math.h>
#include "numglobs.h"
#include "numconst.h"
#include "nummacs.h"
#include "onemesh.h"
#include "accuracy.h"

#define LEVEL_ALPHA_SI 3.1e-8	/* From de Graaf & Klaasen, pg. 12 */

ONEQfreezeOut(pNode, ndFac, naFac, dNdFac, dNaFac)
  ONEnode *pNode;
  double *ndFac, *naFac, *dNdFac, *dNaFac;
{
  double temp1, temp2;
  double eLev;
  ONEmaterial *info;

  if (pNode->pRightElem && pNode->pRightElem->evalNodes[0]) {
    info = pNode->pRightElem->matlInfo;
  } else {
    info = pNode->pLeftElem->matlInfo;
  }

  eLev = info->eDon;
  if (info->material != GAAS) {
    eLev -= LEVEL_ALPHA_SI * pow(pNode->nd * NNorm, 1.0 / 3.0);
    if (eLev < 0.0)
      eLev = 0.0;
  }
  if (eLev >= ExpLim) {
    *ndFac = 0.0;
    *dNdFac = 0.0;
  } else if (eLev <= -ExpLim) {
    *ndFac = 1.0;
    *dNdFac = 0.0;
  } else {
    temp1 = info->gDon * pNode->nConc * NNorm * exp(eLev) / info->nc0;
    temp2 = 1.0 / (1.0 + temp1);
    *ndFac = temp2;
    *dNdFac = -temp2 * temp2 * temp1;
  }

  eLev = info->eAcc;
  if (info->material != GAAS) {
    eLev -= LEVEL_ALPHA_SI * pow(pNode->na * NNorm, 1.0 / 3.0);
    if (eLev < 0.0)
      eLev = 0.0;
  }
  if (eLev >= ExpLim) {
    *naFac = 0.0;
    *dNaFac = 0.0;
  } else if (eLev <= -ExpLim) {
    *naFac = 1.0;
    *dNaFac = 0.0;
  } else {
    temp1 = info->gAcc * pNode->pConc * NNorm * exp(eLev) / info->nv0;
    temp2 = 1.0 / (1.0 + temp1);
    *naFac = temp2;
    *dNaFac = temp2 * temp2 * temp1;
  }
}

ONE_freezeOut(pNode, nConc, pConc, ndFac, naFac, dNdFac, dNaFac)
  ONEnode *pNode;
  double nConc, pConc;
  double *ndFac, *naFac, *dNdFac, *dNaFac;
{
  double temp1, temp2;
  double eLev;
  ONEmaterial *info;

  if (pNode->pRightElem && pNode->pRightElem->evalNodes[0]) {
    info = pNode->pRightElem->matlInfo;
  } else {
    info = pNode->pLeftElem->matlInfo;
  }

  eLev = info->eDon;
  if (info->material != GAAS) {
    eLev -= LEVEL_ALPHA_SI * pow(pNode->nd * NNorm, 1.0 / 3.0);
    if (eLev < 0.0)
      eLev = 0.0;
  }
  if (eLev >= ExpLim) {
    *ndFac = 0.0;
    *dNdFac = 0.0;
  } else if (eLev <= -ExpLim) {
    *ndFac = 1.0;
    *dNdFac = 0.0;
  } else {
    temp1 = info->gDon * exp(eLev) * NNorm / info->nc0;
    temp2 = 1.0 / (1.0 + nConc * temp1);
    *ndFac = temp2;
    *dNdFac = -temp2 * temp2 * temp1;
  }

  eLev = info->eAcc;
  if (info->material != GAAS) {
    eLev -= LEVEL_ALPHA_SI * pow(pNode->na * NNorm, 1.0 / 3.0);
    if (eLev < 0.0)
      eLev = 0.0;
  }
  if (eLev >= ExpLim) {
    *naFac = 0.0;
    *dNaFac = 0.0;
  } else if (eLev <= -ExpLim) {
    *naFac = 1.0;
    *dNaFac = 0.0;
  } else {
    temp1 = info->gAcc * exp(eLev) * NNorm / info->nv0;
    temp2 = 1.0 / (1.0 + pConc * temp1);
    *naFac = temp2;
    *dNaFac = -temp2 * temp2 * temp1;
  }
}
