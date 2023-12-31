/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include <math.h>
#include "nummacs.h"
#include "accuracy.h"

/*
 * this function computes the bernoulli function 
 * f(x) = x / ( exp (x) - 1 ) and its derivatives. the function
 * f(-x) alongwith its derivative is also computed.
 */

/*
 * input    delta-psi 
 * outputs  f(x), df(x)/dx, f(-x), and df(-x)/dx
 */

bernoulli (x, pfx, pDfxDx, pfMx, pDfMxDx, derivAlso)
double x, *pfx, *pDfxDx, *pfMx, *pDfMxDx;
BOOLEAN derivAlso;
{
    double fx, fMx; 
    double dFxDx = 0.0; 
    double dFMxDx = 0.0;
    double expX, temp;

    if( x <= -BMax ) {
	fx = -x;
	if( x <= -ExpLim ) {
	    fMx = 0.0;
	    if( derivAlso ) {
		dFxDx = -1.0;
		dFMxDx = 0.0;
	    }
	}
	else {
	    expX = exp( x );
	    fMx = -x * expX;
	    if( derivAlso ) {
		dFxDx = fMx - 1.0;
		dFMxDx = -expX * ( 1.0 + x );
	    }
	}
    }
    else if ( ABS( x) <= BMin ) {
	fx = 1.0 / (1.0 + 0.5 * x );
	fMx = 1.0 / (1.0 - 0.5 * x );
	if( derivAlso ) {
	    temp = 1.0 + x;
	    dFxDx = -(0.5 + x / 3.0) / temp;
	    dFMxDx = (0.5 + 2 * x /3.0 )/ temp;
	}
    }
    else if ( x >= BMax ) {
	fMx = x;
	if( x >= ExpLim ) {
	    fx = 0.0;
	    if( derivAlso ) {
		dFxDx = 0.0;
		dFMxDx = 1.0;
	    }
	}
	else {
	    expX = exp( -x );
	    fx = x * expX;
	    if( derivAlso ) {
		dFxDx = expX * ( 1.0 - x );
		dFMxDx = 1.0 - fx;
	    }
	}
    }
    else {
	expX = exp( x );
	temp = 1.0 / ( expX - 1.0 );
	fx = x * temp;
	fMx = expX * fx;
	if( derivAlso ) {
	    dFxDx = temp * ( 1.0 - fMx );
	    dFMxDx = temp * ( expX - fMx );
	}
    }
    *pfx = fx;
    *pfMx = fMx;
    *pDfxDx = dFxDx;
    *pDfMxDx = dFMxDx;
}
