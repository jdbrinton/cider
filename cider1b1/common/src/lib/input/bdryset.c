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
#include "bdrydefs.h"
#include "meshext.h"
#include "gendev.h"
#include "sperror.h"
#include "suffix.h"

#ifdef __STDC__
extern int BDRYcheck( BDRYcard *, DOMNdomain * );
extern int BDRYsetup( BDRYcard *, MESHcoord *, MESHcoord *, DOMNdomain * );
#else
extern int BDRYcheck();
extern int BDRYsetup();
#endif /* STDC */


/*
 * Name:	BDRYcheck
 * Purpose:	checks a list of BDRYcards for input errors
 * Formals:	cardList: the list to check
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical device setup routines
 * Calls:	error message handler
 */
int
BDRYcheck( cardList, domnList )
BDRYcard *cardList;
DOMNdomain *domnList;
{
  register BDRYcard *card;
  DOMNdomain *domn;
  int cardNum = 0;
  int error = OK;
  char ebuf[512];		/* error message buffer */

  for ( card = cardList; card != NIL(BDRYcard); card = card->BDRYnextCard ) {
    cardNum++;
    if (card->BDRYxLowGiven && card->BDRYixLowGiven) {
      sprintf( ebuf,
	  "boundary card %d uses both location and index - location ignored",
	  cardNum );
      SPfrontEnd->IFerror( ERR_INFO, ebuf, NIL(IFuid) );
      card->BDRYxLowGiven = FALSE;
    }
    if (card->BDRYxHighGiven && card->BDRYixHighGiven) {
      sprintf( ebuf,
	  "boundary card %d uses both location and index - location ignored",
	  cardNum );
      SPfrontEnd->IFerror( ERR_INFO, ebuf, NIL(IFuid) );
      card->BDRYxHighGiven = FALSE;
    }
    if (card->BDRYyLowGiven && card->BDRYiyLowGiven) {
      sprintf( ebuf,
	  "boundary card %d uses both location and index - location ignored",
	  cardNum );
      SPfrontEnd->IFerror( ERR_INFO, ebuf, NIL(IFuid) );
      card->BDRYyLowGiven = FALSE;
    }
    if (card->BDRYyHighGiven && card->BDRYiyHighGiven) {
      sprintf( ebuf,
	  "boundary card %d uses both location and index - location ignored",
	  cardNum );
      SPfrontEnd->IFerror( ERR_INFO, ebuf, NIL(IFuid) );
      card->BDRYyHighGiven = FALSE;
    }
    if (!card->BDRYdomainGiven) {
      sprintf( ebuf,
	  "boundary card %d is missing a domain index",
	  cardNum );
      SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
      error = E_PRIVATE;
    } else {
      /* Make sure the domain exists */
      for ( domn = domnList; domn != NIL(DOMNdomain); domn = domn->next ) {
	if ( card->BDRYdomain == domn->id ) {
	  break;
	}
      }
      if (domn == NIL(DOMNdomain)) {
	sprintf( ebuf,
	    "boundary card %d specifies a non-existent domain",
	    cardNum );
	SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
	error = E_PRIVATE;
      }
    }

    if (!card->BDRYneighborGiven) {
      card->BDRYneighbor = card->BDRYdomain;
    } else {
      /* Make sure the neighbor exists */
      for ( domn = domnList; domn != NIL(DOMNdomain); domn = domn->next ) {
	if ( card->BDRYneighbor == domn->id ) {
	  break;
	}
      }
      if (domn == NIL(DOMNdomain)) {
	sprintf( ebuf,
	    "interface card %d specifies a non-existent domain",
	    cardNum );
	SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
	error = E_PRIVATE;
      }
    }

    if (!card->BDRYqfGiven) {
      card->BDRYqf = 0.0;
    }
    if (!card->BDRYsnGiven) {
      card->BDRYsn = 0.0;
    }
    if (!card->BDRYspGiven) {
      card->BDRYsp = 0.0;
    }
    if (!card->BDRYlayerGiven) {
      card->BDRYlayer = 0.0;
    }

/* Return now if anything has failed */
    if (error) return(error);
  }
  return(OK);
}



/*
 * Name:	BDRYsetup
 * Purpose:	Checks BDRY cards and then sets the indices
 * Formals:	cardList: list of cards to setup, returns with indices set
 *		xMeshList: list of coordinates in the x mesh
 *		yMeshList: list of coordinates in the y mesh
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical devices
 * Calls:	BDRYcheck
 */
int
BDRYsetup( cardList, xMeshList, yMeshList, domnList )
BDRYcard *cardList;
MESHcoord *xMeshList;
MESHcoord *yMeshList;
DOMNdomain *domnList;
{
  register BDRYcard *card;
  int ixMin, ixMax, iyMin, iyMax;
  int cardNum = 0;
  int error;
  char ebuf[512];		/* error message buffer */

/* Check the card list */
  if (error = BDRYcheck( cardList, domnList )) return( error );

/* Find the limits on the indices */
  MESHiBounds( xMeshList, &ixMin, &ixMax );
  MESHiBounds( yMeshList, &iyMin, &iyMax );

  error = OK;
  for ( card = cardList; card != NIL(BDRYcard); card = card->BDRYnextCard ) {
    cardNum++;

    if (card->BDRYixLowGiven) {
      card->BDRYixLow = MAX(card->BDRYixLow, ixMin);
    }
    else if (card->BDRYxLowGiven) {
      card->BDRYixLow = MESHlocate( xMeshList, card->BDRYxLow );
    }
    else {
      card->BDRYixLow = ixMin;
    }
    if (card->BDRYixHighGiven) {
      card->BDRYixHigh = MIN(card->BDRYixHigh, ixMax);
    }
    else if (card->BDRYxHighGiven) {
      card->BDRYixHigh = MESHlocate( xMeshList, card->BDRYxHigh );
    }
    else {
      card->BDRYixHigh = ixMax;
    }
    if (card->BDRYixLow > card->BDRYixHigh) {
      sprintf( ebuf,
	  "boundary card %d has low x index (%d) > high x index (%d)",
	  cardNum, card->BDRYixHigh, card->BDRYixLow );
      SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
      error = E_PRIVATE;
    }
    if (card->BDRYiyLowGiven) {
      card->BDRYiyLow = MAX(card->BDRYiyLow, iyMin);
    }
    else if (card->BDRYyLowGiven) {
      card->BDRYiyLow = MESHlocate( yMeshList, card->BDRYyLow );
    }
    else {
      card->BDRYiyLow = iyMin;
    }
    if (card->BDRYiyHighGiven) {
      card->BDRYiyHigh = MIN(card->BDRYiyHigh, iyMax);
    }
    else if (card->BDRYyHighGiven) {
      card->BDRYiyHigh = MESHlocate( yMeshList, card->BDRYyHigh );
    }
    else {
      card->BDRYiyHigh = iyMax;
    }
    if (card->BDRYiyLow > card->BDRYiyHigh) {
      sprintf( ebuf,
	  "boundary card %d has low y index (%d) > high y index (%d)",
	  cardNum, card->BDRYiyHigh, card->BDRYiyLow );
      SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
      error = E_PRIVATE;
    }
  }
  return( error );
}
