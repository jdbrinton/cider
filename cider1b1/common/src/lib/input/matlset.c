/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author: 1992 David A. Gates, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include <stdio.h>
#include <math.h>
#include "const.h"
#include "util.h"
#include "cktdefs.h"
#include "nummacs.h"
#include "numconst.h"
#include "numenum.h"
#include "matldefs.h"
#include "material.h"
#include "sperror.h"
#include "suffix.h"

#ifdef __STDC__
extern int MATLcheck( MATLcard * );
extern int MATLsetup( MATLcard *, MaterialInfo ** );
#else
extern int MATLcheck();
extern int MATLsetup();
#endif /* STDC */



/*
 * Name:	MATLcheck
 * Purpose:	checks a list of MATLcards for input errors
 * Formals:	cardList: the list to check
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical device setup routines
 * Calls:	error message handler
 */
int
  MATLcheck( cardList )
MATLcard *cardList;
{
  register MATLcard *card, *card2;
  int cardNum = 0, cardNum2;
  int error = OK;
  char ebuf[512];		/* error message buffer */
  
  for ( card = cardList; card != NIL(MATLcard); card = card->MATLnextCard ) {
    cardNum++;

    if( !card->MATLmaterialGiven ) {
      card->MATLmaterial = SILICON;
    }
    if (!card->MATLnumberGiven) {
      sprintf( ebuf,
	  "material card %d is missing an id number",
	  cardNum );
      SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
      error = E_PRIVATE;
    }

    /* Return now if anything has failed */
    if (error) return(error);

    /* Make sure this id is different from all previous ones. */
    cardNum2 = 0;
    for ( card2 = cardList; card2 != card; card2 = card2->MATLnextCard ) {
      cardNum2++;
      if (card2->MATLnumber == card->MATLnumber) {
	sprintf( ebuf,
	    "material cards %d and %d use same id %d",
	    cardNum2, cardNum, card->MATLnumber );
	SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
	error = E_PRIVATE;
      }
    }

    /* Return now if anything has failed */
    if (error) return(error);
  }
  return(OK);
}



/*
 * Name:	MATLsetup
 * Purpose:	setup the physical model parameters
 * Formals:	cardList: list of cards to setup
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical devices
 * Calls:	MATLcheck
 */
int
  MATLsetup( cardList, materialList )
MATLcard *cardList;
MaterialInfo **materialList;
{
  register MATLcard *card;
  MATLmaterial *newMaterial;
  int error;

/* Initialize list of electrodes */
  *materialList = NIL(MATLmaterial);
  
  /* Check the card list */
  if (error = MATLcheck( cardList )) return( error );
  
  for ( card = cardList; card != NIL(MATLcard); card = card->MATLnextCard ) {

    if (*materialList == NIL(MATLmaterial)) {
      RALLOC( newMaterial, MATLmaterial, 1 );
      *materialList = newMaterial;
    }
    else {
      RALLOC( newMaterial->next, MATLmaterial, 1 );
      newMaterial = newMaterial->next;
    }
    newMaterial->next = NIL(MATLmaterial);
    newMaterial->id = card->MATLnumber;
    newMaterial->material = card->MATLmaterial;

    /* Fill in default values */
    MATLdefaults( newMaterial );

    /* Now override with parameters set on the card */
    if ( card->MATLpermittivityGiven ) {
      newMaterial->eps  = card->MATLpermittivity;
      /* Multiply by permittivity of free space if relative epsilon given. */
      if (newMaterial->eps > 0.1) {
	newMaterial->eps *= EPS0;
      }
    }
    if ( card->MATLaffinityGiven ) {
      newMaterial->affin  = card->MATLaffinity;
    }
    if ( card->MATLnc0Given ) {
      newMaterial->nc0  = card->MATLnc0;
    }
    if ( card->MATLnv0Given ) {
      newMaterial->nv0  = card->MATLnv0;
    }
    if ( card->MATLeg0Given ) {
      newMaterial->eg0  = card->MATLeg0;
    }
    if ( card->MATLdEgdTGiven ) {
      newMaterial->dEgDt  = card->MATLdEgdT;
    }
    if ( card->MATLtrefEgGiven ) {
      newMaterial->trefBGN  = card->MATLtrefEg;
    }
    if ( card->MATLdEgdNGiven ) {
      newMaterial->dEgDn[ELEC]  = card->MATLdEgdN;
    }
    if ( card->MATLnrefEgGiven ) {
      newMaterial->nrefBGN[ELEC]  = card->MATLnrefEg;
    }
    if ( card->MATLdEgdPGiven ) {
      newMaterial->dEgDn[HOLE]  = card->MATLdEgdP;
    }
    if ( card->MATLprefEgGiven ) {
      newMaterial->nrefBGN[HOLE]  = card->MATLprefEg;
    }
    if ( card->MATLtaup0Given ) {
      newMaterial->tau0[HOLE]  = card->MATLtaup0;
    }
    if ( card->MATLtaun0Given ) {
      newMaterial->tau0[ELEC]  = card->MATLtaun0;
    }
    if ( card->MATLtaup0Given ) {
      newMaterial->tau0[HOLE]  = card->MATLtaup0;
    }
    if ( card->MATLnrefSRHnGiven ) {
      newMaterial->nrefSRH[ELEC]  = card->MATLnrefSRHn;
    }
    if ( card->MATLnrefSRHpGiven ) {
      newMaterial->nrefSRH[HOLE]  = card->MATLnrefSRHp;
    }
    if ( card->MATLcnAugGiven ) {
      newMaterial->cAug[ELEC]  = card->MATLcnAug;
    }
    if ( card->MATLcpAugGiven ) {
      newMaterial->cAug[HOLE]  = card->MATLcpAug;
    }
    if ( card->MATLaRichNGiven ) {
      newMaterial->aRich[ELEC]  = card->MATLaRichN;
    }
    if ( card->MATLaRichPGiven ) {
      newMaterial->aRich[HOLE]  = card->MATLaRichP;
    }

  }
  return( OK );
}
