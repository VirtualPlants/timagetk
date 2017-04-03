/*************************************************************************
 * recline.c - tools for recursive filtering of 1D lines
 *
 * $Id: recline.c,v 1.5 2003/06/25 07:48:54 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * DESCRIPTION: 
 *
 * Recursive filtering of a line (a 1D array)
 * Filter coefficient are static variables.
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * June, 9 1998
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#include <recline.h>

static int _verbose_ = 0;

void setVerboseInRecLine( int v )
{
  _verbose_ = v;
}

void incrementVerboseInRecLine(  )
{
  _verbose_ ++;
}

void decrementVerboseInRecLine(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}


#define EXIT_ON_FAILURE 0
#define EXIT_ON_SUCCESS 1



void printRecursiveCoefficients( FILE *theFile,  RFcoefficientType *RFC, char *d )
{
  char *proc = "printRecursiveCoefficients";
  FILE *f;
  f =  ( theFile == (FILE*)NULL ) ? stderr : theFile;
  
  if ( d != (char*)NULL ) 
    fprintf( f, " - %s: %s\n", proc, d );
  
  fprintf( f, " - filter type:" );
  switch ( RFC->type_filter ) {
  default :
  case UNKNOWN_FILTER : fprintf( f, "unknown\n" ); break;
  case ALPHA_DERICHE : fprintf( f, "Deriche's filter (exponential (- alpha |X|))\n" ); break;
  case GAUSSIAN_DERICHE : fprintf( f, "gaussian approximation (Deriche's coefficients)\n" ); break;
  case GAUSSIAN_FIDRICH : fprintf( f, "gaussian approximation (Fidrich's coefficients)\n" ); break;
  case GAUSSIAN_YOUNG_1995 : fprintf( f, "gaussian approximation (Young and van Vliet's coefficients, 1995)\n" ); break;
  case GAUSSIAN_YOUNG_2002 : fprintf( f, "gaussian approximation (Young and van Vliet's coefficients, 2002)\n" ); break;
  case GABOR_YOUNG_2002 : fprintf( f, "gabor/gaussian approximation (Young and van Vliet's coefficients, 2002)\n" ); break;
  case GAUSSIAN_CONVOLUTION : fprintf( f, "brute-force convolution by the gaussian\n" ); break;
  }

  fprintf( f, " - derivative order:" );
  switch ( RFC->derivative ) {
  default : 
  case NODERIVATIVE : fprintf( f, "no derivative\n" ); break;
  case DERIVATIVE_0 : fprintf( f, "smoothing\n" ); break;
  case DERIVATIVE_1 : fprintf( f, "1st derivative\n" ); break;
  case DERIVATIVE_2 : fprintf( f, "2nd derivative\n" ); break;
  case DERIVATIVE_3 : fprintf( f, "3rd derivative\n" ); break;
  case DERIVATIVE_1_EDGES  : fprintf( f, "1st derivative (edge)\n" ); break;
  }

  fprintf( f, " - denominator:\n" );
  fprintf( f, "   %f %f %f %f\n", RFC->sd1, RFC->sd2, RFC->sd3, RFC->sd4 );
  fprintf( f, " - positive numerator:\n" );
  fprintf( f, "   (%f) %f %f %f %f\n", RFC->spm, RFC->sp0, RFC->sp1, RFC->sp2, RFC->sp3 );
  fprintf( f, " - negative numerator:\n" );
  fprintf( f, "   (%f) %f %f %f %f %f\n", RFC->snm, RFC->sn0, RFC->sn1, RFC->sn2, RFC->sn3, RFC->sn4 );
  fprintf( f, "\n" );

  switch ( RFC->type_filter ) {
  default : break;
  case GAUSSIAN_YOUNG_1995 : 
  case GAUSSIAN_YOUNG_2002 : 
  case GABOR_YOUNG_2002 : 
    if ( RFC->derivative == DERIVATIVE_0 ) {
      fprintf( f, " - Triggs' matrix:\n" );
      fprintf( f, "\t %f %f %f\n", RFC->TriggsMat[0], RFC->TriggsMat[1], RFC->TriggsMat[2] );
      fprintf( f, "\t %f %f %f\n", RFC->TriggsMat[3], RFC->TriggsMat[4], RFC->TriggsMat[5] );
      fprintf( f, "\t %f %f %f\n", RFC->TriggsMat[6], RFC->TriggsMat[7], RFC->TriggsMat[8] );
    }
    break;
  }

}





static void _ComputeTriggsMatrix( double *TriggsMat, double a1, double a2, double a3 ) 
{
  int i;
  double a;
  
  TriggsMat[0] = - a3*a1 + 1.0 - a3*a3 - a2;
  TriggsMat[1] = (a3 + a1) * (a2 + a3*a1);
  TriggsMat[2] = a3 * (a1 + a3*a2);
  TriggsMat[3] = a1 + a3*a2;
  TriggsMat[4] = - (a2 - 1.0) * (a2 + a3*a1);
  TriggsMat[5] = - (a3*a1 + a3*a3 + a2 - 1.0) * a3;
  TriggsMat[6] = a3*a1 + a2 + a1*a1 - a2*a2;
  TriggsMat[7] = a1*a2 + a3*a2*a2 - a1*a3*a3 - a3*a3*a3 - a3*a2 + a3;
  TriggsMat[8] = a3 * (a1 + a3*a2);
  a = (1.0 + a1 - a2 + a3) * (1.0 - a1 - a2 - a3) * (1 + a2 + (a1 - a3)*a3);
  for ( i=0; i<9; i++ ) TriggsMat[i] /= a; 
}





RFcoefficientType * InitRecursiveCoefficients( double x, 
                                               filterType type_filter,
                                               derivativeOrder derivative )
{
  char *proc="InitRecursiveCoefficients";
  int i;
  double q, scale;
  double m0 = 1.16680, m1 = 1.10783, m2 = 1.40586;
  double ex, k1, k2;
  double a0, a1, c0, c1, omega0, omega1;
  double B, b0, b1, b2, b3;
  double cos0, sin0, cos1, sin1;
  double sumA=0.0, sumC=0.0, aux;

  RFcoefficientType *RFC = NULL;
  RFC = (RFcoefficientType *)malloc( sizeof(RFcoefficientType) );
  if ( RFC == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: allocation failed\n", proc );
    return( NULL );
  }
  
  RFC->sd1 = RFC->sd2 = RFC->sd3 = RFC->sd4 = 0.0;
  RFC->spm = RFC->sp0 = RFC->sp1 = RFC->sp2 = RFC->sp3 = 0.0;
  RFC->snm = RFC->sn0 = RFC->sn1 = RFC->sn2 = RFC->sn3 = RFC->sn4 = 0.0;
  
  RFC->type_filter = UNKNOWN_FILTER;
  RFC->derivative  = NODERIVATIVE;
  
  q = 0.0 ;
  ex = k1 = k2 = 0.0;
  a0 = a1 = c0 = c1 = 0.0;
  B = 0.0;
  b0 = b1 = b2 = b3 = 0.0;
  omega0 = omega1 = 0.0;
  
  for ( i=0; i<9; i++ ) RFC->TriggsMat[i] = 0.0;

  /*--- Selon le type de filtrage (filtres de Deriche,
    ou approximation de la gaussienne), x designe
    soit alpha, soit sigma                         ---*/
  
  switch ( type_filter ) {

  case GABOR_YOUNG_2002 :

    /* from IEEE TRANSACTIONS ON SIGNAL PROCESSING, VOL. 50, NO. 11, NOVEMBER 2002
       Recursive Gabor Filtering
       Ian T. Young, Senior Member, IEEE, Lucas J. van Vliet, and Michael van Ginkel
       for Gabor Filter (section III.A) */
    if ( x < 3.556 )
      q = -0.2568 + 0.5784 * x + 0.0561 * x * x;
    else
      q = 2.5091 + 0.9804 * (x - 3.556);
    
    scale = (m0 + q) * (m1*m1 + m2*m2 + 2.0*m1*q + q*q);
    b1 = -q * (2.0*m0*m1 + m1*m1 + m2*m2 + (2.0*m0 + 4.0*m1) * q + 3.0*q*q) / scale;
    b2 = q*q * (m0 + 2.0*m1 + 3.0*q) / scale;
    b3 = - q*q*q / scale;
    
    B = (m0 * (m1*m1 + m2*m2) / scale );
    
    RFC->sd1  = - b1;
    RFC->sd2  = - b2;
    RFC->sd3  = - b3;
    _ComputeTriggsMatrix( RFC->TriggsMat, RFC->sd1, RFC->sd2, RFC->sd3 );
    
    switch ( derivative ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of derivative order.\n", proc );
      }
      free( RFC );
      return( NULL );
    case DERIVATIVE_0 :
      RFC->sp0 = B;
      RFC->sn0 = B;
      break;
    case DERIVATIVE_1 :
    case DERIVATIVE_1_EDGES :
      RFC->spm = - B/2;
      RFC->sp1 = B/2;
      RFC->sn0 = B;
      break;
    case DERIVATIVE_2 :
      RFC->spm = -B;
      RFC->sp0 = B;
      RFC->sn0 = -B;
      RFC->sn1 = B;
      break;
    case DERIVATIVE_3 :
      RFC->spm = B;
      RFC->sp0 = - 2.0 * B;
      RFC->sp1 = B;
      RFC->snm = - B/2;
      RFC->sn1 = B/2;
      break;
    }

    RFC->type_filter = type_filter;
    RFC->derivative  = derivative;
    break;

  case GAUSSIAN_YOUNG_2002 :
    if ( x < 1.0 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of coefficient (should be >= 1.0).\n", proc );
      }
      free( RFC );
      return( NULL );
    }    
    /* from IEEE TRANSACTIONS ON SIGNAL PROCESSING, VOL. 50, NO. 11, NOVEMBER 2002
       Recursive Gabor Filtering
       Ian T. Young, Senior Member, IEEE, Lucas J. van Vliet, and Michael van Ginkel
       for Gaussian filter (section II)
    */
    q = 1.31564 * ( sqrt ( 1.0 + 0.490811 * x * x ) - 1.0 );

    scale = (m0 + q) * (m1*m1 + m2*m2 + 2.0*m1*q + q*q);
    b1 = -q * (2.0*m0*m1 + m1*m1 + m2*m2 + (2.0*m0 + 4.0*m1) * q + 3.0*q*q) / scale;
    b2 = q*q * (m0 + 2.0*m1 + 3.0*q) / scale;
    b3 = - q*q*q / scale;

    /* use the same formula as for Gabor filter
     */
    B = (m0 * (m1*m1 + m2*m2) / scale );

    RFC->sd1  = - b1;
    RFC->sd2  = - b2;
    RFC->sd3  = - b3;
    _ComputeTriggsMatrix( RFC->TriggsMat, RFC->sd1, RFC->sd2, RFC->sd3 );

    switch ( derivative ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of derivative order.\n", proc );
      }
      free( RFC );
      return( NULL );
    case DERIVATIVE_0 :
      RFC->sp0 = B;
      RFC->sn0 = B;
      break;
    case DERIVATIVE_1 :
    case DERIVATIVE_1_EDGES :
      RFC->spm = - B/2;
      RFC->sp1 = B/2;
      RFC->sn0 = B;
      break;
    case DERIVATIVE_2 :
      RFC->spm = -B;
      RFC->sp0 = B;
      RFC->sn0 = -B;
      RFC->sn1 = B;
      break;
    case DERIVATIVE_3 :
      RFC->spm = B;
      RFC->sp0 = - 2.0 * B;
      RFC->sp1 = B;
      RFC->snm = - B/2;
      RFC->sn1 = B/2;
      break;
    }
    
    RFC->type_filter = type_filter;
    RFC->derivative  = derivative;
    break;

  case GAUSSIAN_YOUNG_1995 :
    if ( x < 0.5 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of coefficient (should be >= 0.5).\n", proc );
      }
      free( RFC );
      return( NULL );
    }
    if ( x <= 2.5 )
      q = 3.97156 - 4.14554 * sqrt( 1.0 - 0.26891 * x );
    else 
      q = 0.98711 * x - 0.96330;
    b0 = 1.57825 + 2.44413*q + 1.4281*q*q + 0.422205*q*q*q;
    b1 = 2.44413*q + 2.85619*q*q + 1.26661*q*q*q;
    b2 = -( 1.4281*q*q + 1.26661*q*q*q);
    b3 = 0.422205*q*q*q;

    B = 1.0 - (b1+b2+b3)/b0;

    RFC->sd1  = b1 / b0;
    RFC->sd2  = b2 / b0;
    RFC->sd3  = b3 / b0;
    _ComputeTriggsMatrix( RFC->TriggsMat, RFC->sd1, RFC->sd2, RFC->sd3 );

    switch ( derivative ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of derivative order.\n", proc );
      }
      free( RFC );
      return( NULL );
    case DERIVATIVE_0 :
      RFC->sp0 = B;
      RFC->sn0 = B;
      break;
    case DERIVATIVE_1 :
    case DERIVATIVE_1_EDGES :
      RFC->spm = - B/2;
      RFC->sp1 = B/2;
      RFC->sn0 = B;
      break;
    case DERIVATIVE_2 :
      RFC->spm = -B;
      RFC->sp0 = B;
      RFC->sn0 = -B;
      RFC->sn1 = B;
      break;
    case DERIVATIVE_3 :
      RFC->spm = B;
      RFC->sp0 = - 2.0 * B;
      RFC->sp1 = B;
      RFC->snm = - B/2;
      RFC->sn1 = B/2;
      break;
    }
    RFC->type_filter = type_filter;
    RFC->derivative  = derivative;
    break;

  case GAUSSIAN_FIDRICH :
    
    if ( x < 0.1 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of coefficient (should be >= 0.1).\n", proc );
      }
      free( RFC );
      return( NULL );
    }

    switch ( derivative ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of derivative order.\n", proc );
      }
      free( RFC );
      return( NULL );
    case DERIVATIVE_0 :
      a0 =  0.6570033214 / x;
      a1 =  1.978946687  / x;
      c0 = -0.2580640608 / x;
      c1 = -0.2391206463 / x;
      omega0 = 0.6512453378;
      omega1 = 2.05339943;
      b0 = 1.906154352;
      b1 = 1.881305409;
      break;
    case DERIVATIVE_1 :
    case DERIVATIVE_1_CONTOURS :
      a0 = -0.1726729496 / x;
      a1 = -2.003565572  / x;
      c0 =  0.1726730777 / x;
      c1 =  0.4440126835 / x;
      b0 = 1.560644213;
      b1 = 1.594202256;
      omega0 = 0.6995461735;
      omega1 = 2.144671764;
      break;
    case DERIVATIVE_2 :
      a0 = -0.7241334169 / x;
      a1 =  1.688628765  / x;
      c0 =  0.3251949838 / x;
      c1 = -0.7211796018 / x;
      b0 = 1.294951143;
      b1 = 1.427007123;
      omega0 = 0.7789803775;
      omega1 = 2.233566862;
      break;
    case DERIVATIVE_3 :
      a0 =  1.285774106  / x;
      a1 = -0.2896378408 / x;
      c0 = -1.28577129   / x;
      c1 =  0.26249833   / x;
      b0 = 1.01162886;
      b1 = 1.273344739;
      omega0 = 0.9474270928;
      omega1 = 2.337607006;
      break;
    }
    
    omega0 /= x;   sin0 = sin( omega0 );   cos0 = cos( omega0 ); 
    omega1 /= x;   sin1 = sin( omega1 );   cos1 = cos( omega1 ); 
    b0 /= x;
    b1 /= x;

    RFC->sp0  = a0 + c0;
    RFC->sp1  = exp( -b1 ) * (c1 * sin1 - (c0 + 2 * a0) * cos1);
    RFC->sp1 += exp( -b0 ) * (a1 * sin0 - (2 * c0 + a0) * cos0);
    RFC->sp2  = 2.0 * exp( -b0 - b1 ) 
      * ((a0 + c0) * cos1 * cos0 - cos1 * a1 * sin0 - cos0 * c1 * sin1);
    RFC->sp2 += c0 * exp( -2.0 * b0 ) + a0 * exp( -2.0 * b1 );
    RFC->sp3  = exp( -b1 - 2.0 * b0 ) * (c1 * sin1 - c0 * cos1);
    RFC->sp3 += exp( -b0 - 2.0 * b1 ) * (a1 * sin0 - a0 * cos0);
    
    RFC->sd1  = -2.0 * exp( -b1 ) * cos1 - 2.0 * exp( -b0 ) * cos0;
    RFC->sd2  = 4.0 * cos1 * cos0 * exp( -b0 - b1 ) 
      + exp( -2.0 * b1 ) + exp( -2.0 * b0 );
    RFC->sd3 = -2.0 * cos0 * exp( -b0 - 2.0 * b1 ) 
      - 2.0 * cos1 * exp( -b1 - 2.0 * b0 );
    RFC->sd4 = exp( -2.0 * b0 - 2.0 * b1 );
    
    switch ( derivative ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of derivative order.\n", proc );
      }
      free( RFC );
      return( NULL );
    case DERIVATIVE_0 :
    case DERIVATIVE_2 :
      RFC->sn1 =   RFC->sp1 - RFC->sd1 * RFC->sp0;
      RFC->sn2 =   RFC->sp2 - RFC->sd2 * RFC->sp0;
      RFC->sn3 =   RFC->sp3 - RFC->sd3 * RFC->sp0;
      RFC->sn4 = - RFC->sd4 * RFC->sp0;
      break;
    case DERIVATIVE_1 :
    case DERIVATIVE_1_CONTOURS :
    case DERIVATIVE_3 :
      RFC->sn1 = - RFC->sp1 + RFC->sd1 * RFC->sp0;
      RFC->sn2 = - RFC->sp2 + RFC->sd2 * RFC->sp0;
      RFC->sn3 = - RFC->sp3 + RFC->sd3 * RFC->sp0;
      RFC->sn4 =   RFC->sd4 * RFC->sp0;
    }
    
    RFC->type_filter = type_filter;
    RFC->derivative  = derivative;
    break;
    
  case GAUSSIAN_DERICHE :
    
    if ( x < 0.1 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of coefficient (should be >= 0.1).\n", proc );
      }
      free( RFC );
      return( NULL );
    }

    switch ( derivative ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: switch to default coefficients (smoothing).\n", proc );
      }
      derivative = DERIVATIVE_0;
    case DERIVATIVE_0 :
      a0     =  1.68;
      omega0 =  0.6318;
      a1     =  3.735;
      b0     =  1.783;
      c0     = -0.6803;
      omega1 =  1.997;
      c1     = -0.2598;
      b1     =  1.723;
      break;
    case DERIVATIVE_1 :
    case DERIVATIVE_1_CONTOURS :
      a0     =  -0.6472;
      omega0 =  0.6719;
      a1     =  -4.531;
      b0     =  1.527;
      c0     =  0.6494;
      omega1 =  2.072;
      c1     =  0.9557;
      b1     =  1.516;
      break;
    case DERIVATIVE_2 :
      a0     = -1.331;
      omega0 =  0.748;
      a1     =  3.661;
      b0     =  1.24;
      c0     =  0.3225;
      omega1 =  2.166;
      c1     = -1.738;
      b1     =  1.314;
    }

    omega0 /= x;   sin0 = sin( omega0 );   cos0 = cos( omega0 ); 
    omega1 /= x;   sin1 = sin( omega1 );   cos1 = cos( omega1 ); 
    b0 /= x;
    b1 /= x;

    /*--- normalisation ---*/
    switch ( derivative ) {
    default :
    case DERIVATIVE_0 :
      sumA  = 2.0 * a1 * exp( b0 ) * cos0 * cos0 - a0 * sin0 * exp( 2.0 * b0 );
      sumA += a0 * sin0 - 2.0 * a1 * exp( b0 );
      sumA /= ( 2.0 * cos0 * exp( b0 ) - exp( 2.0 * b0 ) - 1 ) * sin0;
      sumC  = 2.0 * c1 * exp( b1 ) * cos1 * cos1 - c0 * sin1 * exp( 2.0 * b1 );
      sumC += c0 * sin1 - 2.0 * c1 * exp( b1 );
      sumC /= ( 2.0 * cos1 * exp( b1 ) - exp( 2.0 * b1 ) - 1 ) * sin1;
      break;
    case DERIVATIVE_1 :
      aux   = exp( 4.0 * b0 ) - 4.0 * cos0 * exp( 3.0 * b0 );
      aux  += 2.0 * exp( 2.0 * b0 ) + 4.0 * cos0 * cos0 * exp( 2.0 * b0 );
      aux  += 1.0 - 4.0 * cos0 * exp( b0 );
      sumA  = a0 * cos0 - a1 * sin0 + a1 * sin0 * exp( 2.0 * b0 );
      sumA += a0 * cos0 * exp( 2.0 * b0 ) - 2.0 * a0 * exp( b0 );
      sumA *= exp( b0 ) / aux;
      aux   = exp( 4.0 * b1 ) - 4.0 * cos1 * exp( 3.0 * b1 );
      aux  += 2.0 * exp( 2.0 * b1 ) + 4.0 * cos1 * cos1 * exp( 2.0 * b1 );
      aux  += 1.0 - 4.0 * cos1 * exp( b1 );
      sumC  = c0 * cos1 - c1 * sin1 + c1 * sin1 * exp( 2.0 * b1 );
      sumC += c0 * cos1 * exp( 2.0 * b1 ) - 2.0 * c0 * exp( b1 );
      sumC *= exp( b1 ) / aux;
      /*--- on multiplie les sommes par 2 car on n'a calcule que des demi-sommes 
        et on change le signe car la somme doit etre egale a -1              ---*/
      sumA *= (-2.0);
      sumC *= (-2.0);
      break;
    case DERIVATIVE_1_CONTOURS :
      /*--- la somme de 1 a l'infini est egale a 1 : cela introduit
        un petit biais (reponse un rien superieur a la hauteur du step).
        Avec une somme de 0 a l'infini, c'est pire                       ---*/
      sumA  = a1 * exp( b0 ) - a1 * cos0 * cos0 * exp( b0 );
      sumA += a0 * cos0 * sin0 * exp( b0 ) - a0 * sin0;
      sumA /= sin0 * ( 2.0 * cos0 * exp( b0 ) - exp( 2.0 * b0 ) - 1 );
      sumC  = c1 * exp( b1 ) - c1 * cos1 * cos1 * exp( b1 );
      sumC += c0 * cos1 * sin1 * exp( b1 ) - c0 * sin1;
      sumC /= sin1 * ( 2.0 * cos1 * exp( b1 ) - exp( 2.0 * b1 ) - 1 );
      break;
    case DERIVATIVE_2 :
      aux   = 12.0 * cos0 * exp( 3.0 * b0 ) - 3.0 * exp( 2.0 * b0 );
      aux  += 8.0 * cos0 * cos0 * cos0 * exp( 3.0 * b0 ) 
        - 12.0 * cos0 * cos0 * exp( 4.0 * b0 );
      aux  -= 3.0 * exp( 4.0 * b0 );
      aux  += 6.0 * cos0 * exp( 5.0 * b0 ) -  exp( 6.0 * b0 ) 
        + 6.0 * cos0 * exp( b0 );
      aux  -= ( 1.0 + 12.0 * cos0 * cos0 * exp( 2.0 * b0 ) );
      sumA  = 4.0 * a0 * sin0 * exp( 3.0 * b0 ) 
        + a1 * cos0 * cos0 * exp( 4.0 * b0 );
      sumA -= ( 4.0 * a0 * sin0 * exp( b0 ) 
                + 6.0 * a1 * cos0 * cos0 * exp( 2.0 * b0 ) );
      sumA += 2.0 * a1 * cos0 * cos0 * cos0 * exp( b0 ) 
        - 2.0 * a1 * cos0 * exp( b0 );
      sumA += 2.0 * a1 * cos0 * cos0 * cos0 * exp( 3.0 * b0 ) 
        - 2.0 * a1 * cos0 * exp( 3.0 * b0 );
      sumA += a1 * cos0 * cos0 - a1 * exp( 4.0 * b0 );
      sumA += 2.0 * a0 * sin0 * cos0 * cos0 * exp( b0 ) 
        - 2.0 * a0 * sin0 * cos0 * cos0 * exp( 3.0 * b0 );
      sumA -= ( a0 * sin0 * cos0 * exp( 4.0 * b0 ) + a1 );
      sumA += 6.0 * a1 * exp( 2.0 * b0 ) + a0 * cos0 * sin0;
      sumA *= 2.0 * exp( b0 ) / ( aux * sin0 );
      aux   = 12.0 * cos1 * exp( 3.0 * b1 ) - 3.0 * exp( 2.0 * b1 );
      aux  += 8.0 * cos1 * cos1 * cos1 * exp( 3.0 * b1 ) 
        - 12.0 * cos1 * cos1 * exp( 4.0 * b1 );
      aux  -= 3.0 * exp( 4.0 * b1 );
      aux  += 6.0 * cos1 * exp( 5.0 * b1 ) -  exp( 6.0 * b1 ) 
        + 6.0 * cos1 * exp( b1 );
      aux  -= ( 1.0 + 12.0 * cos1 * cos1 * exp( 2.0 * b1 ) );
      sumC  = 4.0 * c0 * sin1 * exp( 3.0 * b1 ) 
        + c1 * cos1 * cos1 * exp( 4.0 * b1 );
      sumC -= ( 4.0 * c0 * sin1 * exp( b1 ) 
                + 6.0 * c1 * cos1 * cos1 * exp( 2.0 * b1 ) );
      sumC += 2.0 * c1 * cos1 * cos1 * cos1 * exp( b1 ) 
        - 2.0 * c1 * cos1 * exp( b1 );
      sumC += 2.0 * c1 * cos1 * cos1 * cos1 * exp( 3.0 * b1 ) 
        - 2.0 * c1 * cos1 * exp( 3.0 * b1 );
      sumC += c1 * cos1 * cos1 - c1 * exp( 4.0 * b1 );
      sumC += 2.0 * c0 * sin1 * cos1 * cos1 * exp( b1 ) 
        - 2.0 * c0 * sin1 * cos1 * cos1 * exp( 3.0 * b1 );
      sumC -= ( c0 * sin1 * cos1 * exp( 4.0 * b1 ) + c1 );
      sumC += 6.0 * c1 * exp( 2.0 * b1 ) + c0 * cos1 * sin1;
      sumC *= 2.0 * exp( b1 ) / ( aux * sin1 );
      /*--- on divise les sommes par 2 (la somme doit etre egale a 2) ---*/
      sumA /= 2;
      sumC /= 2;
    }
    a0 /= ( sumA + sumC );
    a1 /= ( sumA + sumC );
    c0 /= ( sumA + sumC );
    c1 /= ( sumA + sumC );
    
    /*--- coefficients du calcul recursif ---*/
    RFC->sp0  = a0 + c0;
    RFC->sp1  = exp( -b1 ) * (c1 * sin1 - (c0 + 2 * a0) * cos1);
    RFC->sp1 += exp( -b0 ) * (a1 * sin0 - (2 * c0 + a0) * cos0);
    RFC->sp2  = 2.0 * exp( -b0 - b1 ) 
      * ((a0 + c0) * cos1 * cos0 - cos1 * a1 * sin0 - cos0 * c1 * sin1);
    RFC->sp2 += c0 * exp( -2.0 * b0 ) + a0 * exp( -2.0 * b1 );
    RFC->sp3  = exp( -b1 - 2.0 * b0 ) * (c1 * sin1 - c0 * cos1);
    RFC->sp3 += exp( -b0 - 2.0 * b1 ) * (a1 * sin0 - a0 * cos0);
    
    RFC->sd1  = -2.0 * exp( -b1 ) * cos1 - 2.0 * exp( -b0 ) * cos0;
    RFC->sd2  = 4.0 * cos1 * cos0 * exp( -b0 - b1 ) 
      + exp( -2.0 * b1 ) + exp( -2.0 * b0 );
    RFC->sd3 = -2.0 * cos0 * exp( -b0 - 2.0 * b1 ) 
      - 2.0 * cos1 * exp( -b1 - 2.0 * b0 );
    RFC->sd4 = exp( -2.0 * b0 - 2.0 * b1 );
    
    switch ( derivative ) {
    default :
    case DERIVATIVE_0 :
    case DERIVATIVE_2 :
      RFC->sn1 =   RFC->sp1 - RFC->sd1 * RFC->sp0;
      RFC->sn2 =   RFC->sp2 - RFC->sd2 * RFC->sp0;
      RFC->sn3 =   RFC->sp3 - RFC->sd3 * RFC->sp0;
      RFC->sn4 = - RFC->sd4 * RFC->sp0;
      break;
    case DERIVATIVE_1 :
    case DERIVATIVE_1_CONTOURS :
    case DERIVATIVE_3 :
      RFC->sn1 = - RFC->sp1 + RFC->sd1 * RFC->sp0;
      RFC->sn2 = - RFC->sp2 + RFC->sd2 * RFC->sp0;
      RFC->sn3 = - RFC->sp3 + RFC->sd3 * RFC->sp0;
      RFC->sn4 =   RFC->sd4 * RFC->sp0;
    }
    
    RFC->type_filter = type_filter;
    RFC->derivative  = derivative;
    break;



  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: switch to default recursive filter (Deriche's filters).\n", proc );
    }
    type_filter = ALPHA_DERICHE;
  case ALPHA_DERICHE :

    if ( (x < 0.1) || (x > 1.9) ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: improper value of coefficient (should be >= 0.1 and <= 1.9).\n", proc );
      }
      free( RFC );
      return( NULL );
    }
    ex = exp( (-x) );
    
    switch ( derivative ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: switch to default coefficients (smoothing).\n", proc );
      }
      derivative = DERIVATIVE_0;
    case DERIVATIVE_0 :
      RFC->sp0 = (1.0 - ex) * (1.0 - ex) / (1.0 + 2.0 * x * ex - ex * ex);
      RFC->sp1 = RFC->sp0 * (x - 1.0) * ex;
      RFC->sn1 = RFC->sp0 * (x + 1.0) * ex;
      RFC->sn2 = (- RFC->sp0) * ex * ex;
      RFC->sd1 = (- 2.0) * ex;
      RFC->sd2 = ex * ex;
      break;
    case DERIVATIVE_1 :
      RFC->sp1 = - (1.0 - ex) * (1.0 - ex) * (1.0 - ex) / (2.0 * (1.0 + ex));
      RFC->sn1 = (- RFC->sp1);
      RFC->sd1 = (- 2.0) * ex;
      RFC->sd2 = ex * ex;
      break;
    case DERIVATIVE_1_CONTOURS :
      RFC->sp1 = - (1.0 - ex) * (1.0 - ex);
      RFC->sn1 = (- RFC->sp1);
      RFC->sd1 = (- 2.0) * ex;
      RFC->sd2 = ex * ex;
      break;
    case DERIVATIVE_2 :
      k1 = (- 2.0) * (1.0 - ex) * (1.0 - ex) * (1.0 - ex);
      k1 /= (1.0 + ex) * (1.0 + ex) * (1.0 + ex);
      k2 = (1.0 - ex * ex) / (2.0 * ex);
      RFC->sp0 = k1;
      RFC->sp1 = (- k1) * (1.0 + k2) * ex;
      RFC->sn1 = k1 * (1.0 - k2) * ex;
      RFC->sn2 = (- k1) * ex * ex;
      RFC->sd1 = (- 2.0) * ex;
      RFC->sd2 = ex * ex;
      break;
    case DERIVATIVE_3 :
      k1 = (1.0 + x) * ex + (x - 1.0);
      k2 = (1.0 - ex) / k1;
      k1 *= (1.0 - ex) * (1.0 - ex) * (1.0 - ex) * (1.0 - ex);
      k1 /= 2.0 * x * x * ex * ex;
      k1 /= ex + 1.0;
      RFC->sp0 = k1 * x * (k2 + 1.0);
      RFC->sp1 = (- k1) * x * (1.0 + k2 + k2*x) * ex;
      RFC->sn0 = (- RFC->sp0);
      RFC->sn1 = (- RFC->sp1);
      RFC->sd1 = (- 2.0) * ex;
      RFC->sd2 = ex * ex;
    }
    RFC->type_filter = type_filter;
    RFC->derivative  = derivative;
  }

  return( RFC );
}





