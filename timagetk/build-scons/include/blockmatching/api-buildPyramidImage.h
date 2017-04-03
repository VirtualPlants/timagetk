/*************************************************************************
 * api-buildPyramidImage.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Jeu  1 oct 2015 14:43:43 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _api_buildpyramidimage_h_
#define _api_buildpyramidimage_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>
#include <string-tools.h>

#include <bal-image.h>



typedef struct lineCmdParamBuildPyramidImage {

  /* image names and output type
   */
  char input_image_name[STRINGLENGTH];

  char output_image_format[STRINGLENGTH];
  char output_image_prefix[STRINGLENGTH];
  char output_image_suffix[STRINGLENGTH];

  char output_trsf_format[STRINGLENGTH];
  char output_trsf_prefix[STRINGLENGTH];
  char output_trsf_suffix[STRINGLENGTH];

  /* specific arguments
   */

  int normalisation;

  int pyramid_lowest_level;
  int pyramid_highest_level;
  int pyramid_gaussian_filtering;

  int use_default_filename;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamBuildPyramidImage;



extern int API_buildPyramidImage( bal_image *image,
                                  stringList *image_names,
                                  stringList *trsfs_names,
                                  char *param_str_1, char *param_str_2 );


extern char *API_Help_buildPyramidImage( int h );

extern void API_ErrorParse_buildPyramidImage( char *program, char *str, int flag );

extern void API_InitParam_buildPyramidImage( lineCmdParamBuildPyramidImage *par );

extern void API_PrintParam_buildPyramidImage( FILE *theFile, char *program,
                                         lineCmdParamBuildPyramidImage *par,
                                         char *str );

extern void API_ParseParam_buildPyramidImage( int firstargc, int argc, char *argv[],
                                 lineCmdParamBuildPyramidImage *p );



#ifdef __cplusplus
}
#endif

#endif
