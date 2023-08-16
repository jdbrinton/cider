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
#include "contdefs.h"
#include "sperror.h"
#include "suffix.h"

#ifdef __STDC__
extern int CONTnewCard(GENERIC**,GENERIC*);
extern int CONTparam(int,IFvalue*,GENERIC*);
#else
extern int CONTnewCard();
extern int CONTparam();
#endif /* STDC */

IFparm CONTpTable[] = {
  IP("neutral",		CONT_NEUTRAL,	IF_FLAG,	"Charge neutral"),
  IP("aluminum",	CONT_ALUMINUM,	IF_FLAG,	"Aluminum contact"),
  IP("p.polysilicon",	CONT_P_POLY,	IF_FLAG,	"P poly contact"),
  IP("n.polysilicon",	CONT_N_POLY,	IF_FLAG,	"N poly contact"),
  IP("workfunction",	CONT_WORKFUN,	IF_REAL,	"Metal work function"),
  IP("number",		CONT_NUMBER,	IF_INTEGER,	"Electrode ID number")
};

IFcardInfo CONTinfo = {
  "contact",
  "Properties of a contact to the device",
  NUMELEMS(CONTpTable),
  CONTpTable,

  CONTnewCard,
  CONTparam,
  NULL
};

int
CONTnewCard( inCard, inModel )
    GENERIC **inCard;
    GENERIC *inModel;
{
    CONTcard *tmpCard, *newCard;
    GENnumModel *model = (GENnumModel *)inModel;

    newCard = NEW( CONTcard );
    if (!newCard) {
        *inCard = (GENERIC *)NULL;
        return(E_NOMEM);
    }
    newCard->CONTnextCard = (CONTcard *)NULL;
    *inCard = (GENERIC *)newCard;

    tmpCard = model->GENcontacts;
    if (!tmpCard) { /* First in list */
        model->GENcontacts = newCard;
    } else {
	/* Go to end of list */
        while (tmpCard->CONTnextCard) tmpCard = tmpCard->CONTnextCard;
	/* And add new card */
	tmpCard->CONTnextCard = newCard;
    }
    return(OK);
}

int
CONTparam( param, value, inCard )
    int param;
    IFvalue *value;
    GENERIC *inCard;
{
    CONTcard *card = (CONTcard *)inCard;

    switch (param) {
	case CONT_NEUTRAL:
	    if ( value->iValue ) {
	        card->CONTtype = CONT_NEUTRAL;
	        card->CONTtypeGiven = TRUE;
	    } else {
		if ( card->CONTtype == CONT_NEUTRAL ) {
		    card->CONTtype == -1;
		    card->CONTtypeGiven = FALSE;
		}
	    }
	    break;
	case CONT_ALUMINUM:
	    if ( value->iValue ) {
	        card->CONTtype = CONT_ALUMINUM;
	        card->CONTtypeGiven = TRUE;
	    } else {
		if ( card->CONTtype == CONT_ALUMINUM ) {
		    card->CONTtype == -1;
		    card->CONTtypeGiven = FALSE;
		}
	    }
	    break;
	case CONT_P_POLY:
	    if ( value->iValue ) {
	        card->CONTtype = CONT_P_POLY;
	        card->CONTtypeGiven = TRUE;
	    } else {
		if ( card->CONTtype == CONT_P_POLY ) {
		    card->CONTtype == -1;
		    card->CONTtypeGiven = FALSE;
		}
	    }
	    break;
	case CONT_N_POLY:
	    if ( value->iValue ) {
	        card->CONTtype = CONT_N_POLY;
	        card->CONTtypeGiven = TRUE;
	    } else {
		if ( card->CONTtype == CONT_N_POLY ) {
		    card->CONTtype == -1;
		    card->CONTtypeGiven = FALSE;
		}
	    }
	    break;
	case CONT_WORKFUN:
	    card->CONTtype = CONT_WORKFUN;
	    card->CONTtypeGiven = TRUE;
	    card->CONTworkfun = value->rValue;
	    card->CONTworkfunGiven = TRUE;
	    break;
	case CONT_NUMBER:
	    card->CONTnumber = value->iValue;
	    card->CONTnumberGiven = TRUE;
	    break;
	default:
	    return(E_BADPARM);
	    break;
    }
    return(OK);
}
