/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/xdrstuff.h,v 1.1 91/12/06 17:28:00 harrison Exp Locker: harrison $ */

/*
  Called automatically at start to allocate the XDR buffers
*/
extern void CreateXdrBuf();


/*
  Call to free the xdr buffers
*/
extern void DestroyXdrBuf();


/*
  int WriteXdrDouble(sock, x, n_double)
    int sock;
    double *x;
    long n_double;
  Write double x[n_double] to the socket translating to XDR representation.

  Returned is the number of bytes written to the socket.

  All errors are treated as fatal.
*/
extern int WriteXdrDouble();


/*
  int ReadXdrDouble(sock, x, n_double)
    int sock;
    double *x;
    long n_double;
  Read double x[n_double] from the socket translating from XDR representation.

  Returned is the number of bytes read from the socket.

  All errors are treated as fatal.
*/
extern int ReadXdrDouble();


/*
int WriteXdrLong(sock, x, n_long)
    int sock;
    long *x;
    long n_long;
  Write long x[n_long] to the socket translating to XDR representation.

  Returned is the number of bytes written to the socket.

  All errors are treated as fatal.
*/
extern int WriteXdrLong();


/*
int ReadXdrLong(sock, x, n_long)
    int sock;
    long *x;
    long n_long;
  Read long x[n_long] from the socket translating from XDR representation.

  Returned is the number of bytes read from the socket.

  All errors are treated as fatal.
*/
extern int ReadXdrLong();
