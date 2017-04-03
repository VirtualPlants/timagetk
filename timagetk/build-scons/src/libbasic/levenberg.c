/*************************************************************************
 * levenberg.c -
 *
 * $Id: levenberg.c,v 1.6 2000/07/12 17:54:38 greg Exp $
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

#include <levenberg.h>

#define EPS 1e-8

static int _verbose_ = 0;
static double _lambda_max_ = 1e+14;
static double _increase_min_ = 1e-8;


void setVerboseInLevenberg ( int v )
{
  _verbose_ = v;
}

void incrementVerboseInLevenberg(  )
{
  _verbose_ ++;
}

void decrementVerboseInLevenberg(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





void Levenberg_setLambdaMax( double d ) 
{
  if ( d < 1e+10 ) {
    _lambda_max_ = 1e+10;
  } else {
    _lambda_max_ = d;
  }
}

void Levenberg_setIncreaseMin( double d ) 
{
  if ( d < 0 || d > 1e-4 ) {
    _increase_min_ = 1e-4;
  } else {
    _increase_min_ = d;
  }
}







/* y = thePar[0] * x / (thePar[1] + x)
   x -> xenon
   y -> hmpao
*/
double _LassenFunction( double x, double *thePar, double *derPar )
{
  if ( thePar[1] + x == 0 ) {
    derPar[0] = derPar[1] = 0.0;
    return( 0.0 );
  }
  derPar[0] = x / (thePar[1] + x);
  derPar[1] = - thePar[0] * derPar[0] / (thePar[1] + x);
  return( thePar[0] * derPar[0] );
}














/* amplitude = thePar[0]
   moyenne = thePar[1]
   sigma = thePar[2]

   gaussienne = thePar[0] * exp ( - (x-thePar[1])*(x-thePar[1])/(2*thePar[2]*thePar[2]) )
*/ 
double _GaussianForLM ( double x, double *thePar, double *derPar )
{
  double a, e;
  
  if ( thePar[2] == 0.0 ) {
    derPar[0] = derPar[1] = derPar[2] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])/thePar[2];
  e = exp ( - a*a/2.0 );
  
  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[2];
  derPar[2] = thePar[0] * e * a * a / thePar[2];
  
  return( thePar[0] * e );
}




