/*************************************************************************
 * api-linearFilter.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer 24 jui 2015 17:34:20 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _api_linearfilter_h_
#define _api_linearfilter_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <vt_typedefs.h>
#include <vt_image.h>
#include <linearFiltering-common.h>



typedef enum enumOutputLinearFilter {
  _FILTER_,
  _GRADIENT_MODULUS_,
  _HESSIAN_,
  _LAPLACIAN_,
  _ZCROSSINGS_HESSIAN_,
  _ZCROSSINGS_LAPLACIAN_,
  _GRADIENT_HESSIAN_,
  _GRADIENT_LAPLACIAN_,
  _EXTREMA_GRADIENT_
} enumOutputLinearFilter;



typedef struct lineCmdParamLinearFilter {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;

  /* filter definitions
   * and computation parameters
   */
  enumOutputLinearFilter typeOutput;
  typeFilteringCoefficients filter[3];
  int borderLengths[3];
  int sliceComputation;


  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamLinearFilter;


extern int API_linearFilter( vt_image *image, vt_image *imres, char *param_str_1, char *param_str_2 );



extern char *API_Help_linearFilter( int h );

extern void API_ErrorParse_linearFilter( char *program, char *str, int flag );

extern void API_InitParam_linearFilter( lineCmdParamLinearFilter *par );

extern void API_PrintParam_linearFilter( FILE *theFile, char *program,
                                         lineCmdParamLinearFilter *par,
                                         char *str );

extern void API_ParseParam_linearFilter( int firstargc, int argc, char *argv[],
                                 lineCmdParamLinearFilter *p );



#ifdef __cplusplus
}
#endif

#endif
