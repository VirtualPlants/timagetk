/*************************************************************************
 * api-composeTrsf.h -
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

#ifndef _api_composetrsf_h_
#define _api_composetrsf_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <string-tools.h>

#include <typedefs.h>

#include <bal-transformation.h>



typedef enum typeComposeTrsfComputation {
  _MEMORY_,
  _STREAMING_
} typeComposeTrsfComputation;



typedef struct lineCmdParamComposeTrsf {

  /* image names and output type
   */
  stringList input_names;
  char input_list[STRINGLENGTH];
  char input_format[STRINGLENGTH];

  int firstindex;
  int lastindex;

  char output_name[STRINGLENGTH];

  /* specific arguments
   */
  typeComposeTrsfComputation computation;

  /* template
   */
  char template_name[STRINGLENGTH];
  bal_integerPoint template_dim;
  bal_doublePoint template_voxel;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamComposeTrsf;

/* this API is kept for historical reasons
 * ie compatibility with STracking
 */
extern int composeTrsf( char *restrsf_name,
                        char *template_image_name,
                        bal_integerPoint dim,
                        bal_doublePoint voxel,
                        /*
                         * int first_trsf,
                         * int last_trsf,
                         */
                        char** argv,
                        int argc,
                        int* is_a_trsf,
                        int isDebug,
                        int isVerbose );

extern int API_INTERMEDIARY_composeTrsf( stringList *image_names,
                                         char *output_name,
                                         char* template_image_name,
                                         char *param_str_1, char *param_str_2 );

extern int API_composeTrsf( bal_transformation **trsfsStructure,
                            int n,
                            bal_transformation *resTrsf,
                            char *param_str_1, char *param_str_2 );


extern char *API_Help_composeTrsf( int h );

extern void API_ErrorParse_composeTrsf( char *program, char *str, int flag );

extern void API_InitParam_composeTrsf( lineCmdParamComposeTrsf *par );

extern void API_FreeParam_composeTrsf( lineCmdParamComposeTrsf *p );

extern void API_PrintParam_composeTrsf( FILE *theFile, char *program,
                                         lineCmdParamComposeTrsf *par,
                                         char *str );

extern void API_ParseParam_composeTrsf( int firstargc, int argc, char *argv[],
                                 lineCmdParamComposeTrsf *p );



#ifdef __cplusplus
}
#endif

#endif
