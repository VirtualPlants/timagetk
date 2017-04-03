/*************************************************************************
 * api-connexe.h -
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


#ifndef _api_connexe_h_
#define _api_connexe_h_


#ifdef __cplusplus
extern "C" {
#endif



#include <vt_typedefs.h>
#include <vt_image.h>
#include <vt_connexe.h>


typedef enum enumConnexeTypeComputation {
    _VT_CONNECTED,
    _VT_HYSTERESIS,
    _VT_SEEDPT,
    _VT_SEEDSIM
} enumConnexeTypeComputation;



typedef struct lineCmdParamConnexe {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];
  int input_inv;
  int input_swap;
  ImageType output_type;



  /* specific arguments
   */
  enumConnexeTypeComputation typeComputation;
  char input_seeds_name[STRINGLENGTH];
  vt_ipt seed;

  float low_threshold;
  float high_threshold;

  vt_connexe cpar;

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamConnexe;



/* compute connected components from 'image'
 * - if 'imseeds' is given (!= NULL), compute seeded connected
 *   components (ie connected components that contain foreground
 *   of 'imseeds')
 * - if an image of labels is desired as output, it is safer to use
 *   unsigned short int as the type of the result image 'imres'
 *
 */

extern int API_connexe( vt_image *image,
                        vt_image *imseeds,
                        vt_image *imres,
                        char *param_str_1, char *param_str_2 );



extern char *API_Help_connexe( int h );

extern void API_ErrorParse_connexe( char *program, char *str, int flag );

extern void API_InitParam_connexe( lineCmdParamConnexe *par );

extern void API_PrintParam_connexe( FILE *theFile, char *program,
                                         lineCmdParamConnexe *par,
                                         char *str );

extern void API_ParseParam_connexe( int firstargc, int argc, char *argv[],
                                 lineCmdParamConnexe *p );



#ifdef __cplusplus
}
#endif

#endif
