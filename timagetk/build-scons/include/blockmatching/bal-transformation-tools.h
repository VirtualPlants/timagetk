/*************************************************************************
 * bal-transformation-tools.h -
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



#ifndef BAL_TRANSFORMATION_TOOLS_H
#define BAL_TRANSFORMATION_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string-tools.h>

#include <bal-image.h>
#include <bal-transformation.h>
#include <bal-field.h>
#include <bal-estimator.h>



typedef enum enumTransformationInterpolation {
  NEAREST,
  LINEAR,
  CSPLINE
} enumTransformationInterpolation;

typedef enum enumVectorFieldInverseInitialization {
  ZERO,
  FORWARD_INTERPOLATION,
} enumVectorFieldInverseInitialization;




extern void BAL_SetVerboseInBalTransformationTools( int v );
extern void BAL_IncrementVerboseInBalTransformationTools(  );
extern void BAL_DecrementVerboseInBalTransformationTools(  );
extern void BAL_SetDebugInBalTransformationTools( int v );
extern void BAL_IncrementDebugInBalTransformationTools(  );
extern void BAL_DecrementDebugInBalTransformationTools(  );




/***************************************************
 *
 * transformation from one image to an other
 *
 ***************************************************/

extern int BAL_ComputeImageToImageTransformation( bal_image *subsampled_image,
                                                  bal_image *image_to_be_subsampled,
                                                  bal_transformation *subsampling_trsf );



/***************************************************
 *
 * transformation from a pairing field
 *
 ***************************************************/

extern int BAL_ComputeIncrementalTransformation( bal_transformation *T,  
                                                 FIELD *field,
                                                 bal_estimator *estimator );

extern int BAL_ComputeTransformationResiduals( bal_transformation *T,  
                                               FIELD *field );

/***************************************************
 *
 * transformation composition
 * t_res = t1 o t2 
   t_{I3<-I1} = t_{I3<-I2} o t_{I2<-I1}
 *
 ***************************************************/

extern int BAL_TransformationComposition( bal_transformation *t_res,
                                          bal_transformation *t1,
                                          bal_transformation *t2 );

extern int BAL_TransformationListComposition( bal_transformation *t_res,
                                             bal_transformation **trsfsStructure,
                                             int n );

extern enumTypeTransfo BAL_TypeTransformationComposition( bal_transformation *t1, 
                                                          bal_transformation *t2 );

extern int BAL_AllocTransformationComposition( bal_transformation *res, 
                                               bal_transformation *t1,
                                               bal_transformation *t2,
                                               bal_image *ref );

extern enumTypeTransfo BAL_TypeTransformationListComposition( bal_transformation **array,
                                                              int n );

extern int BAL_AllocTransformationListComposition( bal_transformation *res,
                                                   bal_transformation **array,
                                                   int n,
                                                   bal_image *ref );


/***************************************************
 *
 * transformation use
 *
 ***************************************************/

/* point transformation
 */
extern int BAL_TransformFloatPoint( bal_floatPoint *thePt, bal_floatPoint *resPt, bal_transformation *theTr );

extern int BAL_TransformDoublePoint( bal_doublePoint *thePt, bal_doublePoint *resPt, bal_transformation *theTr );

/* image resampling
 */
extern int BAL_ResampleImage( bal_image *image, bal_image *resim, bal_transformation *theTr,
                              enumTransformationInterpolation interpolation );

extern int BAL_LinearResamplingCoefficients( bal_image *image, bal_image *resim,
                                             bal_transformation *theTr,
                                             enumTransformationInterpolation interpolation,
                                             int index );



/***************************************************
 *
 * transformation conversion
 *
 ***************************************************/
/* unit conversion

   'theTrsf' is the transformation that goes from 'resim' to 'image' 
   ie allows to resample 'image' into the geometry of 'resim'.
   When 'theTrsf' is the transformation issued from matching,
   'resim' is the reference image and 'image' the floating image.
   
*/

