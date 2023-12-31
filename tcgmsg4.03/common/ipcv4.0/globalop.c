/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/globalop.c,v 1.1 91/12/06 17:26:34 harrison Exp Locker: harrison $ */

#include "sndrcv.h"
#include "msgtypesc.h"

#define MAX(a,b) (((a) >= (b)) ? (a) : (b))
#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define ABS(a) (((a) >= 0) ? (a) : (-(a)))

#if defined(ULTRIX) || defined(SGI) || defined(NEXT) || defined(HPUX) || \
defined(KSR)
extern void *malloc();
#else
extern char *malloc();
#endif
extern void free();

#if !defined(IPSC) && !defined(DELTA)
#include "sndrcvP.h"

#define GOP_BUF_SIZE 8192

static void idoop(n, op, x, work)
     long n;
     char *op;
     long *x, *work;
{
  if (strncmp(op,"+",1) == 0)
    while(n--)
      *x++ += *work++;
  else if (strncmp(op,"*",1) == 0)
    while(n--)
      *x++ *= *work++;
  else if (strncmp(op,"max",3) == 0)
    while(n--) {
      *x = MAX(*x, *work);
      x++; work++;
    }
  else if (strncmp(op,"min",3) == 0)
    while(n--) {
      *x = MIN(*x, *work);
      x++; work++;
    }
  else if (strncmp(op,"absmax",6) == 0)
    while(n--) {
      register long x1 = ABS(*x), x2 = ABS(*work);
      *x = MAX(x1, x2);
      x++; work++;
    }
  else if (strncmp(op,"absmin",6) == 0)
    while(n--) {
      register long x1 = ABS(*x), x2 = ABS(*work);
      *x = MIN(x1, x2);
      x++; work++;
    }
  else if (strncmp(op,"none",4) == 0)
    /* No op */ ;
  else
    Error("idoop: unknown operation requested", (long) n);
}

static void ddoop(n, op, x, work)
     long n;
     char *op;
     double *x, *work;
{
  if (strncmp(op,"+",1) == 0)
    while(n--)
      *x++ += *work++;
  else if (strncmp(op,"*",1) == 0)
    while(n--)
      *x++ *= *work++;
  else if (strncmp(op,"max",3) == 0)
    while(n--) {
      *x = MAX(*x, *work);
      x++; work++;
    }
  else if (strncmp(op,"min",3) == 0)
    while(n--) {
      *x = MIN(*x, *work);
      x++; work++;
    }
  else if (strncmp(op,"absmax",6) == 0)
    while(n--) {
      register double x1 = ABS(*x), x2 = ABS(*work);
      *x = MAX(x1, x2);
      x++; work++;
    }
  else if (strncmp(op,"absmin",6) == 0)
    while(n--) {
      register double x1 = ABS(*x), x2 = ABS(*work);
      *x = MIN(x1, x2);
      x++; work++;
    }
  else if (strncmp(op,"none",4) == 0)
    /* No op */ ;
  else
    Error("ddoop: unknown operation requested", (long) n);
}

/*ARGSUSED*/
void DGOP_(ptype, x, pn, op)
     double *x;
     long *ptype, *pn;
     char *op;
/*
  Global summation optimized for networks of clusters of processes.

  This routine is directly callable from C only.  There is a
  wrapper that makes fortran work (see bottom of this file).
*/
{
  long me = NODEID_();
  long master = SR_clus_info[SR_clus_id].masterid;
  long nslave = SR_clus_info[SR_clus_id].nslave;
  long slaveid = me - master;
  long synch = 1;
  long type = (*ptype & MSGDBL) ? *ptype : *ptype + MSGDBL;
  long nleft = *pn;
  long buflen = MIN(nleft,GOP_BUF_SIZE); /* Try to get even sized buffers */
  long nbuf   = (nleft-1) / buflen + 1;
  long zero = 0;
  double *tmp = x;
  double *work;
  long nb, ndo, lenmes, from, up, left, right;

  buflen = (nleft-1) / nbuf + 1;
  if (!(work = (double *) malloc((unsigned) (buflen*sizeof(double)))))
     Error("DGOP: failed to malloc workspace", nleft);

  /* This loop for pipelining and to avoid caller
     having to provide workspace */

  while (nleft) {
    ndo = MIN(nleft, buflen);
    nb  = ndo * sizeof(double);

    /* Do summation amoung slaves in a cluster */

    up    = master + (slaveid-1)/2;
    left  = master + 2*slaveid + 1;
    right = master + 2*slaveid + 2;

    if (left < (master+nslave)) {
      RCV_(&type, (char *) work, &nb, &lenmes, &left, &from, &synch);
      ddoop(ndo, op, x, work);
    }
    if (right < (master+nslave)) {
      RCV_(&type, (char *) work, &nb, &lenmes, &right, &from, &synch);
      ddoop(ndo, op, x, work);
    }
    if (me != master)
      SND_(&type, (char *) x, &nb, &up, &synch);

    /* Do summation amoung masters */

    if (me == master) {
      up    = (SR_clus_id-1)/2;
      left  = 2*SR_clus_id + 1;
      right = 2*SR_clus_id + 2;
      up = SR_clus_info[up].masterid;
      left = (left < SR_n_clus) ? SR_clus_info[left].masterid : -1;
      right = (right < SR_n_clus) ? SR_clus_info[right].masterid : -1;

      if (left > 0) {
        RCV_(&type, (char *) work, &nb, &lenmes, &left, &from, &synch);
        ddoop(ndo, op, x, work);
      }
      if (right > 0) {
        RCV_(&type, (char *) work, &nb, &lenmes, &right, &from, &synch);
        ddoop(ndo, op, x, work);
      }
      if (me != 0)
        SND_(&type, (char *) x, &nb, &up, &synch);
    }
    nleft -= ndo;
    x     += ndo;
    type  += 13;   /* Temporary hack for hippi switch */
  }
  free((char *) work);

  /* Zero has the results ... broadcast them back */
  nb = *pn * sizeof(double);
  BRDCST_(&type, (char *) tmp, &nb, &zero);
}

