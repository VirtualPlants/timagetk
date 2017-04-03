/*************************************************************************
 * api-interpolateTrsfs.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gael Michelin (gael.michelin@inria.fr)
 *
 * CREATION DATE:
 * Mer  2 sep 2015 11:37:10 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _api_interpolatetrsfs_h_
#define _api_interpolatetrsfs_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>

#include <bal-transformation.h>



typedef struct lineCmdParamInterpolateTrsfs {

  /* image names, formats and output type
   */
  char input_name[STRINGLENGTH];
  char output_format_0[STRINGLENGTH];
  char output_format_1[STRINGLENGTH];

  /* specific arguments
   */
  char template_name[STRINGLENGTH];
  bal_integerPoint template_dim;
  bal_doublePoint template_voxel;

  int n_img;

  /* vector field inversion parameters
   */
  char error_name[STRINGLENGTH];

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamInterpolateTrsfs;




/* this API is kept for historical reasons
 * ie compatibility with STracking

extern int invTrsf( char* thetrsf_name,
                    char* restrsf_name,
                    char* template_image_name,
                    bal_integerPoint dim,
                    bal_doublePoint voxel,
                    int verbose );
 */

/* this API read the input files and then call the following one
 */
extern int API_INTERMEDIARY_interpolateTrsfs( char* thetrsf_name,
                                     char* template_image_name,
                                     char *param_str_1, char *param_str_2 );


/* this API does the job, ie invert the read input
 * transformation into the allocated output one
 */
extern int API_interpolateTrsfs( bal_transformation *theTrsf,
                        bal_transformation *resTrsf,
                        char *param_str_1,
                        char *param_str_2 );

extern void API_MultiplyTrsf(bal_transformation *trsf, bal_transformation *res, double coef);

extern char *API_Help_interpolateTrsfs( int h );

extern void API_ErrorParse_interpolateTrsfs( char *program, char *str, int flag );

extern void API_InitParam_interpolateTrsfs( lineCmdParamInterpolateTrsfs *par );

extern void API_PrintParam_interpolateTrsfs( FILE *theFile, char *program,
                                         lineCmdParamInterpolateTrsfs *par,
                                         char *str );

extern void API_ParseParam_interpolateTrsfs( int firstargc, int argc, char *argv[],
                                 lineCmdParamInterpolateTrsfs *p );



#ifdef __cplusplus
}
#endif

#endif
