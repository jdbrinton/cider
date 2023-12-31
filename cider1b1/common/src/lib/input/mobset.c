/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
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
#include "mobdefs.h"
#include "material.h"
#include "sperror.h"
#include "suffix.h"

#ifdef __STDC__
extern int MOBcheck( MOBcard *, MaterialInfo * );
extern int MOBsetup( MOBcard *, MaterialInfo * );
#else
extern int MOBcheck();
extern int MOBsetup();
#endif /* STDC */



/*
 * Name:	MOBcheck
 * Purpose:	checks a list of MOBcards for input errors
 * Formals:	cardList: the list to check
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical device setup routines
 * Calls:	error message handler
 */
int
MOBcheck( cardList, matlList )
MOBcard *cardList;
MaterialInfo *matlList;
{
  register MOBcard *card;
  MATLmaterial *matl;
  int cardNum = 0;
  int error = OK;
  char ebuf[512];		/* error message buffer */

  for ( card = cardList; card != NIL(MOBcard); card = card->MOBnextCard ) {
    cardNum++;
    if (!card->MOBmaterialGiven) {
      sprintf( ebuf,
	  "mobility card %d is missing a material index",
	  cardNum );
      SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
      error = E_PRIVATE;
    } else {
      /* Make sure the material exists */
      for ( matl = matlList; matl != NIL(MATLmaterial); matl = matl->next ) {
	if ( card->MOBmaterial == matl->id ) {
	  break;
	}
      }
      if (matl == NIL(MATLmaterial)) {
	sprintf( ebuf,
	    "mobility card %d specifies a non-existent material",
	    cardNum );
	SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
	error = E_PRIVATE;
      }
    }
    if (!card->MOBcarrierGiven) {
      card->MOBcarrier = ELEC;
    }
    if (!card->MOBcarrTypeGiven) {
      card->MOBcarrType = MAJOR;
    }
    if (!card->MOBinitGiven) {
      card->MOBinit = FALSE;
    }

/* Return now if anything has failed */
    if (error) return(error);

  }
  return(OK);
}



/*
 * Name:	MOBsetup
 * Purpose:	setup the mobility model parameters
 * Formals:	cardList: list of cards to setup
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical devices
 * Calls:	MOBcheck
 */
int
MOBsetup( cardList, materialList )
MOBcard *cardList;
MaterialInfo *materialList;
{
  register MOBcard *card;
  MATLmaterial *matl;
  int error;

/* Check the card list */
  if (error = MOBcheck( cardList, materialList )) return( error );

  for ( card = cardList; card != NIL(MOBcard); card = card->MOBnextCard ) {

    /* Find the right material */
    for ( matl = materialList; matl != NIL(MATLmaterial); matl = matl->next ) {
      if ( card->MOBmaterial == matl->id ) {
	break;
      }
    }

    /* Default models depend on previous value */
    if (!card->MOBconcModelGiven) {
      card->MOBconcModel = matl->concModel;
    }
    if (!card->MOBfieldModelGiven) {
      card->MOBfieldModel = matl->fieldModel;
    }

    /* Load in default values if desired */
    if ( card->MOBinitGiven ) {
      MOBdefaults( matl, card->MOBcarrier, card->MOBcarrType,
	    card->MOBconcModel, card->MOBfieldModel );
    }

    /* Override defaults */
    if ( card->MOBconcModelGiven ) {
      matl->concModel = card->MOBconcModel;
    }
    if ( card->MOBfieldModelGiven ) {
      matl->fieldModel = card->MOBfieldModel;
    }
    if ( card->MOBmuMaxGiven ) {
	matl->muMax[card->MOBcarrier][card->MOBcarrType] = card->MOBmuMax;
    }
    if ( card->MOBmuMinGiven ) {
	matl->muMin[card->MOBcarrier][card->MOBcarrType] = card->MOBmuMin;
    }
    if ( card->MOBntRefGiven ) {
	matl->ntRef[card->MOBcarrier][card->MOBcarrType] = card->MOBntRef;
    }
    if ( card->MOBntExpGiven ) {
	matl->ntExp[card->MOBcarrier][card->MOBcarrType] = card->MOBntExp;
    }
    if ( card->MOBvSatGiven ) {
	matl->vSat[card->MOBcarrier] = card->MOBvSat;
    }
    if ( card->MOBvWarmGiven ) {
	matl->vWarm[card->MOBcarrier] = card->MOBvWarm;
    }
    if ( card->MOBmusGiven ) {
	matl->mus[card->MOBcarrier] = card->MOBmus;
    }
    if ( card->MOBecAGiven ) {
	matl->thetaA[card->MOBcarrier] = 1.0 / MAX( card->MOBecA, 1e-20 );
    }
    if ( card->MOBecBGiven ) {
	matl->thetaB[card->MOBcarrier] = 1.0 / MAX( ABS(card->MOBecB), 1e-20 );
	matl->thetaB[card->MOBcarrier] *= matl->thetaB[card->MOBcarrier];
	matl->thetaB[card->MOBcarrier] *= SGN( card->MOBecB );
    }
  }
  return( OK );
}
