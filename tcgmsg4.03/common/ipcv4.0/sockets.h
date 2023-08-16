/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/sockets.h,v 1.1 91/12/06 17:27:31 harrison Exp Locker: harrison $ */

extern void ShutdownAll();
extern int ReadFromSocket();
extern int WriteToSocket();
extern void CreateSocketAndBind();
extern int ListenAndAccept();
extern int CreateSocketAndConnect();
extern long PollSocket();
