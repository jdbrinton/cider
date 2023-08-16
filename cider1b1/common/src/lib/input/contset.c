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
#include "contdefs.h"
#include "meshext.h"
#include "gendev.h"
#include "sperror.h"
#include "suffix.h"

#ifdef __STDC__
extern int CONTcheck( CONTcard * );
extern int CONTsetup( CONTcard *, ELCTelectrode * );
#else
extern int CONTcheck();
extern int CONTsetup();
#endif /* STDC */


/*
 * Name:	CONTcheck
 * Purpose:	checks a list of CONTcards for input errors
 * Formals:	cardList: the list to check
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical device setup routines
 * Calls:	error message handler
 */
int
CONTcheck( cardList )
CONTcard *cardList;
{
  register CONTcard *card;
  int cardNum = 0;
  int error = OK;
  char ebuf[512];		/* error message buffer */

  for ( card = cardList; card != NIL(CONTcard); card = card->CONTnextCard ) {
    cardNum++;
    if (!card->CONTnumberGiven) {
      sprintf( ebuf,
	  "contact card %d is missing an electrode index",
	  cardNum );
      SPfrontEnd->IFerror( ERR_WARNING, ebuf, NIL(IFuid) );
      error = E_PRIVATE;
    }

/* Return now if anything has failed */
    if (error) return(error);
  }
  return(OK);
}



/*
 * Name:	CONTsetup
 * Purpose:	copies information from list of CONTcard's to ELCTelectrode's
 * Formals:	cardList: list of cards to setup
 *		electrodeList: previously built list of ELCTelectrode's
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical devices
 * Calls:	CONTcheck
 */
int
CONTsetup( cardList, electrodeList )
CONTcard *cardList;
ELCTelectrode *electrodeList;
{
  register CONTcard *card;
  ELCTelectrode *electrode;
  int error;

/* Check the card list */
  if (error = CONTcheck( cardList )) return( error );

  for ( card = cardList; card != NIL(CONTcard); card = card->CONTnextCard ) {

    /* Copy workfunction to all matching electrodes */
    for ( electrode = electrodeList; electrode != NIL(ELCTelectrode);
	electrode = electrode->next ) {
      if ( card->CONTnumber == electrode->id ) {
	if ( card->CONTworkfunGiven ) {
	  electrode->workf = card->CONTworkfun;
	} else {
	  electrode->workf = 4.10 /* electron volts */;
	}
      }
    }
  }
  return( OK );
}
