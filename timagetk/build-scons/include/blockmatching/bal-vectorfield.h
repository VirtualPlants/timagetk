/*************************************************************************
 * bal-vectorfield.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#ifndef BAL_VECTORFIELD_H
#define BAL_VECTORFIELD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-stddef.h>

#include <bal-transformation.h>
#include <bal-field.h>
#include <bal-estimator.h>


extern void BAL_SetVerboseInBalVectorField( int v );
extern void BAL_IncrementVerboseInBalVectorField(  );
extern void BAL_DecrementVerboseInBalVectorField(  );


extern int BAL_SmoothVectorField( bal_transformation* theTrsf, bal_doublePoint *theSigma );

extern int BAL_VectorField_Residuals( bal_transformation* theTrsf, 
				      FIELD * field );

extern int BAL_ComputeVectorFieldTransformation( bal_transformation* theTrsf, 
						 FIELD * field, 
						 bal_estimator *estimator );

#ifdef __cplusplus
}
#endif

#endif
