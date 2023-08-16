/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles, 1991 David A. Gates
**********/

#include "spice.h"
#include <stdio.h>
#include "inpdefs.h"
#include "util.h"
#include "ifsim.h"
#include "cpstd.h"
#include "fteext.h"
#include "numcards.h"
#include "carddefs.h"
#include "numgen.h"
#include "suffix.h"

extern INPmodel *modtab;
extern IFcardInfo *INPcardTab[];
extern int INPnumCards;
#define E_MISSING	-1
#define E_AMBIGUOUS	-2

char *
INPgetMod(ckt,name,model,tab)
    GENERIC * ckt;
    char *name;
    INPmodel **model;
    INPtables *tab;
{
    INPmodel *modtmp;
    IFvalue * val;
    register int j;
    char * line;
    char *parm;
    char *err = NULL;
    char *temp;
    int error;

    for (modtmp = modtab;modtmp != (INPmodel *)NULL;modtmp =
            ((modtmp)->INPnextModel)) {
        if (strcmp((modtmp)->INPmodName,name) == 0) {
            /* found the model in question - now instantiate if necessary */
            /* and return an appropriate pointer to it */
            if(modtmp->INPmodType<0) {
                /* illegal device type, so can't handle */
                *model = (INPmodel *)NULL;
                err = (char *)MALLOC((35+strlen(name)) * sizeof(char));
                (void) sprintf(err,
                        "Unknown device type for model %s \n",name);
                return(err);
            }
            if(! ((modtmp)->INPmodUsed )) {
                /* not already defined, so create & give parameters */
                error = (*(ft_sim->newModel))( ckt,(modtmp)->INPmodType, 
                        &((modtmp)->INPmodfast), (modtmp)->INPmodName);
                if(error) return(INPerror(error));

                /* parameter isolation, identification, binding */
		/* Handle Numerical Models Differently */
		if ( ((modtmp)->INPmodType == INPtypelook("NUMD")) ||
		     ((modtmp)->INPmodType == INPtypelook("NBJT")) ||
		     ((modtmp)->INPmodType == INPtypelook("NUMD2")) ||
		     ((modtmp)->INPmodType == INPtypelook("NBJT2")) ||
		     ((modtmp)->INPmodType == INPtypelook("NUMOS")) ) {
		    error = INPparseNumMod( ckt, modtmp, tab, &err );
		    if (error) return(INPerror(error));
		} else {     
		/* It's an analytical model */
                line = ((modtmp)->INPmodLine)->line;
                INPgetTok(&line,&parm,1);     /* throw away '.model' */
                INPgetTok(&line,&parm,1);     /* throw away 'modname' */
                while(*line != 0) {
                    INPgetTok(&line,&parm,1);
		    if (!*parm)
			continue;
                    for(j=0;j<(*(*(ft_sim->devices)[(modtmp)->INPmodType]).
                            numModelParms); j++) {
                        if (strcmp(parm,((*(ft_sim->devices) [ (modtmp)->
                                INPmodType ]).modelParms[j].keyword)) == 0) {
                            val = INPgetValue(ckt,&line,
                                    ((*(ft_sim->devices)[(modtmp)->
                                    INPmodType ]).modelParms[j].
                                    dataType),tab);
                            error = (*(ft_sim->setModelParm))(ckt, 
                                    ((modtmp)->INPmodfast),
                                    (*(ft_sim->devices)[(modtmp)->INPmodType ]).
                                    modelParms[j].id,val,(IFvalue*)NULL);
                            if(error) return(INPerror(error));
                            break;
                        } 
                    }
                    if (strcmp(parm,"level")==0) {
                        /* just grab the level number and throw away */
                        /* since we already have that info from pass1 */
                        val = INPgetValue(ckt,&line,IF_REAL,tab);
                    } else if(j >= 
                            (*(*(ft_sim->devices)[(modtmp)->INPmodType]).
                                    numModelParms)) {
                        temp = (char *)MALLOC((40+strlen(parm)) * sizeof(char));
                        (void)sprintf(temp,
                            "unrecognized parameter (%s) - ignored", parm);
                        err = INPerrCat(err,temp);
                    }
                    FREE(parm);
                }
		} /* analytical vs. numerical model parsing */
                (modtmp)->INPmodUsed=1;
                (modtmp)->INPmodLine->error = err;
            }
            *model = modtmp;
            return((char *)NULL);
        }
    }
    /* didn't find model - ERROR  - return model */
    *model = (INPmodel *)NULL;
    err = (char *)MALLOC((60+strlen(name)) * sizeof(char));
    (void) sprintf(err,
            " unable to find definition of model %s - default assumed \n",name);
    return(err);
}

