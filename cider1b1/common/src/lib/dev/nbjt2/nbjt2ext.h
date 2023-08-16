/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Karti Mayaram
**********/

#ifndef NBJT2EXT_H
#define NBJT2EXT_H

#ifdef __STDC__

extern int NBJT2acLoad(GENmodel *, CKTcircuit *);
extern int NBJT2ask(CKTcircuit *, GENinstance *, int, IFvalue *, IFvalue *);
extern int NBJT2delete(GENmodel *, IFuid, GENinstance **);
extern void NBJT2destroy(GENmodel **);
extern int NBJT2getic(GENmodel *, CKTcircuit *);
extern int NBJT2load(GENmodel *, CKTcircuit *);
extern int NBJT2mDelete(GENmodel **, IFuid, GENmodel *);
extern int NBJT2mParam(int, IFvalue *, GENmodel *);
extern int NBJT2param(int, IFvalue *, GENinstance *, IFvalue *);
extern int NBJT2pzLoad(GENmodel *, CKTcircuit *, SPcomplex *);
extern int NBJT2setup(SMPmatrix *, GENmodel *, CKTcircuit *, int *);
extern int NBJT2temp(GENmodel *, CKTcircuit *);
extern int NBJT2trunc(GENmodel *, CKTcircuit *, double *);

#else				/* STDC */

extern int NBJT2acLoad();
extern int NBJT2ask();
extern int NBJT2delete();
extern void NBJT2destroy();
extern int NBJT2getic();
extern int NBJT2load();
extern int NBJT2mDelete();
extern int NBJT2mParam();
extern int NBJT2param();
extern int NBJT2pzLoad();
extern int NBJT2setup();
extern int NBJT2temp();
extern int NBJT2trunc();

#endif				/* STDC */
#endif				/* NBJT2EXT_H */
