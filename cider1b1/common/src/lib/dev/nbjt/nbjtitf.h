/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/
#ifdef DEV_nbjt

#ifndef DEV_NBJT
#define DEV_NBJT

#include "nbjtext.h"
extern IFparm NBJTpTable[];
extern IFparm NBJTmPTable[];
extern char *NBJTnames[];
extern int NBJTpTSize;
extern int NBJTmPTSize;
extern int NBJTnSize;
extern int NBJTiSize;
extern int NBJTmSize;

SPICEdev NBJTinfo = {
  {
    "NBJT",
    "1D Numerical Bipolar Junction Transistor model",

    &NBJTnSize,
    &NBJTnSize,
    NBJTnames,

    &NBJTpTSize,
    NBJTpTable,

    &NBJTmPTSize,
    NBJTmPTable,
    DEV_DEFAULT
  },

  NBJTparam,
  NBJTmParam,
  NBJTload,
  NBJTsetup,
  NULL,
  NBJTsetup,
  NBJTtemp,
  NBJTtrunc,
  NULL,
  NBJTacLoad,
  NULL,
  NBJTdestroy,
#ifdef DELETES
  NBJTmDelete,
  NBJTdelete,
#else				/* DELETES */
  NULL,
  NULL,
#endif				/* DELETES */
  NULL,
  NBJTask,
  NULL,
#ifdef AN_pz
  NBJTpzLoad,
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

  &NBJTiSize,
  &NBJTmSize


};
#endif
#endif