double _RayleighCenteredForLM ( double x, double *thePar, double *derPar )
{
  double a, b, e;

  if ( thePar[1] == 0.0 ) {
    derPar[0] = derPar[1] = 0.0;
    return( 0.0 );
  }

  a = x*x/(thePar[1]*thePar[1]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[1];

  return( thePar[0] * e );
}

double _RayleighForLM ( double x, double *thePar, double *derPar )
{
  double a, b, e;

  if ( thePar[2] == 0.0 ) {
    derPar[0] = derPar[1] = derPar[2] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
  b = exp (-a*0.5);
  e = b*(x-thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0]*b*(a-1);
  derPar[2] = thePar[0] * e * a / thePar[2];

  return( thePar[0] * e );
}

double _RayleighPosForLM ( double x, double *thePar, double *derPar )
{
  double a, b, e;

  if ( thePar[2] == 0.0 || x-thePar[1] < 0) {
    derPar[0] = derPar[1] = derPar[2] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
  b = exp (-a*0.5);
  e = b*(x-thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0]*b*(a-1);
  derPar[2] = thePar[0] * e * a / thePar[2];

  return( thePar[0] * e );
}



double _XnForLM ( double x, double *thePar, double *derPar )
{
  double e;

  if ( thePar[1] == 0.0 ) {
    derPar[0] = derPar[1] = 0.0;
    return( 0.0 );
  }

  e = pow(x,thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0] * e * log(x);

  return( thePar[0] * e );
}

double _MixtureOf1RayleighCentered1XnForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[1] == 0.0 || thePar[3] == 0.0) {
    for (i=0 ; i<4 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = x*x/(thePar[1]*thePar[1]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[1];

  r += thePar[0] * e;

  e = pow(x,thePar[3]);

  derPar[2] = e;
  derPar[3] = thePar[2] * e * log(x);

  r += thePar[2] * e;

  return( r );
}

double _MixtureOf1Rayleigh1XnForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
  b = exp (-a*0.5);
  e = b*(x-thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0]*b*(a-1);
  derPar[2] = thePar[0] * e * a / thePar[2];

  r += thePar[0] * e;

  e = pow(x,thePar[4]);

  derPar[3] = e;
  derPar[4] = thePar[3] * e * log(x);

  r += thePar[3] * e;

  return( r );
}

double _MixtureOf1RayleighPos1XnForLM ( double x, double *thePar,
                        double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  if ( x-thePar[1] < 0.0 ) {
    for (i=0 ; i<3 ; i++) derPar[i] = 0.0;
    r+=0.0;
  }
  else
  {
    a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
    b = exp (-a*0.5);
    e = b*(x-thePar[1]);

    derPar[0] = e;
    derPar[1] = thePar[0]*b*(a-1);
    derPar[2] = thePar[0] * e * a / thePar[2];
    r += thePar[0] * e;
  }

  e = pow(x,thePar[4]);

  derPar[3] = e;
  derPar[4] = thePar[3] * e * log(x);

  r += thePar[3] * e;

  return( r );

}


double _MixtureOf1Xn1RayleighCenteredForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[1] == 0.0 || thePar[3] == 0.0) {
    for (i=0 ; i<4 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  e = pow(x,thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0] * e * log(x);

  r += thePar[0] * e;

  a = x*x/(thePar[3]*thePar[3]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[2] = e;
  derPar[3] = thePar[2] * e * a / thePar[3];


  r += thePar[2] * e;

  return( r );
}

double _MixtureOf1Xn1RayleighForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[1] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  e = pow(x,thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0] * e * log(x);

  r += thePar[0] * e;

  a = (x-thePar[3])*(x-thePar[3])/(thePar[4]*thePar[4]);
  b = exp (-a*0.5);
  e = b*(x-thePar[3]);

  derPar[2] = e;
  derPar[3] = thePar[2]*b*(a-1);
  derPar[4] = thePar[2] * e * a / thePar[4];


  r += thePar[2] * e;

  return( r );
}

double _MixtureOf1Xn1RayleighPosForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[1] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  e = pow(x,thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0] * e * log(x);

  r += thePar[0] * e;

  if (x-thePar[3] < 0.0) {
    for (i=0 ; i<3 ; i++) derPar[i+2] = 0.0;
    r += 0.0;
  }
  else {
    a = (x-thePar[3])*(x-thePar[3])/(thePar[4]*thePar[4]);
    b = exp (-a*0.5);
    e = b*(x-thePar[3]);

    derPar[2] = e;
    derPar[3] = thePar[2]*b*(a-1);
    derPar[4] = thePar[2] * e * a / thePar[4];

    r += thePar[2] * e;
  }

  return( r );


}

double _MixtureOf1Gaussian1RayleighCenteredForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])/thePar[2];
  e = exp ( - a*a/2.0 );

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[2];
  derPar[2] = thePar[0] * e * a * a / thePar[2];

  r += thePar[0] * e;

  a = x*x/(thePar[4]*thePar[4]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[3] = e;
  derPar[4] = thePar[3] * e * a / thePar[4];


  r += thePar[3] * e;

  return( r );
}

double _MixtureOf1Gaussian1RayleighForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[5] == 0.0) {
    for (i=0 ; i<6 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])/thePar[2];
  e = exp ( - a*a/2.0 );

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[2];
  derPar[2] = thePar[0] * e * a * a / thePar[2];

  r += thePar[0] * e;

  a = (x-thePar[4])*(x-thePar[4])/(thePar[5]*thePar[5]);
  b = exp (-a*0.5);
  e = b*(x-thePar[4]);

  derPar[3] = e;
  derPar[4] = thePar[3]*b*(a-1);
  derPar[5] = thePar[3] * e * a / thePar[5];


  r += thePar[3] * e;

  return( r );
}

