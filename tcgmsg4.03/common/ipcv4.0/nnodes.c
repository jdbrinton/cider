/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/nnodes.c,v 1.1 91/12/06 17:26:55 harrison Exp Locker: harrison $ */

#include "sndrcv.h"
#include "sndrcvP.h"

long NNODES_()
/*
  return total no. of processes
*/
{
  return SR_n_proc;
}

