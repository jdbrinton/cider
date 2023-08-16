/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include <math.h>
#include "nummacs.h"

/* functions to compute max and one norms of a given vector of doubles */

double maxNorm( vector, size )
double *vector;
int size;
{
    double norm = 0.0;
    double candidate;
    int index, nIndex;

    nIndex = 1;
    for( index = 1; index <= size; index++ ) {
	candidate = ABS(vector[ index ]);
	if( candidate > norm ) {
	    norm = candidate;
	    nIndex = index;
	}
    }
    /*
    printf("\n maxNorm: index = %d", nIndex);
    */
    return( norm );
}


double oneNorm( vector, size )
double *vector;
int size;
{
    double norm = 0.0;
    double value;
    int index;

    for( index = 1; index <= size; index++ ) {
	value = vector[ index ];
	if( value < 0.0 )
	    norm -= value;
	else
	    norm += value;
    }
    return( norm );
}

double l2Norm( vector, size )
double *vector;
int size;
{
    double norm = 0.0;
    double value; 
    int index;

    for( index = 1; index <= size; index++ ) {
	value = vector[ index ];
	norm += value * value;
    }
    norm = sqrt( norm );
    return( norm );
}

/* 
 * dot():
 *   computes dot product of two vectors
 */
double dot( vector1, vector2, size )
double *vector1, *vector2;
int size;
{
    register double dot = 0.0;
    register int index;

    for( index = 1; index <= size; index++ ) {
	dot += vector1[ index ] * vector2[ index ];
    }
    return( dot );
}
