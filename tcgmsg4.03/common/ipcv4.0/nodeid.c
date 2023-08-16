/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/nodeid.c,v 1.1 91/12/06 17:26:57 harrison Exp Locker: harrison $ */

#include "sndrcv.h"
#include "sndrcvP.h"

long NODEID_()
/*
  return logical node no. of current process
*/
{
  return SR_proc_id;
}
