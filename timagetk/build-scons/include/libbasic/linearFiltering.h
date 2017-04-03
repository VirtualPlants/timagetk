/*************************************************************************
 * linearFiltering.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Nov 28 11:49:01 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _linearfiltering_h_
#define _linearfiltering_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <typedefs.h>

#include <linearFiltering-common.h>


extern void setVerboseInLinearFiltering( int v );
extern void incrementVerboseInLinearFiltering(  );
extern void decrementVerboseInLinearFiltering(  );

extern void setDebugInLinearFiltering( int v );
extern void incrementDebugInLinearFiltering(  );
extern void decrementDebugInLinearFiltering(  );


#include "linearFiltering-gradient.h"
#include "linearFiltering-hessian.h"
#include "linearFiltering-laplacian.h"


extern int separableLinearFiltering( void *bufferIn,
                                     bufferType typeIn,
                                     void *bufferOut,
                                     bufferType typeOut,
                                     int *bufferDims,
                                     int *borderLengths,
                                     typeFilteringCoefficients *filter );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _linearfiltering_h_ */