double _MixtureOf1Gaussian1RayleighPosForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[5] == 0.0) {
    for (i=0 ; i<6 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])/thePar[2];
  e = exp ( - a*a/2.0 );

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[2];
  derPar[2] = thePar[0] * e * a * a / thePar[2];

  r += thePar[0] * e;

  if (x-thePar[3] < 0.0) {
    for (i=0 ; i<3 ; i++) derPar[i+3] = 0.0;
    r += 0.0;
  }
  else {
    a = (x-thePar[4])*(x-thePar[4])/(thePar[5]*thePar[5]);
    b = exp (-a*0.5);
    e = b*(x-thePar[4]);

    derPar[3] = e;
    derPar[4] = thePar[3]*b*(a-1);
    derPar[5] = thePar[3] * e * a / thePar[5];

    r += thePar[3] * e;
  }

  return( r );
}

double _MixtureOf1RayleighCentered1GaussianForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[1] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = x*x/(thePar[1]*thePar[1]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[1];


  r += thePar[0] * e;

  a = (x-thePar[3])/thePar[4];
  e = exp ( - a*a*0.5 );

  derPar[2] = e;
  derPar[3] = thePar[2] * e * a / thePar[4];
  derPar[4] = thePar[2] * e * a * a / thePar[4];

  r += thePar[2] * e;

  return( r );
}

double _MixtureOf1Rayleigh1GaussianForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[5] == 0.0) {
    for (i=0 ; i<6 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
  b = exp (-a*0.5);
  e = b*(x-thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0]*b*(a-1);
  derPar[2] = thePar[0] * e * a / thePar[2];


  r += thePar[0] * e;

  a = (x-thePar[4])/thePar[5];
  e = exp ( - a*a*0.5 );

  derPar[3] = e;
  derPar[4] = thePar[3] * e * a / thePar[5];
  derPar[5] = thePar[3] * e * a * a / thePar[5];

  r += thePar[3] * e;

  return( r );
}

double _MixtureOf1RayleighPos1GaussianForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[5] == 0.0) {
    for (i=0 ; i<6 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  if ( x-thePar[1] < 0.0 ) {
    for (i=0 ; i<3 ; i++) derPar[i] = 0.0;
    r += 0.0 ;
  }
  else {
    a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
    b = exp (-a*0.5);
    e = b*(x-thePar[1]);

    derPar[0] = e;
    derPar[1] = thePar[0]*b*(a-1);
    derPar[2] = thePar[0] * e * a / thePar[2];

    r += thePar[0] * e;
  }

  a = (x-thePar[4])/thePar[5];
  e = exp ( - a*a*0.5 );

  derPar[3] = e;
  derPar[4] = thePar[3] * e * a / thePar[5];
  derPar[5] = thePar[3] * e * a * a / thePar[5];

  r += thePar[3] * e;

  return( r );
}

double _MixtureOf1Gaussian1XnForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])/thePar[2];
  e = exp ( - a*a/2.0 );

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[2];
  derPar[2] = thePar[0] * e * a * a / thePar[2];

  r += thePar[0] * e;

  e = pow(x,thePar[4]);

  derPar[3] = e;
  derPar[4] = thePar[3] * e * log(x);

  r += thePar[3] * e;

  return( r );
}

double _MixtureOf1Xn1GaussianForLM ( double x, double *thePar,
                                          double *derPar )
{
  double a, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  e = pow(x,thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0] * e * log(x);

  r += thePar[0] * e;

  a = (x-thePar[3])/thePar[4];
  e = exp ( - a*a/2.0 );

  derPar[2] = e;
  derPar[3] = thePar[2] * e * a / thePar[4];
  derPar[4] = thePar[2] * e * a * a / thePar[4];

  r += thePar[2] * e;

  return( r );
}

double _MixtureOf1RayleighCentered1RayleighForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[1] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = x*x/(thePar[1]*thePar[1]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[1];

  r += thePar[0] * e;

  a = (x-thePar[3])*(x-thePar[3])/(thePar[4]*thePar[4]);
  b = exp (-a*0.5);
  e = b*(x-thePar[3]);

  derPar[2] = e;
  derPar[3] = thePar[2]*b*(a-1);
  derPar[4] = thePar[2] * e * a / thePar[4];


  r += thePar[2] * e;

  return( r );
}

