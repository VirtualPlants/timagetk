/****************************************************
 * topological-thickening.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun 29 fév 2016 18:28:00 CET
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#ifndef _topological_thickening_h_
#define _topological_thickening_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <chamferdistance.h>
#include <topological-operations-common.h>


extern void setVerboseInTopologicalThickening( int v );
extern void incrementVerboseInTopologicalThickening();
extern void decrementVerboseInTopologicalThickening();







typedef struct typeThickeningParameters {
  int maxIteration;
  int connectivity;
  typeChamferMask *theMask;
  enumTypeSort additionalSorting;
} typeThickeningParameters;

extern void initTypeThickeningParameters( typeThickeningParameters *p );




extern int InitializeThickeningImage( unsigned char *resBuf,
				      unsigned short *theDistance,
				      int *theDim );

extern int ThickeningImage( unsigned char *resBuf,
			    unsigned short *theDistance,
			    unsigned short *thePropagation,
			    int *theDim,
			    typeThickeningParameters *par );


#ifdef __cplusplus
}
#endif

#endif
