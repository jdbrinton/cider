/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Karti Mayaram
**********/

#ifndef NUMDEXT_H
#define NUMDEXT_H

#ifdef __STDC__

extern int NUMDacLoad(GENmodel *, CKTcircuit *);
extern int NUMDask(CKTcircuit *, GENinstance *, int, IFvalue *, IFvalue *);
extern int NUMDdelete(GENmodel *, IFuid, GENinstance **);
extern void NUMDdestroy(GENmodel **);
extern int NUMDgetic(GENmodel *, CKTcircuit *);
extern int NUMDload(GENmodel *, CKTcircuit *);
extern int NUMDmDelete(GENmodel **, IFuid, GENmodel *);
extern int NUMDmParam(int, IFvalue *, GENmodel *);
extern int NUMDparam(int, IFvalue *, GENinstance *, IFvalue *);
extern int NUMDpzLoad(GENmodel *, CKTcircuit *, SPcomplex *);
extern int NUMDsetup(SMPmatrix *, GENmodel *, CKTcircuit *, int *);
extern int NUMDtemp(GENmodel *, CKTcircuit *);
extern int NUMDtrunc(GENmodel *, CKTcircuit *, double *);

#else				/* stdc */

extern int NUMDacLoad();
extern int NUMDask();
extern int NUMDdelete();
extern void NUMDdestroy();
extern int NUMDgetic();
extern int NUMDload();
extern int NUMDmDelete();
extern int NUMDmParam();
extern int NUMDparam();
extern int NUMDpzLoad();
extern int NUMDsetup();
extern int NUMDtemp();
extern int NUMDtrunc();

#endif				/* stdc */
#endif				/* NUMDEXT_H */
