/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Karti Mayaram
**********/

#ifndef NUMOSEXT_H
#define NUMOSEXT_H

#ifdef __STDC__

extern int NUMOSacLoad(GENmodel *, CKTcircuit *);
extern int NUMOSask(CKTcircuit *, GENinstance *, int, IFvalue *, IFvalue *);
extern int NUMOSdelete(GENmodel *, IFuid, GENinstance **);
extern void NUMOSdestroy(GENmodel **);
extern int NUMOSgetic(GENmodel *, CKTcircuit *);
extern int NUMOSload(GENmodel *, CKTcircuit *);
extern int NUMOSmDelete(GENmodel **, IFuid, GENmodel *);
extern int NUMOSmParam(int, IFvalue *, GENmodel *);
extern int NUMOSparam(int, IFvalue *, GENinstance *, IFvalue *);
extern int NUMOSpzLoad(GENmodel *, CKTcircuit *, SPcomplex *);
extern int NUMOSsetup(SMPmatrix *, GENmodel *, CKTcircuit *, int *);
extern int NUMOStemp(GENmodel *, CKTcircuit *);
extern int NUMOStrunc(GENmodel *, CKTcircuit *, double *);

#else				/* stdc */

extern int NUMOSacLoad();
extern int NUMOSask();
extern int NUMOSdelete();
extern void NUMOSdestroy();
extern int NUMOSgetic();
extern int NUMOSload();
extern int NUMOSmDelete();
extern int NUMOSmParam();
extern int NUMOSparam();
extern int NUMOSpzLoad();
extern int NUMOSsetup();
extern int NUMOStemp();
extern int NUMOStrunc();

#endif				/* stdc */
#endif				/* NUMOSEXT_H */