double _MixtureOf1RayleighPos1RayleighForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[5] == 0.0) {
    for (i=0 ; i<6 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  if ( x-thePar[1] < 0.0 ) {
    for (i=0 ; i<3 ; i++) derPar[i] = 0.0;
    r += 0.0 ;
  }
  else {
    a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
    b = exp (-a*0.5);
    e = b*(x-thePar[1]);

    derPar[0] = e;
    derPar[1] = thePar[0]*b*(a-1);
    derPar[2] = thePar[0] * e * a / thePar[2];

    r += thePar[0] * e;
  }

  a = (x-thePar[4])*(x-thePar[4])/(thePar[5]*thePar[5]);
  b = exp (-a*0.5);
  e = b*(x-thePar[4]);

  derPar[3] = e;
  derPar[4] = thePar[3]*b*(a-1);
  derPar[5] = thePar[3] * e * a / thePar[5];


  r += thePar[3] * e;

  return( r );
}

double _MixtureOf1RayleighPos1RayleighCenteredForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  if ( x-thePar[1] < 0.0 ) {
    for (i=0 ; i<3 ; i++) derPar[i] = 0.0;
    r += 0.0 ;
  }
  else {
    a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
    b = exp (-a*0.5);
    e = b*(x-thePar[1]);

    derPar[0] = e;
    derPar[1] = thePar[0]*b*(a-1);
    derPar[2] = thePar[0] * e * a / thePar[2];

    r += thePar[0] * e;
  }

  a = x*x/(thePar[4]*thePar[4]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[3] = e;
  derPar[4] = thePar[3] * e * a / thePar[4];


  r += thePar[3] * e;

  return( r );
}


double _MixtureOf1RayleighCentered1RayleighPosForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[1] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = x*x/(thePar[1]*thePar[1]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[1];


  r += thePar[0] * e;

  if (x-thePar[2] < 0.0) {
    for (i=0 ; i<3 ; i++) derPar[i+2] = 0.0;
    r += 0.0;
  }
  else {
    a = (x-thePar[3])*(x-thePar[3])/(thePar[4]*thePar[4]);
    b = exp (-a*0.5);
    e = b*(x-thePar[3]);

    derPar[2] = e;
    derPar[3] = thePar[2]*b*(a-1);
    derPar[4] = thePar[2] * e * a / thePar[4];

    r += thePar[2] * e;
  }

  return( r );
}


double _MixtureOf1Rayleigh1RayleighPosForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[5] == 0.0) {
    for (i=0 ; i<6 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
  b = exp (-a*0.5);
  e = b*(x-thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0]*b*(a-1);
  derPar[2] = thePar[0] * e * a / thePar[2];


  r += thePar[0] * e;

  if (x-thePar[3] < 0.0) {
    for (i=0 ; i<3 ; i++) derPar[i+3] = 0.0;
    r += 0.0;
  }
  else {
    a = (x-thePar[4])*(x-thePar[4])/(thePar[5]*thePar[5]);
    b = exp (-a*0.5);
    e = b*(x-thePar[4]);

    derPar[3] = e;
    derPar[4] = thePar[3]*b*(a-1);
    derPar[5] = thePar[3] * e * a / thePar[5];

    r += thePar[3] * e;
  }

  return( r );
}

double _MixtureOf1Rayleigh1RayleighCenteredForLM ( double x, double *thePar,
    double *derPar )
{
  double a, b, e, r=0.0;
  int i;

  if ( thePar[2] == 0.0 || thePar[4] == 0.0) {
    for (i=0 ; i<5 ; i++) derPar[i] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])*(x-thePar[1])/(thePar[2]*thePar[2]);
  b = exp (-a*0.5);
  e = b*(x-thePar[1]);

  derPar[0] = e;
  derPar[1] = thePar[0]*b*(a-1);
  derPar[2] = thePar[0] * e * a / thePar[2];


  r += thePar[0] * e;

  a = x*x/(thePar[4]*thePar[4]);
  b = exp (-a*0.5);
  e = b*x;

  derPar[3] = e;
  derPar[4] = thePar[3] * e * a / thePar[4];

  r += thePar[3] * e;

  return( r );
}


