/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/drand48.c,v 1.1 91/12/06 17:26:23 harrison Exp Locker: harrison $ */

#include "srftoc.h"

extern long random();
extern int srandom();

double DRAND48_()
{
  return ( (double) random() ) * 4.6566128752458e-10;
}

void SRAND48_(seed)
  unsigned *seed;
{
  (void) srandom(*seed);
}
