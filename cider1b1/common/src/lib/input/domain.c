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
#include "numenum.h"
#include "domndefs.h"
#include "sperror.h"
#include "suffix.h"

#define UM_TO_CM 1.0e-4

#ifdef __STDC__
extern int DOMNnewCard(GENERIC**,GENERIC*);
extern int DOMNparam(int,IFvalue*,GENERIC*);
#else
extern int DOMNnewCard();
extern int DOMNparam();
#endif /* STDC */

IFparm DOMNpTable[] = {
  IP("x.low", 	DOMN_X_LOW,	IF_REAL,	"Location of left edge"),
  IP("x.high",	DOMN_X_HIGH,	IF_REAL,	"Location of right edge"),
  IP("y.low",	DOMN_Y_LOW,	IF_REAL,	"Location of top edge"),
  IP("y.high",	DOMN_Y_HIGH,	IF_REAL,	"Location of bottom edge"),
  IP("ix.low", 	DOMN_IX_LOW,	IF_INTEGER,	"Index of left edge"),
  IP("ix.high",	DOMN_IX_HIGH,	IF_INTEGER,	"Index of right edge"),
  IP("iy.low",	DOMN_IY_LOW,	IF_INTEGER,	"Index of top edge"),
  IP("iy.high",	DOMN_IY_HIGH,	IF_INTEGER,	"Index of bottom edge"),
  IP("number",	DOMN_NUMBER,	IF_INTEGER,	"Domain ID number"),
  IP("material",DOMN_MATERIAL,	IF_INTEGER,	"Material ID number")
};

IFcardInfo DOMNinfo = {
  "domain",
  "Identify material-type for portion of a device",
  NUMELEMS(DOMNpTable),
  DOMNpTable,

  DOMNnewCard,
  DOMNparam,
  NULL
};
IFcardInfo REGNinfo = {
  "region",
  "Identify material-type for portion of a device",
  NUMELEMS(DOMNpTable),
  DOMNpTable,

  DOMNnewCard,
  DOMNparam,
  NULL
};

int
DOMNnewCard( inCard, inModel )
    GENERIC **inCard;
    GENERIC *inModel;
{
    DOMNcard *tmpCard, *newCard;
    GENnumModel *model = (GENnumModel *)inModel;

    newCard = NEW( DOMNcard );
    if (!newCard) {
        *inCard = (GENERIC *)NULL;
        return(E_NOMEM);
    }
    newCard->DOMNnextCard = (DOMNcard *)NULL;
    *inCard = (GENERIC *)newCard;

    tmpCard = model->GENdomains;
    if (!tmpCard) { /* First in list */
        model->GENdomains = newCard;
    } else {
	/* Go to end of list */
        while (tmpCard->DOMNnextCard) tmpCard = tmpCard->DOMNnextCard;
	/* And add new card */
	tmpCard->DOMNnextCard = newCard;
    }
    return(OK);
}

int
DOMNparam( param, value, inCard )
    int param;
    IFvalue *value;
    GENERIC *inCard;
{
    DOMNcard *card = (DOMNcard *)inCard;

    switch (param) {
	case DOMN_X_LOW:
	    card->DOMNxLow = value->rValue * UM_TO_CM;
	    card->DOMNxLowGiven = TRUE;
	    break;
	case DOMN_X_HIGH:
	    card->DOMNxHigh = value->rValue * UM_TO_CM;
	    card->DOMNxHighGiven = TRUE;
	    break;
	case DOMN_Y_LOW:
	    card->DOMNyLow = value->rValue * UM_TO_CM;
	    card->DOMNyLowGiven = TRUE;
	    break;
	case DOMN_Y_HIGH:
	    card->DOMNyHigh = value->rValue * UM_TO_CM;
	    card->DOMNyHighGiven = TRUE;
	    break;
	case DOMN_IX_LOW:
	    card->DOMNixLow = value->iValue;
	    card->DOMNixLowGiven = TRUE;
	    break;
	case DOMN_IX_HIGH:
	    card->DOMNixHigh = value->iValue;
	    card->DOMNixHighGiven = TRUE;
	    break;
	case DOMN_IY_LOW:
	    card->DOMNiyLow = value->iValue;
	    card->DOMNiyLowGiven = TRUE;
	    break;
	case DOMN_IY_HIGH:
	    card->DOMNiyHigh = value->iValue;
	    card->DOMNiyHighGiven = TRUE;
	    break;
	case DOMN_NUMBER:
	    card->DOMNnumber = value->iValue;
	    card->DOMNnumberGiven = TRUE;
	    break;
	case DOMN_MATERIAL:
	    card->DOMNmaterial = value->iValue;
	    card->DOMNmaterialGiven = TRUE;
	    break;
	default:
	    return(E_BADPARM);
	    break;
    }
    return(OK);
}