double _MixtureOf2RayleighsCenteredForLM ( double x, double *thePar,
                                          double *derPar )
{
  return( _MixtureOfRayleighsCenteredForLM( x, thePar, derPar, 2 ) );
}

double _MixtureOf2RayleighsForLM ( double x, double *thePar,
                                          double *derPar )
{
  return( _MixtureOfRayleighsForLM( x, thePar, derPar, 2 ) );
}

double _MixtureOf2XnsForLM ( double x, double *thePar,
                                          double *derPar )
{
  return( _MixtureOfXnsForLM( x, thePar, derPar, 2 ) );
}

double _MixtureOf2RayleighsPosForLM ( double x, double *thePar,
    double *derPar )
{
  return( _MixtureOfRayleighsPosForLM( x, thePar, derPar, 2 ) );
}


/* Pour chaque rayleigh i ( 0<=i<nbRayleigh )

   amplitude[i] = thePar[i*2]
   sigma[i]     = thePar[i*2+1]

   rayleigh[i] = thePar[i*2] * thePar[i*2+1] * exp ( - x*x / (2*thePar[i*2+1]*thePar[i*2+1]) )
*/

double _MixtureOfRayleighsCenteredForLM ( double x, double *thePar,
                                  double *derPar, int nbRayleighsC)
{
  int i, j;
  double a, b, e;
  double r=0.0;

  for ( i=0; i<nbRayleighsC; i++ ) {
    if ( thePar[i*2+2] == 0.0 ) {
      for ( j=0; j<2*nbRayleighsC; j++ ) derPar[j] = 0.0;
      return( 0.0 );
    }
  }

  for ( i=0; i<nbRayleighsC; i++ ) {

      a = x*x/(thePar[2*i+1]*thePar[2*i+1]);
      b = exp (-a*0.5);
      e = b*x;

      derPar[2*i  ] = e;
      derPar[2*i+1] = thePar[2*i] * e * a / thePar[2*i+1];




      r += thePar[2*i] * e;
  }
  return( r );
}

double _MixtureOfRayleighsForLM ( double x, double *thePar,
                                  double *derPar, int nbRayleighs)
{
  int i, j;
  double a, b, e;
  double r=0.0;

  for ( i=0; i<nbRayleighs; i++ ) {
    if ( thePar[i*3+2] == 0.0 ) {
      for ( j=0; j<3*nbRayleighs; j++ ) derPar[j] = 0.0;
      return( 0.0 );
    }
  }

  for ( i=0; i<nbRayleighs; i++ ) {

      a = (x-thePar[3*i+1])*(x-thePar[3*i+1])/(thePar[3*i+2]*thePar[3*i+2]);
      b = exp (-a*0.5);
      e = b*(x-thePar[3*i+1]);

      derPar[3*i  ] = e;
      derPar[3*i+1] = thePar[3*i]*b*(a-1);
      derPar[3*i+2] = thePar[3*i] * e * a / thePar[3*i+2];




      r += thePar[3*i] * e;
  }
  return( r );
}


double _MixtureOfRayleighsPosForLM ( double x, double *thePar,
    double *derPar, int nbRayleighs )
{
  int i, j;
  double a, b, e;
  double r=0.0;

  for ( i=0; i<nbRayleighs; i++ ) {
    if ( thePar[i*3+2] == 0.0 ) {
      for ( j=0; j<3*nbRayleighs; j++ ) derPar[j] = 0.0;
      return( 0.0 );
    }
  }

  for ( i=0; i<nbRayleighs; i++ ) {

      if(x-thePar[3*i+1] < 0.0) {
        for (j=0;j<3; j++) derPar[3*i+j] = 0.0;
        r += 0.0;
      }
      else {
        a = (x-thePar[3*i+1])*(x-thePar[3*i+1])/(thePar[3*i+2]*thePar[3*i+2]);
        b = exp (-a*0.5);
        e = b*(x-thePar[3*i+1]);

        derPar[3*i  ] = e;
        derPar[3*i+1] = thePar[3*i]*b*(a-1);
        derPar[3*i+2] = thePar[3*i] * e * a / thePar[3*i+2];

        r += thePar[3*i] * e;
      }
  }
  return( r );
}
/* Pour chaque Xn i ( 0<=i<nbXn )

   coef[i] = thePar[i*2]
   n[i]     = thePar[i*2+1]

   Xn[i] = thePar[i*2] * pow( x, thePar[i*2+1] )
*/

