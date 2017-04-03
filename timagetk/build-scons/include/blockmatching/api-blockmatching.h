/*************************************************************************
 * api-blockmatching.h -
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

#ifndef _api_blockmatching_h_
#define _api_blockmatching_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <typedefs.h>

#include <bal-blockmatching-param.h>
#include <bal-image.h>
#include <bal-transformation.h>

typedef struct lineCmdParamBlockmatching {

  /* - file names
   * - transformation names
   */

    char floating_image[STRINGLENGTH];
    char reference_image[STRINGLENGTH];
    char result_image[STRINGLENGTH];

    char left_real_transformation[STRINGLENGTH];
    char left_voxel_transformation[STRINGLENGTH];

    char initial_result_real_transformation[STRINGLENGTH];
    char initial_result_voxel_transformation[STRINGLENGTH];

    char result_real_transformation[STRINGLENGTH];
    char result_voxel_transformation[STRINGLENGTH];

    /* ImageType output_type;
     */

    /* pre-processing before matching
     */
    int normalisation;

    /* post-processing after matching
     */
    int compositionWithLeftTransformation;

    /* parameters for hierarchical block matching
     */
    bal_blockmatching_pyramidal_param param;

    /* writing stuff
     */
    int use_default_filename;
    char command_line_file[STRINGLENGTH];
    char log_file[STRINGLENGTH];

    /* general parameters
     */
    int print_lineCmdParam;
    int print_time;

} lineCmdParamBlockmatching;


/*****************************************************************
 * API designed for STracking and PIB.
 *
 * It is kept for historical reasons and backward compatibility
 * but should not be used in newer applications/developments.
 * Use API_blockmatching() instead.
 *
 * It is very similar to API_INTERMEDIARY_blockmatching() except
 * that parameters are explicitly given.
 * It will call the internal procedure _API_INTERMEDIARY_blockmatching().
 *
 *****************************************************************/

extern int blockmatching( char *floating_image,
                          char *reference_image,
                          char *result_image,
                          char *left_real_transformation,
                          char *left_voxel_transformation,
                          char *initial_result_real_transformation,
                          char *initial_result_voxel_transformation,
                          char *result_real_transformation,
                          char *result_voxel_transformation,
                          int normalisation,
                          bal_blockmatching_pyramidal_param param,
                          int use_default_filename,
                          char *command_line_file,
                          char *log_file,
                          int print_time,
                          int print_parameters,
                          int isDebug );



/*****************************************************************
 * API designed as an intermediary step between the command line
 * interface (ie the inline command blockmatching) and
 * an other internal interface API _API_INTERMEDIARY_blockmatching().
 *
 * It is very similar to blockmatching() except that parameters
 * are given in strings mimicking the command line options.
 * It will call the internal procedure _API_INTERMEDIARY_blockmatching().
 *
 * It exists for development reasons, but should not be used.
 *****************************************************************/

extern int API_INTERMEDIARY_blockmatching( char *floating_image,
                                           char *reference_image,
                                           char *result_image,
                                           char *left_real_transformation,
                                           char *left_voxel_transformation,
                                           char *initial_result_real_transformation,
                                           char *initial_result_voxel_transformation,
                                           char *result_real_transformation,
                                           char *result_voxel_transformation,
                                           char *param_str_1, char *param_str_2 );

/*****************************************************************
 * entry point for TissueLab
 * it will just call _API_blockmatching()
 *****************************************************************/

/* register (I_flo o T_left o T_init) onto I_ref
 * and return T_res such that (I_flo o T_left o T_res)
 * is comparable to I_ref
 *
 * Initial values:
 * T_left (*leftTransformation) can be NULL
 * T_init (*initResultTransformation) can be NULL
 * If both are NULL, the initial transformation will be the translation
 * that superimposes the FOV centers of the two images
 *
 * the returned transformation pointer can be *initResultTransformation.
 * Then, if not NULL, initResultTransformation value can have changed
 * (this has to be kept in mind in case this transformation is further used).
 *
 * The returned transformation as well as its container have to be desallocated
 * afterwards *if and only if* the pointer *initResultTransformation was NULL
 * (else *initResultTransformation is the returned value).
 * Thus the call should be something like
 *
 * Tres = API_blockmatching( Iflo, Iref, Tleft, Tinit, str1, str2 );
 * if ( Tres == (bal_transformation*)NULL ) {
 *   # some error occurs
 *   # do something
 * }
 * ...
 * # desallocation of Tres
 * if ( Tres != Tinit ) {
 *   BAL_FreeTransformation( Tres );
 *   free( Tres );
 * }
 * ...
 *
 */

extern bal_transformation *API_blockmatching( bal_image *floatingImage,
                                              bal_image *referenceImage,
                                              bal_image *resultImage,
                                              bal_transformation *leftTransformation,
                                              bal_transformation *initResultTransformation,
                                              char *param_str_1, char *param_str_2 );



extern char *API_Help_blockmatching( int h );

extern void API_ErrorParse_blockmatching( char *program, char *str, int flag );

extern void API_InitParam_blockmatching( lineCmdParamBlockmatching *par );

extern void API_PrintParam_blockmatching( FILE *theFile, char *program,
                                         lineCmdParamBlockmatching *par,
                                         char *str );

extern void API_ParseParam_blockmatching( int firstargc, int argc, char *argv[],
                                 lineCmdParamBlockmatching *p );



#ifdef __cplusplus
}
#endif

#endif
