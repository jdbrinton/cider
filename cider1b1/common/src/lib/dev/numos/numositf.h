/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/
#ifdef DEV_numos

#ifndef DEV_NUMOS
#define DEV_NUMOS

#include "numosext.h"
extern IFparm NUMOSpTable[];
extern IFparm NUMOSmPTable[];
extern char *NUMOSnames[];
extern int NUMOSpTSize;
extern int NUMOSmPTSize;
extern int NUMOSnSize;
extern int NUMOSiSize;
extern int NUMOSmSize;

SPICEdev NUMOSinfo = {
  {
    "NUMOS",
    "2D Numerical MOS Field Effect Transistor model",

    &NUMOSnSize,
    &NUMOSnSize,
    NUMOSnames,

    &NUMOSpTSize,
    NUMOSpTable,

    &NUMOSmPTSize,
    NUMOSmPTable,
    DEV_DEFAULT
  },

  NUMOSparam,
  NUMOSmParam,
  NUMOSload,
  NUMOSsetup,
  NULL,
  NUMOSsetup,
  NUMOStemp,
  NUMOStrunc,
  NULL,
  NUMOSacLoad,
  NULL,
  NUMOSdestroy,
#ifdef DELETES
  NUMOSmDelete,
  NUMOSdelete,
#else				/* DELETES */
  NULL,
  NULL,
#endif				/* DELETES */
  NULL,
  NUMOSask,
  NULL,
#ifdef AN_pz
  NUMOSpzLoad,
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

  &NUMOSiSize,
  &NUMOSmSize


};
#endif
#endif