double _MixtureOfXnsForLM ( double x, double *thePar,
                                  double *derPar, int nbXns)
{
  int i, j;
  double e;
  double r=0.0;

  for ( i=0; i<nbXns; i++ ) {
    if ( thePar[i*2+1] == 0.0 ) {
      for ( j=0; j<2*nbXns; j++ ) derPar[j] = 0.0;
      return( 0.0 );
    }
  }

  for ( i=0; i<nbXns; i++ ) {
      e = pow(x,thePar[i*2+1]);

      derPar[i*2] = e;
      derPar[i*2+1] = thePar[i*2] * e * log(x);

      r += thePar[i*2] * e;
  }
  return( r );
}





double _MixtureOf2GaussiansForLM ( double x, double *thePar, 
				  double *derPar ) 
{
  return( _MixtureOfGaussiansForLM( x, thePar, derPar, 2 ) );
}
double _MixtureOf3GaussiansForLM ( double x, double *thePar, 
				  double *derPar ) 
{
  return( _MixtureOfGaussiansForLM( x, thePar, derPar, 3 ) );
}
double _MixtureOf4GaussiansForLM ( double x, double *thePar, 
				  double *derPar ) 
{
  return( _MixtureOfGaussiansForLM( x, thePar, derPar, 4 ) );
}
double _MixtureOf5GaussiansForLM ( double x, double *thePar, 
				  double *derPar ) 
{
  return( _MixtureOfGaussiansForLM( x, thePar, derPar, 5 ) );
}
double _MixtureOf6GaussiansForLM ( double x, double *thePar, 
				  double *derPar ) 
{
  return( _MixtureOfGaussiansForLM( x, thePar, derPar, 6 ) );
}
double _MixtureOf7GaussiansForLM ( double x, double *thePar, 
				  double *derPar ) 
{
  return( _MixtureOfGaussiansForLM( x, thePar, derPar, 7 ) );
}






/* Pour chaque gaussienne i ( 0<=i<nbGaussians )

   amplitude[i] = thePar[i*3]
   moyenne[i]   = thePar[i*3+1]
   sigma[i]     = thePar[i*3+2]

   gaussienne[i] = thePar[i*3] * exp ( - (x-thePar[i*3+1])*(x-thePar[i*3+1])
                                       / (2*thePar[i*3+2]*thePar[i*3+2]) )
*/ 

double _MixtureOfGaussiansForLM ( double x, double *thePar, 
                                  double *derPar, int nbGaussians )
{
  int i, j;
  double a, e;
  double r=0.0;

  for ( i=0; i<nbGaussians; i++ ) {
    if ( thePar[i*3+2] == 0.0 ) {
      for ( j=0; j<3*nbGaussians; j++ ) derPar[j] = 0.0;
      return( 0.0 );
    }
  }
  
  for ( i=0; i<nbGaussians; i++ ) {
    a = (x-thePar[i*3+1])/thePar[i*3+2];
    e = exp ( - a*a/2.0 );
  
    derPar[i*3] = e;
    derPar[i*3+1] = thePar[i*3] * e * a / thePar[i*3+2];
    derPar[i*3+2] = thePar[i*3] * e * a * a / thePar[i*3+2];
    
    r += thePar[i*3] * e;
  }
  
  return( r );
}










