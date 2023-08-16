/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/mitod.c,v 1.1 91/12/06 17:26:49 harrison Exp Locker: harrison $ */

#include "sndrcv.h"

/*
  These routines use C's knowledge of the sizes of data types
  to generate a portable mechanism for FORTRAN to translate
  between bytes, integers and doubles. Note that we assume that
  FORTRAN integers are the same size as C longs.
*/

long MITOD_(n)
     long *n;
/*
  Return the minimum no. of doubles in which we can store n longs
*/
{
  if (*n < 0)
    Error("MITOD_: negative argument",*n);

  return (long) ( (MITOB_(n) + sizeof(double) - 1) / sizeof(double) );
}
