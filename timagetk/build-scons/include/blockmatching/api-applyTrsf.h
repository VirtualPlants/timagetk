/*************************************************************************
 * api-applyTrsf.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Jeu  2 jul 2015 09:43:10 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _api_applytrsf_h_
#define _api_applytrsf_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>

#include <bal-image.h>
#include <bal-transformation-tools.h>


typedef struct lineCmdParamApplyTrsf {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  ImageType output_type;



  /* transformation
   */
  char input_real_transformation[STRINGLENGTH];
  char input_voxel_transformation[STRINGLENGTH];

  char output_real_transformation[STRINGLENGTH];
  char output_voxel_transformation[STRINGLENGTH];

  /* template
   */
  char template_name[STRINGLENGTH];
  bal_integerPoint template_dim;
  bal_doublePoint template_voxel;

  /* specific arguments
   */
  int resize;
  enumTransformationInterpolation interpolation;

  /* linear resampling coefficient
   */
  char output_coefficient_name[STRINGLENGTH];
  int coefficient_index;

  /* transformation modulus
   */
  char output_trsf_modulus[STRINGLENGTH];

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamApplyTrsf;



/* this API is kept for historical reasons
 * ie compatibility with STracking
 */
extern int applyTrsf( char *theim_name,
                      char *resim_name,
                      char *real_transformation_name,
                      char *voxel_transformation_name,
                      char *result_real_transformation_name,
                      char *result_voxel_transformation_name,
                      char *template_image_name,
                      bal_integerPoint dim,
                      bal_floatPoint voxel,
                      int resize,
                      enumTransformationInterpolation interpolation,
                      ImageType type,
                      int isDebug,
                      int isVerbose );

extern int API_INTERMEDIARY_applyTrsf( char* theimage_name,
                                       char* resimage_name,
                                       char* template_image_name,
                                       char *real_transformation_name,
                                       char *voxel_transformation_name,
                                       char *result_real_transformation_name,
                                       char *result_voxel_transformation_name,
                                       char *param_str_1, char *param_str_2 );

extern int API_applyTrsf( bal_image *image,
                          bal_image *imres,
                          bal_transformation *trsf,
                             char *param_str_1,
                             char *param_str_2 );



/* computation of images of linear resampling coefficients
 */

extern int API_coeffTrsf( bal_image *image, bal_image *imres,
                          bal_transformation *trsf,
                          char *param_str_1, char *param_str_2 );



/* computation of images of transformation modulus
 * = image of point displacement modulus
 */

extern int API_amplitudeTrsf( bal_image *imres,
                            bal_transformation *trsf );



extern char *API_Help_applyTrsf( int h );

extern void API_ErrorParse_applyTrsf( char *program, char *str, int flag );

extern void API_InitParam_applyTrsf( lineCmdParamApplyTrsf *par );

extern void API_PrintParam_applyTrsf( FILE *theFile, char *program,
                                         lineCmdParamApplyTrsf *par,
                                         char *str );

extern void API_ParseParam_applyTrsf( int firstargc, int argc, char *argv[],
                                 lineCmdParamApplyTrsf *p );



#ifdef __cplusplus
}
#endif

#endif
