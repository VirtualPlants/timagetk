/*************************************************************************
 * api-cellfilter.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Dim 19 jul 2015 11:56:28 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _api_cellfilter_h_
#define _api_cellfilter_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <morphotools.h>

#include <vt_typedefs.h>
#include <vt_neighborhood.h>
#include <vt_image.h>



typedef struct lineCmdParamCellfilter {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;


  /* specific arguments for morphological processing
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


  /* specific arguments for post-processing
   */
  int low_threshold;
  int high_threshold;
  int connectivity;
  int min_size;


  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamCellfilter;



extern int API_cellfilter( vt_image *image, vt_image *imres,
                           typeStructuringElement *userSE,
                           char *param_str_1, char *param_str_2 );

extern char *API_Help_cellfilter( int h );

extern void API_ErrorParse_cellfilter( char *program, char *str, int flag );

extern void API_InitParam_cellfilter( lineCmdParamCellfilter *par );

extern void API_PrintParam_cellfilter( FILE *theFile, char *program,
                                         lineCmdParamCellfilter *par,
                                         char *str );

extern void API_ParseParam_cellfilter( int firstargc, int argc, char *argv[],
                                 lineCmdParamCellfilter *p );



#ifdef __cplusplus
}
#endif

#endif
