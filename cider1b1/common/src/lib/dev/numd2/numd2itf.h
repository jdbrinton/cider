/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/
#ifdef DEV_numd2

#ifndef DEV_NUMD2
#define DEV_NUMD2

#include "numd2ext.h"
extern IFparm NUMD2pTable[];
extern IFparm NUMD2mPTable[];
extern char *NUMD2names[];
extern int NUMD2pTSize;
extern int NUMD2mPTSize;
extern int NUMD2nSize;
extern int NUMD2iSize;
extern int NUMD2mSize;

SPICEdev NUMD2info = {
  {
    "NUMD2",
    "2D Numerical Junction Diode model",

    &NUMD2nSize,
    &NUMD2nSize,
    NUMD2names,

    &NUMD2pTSize,
    NUMD2pTable,

    &NUMD2mPTSize,
    NUMD2mPTable,
    DEV_DEFAULT
  },

  NUMD2param,
  NUMD2mParam,
  NUMD2load,
  NUMD2setup,
  NULL,
  NUMD2setup,
  NUMD2temp,
  NUMD2trunc,
  NULL,
  NUMD2acLoad,
  NULL,
  NUMD2destroy,
#ifdef DELETES
  NUMD2mDelete,
  NUMD2delete,
#else				/* DELETES */
  NULL,
  NULL,
#endif				/* DELETES */
  NULL,
  NUMD2ask,
  NULL,
#ifdef AN_pz
  NUMD2pzLoad,
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

  &NUMD2iSize,
  &NUMD2mSize


};
#endif
#endif
