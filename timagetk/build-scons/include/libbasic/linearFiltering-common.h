/*************************************************************************
 * linearFiltering-common.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Fri Nov 30 00:02:20 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _linearfilteringcommon_h_
#define _linearfilteringcommon_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */




/* The different filter's types.
 *
 * DESCRIPTION:
 *
 * - ALPHA_DERICHE is the first recurvise filter designed
 *   by R. Deriche. See REFERENCES.
 *
 * - with ALPHA_DERICHE's filters, one can either compute
 *   derivatives from order 0 (smoothing) to 3, or extract edges.
 *
 * - GAUSSIAN_DERICHE is a 4th order recursive filter which
 *   approximates the gaussien. See
 *   "Recursively Implementing The Gaussian and Its Derivatives",
 *   R. Deriche, International Conference On Image Processing,
 *   pp 263-267, Singapore, September 1992. Also INRIA research
 *   report.
 *
 * - with GAUSSIAN_DERICHE's filters, one can either compute
 *   derivatives from order 0 (smoothing) to 2, or extract edges.
 *
 * - Extracting edges with ALPHA_DERICHE's filters is faster but
 *   the modulus of the gradient (the estimated height of the step 
 *   edge) depens on the gradient orientation because the filter
 *   is not isotropic. Heights are better estimated with 
 *   GAUSSIAN_DERICHE's filters but they seem not be perfectly
 *   symmetrical.
 *
 * REFERENCES:
 *
 * - "Optimal edge detection using recursive filtering", R. Deriche,
 *   International Journal of Computer Vision, pp 167-187, 1987.
 *
 * - "Recursive filtering and edge tracking: two primary tools
 *    for 3-D edge detection", O. Monga, R. Deriche,
 *   G. Malandain and J.-P. Cocquerez, Image and Vision
 *   Computing 4:9, pp 203-214, August 1991.
 */
typedef enum {
  UNKNOWN_FILTER = 0 /* unknown filter type */,
  ALPHA_DERICHE = 1 /* Deriche's filter (exponential (- alpha |X|)) */,
  GAUSSIAN_DERICHE = 2 /* gaussian approximation (Deriche's coefficients) */,
  GAUSSIAN_FIDRICH = 3 /* gaussian approximation (Fidrich's coefficients) */,
  GAUSSIAN_YOUNG_1995 = 4,
  GAUSSIAN_YOUNG_2002 = 5,
  GABOR_YOUNG_2002 = 6,
  GAUSSIAN_CONVOLUTION = 7
} filterType;



/* Order of the derivative to be computed.
 *
 * DESCRIPTION:
 *
 * - NODERIVATIVE nothing will be done.
 *
 * - DERIVATIVE_0 means smoothing.
 *
 * - DERIVATIVE_1 first derivative. The normalization
 *   of the filter is made so that the response to the
 *   signal i=x will be 1.
 *
 * - DERIVATIVE_1_CONTOURS first derivative but adapted
 *   to edge detections. The normalization of the filter 
 *   is made so that the response to a step edge is 
 *   the step edge height.
 *
 * - DERIVATIVE_2 second derivative. The normalization
 *   of the filter is made so that the response to the
 *   signal i=x*2/2 will be 1.
 *
 * - DERIVATIVE_3 third derivative. The normalization
 *   of the filter is made so that the response to the
 *   signal i=x*3/6 will be 1.
 */
typedef enum {
  NODERIVATIVE  = -1 /* no derivative (no filtering) */,
  DERIVATIVE_0  = 0 /* smoothing */,
  SMOOTHING     = 0 /* smoothing */,
  DERIVATIVE_1  = 1 /* derivative of order 1 */,
  DERIVATIVE_2  = 2 /* derivative of order 2 */,
  DERIVATIVE_3  = 3 /* derivative of order 3 */,
  DERIVATIVE_1_CONTOURS = 11 /* derivative of order 1, normalization adapted to
                                contours. The response to a step-edge is the
                                height of the step. */,
  DERIVATIVE_1_EDGES = 11 /* derivative of order 1, normalization adapted to
                                contours. The response to a step-edge is the
                                height of the step. */
} derivativeOrder;



typedef struct {
  filterType type;
  derivativeOrder derivative;
  double coefficient;
  void *parameters;
} typeFilteringCoefficients;



extern void setVerboseInLinearFilteringCommon( int v );
extern void incrementVerboseInLinearFilteringCommon(  );
extern void decrementVerboseInLinearFilteringCommon(  );

extern void SetConvolution1DSigmaMultiplier( double m );
extern double GetConvolution1DSigmaMultiplier( );

extern int buildFilteringCoefficients( typeFilteringCoefficients *c );

extern void freeFilteringCoefficients( typeFilteringCoefficients *c );
extern void initFilteringCoefficients( typeFilteringCoefficients *c );
extern void printFilteringCoefficients( FILE *theFile, typeFilteringCoefficients *c, char *d );



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _linearfilteringcommon_h_ */
