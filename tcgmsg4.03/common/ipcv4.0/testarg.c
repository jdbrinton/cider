/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/testarg.c,v 1.1 91/12/06 17:27:43 harrison Exp Locker: harrison $ */

/*
  This checks the functioning of the include file farg.h
*/

#include "farg.h"

void parg()
{
  int i;

  for (i=0; i<ARGC_; i++)
    (void) printf("argv(%d)=%s\n", i, ARGV_[i]);
}