extern int BAL_ChangeTransformationToRealUnit( bal_image *image,
                                               bal_image *resim,
                                               bal_transformation *theTrsf,
                                               bal_transformation *resTrsf );

extern int BAL_ChangeTransformationToVoxelUnit( bal_image *image,
                                                bal_image *resim,
                                                bal_transformation *theTrsf,
                                                bal_transformation *resTrsf );



/***************************************************
 *
 * transformation inversion
 *
 ***************************************************/

extern void BAL_SetDerivationSigmaForVectorFieldInversionInBalTransformationTools( double s );
extern double BAL_GetDerivationSigmaForVectorFieldInversionInBalTransformationTools();
extern void BAL_SetIterationsMaxForVectorFieldInversionInBalTransformationTools( int i );
extern int BAL_GetIterationsMaxForVectorFieldInversionInBalTransformationTools( );
extern void BAL_SetErrorMaxForVectorFieldInversionInBalTransformationTools( double e );
extern double BAL_GetErrorMaxForVectorFieldInversionInBalTransformationTools( );
extern void BAL_SetInitializationForVectorFieldInversionInBalTransformationTools( enumVectorFieldInverseInitialization i );
extern enumVectorFieldInverseInitialization BAL_GetInitializationForVectorFieldInversionInBalTransformationTools();
extern void BAL_SetForwardSigmaForVectorFieldInversionInBalTransformationTools( double s );
extern double BAL_GetForwardSigmaForVectorFieldInversionInBalTransformationTools();

extern void BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( bal_image *i );

extern int BAL_InverseTransformation( bal_transformation *theTrsf,  
                                      bal_transformation *invTrsf );



/***************************************************
 *
 * transformation construction
 *
 ***************************************************/

extern void BAL_SetSinusoidAmplitudeForVectorFieldTransformation( double *a );
extern void BAL_SetSinusoidPeriodForVectorFieldTransformation( double *p );

extern int BAL_Sinusoid3DVectorField( bal_transformation *theTrsf );
extern int BAL_Sinusoid2DVectorField( bal_transformation *theTrsf );

/* random transformation 
 */

extern void BAL_SetMinAngleForRandomTransformation( double d );
extern void BAL_SetMaxAngleForRandomTransformation( double d );
extern void BAL_SetMinScaleForRandomTransformation( double d );
extern void BAL_SetMaxScaleForRandomTransformation( double d );
extern void BAL_SetMinShearForRandomTransformation( double d );
extern void BAL_SetMaxShearForRandomTransformation( double d );
extern void BAL_SetMinTranslationForRandomTransformation( double d );
extern void BAL_SetMaxTranslationForRandomTransformation( double d );

extern int BAL_Random2DTranslationMatrix( bal_transformation *theTrsf );
extern int BAL_Random3DTranslationMatrix( bal_transformation *theTrsf );
extern int BAL_Random2DTranslationScalingMatrix( bal_transformation *theTrsf );
extern int BAL_Random3DTranslationScalingMatrix( bal_transformation *theTrsf );
extern int BAL_Random2DRigidMatrix( bal_transformation *theTrsf );
extern int BAL_Random3DRigidMatrix( bal_transformation *theTrsf );
extern int BAL_Random2DSimilitudeMatrix( bal_transformation *theTrsf ) ;
extern int BAL_Random3DSimilitudeMatrix( bal_transformation *theTrsf );
extern int BAL_Random2DAffineMatrix( bal_transformation *theTrsf );
extern int BAL_Random3DAffineMatrix( bal_transformation *theTrsf );
extern int BAL_Random2DVectorField( bal_transformation *theTrsf );
extern int BAL_Random3DVectorField( bal_transformation *theTrsf );
extern int BAL_SetTransformationToRandom( bal_transformation *t );

#ifdef __cplusplus
}
#endif

#endif
