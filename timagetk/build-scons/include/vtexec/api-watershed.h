/*************************************************************************
 * api-watershed.h -
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

#ifndef _api_watershed_h_
#define _api_watershed_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <vt_typedefs.h>
#include <vt_image.h>

#include <watershed.h>



typedef struct lineCmdParamWatershed {

  /* image names and output type
   */
  char input_seeds_name[STRINGLENGTH];
  char input_gradient_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;

  /* watershed controls
   */
  enumWatershedLabelChoice labelChoice;
  int maxIterations;
  int allocatedBunchSize;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamWatershed;



extern int API_watershed( vt_image *gradientImage,
                          vt_image *seedsImage,
                          vt_image *imres,
                          char *param_str_1,
                          char *param_str_2 );



extern char *API_Help_watershed( int h );

extern void API_ErrorParse_watershed( char *program, char *str, int flag );

extern void API_InitParam_watershed( lineCmdParamWatershed *par );

extern void API_PrintParam_watershed( FILE *theFile, char *program,
                                         lineCmdParamWatershed *par,
                                         char *str );

extern void API_ParseParam_watershed( int firstargc, int argc, char *argv[],
                                 lineCmdParamWatershed *p );



#ifdef __cplusplus
}
#endif

#endif
