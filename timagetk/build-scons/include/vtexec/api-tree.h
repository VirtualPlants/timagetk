/*************************************************************************
 * api-tree.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2016, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mar 22 mar 2016 12:04:49 CET
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _api_tree_h_
#define _api_tree_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <topological-tree.h>

#include <vt_typedefs.h>
#include <vt_image.h>


typedef enum enumTreeImageType {
  _NEIGHBORS_COUNT_,
  _CONNECTED_COMPONENTS_
} enumTreeImageType;


typedef struct lineCmdParamTree {

  /* image names and output type
   */
  char input_name[STRINGLENGTH];
  char output_name[STRINGLENGTH];

  char vtktree_name[STRINGLENGTH];

  enumTreeImageType typeOutput;

  int input_inv;
  int input_swap;
  ImageType output_type;

  /* specific arguments
   */

  /* ... */

  /* general parameters
   */
  int print_lineCmdParam;
  int print_time;

} lineCmdParamTree;



extern int API_tree( vt_image *image,
                     vt_image *imres,
                     char *param_str_1,
                     char *param_str_2 );

extern int API_symbolicTree( vt_image *image,
                             typeTree *tree,
                             char *param_str_1,
                             char *param_str_2 );


extern char *API_Help_tree( int h );

extern void API_ErrorParse_tree( char *program, char *str, int flag );

extern void API_InitParam_tree( lineCmdParamTree *par );

extern void API_PrintParam_tree( FILE *theFile, char *program,
                                         lineCmdParamTree *par,
                                         char *str );

extern void API_ParseParam_tree( int firstargc, int argc, char *argv[],
                                 lineCmdParamTree *p );



#ifdef __cplusplus
}
#endif

#endif
