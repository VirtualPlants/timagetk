/****************************************************
 * watershed.h - 
 *
 * $Id: watershed.h,v 1.4 2008/02/26 17:42:51 greg Exp $
 *
 * Copyright (c) INRIA 2001
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Wed May  9 13:30:26 MEST 2001
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#ifndef _watershed_h_
#define _watershed_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <typedefs.h>



typedef enum enumWatershedLabelChoice {
  /* premier 6-voisin rencontre lors de la mise dans la liste */
  _FIRST_ENCOUNTERED_NEIGHBOR_ = 0,
  /* plus petit label */
  _MIN_LABEL_ = 1,
  /* label avec plus de representants */
  _MOST_REPRESENTED_ = 2
} enumWatershedLabelChoice;


extern void setVerboseInWatershed( int v );
extern void incrementVerboseInWatershed(  );
extern void decrementVerboseInWatershed(  );


extern void watershed_setlabelchoice( enumWatershedLabelChoice choice );

extern void watershed_setnoverbose();
extern void watershed_setverbose();

extern void watershed_setNumberOfPointsForAllocation( int n );
extern int watershed_getNumberOfPointsForAllocation();

extern void watershed_setMaxNumberOfIterations( int n );



extern int watershed( void *theGradient, bufferType theGradientType,
		      void *theLabelsInput, void *theLabelsOutput,
		      bufferType theLabelsType,
		      int *theDim );

#ifdef __cplusplus

}
#endif

#endif
