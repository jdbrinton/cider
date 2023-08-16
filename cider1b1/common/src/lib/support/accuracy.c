/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include <math.h>
#include "accuracy.h"

/* XXX Globals are hiding here. */
double BMin, BMax, ExpLim, Acc, MuLim, MutLim;

void
evalAccLimits()
{
    double acc = 1.0;
    double xl = 0.0;
    double xu = 1.0;
    double xh, x1, x2, expLim;
    double muLim, temp1, temp2, temp3, temp4;

    for( ; acc+1.0 > 1.0 ; ) {
	acc *= 0.5;
    }
    acc *= 2.0;
    Acc = acc;

    xh = 0.5 * (xl + xu);
    for( ; xu-xl > 2.0 * acc * (xu + xl); ) {
	x1 = 1.0 / ( 1.0 + 0.5 * xh );
	x2 = xh /( exp(xh) - 1.0 );
	if( x1 - x2 <= acc * (x1 + x2)) {
	    xl = xh;
	} else {
	    xu = xh;
	}
	xh = 0.5 * (xl + xu);
    }

    BMin = xh;
    BMax = -log( acc );
    expLim = 80.0;
    for( ; exp( -expLim ) > 0.0; ) {
	expLim += 1.0;
    }
    expLim -= 1.0;
    ExpLim = expLim;

    muLim = 1.0;
    temp4 = 0.0;
    for( ; 1.0 - temp4 > acc; ){
        muLim *= 0.5;
	temp1 = muLim;
	temp2 = pow( temp1 , 0.333 ) ;
	temp3 = 1.0 / (1.0 + temp1 * temp2 );
	temp4 = pow( temp3 , 0.37/1.333 );
    }
    muLim *= 2.0;
    MuLim = muLim;

    muLim = 1.0;
    temp3 = 0.0;
    for( ; 1.0 - temp3 > acc; ){
        muLim *= 0.5;
	temp1 = muLim;
	temp2 = 1.0 / (1.0 + temp1 * temp1 );
	temp3 = sqrt( temp2 );
    }
    muLim *= 2.0;
    MutLim = muLim;

}

/*
main ()
{
    double x;
    double bx, dbx, bMx, dbMx;

    evalAccLimits();
    for( x = 0.0; x <= 100.0 ; x += 1.0 ) {
	bernoulliNew( x, &bx, &dbx, &bMx, &dbMx, 1);
	printf( "\nbernoulliNew: x = %e bx = %e dbx = %e bMx = %e dbMx = %e ", x, bx, dbx, bMx, dbMx );
	bernoulli( x, &bx, &dbx, &bMx, &dbMx);
	printf( "\nbernoulli: x = %e bx = %e dbx = %e bMx = %e dbMx = %e ", x, bx, dbx, bMx, dbMx );
    }
    for( x = 0.0; x >= -100.0 ; x -= 1.0 ) {
	bernoulliNew( x, &bx, &dbx, &bMx, &dbMx, 1);
	printf( "\nbernoulliNew: x = %e bx = %e dbx = %e bMx = %e dbMx = %e ", x, bx, dbx, bMx, dbMx );
	bernoulli( x, &bx, &dbx, &bMx, &dbMx);
	printf( "\nbernoulli: x = %e bx = %e dbx = %e bMx = %e dbMx = %e ", x, bx, dbx, bMx, dbMx );
    }
}

*/
