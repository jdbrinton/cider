/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Karti Mayaram
**********/

#ifndef NUMD2EXT_H
#define NUMD2EXT_H

#ifdef __STDC__

extern int NUMD2acLoad(GENmodel *, CKTcircuit *);
extern int NUMD2ask(CKTcircuit *, GENinstance *, int, IFvalue *, IFvalue *);
extern int NUMD2delete(GENmodel *, IFuid, GENinstance **);
extern void NUMD2destroy(GENmodel **);
extern int NUMD2getic(GENmodel *, CKTcircuit *);
extern int NUMD2load(GENmodel *, CKTcircuit *);
extern int NUMD2mDelete(GENmodel **, IFuid, GENmodel *);
extern int NUMD2mParam(int, IFvalue *, GENmodel *);
extern int NUMD2param(int, IFvalue *, GENinstance *, IFvalue *);
extern int NUMD2pzLoad(GENmodel *, CKTcircuit *, SPcomplex *);
extern int NUMD2setup(SMPmatrix *, GENmodel *, CKTcircuit *, int *);
extern int NUMD2temp(GENmodel *, CKTcircuit *);
extern int NUMD2trunc(GENmodel *, CKTcircuit *, double *);

#else				/* stdc */

extern int NUMD2acLoad();
extern int NUMD2ask();
extern int NUMD2delete();
extern void NUMD2destroy();
extern int NUMD2getic();
extern int NUMD2load();
extern int NUMD2mDelete();
extern int NUMD2mParam();
extern int NUMD2param();
extern int NUMD2pzLoad();
extern int NUMD2setup();
extern int NUMD2temp();
extern int NUMD2trunc();

#endif				/* stdc */
#endif				/* NUMD2EXT_H */
