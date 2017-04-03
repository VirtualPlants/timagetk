/*************************************************************************
 * api-invTrsf.h -
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

#ifndef _api_invtrsf_h_
#define _api_invtrsf_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>

#include <bal-transformation.h>



typedef struct lineCmdParamInvTrsf {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];

  /* specific arguments
   */
  char template_name[STRINGLENGTH];
  bal_integerPoint template_dim;
  bal_doublePoint template_voxel;

  /* vector field inversion parameters
   */
  char error_name[STRINGLENGTH];

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamInvTrsf;




/* this API is kept for historical reasons
 * ie compatibility with STracking
 */
extern int invTrsf( char* thetrsf_name,
                    char* restrsf_name,
                    char* template_image_name,
                    bal_integerPoint dim,
                    bal_doublePoint voxel,
                    int verbose );

/* this API read the input files and then call the following one
 */
extern int API_INTERMEDIARY_invTrsf( char* thetrsf_name,
                                     char* restrsf_name,
                                     char* template_image_name,
                                     char *param_str_1, char *param_str_2 );

/* this API does the job, ie invert the read input
 * transformation into the allocated output one
 */
extern int API_invTrsf( bal_transformation *theTrsf,
                        bal_transformation *resTrsf,
                        char *param_str_1,
                        char *param_str_2 );



extern char *API_Help_invTrsf( int h );

extern void API_ErrorParse_invTrsf( char *program, char *str, int flag );

extern void API_InitParam_invTrsf( lineCmdParamInvTrsf *par );

extern void API_PrintParam_invTrsf( FILE *theFile, char *program,
                                         lineCmdParamInvTrsf *par,
                                         char *str );

extern void API_ParseParam_invTrsf( int firstargc, int argc, char *argv[],
                                 lineCmdParamInvTrsf *p );



#ifdef __cplusplus
}
#endif

#endif
