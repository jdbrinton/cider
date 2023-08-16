/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include <math.h>
#include "numglobs.h"
#include "nummacs.h"
#include "numenum.h"
#include "twomesh.h"
#include "twodev.h"

static void setDirichlet();

void NUMD2setBCs( pDevice, vd )
TWOdevice *pDevice;
double vd;
{
    TWOcontact *pContact = pDevice->pLastContact;

    setDirichlet( pContact, - vd );
}

void NBJT2setBCs( pDevice, vce, vbe )
TWOdevice *pDevice;
double vce, vbe;
{
    TWOcontact *pCollContact = pDevice->pFirstContact;
    TWOcontact *pBaseContact = pDevice->pFirstContact->next;

    setDirichlet( pCollContact, vce );
    setDirichlet( pBaseContact, vbe );
}

void NUMOSsetBCs( pDevice, vdb, vsb, vgb )
TWOdevice *pDevice;
double vdb, vsb, vgb;
{
    TWOcontact *pDContact = pDevice->pFirstContact;
    TWOcontact *pGContact = pDevice->pFirstContact->next;
    TWOcontact *pSContact = pDevice->pFirstContact->next->next;

    setDirichlet( pDContact, vdb );
    setDirichlet( pSContact, vsb );
    setDirichlet( pGContact, vgb );
}

static void
  setDirichlet( pContact, voltage )
TWOcontact *pContact;
double voltage;
{
  int index, numContactNodes, i;
  TWOelem *pElem;
  TWOnode *pNode;
  double psi, ni, pi, nie;
  double conc, sign, absConc;

  voltage /= VNorm;
  
  numContactNodes = pContact->numNodes;
  for ( index = 0; index < numContactNodes; index++ ) {
    pNode = pContact->pNodes[ index ];

    /* Find this node's owner element. */
    for ( i = 0; i <= 3; i++ ) {
      pElem = pNode->pElems[ i ];
      if ( pElem ISNOT NIL(TWOelem) && pElem->evalNodes[ (i+2)%4 ] ) {
	break; /* got it */
      }
    }

    if (pElem->elemType IS INSULATOR) {
      pNode->psi = RefPsi - pNode->eaff;
      pNode->nConc = 0.0;
      pNode->pConc = 0.0;
    }
    else if (pElem->elemType IS SEMICON) {
      nie = pNode->nie;
      conc = pNode->netConc / nie;
      sign = SGN( conc );
      absConc = ABS( conc );
      if ( conc ISNOT 0.0 ) {
	psi = sign * log( 0.5 * absConc + sqrt( 1.0 + 0.25*absConc*absConc ));
	ni = nie * exp( psi );
	pi = nie * exp( - psi );
      }
      else {
	psi = 0.0;
	ni = nie;
	pi = nie;
      }
      pNode->psi = pElem->matlInfo->refPsi + psi;
      pNode->nConc = ni;
      pNode->pConc = pi;
    }
    pNode->psi += voltage;
  }
}
