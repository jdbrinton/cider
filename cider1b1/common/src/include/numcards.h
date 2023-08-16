/**********
Copyright 1991 Regents of the University of California. All rights reserved.
Authors: 1991 David Gates
**********/
/*
 * Structures for parsing numerical-device input cards
 */
#ifndef NUMCARDS_H
#define NUMCARDS_H

#include "ifsim.h"

/*
 * Generic header for a linked list of cards 
 */

typedef struct sGENcard {
    struct sGENcard *GENnextCard; /* pointer to next card of this type */
} GENcard;

/*
 * Structure: IFcardInfo
 *  
 * This structure is a generic description of an input card to
 * the program.  It can be used in situations where input parameters
 * need to be grouped together under a single heading.
 */


typedef struct sIFcardInfo {
    char *name;			/* name of the card */
    char *description;		/* description of its purpose */

    int numParms;		/* number of parameter descriptors */
    IFparm *cardParms;		/* array  of parameter descriptors */

#ifdef __STDC__
    int (*newCard)(GENERIC**,GENERIC*);
	/* routine to add a new card to a numerical device model */
    int (*setCardParm)(int,IFvalue*,GENERIC*);
	/* routine to input a parameter to a card instance */
    int (*askCardQuest)(int,IFvalue*,GENERIC*);
	/* routine to find out about a card's details */
#else
    int (*newCard)();
    int (*setCardParm)();
    int (*askCardQuest)();
#endif /* STDC */
} IFcardInfo;

/* Macros for IFparm descriptors */
#ifndef IOP
/*
 * these are for the IBM PC which gets upset by the long description
 * strings used in IFparm definitions because they add up to more than
 * 64k of static data
 */
#ifndef HAS_MINDATA
#define IP(a,b,c,d) { a , b , c|IF_SET , d }
#define OP(a,b,c,d) { a , b , c|IF_ASK , d }
#define IOP(a,b,c,d) { a , b , c|IF_SET|IF_ASK , d }
#define P(a,b,c,d) { a , b , c , d }
#else /* HAS_MINDATA */
#define IP(a,b,c,d) { a , b , c|IF_SET , 0 }
#define OP(a,b,c,d) { a , b , c|IF_ASK , 0 }
#define IOP(a,b,c,d) { a , b , c|IF_SET|IF_ASK , 0 }
#define P(a,b,c,d) { a , b , c , 0 }
#endif /* HAS_MINDATA */
#endif /* IOP */

#endif /* NUMCARDS_H */
