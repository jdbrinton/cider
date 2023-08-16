/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/
#ifdef DEV_numd

#ifndef DEV_NUMD
#define DEV_NUMD

#include "numdext.h"
extern IFparm NUMDpTable[];
extern IFparm NUMDmPTable[];
extern char *NUMDnames[];
extern int NUMDpTSize;
extern int NUMDmPTSize;
extern int NUMDnSize;
extern int NUMDiSize;
extern int NUMDmSize;

SPICEdev NUMDinfo = {
  {
    "NUMD",
    "1D Numerical Junction Diode model",

    &NUMDnSize,
    &NUMDnSize,
    NUMDnames,

    &NUMDpTSize,
    NUMDpTable,

    &NUMDmPTSize,
    NUMDmPTable,
    DEV_DEFAULT
  },

  NUMDparam,
  NUMDmParam,
  NUMDload,
  NUMDsetup,
  NULL,				/* no unSetup routine */
  NUMDsetup,
  NUMDtemp,
  NUMDtrunc,
  NULL,
  NUMDacLoad,
  NULL,
  NUMDdestroy,
#ifdef DELETES
  NUMDmDelete,
  NUMDdelete,
#else				/* DELETES */
  NULL,
  NULL,
#endif				/* DELETES */
  NULL,
  NUMDask,
  NULL,
#ifdef AN_pz
  NUMDpzLoad,
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

  &NUMDiSize,
  &NUMDmSize


};
#endif
#endif