/* amplitude = thePar[0]
   moyenne = thePar[1]
   sigma = thePar[2]

   gaussienne = thePar[0] * exp ( - (x-thePar[1])*(x-thePar[1])/(2*thePar[2]*thePar[2]) )
*/ 
double _NonSymetricGaussianForLM ( double x, double *thePar, double *derPar )
{
  double a, e;


  if ( x < thePar[1] ) {

    if ( thePar[2] == 0.0 ) {
      derPar[0] = derPar[1] = derPar[2] = derPar[3] = 0.0;
      return( 0.0 );
    }

    a = (x-thePar[1])/thePar[2];
    e = exp ( - a*a/2.0 );
  
    derPar[0] = e;
    derPar[1] = thePar[0] * e * a / thePar[2];
    derPar[2] = thePar[0] * e * a * a / thePar[2];
    derPar[3] = 0.0;

    return( thePar[0] * e );
  }
  

  if ( thePar[3] == 0.0 ) {
    derPar[0] = derPar[1] = derPar[2] = derPar[3] = 0.0;
    return( 0.0 );
  }

  a = (x-thePar[1])/thePar[3];
  e = exp ( - a*a/2.0 );
  
  derPar[0] = e;
  derPar[1] = thePar[0] * e * a / thePar[3];
  derPar[2] = 0.0;
  derPar[3] = thePar[0] * e * a * a / thePar[3];
  
  return( thePar[0] * e );
}














/* on evalue le chi2 
   = sum(i=0..length) w[i] ( ((y[i] - f(x[i]))/s[i])^2 )
*/

static double _ComputeChi2AndDerivatives( void *theX, bufferType xType,
					  void *theY, bufferType yType,
					  void *theW, bufferType wType,
					  void *theS, bufferType sType,
					  int length,
					  double *theParams,
					  double *derParams,
					  int nbParams,
					  typeFuncEval funcEval,
					  double *alpha,
					  double *beta )
{
  char *proc = "_ComputeChi2AndDerivatives";
  double chi = 0.0;
  double x, y, dy, c;
  int i, j, k;
  
  if ( nbParams <= 0 ) {
    return( 0 );
  }

  for ( i=0; i<nbParams*nbParams; i++ ) alpha[i] = 0.0;
  for ( i=0; i<nbParams; i++ ) beta[i] = 0.0;
  
  if ( xType != yType || xType != wType || xType != sType ) {
    if ( 1 || _verbose_ )
      fprintf( stderr, "%s: unable to handle different types\n", proc );
    return( 0.0 );
  }
  
  switch( xType ) {
  case DOUBLE :
    {
      double *bufX = (double*)theX;
      double *bufY = (double*)theY;
      double *bufW = (double*)theW;
      double *bufS = (double*)theS;
      for ( i = 0; i < length; i++ ) {

	if ( bufS[i] == 0.0 ) continue;
	x = bufX[i];
	c = bufW[i] / (bufS[i]*bufS[i]);
	y = (*funcEval)( x, theParams, derParams );
	
	
	dy = bufY[i] - y;

	if ( 0 )
	  fprintf( stderr, "at x[%3d]=%f : chi + c *dy *dy = %f + %f * (%f -%f)^2 =",
		   i, x, chi, c, bufY[i], y );


	for ( j = 0; j < nbParams; j++ ) {
	  beta[j] += c * dy * derParams[j];
	  for ( k = 0; k < nbParams; k++ ) {
	    alpha[j*nbParams+k] += c * derParams[j] * derParams[k];
	  }
	}
	
	chi += c * dy * dy;

	if ( 0 )
	  fprintf( stderr, "%f\n", chi );
	
      }
    }
    break;
  default :
    if ( 1 || _verbose_ )
      fprintf( stderr, "%s: unable to handle such type\n", proc );
    return( 0.0 );
  }
  return( chi );
}
















