/*************************************************************************
 * api-execTemplate.h -
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
 * to generate files:
 * sed -e "s/execTemplate/execuTable/g" \
 *     -e "s/ExecTemplate/ExecuTable/g" \
 *     -e "s/exectemplate/executable/g" \
 *     [api-]execTemplate.[c,h] > [api-]execTable.[c,h]
 *
 */

#ifndef _api_exectemplate_h_
#define _api_exectemplate_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <vt_typedefs.h>
#include <vt_image.h>



typedef struct lineCmdParamExecTemplate {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;

  /* specific arguments
   */

  /* ... */

  /* general parameters
   */
  int allow_stdin_stdout;
  int print_lineCmdParam;
  int print_time;

} lineCmdParamExecTemplate;



extern int API_execTemplate( vt_image *image,
                             vt_image *imres,
                             char *param_str_1,
                             char *param_str_2 );



extern char *API_Help_execTemplate( int h );

extern void API_ErrorParse_execTemplate( char *program, char *str, int flag );

extern void API_InitParam_execTemplate( lineCmdParamExecTemplate *par );

extern void API_PrintParam_execTemplate( FILE *theFile, char *program,
                                         lineCmdParamExecTemplate *par,
                                         char *str );

extern void API_ParseParam_execTemplate( int firstargc, int argc, char *argv[],
                                 lineCmdParamExecTemplate *p );



#ifdef __cplusplus
}
#endif

#endif
