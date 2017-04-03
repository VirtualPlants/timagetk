/*************************************************************************
 * bal-estimator.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <bal-behavior.h>

#include <bal-estimator.h>


static int _verbose_ = 1;



void BAL_SetVerboseInBalEstimator( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalEstimator(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalEstimator(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}



void BAL_InitPropagation( bal_propagation *p )
{
    p->constant = 0.0;
    p->fading = 0.0;
    p->type = TYPE_SKIZ_PROPAGATION;
}



void BAL_InitEstimator( bal_estimator *estimator ) 
{
  estimator->type = TYPE_WLTS;
  estimator->max_iterations = 100;

  estimator->standard_deviation_threshold = -1.0;
  estimator->retained_fraction = 0.500000;

  estimator->sigma.x = 4.0;
  estimator->sigma.y = 4.0;
  estimator->sigma.z = 4.0;

  BAL_InitPropagation( &(estimator->propagation) );
}



void BAL_PrintEstimator( FILE *f, bal_estimator *estimator ) 
{
  fprintf( f, "estimator->type = " );
  switch ( estimator->type ) {
  default : fprintf( f, "unknown\n" ); break;
  case TYPE_LS : fprintf( f, "TYPE_LS\n" ); break;
  case TYPE_WLS : fprintf( f, "TYPE_WLS\n" ); break;
  case TYPE_LTS : fprintf( f, "TYPE_LTS\n" ); break;
  case TYPE_WLTS : fprintf( f, "TYPE_WLTS\n" ); break;
  }
  fprintf( f, "estimator->max_iterations = %d\n", estimator->max_iterations );
  fprintf( f, "estimator->retained_fraction = %f\n", estimator->retained_fraction );
  fprintf( f, "estimator->standard_deviation_threshold = %f\n", estimator->standard_deviation_threshold );
  fprintf( f, "estimator->sigma = [%f %f %f]\n", estimator->sigma.x, estimator->sigma.y, estimator->sigma.z );
  fprintf( f, "estimator->propagation.constant = %f\n", estimator->propagation.constant );
  fprintf( f, "estimator->propagation.fading = %f\n", estimator->propagation.fading );
  fprintf( f, "estimator->propagation.type = " );
  switch ( estimator->propagation.type ) {
  default : fprintf( f, "unknown\n" ); break;
  case TYPE_DIRECT_PROPAGATION : fprintf( f, "TYPE_DIRECT_PROPAGATION\n" ); break;
  case TYPE_SKIZ_PROPAGATION : fprintf( f, "TYPE_SKIZ_PROPAGATION\n" ); break;

  }
}
