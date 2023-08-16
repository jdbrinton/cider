/**********
Copyright 1991 Regents of the University of California. All rights reserved.
Authors: 1987 Karti Mayaram, 1991 David Gates
**********/
/*
 * Complex Number Macros for Numerical Admittances
 */
#ifndef NUMCMPLX_H
#define NUMCMPLX_H

typedef struct {
    double real;
    double imag;
} complex;

struct mosAdmittances {
    complex yIdVdb;
    complex yIdVsb;
    complex yIdVgb;
    complex yIsVdb;
    complex yIsVsb;
    complex yIsVgb;
    complex yIgVdb;
    complex yIgVsb;
    complex yIgVgb;
};


/* Complex assignment statements. */
#define  CMPLX_MULT_SCALAR(to,from, scalar)      \
	 {   (to).real = (from).real * scalar;   \
	     (to).imag = (from).imag * scalar;   \
	 }
#define  CMPLX_MULT_SELF_SCALAR(cnum, scalar)      \
	 {   (cnum).real *= scalar;   \
	     (cnum).imag *= scalar;   \
	 }
#define  CMPLX_NEGATE_SELF(cnum)	\
         {   (cnum).real = -(cnum).real;	\
             (cnum).imag = -(cnum).imag;	\
         }
#define  CMPLX_ADD_SELF_SCALAR(cnum, scalar)      \
	 {   (cnum).real += scalar;   \
	 }
#define  CMPLX_ASSIGN(to,from)		\
         {   (to).real = (from).real;	\
             (to).imag = (from).imag;	\
         }
#define  CMPLX_ASSIGN_VALUE(cnum, vReal, vImag)		\
         {   (cnum).real = vReal;	\
             (cnum).imag = vImag;	\
         }
#define  CMPLX_CONJ_ASSIGN(to,from)	\
         {   (to).real = (from).real;	\
             (to).imag = -(from).imag;	\
         }
#define  CMPLX_NEGATE_ASSIGN(to,from)	\
         {   (to).real = -(from).real;	\
             (to).imag = -(from).imag;	\
         }
#define  CMPLX_CONJ_NEGATE_ASSIGN(to,from)	\
         {   (to).real = -(from).real;		\
             (to).imag = (from).imag;		\
         }
#define  CONJUGATE(a)	(a).imag = -(a).imag

/* Macro function that returns the approx magnitude (L-1 norm) of a complex 
 * number. */
#define  CMPLX_1_NORM(a)	(ABS((a).real) + ABS((a).imag))

/* Macro function that returns the approx magnitude (L-infinity norm) of a 
 * complex number. */
#define  CMPLX_INF_NORM(a)	(MAX (ABS((a).real),ABS((a).imag)))

/* Macro function that returns the magnitude (L-2 norm) of a complex number. */
#define  CMPLX_2_NORM(a)	(sqrt((a).real*(a).real + (a).imag*(a).imag))

/* Macro function that performs complex addition. */
#define  CMPLX_ADD(to,from_a,from_b)			\
         {   (to).real = (from_a).real + (from_b).real;	\
             (to).imag = (from_a).imag + (from_b).imag;	\
         }

/* Macro function that performs complex subtraction. */
#define  CMPLX_SUBT(to,from_a,from_b)			\
         {   (to).real = (from_a).real - (from_b).real;	\
             (to).imag = (from_a).imag - (from_b).imag;	\
         }

/* Macro function that is equivalent to += operator for complex numbers. */
#define  CMPLX_ADD_ASSIGN(to,from)	\
         {   (to).real += (from).real;	\
             (to).imag += (from).imag;	\
         }

/* Macro function that is equivalent to -= operator for complex numbers. */
#define  CMPLX_SUBT_ASSIGN(to,from)	\
         {   (to).real -= (from).real;	\
             (to).imag -= (from).imag;	\
         }

/* Macro function that multiplies two complex numbers. */
#define  CMPLX_MULT(to,from_a,from_b)				\
         {   (to).real = (from_a).real * (from_b).real -	\
                         (from_a).imag * (from_b).imag;		\
             (to).imag = (from_a).real * (from_b).imag +	\
                         (from_a).imag * (from_b).real;		\
         }

/* Macro function that multiplies two complex numbers, the first of which is 
 * conjugated. */
#define  CMPLX_CONJ_MULT(to,from_a,from_b)			\
         {   (to).real = (from_a).real * (from_b).real +	\
                         (from_a).imag * (from_b).imag;		\
             (to).imag = (from_a).real * (from_b).imag -	\
                         (from_a).imag * (from_b).real;		\
         }

/* Macro function that multiplies two complex numbers and then adds them
 * to another. to = add + mult_a * mult_b */
