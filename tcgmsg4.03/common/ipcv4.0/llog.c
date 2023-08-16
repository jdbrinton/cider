/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/llog.c,v 1.1 91/12/06 17:26:40 harrison Exp Locker: harrison $ */

#include <stdio.h>

#include <sys/types.h>
#include <sys/time.h>

#include "sndrcv.h"

#if defined(SUN)
extern char *sprintf();
#endif
#ifndef SGI
extern time_t time();
#endif

extern void Error();

void LLOG_()
/*
  close and open stdin and stdout to append to a local logfile
  with the name log.<process#> in the current directory
*/
{
  char name[12];
  time_t t;

  (void) sprintf(name, "log.%03ld",NODEID_());

  (void) fflush(stdout);
  (void) fflush(stderr);

  if (freopen(name, "A", stdout) == (FILE *) NULL)
    Error("LLOG_: error re-opening stdout", (long) -1);

  if (freopen(name, "A", stderr) == (FILE *) NULL)
    Error("LLOG_: error re-opening stderr", (long) -1);

  (void) time(&t);
  (void) printf("\n\nLog file opened : %s\n\n",ctime(&t));
  (void) fflush(stdout);
}