/*
 * Parse a numerical model by running through the list of original
 * input cards which make up the model
 * Given:
 * 1. First card looks like: .model modname modtype <level=val>
 * 2. Other cards look like: +<whitespace>? where ? tells us what
 * to do with the next card:
 *    '#$*' = comment card
 *    '+'   = continue previous card
 *    other = new card
 */
int
INPparseNumMod( ckt, model, tab, errMessage )
    GENERIC * ckt;
    INPmodel *model;
    INPtables *tab;
    char **errMessage;
{
    card *txtCard;	/* Text description of a card */
    GENcard *tmpCard;	/* Processed description of a card */
    IFcardInfo *info;	/* Info about the type of card located */
    char *line;
    char *cardName = NULL;	/* name of a card */
    char *parm;		/* name of a parameter */
    int cardType;	/* type/index for the current card */
    int cardNum = 0;	/* number of this card in the overall line */
    int lastType = E_MISSING;	/* type of previous card */
    char *err = NULL, *tmp;    /* Strings for error messages */
    IFvalue *value;
    int error, idx, invert;

    /* Chase down to the top of the list of actual cards */
    txtCard = model->INPmodLine->actualLine;

    /* Skip the first card if it exists since there's nothing interesting */
    /* txtCard will be empty if the numerical model is empty */
    if (txtCard) txtCard = txtCard->nextcard;

    /* Now parse each remaining card */
    while (txtCard) {
	line = txtCard->line;
	cardType = E_MISSING;
	cardNum++;

	/* Skip the initial '+' and any whitespace. */
	line++;
	while (*line == ' ' || *line == '\t')
	    line++;

	switch (*line) {
	case '*':
	case '$':
	case '#':
	case '\0':
	case '\n':
	    /* comment or empty cards */
	    lastType = E_MISSING;
	    break;
        case '+':
	    /* continuation card */
	    if (lastType >= 0) {
	        cardType = lastType;
                while (*line == '+') line++;	/* Skip leading '+'s */
	    } else {
	        tmp = (char *)MALLOC((55)*sizeof(char));
		(void) sprintf(tmp,
		    "Error on card %d : illegal continuation \'+\' - ignored",
		    cardNum);
		err = INPerrCat(err,tmp);
		lastType = E_MISSING;
	        break;
	    }
	    /* FALL THRU when continuing a card */
        default:
	    if (cardType == E_MISSING) {
	        /* new command card */
		if (cardName) FREE(cardName);	/* get rid of old card name */
	        INPgetTok(&line,&cardName,1);	/* get new card name */
		if (*cardName) { 		/* Found a name? */
		    cardType = INPfindCard(cardName,INPcardTab,INPnumCards);
		    if (cardType >= 0) {
		        /* Add card structure to model */
			info = INPcardTab[cardType];
			error = (*(info->newCard))( (GENERIC *)&tmpCard,
			        model->INPmodfast );
			if (error) return(error);
		    /* Handle parameter-less cards */
		    } else if (cinprefix( cardName, "title", 3 ) ) {
		        /* Do nothing */
		    } else if (cinprefix( cardName, "comment", 3 ) ) {
		        /* Do nothing */
		    } else if (cinprefix( cardName, "end", 3 ) ) {
			/* Terminate parsing */
			txtCard = ((card *) 0);
			cardType = E_MISSING;
		    } else {
		        /* Error */
		        tmp =(char *)MALLOC((55+strlen(cardName))*sizeof(char));
			(void) sprintf(tmp,
			    "Error on card %d : unrecognized name (%s) - ignored",
			    cardNum, cardName );
			err = INPerrCat(err,tmp);
		    }
		}
	    }
	    if (cardType >= 0) { /* parse the rest of this line */
		while (*line) {
		    /* Strip leading carat from booleans */
		    if (*line == '^') {
		      invert = true;
		      *line++;	/* Skip the '^' */
		    } else {
		      invert = false;
		    }
		    INPgetTok(&line,&parm,1);
		    if (!*parm)
		      break;
		    idx = INPfindParm(parm, info->cardParms, info->numParms);
		    if (idx == E_MISSING) {
			/* parm not found */
                        tmp = (char *)MALLOC((60+strlen(parm)) * sizeof(char));
                        (void)sprintf(tmp,
                            "Error on card %d : unrecognized parameter (%s) - ignored",
			    cardNum, parm);
                        err = INPerrCat(err, tmp);
		    } else if (idx == E_AMBIGUOUS) {
			/* parm ambiguous */
                        tmp = (char *)MALLOC((58+strlen(parm)) * sizeof(char));
                        (void)sprintf(tmp,
                            "Error on card %d : ambiguous parameter (%s) - ignored",
			    cardNum, parm);
                        err = INPerrCat(err, tmp);
		    } else {
		        value = INPgetValue( ckt, &line,
			    ((info->cardParms)[idx]).dataType, tab );
			if (invert) { /* invert if it's a boolean entry */
			  if (((((info->cardParms)[idx]).dataType)&IF_VARTYPES)
			      == IF_FLAG) {
			    value->iValue = 0;
			  } else {
                            tmp =(char *)MALLOC((63+strlen(parm))*sizeof(char));
                            (void)sprintf(tmp,
                              "Error on card %d : non-boolean parameter (%s) - \'^\' ignored",
			      cardNum, parm);
                            err = INPerrCat(err, tmp);
			  }
			}
                        error = (*(info->setCardParm))(
			    ((info->cardParms)[idx]).id, value, tmpCard );
			if (error) return(error);
		    }
		    FREE(parm);
		}
	    }
	    lastType = cardType;
	    break;
	}
	if (txtCard) txtCard = txtCard->nextcard;
    }
    *errMessage = err;
    return( 0 );
}