void IGOP_(ptype, x, pn, op)
     long *x;
     long *ptype, *pn;
     char *op;
/*
  Global summation optimized for networks of clusters of processes.

  This routine is directly callable from C only.  There is a
  wrapper that makes fortran work (see the bottom of this file).
*/
{
  long me = NODEID_();
  long master = SR_clus_info[SR_clus_id].masterid;
  long nslave = SR_clus_info[SR_clus_id].nslave;
  long slaveid = me - master;
  long synch = 1;
  long type = (*ptype & MSGINT) ? *ptype : *ptype + MSGINT;
  long nleft = *pn;
  long zero = 0;
  long *tmp = x;
  long *work;
  long nb, ndo, lenmes, from, up, left, right;

  if (!(work = (long *) 
	malloc((unsigned) (MIN(nleft,GOP_BUF_SIZE)*sizeof(long)))))
     Error("IGOP: failed to malloc workspace", nleft);

  /* This loop for pipelining and to avoid caller
     having to provide workspace */

  while (nleft) {
    ndo = MIN(nleft, GOP_BUF_SIZE);
    nb  = ndo * sizeof(long);

    /* Do summation amoung slaves in a cluster */

    up    = master + (slaveid-1)/2;
    left  = master + 2*slaveid + 1;
    right = master + 2*slaveid + 2;

    if (left < (master+nslave)) {
      RCV_(&type, (char *) work, &nb, &lenmes, &left, &from, &synch);
      idoop(ndo, op, x, work);
    }
    if (right < (master+nslave)) {
      RCV_(&type, (char *) work, &nb, &lenmes, &right, &from, &synch);
     idoop(ndo, op, x, work);
    }
    if (me != master)
      SND_(&type, (char *) x, &nb, &up, &synch);

    /* Do summation amoung masters */

    if (me == master) {
      up    = (SR_clus_id-1)/2;
      left  = 2*SR_clus_id + 1;
      right = 2*SR_clus_id + 2;
      up = SR_clus_info[up].masterid;
      left = (left < SR_n_clus) ? SR_clus_info[left].masterid : -1;
      right = (right < SR_n_clus) ? SR_clus_info[right].masterid : -1;

      if (left > 0) {
        RCV_(&type, (char *) work, &nb, &lenmes, &left, &from, &synch);
        idoop(ndo, op, x, work);
      }
      if (right > 0) {
        RCV_(&type, (char *) work, &nb, &lenmes, &right, &from, &synch);
        idoop(ndo, op, x, work);
      }
      if (me != 0)
        SND_(&type, (char *) x, &nb, &up, &synch);
    }
    nleft -= ndo;
    x     += ndo;
    type  += 13;   /* Temporary hack for hippi switch */
  }
  (void) free((char *) work);

  /* Zero has the results ... broadcast them back */
  nb = *pn * sizeof(long);
  BRDCST_(&type, (char *) tmp, &nb, &zero);
}

#endif

/* Wrapper for fortran interface ... UGH ... note that
   string comparisons above do NOT rely on NULL termination
   of the operation character string */

#ifdef CRAY
#include <fortran.h>
#endif
#ifdef ARDENT
struct char_desc {
  char *string;
  int len;
};
#endif

/*ARGSUSED*/
#ifdef ARDENT
void dgop_(ptype, x, pn, arg)
     long *ptype, *pn;
     double *x;
     struct char_desc *arg;
{
  char *op = arg->string;
  int len_op = arg->len;
#endif
#if defined(CRAY)
void dgop_(ptype, x, pn, arg)
     long *ptype, *pn;
     double *x;
     _fcd arg;
{
  char *op = _fcdtocp(arg);
  int len_op = _fcdlen(arg);
#endif
#if !defined(ARDENT) && !defined(CRAY)
void dgop_(ptype, x, pn, op, len_op)
     long *ptype, *pn;
     double *x;
     char *op;
     int len_op;
{
#endif
  DGOP_(ptype, x, pn, op);
}

/* This crap to handle FORTRAN character strings */

/*ARGSUSED*/
#ifdef ARDENT
void igop_(ptype, x, pn, arg)
     long *ptype, *pn;
     long *x;
     struct char_desc *arg;
{
  char *op = arg->string;
  int len_op = arg->len;
#endif
#if defined(CRAY)
void igop_(ptype, x, pn, arg)
     long *ptype, *pn;
     long *x;
     _fcd arg;
{
  char *op = _fcdtocp(arg);
  int len_op = _fcdlen(arg);
#endif
#if !defined(ARDENT) && !defined(CRAY)
void igop_(ptype, x, pn, op, len_op)
     long *ptype, *pn;
     long *x;
     char *op;
     int len_op;
{
#endif
  IGOP_(ptype, x, pn, op);
}
