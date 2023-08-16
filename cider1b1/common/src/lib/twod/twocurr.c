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
nodeCurrents(pElem, pNode, mun, mup,
    jnx, jny, jpx, jpy, jdx, jdy)
  TWOelem *pElem;
  TWOnode *pNode;
  double *mun, *mup;
  double *jnx, *jny, *jpx, *jpy, *jdx, *jdy;
{

  TWOelem *pElemTL, *pElemTR, *pElemBL, *pElemBR;
  TWOedge *pEdgeT, *pEdgeB, *pEdgeL, *pEdgeR;
  int materT, materB, materL, materR;
  int numFound = 0;
  double dxL, dxR, dyT, dyB;
  double epsL, epsR, epsT, epsB;
  double coeff1, coeff2;

  /* Find all four neighboring elements */
  pElemTL = pNode->pTLElem;
  pElemTR = pNode->pTRElem;
  pElemBL = pNode->pBLElem;
  pElemBR = pNode->pBRElem;

  /* Null edge pointers */
  pEdgeT = pEdgeB = pEdgeL = pEdgeR = NIL(TWOedge);

  /* Zero mobilities */
  *mun = *mup = 0.0;

  /* Find edges next to node */
  if (pElemTL ISNOT NIL(TWOelem)) {
    numFound++;
    *mun += pElemTL->mun0;
    *mup += pElemTL->mup0;
    if (pElemTL->evalEdges[1]) {
      pEdgeT = pElemTL->pRightEdge;
      materT = pElemTL->elemType;
      dyT = pElemTL->dy;
      epsT = pElemTL->epsRel;
    }
    if (pElemTL->evalEdges[2]) {
      pEdgeL = pElemTL->pBotEdge;
      materL = pElemTL->elemType;
      dxL = pElemTL->dx;
      epsL = pElemTL->epsRel;
    }
  }
  if (pElemTR ISNOT NIL(TWOelem)) {
    numFound++;
    *mun += pElemTR->mun0;
    *mup += pElemTR->mup0;
    if (pElemTR->evalEdges[3]) {
      pEdgeT = pElemTR->pLeftEdge;
      materT = pElemTR->elemType;
      epsT = pElemTR->epsRel;
    }
    if (pElemTR->evalEdges[2]) {
      pEdgeR = pElemTR->pBotEdge;
      materR = pElemTR->elemType;
      dxR = pElemTR->dx;
      epsR = pElemTR->epsRel;
    }
  }
  if (pElemBR ISNOT NIL(TWOelem)) {
    numFound++;
    *mun += pElemBR->mun0;
    *mup += pElemBR->mup0;
    if (pElemBR->evalEdges[3]) {
      pEdgeB = pElemBR->pLeftEdge;
      materB = pElemBR->elemType;
      dyB = pElemBR->dy;
      epsB = pElemBR->epsRel;
    }
    if (pElemBR->evalEdges[0]) {
      pEdgeR = pElemBR->pTopEdge;
      materR = pElemBR->elemType;
      dxR = pElemBR->dx;
      epsR = pElemBR->epsRel;
    }
  }
  if (pElemBL ISNOT NIL(TWOelem)) {
    numFound++;
    *mun += pElemBL->mun0;
    *mup += pElemBL->mup0;
    if (pElemBL->evalEdges[1]) {
      pEdgeB = pElemBL->pRightEdge;
      materB = pElemBL->elemType;
      dyB = pElemBL->dy;
      epsB = pElemBL->epsRel;
    }
    if (pElemBL->evalEdges[0]) {
      pEdgeL = pElemBL->pTopEdge;
      materL = pElemBL->elemType;
      dxL = pElemBL->dx;
      epsL = pElemBL->epsRel;
    }
  }
  *mun /= (double) numFound;
  *mup /= (double) numFound;
  /* compute horizontal vector components */
  /* No more than one of Left Edge or Right Edge is absent */
  /* If one is absent the other is guaranteed to be from silicon */
  if (pEdgeL IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *jnx = pEdgeR->jn;
      *jpx = pEdgeR->jp;
      *jdx = pEdgeR->jd;
    } else {
      *jnx = 0.0;
      *jpx = 0.0;
      *jdx = 0.0;
    }
  } else if (pEdgeR IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *jnx = pEdgeL->jn;
      *jpx = pEdgeL->jp;
      *jdx = pEdgeL->jd;
    } else {
      *jnx = 0.0;
      *jpx = 0.0;
      *jdx = 0.0;
    }
  } else {			/* Both edges are present */
    coeff1 = dxL / (dxL + dxR);
    coeff2 = dxR / (dxL + dxR);
    if (materL IS INSULATOR OR materR IS INSULATOR) {
      *jnx = 0.0;
      *jpx = 0.0;
      *jdx = coeff2 * epsL * pEdgeL->jd + coeff1 * epsR * pEdgeR->jd;
    } else {
      *jnx = coeff2 * pEdgeL->jn + coeff1 * pEdgeR->jn;
      *jpx = coeff2 * pEdgeL->jp + coeff1 * pEdgeR->jp;
      *jdx = coeff2 * pEdgeL->jd + coeff1 * pEdgeR->jd;
    }
  }

  /* compute vertical vector components */
  /* No more than one of Top Edge or Bottom Edge is absent */
  /* If one is absent the other is guaranteed to be from silicon */
  if (pEdgeT IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *jny = pEdgeB->jn;
      *jpy = pEdgeB->jp;
      *jdy = pEdgeB->jd;
    } else {
      *jny = 0.0;
      *jpy = 0.0;
      *jdy = 0.0;
    }
  } else if (pEdgeB IS NIL(TWOedge)) {
    if (pNode->nodeType IS CONTACT) {
      *jny = pEdgeT->jn;
      *jpy = pEdgeT->jp;
      *jdy = pEdgeT->jd;
    } else {
      *jny = 0.0;
      *jpy = 0.0;
      *jdy = 0.0;
    }
  } else {			/* Both edges are present */
    coeff1 = dyT / (dyT + dyB);
    coeff2 = dyB / (dyT + dyB);
    if (materT IS INSULATOR OR materB IS INSULATOR) {
      *jny = 0.0;
      *jpy = 0.0;
      *jdy = coeff2 * epsT * pEdgeT->jd + coeff1 * epsB * pEdgeB->jd;
    } else {
      *jny = coeff2 * pEdgeT->jn + coeff1 * pEdgeB->jn;
      *jpy = coeff2 * pEdgeT->jp + coeff1 * pEdgeB->jp;
      *jdy = coeff2 * pEdgeT->jd + coeff1 * pEdgeB->jd;
    }
  }
}
