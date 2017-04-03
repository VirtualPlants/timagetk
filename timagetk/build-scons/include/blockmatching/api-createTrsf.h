/*************************************************************************
 * api-createTrsf.h -
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

#ifndef _api_createtrsf_h_
#define _api_createtrsf_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>

#include <bal-transformation.h>


typedef enum {
  _RANDOM_,
  _IDENTITY_,
  _SINUS_2D_,
  _SINUS_3D_
} enumValueTransfo;



typedef struct lineCmdParamCreateTrsf {

  /* image names and output type
   */
  char output_name[STRINGLENGTH];

  enumTypeTransfo transformation_type;

  /* specific arguments
   */
  char template_name[STRINGLENGTH];
  bal_integerPoint template_dim;
  bal_doublePoint template_voxel;

  enumValueTransfo transformation_value;

  char template_fixedpoint[STRINGLENGTH];
  bal_doublePoint fixedpoint;

  int print_transformation;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamCreateTrsf;

extern int createRandomTrsf( char *restrsf_name,
                             char *template_image_name,
                             bal_doublePoint fixedpoint,
                             enumTypeTransfo transformation_type,
                             int print );

extern int createVectorTrsf( char *restrsf_name,
                             char *template_image_name,
                             bal_integerPoint dim,
                             bal_doublePoint voxel,
                             enumTypeTransfo transformation_type,
                             enumValueTransfo vector_type,
                             int isDebug,
                             int isVerbose );

extern int API_INTERMEDIARY_createTrsf( char *output_name,
                                        char *template_vector,
                                        char *template_fixedpoint,
                                        char *param_str_1, char *param_str_2 );

extern int API_createTrsf( bal_transformation *restrsf,
                           bal_image *template_fixedpoint,
                           char *param_str_1, char *param_str_2 );



extern char *API_Help_createTrsf( int h );

extern void API_ErrorParse_createTrsf( char *program, char *str, int flag );

extern void API_InitParam_createTrsf( lineCmdParamCreateTrsf *par );

extern void API_PrintParam_createTrsf( FILE *theFile, char *program,
                                         lineCmdParamCreateTrsf *par,
                                         char *str );

extern void API_ParseParam_createTrsf( int firstargc, int argc, char *argv[],
                                 lineCmdParamCreateTrsf *p );



#ifdef __cplusplus
}
#endif

#endif
