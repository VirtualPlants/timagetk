/*************************************************************************
 * bal-estimator.h -
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



#ifndef BAL_ESTIMATOR_H
#define BAL_ESTIMATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include <bal-stddef.h>



extern void BAL_SetVerboseInBalEstimator( int v );
extern void BAL_IncrementVerboseInBalEstimator(  );
extern void BAL_DecrementVerboseInBalEstimator(  );




/****************************************
 * 
 * transformation estimation
 *
 ****************************************/
/***
    Different types of estimator
***/
typedef enum enumTypeEstimator {
  TYPE_LS,   /* least squares */
  TYPE_WLS,  /* weighted least squares */
  TYPE_LTS,  /* least trimmed squares */
  TYPE_WLTS  /* weighted least trimmed squares */
} enumTypeEstimator;


typedef enum enumTypePropagation {
  TYPE_DIRECT_PROPAGATION,
  TYPE_SKIZ_PROPAGATION
} enumTypePropagation;

typedef struct bal_propagation {
  float constant;
  float fading;
  enumTypePropagation type;
} bal_propagation;

typedef struct bal_estimator {
  enumTypeEstimator type;
  
  /* maximal number of iterations
     for trimmed estimation
  */
  int max_iterations;

  /*************************************************************
   * Pairs selection.
     There are two methods:
     1. large values are discarded based on the (assumed normal) distribution
        parameters
     2. the pairs associated to the largest residuals are discarded
   */

  /* threshold to discard outliers from residuals
     out of the range 'average +/- threshold * standard deviation' 
     Does nothing if negative
  */
  double standard_deviation_threshold;
  
  /* retained fraction of samples for 
     trimmed estimation */
  double retained_fraction;
 
  
  /* standard deviation for vector field smoothing
   * ie fluid regularization
   */
  bal_doublePoint sigma;

  /* vector propagation
   * (vector field estimation from pairs of points)
   */
  bal_propagation propagation;

} bal_estimator;





/****************************************
 * 
 * these structures allows to defines parameter values for both 
 * the highest and the lowest levels of the pyramid.
 * Recall the level #0 is the original image.
 *Values for the in-between levels will be linearly interpolated.miscellaneous structures
 *
 ****************************************/

typedef struct {
  bal_estimator lowest;
  bal_estimator highest;
} bal_pyramidEstimator;





extern void BAL_InitEstimator( bal_estimator *estimator );
extern void BAL_PrintEstimator( FILE *f, bal_estimator *estimator );

#ifdef __cplusplus
}
#endif

#endif
