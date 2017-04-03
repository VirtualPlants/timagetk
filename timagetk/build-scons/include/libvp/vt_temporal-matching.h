/*************************************************************************
 * vt_temporal-matching.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun 30 sep 2013 21:36:23 CEST
 *
 * ADDITIONS, CHANGES
 *
 */


#ifndef _vt_temporal_matching_h_
#define _vt_temporal_matching_h_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
  float measure;
  float time;
} measureType;   




typedef struct {
  measureType *data;

  float dt;

  int n_data;
  int n_allocated_data;

} measureListType;






extern void initmeasureList( measureListType *l );
extern void freeMeasureList( measureListType *l );
extern void fprintfMeasureList( FILE *f, measureListType *l );
extern int readMeasureList(  measureListType *l, char *filename );
extern int extractMeasureMaxima(  measureListType *max,  measureListType *l );
extern int temporalRegistration( measureListType *ref,
			  measureListType *flo,
			  int leftBoundary,
			  int rightBoundary,
			  int maxShift );


extern void writeScilab( measureListType *theOriginalRef,
			 measureListType *theOriginalFlo,
			 char *name );

#ifdef __cplusplus
}
#endif

#endif 
