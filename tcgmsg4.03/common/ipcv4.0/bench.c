/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/test.c,v 1.1 91/12/06 17:27:41 harrison Exp Locker: harrison $ */

#include <stdio.h>
#if !defined(SEQUENT) && !defined(CONVEX)
#include <memory.h>
#endif

#include "sndrcv.h"
#include "evlog.h"

extern char *memalign();
#if defined(ULTRIX) || defined(SGI) || defined(NEXT) || defined(HPUX)
extern void *malloc();
#else
extern char *malloc();
#endif
extern unsigned char CheckByte();
extern double DRAND48_();
#if defined(SUN)
extern char *sprintf();
#endif
#ifdef IPSC
#define bzero(A,N) memset((A), 0, (N))
#endif

static void TestGlobals( passes )
int passes;
{
#define MAXLENG 2048
  double *dtest;
  long len, pass;
  long me = NODEID_(), nproc = NNODES_(), from=NNODES_()-1;
  long dtype=4+MSGDBL;
  double start, used, overhead;
  FILE *data;
  char buf[80];

  if (!(dtest = (double *) malloc((unsigned) (MAXLENG*sizeof(double)))))
    Error("TestGlobals: failed to allocated dtest", (long) MAXLENG);

  for (pass = 1; pass <= passes; pass++ ) {
    if (me == 0) {
      (void) sprintf(buf, "gdsum%dr%d.xgf", nproc, pass );
      data = fopen(buf,"w");
      (void) printf("Global Double Sum Test\n");
      (void) fflush(stdout);
    }

    for (len=1; len <= 2001; len += 20) {
      long dlen = len*sizeof(double);
      long i;
      int nloops = 20 + 100/len;
      int loop = nloops;
      
      /* Test global sum */
      start = TCGTIME_();
      while (loop--) {
	for (i=0; i<len; i++) {
	  dtest[i] = (double) i*me;
	}
      }
      /*
      overhead = TCGTIME_() - start;
      */
      overhead = 0.0;

      loop = nloops;
      start = TCGTIME_();
      while (loop--) {
	for (i=0; i<len; i++) {
	  dtest[i] = (double) i*me;
	}
	DGOP_(&dtype, dtest, &len, "+");
      }
      used = TCGTIME_() - start;

      if (me == 0) {
	fprintf(data, "%d %g\n",
	    len, (used - overhead)/nloops);
	fflush(data);
      }
    }
      
    if (me == 0) {
      fclose(data);
    }
  }
  free((char *) dtest);
}

int main(argc, argv)
    int argc;
    char **argv;
{
  long type;
  long lenbuf;
  long node, passes;
  
  PBEGIN_(argc, argv);

  (void) printf("In process %ld\n", NODEID_());
  (void) fflush(stdout);

  /* Read user input for action */

  lenbuf = sizeof(long);
  node = 0;

  (void) fflush(stdout);
  if (NODEID_() == 0)
     (void) sleep(1);
  type = 999;
  SYNCH_(&type);
  (void) sleep(1);

/*
  if (NODEID_() == 0) {
  again:
    (void) printf("\nEnter number of passes: ");

    (void) fflush(stdout);
    if (scanf("%ld", &passes) != 1)
      Error("%s: input of option failed", argv[0], (long) -1);
    (void) printf("\n");
    (void) fflush(stdout);
    if ( (passes < 1) || (passes > 10) ) {
      (void) printf("%s: too few or too many passes.\n");
      goto again;
    }
  }
  */
  passes = 5;
  type = 2 | MSGINT;
  BRDCST_(&type, (char *) &passes, &lenbuf, &node);
  TestGlobals( passes );
  PEND_();
}
