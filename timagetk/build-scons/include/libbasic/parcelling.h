/****************************************************
 * parcelling.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2008
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

#ifndef _parcelling_h_
#define _parcelling_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <typedefs.h>

#include <chamferdistance.h>

extern void setVerboseInParcelling( int v );
extern void incrementVerboseInParcelling(  );
extern void decrementVerboseInParcelling(  );

extern void setDebugInParcelling( int d );
extern void incrementDebugInParcelling(  );
extern void decrementDebugInParcelling(  );

extern long int getRandomSeedInParcelling();
extern void setRandomSeedInParcelling( long int s );



extern void parcelling_setNumberOfPointsForAllocation( int n );

extern void parcelling_setNumberOfIterations( int n );
  
extern void parcelling_ForceExactCenterCalculation();
extern void parcelling_DoNotForceExactCenterCalculation();

extern void parcelling_setPartialUpdate( int a );


extern int parcelling( void *theBuf, bufferType theBufType, 
                         int **theSeeds, int nparcels,
                         void *theOutputLabels, bufferType theOutputLabelsType,
                         void *theOutputDistance, bufferType theOutputDistanceType,
                         int *theDim, int inSeeds, typeChamferMask *theMask );

#ifdef __cplusplus

}
#endif

#endif
