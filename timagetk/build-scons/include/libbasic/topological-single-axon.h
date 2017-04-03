/****************************************************
 * topological-single-axon.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Lun  7 mar 2016 22:28:25 CET
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#ifndef _topological_single_axon_h_
#define _topological_single_axon_h_

#ifdef __cplusplus
extern "C" {
#endif




extern void setVerboseInTopologicalSingleAxon( int v );
extern void incrementVerboseInTopologicalSingleAxon( );
extern void decrementVerboseInTopologicalSingleAxon( );
extern void setDebugInTopologicalSingleAxon( int v );
extern void incrementDebugInTopologicalSingleAxon( );
extern void decrementDebugInTopologicalSingleAxon( );









typedef struct typeSingleAxonParameters {
  int low_threshold;
  int high_threshold;
  int stepToStop;
} typeSingleAxonParameters;



extern void initTypeSingleAxonParameters( typeSingleAxonParameters *p );

extern int treePropagation( unsigned char *theBuf,
                            void *theValues, bufferType theType, int *theDim,
                            typeSingleAxonParameters *p );






#ifdef __cplusplus
}
#endif

#endif
