/*************************************************************************
 * vt_levenberg.h -
 *
 * $Id: levenberg.h,v 1.5 2000/07/12 17:54:38 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Mar 22 09:23:04 MET 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#ifndef _levenberg_h_
#define _levenberg_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <math.h>
#include <string.h>

#include <typedefs.h>
#include <systlin.h>


extern void setVerboseInLevenberg ( int v );
extern void incrementVerboseInLevenberg(  );
extern void decrementVerboseInLevenberg(  );


extern void Levenberg_setLambdaMax( double d );
extern void Levenberg_setIncreaseMin( double d );




typedef double (*typeFuncEval)( double, double*, double* );	


extern double _LassenFunction( double x, double *thePar, double *derPar );
extern double _GaussianForLM ( double x, double *thePar, double *derPar );

extern double _RayleighCenteredForLM ( double x, double *thePar, double *derPar );
extern double _RayleighForLM ( double x, double *thePar, double *derPar );
extern double _RayleighPosForLM ( double x, double *thePar, double *derPar );
extern double _XnForLM ( double x, double *thePar, double *derPar );

extern double _MixtureOf1RayleighCentered1XnForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Rayleigh1XnForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1RayleighPos1XnForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Xn1RayleighCenteredForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Xn1RayleighForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Xn1RayleighPosForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Gaussian1RayleighCenteredForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Gaussian1RayleighForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Gaussian1RayleighPosForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1RayleighCentered1GaussianForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Rayleigh1GaussianForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1RayleighPos1GaussianForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Gaussian1XnForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Xn1GaussianForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1RayleighCentered1RayleighForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1RayleighPos1RayleighForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1RayleighPos1RayleighCenteredForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Rayleigh1RayleighCenteredForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1Rayleigh1RayleighPosForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf1RayleighCentered1RayleighPosForLM ( double x, double *thePar,
                                          double *derPar );

extern double _MixtureOf2RayleighsCenteredForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf2RayleighsForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf2RayleighsPosForLM ( double x, double *thePar,
                                          double *derPar );
extern double _MixtureOf2XnsForLM ( double x, double *thePar,
                                          double *derPar );


extern double _MixtureOfRayleighsCenteredForLM ( double x, double *thePar,
                                          double *derPar, int nbRayleighs );
extern double _MixtureOfRayleighsForLM ( double x, double *thePar,
                                          double *derPar, int nbRayleighs );
extern double _MixtureOfRayleighsPosForLM ( double x, double *thePar,
                                          double *derPar, int nbRayleighs );
extern double _MixtureOfXnsForLM ( double x, double *thePar,
                                          double *derPar, int nbXns );


extern double _MixtureOf2GaussiansForLM ( double x, double *thePar, 
					  double *derPar );
extern double _MixtureOf3GaussiansForLM ( double x, double *thePar, 
					  double *derPar );
extern double _MixtureOf4GaussiansForLM ( double x, double *thePar, 
					  double *derPar );
extern double _MixtureOf5GaussiansForLM ( double x, double *thePar, 
					  double *derPar );
extern double _MixtureOf6GaussiansForLM ( double x, double *thePar, 
					  double *derPar );
extern double _MixtureOf7GaussiansForLM ( double x, double *thePar, 
					  double *derPar );

extern double _MixtureOfGaussiansForLM ( double x, double *thePar, 
					 double *derPar, int nbGaussians );
extern double _NonSymetricGaussianForLM ( double x, double *thePar, 
					  double *derPar );
  


extern int Modeling1DDataWithLevenberg( void *theX, bufferType xType,
					void *theY, bufferType yType,
					void *theW, bufferType wType,
					void *theS, bufferType sType,
					int theLength,
					double *theParams, int nbParams,
					typeFuncEval funcEval );







#ifdef __cplusplus
}
#endif

#endif /* _levenberg_h_  */
