/*************************************************************************
 * api-morpho.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sam 18 jul 2015 11:59:27 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _api_morpho_h_
#define _api_morpho_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <morphotools.h>

#include <vt_typedefs.h>
#include <vt_neighborhood.h>
#include <vt_image.h>



typedef struct lineCmdParamMorpho {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;

  /* specific arguments
   */
  char element_name[STRINGLENGTH];

  enumMorphologicalOperation type_operation;
  int binary_mode;

  int nb_iterations;
  Neighborhood neighborhood;

  int radius;
  int euclidean_sphere;
  int chamfer;

  DimType dim;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamMorpho;



extern int API_morpho( vt_image *image,
                       vt_image *imres,
                       typeStructuringElement *userSE,
                       char *param_str_1,
                       char *param_str_2 );



extern char *API_Help_morpho( int h );

extern void API_ErrorParse_morpho( char *program, char *str, int flag );

extern void API_InitParam_morpho( lineCmdParamMorpho *par );

extern void API_PrintParam_morpho( FILE *theFile, char *program,
                                         lineCmdParamMorpho *par,
                                         char *str );

extern void API_ParseParam_morpho( int firstargc, int argc, char *argv[],
                                 lineCmdParamMorpho *p );



#ifdef __cplusplus
}
#endif

#endif
