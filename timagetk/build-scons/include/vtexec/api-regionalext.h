/*************************************************************************
 * api-regionalext.h -
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

#ifndef _api_regionalext_h_
#define _api_regionalext_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <regionalext.h>

#include <vt_typedefs.h>
#include <vt_image.h>







typedef struct lineCmdParamRegionalext {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;

  /* output type for regional minima or maxima
   */
  char binary_output_name[STRINGLENGTH];
  ImageType binary_output_type;

  /* calculation related variables
   */

  enumRegionalExtremum typeExtremum;

  double height;
  double heightMultiplier;

  double heightmin;
  double heightmax;

  int connectivity;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamRegionalext;


extern int API_regionalext( vt_image *image, vt_image *imres, char *param_str_1, char *param_str_2 );



extern char *API_Help_regionalext( int h );

extern void API_ErrorParse_regionalext( char *program, char *str, int flag );

extern void API_InitParam_regionalext( lineCmdParamRegionalext *par );

extern void API_PrintParam_regionalext( FILE *theFile, char *program,
                                         lineCmdParamRegionalext *par,
                                         char *str );

extern void API_ParseParam_regionalext( int firstargc, int argc, char *argv[],
                                 lineCmdParamRegionalext *p );



#ifdef __cplusplus
}
#endif

#endif
