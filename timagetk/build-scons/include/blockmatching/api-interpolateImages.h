/*************************************************************************
 * api-interpolateImages.h -
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

#ifndef _api_interpolateimages_h_
#define _api_interpolateimages_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>

#include <bal-image.h>
#include <bal-transformation.h>

#include <bal-transformation-tools.h>


typedef struct lineCmdParamInterpolateImages {

  /* input image names and output images format and type
   */
  char input_name_0[STRINGLENGTH];
  char input_name_1[STRINGLENGTH];
  char input_trsf[STRINGLENGTH];
  char template_name[STRINGLENGTH];
  char output_format[STRINGLENGTH];
  ImageType output_type;

  /* specific arguments
   */
  int n_img;

  enumTransformationInterpolation interpolation;

  int flag_template;
  bal_integerPoint template_dim;
  bal_doublePoint template_voxel;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamInterpolateImages;



extern int API_interpolateImages( bal_image *image0,
                             bal_image *image1,
                             bal_transformation *T_1_t,
                             bal_image *imres,
                             double t,
                             char *param_str_1,
                             char *param_str_2 );


extern int API_LinearCombination(bal_image *I_0, bal_image *I_1, bal_image *imres, double t);

extern char *API_Help_interpolateImages( int h );

extern void API_ErrorParse_interpolateImages( char *program, char *str, int flag );

extern void API_InitParam_interpolateImages( lineCmdParamInterpolateImages *par );

extern void API_PrintParam_interpolateImages( FILE *theFile, char *program,
                                         lineCmdParamInterpolateImages *par,
                                         char *str );

extern void API_ParseParam_interpolateImages( int firstargc, int argc, char *argv[],
                                 lineCmdParamInterpolateImages *p );



#ifdef __cplusplus
}
#endif

#endif
