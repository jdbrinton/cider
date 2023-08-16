#if defined(CRAY) || defined(ARDENT)
#define NICEFTN_   NICEFTN
#else
#if (defined(AIX) && !defined(EXTNAME))
#define NICEFTN_   niceftn
#else
#define NICEFTN_   niceftn_
#endif
#endif


int NICEFTN_(ival)
     int *ival;
/*
  Wrapper around nice for FORTRAN users courtesy of Rick Kendall
  ... C has the system interface
*/
{
#ifndef IPSC
  return nice(*ival);
#else
  return 0;
#endif
}
