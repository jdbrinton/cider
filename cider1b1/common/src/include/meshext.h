/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
**********/
/**********
 Mesh Definitions and Declarations.
**********/
#ifndef MESHEXT_H
#define MESHEXT_H

#include "meshdefs.h"
#include "gendev.h"

/* Exports */
#ifdef __STDC__
extern double *MESHmkArray( MESHcoord *, int );
extern void MESHiBounds( MESHcoord *, int *, int * );
extern void MESHlBounds( MESHcoord *, double *, double * );
extern int MESHlocate( MESHcoord *, double );
extern int MESHcheck( char, MESHcard * );
extern int MESHsetup( char, MESHcard *, MESHcoord **, int * );
#else
extern double *MESHmkArray();
extern void MESHiBounds();
extern void MESHlBounds();
extern int MESHlocate();
extern int MESHcheck();
extern int MESHsetup();
#endif /* __STDC__ */
#endif /* MESHEXT_H */
