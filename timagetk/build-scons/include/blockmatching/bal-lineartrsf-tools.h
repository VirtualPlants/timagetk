/*************************************************************************
 * bal-lineartrsf-tools.h -
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



#ifndef BAL_LINEARTRSF_TOOLS_H
#define BAL_LINEARTRSF_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

void _SetRandomSeed( long int seed );



void _IdentityMatrix( double *mat );


void _Random2DTranslationVector( double *v,
				 double *translation_interval );
void _Random2DRotationMatrix( double *mat,
			      double *angle_interval );
void _Random2DScaleMatrix( double *mat,
			   double *scale_interval );
void _Random2DSimilitudeMatrix( double *mat,
				double *angle_interval,
				double *scale_interval );
void _Random2DAffineMatrix( double *mat,
			    double *angle_interval,
			    double *scale_interval,
			    double *shear_interval );

void _Random3DTranslationVector( double *v,
				 double *translation_interval );
void _Random3DRotationMatrix( double *mat,
			      double *angle_interval );
void _Random3DScaleMatrix( double *mat,
			   double *scale_interval );
void _Random3DSimilitudeMatrix( double *mat,
				double *angle_interval,
				double *scale_interval );
void _Random3DAffineMatrix( double *mat,
			    double *angle_interval,
			    double *scale_interval,
			    double *shear_interval );

#ifdef __cplusplus
}
#endif

#endif