#define  CMPLX_MULT_ADD(to,mult_a,mult_b,add)				\
         {   (to).real = (mult_a).real * (mult_b).real -		\
			 (mult_a).imag * (mult_b).imag + (add).real;	\
             (to).imag = (mult_a).real * (mult_b).imag +		\
			 (mult_a).imag * (mult_b).real + (add).imag;	\
         }

/* Macro function that multiplies two complex numbers and then adds them
 * to another. to = add + mult_a* * mult_b where mult_a* repersents mult_a
 * conjugate. */
#define  CMPLX_CONJ_MULT_ADD(to,mult_a,mult_b,add)			\
         {   (to).real = (mult_a).real * (mult_b).real +		\
			 (mult_a).imag * (mult_b).imag + (add).real;	\
             (to).imag = (mult_a).real * (mult_b).imag -		\
			 (mult_a).imag * (mult_b).real + (add).imag;	\
         }

/* Macro function that multiplies two complex numbers and then adds them
 * to another. to += mult_a * mult_b */
#define  CMPLX_MULT_ADD_ASSIGN(to,from_a,from_b)		\
         {   (to).real += (from_a).real * (from_b).real -	\
			  (from_a).imag * (from_b).imag;	\
             (to).imag += (from_a).real * (from_b).imag +	\
			  (from_a).imag * (from_b).real;	\
         }

/* Macro function that multiplies two complex numbers and then subtracts them
 * from another. */
#define  CMPLX_MULT_SUBT_ASSIGN(to,from_a,from_b)		\
         {   (to).real -= (from_a).real * (from_b).real -	\
			  (from_a).imag * (from_b).imag;	\
             (to).imag -= (from_a).real * (from_b).imag +	\
			  (from_a).imag * (from_b).real;	\
         }

/* Macro function that multiplies two complex numbers and then adds them
 * to the destination. to += from_a* * from_b where from_a* repersents from_a
 * conjugate. */
#define  CMPLX_CONJ_MULT_ADD_ASSIGN(to,from_a,from_b)		\
         {   (to).real += (from_a).real * (from_b).real +	\
			  (from_a).imag * (from_b).imag;	\
             (to).imag += (from_a).real * (from_b).imag -	\
			  (from_a).imag * (from_b).real;	\
         }

/* Macro function that multiplies two complex numbers and then subtracts them
 * from the destination. to -= from_a* * from_b where from_a* repersents from_a
 * conjugate. */
#define  CMPLX_CONJ_MULT_SUBT_ASSIGN(to,from_a,from_b)		\
         {   (to).real -= (from_a).real * (from_b).real +	\
			  (from_a).imag * (from_b).imag;	\
             (to).imag -= (from_a).real * (from_b).imag -	\
			  (from_a).imag * (from_b).real;	\
         }

/* Macro function that divides two complex numbers. */
#define  CMPLX_DIV(to,num,den)					\
         {   SPARSE_real  r, s;					\
             if (ABS((den).real) > ABS((den).imag))		\
             {  r = (den).imag / (den).real;			\
                s = (den).real + r*(den).imag;			\
                (to).real = ((num).real + r*(num).imag)/s;	\
                (to).imag = ((num).imag - r*(num).real)/s;	\
             }							\
             else						\
             {  r = (den).real / (den).imag;			\
                s = (den).imag + r*(den).real;			\
                (to).real = (r*(num).real + (num).imag)/s;	\
                (to).imag = (r*(num).imag - (num).real)/s;	\
             }							\
         }

#define  CMPLX_DIV_ASSIGN(num,den)				\
         {   SPARSE_real  r, s, t;				\
             if (ABS((den).real) > ABS((den).imag))		\
             {  r = (den).imag / (den).real;			\
                s = (den).real + r*(den).imag;			\
                t = ((num).real + r*(num).imag)/s;		\
                (num).imag = ((num).imag - r*(num).real)/s;	\
                (num).real = t;					\
             }							\
             else						\
             {  r = (den).real / (den).imag;			\
                s = (den).imag + r*(den).real;			\
                t = (r*(num).real + (num).imag)/s;		\
                (num).imag = (r*(num).imag - (num).real)/s;	\
                (num).real = t;					\
             }							\
         }

#define  CMPLX_RECIPROCAL(to,den)				\
         {   SPARSE_real  r;					\
             if (ABS((den).real) > ABS((den).imag))		\
             {  r = (den).imag / (den).real;			\
                (to).imag = r*((to).real = 1.0/((den).real + r*(den).imag)); \
             }							\
             else						\
             {  r = (den).real / (den).imag;			\
                (to).real = r*((to).imag = 1.0/((den).imag + r*(den).real)); \
             }							\
         }
#endif /* NUMCMPLX_H */