int RecursiveFilter1D( RFcoefficientType *RFC,
                       double *in,
                       double *out,
                       double *work1,
                       double *work2,
                       int dim )
{
  char *proc="RecursiveFilter1D";
  register double rpm, rp0, rp1, rp2, rp3;
  register double rd1, rd2, rd3, rd4;
  register double rnm, rn0, rn1, rn2, rn3, rn4;
  register int i;
  register double *w0, *w1, *w2, *w3, *w4;
  register double *d0, *d1, *d2, *d3, *d4;
  /* Triggs variables
   */
  double uplus, vplus;
  double vn, vn1, vn2;

  if ( RFC->type_filter == UNKNOWN_FILTER ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unknown type of recursive filter.\n", proc );
    return( EXIT_ON_FAILURE );
  }
  if ( RFC->derivative == NODERIVATIVE ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unknown type of derivative.\n", proc );
    return( EXIT_ON_FAILURE );
  }

  rd1 = rd2 = rd3 = rd4 = 0.0;
  rpm = rp0 = rp1 = rp2 = rp3 = 0.0;
  rnm = rn0 = rn1 = rn2 = rn3 = rn4 = 0.0;
  
  switch( RFC->type_filter ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: unknown type of recursive filter.\n", proc );
    return( EXIT_ON_FAILURE );

 

  case GAUSSIAN_YOUNG_1995 :
  case GABOR_YOUNG_2002 :
  case GAUSSIAN_YOUNG_2002 :
    rd1 = RFC->sd1;   rd2 = RFC->sd2;   rd3 = RFC->sd3; 
    switch( RFC->derivative ) {
    default :
      if ( _verbose_ )
      fprintf( stderr, "%s: improper value of derivative order.\n", proc );
    return( EXIT_ON_FAILURE );

    case DERIVATIVE_0 :
      rp0 = RFC->sp0;
      rn0 = RFC->sn0;

      /* Triggs
       */
      uplus = in[dim-1] / (1.0 - rd1 - rd2 - rd3 );
      uplus = rp0 * in[dim-1] / (1.0 - rd1 - rd2 - rd3 );
      vplus = uplus / (1.0 - rd1 - rd2 - rd3 );

      /* forward
         w[0] = B in[0]
         w[1] = B in[1] + b1 w[0]
         w[2] = B in[2] + b1 w[1] + b2 w[0]
         w[n] = B in[n] + b1 w[n-1] + b2 w[n-2] + b3 w[n-3]
       */
      d3 = in;        d2 = d3+1;   d1 = d2+1;   d0 = d1+1;
      w3 = work1;   w2 = w3+1;   w1 = w2+1;   w0 = w1+1;
      /* first guess would have to apply naively the recursive filter
         ie, by assuming previous values of the running sum wi are 0's
         *(w3) = rp0 * (*d3);
         *(w2) = rp0 * (*d2) + rd1 * (*w3);
         *(w1) = rp0 * (*d1) + rd1 * (*w2) + rd2 * (*w3);
         However Triggs shown that they should be computed as above
      */
      *(w1) = *(w2) = *(w3) =  rp0 * (*d3) / ( 1.0 - rd1 - rd2 - rd3 );
      for ( i=3; i<dim; i++,w0++,w1++,w2++,w3++,d0++ ) 
        *(w0) = rp0 * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      
      /* backward
         out[dim-1] = B w[dim-1];
         out[dim-2] = B w[dim-2] + b1 out[dim-1]
         out[dim-3] = B w[dim-3] + b1 out[dim-2] + b2 out[dim-1]
         out[n] = B w[n] + b1 out[n+1] + b2 out[n+2] + b3 out[n+3]
       */

      d3 = work1+dim-1;   d2 = d3-1;   d1 = d2-1;   d0 = d1-1;
      w3 = out+dim-1;     w2 = w3-1;   w1 = w2-1;   w0 = w1-1;

      /* first guess would have to apply naively the recursive filter
         ie, by assuming previous values of the running sum wi are 0's
         *(w3) = rn0 * (*d3);
         *(w2) = rn0 * (*d2) + rd1 * (*w3);
         *(w1) = rn0 * (*d1) + rd1 * (*w2) + rd2 * (*w3);
         second guess, adapt the computation done for the forward pass
         *(w1) = *(w2) = *(w3) =  rn0 * (*d3) / ( 1.0 - rd1 - rd2 - rd3 );
         Triggs' proposal is somewaht more complicated
      */
      vn  = RFC->TriggsMat[0] * (work1[dim-1] - uplus) 
        + RFC->TriggsMat[1] * (work1[dim-2] - uplus)
        + RFC->TriggsMat[2] * (work1[dim-3] - uplus)
        + vplus;
      vn1 = RFC->TriggsMat[3] * (work1[dim-1] - uplus) 
        + RFC->TriggsMat[4] * (work1[dim-2] - uplus)
        + RFC->TriggsMat[5] * (work1[dim-3] - uplus)
        + vplus;
      vn2 = RFC->TriggsMat[6] * (work1[dim-1] - uplus) 
        + RFC->TriggsMat[7] * (work1[dim-2] - uplus)
        + RFC->TriggsMat[8] * (work1[dim-3] - uplus)
        + vplus;
      
      vn  *= rn0;
      vn1 *= rn0;
      vn2 *= rn0;

      *(w3) = rn0 * (*d3) + rd1 * vn    + rd2 * vn1   + rd3 * vn2;
      *(w2) = rn0 * (*d2) + rd1 * (*w3) + rd2 * vn    + rd3 * vn1;
      *(w1) = rn0 * (*d1) + rd1 * (*w2) + rd2 * (*w3) + rd3 * vn;
      for ( i=dim-4; i>=0; i--,w0--,w1--,w2--,w3--,d0-- )
        *(w0) = rn0 * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      break;

    case DERIVATIVE_1 :
    case DERIVATIVE_1_EDGES :
      rpm = RFC->spm;
      rp1 = RFC->sp1;
      rn0 = RFC->sn0;

      /* forward
         w[0]     =                 B/2 in[1]
         w[1]     = - B/2 in[0]   + B/2 in[2]    + b1 w[1] + b2 w[0]
         w[2]     = - B/2 in[2]   + B/2 in[3]    + b1 w[1] + b2 w[0]
         w[n]     = - B/2 in[n-1] + B/2 in[n+1]  + b1 w[n-1] + b2 w[n-2] + b3 w[n-3]
         w[dim-1] = - B/2 in[dim-2]              + b1 w[dim-2] + b2 w[dim-3] + b3 w[dim-4]
       */
      d4 = in;      d3 = d4+1;   d2 = d3+1;   d1 = d2+1;   d0 = d1+1;
      w3 = work1;   w2 = w3+1;   w1 = w2+1;   w0 = w1+1;
      *(w3) =               rp1 * (*d3); 
      *(w2) = rpm * (*d4) + rp1 * (*d2) + rd1 * (*w3);
      *(w1) = rpm * (*d3) + rp1 * (*d1) + rd1 * (*w2) + rd2 * (*w3);
      for ( i=3; i<dim-1; i++,w0++,w1++,w2++,w3++,d0++,d2++ ) 
        *(w0) = rpm * (*d2) + rp1 * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      *(w0) = rpm * (*d2) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      
      /* backward
         out[dim-1] = B w[dim-1];
         out[dim-2] = B w[dim-2] + b1 out[dim-1]
         out[dim-3] = B w[dim-3] + b1 out[dim-2] + b2 out[dim-1]
         out[n] = B w[n] + b1 out[n+1] + b2 out[n+2] + b3 out[n+3]
       */
       d3 = work1+dim-1;   d2 = d3-1;   d1 = d2-1;   d0 = d1-1;
      w3 = out+dim-1;     w2 = w3-1;   w1 = w2-1;   w0 = w1-1;
      *(w3) = rn0 * (*d3); 
      *(w2) = rn0 * (*d2) + rd1 * (*w3);
      *(w1) = rn0 * (*d1) + rd1 * (*w2) + rd2 * (*w3);
      for ( i=dim-4; i>=0; i--,w0--,w1--,w2--,w3--,d0-- )
        *(w0) = rn0 * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      break;

    case DERIVATIVE_2 :
      rpm = RFC->spm;
      rp0 = RFC->sp0;
      rn0 = RFC->sn0;
      rn1 = RFC->sn1;

      /* forward
         w[0] = B in[0]
         w[1] = B in[1] - B in[0] + b1 w[0]
         w[2] = B in[2] - B in[1] + b1 w[1] + b2 w[0]
         w[n] = B in[n] - B in[n-1] + b1 w[n-1] + b2 w[n-2] + b3 w[n-3]
       */
      d3 = in;      d2 = d3+1;   d1 = d2+1;   d0 = d1+1;
      w3 = work1;   w2 = w3+1;   w1 = w2+1;   w0 = w1+1;
      *(w3) =               rp0 * (*d3); 
      *(w2) = rpm * (*d3) + rp0 * (*d2) + rd1 * (*w3);
      *(w1) = rpm * (*d2) + rp0 * (*d1) + rd1 * (*w2) + rd2 * (*w3);
      for ( i=3; i<dim; i++,w0++,w1++,w2++,w3++,d0++,d1++ ) 
        *(w0) = rpm * (*d1) + rp0 * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      
      /* backward
         out[dim-1] =            - B w[dim-1];
         out[dim-2] = B w[dim-1] - B w[dim-2] + b1 out[dim-1]
         out[dim-3] = B w[dim-2] - B w[dim-3] + b1 out[dim-2] + b2 out[dim-1]
         out[n] = B w[n+1] - B w[n] + b1 out[n+1] + b2 out[n+2] + b3 out[n+3]
       */
      d3 = work1+dim-1;   d2 = d3-1;   d1 = d2-1;   d0 = d1-1;
      w3 = out+dim-1;     w2 = w3-1;   w1 = w2-1;   w0 = w1-1;
      *(w3) =               rn0 * (*d3); 
      *(w2) = rn1 * (*d3) + rn0 * (*d2) + rd1 * (*w3);
      *(w1) = rn1 * (*d2) + rn0 * (*d1) + rd1 * (*w2) + rd2 * (*w3);
      for ( i=dim-4; i>=0; i--,w0--,w1--,w2--,w3--,d0--,d1-- )
        *(w0) = rn1 * (*d1) + rn0 * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      break;

    case DERIVATIVE_3 :
      rpm = RFC->spm;
      rp0 = RFC->sp0;
      rp1 = RFC->sp1;
      rnm = RFC->snm;
      rn1 = RFC->sn1;

      /* forward
         w[0] = B in[1] - 2B in[0]
         w[1] = B in[2] - 2B in[1] + B in[0] + b1 w[0]
         w[2] = B in[3] - 2B in[2] + B in[1] + b1 w[1] + b2 w[0]
         w[n] = B in[n+1] - 2B in[n] + B in[n-1] + b1 w[n-1] + b2 w[n-2] + b3 w[n-3]
         w[dim-1] =       - 2B in[dim-1] + B in[dim-2] + b1 w[dim-2] + b2 w[dim-3] + b3 w[dim-4]
       */
      d4 = in;      d3 = d4+1;   d2 = d3+1;   d1 = d2+1;   d0 = d1+1;
      w3 = work1;   w2 = w3+1;   w1 = w2+1;   w0 = w1+1;
      *(w3) =               rp0 * (*d4) + rp1 * (*d3); 
      *(w2) = rpm * (*d3) + rp0 * (*d3) + rp1 * (*d2) + rd1 * (*w3);
      *(w1) = rpm * (*d2) + rp0 * (*d2) + rp1 * (*d1) + rd1 * (*w2) + rd2 * (*w3);
      for ( i=3; i<dim-1; i++,w0++,w1++,w2++,w3++,d0++,d1++,d2++ ) 
        *(w0) = rpm * (*d2) + rp0 * (*d1) + rp1 * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      *(w0) = rpm * (*d2) + rp0 * (*d1) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      
      /* backward
         out[dim-1] =              - B/2 w[dim-2]
         out[dim-2] = B/2 w[dim-1] - B/2 w[dim-3] + b1 out[dim-1]
         out[dim-3] = B/2 w[dim-2] - B/2 w[dim-4] + b1 out[dim-2] + b2 out[dim-1]
         out[n] = B/2 w[n+1] - B/2 w[n-1] + b1 out[n+1] + b2 out[n+2] + b3 out[n+3]
       */
      d4 = work1+dim-1;   d3 = d4-1;   d2 = d3-1;   d1 = d2-1;   d0 = d1-1;
      w3 = out+dim-1;     w2 = w3-1;   w1 = w2-1;   w0 = w1-1;
      *(w3) =               rnm * (*d3); 
      *(w2) = rn1 * (*d4) + rnm * (*d2) + rd1 * (*w3);
      *(w1) = rn1 * (*d3) + rnm * (*d1) + rd1 * (*w2) + rd2 * (*w3);
      for ( i=dim-4; i>0; i--,w0--,w1--,w2--,w3--,d0--,d2-- )
        *(w0) = rn1 * (*d2) + rnm * (*d0) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      *(w0) = rn1 * (*d2) + rd1 * (*w1) + rd2 * (*w2) + rd3 * (*w3);
      break;

    }
    break;
    
  case GAUSSIAN_FIDRICH :
  case GAUSSIAN_DERICHE :
    /*--- filtrage generique d'ordre 4 ---*/
    rp0 = RFC->sp0;   rp1 = RFC->sp1;   rp2 = RFC->sp2;   rp3 = RFC->sp3;
    rd1 = RFC->sd1;   rd2 = RFC->sd2;   rd3 = RFC->sd3;   rd4 = RFC->sd4;
    rn1 = RFC->sn1;   rn2 = RFC->sn2;   rn3 = RFC->sn3;   rn4 = RFC->sn4;
    
    /* on positionne les pointeurs 
     */
    w4 = work1;   w3 = w4+1;   w2 = w3+1;   w1 = w2+1;   w0 = w1+1;
    d3 = in+1;    d2 = d3+1;   d1 = d2+1;   d0 = d1+1;
    /*--- calcul de y+ ---*/
    *(w4) = rp0 * *(in);
    *(w3) = rp0 * *(d3) + rp1 * *(in)
          - rd1 * *(w4);   
    *(w2) = rp0 * *(d2) + rp1 * *(d3) + rp2 * *(in)
          - rd1 * *(w3) - rd2 * *(w4);
    *(w1) = rp0 * *(d1) + rp1 * *(d2) + rp2 * *(d3) + rp3 * *(in)
          - rd1 * *(w2) - rd2 * *(w3) - rd3 * *(w4);
    for (i=4; i<dim; i++,w0++,w1++,w2++,w3++,w4++,d0++,d1++,d2++,d3++) 
      *(w0) = rp0 * *(d0) + rp1 * *(d1) + rp2 * *(d2) + rp3 * *(d3)
            - rd1 * *(w1) - rd2 * *(w2) - rd3 * *(w3) - rd4 * *(w4);
    
    /* on positionne les pointeurs 
     */
    w4 = work2+dim-1;   w3 = w4-1;   w2 = w3-1;   w1 = w2-1;   w0 = w1-1;
    d4 = in+dim-1;      d3 = d4-1;   d2 = d3-1;   d1 = d2-1;
    /*--- calcul de y- ---*/
    *(w4) = 0;
    *(w3) = rn1 * *(d4);
    *(w2) = rn1 * *(d3) + rn2 * *(d4) 
          - rd1 * *(w3);
    *(w1) = rn1 * *(d2) + rn2 * *(d3) + rn3 * *(d4) 
          - rd1 * *(w2) - rd2 * *(w3);
    for (i=dim-5; i>=0; i--,w0--,w1--,w2--,w3--,w4--,d1--,d2--,d3--,d4--)
      *(w0) = rn1 * *(d1) + rn2 * *(d2) + rn3 * *(d3) + rn4 * *(d4)
            - rd1 * *(w1) - rd2 * *(w2) - rd3 * *(w3) - rd4 * *(w4);

    /*--- calcul final ---*/
    w1 = work1;   w2 = work2;   d0 = out;
    for (i=0 ; i<dim ; i++,w1++,w2++,d0++)
      *d0 = *w1 + *w2;
    
    break;

  case ALPHA_DERICHE :
    
    switch( RFC->derivative ) {
    default :
    case DERIVATIVE_0 :
    case DERIVATIVE_2 :

      rp0 = RFC->sp0;   rp1 = RFC->sp1;
      rd1 = RFC->sd1;   rd2 = RFC->sd2;
      rn1 = RFC->sn1;   rn2 = RFC->sn2;
      
      /* on positionne les pointeurs 
       */
      w2 = work1;   w1 = w2+1;   w0 = w1+1;
      d1 = in+1;    d0 = d1+1;
      /*--- calcul de y+ ---*/
      *(w2) = rp0 * *(in);
      *(w1) = rp0 * *(d1) + rp1 * *(in) 
            - rd1 * *(w2);
      for (i=2;  i<dim; i++,w0++,w1++,w2++,d0++,d1++)
        *(w0) = rp0 * *(d0) + rp1 * *(d1)
              - rd1 * *(w1) - rd2 * *(w2);
      
      w2 = work2+dim-1;   w1 = w2-1;   w0 = w1-1;
      d2 = in+dim-1;      d1 = d2-1;
      /*--- calcul de y- ---*/
      *(w2) = 0.0;
      *(w1) = rn1 * *(d2);
      for (i=dim-3; i>=0; i--,w0--,w1--,w2--,d1--,d2--)
        *(w0) = rn1 * *(d1) + rn2 * *(d2)
              - rd1 * *(w1) - rd2 * *(w2);
      
      /*--- calcul final ---*/
      w1 = work1;   w2 = work2;   d0 = out;
      for (i=0 ; i<dim ; i++,w1++,w2++,d0++)
        *d0 = *w1 + *w2;
      
      break;
      
    case DERIVATIVE_1 :
    case DERIVATIVE_1_CONTOURS :
      rp1 = RFC->sp1;
      rn1 = RFC->sn1;
      rd1 = RFC->sd1;   rd2 = RFC->sd2;
      
      /* on positionne les pointeurs 
       */
      w2 = work1;   w1 = w2+1;   w0 = w1+1;
      d1 = in+1;
      /*--- calcul de y+ ---*/
      *(w2) = 0.0;
      *(w1) = rp1 * *(in);     
      for (i=2;  i<dim; i++,w0++,w1++,w2++,d1++)
        *(w0) = rp1 * *(d1)
              - rd1 * *(w1) - rd2 * *(w2);
      
      
      w2 = work2+dim-1;   w1 = w2-1;   w0 = w1-1;
      d2 = in+dim-1;      d1 = d2-1;
      /*--- calcul de y- ---*/
      *(w2) = 0.0;
      *(w1) = rn1 * *(d2);
      for (i=dim-3; i>=0; i--,w0--,w1--,w2--,d1--)
        *(w0) = rn1 * *(d1)
              - rd1 * *(w1) - rd2 * *(w2);
      
      /*--- calcul final ---*/
      w1 = work1;   w2 = work2;   d0 = out;
      for (i=0 ; i<dim ; i++,w1++,w2++,d0++)
        *d0 = *w1 + *w2;
      
      break;

    case DERIVATIVE_3 :
      rp0 = RFC->sp0;   rp1 = RFC->sp1;
      rd1 = RFC->sd1;   rd2 = RFC->sd2;
      rn0 = RFC->sn0;   rn1 = RFC->sn1;
      
      w2 = work1;   w1 = w2+1;   w0 = w1+1;
      d1 = in+1;   d0 = d1+1;
      /*--- calcul de y+ ---*/
      *(w2) = rp0 * *(in);
      *(w1) = rp0 * *(d1) + rp1 * *(in) 
            - rd1 * *(w2);
      for (i=2;  i<dim; i++,w0++,w1++,w2++,d0++,d1++)
        *(w0) = rp0 * *(d0) + rp1 * *(d1)
              - rd1 * *(w1) - rd2 * *(w2);
      
      w2 = work2+dim-1;   w1 = w2-1;   w0 = w1-1;
      d2 = in+dim-1;      d1 = d2-1;   d0 = d1-1;
      /*--- calcul de y- ---*/
      *(w2) = rn0 * *(d2);
      *(w1) = rn0 * *(d1) + rn1 * *(d2) 
            - rd1 * *(w2);
      for (i=dim-3; i>=0; i--,w0--,w1--,w2--,d0--,d1--)
        *(w0) = rn0 * *(d0) + rn1 * *(d1)
              - rd1 * *(w1) - rd2 * *(w2);
      
      /*--- calcul final ---*/
      w1 = work1;   w2 = work2;   d0 = out;
      for (i=0 ; i<dim ; i++,w1++,w2++,d0++)
        *d0 = *w1 + *w2;
      
    }
  }
  return( EXIT_ON_SUCCESS );
}

