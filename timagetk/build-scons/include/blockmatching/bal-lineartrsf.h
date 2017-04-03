/*************************************************************************
 * bal-lineartrsf.h -
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



#ifndef BAL_LINEARTRSF_H
#define BAL_LINEARTRSF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-stddef.h>

#include <bal-transformation.h>
#include <bal-field.h>
#include <bal-estimator.h>



extern void BAL_SetVerboseInBalLinearTrsf( int v );
extern void BAL_IncrementVerboseInBalLinearTrsf(  );
extern void BAL_DecrementVerboseInBalLinearTrsf(  );

extern int BAL_LinearTrsf_Residuals( bal_transformation* theTrsf, 
				     FIELD * field );

extern int BAL_ComputeLinearTransformation( bal_transformation* theTrsf, 
						 FIELD * field, 
						 bal_estimator *estimator );

#ifdef __cplusplus
}
#endif

#endif
