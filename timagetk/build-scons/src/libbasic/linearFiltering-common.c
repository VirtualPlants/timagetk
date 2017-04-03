/*************************************************************************
 * linearFiltering-tools.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Thu Nov 29 23:15:10 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <convolution1D.h>
#include <recline.h>
#include <linearFiltering-common.h>

static int _verbose_ = 1;
static double _sigma_multiplier_ = 3.0;



void setVerboseInLinearFilteringCommon( int v )
{
  _verbose_ = v;
}

void incrementVerboseInLinearFilteringCommon(  )
{
  _verbose_ ++;
}

void decrementVerboseInLinearFilteringCommon(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}



void SetConvolution1DSigmaMultiplier( double m ) 
{
  if ( m >= 1.0 ) _sigma_multiplier_ = m;
}

double GetConvolution1DSigmaMultiplier( ) 
{
  return( _sigma_multiplier_ );
}



int buildFilteringCoefficients( typeFilteringCoefficients *c )
{
  char *proc = "buildFilteringCoefficients";
  type1DConvolutionMask *convolution = NULL;
  
  if ( c->coefficient <= 0.0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: null or negative coefficient\n", proc );
    return( -1 );
  }

  if ( c->type == UNKNOWN_FILTER ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unknown filter type\n", proc );
    return( -1 );
  }

  if ( c->derivative == NODERIVATIVE ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unknown derivative order\n", proc );
    return( -1 );
  }

  switch( c->type ) {
  default :
  case UNKNOWN_FILTER :
    if ( _verbose_ )
        fprintf( stderr, "%s: filter type not handled yet\n", proc );
      return( 0 );
  case ALPHA_DERICHE :
  case GAUSSIAN_DERICHE :
  case GAUSSIAN_FIDRICH :
  case GAUSSIAN_YOUNG_1995 :
  case GAUSSIAN_YOUNG_2002 :
  case GABOR_YOUNG_2002 :
    c->parameters = (void*)InitRecursiveCoefficients( c->coefficient, c->type, c->derivative );
    if ( c->parameters == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when computing recursive filter values\n", proc );
      return( 0 );
    }
    return( 1 );

  case GAUSSIAN_CONVOLUTION :
    convolution = (type1DConvolutionMask *)malloc( sizeof( type1DConvolutionMask ) );
    if ( convolution == (type1DConvolutionMask *)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate convolutionMask structure\n", proc );
      return( -1 );
    }
    /* magic formula
       halflength = C * sigma + 1 with C to be chosen in [3 ... 6]
    */
    convolution->halflength = (int)(_sigma_multiplier_ * c->coefficient + 1 + 0.5);
    convolution->length = 2 * convolution->halflength + 1;
    convolution->data = _build1DGaussianMask( c->coefficient, 
                                              convolution->length, c->derivative );
    if ( convolution->data == (double*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when computing mask values\n", proc );
      free( convolution );
      return( 0 );
    }
    c->parameters = (void*)convolution;
    return( 1 );
  }
  return( 0 );
}



void freeFilteringCoefficients( typeFilteringCoefficients *c )
{
  char *proc = "freeFilteringCoefficients";

  switch( c->type ) {
  default :
  case UNKNOWN_FILTER :
    if ( _verbose_ )
      fprintf( stderr, "%s: filter type not handled yet\n", proc );
    return;
  case ALPHA_DERICHE :
  case GAUSSIAN_DERICHE :
  case GAUSSIAN_FIDRICH :
  case GAUSSIAN_YOUNG_1995 :
  case GAUSSIAN_YOUNG_2002 :
  case GABOR_YOUNG_2002 :
    if ( c->parameters != NULL ) {
      free( c->parameters );
      c->parameters = NULL;
    }
    break;
  case GAUSSIAN_CONVOLUTION :
    if ( c->parameters != NULL ) {
      free1DConvolutionMask( (type1DConvolutionMask *)c->parameters );
      free( c->parameters );
      c->parameters = NULL;
    }
    break;
  }

  return;
}



void initFilteringCoefficients( typeFilteringCoefficients *c )
{
  c->type = UNKNOWN_FILTER;
  c->type = GAUSSIAN_YOUNG_2002;
  c->derivative = NODERIVATIVE;
  c->coefficient = 1.0;
  c->parameters = (void*)NULL;
} 



void printFilteringCoefficients( FILE *theFile, typeFilteringCoefficients *c, char *d )
{
  char *proc = "printFilteringCoefficients";
  FILE *f;

  f =  ( theFile == (FILE*)NULL ) ? stdout : theFile;

  if ( d != (char*)NULL ) 
    fprintf( f, " %s: %s\n", proc, d );
  
  fprintf( f, " - filter type: " );
  switch( c->type ) {
  default :
  case UNKNOWN_FILTER :
    fprintf( f, "unknown\n" ); break;
  case ALPHA_DERICHE :
    fprintf( f, "Deriche's recursive filter, ie exp(- alpha *|x|)\n" ); break;
  case GAUSSIAN_DERICHE :
    fprintf( f, "Deriche's recursive approximation of the gaussian\n" ); break; 
  case GAUSSIAN_FIDRICH :
    fprintf( f, "Fidrich's recursive approximation of the gaussian\n" ); break; 
  case GAUSSIAN_YOUNG_1995 :
    fprintf( f, "Young's recursive approximation of the gaussian (1995)\n" ); break; 
  case GAUSSIAN_YOUNG_2002 :
    fprintf( f, "Young's recursive approximation of the gaussian (2002)\n" ); break; 
  case GABOR_YOUNG_2002 :
    fprintf( f, "Young's recursive approximation of Gabor's filter (2002)\n" ); break; 
  case GAUSSIAN_CONVOLUTION :
    fprintf( f, "convolution with a discrete gaussian\n" ); break;
  }
  
  fprintf( f, " - derivative order: " );
  switch( c->derivative ) {
  default :
  case NODERIVATIVE :
    fprintf( f, "no derivative\n" ); break;
  case DERIVATIVE_0 :
    fprintf( f, "smoothing\n" ); break;
  case DERIVATIVE_1 :
    fprintf( f, "first derivative\n" ); break;
  case DERIVATIVE_2 :
    fprintf( f, "second derivative\n" ); break;
  case DERIVATIVE_3 :
    fprintf( f, "third derivative\n" ); break;
  case DERIVATIVE_1_CONTOURS :
    fprintf( f, "first derivative, normalized for edges\n" ); break;
  }

  fprintf( f, " - coefficient: %lf\n", c->coefficient );
  fprintf( f, " - parameters:\n" );
  if ( c->parameters == NULL ) {
    fprintf( f, "   NULL parameters\n" );
  }
  else {
    switch( c->type ) {
    default :
    case UNKNOWN_FILTER :
    fprintf( f, "   unknown\n" ); break;
    case ALPHA_DERICHE :
    case GAUSSIAN_DERICHE :
    case GAUSSIAN_FIDRICH :
    case GAUSSIAN_YOUNG_1995 :
    case GAUSSIAN_YOUNG_2002 :
    case GABOR_YOUNG_2002 :
      printRecursiveCoefficients( f, (RFcoefficientType *)(c->parameters), (char*)NULL );
      break;
    case GAUSSIAN_CONVOLUTION :
      print1DConvolutionMask( f, (type1DConvolutionMask *)(c->parameters), (char*)NULL );
      break;
    }
  }
  
}
