/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/setdbg.c,v 1.1 91/12/06 17:27:16 harrison Exp Locker: harrison $ */

#include "sndrcv.h"
#include "sndrcvP.h"

void SETDBG_(value)
    long *value;
/*
  set global debug flag for this process to value
*/
{
  SR_debug = *value;
}

