/**********
Copyright 1991 Regents of the University of California. All rights reserved.
Authors: 1987 Karti Mayaram, 1991 David Gates
**********/
/*
 * Macros used by numerical simulation routines
 * Note: Memory Allocation should probably be changed to Spice Routines
 */
#ifndef NUMMACS_H
#define NUMMACS_H

#include <stdio.h>

/* 
 * BOOLEANS
 */

#define  BOOLEAN	int
#ifndef  FALSE
#define  FALSE		0
#endif
#ifndef  TRUE
#define  TRUE		1
#endif
#define  YES            1
#define  NO             0
#define  NOT		!
#define  AND		&&
#define  OR		||
#define  IS		==
#define  ISNOT          !=

#define NIL(type) 	((type *)0)

/* 
 * RELATIONAL
 */

/* Macro functions that return the maximun or minimum independent of type. */
#ifndef MAX
#define  MAX(a,b)           ((a) > (b) ? (a) : (b))
#define  MIN(a,b)           ((a) < (b) ? (a) : (b))
#endif

/* Macro function that returns the absolute value of a floating point number. */
#define  ABS(a)             ((a) < 0.0 ? -(a) : (a))
#define  SGN(a)             ((a) < 0.0 ? -(1.0) : (1.0))

/* Macro procedure that swaps two entities. */
#define  SWAP(type, a, b)   {type swapx; swapx = a; a = b; b = swapx;}


/*
 * MEMORY ALLOCATION
 */

#ifndef MALLOC
#define MALLOC(type,number)  (type *)malloc((unsigned)(sizeof(type)*(number)))
#define CALLOC(type,number)  (type *)calloc((number), (unsigned)(sizeof(type)))
#define REALLOC(ptr,type,number)  \
  ptr = (type *)realloc((char *)ptr,(unsigned)(sizeof(type)*(number)))
#define FREE(pointer)  \
  {  if ((pointer) != NULL) free((char *)pointer); (pointer) = NULL; }
#endif

#define ALLOC(ptr,type,number)   \
if ((number) && (!(ptr = (type *)calloc((number), (unsigned)(sizeof(type)))))) {\
  fprintf( stderr, "Out of Memory\n" );\
  exit( 1 );\
}
#define XALLOC(ptr,type,number)   \
if ((number) && (!(ptr = (type *)calloc((number), (unsigned)(sizeof(type)))))) {\
  SPfrontEnd->IFerror( E_PANIC, "Out of Memory", NIL(IFuid) );\
  exit( 1 );\
}
#define RALLOC(ptr,type,number)\
if ((number) && (!(ptr = (type *)calloc((number), (unsigned)(sizeof(type)))))) {\
  return(E_NOMEM);\
}



/*
 * TIMING
 */

/* Macro that queries the system to find the process time. */

#define ELAPSED_TIME( time )				\
{   struct tms {int user, sys, cuser, csys;} buffer;	\
							\
    times(&buffer);					\
    time = buffer.user / 60.0;				\
}

#endif /* NUMMACS_H */
