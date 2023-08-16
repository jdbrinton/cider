/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Karti Mayaram
**********/

#ifndef NBJT_H
#define NBJT_H

#ifdef __STDC__

extern int NBJTacLoad(GENmodel *, CKTcircuit *);
extern int NBJTask(CKTcircuit *, GENinstance *, int, IFvalue *, IFvalue *);
extern int NBJTdelete(GENmodel *, IFuid, GENinstance **);
extern void NBJTdestroy(GENmodel **);
extern int NBJTgetic(GENmodel *, CKTcircuit *);
extern int NBJTload(GENmodel *, CKTcircuit *);
extern int NBJTmDelete(GENmodel **, IFuid, GENmodel *);
extern int NBJTmParam(int, IFvalue *, GENmodel *);
extern int NBJTparam(int, IFvalue *, GENinstance *, IFvalue *);
extern int NBJTpzLoad(GENmodel *, CKTcircuit *, SPcomplex *);
extern int NBJTsetup(SMPmatrix *, GENmodel *, CKTcircuit *, int *);
extern int NBJTtemp(GENmodel *, CKTcircuit *);
extern int NBJTtrunc(GENmodel *, CKTcircuit *, double *);

#else				/* stdc */

extern int NBJTacLoad();
extern int NBJTask();
extern int NBJTdelete();
extern void NBJTdestroy();
extern int NBJTgetic();
extern int NBJTload();
extern int NBJTmDelete();
extern int NBJTmParam();
extern int NBJTparam();
extern int NBJTpzLoad();
extern int NBJTsetup();
extern int NBJTtemp();
extern int NBJTtrunc();

#endif				/* stdc */
#endif				/* NBJT_H */
