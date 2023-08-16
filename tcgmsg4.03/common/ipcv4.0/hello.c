/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/hello.c,v 1.1 91/12/06 17:26:36 harrison Exp Locker: harrison $ */

#include "sndrcv.h"

int main(argc, argv)
     int argc;
     char **argv;
/*
  Traditional first parallel program
*/
{
  PBEGIN_(argc, argv);

  (void) printf("Hello, world from node %ld.\n",NODEID_());

  PEND_();

  return 0;
}