int Modeling1DDataWithLevenberg( void *theX, bufferType xType,
				 void *theY, bufferType yType,
				 void *theW, bufferType wType,
				 void *theS, bufferType sType,
				 int theLength,
				 double *theParams, int nbParams,
				 typeFuncEval funcEval
				 )
{
  char *proc = "Modeling1DDataWithLevenberg";
  double *matAlpha  = (double*)NULL;
  double *matPrime  = (double*)NULL;
  double *derParams = (double*)NULL;
  double *vecBeta   = (double*)NULL;
  double *incParams = (double*)NULL;
  double *newParams = (double*)NULL;
  double *theAllocatedBuffer = (double*)NULL;
  
  double lambda = 0.001;
  double chi2, newchi2;
  int stop = 0;
  int i;

  if ( nbParams <= 0 ) {
    return( 0 );
  }

  /* il faut 2 matrices nbParams * nbParams
             4 vecteurs nbParams
  */
  theAllocatedBuffer = (double*)malloc( (2*(nbParams*nbParams)+4*nbParams)*sizeof(double) );
  if ( theAllocatedBuffer == (double*)NULL ) {
    fprintf( stderr, "%s: unable to allocate buffer\n", proc );
    return( 0 );
  }
  matPrime = matAlpha = theAllocatedBuffer;
  matPrime += nbParams*nbParams;
  derParams = matPrime;
  derParams += nbParams*nbParams;
  vecBeta  = derParams;
  vecBeta  += nbParams;
  incParams = vecBeta;
  incParams += nbParams;
  newParams = incParams;
  newParams += nbParams;
 



  /* calcul du chi 2 initial et des derivees
   */
  chi2 = _ComputeChi2AndDerivatives( theX, xType, theY, yType, theW, wType, theS, sType, 
				     theLength, theParams, derParams, nbParams,
				     funcEval, matAlpha, vecBeta );
  


  
  do {
    /* on construit A' = A + Lambda Diag (A)
     */
    (void)memcpy( matPrime, matAlpha, nbParams*nbParams*sizeof(double) );
    for ( i = 0; i < nbParams; i++ )
      matPrime[ i*nbParams+i ] += lambda * matAlpha[ i*nbParams+i ];



    if ( _verbose_ >= 1 ) {
      int a,b;
      printf( "\n" );
      printf( " ...... [a] = {");
      for (a=0;a<nbParams;a++) printf( " %f", theParams[a] );
      printf( " }\n" );
      printf( " ...... [BETA] = {");
      for (a=0;a<nbParams;a++) printf( " %f", vecBeta[a] );
      printf( " }\n" );
      printf( " ...... [ALPHA] = [");
      for (b=0;b<nbParams;b++) {
	if ( b > 0 ) printf( "                  [");
	for (a=0;a<nbParams;a++) printf( " %f", matAlpha[a+b*nbParams] );
	printf( " ]\n" );
      }

      printf( " ...... [ALPHA'] = [");
      for (b=0;b<nbParams;b++) {
	if ( b > 0 ) printf( "                   [");
	for (a=0;a<nbParams;a++) printf( " %f", matPrime[a+b*nbParams] );
	printf( " ]\n" );
      }
    }




    /* on resoud A' * incParams = Beta
     */
    if ( _SolveLinearSystem( matPrime, vecBeta, incParams, nbParams ) != 1 ) {
      free( theAllocatedBuffer );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to solve linear system\n", proc );
      return( 0 );
    }
    




    if ( _verbose_ >= 1 ) {
      int a;

      printf( " ...... [da] = {");
      for (a=0;a<nbParams;a++) printf( " %f", incParams[a] );
      printf( " }\n" );
      
    }




    /* nouveaux parametres
     */
    for ( i = 0; i < nbParams; i++ )
      newParams[i] = theParams[i] + incParams[i];
    
    newchi2 = _ComputeChi2AndDerivatives( theX, xType, theY, yType, 
					  theW, wType, theS, sType, 
					  theLength, 
					  newParams, derParams, nbParams,
					  funcEval, matPrime, incParams );
    if ( newchi2 >= chi2 ) {

      lambda *= 10;
      if ( _verbose_ ) {
	printf( "... Chi2DChi = %f > Chi2 = %f", newchi2, chi2 );
	printf( "   =>   lambda *= 10 -> %f \n", lambda );
      }
      if ( lambda > _lambda_max_ ) stop = 1;

    } else {

      lambda /= 10;
      if ( _verbose_ ) {	
	printf( "... Chi2DChi = %f < Chi2 = %f", newchi2, chi2 );
	printf( "   =>   lambda /= 10 -> %f \n", lambda );
      }
      
      (void)memcpy( theParams, newParams, nbParams*sizeof(double) );
      (void)memcpy( matAlpha, matPrime, nbParams*nbParams*sizeof(double) );
      (void)memcpy( vecBeta, incParams, nbParams*sizeof(double) );

      if ( (chi2-newchi2)/chi2 < _increase_min_ ) stop = 1;

      chi2 = newchi2;

    }
  } while ( stop == 0 );
  




  free( theAllocatedBuffer );
  return( 1 );
}









