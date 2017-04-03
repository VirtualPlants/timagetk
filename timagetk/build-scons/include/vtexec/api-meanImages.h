/*************************************************************************
 * api-meanImages.h -
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

#ifndef _api_meanimages_h_
#define _api_meanimages_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <string-tools.h>

#include <vt_typedefs.h>
#include <vt_image.h>



typedef enum typeMeanImagesOperation {
  _MAX_,
  _MEAN_,
  _MEDIAN_,
  _MIN_,
  _QUANTILE_,
  _ROBUST_MEAN_,
  _STDDEV_,
  _SUM_,
  _VAR_
} typeMeanImagesOperation;



typedef enum typeMeanImagesComputation {
  _MEMORY_,
  _STREAMING_
} typeMeanImagesComputation;



typedef struct lineCmdParamMeanImages {

  /* image names and output type
   */
  stringList input_names;
  char input_list[STRINGLENGTH];
  char input_format[STRINGLENGTH];

  stringList mask_names;
  char mask_list[STRINGLENGTH];
  char mask_format[STRINGLENGTH];

  int firstindex;
  int lastindex;

  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;

  /* specific arguments
   */

  typeMeanImagesOperation operation;

  vt_ipt window;
  double quantile; /* 0: min, 0.5: median, 1:max */
  double lts_fraction; /* samples to be kept */

  typeMeanImagesComputation computation;


  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamMeanImages;


extern int API_INTERMEDIARY_meanImages( stringList *image_names,
                                     stringList *mask_names,
                                     char *output_name,
                                     char *param_str_1, char *param_str_2 );

extern int API_meanImages( vt_image **imageStructure,
                           vt_image **maskStructure,
                           int n,
                           vt_image *imres, char *param_str_1, char *param_str_2 );



extern char *API_Help_meanImages( int h );

extern void API_ErrorParse_meanImages( char *program, char *str, int flag );

extern void API_InitParam_meanImages( lineCmdParamMeanImages *par );

extern void API_FreeParam_meanImages( lineCmdParamMeanImages *par );

extern void API_PrintParam_meanImages( FILE *theFile, char *program,
                                         lineCmdParamMeanImages *par,
                                         char *str );

extern void API_ParseParam_meanImages( int firstargc, int argc, char *argv[],
                                 lineCmdParamMeanImages *p );



#ifdef __cplusplus
}
#endif

#endif
