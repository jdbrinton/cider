/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
**********/

#include <string.h>
#include <ctype.h>

/*
 * Case-insensitive test of whether p is a prefix of s and at least the
 * first n characters are the same
 */
int
cinprefix(p, s, n)
register char *p, *s;
register int n;
{
  if (!p || !s) return( 0 );

  while (*p) {
    if ((isupper(*p) ? tolower(*p) : *p) != (isupper(*s) ? tolower(*s) : *s))
      return( 0 );
    p++;
    s++;
    n--;
  }
  if (n > 0)
    return( 0 );
  else
    return( 1 );
}

/*
 * Case-insensitive match of prefix string p against string s
 * returns the number of matching characters
 */
int
cimatch(p, s)
register char *p, *s;
{
  register int n = 0;

  if (!p || !s) return( 0 );

  while (*p) {
    if ((isupper(*p) ? tolower(*p) : *p) != (isupper(*s) ? tolower(*s) : *s))
      return( n );
    p++;
    s++;
    n++;
  }
  return( n );
}
