/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author: 1992 David A. Gates, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include <stdio.h>
#include <math.h>
#include "const.h"
#include "util.h"
#include "misc.h"
#include "cktdefs.h"
#include "nummacs.h"
#include "numenum.h"
#include "outpdefs.h"
#include "sperror.h"
#include "suffix.h"

#ifdef __STDC__
extern int OUTPcheck( OUTPcard * );
extern int OUTPsetup( OUTPcard * );
#else
extern int OUTPcheck();
extern int OUTPsetup();
#endif /* STDC */



/*
 * Name:	OUTPcheck
 * Purpose:	checks a list of OUTPcards for input errors, sets defaults
 * Formals:	cardList: the list to check
 * Returns:	OK/E_PRIVATE
 * Users:	numerical device setup routines, output routines
 * Calls:	error message handler
 */
int
  OUTPcheck( cardList )
OUTPcard *cardList;
{
  register OUTPcard *card, *card2;
  int cardNum = 0, cardNum2;
  int error = OK;
  char ebuf[512];		/* error message buffer */
  
  for ( card = cardList; card != NIL(OUTPcard); card = card->OUTPnextCard ) {
    cardNum++;

    card->OUTPnumVars = -1;

    if ( !card->OUTPdcDebugGiven ) {
      card->OUTPdcDebug = FALSE;
    }
    if ( !card->OUTPtranDebugGiven ) {
      card->OUTPtranDebug = FALSE;
    }
    if ( !card->OUTPacDebugGiven ) {
      card->OUTPacDebug = FALSE;
    }
    if ( !card->OUTPgeomGiven ) {
      card->OUTPgeom = FALSE;
    }
    if ( !card->OUTPmeshGiven ) {
      card->OUTPmesh = FALSE;
    }
    if ( !card->OUTPmaterialGiven ) {
      card->OUTPmaterial = FALSE;
    }
    if ( !card->OUTPglobalsGiven ) {
      card->OUTPglobals = FALSE;
    }
    if ( !card->OUTPstatsGiven ) {
      card->OUTPstats = TRUE;
    }
    if ( !card->OUTProotFileGiven ) {
      card->OUTProotFile = copy("");
    }
    if ( !card->OUTPfileTypeGiven ) {
      card->OUTPfileType = RAWFILE;
    }
    if ( !card->OUTPdopingGiven ) {
      card->OUTPdoping = TRUE;
    }
    if ( !card->OUTPpsiGiven ) {
      card->OUTPpsi = TRUE;
    }
    if ( !card->OUTPequPsiGiven ) {
      card->OUTPequPsi = FALSE;
    }
    if ( !card->OUTPvacPsiGiven ) {
      card->OUTPvacPsi = FALSE;
    }
    if ( !card->OUTPnConcGiven ) {
      card->OUTPnConc = TRUE;
    }
    if ( !card->OUTPpConcGiven ) {
      card->OUTPpConc = TRUE;
    }
    if ( !card->OUTPphinGiven ) {
      card->OUTPphin = FALSE;
    }
    if ( !card->OUTPphipGiven ) {
      card->OUTPphip = FALSE;
    }
    if ( !card->OUTPphicGiven ) {
      card->OUTPphic = FALSE;
    }
    if ( !card->OUTPphivGiven ) {
      card->OUTPphiv = FALSE;
    }
    if ( !card->OUTPeFieldGiven ) {
      card->OUTPeField = TRUE;
    }
    if ( !card->OUTPjcGiven ) {
      card->OUTPjc = FALSE;
    }
    if ( !card->OUTPjdGiven ) {
      card->OUTPjd = TRUE;
    }
    if ( !card->OUTPjnGiven ) {
      card->OUTPjn = TRUE;
    }
    if ( !card->OUTPjpGiven ) {
      card->OUTPjp = TRUE;
    }
    if ( !card->OUTPjtGiven ) {
      card->OUTPjt = FALSE;
    }
    if ( !card->OUTPuNetGiven ) {
      card->OUTPuNet = FALSE;
    }
    if ( !card->OUTPmunGiven ) {
      card->OUTPmun = FALSE;
    }
    if ( !card->OUTPmupGiven ) {
      card->OUTPmup = FALSE;
    }
  }
  return(OK);
}



/*
 * Name:	OUTPsetup
 * Purpose:	setup the output card
 * Formals:	cardList: list of cards to setup
 * Returns:	OK/E_PRIVATE
 * Users:	 numerical devices
 * Calls:	OUTPcheck
 */
int
  OUTPsetup( cardList )
OUTPcard *cardList;
{
  int error;

  /* Check the card list */
  if (error = OUTPcheck( cardList )) return( error );

  return( OK );
}
