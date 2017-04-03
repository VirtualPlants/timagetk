/*************************************************************************
 * reech4x4-coeff.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mar 23 fév 2016 14:25:09 CET
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#ifndef _reech4x4_coeff_h_
#define _reech4x4_coeff_h_

#ifdef __cplusplus
extern "C" {
#endif





#include <stdlib.h>
#include <stdio.h>



extern void setVerboseInReech4x4Coeff( int v );
extern void incrementVerboseInReech4x4Coeff(  );
extern void decrementVerboseInReech4x4Coeff(  );


extern void Coeff3DTriLin4x4 ( int *theDim, /* dimensions of this buffer */
                               float* resBuf, /* result buffer */
                               int *resDim,  /* dimensions of this buffer */
                               double* mat,   /* transformation matrix */
                               int index
                               );

extern void Coeff2DTriLin4x4 ( int *theDim, /* dimensions of this buffer */
                               float* resBuf, /* result buffer */
                               int *resDim,  /* dimensions of this buffer */
                               double* mat,   /* transformation matrix */
                               int index
                               );



#ifdef __cplusplus
}
#endif


#endif
