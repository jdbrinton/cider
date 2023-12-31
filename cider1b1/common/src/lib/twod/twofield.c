/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
**********/

#include <math.h>
#include "numglobs.h"
#include "numenum.h"
#include "nummacs.h"
#include "twomesh.h"

void
nodeFields(pElem, pNode, ex, ey)
  TWOelem *pElem;
  TWOnode *pNode;
  double *ex, *ey;
{

  TWOelem *pElemTL, *pElemTR, *pElemBL, *pElemBR;
  TWOedge *pEdgeT, *pEdgeB, *pEdgeL, *pEdgeR;
  int materT, materB, materL, materR;
  double dxL, dxR, dyT, dyB;
  double ef1, ef2, coeff1, coeff2;

  /* Find all four neighboring elements */
  pElemTL = pNode->pTLElem;
  pElemTR = pNode->pTRElem;
  pElemBL = pNode->pBLElem;
  pElemBR = pNode->pBRElem;

  /* Null edge pointers */
  pEdgeT = pEdgeB = pEdgeL = pEdgeR = NIL(TWOedge);

  /* Find edges next to node */
  if (pElemTL ISNOT NIL(TWOelem)) {
    if (pElemTL->evalEdges[1]) {
      pEdgeT = pElemTL->pRightEdge;
      materT = pElemTL->elemType;
      dyT = pElemTL->dy;
    }
    if (pElemTL->evalEdges[2]) {
      pEdgeL = pElemTL->pBotEdge;
      materL = pElemTL->elemType;
      dxL = pElemTL->dx;
    }
  }
  if (pElemTR ISNOT NIL(TWOelem)) {
    if (pElemTR->evalEdges[3]) {
      pEdgeT = pElemTR->pLeftEdge;
      materT = pElemTR->elemType;
      dyT = pElemTR->dy;
    }
    if (pElemTR->evalEdges[2]) {
      pEdgeR = pElemTR->pBotEdge;
      materR = pElemTR->elemType;
      dxR = pElemTR->dx;
    }
  }
  if (pElemBR ISNOT NIL(TWOelem)) {
    if (pElemBR->evalEdges[3]) {
      pEdgeB = pElemBR->pLeftEdge;
      materB = pElemBR->elemType;
      dyB = pElemBR->dy;
    }
    if (pElemBR->evalEdges[0]) {
      pEdgeR = pElemBR->pTopEdge;
      materR = pElemBR->elemType;
      dxR = pElemBR->dx;
    }
  }
  if (pElemBL ISNOT NIL(TWOelem)) {
    if (pElemBL->evalEdges[1]) {
      pEdgeB = pElemBL->pRightEdge;
      materB = pElemBL->elemType;
      dyB = pElemBL->dy;
    }
    if (pElemBL->evalEdges[0]) {
      pEdgeL = pElemBL->pTopEdge;
      materL = pElemBL->elemType;
      dxL = pElemBL->dx;
    }
  }
  /* compute horizontal vector components */
  /* No more than one of Left Edge or Right Edge is absent */
  if (pEdgeL IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *ex = -pEdgeR->dPsi / dxR;
    } else {
      *ex = 0.0;
    }
  } else if (pEdgeR IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *ex = -pEdgeL->dPsi / dxL;
    } else {
      *ex = 0.0;
    }
  } else {			/* Both edges are present */
    coeff1 = dxL / (dxL + dxR);
    coeff2 = dxR / (dxL + dxR);
    ef1 = -pEdgeL->dPsi / dxL;
    ef2 = -pEdgeR->dPsi / dxR;
    *ex = coeff2 * ef1 + coeff1 * ef2;
  }

  /* compute vertical vector components */
  /* No more than one of Top Edge or Bottom Edge is absent */
  if (pEdgeT IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *ey = -pEdgeB->dPsi / dyB;
    } else {
      *ey = 0.0;
    }
  } else if (pEdgeB IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *ey = -pEdgeT->dPsi / dyT;
    } else {
      *ey = 0.0;
    }
  } else {			/* Both edges are present */
    coeff1 = dyT / (dyT + dyB);
    coeff2 = dyB / (dyT + dyB);
    ef1 = -pEdgeT->dPsi / dyT;
    ef2 = -pEdgeB->dPsi / dyB;
    *ey = coeff2 * ef1 + coeff1 * ef2;
  }
}
