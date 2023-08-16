/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/strdup.c,v 1.1 91/12/06 17:27:36 harrison Exp Locker: harrison $ */

#if defined(ULTRIX) || defined(SGI) || defined(NEXT) || defined(HPUX)
extern void *malloc();
#else
extern char *malloc();
#endif
extern char *strcpy();

char *strdup(s)
    char *s;
{
  char *new;

  if (new = malloc((unsigned) (strlen(s)+1)))
    (void) strcpy(new,s);

  return new;
}
