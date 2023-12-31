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
#include "meshdefs.h"
#include "sperror.h"
#include "suffix.h"

#ifdef __STDC__
extern int XMSHnewCard(GENERIC**,GENERIC*);
extern int YMSHnewCard(GENERIC**,GENERIC*);
extern int MESHparam(int,IFvalue*,GENERIC*);
#else
extern int XMSHnewCard();
extern int YMSHnewCard();
extern int MESHparam();
#endif /* STDC */

IFparm MESHpTable[] = {
  IP("location",MESH_LOCATION,	IF_REAL,	"Meshline location"),
  IP("width",	MESH_WIDTH,	IF_REAL,	"Distance to next line"),
  IP("number",	MESH_NUMBER,	IF_INTEGER,	"Meshline number"),
  IP("node",	MESH_NUMBER,	IF_INTEGER,	"Meshline number"),
  IP("ratio",	MESH_RATIO,	IF_REAL,	"Suggested spacing ratio"),
  IP("h.start",	MESH_H_START,	IF_REAL,	"Spacing at start of interval"),
  IP("h1",	MESH_H_START,	IF_REAL,	"Spacing at start of interVal"),
  IP("h.end",	MESH_H_END,	IF_REAL,	"Spacing at end of interval"),
  IP("h2",	MESH_H_END,	IF_REAL,	"Spacing at end of interval"),
  IP("h.max",	MESH_H_MAX,	IF_REAL,	"Max spacing during interval"),
  IP("h3",	MESH_H_MAX,	IF_REAL,	"Max spacing during interval")
};

IFcardInfo XMSHinfo = {
  "x.mesh",
  "Location of mesh lines",
  NUMELEMS(MESHpTable),
  MESHpTable,

  XMSHnewCard,
  MESHparam,
  NULL
};

IFcardInfo YMSHinfo = {
  "y.mesh",
  "Location of mesh lines",
  NUMELEMS(MESHpTable),
  MESHpTable,

  YMSHnewCard,
  MESHparam,
  NULL
};

int
XMSHnewCard( inCard, inModel )
    GENERIC **inCard;
    GENERIC *inModel;
{
    MESHcard *tmpCard, *newCard;
    GENnumModel *model = (GENnumModel *)inModel;

    newCard = NEW( MESHcard );
    if (!newCard) {
        *inCard = (GENERIC *)NULL;
        return(E_NOMEM);
    }
    newCard->MESHnextCard = (MESHcard *)NULL;
    *inCard = (GENERIC *)newCard;

    tmpCard = model->GENxMeshes;
    if (!tmpCard) { /* First in list */
        model->GENxMeshes = newCard;
    } else {
	/* Go to end of list */
        while (tmpCard->MESHnextCard) tmpCard = tmpCard->MESHnextCard;
	/* And add new card */
	tmpCard->MESHnextCard = newCard;
    }
    return(OK);
}

int
YMSHnewCard( inCard, inModel )
    GENERIC **inCard;
    GENERIC *inModel;
{
    MESHcard *tmpCard, *newCard;
    GENnumModel *model = (GENnumModel *)inModel;

    newCard = NEW( MESHcard );
    if (!newCard) {
        *inCard = (GENERIC *)NULL;
        return(E_NOMEM);
    }
    newCard->MESHnextCard = (MESHcard *)NULL;
    *inCard = (GENERIC *)newCard;

    tmpCard = model->GENyMeshes;
    if (!tmpCard) { /* First in list */
        model->GENyMeshes = newCard;
    } else {
	/* Go to end of list */
        while (tmpCard->MESHnextCard) tmpCard = tmpCard->MESHnextCard;
	/* And add new card */
	tmpCard->MESHnextCard = newCard;
    }
    return(OK);
}

int
MESHparam( param, value, inCard )
    int param;
    IFvalue *value;
    GENERIC *inCard;
{
    MESHcard *card = (MESHcard *)inCard;

    switch (param) {
	case MESH_LOCATION:
	    card->MESHlocation = value->rValue;
	    card->MESHlocationGiven = TRUE;
	    break;
	case MESH_WIDTH:
	    card->MESHwidth = value->rValue;
	    card->MESHwidthGiven = TRUE;
	    break;
	case MESH_H_START:
	    card->MESHhStart = value->rValue;
	    card->MESHhStartGiven = TRUE;
	    break;
	case MESH_H_END:
	    card->MESHhEnd = value->rValue;
	    card->MESHhEndGiven = TRUE;
	    break;
	case MESH_H_MAX:
	    card->MESHhMax = value->rValue;
	    card->MESHhMaxGiven = TRUE;
	    break;
	case MESH_RATIO:
	    card->MESHratio = value->rValue;
	    card->MESHratioGiven = TRUE;
	    break;
	case MESH_NUMBER:
	    card->MESHnumber = value->iValue;
	    card->MESHnumberGiven = TRUE;
	    break;
	default:
	    return(E_BADPARM);
	    break;
    }
    return(OK);
}
