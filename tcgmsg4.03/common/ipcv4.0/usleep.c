/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/usleep.c,v 1.1 91/12/06 17:26:20 harrison Exp Locker: harrison $ */

#ifdef AIX
#include <stdio.h>
#include <sys/select.h>
#endif
#include <sys/types.h>
#include <sys/time.h>

#ifdef STUPIDUSLEEP
void USleep(us)
     long us;
{
  int s = us/1000000;
  if (s == 0)
	s = 1;
  (void) sleep(s);
}
#else
void USleep(us)
     long us;
/*
  Sleep for the specified no. of micro-seconds ... uses the timeout
  on select ... it seems to be accurate to about a few centiseconds
  on a sun.  I don't know how much system resources it eats.
*/
{
  int width=0;
  struct timeval timelimit;

  timelimit.tv_sec = (int) (us/1000000);
  timelimit.tv_usec = (int) (us - timelimit.tv_sec*1000000);

  (void) select(width, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0,
		&timelimit);
}
#endif