/*
 * Locate the best match to a card name in an IFcardInfo table
 */
int
INPfindCard( name, table, numCards )
    char *name;
    IFcardInfo *table[];
    int numCards;
{
    register int test;
    int match, bestMatch;
    int best;
    int length;

    length = strlen(name);

    /* compare all the names in the card table to this name */
    best = E_MISSING;
    bestMatch = 0;
    for ( test = 0; test < numCards; test++ ) {
        match = cimatch( name, table[test]->name );
        if ((match == bestMatch ) && (match > 0)){
	    best = E_AMBIGUOUS;
        } else if ((match > bestMatch) && (match == length)) {
	    best = test;
	    bestMatch = match;
        }
    }
    return(best);
}

/*
 * Locate the best match to a parameter name in an IFparm table
 */
int
INPfindParm( name, table, numParms )
    char *name;
    IFparm *table;
    int numParms;
{
    register int test, best;
    int match, bestMatch;
    int id, bestId;
    int length;

    length = strlen(name);

    /* compare all the names in the parameter table to this name */
    best = E_MISSING;
    bestId = -1;
    bestMatch = 0;
    for ( test = 0; test < numParms; test++ ) {
        match = cimatch( name, table[test].keyword );
	if ( (match == length) && (match == strlen(table[test].keyword)) ) {
	    /* exact match */
	    best = test;
	    /* all done */
	    break;
	}
	id = table[test].id;
        if ((match == bestMatch) && (match > 0) && (id != bestId)) {
	    best = E_AMBIGUOUS;
        } else if ((match > bestMatch) && (match == length)) {
	    bestMatch = match;
	    bestId = id;
	    best = test;
        }
    }
    return(best);
}
