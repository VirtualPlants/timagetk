/*************************************************************************
 * chamferdistance-mask.c - computation of chamfer masks
 *
 * $Id$
 *
 * Copyright (c) INRIA 2014
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Jeu  5 jui 2014 15:49:09 CEST
 *
 * ADDITIONS, CHANGES
 *
 * - 
 *
 *
 *
 *
 */



#ifndef _CHAMFERDISTANCE_MASK_H_
#define _CHAMFERDISTANCE_MASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <chamferdistance.h>



extern void setVerboseInChamferDistanceMask( int v );
extern void incrementVerboseInChamferDistanceMask(  );
extern void decrementVerboseInChamferDistanceMask(  );



extern int buildChamferMask( double *theVoxelSize, /* must be 3 values */
		      int theMaskDim, /* 2 or 3 */
		      int theMaskSize, /* size of mask: 3, 5, 7, etc. */
		      int maxvalueofmincoefficient, /* maximal value for the minimal coefficient value */
		      enumDistance typePredefinedMask, /* predefined masks */
		      typeChamferMask *theMask );



#ifdef __cplusplus
}
#endif

#endif
