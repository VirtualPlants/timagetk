/****************************************************
 * regionalext.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Feb  6 08:29:34 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#ifndef _regionalext_h_
#define _regionalext_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <typedefs.h>

typedef enum {
  _REGIONAL_MAX_,
  _REGIONAL_MIN_
} enumRegionalExtremum;

extern void setVerboseInRegionalExt( int v );
extern void incrementVerboseInRegionalExt(  );
extern void decrementVerboseInRegionalExt(  );

void regionalext_setNumberOfPointsForAllocation( int n );



extern int regionalMaxima( void *theInput, void *theOutput, 
                           bufferType theType, int *theDim,
                           int height, double multiplier,
                           int connectivity );

extern int regionalMinima( void *theInput, void *theOutput, 
                           bufferType theType, int *theDim,
                           int height, double multiplier,
                           int connectivity );

extern int regionalHierarchicalMaxima( void *theInput, void *theOutput, 
                                       bufferType theType, int *theDim,
                                       int heightmin, int heightmax,
                                       int connectivity );

extern int regionalHierarchicalMinima( void *theInput, void *theOutput, 
                                       bufferType theType, int *theDim,
                                       int heightmin, int heightmax,
                                       int connectivity );

#ifdef __cplusplus
}
#endif

#endif
