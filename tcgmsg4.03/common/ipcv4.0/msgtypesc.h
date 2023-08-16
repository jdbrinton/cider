/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/msgtypesc.h,v 1.1 91/12/06 17:26:51 harrison Exp Locker: harrison $ */

#ifndef MSGTYPES_H_
#define MSGTYPES_H_

/* 
   This defines bit masks that can be OR'ed with user types (1-32767)
   to indicate the nature of the data to the message passing system
*/ 
#ifdef IPSC
#define MSGDBL 0
#define MSGINT 0
#define MSGCHR 0
#else
#define MSGDBL  65536
#define MSGINT 131072
#define MSGCHR 262144
#endif

#endif
