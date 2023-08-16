/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/
#ifdef DEV_nbjt2

#ifndef DEV_NBJT2
#define DEV_NBJT2

#include "nbjt2ext.h"
extern IFparm NBJT2pTable[];
extern IFparm NBJT2mPTable[];
extern char *NBJT2names[];
extern int NBJT2pTSize;
extern int NBJT2mPTSize;
extern int NBJT2nSize;
extern int NBJT2iSize;
extern int NBJT2mSize;

SPICEdev NBJT2info = {
  {
    "NBJT2",
    "2D Numerical Bipolar Junction Transistor model",

    &NBJT2nSize,
    &NBJT2nSize,
    NBJT2names,

    &NBJT2pTSize,
    NBJT2pTable,

    &NBJT2mPTSize,
    NBJT2mPTable,
    DEV_DEFAULT
  },

  NBJT2param,
  NBJT2mParam,
  NBJT2load,
  NBJT2setup,
  NULL,
  NBJT2setup,
  NBJT2temp,
  NBJT2trunc,
  NULL,
  NBJT2acLoad,
  NULL,
  NBJT2destroy,
#ifdef DELETES
  NBJT2mDelete,
  NBJT2delete,
#else				/* DELETES */
  NULL,
  NULL,
#endif				/* DELETES */
  NULL,
  NBJT2ask,
  NULL,
#ifdef AN_pz
  NBJT2pzLoad,
#else				/* AN_pz */
  NULL,
#endif				/* AN_pz */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  &NBJT2iSize,
  &NBJT2mSize


};
#endif
#endif
