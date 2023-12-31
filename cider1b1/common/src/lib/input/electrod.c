/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "util.h"
#include "numcards.h"
#include "numgen.h"
#include "elctdefs.h"
#include "sperror.h"
#include "suffix.h"

#define UM_TO_CM 1.0e-4

#ifdef __STDC__
extern int ELCTnewCard(GENERIC**,GENERIC*);
extern int ELCTparam(int,IFvalue*,GENERIC*);
#else
extern int ELCTnewCard();
extern int ELCTparam();
#endif /* STDC */

IFparm ELCTpTable[] = {
  IP("x.low", 	ELCT_X_LOW,	IF_REAL,	"Location of left edge"),
  IP("x.high",	ELCT_X_HIGH,	IF_REAL,	"Location of right edge"),
  IP("y.low",	ELCT_Y_LOW,	IF_REAL,	"Location of top edge"),
  IP("y.high",	ELCT_Y_HIGH,	IF_REAL,	"Location of bottom edge"),
  IP("ix.low", 	ELCT_IX_LOW,	IF_INTEGER,	"Index of left edge"),
  IP("ix.high",	ELCT_IX_HIGH,	IF_INTEGER,	"Index of right edge"),
  IP("iy.low",	ELCT_IY_LOW,	IF_INTEGER,	"Index of top edge"),
  IP("iy.high",	ELCT_IY_HIGH,	IF_INTEGER,	"Index of bottom edge"),
  IP("number",	ELCT_NUMBER,	IF_INTEGER,	"Electrode ID number")
};

IFcardInfo ELCTinfo = {
  "electrode",
  "Location of a contact to the device",
  NUMELEMS(ELCTpTable),
  ELCTpTable,

  ELCTnewCard,
  ELCTparam,
  NULL
};

int
ELCTnewCard( inCard, inModel )
    GENERIC **inCard;
    GENERIC *inModel;
{
    ELCTcard *tmpCard, *newCard;
    GENnumModel *model = (GENnumModel *)inModel;

    newCard = NEW( ELCTcard );
    if (!newCard) {
        *inCard = (GENERIC *)NULL;
        return(E_NOMEM);
    }
    newCard->ELCTnextCard = (ELCTcard *)NULL;
    *inCard = (GENERIC *)newCard;

    tmpCard = model->GENelectrodes;
    if (!tmpCard) { /* First in list */
        model->GENelectrodes = newCard;
    } else {
	/* Go to end of list */
        while (tmpCard->ELCTnextCard) tmpCard = tmpCard->ELCTnextCard;
	/* And add new card */
	tmpCard->ELCTnextCard = newCard;
    }
    return(OK);
}


int
ELCTparam( param, value, inCard )
    int param;
    IFvalue *value;
    GENERIC *inCard;
{
    ELCTcard *card = (ELCTcard *)inCard;

    switch (param) {
	case ELCT_X_LOW:
	    card->ELCTxLow = value->rValue * UM_TO_CM;
	    card->ELCTxLowGiven = TRUE;
	    break;
	case ELCT_X_HIGH:
	    card->ELCTxHigh = value->rValue * UM_TO_CM;
	    card->ELCTxHighGiven = TRUE;
	    break;
	case ELCT_Y_LOW:
	    card->ELCTyLow = value->rValue * UM_TO_CM;
	    card->ELCTyLowGiven = TRUE;
	    break;
	case ELCT_Y_HIGH:
	    card->ELCTyHigh = value->rValue * UM_TO_CM;
	    card->ELCTyHighGiven = TRUE;
	    break;
	case ELCT_IX_LOW:
	    card->ELCTixLow = value->iValue;
	    card->ELCTixLowGiven = TRUE;
	    break;
	case ELCT_IX_HIGH:
	    card->ELCTixHigh = value->iValue;
	    card->ELCTixHighGiven = TRUE;
	    break;
	case ELCT_IY_LOW:
	    card->ELCTiyLow = value->iValue;
	    card->ELCTiyLowGiven = TRUE;
	    break;
	case ELCT_IY_HIGH:
	    card->ELCTiyHigh = value->iValue;
	    card->ELCTiyHighGiven = TRUE;
	    break;
	case ELCT_NUMBER:
	    card->ELCTnumber = value->iValue;
	    card->ELCTnumberGiven = TRUE;
	    break;
	default:
	    return(E_BADPARM);
	    break;
    }
    return(OK);
}
