/*************************************************************************
 * api-intermediaryTrsf.h -
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

#ifndef _api_intermediarytrsf_h_
#define _api_intermediarytrsf_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>

#include <bal-transformation.h>



typedef struct lineCmdParamIntermediaryTrsf {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char template_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];

  /* specific arguments
   */



  int flag_to_0;

  int flag_template;
  bal_integerPoint template_dim;
  bal_doublePoint template_voxel;

  double t;

  /* ... */


  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamIntermediaryTrsf;



extern int API_intermediaryTrsf( bal_transformation *theTrsf,
                                 bal_transformation *resTrsf,
                                 double t,
                                 char *param_str_1,
                                 char *param_str_2 );

extern int API_intermediaryTrsfStuff(bal_transformation *theTrsf,
                                     bal_transformation *resTrsf,
                                     double t,
                                     int flag_to_0);

extern void API_MultiplyTrsf(bal_transformation *trsf, bal_transformation *res, double coef);


extern char *API_Help_intermediaryTrsf( int h );

extern void API_ErrorParse_intermediaryTrsf( char *program, char *str, int flag );

extern void API_InitParam_intermediaryTrsf( lineCmdParamIntermediaryTrsf *par );

extern void API_PrintParam_intermediaryTrsf( FILE *theFile, char *program,
                                         lineCmdParamIntermediaryTrsf *par,
                                         char *str );

extern void API_ParseParam_intermediaryTrsf( int firstargc, int argc, char *argv[],
                                 lineCmdParamIntermediaryTrsf *p );



#ifdef __cplusplus
}
#endif

#endif
