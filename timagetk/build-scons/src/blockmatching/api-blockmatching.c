/*************************************************************************
 * api-blockmatching.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer 24 jui 2015 17:33:43 CEST
 *
 * ADDITIONS, CHANGES
 *
 * to generate files:
 * sed -e "s/blockmatching/execuTable/g" \
 *     -e "s/Blockmatching/ExecuTable/g" \
 *     -e "s/blockmatching/executable/g" \
 *     [api-]blockmatching.[c,h] > [api-]execTable.[c,h]
 *
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <chunks.h>
#include <linearFiltering.h>
#include <reech4x4.h>
#include <reech-def.h>

#include <bal-behavior.h>
#include <bal-block-tools.h>
#include <bal-blockmatching-param-tools.h>
#include <bal-blockmatching.h>
#include <bal-field-tools.h>
#include <bal-image-tools.h>
#include <bal-lineartrsf.h>
#include <bal-pyramid.h>
#include <bal-transformation-tools.h>
#include <bal-vectorfield.h>

#include <api-blockmatching.h>






static int _verbose_ = 1;
static int _debug_ = 0;
static char *defaultprefix = "blockmatching";


static int _ReadTransformation( bal_image *floatingImage,
                                bal_image *referenceImage,
                                bal_transformation *readTransformation,
                                char *real_transformation,
                                char *voxel_transformation );

static int _WriteResultTransformation( bal_image *floatingImage,
                                       bal_image *referenceImage,
                                       bal_transformation *resTransformation,
                                       char *result_real_transformation,
                                       char *result_voxel_transformation,
                                       int use_default_filename );

static void _API_ParseParam_blockmatching( char *str, lineCmdParamBlockmatching *p );





/************************************************************
 *
 * static APIs
 * because of the weird definition of blockmatching(), we have
 * to do some unelegant stuff :(
 *
 ************************************************************/

static bal_transformation *_API_blockmatching( bal_image *floatingImage,
                                        bal_image *referenceImage,
                                        bal_transformation *leftTransformation,
                                        bal_transformation *initResultTransformation,
                                        lineCmdParamBlockmatching *thePar,
                                        char *param_str_1, char *param_str_2 )
{
  char *proc = "_API_blockmatching";
  lineCmdParamBlockmatching readPar, *par;

  bal_transformation *resultTransformation = (bal_transformation*)NULL;



  /* read parameters from string
   * if they have not been read before
   */
  if ( thePar == (lineCmdParamBlockmatching*)NULL ) {
      /* parameter initialization
       */
      API_InitParam_blockmatching( &readPar );

      /* parameter parsing
       */
      if ( param_str_1 != (char*)NULL )
          _API_ParseParam_blockmatching( param_str_1, &readPar );
      if ( param_str_2 != (char*)NULL )
          _API_ParseParam_blockmatching( param_str_2, &readPar );

      par = &readPar;
  }
  else {
      par = thePar;
  }

  if ( par->print_lineCmdParam )
      API_PrintParam_blockmatching( stderr, proc, par, (char*)NULL );





  /***************************************************
   *
   * matching
   *
   ***************************************************/

  if ( _debug_ ) {
    fprintf( stderr, "========== before matching ===========\n" );
    BAL_PrintTransformation( stderr,  leftTransformation, "left transformation" );
    BAL_PrintTransformation( stderr,  initResultTransformation, "initial result transformation" );
    fprintf( stderr, "=====================================\n" );
  }

  resultTransformation = BAL_PyramidalBlockMatching( referenceImage, floatingImage,
                                                     leftTransformation,
                                                     initResultTransformation,
                                                     &(par->param) );
  if ( resultTransformation == (bal_transformation*)NULL ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to register the images \n", proc );
      return( (bal_transformation*)NULL );
  }

  if ( _debug_ ) {
    fprintf( stderr, "========== after matching ===========\n" );
    BAL_PrintTransformation( stderr,  resultTransformation, "computed transformation" );
    fprintf( stderr, "=====================================\n" );
  }

  return( resultTransformation );
}





/*****************************************************************
 *
 * internal API where reading and writing occurs.
 *
 *****************************************************************/

static int _API_INTERMEDIARY_blockmatching( char *floating_image,
                                            char *reference_image,
                                            char *result_image,
                                            char *left_real_transformation,
                                            char *left_voxel_transformation,
                                            char *initial_result_real_transformation,
                                            char *initial_result_voxel_transformation,
                                            char *result_real_transformation,
                                            char *result_voxel_transformation,
                                            lineCmdParamBlockmatching *thePar,
                                            char *param_str_1, char *param_str_2 )
{
    char *proc = "_API_INTERMEDIARY_blockmatching";
    lineCmdParamBlockmatching readPar, *par;

    bal_image theReferenceImage;
    bal_image theFloatingImage;
    bal_image theResultImage;

    bal_transformation theReadResultTransformation;
    bal_transformation *readResultTransformation = (bal_transformation *)NULL;
    bal_transformation theReadLeftTransformation;
    bal_transformation *readLeftTransformation = (bal_transformation *)NULL;

    bal_transformation *theResultTransformation = (bal_transformation *)NULL;

    bal_transformation theComposedTransformation;
    bal_transformation *composedTransformation = (bal_transformation *)NULL;
    bal_transformation *resamplingTransformation = (bal_transformation *)NULL;
    bal_transformation *tobewrittenTransformation = (bal_transformation *)NULL;

    time_t t = time(NULL);
    char auxstr[STRINGLENGTH];
    char *auxptr;



    if ( thePar == (lineCmdParamBlockmatching*)NULL ) {
        /* parameter initialization
         */
        API_InitParam_blockmatching( &readPar );

        /* parameter parsing
         */
        if ( param_str_1 != (char*)NULL )
            _API_ParseParam_blockmatching( param_str_1, &readPar );
        if ( param_str_2 != (char*)NULL )
            _API_ParseParam_blockmatching( param_str_2, &readPar );

        par = &readPar;
    }
    else {
        par = thePar;
    }

    /************************************************************
     *
     *  here is the stuff
     *
     ************************************************************/

    /* setting additional I/O stuff
     * - write command line
     * - open log file
     */

    if ( par->param.verbose > 0 ) {
        if ( par->log_file[0] != '\0' ) {
          if ( strlen( par->log_file ) == 4 && strcmp( par->log_file, "NULL" ) == 0 )
            par->param.verbosef = NULL;
          else if ( strlen( par->log_file ) == 6 && strcmp( par->log_file, "stderr" ) == 0 )
            par->param.verbosef = stderr;
          else if ( strlen( par->log_file ) == 6 && strcmp( par->log_file, "stdout" ) == 0 )
            par->param.verbosef = stdout;
          else {
            par->param.verbosef = fopen( par->log_file, "w" );
            if ( par->param.verbosef == NULL ) {
                if ( _verbose_ ) {
                    fprintf( stderr, "%s: unable to open '%s' for writing, switch to 'stderr'\n",
                             proc, par->log_file );
                }
                par->param.verbosef = stderr;
            }
          }
        }
        else if ( par->use_default_filename == 1 ) {
          sprintf( auxstr, "%s-%d-trace.txt", defaultprefix, getpid() );
          par->param.verbosef = fopen( auxstr, "w" );
          if ( par->param.verbosef == NULL ) {
              if ( _verbose_ ) {
                  fprintf( stderr, "%s: unable to open '%s' for writing, switch to 'stderr'\n",
                           proc, auxstr );
              }
              par->param.verbosef = stderr;
          }
        }

        BAL_SetVerboseFileInBalFieldTools( par->param.verbosef );
    }

    /* writing some stuff
     */
    if (par->param.verbosef != NULL) {
        fprintf( par->param.verbosef, "%% %s\n", ctime( &t ) );
        BAL_PrintDefines( par->param.verbosef );
        if ( par->print_lineCmdParam )
            API_PrintParam_blockmatching( par->param.verbosef, proc, par, (char*)NULL );
    }



    /************************************************************
     * images
     ************************************************************/

    /* reading reference image
     */
    if ( par->param.verbosef != NULL ) {
      fprintf( par->param.verbosef, "\tReading reference image '%s'\n", reference_image );
    }
    if ( BAL_ReadImage( &theReferenceImage, reference_image, par->normalisation ) != 1 ) {
      if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
        fclose( par->param.verbosef );
      if ( _verbose_ )
          fprintf( stderr, "%s: can not read '%s'\n", proc, reference_image );
      return( -1 );
    }

    /* reading floating image
     */
    if ( par->param.verbosef != NULL ) {
      fprintf( par->param.verbosef, "\tReading floating image '%s'\n", floating_image );
    }
    if ( BAL_ReadImage( &theFloatingImage, floating_image, par->normalisation ) != 1 ) {
      BAL_FreeImage( &theReferenceImage );
      if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
        fclose( par->param.verbosef );
      if ( _verbose_ )
          fprintf( stderr, "%s: can not read '%s'\n", proc, floating_image );
      return( -1 );
    }



    /************************************************************
     * transformations
     ************************************************************/


    /* initial result transformation *readResultTransformation
     *
     * read from file names (if given) else set to NULL
     * Previously, it was set to identity (to be done further)
     * when no file names were given.
     *
     */

    BAL_InitTransformation( &theReadResultTransformation );
    readResultTransformation = (bal_transformation*)NULL;

    if ( ( initial_result_real_transformation != NULL
           && initial_result_real_transformation[0] != '\0')
         || ( initial_result_voxel_transformation != NULL
              && initial_result_voxel_transformation[0] != '\0' ) ) {

      if ( _ReadTransformation( &theFloatingImage,
                                &theReferenceImage,
                                &theReadResultTransformation,
                                initial_result_real_transformation,
                                initial_result_voxel_transformation ) != 1 ) {
        BAL_FreeImage( &theFloatingImage );
        BAL_FreeImage( &theReferenceImage );
        if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
          fclose( par->param.verbosef );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read initial result transformation\n", proc );
        return( -1 );
      }

      readResultTransformation = &theReadResultTransformation;

    }



    /* read left transformation
     * it only serves for a first transformation of the floating
     * image and will not be updated
     *
     */

    BAL_InitTransformation( &theReadLeftTransformation );
    readLeftTransformation = (bal_transformation*)NULL;

    if ( ( left_real_transformation != NULL
           && left_real_transformation[0] != '\0' )
         || ( left_voxel_transformation != NULL
              && left_real_transformation[0] != '\0' ) ) {

      if ( _ReadTransformation( &theFloatingImage,
                                &theReferenceImage,
                                &theReadLeftTransformation,
                                left_real_transformation,
                                left_voxel_transformation ) != 1 ) {
        if ( readResultTransformation != (bal_transformation*)NULL )
          BAL_FreeTransformation( &theReadResultTransformation );
        BAL_FreeImage( &theFloatingImage );
        BAL_FreeImage( &theReferenceImage );
        if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
          fclose( par->param.verbosef );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read initial result transformation\n", proc );
        return( -1 );
      }

      readLeftTransformation = &theReadLeftTransformation;

    }

    /***************************************************
     *
     * matching
     * - readLeftTransformation and readResultTransformation may be NULL
     * - readResultTransformation type may be different from
     *   par->param.transformation_type
     *
     * returns a pointer to a bal_transformation that may be readResultTransformation
     *
     ***************************************************/

    theResultTransformation = _API_blockmatching( &theFloatingImage,
                                                  &theReferenceImage,
                                                  readLeftTransformation,
                                                  readResultTransformation,
                                                  par, param_str_1, param_str_2 );
    if ( theResultTransformation == (bal_transformation*)NULL  ) {
        if ( readLeftTransformation != (bal_transformation*)NULL )
            BAL_FreeTransformation( &theReadLeftTransformation );
        if ( readResultTransformation != (bal_transformation*)NULL )
          BAL_FreeTransformation( &theReadResultTransformation );
        BAL_FreeImage( &theFloatingImage );
        BAL_FreeImage( &theReferenceImage );
        if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
          fclose( par->param.verbosef );
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to register images\n", proc );
        return( -1 );
    }



    /***************************************************
     *
     * writing output transformation
     *
     ***************************************************/

    /* a composition transformation is required
     * if readLeftTransformation is not NULL
     *    and something has to be written
     *    either the composed transformation
     *    or the result image
     *
     * if there exist an initial transformation,
     * the resampling transformation is the composed one
     * else it is the result one (from the matching)
     *
     * the transformation to be written if the composed if
     * par->compositionWithLeftTransformation is true else it is the
     * result one (from the matching)
     */

    BAL_InitTransformation( &theComposedTransformation );
    composedTransformation = (bal_transformation*)NULL;
    resamplingTransformation = (bal_transformation*)NULL;
    tobewrittenTransformation = theResultTransformation;

    if ( readLeftTransformation != (bal_transformation*)NULL
         && ( (par->use_default_filename == 1)
              || ( par->compositionWithLeftTransformation == 1
                   && ((result_real_transformation != NULL
                       && result_real_transformation[0] != '\0')
                   || (result_voxel_transformation != NULL
                       && result_voxel_transformation[0] != '\0')) )
              || (result_image != NULL
                  && result_image[0] != '\0') ) ) {

      if ( BAL_AllocTransformationComposition( &theComposedTransformation,
                                               readLeftTransformation,
                                               theResultTransformation,
                                               &theReferenceImage ) != 1 ) {
        if ( readLeftTransformation != (bal_transformation*)NULL )
            BAL_FreeTransformation( &theReadLeftTransformation );
        if ( readResultTransformation != (bal_transformation*)NULL )
          BAL_FreeTransformation( &theReadResultTransformation );
        if ( theResultTransformation != readResultTransformation ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        BAL_FreeImage( &theFloatingImage );
        BAL_FreeImage( &theReferenceImage );
        if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
          fclose( par->param.verbosef );
        if ( _verbose_ )
            fprintf( stderr, "%s: can not allocate composition transformation\n", proc );
        return( -1 );
      }

      if ( BAL_TransformationComposition( &theComposedTransformation,
                                          readLeftTransformation,
                                          theResultTransformation ) != 1 ) {
        if ( readLeftTransformation != (bal_transformation*)NULL )
            BAL_FreeTransformation( &theReadLeftTransformation );
        if ( readResultTransformation != (bal_transformation*)NULL )
          BAL_FreeTransformation( &theReadResultTransformation );
        if ( theResultTransformation != readResultTransformation ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        BAL_FreeImage( &theFloatingImage );
        BAL_FreeImage( &theReferenceImage );
        if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
          fclose( par->param.verbosef );
        if ( _verbose_ )
            fprintf( stderr, "%s: can not compute composition transformation\n", proc );
        return( -1 );
      }

      composedTransformation = &theComposedTransformation;
      resamplingTransformation = &theComposedTransformation;

      if ( par->compositionWithLeftTransformation == 1 )
        tobewrittenTransformation = &theComposedTransformation;
    }



    /* resampling transformation in case
     * readLeftTransformation == (bal_transformation*)NULL
     */

    if ( readLeftTransformation == (bal_transformation*)NULL
         && ( (par->use_default_filename == 1)
              || (result_image != NULL
                  && result_image[0] != '\0') ) ) {
      resamplingTransformation = theResultTransformation;
    }



    /* init transformation can be freed
     */
    if ( readLeftTransformation != (bal_transformation*)NULL ) {
        BAL_FreeTransformation( &theReadLeftTransformation );
        readLeftTransformation = (bal_transformation*)NULL;
    }



    /* writing output transformation
     */

    if ( _WriteResultTransformation( &theFloatingImage, &theReferenceImage, tobewrittenTransformation,
                                     result_real_transformation, result_voxel_transformation,
                                     par->use_default_filename ) != 1 ) {
      if ( composedTransformation != (bal_transformation*)NULL )
          BAL_FreeTransformation( &theComposedTransformation );
      if ( readResultTransformation != (bal_transformation*)NULL )
        BAL_FreeTransformation( &theReadResultTransformation );
      if ( theResultTransformation != readResultTransformation ) {
        BAL_FreeTransformation( theResultTransformation );
        free( theResultTransformation );
      }
      BAL_FreeImage( &theFloatingImage );
      BAL_FreeImage( &theReferenceImage );
      if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
        fclose( par->param.verbosef );
      if ( _verbose_ )
          fprintf( stderr, "%s: can not write result transformation\n", proc );
      return( -1 );
    }



    /***************************************************
     *
     * writing output image
     *
     ***************************************************/

    if ( (par->use_default_filename == 1)
         || (result_image != NULL
             && result_image[0] != '\0') ) {

      /* re-read floating image if required
       */
      if ( par->normalisation ) {
        BAL_FreeImage( &theFloatingImage );
        if ( BAL_ReadImage( &theFloatingImage, floating_image, 0 ) != 1 ) {
          if ( composedTransformation != (bal_transformation*)NULL )
              BAL_FreeTransformation( &theComposedTransformation );
          if ( readResultTransformation != (bal_transformation*)NULL )
            BAL_FreeTransformation( &theReadResultTransformation );
          if ( theResultTransformation != readResultTransformation ) {
            BAL_FreeTransformation( theResultTransformation );
            free( theResultTransformation );
          }
          BAL_FreeImage( &theReferenceImage );
          if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
            fclose( par->param.verbosef );
          if ( _verbose_ )
              fprintf( stderr, "%s: can not read '%s' (resampling step)\n",
                       proc, floating_image );
          return( -1 );
        }
      }

      /* allocate a result image with the same type than the floating one
         and with the geometry of the reference one
      */
      if ( BAL_InitAllocImageFromImage( &theResultImage, (char*)NULL,
                                        &theReferenceImage, theFloatingImage.type ) != 1 ) {
        if ( composedTransformation != (bal_transformation*)NULL )
            BAL_FreeTransformation( &theComposedTransformation );
        if ( readResultTransformation != (bal_transformation*)NULL )
          BAL_FreeTransformation( &theReadResultTransformation );
        if ( theResultTransformation != readResultTransformation ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        BAL_FreeImage( &theFloatingImage );
        BAL_FreeImage( &theReferenceImage );
        if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
          fclose( par->param.verbosef );
        if ( _verbose_ )
            fprintf( stderr, "%s: can not allocate result image (resampling step)\n", proc );
        return( -1 );
      }
      theResultImage.vx = theReferenceImage.vx;
      theResultImage.vy = theReferenceImage.vy;
      theResultImage.vz = theReferenceImage.vz;

      /* resample floating image
       */
      if ( BAL_ResampleImage( &theFloatingImage, &theResultImage,
                              resamplingTransformation, LINEAR ) != 1 ) {
        BAL_FreeImage( &theResultImage );
        if ( composedTransformation != (bal_transformation*)NULL )
            BAL_FreeTransformation( &theComposedTransformation );
        if ( readResultTransformation != (bal_transformation*)NULL )
          BAL_FreeTransformation( &theReadResultTransformation );
        if ( theResultTransformation != readResultTransformation ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        BAL_FreeImage( &theFloatingImage );
        BAL_FreeImage( &theReferenceImage );
        if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
          fclose( par->param.verbosef );
        if ( _verbose_ )
            fprintf( stderr, "%s: can not resample floating image (resampling step)\n", proc );
        return( -1 );
      }

      /* writing result image
       */
      auxptr = (char*)NULL;
      if ( result_image != NULL && result_image[0] != '\0' ) {
        auxptr = result_image;
      }
      else {
        if ( par->use_default_filename == 1 ) {
          sprintf( auxstr, "%s-%d-result-image.hdr", defaultprefix, getpid() );
          auxptr = auxstr;
        }
      }
      if ( auxptr != (char*)NULL ) {
          if ( BAL_WriteImage( &theResultImage, auxptr ) != 1 ) {
            BAL_FreeImage( &theResultImage );
            if ( composedTransformation != (bal_transformation*)NULL )
                BAL_FreeTransformation( &theComposedTransformation );
            if ( readResultTransformation != (bal_transformation*)NULL )
              BAL_FreeTransformation( &theReadResultTransformation );
            if ( theResultTransformation != readResultTransformation ) {
              BAL_FreeTransformation( theResultTransformation );
              free( theResultTransformation );
            }
            BAL_FreeImage( &theFloatingImage );
            BAL_FreeImage( &theReferenceImage );
              if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
                fclose( par->param.verbosef );
              if ( _verbose_ )
                  fprintf( stderr, "%s: unable to write result image '%s' (resampling step)\n", proc, auxptr );
            return( -1 );
          }
      }
      else {
        if ( _verbose_ )
            fprintf( stderr, "%s: weird, this point should not be reached (resampling step)\n", proc );
      }

      /* freeing result image
       */
      BAL_FreeImage( &theResultImage );

    }



    /***************************************************
     *
     * exiting
     *
     ***************************************************/

    if ( composedTransformation != (bal_transformation*)NULL )
        BAL_FreeTransformation( &theComposedTransformation );
    if ( readResultTransformation != (bal_transformation*)NULL )
      BAL_FreeTransformation( &theReadResultTransformation );
    if ( theResultTransformation != readResultTransformation ) {
      BAL_FreeTransformation( theResultTransformation );
      free( theResultTransformation );
    }
    BAL_FreeImage( &theFloatingImage );
    BAL_FreeImage( &theReferenceImage );

    /* close log file
     */

    if ( par->param.verbosef != NULL && par->param.verbosef != stderr && par->param.verbosef != stdout )
      fclose( par->param.verbosef );

    return( 1 );
}










/************************************************************
 *
 * main API
 *
 ************************************************************/

/* this one is kept for historical reasons,
 * ie Stracking compilation but should disappear
 */
int blockmatching( char *floating_image,
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
                   int isDebug )
{
    char *proc = "blockmatching";
    lineCmdParamBlockmatching par;

    if ( _verbose_ )
        fprintf( stderr, "Warning, '%s' is obsolete\n", proc );

    _debug_ = isDebug;
    BAL_SetDebugInBalFieldTools( _debug_ );
    BAL_SetDebugInBalBlockMatching( _debug_ );
    setDebugInChunks( _debug_ );

    API_InitParam_blockmatching( &par );

    par.normalisation = normalisation;
    par.param = param;
    par.use_default_filename = use_default_filename;
    if ( command_line_file != NULL && command_line_file[0] != '\0' )
        (void)strcpy( par.command_line_file, command_line_file );
    if ( log_file != NULL && log_file[0] != '\0' )
        (void)strcpy( par.log_file, log_file );

    if ( print_time )
        par.print_time = 1;
    if ( print_parameters )
        par.print_lineCmdParam = 1;

    if ( _API_INTERMEDIARY_blockmatching( floating_image, reference_image, result_image,
                                          left_real_transformation,
                                          left_voxel_transformation,
                                          initial_result_real_transformation,
                                          initial_result_voxel_transformation,
                                          result_real_transformation,
                                          result_voxel_transformation,
                                          &par, (char*)NULL, (char*)NULL ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: some error occurs\n", proc );
        return( -1 );
    }

    return( 0 );
}





int API_INTERMEDIARY_blockmatching( char *floating_image,
                                    char *reference_image,
                                    char *result_image,
                                    char *left_real_transformation,
                                    char *left_voxel_transformation,
                                    char *initial_result_real_transformation,
                                    char *initial_result_voxel_transformation,
                                    char *result_real_transformation,
                                    char *result_voxel_transformation,
                                    char *param_str_1, char *param_str_2 )
{
    char *proc = "API_INTERMEDIARY_blockmatching";

    if ( _API_INTERMEDIARY_blockmatching( floating_image, reference_image, result_image,
                                          left_real_transformation,
                                          left_voxel_transformation,
                                          initial_result_real_transformation,
                                          initial_result_voxel_transformation,
                                          result_real_transformation,
                                          result_voxel_transformation,
                                          (lineCmdParamBlockmatching*)NULL,
                                          param_str_1, param_str_2 ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: some error occurs\n", proc );
        return( -1 );
    }

    return( 1 );
}





/************************************************************
 *
 * this is the entry point for TissueLab
 * - if existing, *initResultTransformation serves as 
 *   initialization for the returned transformation *resultTransformation
 *   it may be the returned transformation
 * - if existing, *initTransformation has been composed with
 *   *resultTransformation, so to resample the floating image
 *   we have to compute 
 *   auxTrsf = initTransformation o resultTransformation
 * 
 * The calling function/procedure has to desallocate the returning
 * transformation and the container if different from
 * ...
 * bal_transformation *theTrsf
 * ...
 * theTrsf = API_blockmatching( ... );
 * if ( theTrsf == (bal_transformation *)NULL ) {
 *    some error occurs
 * }
 * ...
 * if some other error oocurs
 *      if ( theTrsf != initResultTransformation ) {
 *        BAL_FreeTransformation( theTrsf );
 *        free( theTrsf );
 *      }
 * ...
 ************************************************************/


bal_transformation *API_blockmatching( bal_image *floatingImage,
                                       bal_image *referenceImage,
                                       bal_image *resultImage,
                                       bal_transformation *leftTransformation,
                                       bal_transformation *initResultTransformation,
                                       char *param_str_1, char *param_str_2 )
{
  char *proc = "API_blockmatching";
  bal_transformation *resultTransformation = (bal_transformation*)NULL;
  bal_transformation auxTransformation;
  bal_transformation *resamplingTransformation = (bal_transformation*)NULL;
  
  resultTransformation = _API_blockmatching( floatingImage,
                                             referenceImage,
                                             leftTransformation,
                                             initResultTransformation,
                                             (lineCmdParamBlockmatching*)NULL,
                                             param_str_1, param_str_2 );
  if ( resultTransformation == (bal_transformation*)NULL ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: some error occurs\n", proc );
      return( (bal_transformation*)NULL );
  }

  /* computation of the resampled floating image
   * should we make some additional test on this image?
   * - dimensions w.r.t. referenceImage?
   * - type w.r.t. floatingImage
   */

  if ( resultImage != (bal_image*)NULL ) {

    /* resampling transformation
     */
    if ( leftTransformation == (bal_transformation*)NULL ) {
      resamplingTransformation = resultTransformation;
    }
    else {
      BAL_InitTransformation( &auxTransformation );
      if ( BAL_AllocTransformationComposition( &auxTransformation,
                                               leftTransformation,
                                               resultTransformation,
                                               referenceImage ) != 1 ) {
        if ( resultTransformation != initResultTransformation ) {
          BAL_FreeTransformation( resultTransformation );
          free( resultTransformation );
        }
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to allocate resampling transformation\n", proc );
        return( (bal_transformation*)NULL );
      }
      if ( BAL_TransformationComposition( &auxTransformation,
                                          leftTransformation,
                                          resultTransformation ) != 1 ) {
        BAL_FreeTransformation( &auxTransformation );
        if ( resultTransformation != initResultTransformation ) {
          BAL_FreeTransformation( resultTransformation );
          free( resultTransformation );
        }
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to compute resampling transformation\n", proc );
        return( (bal_transformation*)NULL );
      }
      resamplingTransformation = &auxTransformation;
    }

    /* resampling
     */
    if ( BAL_ResampleImage( floatingImage, resultImage,
                            resamplingTransformation, LINEAR ) != 1 ) {
      if ( leftTransformation != (bal_transformation*)NULL )
        BAL_FreeTransformation( &auxTransformation );
      if ( resultTransformation != initResultTransformation ) {
        BAL_FreeTransformation( resultTransformation );
        free( resultTransformation );
      }
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute resampled image\n", proc );
      return( (bal_transformation*)NULL );
    }

    /* freeing auxiliary transformation if required
     */
    if ( leftTransformation != (bal_transformation*)NULL )
      BAL_FreeTransformation( &auxTransformation );
  }

  return( resultTransformation );
}





/************************************************************
 *
 * dedicated functions
 *
 ************************************************************/


static int _ReadTransformation( bal_image *floatingImage,
                                bal_image *referenceImage,
                                bal_transformation *readTransformation,
                                char *real_transformation,
                                char *voxel_transformation )
{
  char *proc = "_ReadTransformation";

  if ( real_transformation != NULL && real_transformation[0] != '\0' ) {

    if ( BAL_ReadTransformation( readTransformation, real_transformation ) != 1 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to read 'real' transformation '%s'\n",
                   proc, real_transformation );
      return( -1 );
    }

    readTransformation->transformation_unit = REAL_UNIT;

  }

  else if ( voxel_transformation != NULL && voxel_transformation[0] != '\0' ) {

    if ( BAL_ReadTransformation( readTransformation, voxel_transformation ) != 1 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to read 'voxel' transformation '%s'\n",
                   proc, voxel_transformation );
      return( -1 );
    }

    readTransformation->transformation_unit = VOXEL_UNIT;

    if ( BAL_ChangeTransformationToRealUnit( referenceImage,
                                             floatingImage,
                                             readTransformation,
                                             readTransformation ) != 1 ) {
      BAL_FreeTransformation( readTransformation );
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to convert 'voxel' transformation '%s' into the 'real' world\n",
                   proc, voxel_transformation );
      return( -1 );
    }

  }

  else {
    if ( _verbose_ )
        fprintf( stderr, "%s: weird, this should not be reached\n",
                 proc );
    return( -1 );
  }

  return( 1 );
}





static int _WriteResultTransformation( bal_image *floatingImage,
                                       bal_image *referenceImage,
                                       bal_transformation *resTransformation,
                                       char *result_real_transformation,
                                       char *result_voxel_transformation,
                                       int use_default_filename )
{
    char *proc = "_WriteResultTransformation";
    char *auxptr = (char*)NULL;
    char auxstr[STRINGLENGTH];


    /* writing the "real" transformation
     */

    if ( result_real_transformation != NULL && result_real_transformation[0] != '\0' ) {
      auxptr = result_real_transformation;
    }
    else if ( use_default_filename == 1 ) {
      sprintf( auxstr, "%s-%d-result-real-transformation.trsf", defaultprefix, getpid() );
      auxptr = auxstr;
    }

    if ( auxptr != (char*)NULL ) {
      if ( BAL_WriteTransformation( resTransformation, auxptr ) != 1 ) {
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to write real result transformation '%s'\n", proc, auxptr );
          return( -1 );
      }
    }

    /* writing the "voxel" transformation
     */

    auxptr = (char*)NULL;

    if ( result_voxel_transformation != NULL && result_voxel_transformation[0] != '\0' ) {
        auxptr = result_voxel_transformation;
    }
    else if ( use_default_filename == 1 ) {
      sprintf( auxstr, "%s-%d-result-voxel-transformation.trsf", defaultprefix, getpid() );
      auxptr = auxstr;
    }

    if ( auxptr != (char*)NULL ) {
      if ( BAL_ChangeTransformationToVoxelUnit( floatingImage, referenceImage,
                                                resTransformation, resTransformation ) != 1 ) {
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to convert 'real' transformation into the 'voxel' world\n",
                       proc );
          return( -1 );
      }

      if ( BAL_WriteTransformation( resTransformation, auxptr ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to write voxel result transformation '%s'\n", proc, auxptr );
        return( -1 );
      }
    }

    return( 1 );
}







/************************************************************
 *
 * static functions
 *
 ************************************************************/



static char **_Str2Array( int *argc, char *str )
{
  char *proc = "_Str2Array";
  int n = 0;
  char *s = str;
  char **array, **a;

  if ( s == (char*)NULL || strlen( s ) == 0 ) {
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: empty input string\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  /* go to the first valid character
   */
  while ( *s == ' ' || *s == '\n' || *s == '\t' )
    s++;

  if ( *s == '\0' ) {
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: weird, input string contains only separation characters\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  /* count the number of strings
   */
  for ( n = 0; *s != '\0'; ) {
    n ++;
    while ( *s != ' ' && *s != '\n' && *s != '\t' && *s != '\0' )
      s ++;
    while ( *s == ' ' || *s == '\n' || *s == '\t' )
      s ++;
  }

  if ( _verbose_ >= 5 )
    fprintf( stderr, "%s: found %d strings\n", proc, n );

  /* the value of the strings will be duplicated
   * so that the input string can be freed
   */
  array = (char**)malloc( n * sizeof(char*) + (strlen(str)+1) * sizeof(char) );
  if ( array == (char**)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation failed\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  a = array;
  a += n;
  s = (char*)a;
  (void)strncpy( s, str, strlen( str ) );
  s[ strlen( str ) ] = '\0';

  while ( *s == ' ' || *s == '\n' || *s == '\t' ) {
    *s = '\0';
    s++;
  }

  for ( n = 0; *s != '\0'; ) {
    array[n] = s;
    n ++;
    while ( *s != ' ' && *s != '\n' && *s != '\t' && *s != '\0' )
      s ++;
    while ( *s == ' ' || *s == '\n' || *s == '\t' ) {
      *s = '\0';
      s ++;
    }
  }

  *argc = n;
  return( array );
}





/************************************************************
 *
 * help / documentation
 *
 ************************************************************/



static char *usage = "-reference|-ref %s -floating|-flo %s -result|-res %s\n\
 [-initial-transformation|-init-trsf|-left-transformation|-left-trsf %s]\n\
 [-initial-voxel-transformation|-init-voxel-trsf|-left-voxel-transformation|-left-voxel-trsf %s]\n\
 [-initial-result-transformation|-init-res-trsf %s]\n\
 [-initial-result-voxel-transformation|-init-res-voxel-trsf %s]\n\
 [-result-transformation|-res-trsf %s]\n\
 [-result-voxel-transformation|-res-voxel-trsf %s]\n\
 [-normalisation|-norma|-rescale|-no-normalisation|-no-norma|-no-rescale]\n\
 [-no-composition-with-initial] [-composition-with-initial] \n\
 [-pyramid-lowest-level | -py-ll %d] [-pyramid-highest-level | -py-hl %d]\n\
 [-pyramid-gaussian-filtering | -py-gf]\n\
 [-block-size|-bl-size %d %d %d] [-block-spacing|-bl-space %d %d %d]\n\
 [-block-border|-bl-border %d %d %d]\n\
 [-floating-low-threshold | -flo-lt %d]\n\
 [-floating-high-threshold | -flo-ht %d]\n\
 [-floating-removed-fraction | -flo-rf %f]\n\
 [-reference-low-threshold | -ref-lt %d]\n\
 [-reference-high-threshold | -ref-ht %d]\n\
 [-reference-removed-fraction | -ref-rf %f]\n\
 [-floating-selection-fraction[-ll|-hl] | -flo-frac[-ll|-hl] %lf]\n\
 [-search-neighborhood-half-size | -se-hsize %d %d %d]\n\
 [-search-neighborhood-step | -se-step %d %d %d]\n\
 [-similarity-measure | -similarity | -si [cc]]\n\
 [-similarity-measure-threshold | -si-th %lf]\n\
 [-transformation-type|-transformation|-trsf-type %s]\n\
 [-elastic-regularization-sigma[-ll|-hl] | -elastic-sigma[-ll|-hl]  %lf %lf %lf]\n\
 [-estimator-type|-estimator|-es-type wlts|lts|wls|ls]\n\
 [-lts-cut|-lts-fraction %lf] [-lts-deviation %f] [-lts-iterations %d]\n\
 [-fluid-sigma|-lts-sigma[-ll|-hl] %lf %lf %lf]\n\
 [-vector-propagation-distance|-propagation-distance|-pdistance %f]\n\
 [-vector-fading-distance|-fading-distance|-fdistance %f]\n\
 [-max-iteration[-ll|-hl]|-max-iter[-ll|-hl] %d] [-corner-ending-condition|-rms]\n\
 [-gaussian-filter-type|-filter-type deriche|fidrich|young-1995|young-2002|...\n\
 ...|gabor-young-2002|convolution]\n\
 [-default-filenames|-df] [-no-default-filenames|-ndf]\n\
 [-command-line %s] [-logfile %s]\n\
 [-vischeck] [-write_def]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



 static char *detail = "\
 ### File names ###\n\
 -reference|-ref %s  # name of the reference image (still image)\n\
 -floating|-flo %s   # name of the image to be registered (floating image)\n\
   blocks are defined on this image\n\
 -result|res %s      # name of the result image (default is 'res.inr.gz')\n\
   this is the floating image resampled with the output transformation\n\
   in the same geometry than the reference image\n\
 [-initial-transformation|-init-trsf|-left-transformation|-left-trsf %s]\n\
                     # name of the left/initial transformation\n\
   in 'real' coordinates. Goes from 'reference' to 'floating', ie allows to \n\
   resample 'floating' in the geometry of 'reference' before registration.\n\
   Comes then to register 'floating o init' with 'reference'.\n\
   If indicated, '-initial-voxel-transformation' is ignored.\n\
 [-initial-voxel-transformation|-init-voxel-trsf|-left-voxel-transformation|...\n\
  ...|-left-voxel-trsf %s]      # name of the left/initial\n\
   transformation in 'voxel' coordinates.\n\
 [-initial-result-transformation|-init-res-trsf %s] # name of the\n\
   initialization of the result transformation in 'real' coordinates.\n\
   May be used when the registration has only be conducted at high scales\n\
   and one want to continue it at lower scales.\n\
   Comes then to register 'floating o init' with 'reference'.\n\
   If indicated, '-initial-result-voxel-transformation' is ignored.\n\
 [-initial-result-voxel-transformation|-init-res-voxel-trsf %s] # name of the\n\
   initialization of the result transformation in 'voxel' coordinates.\n\
 [-result-transformation|-res-trsf %s] # name of the result transformation\n\
   in 'real' coordinates. Goes from 'reference' to 'floating', ie allows to \n\
   resample 'floating' in the geometry of 'reference'.\n\
   If an initial transformation is given (with either '-initial-transformation' or\n\
   '-initial-voxel-transformation'), this is the composition of the initial\n\
   transformation and the computed one, unless '-no-composition-with-initial' is\n\
   specified.\n\
 [-result-voxel-transformation|-res-voxel-trsf %s] # name of the result\n\
   transformation in 'voxel' coordinates.\n\
   Fragile with used in conjonction with '-no-composition-with-initial'\n\
 ### pre-processing ###\n\
 [-normalisation|-norma|-rescale] # input images are normalized on one byte\n\
   before matching (this may be the default behavior)\n\
 [-no-normalisation|-no-norma|-no-rescale] # input images are not normalized on\n\
   one byte before matching\n\
 ### post-processing ###\n\
 [-no-composition-with-initial] # the written result transformation is only the\n\
   computed one, ie it is not composed with the initial one (thus does not allow\n\
   to resample the floating image if an initial transformation is given)\n\
 [-composition-with-initial] # the written result transformation is the\n\
   computed one composed with the initial one (thus allows to resample the\n\
   floating image if an initial transformation is given) [default]\n\
 ### pyramid building ###\n\
 [-pyramid-lowest-level | -py-ll %d]    # pyramid lowest level\n\
   (0 = original dimension)\n\
 [-pyramid-highest-level | -py-hl %d]   # pyramid highest level\n\
   default is 3: it corresponds to 32x32x32 for an original 256x256x256 image\n\
 [-pyramid-gaussian-filtering | -py-gf] # before subsampling, the images \n\
   are filtered (ie smoothed) by a gaussian kernel.\n\
 ### block geometry (floating image) ###\n\
 -block-size|-bl-size %d %d %d       # size of the block along X, Y, Z\n\
 -block-spacing|-bl-space %d %d %d   # block spacing in the floating image\n\
 -block-border|-bl-border %d %d %d   # block borders: to be added twice at\n\
   each dimension for statistics computation\n\
 ### block selection ###\n\
 [-floating-low-threshold | -flo-lt %d]     # values <= low threshold are not\n\
   considered\n\
 [-floating-high-threshold | -flo-ht %d]    # values >= high threshold are not\n\
   considered\n\
 [-floating-removed-fraction | -flo-rf %f]  # maximal fraction of removed points\n\
   because of the threshold. If too many points are removed, the block is\n\
   discarded\n\
 [-reference-low-threshold | -ref-lt %d]    # values <= low threshold are not\n\
   considered\n\
 [-reference-high-threshold | -ref-ht %d]   # values >= high threshold are not\n\
   considered\n\
 [-reference-removed-fraction | -ref-rf %f] # maximal fraction of removed points\n\
   because of the threshold. If too many points are removed, the block is\n\
   discarded\n\
 [-floating-selection-fraction[-ll|-hl] | -flo-frac[-ll|-hl] %lf] # fraction of\n\
   blocks from the floating image kept at a pyramid level, the blocks being\n\
   sorted w.r.t their variance (see note (1) for [-ll|-hl])\n\
 ### pairing ###\n\
 [-search-neighborhood-half-size | -se-hsize %d %d %d] # half size of the search\n\
   neighborhood in the reference when looking for similar blocks\n\
 [-search-neighborhood-step | -se-step %d %d %d] # step between blocks to be\n\
   tested in the search neighborhood\n\
 [-similarity-measure | -similarity | -si [cc|ecc|ssd|sad]]  # similarity measure\n\
   cc: correlation coefficient\n\
   ecc: extended correlation coefficient\n\
 [-similarity-measure-threshold | -si-th %lf]    # threshold on the similarity\n\
   measure: pairings below that threshold are discarded\n\
 ### transformation type ###\n\
 [-transformation-type|-transformation|-trsf-type %s] # transformation type\n\
   translation2D, translation3D, translation-scaling2D, translation-scaling3D,\n\
   rigid2D, rigid3D, rigid, similitude2D, similitude3D, similitude,\n\
   affine2D, affine3D, affine, vectorfield2D, vectorfield3D, vectorfield, vector\n\
 ### transformation regularization ###\n\
 [-elastic-regularization-sigma[-ll|-hl] | -elastic-sigma[-ll|-hl]  %lf %lf %lf]\n\
   # sigma for elastic regularization (only for vector field) (see note (1) for\n\
   [-ll|-hl])\n\
 ### transformation estimation ###\n\
 [-estimator-type|-estimator|-es-type %s] # transformation estimator\n\
   wlts: weighted least trimmed squares\n\
   lts: least trimmed squares\n\
   wls: weighted least squares\n\
   ls: least squares\n\
 [-lts-cut|-lts-fraction %lf] # for trimmed estimations, fraction of pairs that are kept\n\
 [-lts-deviation %lf] # for trimmed estimations, defines the threshold to discard\n\
   pairings, ie 'average + this_value * standard_deviation'\n\
 [-lts-iterations %d] # for trimmed estimations, the maximal number of iterations\n\
 [-fluid-sigma|-lts-sigma[-ll|-hl] %lf %lf %lf] # sigma for fluid regularization,\n\
   ie field interpolation and regularization for pairings (only for vector field)\n\
   (see note (1) for [-ll|-hl])\n\
 [-vector-propagation-distance|-propagation-distance|-pdistance %f] # \n\
   distance propagation of initial pairings (ie displacements)\n\
   this implies the same displacement for the spanned sphere\n\
   (only for vectorfield)\n\
 [-vector-fading-distance|-fading-distance|-fdistance %f] # \n\
   area of fading for initial pairings (ie displacements)\n\
   this allows progressive transition towards null displacements\n\
   and thus avoid discontinuites\n\
 ### end conditions for matching loop ###\n\
 [-max-iteration[-ll|-hl]|-max-iter[-ll|-hl] %d]   # maximal number of iteration\n\
   (see note (1) for [-ll|-hl])\n\
 [-corner-ending-condition|-rms] # evolution of image corners\n\
 ### filter type ###\n\
 [-gaussian-filter-type|-filter-type deriche|fidrich|young-1995|young-2002|...\n\
   ...|gabor-young-2002|convolution] # type of filter for image/vector field\n\
   smoothing\n\
  ### misc writing stuff ###\n\
  [-default-filenames|-df]     # use default filename names\n\
  [-no-default-filenames|-ndf] # do not use default filename names\n\
  [-command-line %s]           # write the command line\n\
  [-logfile %s]                # write some output in this logfile\n\
  [-vischeck]  # write an image with 'active' blocks\n\
  [-write_def] # id. \n\
 ### parallelism ###\n\
 [-parallel|-no-parallel] # use parallelism (or not)\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-max-chunks %d] # maximal number of chunks\n\
 [-parallel-scheduling|-ps default|static|dynamic-one|dynamic|guided] # type\n\
   of scheduling for open mp\n\
 ### general parameters ###\n\
 -verbose|-v: increase verboseness\n\
    parameters being read several time, use '-nv -v -v ...'\n\
    to set the verboseness level\n\
 -noverbose|-nv: no verboseness at all\n\
 -debug|-D: increase debug level\n\
 -nodebug: no debug indication\n\
 -print-parameters|-param:\n\
 -print-time|-time:\n\
 -no-time|-notime:\n\
 -h: print option list\n\
 -help: print option list + details\n\
 \n\
 Notes\n\
 (1) If -ll or -hl are respectively added to the option, this specifies only the\n\
   value for respectively the lowest or the highest level of the pyramid (recall\n\
   that the most lowest level, ie #0, refers to the original image). For\n\
   intermediary levels, values are linearly interpolated.\n\
 ";





char *API_Help_blockmatching( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_blockmatching( char *program, char *str, int flag )
{
    if ( flag >= 0 ) {
        if ( program != (char*)NULL )
           (void)fprintf(stderr,"Usage: %s %s\n", program, usage);
        else
            (void)fprintf(stderr,"Command line options: %s\n", usage);
    }
    if ( flag == 1 ) {
      (void)fprintf( stderr, "--------------------------------------------------\n" );
      (void)fprintf(stderr,"%s",detail);
      (void)fprintf( stderr, "--------------------------------------------------\n" );
    }
    if ( str != (char*)NULL )
      (void)fprintf(stderr,"Error: %s\n",str);
    exit(0);
}





/************************************************************
 *
 * parameters management
 *
 ************************************************************/



void API_InitParam_blockmatching( lineCmdParamBlockmatching *p )
{
    (void)strncpy( p->floating_image, "\0", 1 );
    (void)strncpy( p->reference_image, "\0", 1 );
    (void)strncpy( p->result_image, "\0", 1 );

    (void)strncpy( p->left_real_transformation, "\0", 1 );
    (void)strncpy( p->left_voxel_transformation, "\0", 1 );

    (void)strncpy( p->initial_result_real_transformation, "\0", 1 );
    (void)strncpy( p->initial_result_voxel_transformation, "\0", 1 );

    (void)strncpy( p->result_real_transformation, "\0", 1 );
    (void)strncpy( p->result_voxel_transformation, "\0", 1 );

    /* p->output_type = TYPE_UNKNOWN;
     */

    /* pre-processing before matching
     */
#ifdef _ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_
    p->normalisation = 1;
#else
    p->normalisation = 0;
#endif

    /* post-processing after matching
     */
    p->compositionWithLeftTransformation = 0;


    /* parameters for hierarchical block matching
     */
    BAL_InitBlockMatchingPyramidalParameters( &(p->param) );

    /* writing stuff
     */
    p->use_default_filename = 0;
    (void)strncpy( p->command_line_file, "\0", 1 );
    (void)strncpy( p->log_file, "\0", 1 );

    /* general parameters
     */
    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_blockmatching( FILE *theFile, char *program,
                                  lineCmdParamBlockmatching *p, char *str )
{
  FILE *f = theFile;
  if ( theFile == (FILE*)NULL ) f = stderr;

  fprintf( f, "==================================================\n" );
  fprintf( f, "= #defines\n" );
  BAL_PrintDefines( f );
  fprintf( f, "==================================================\n" );
  fprintf( f, "= in line command parameters" );
  if ( program != (char*)NULL )
    fprintf( f, " for '%s'", program );
  if ( str != (char*)NULL )
    fprintf( f, "= %s\n", str );
  fprintf( f, "\n"  );
  fprintf( f, "==================================================\n" );

  fprintf( f, "# file names\n" );

  fprintf( f, "- p->floating_image = " );
  if ( p->floating_image[0] != '\0' )
      fprintf( f, "'%s'\n", p->floating_image );
  else
      fprintf( f, "NULL\n" );
  fprintf( f, "- p->reference_image = " );
  if ( p->reference_image[0] != '\0' )
      fprintf( f, "'%s'\n", p->reference_image );
  else
      fprintf( f, "NULL\n" );
  fprintf( f, "- p->result_image = " );
  if ( p->result_image[0] != '\0' )
      fprintf( f, "'%s'\n", p->result_image );
  else
      fprintf( f, "NULL\n" );

  fprintf( f, "- p->left_real_transformation = " );
  if ( p->left_real_transformation[0] != '\0' )
      fprintf( f, "'%s'\n", p->left_real_transformation );
  else
      fprintf( f, "NULL\n" );
  fprintf( f, "- p->left_voxel_transformation = " );
  if ( p->left_real_transformation[0] != '\0' )
      fprintf( f, "'%s'\n", p->left_voxel_transformation );
  else
      fprintf( f, "NULL\n" );
  fprintf( f, "- p->initial_result_real_transformation = " );
  if ( p->initial_result_real_transformation[0] != '\0' )
      fprintf( f, "'%s'\n", p->initial_result_real_transformation );
  else
      fprintf( f, "NULL\n" );
  fprintf( f, "- p->initial_result_voxel_transformation = " );
  if ( p->initial_result_voxel_transformation[0] != '\0' )
      fprintf( f, "'%s'\n", p->initial_result_voxel_transformation );
  else
      fprintf( f, "NULL\n" );

  fprintf( f, "- p->result_real_transformation = " );
  if ( p->result_voxel_transformation[0] != '\0' )
      fprintf( f, "'%s'\n", p->result_voxel_transformation );
  else
      fprintf( f, "NULL\n" );
  fprintf( f, "- p->result_voxel_transformation = " );
  if ( p->result_voxel_transformation[0] != '\0' )
      fprintf( f, "'%s'\n", p->result_voxel_transformation );
  else
      fprintf( f, "NULL\n" );

  fprintf( f, "# pre-processing\n" );
  fprintf( f, "- p->normalisation = %d\n", p->normalisation );

  fprintf( f, "# post-processing\n" );
  fprintf( f, "- p->compositionWithLeftTransformation = %d\n", p->compositionWithLeftTransformation );

  fprintf( f, "# writing stuff\n" );
  fprintf( f, "- p->use_default_filename = %d\n", p->use_default_filename );
  fprintf( f, "- p->command_line_file = " );
  if ( p->command_line_file[0] != '\0' )
      fprintf( f, "'%s'\n", p->command_line_file );
  else
      fprintf( f, "NULL\n" );
  fprintf( f, "- p->log_file = " );
  if ( p->log_file[0] != '\0' )
      fprintf( f, "'%s'\n", p->log_file );
  else
      fprintf( f, "NULL\n" );

  fprintf( f, "# misc\n" );
  fprintf( f, "- p->print_lineCmdParam =  %d\n", p->print_lineCmdParam );
#ifndef WIN32
  fprintf( f, "- p->print_time =  %d\n", p->print_time );
#endif
  fprintf( f, "\n" );
  fprintf( f, "# parameters for hierarchical block matching\n" );
  BAL_PrintBlockMatchingPyramidalParameters( f, &(p->param) );

  fprintf( f, "==================================================\n" );
}





/************************************************************
 *
 * parameters parsing
 *
 ************************************************************/



static void _API_ParseParam_blockmatching( char *str, lineCmdParamBlockmatching *p )
{
  char *proc = "_API_ParseParam_blockmatching";
  char **argv;
  int i, argc;

  if ( str == (char*)NULL || strlen(str) == 0 )
      return;

  argv = _Str2Array( &argc, str );
  if ( argv == (char**)NULL || argc == 0 ) {
      if ( _debug_ ) {
          fprintf( stderr, "%s: weird, no arguments were found\n", proc );
      }
      return;
  }

  if ( _debug_ > 4 ) {
      fprintf( stderr, "%s: translation from\n", proc );
      fprintf( stderr, "   '%s'\n", str );
      fprintf( stderr, "into\n" );
      for ( i=0; i<argc; i++ )
          fprintf( stderr, "   argv[%2d] = '%s'\n", i, argv[i] );
  }

  API_ParseParam_blockmatching( 0, argc, argv, p );

  free( argv );
}





/************************************************************
 *
 * reading parameters is done in two steps
 * 1. one looks for the transformation type
 *    -> this implies dedicated initialization
 * 2. the other parameters are read
 *
 ************************************************************/

void API_ParseParam_blockmatching( int firstargc, int argc, char *argv[],
                                  lineCmdParamBlockmatching *p )
{
  int i;
  int status;
  int maxchunks;



  /* reading the transformation type
   */

  for ( i=firstargc; i<argc; i++ ) {
    /* transformation type
     */
    if ( strcmp ( argv[i], "-transformation-type" ) == 0
         || strcmp ( argv[i], "-transformation" ) == 0
         || strcmp ( argv[i], "-trsf-type" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-transformation-type", 0 );
      if ( strcmp ( argv[i], "translation2D" ) == 0 ) {
        p->param.transformation_type = TRANSLATION_2D;
      }
      else if ( strcmp ( argv[i], "translation3D" ) == 0 ) {
        p->param.transformation_type = TRANSLATION_3D;
      }
      else if ( strcmp ( argv[i], "translation" ) == 0 && argv[i][11] == '\0') {
        p->param.transformation_type = TRANSLATION_3D;
      }
      else if ( strcmp ( argv[i], "translation-scaling2D" ) == 0 ) {
        p->param.transformation_type = TRANSLATION_SCALING_2D;
      }
      else if ( strcmp ( argv[i], "translation-scaling3D" ) == 0 ) {
        p->param.transformation_type = TRANSLATION_SCALING_3D;
      }
      else if ( strcmp ( argv[i], "rigid2D" ) == 0 ) {
        p->param.transformation_type = RIGID_2D;
      }
      else if ( strcmp ( argv[i], "rigid3D" ) == 0 ) {
        p->param.transformation_type = RIGID_3D;
      }
      else if ( (strcmp ( argv[i], "rigid" ) == 0 && argv[i][5] == '\0') ) {
        p->param.transformation_type = RIGID_3D;
      }
      else if ( strcmp ( argv[i], "similitude2D" ) == 0 ) {
        p->param.transformation_type = SIMILITUDE_2D;
      }
      else if ( strcmp ( argv[i], "similitude3D" ) == 0 ) {
        p->param.transformation_type = SIMILITUDE_3D;
      }
      else if ( strcmp ( argv[i], "similitude" ) == 0 ) {
        p->param.transformation_type = SIMILITUDE_3D;
      }
      else if ( strcmp ( argv[i], "affine2D" ) == 0 ) {
        p->param.transformation_type = AFFINE_2D;
      }
      else if ( strcmp ( argv[i], "affine3D" ) == 0 ) {
        p->param.transformation_type = AFFINE_3D;
      }
      else if ( strcmp ( argv[i], "affine" ) == 0 ) {
        p->param.transformation_type = AFFINE_3D;
      }
      /*
        else if ( strcmp ( argv[i], "spline" ) == 0 ) {
        p->param.transformation_type = SPLINE;
        }
      */
      else if ( strcmp ( argv[i], "vectorfield" ) == 0
                || strcmp ( argv[i], "vector" ) == 0 ) {
        p->param.transformation_type = VECTORFIELD_3D;
      }
      else if ( strcmp ( argv[i], "vectorfield3D" ) == 0
                || strcmp ( argv[i], "vector3D" ) == 0 ) {
        p->param.transformation_type = VECTORFIELD_3D;
      }
      else if ( strcmp ( argv[i], "vectorfield2D" ) == 0
                || strcmp ( argv[i], "vector2D" ) == 0 ) {
        p->param.transformation_type = VECTORFIELD_2D;
      }
      else {
        fprintf( stderr, "unknown transformation type: '%s'\n", argv[i] );
        API_ErrorParse_blockmatching( (char*)NULL, "-transformation-type", 0 );
      }
    }
  }



  /* initialisation of parameters dedicated to either
     linear transformation or vector field
  */

  switch ( p->param.transformation_type ) {
  default :
    API_ErrorParse_blockmatching( (char*)NULL, "unknown transformation type", 0 );
    break;
  case TRANSLATION_2D :
  case TRANSLATION_3D :
  case TRANSLATION_SCALING_2D :
  case TRANSLATION_SCALING_3D :
  case RIGID_2D :
  case RIGID_3D :
  case SIMILITUDE_2D :
  case SIMILITUDE_3D :
  case AFFINE_2D :
  case AFFINE_3D :
    BAL_InitBlockMatchingPyramidalParametersForLinearTransformation( &(p->param) );
    break;
  case VECTORFIELD_3D :
  case VECTORFIELD_2D :
    BAL_InitBlockMatchingPyramidalParametersForVectorfieldTransformation( &(p->param) );
    break;
  }



  /* reading the other parameters
   */

  for ( i=firstargc; i<argc; i++ ) {

    /* transformation type
       already read
    */
    if ( strcmp ( argv[i], "-transformation-type" ) == 0
         || strcmp ( argv[i], "-transformation" ) == 0
         || strcmp ( argv[i], "-trsf-type" ) == 0 ) {
      i ++;
    }

    /* image file names
     */
    else if ( strcmp ( argv[i], "-reference") == 0
              || (strcmp ( argv[i], "-ref") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -reference", 0 );
      (void)strcpy( p->reference_image, argv[i] );
    }
    else if ( strcmp ( argv[i], "-floating") == 0
              || (strcmp ( argv[i], "-flo") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -floating", 0 );
      (void)strcpy( p->floating_image, argv[i] );
    }
    else if ( strcmp ( argv[i], "-result") == 0
              || (strcmp ( argv[i], "-res") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -result", 0 );
      (void)strcpy( p->result_image, argv[i] );
    }

    /* transformation file names
     */
    else if ( strcmp ( argv[i], "-initial-transformation" ) == 0
              || strcmp ( argv[i], "-init-trsf" ) == 0
              || strcmp ( argv[i], "-left-transformation" ) == 0
              || strcmp ( argv[i], "-left-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -left-transformation", 0 );
      (void)strcpy( p->left_real_transformation, argv[i] );
    }
    else if ( strcmp ( argv[i], "-initial-voxel-transformation" ) == 0
              || strcmp ( argv[i], "-init-voxel-trsf" ) == 0
              || strcmp ( argv[i], "-left-voxel-transformation" ) == 0
              || strcmp ( argv[i], "-left-voxel-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -left-voxel-transformation", 0 );
      (void)strcpy( p->left_voxel_transformation, argv[i] );
    }

    else if ( strcmp ( argv[i], "-initial-result-transformation" ) == 0
              || strcmp ( argv[i], "-init-res-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -initial-result-transformation", 0 );
      (void)strcpy( p->initial_result_real_transformation, argv[i] );
    }
    else if ( strcmp ( argv[i], "-initial-result-voxel-transformation" ) == 0
              || strcmp ( argv[i], "-init-res-voxel-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -initial-result-voxel-transformation", 0 );
      (void)strcpy( p->initial_result_voxel_transformation, argv[i] );
    }

    else if ( strcmp ( argv[i], "-result-transformation" ) == 0
              || strcmp ( argv[i], "-res-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -result-transformation", 0 );
      (void)strcpy( p->result_real_transformation, argv[i] );
    }
    else if ( strcmp ( argv[i], "-result-voxel-transformation" ) == 0
              || strcmp ( argv[i], "-res-voxel-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -result-voxel-transformation", 0 );
      (void)strcpy( p->result_voxel_transformation, argv[i] );
    }


    /* pre-processing before matching
       input images normalisation
    */
    else if ( strcmp ( argv[i], "-normalisation") == 0
              || (strcmp ( argv[i], "-norma") == 0 && argv[i][6] == '\0')
              || (strcmp ( argv[i], "-rescale") == 0 && argv[i][8] == '\0') ) {
      p->normalisation = 1;
    }
    else if ( strcmp ( argv[i], "-no-normalisation") == 0
              || (strcmp ( argv[i], "-no-norma") == 0 && argv[i][9] == '\0')
              || (strcmp ( argv[i], "-no-rescale") == 0 && argv[i][11] == '\0') ) {
      p->normalisation = 0;
    }

    /* post-processing after matching
       composition with initial transformation
    */
    else if ( strcmp ( argv[i], "-no-composition-with-initial") == 0
              || strcmp ( argv[i], "-no-composition-with-left") == 0 ) {
      p->compositionWithLeftTransformation = 0;
    }
    else if ( strcmp ( argv[i], "-composition-with-initial") == 0
              || strcmp ( argv[i], "-composition-with-left") == 0 ) {
        p->compositionWithLeftTransformation = 1;
    }



    /* pyramid building
     */
    else if ( strcmp ( argv[i], "-pyramid-lowest-level" ) == 0
              || (strcmp( argv[i], "-py-ll") == 0 && argv[i][6] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -pyramid-lowest-level", 0 );
      status = sscanf( argv[i], "%d", &(p->param.pyramid_lowest_level) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -pyramid-lowest-level", 0 );
    }
    else if ( strcmp ( argv[i], "-pyramid-highest-level" ) == 0
              || (strcmp( argv[i], "-py-hl") == 0 && argv[i][6] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -pyramid-highest-level", 0 );
      status = sscanf( argv[i], "%d", &(p->param.pyramid_highest_level) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -pyramid-highest-level", 0 );
    }
    else if ( strcmp ( argv[i], "-pyramid-gaussian-filtering" ) == 0
              || (strcmp( argv[i], "-py-gf") == 0 && argv[i][6] == '\0') ) {
      p->param.pyramid_gaussian_filtering = 1;
    }


    /* block geometry
     */
    else if ( strcmp (argv[i], "-block-size" ) == 0
              || strcmp (argv[i], "-bl-size" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-size %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.block_dim.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-size %d", 0 );
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-size %d %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.block_dim.y) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-sizes %d %d", 0 );
      i ++;
      if ( i >= argc) p->param.block_dim.z = 1;
      else {
        status = sscanf( argv[i], "%d", &(p->param.block_dim.z) );
        if ( status <= 0 ) {
          i--;
          p->param.block_dim.z = 1;
        }
      }
    }
    else if ( strcmp (argv[i], "-block-spacing" ) == 0
              || strcmp (argv[i], "-bl-space") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-spacing %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.block_spacing.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-spacing %d", 0 );
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-spacing %d %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.block_spacing.y) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-spacing %d %d", 0 );
      i ++;
      if ( i >= argc) p->param.block_spacing.z = 0;
      else {
        status = sscanf( argv[i], "%d", &(p->param.block_spacing.z) );
        if ( status <= 0 ) {
          i--;
          p->param.block_spacing.z = 0;
        }
      }
    }
    else if ( strcmp (argv[i], "-block-border" ) == 0
              || strcmp (argv[i], "-bl-border") == 0  ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-border %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.block_border.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-borders %d", 0 );
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-borders %d %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.block_border.y) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -block-borders %d %d", 0 );
      i ++;
      if ( i >= argc) p->param.block_border.z = 0;
      else {
        status = sscanf( argv[i], "%d", &(p->param.block_border.z) );
        if ( status <= 0 ) {
          i--;
          p->param.block_border.z = 0;
        }
      }
    }


    /* block selection
     */
    else if ( strcmp ( argv[i], "-floating-low-threshold") == 0
              || strcmp ( argv[i], "-flo-lt") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -floating-low-threshold", 0 );
      status = sscanf( argv[i], "%d", &(p->param.floating_selection.low_threshold) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -floating-low-threshold", 0 );
    }
    else if ( strcmp ( argv[i], "-floating-high-threshold") == 0
              || strcmp ( argv[i], "-flo-ht") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -floating-high-threshold", 0 );
      status = sscanf( argv[i], "%d", &(p->param.floating_selection.high_threshold) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -floating-high-threshold", 0 );
    }
    else if ( strcmp ( argv[i], "-floating-removed-fraction") == 0
              || strcmp ( argv[i], "-flo-rf") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -floating-removed-fraction", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.floating_selection.max_removed_fraction) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -floating-removed-fraction", 0 );
    }

    else if ( strcmp ( argv[i], "-reference-low-threshold") == 0
              || strcmp ( argv[i], "-ref-lt") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -reference-low-threshold", 0 );
      status = sscanf( argv[i], "%d", &(p->param.reference_selection.low_threshold) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -reference-low-threshold", 0 );
    }
    else if ( strcmp ( argv[i], "-reference-high-threshold") == 0
              || strcmp ( argv[i], "-ref-ht") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -reference-high-threshold", 0 );
      status = sscanf( argv[i], "%d", &(p->param.reference_selection.high_threshold) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -reference-high-threshold", 0 );
    }
    else if ( strcmp ( argv[i], "-reference-removed-fraction") == 0
              || strcmp ( argv[i], "-ref-rf") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -reference-removed-fraction", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.reference_selection.max_removed_fraction) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -reference-removed-fraction", 0 );
    }

    else if ( ( strcmp ( argv[i], "-floating-selection-fraction") == 0 && argv[i][28] == '\0' )
              || ( strcmp ( argv[i], "-flo-frac") == 0 && argv[i][9] == '\0' ) ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-floating-selection-fraction", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.blocks_fraction.lowest) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-floating-selection-fraction", 0 );
      p->param.blocks_fraction.highest = p->param.blocks_fraction.lowest;
    }
    else if ( strcmp ( argv[i], "-floating-selection-fraction-lowest-level") == 0
              || strcmp ( argv[i], "-floating-selection-fraction-ll") == 0
              || strcmp ( argv[i], "-flo-frac-ll") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-floating-selection-fraction-lowest-level", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.blocks_fraction.lowest) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-floating-selection-fraction-lowest-level", 0 );
    }
    else if ( strcmp ( argv[i], "-floating-selection-fraction-highest-level") == 0
              || strcmp ( argv[i], "-floating-selection-fraction-hl") == 0
              || strcmp ( argv[i], "-flo-frac-hl") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-floating-selection-fraction-highest-level", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.blocks_fraction.highest) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-floating-selection-fraction-highest-level", 0 );
    }


    /* pairing
     */
    else if ( strcmp (argv[i], "-search-neighborhood-half-size" ) == 0
              || strcmp (argv[i], "-se-hsize" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-half-size %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.half_neighborhood_size.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-half-size %d", 0 );
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-half-size %d %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.half_neighborhood_size.y) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-half-size %d %d", 0 );
      i ++;
      if ( i >= argc) p->param.half_neighborhood_size.z = 0;
      else {
        status = sscanf( argv[i], "%d", &(p->param.half_neighborhood_size.z) );
        if ( status <= 0 ) {
          i--;
          p->param.half_neighborhood_size.z = 0;
        }
      }
    }
    else if ( strcmp (argv[i], "-search-neighborhood-step" ) == 0
              || strcmp (argv[i], "-se-step") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-step %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.step_neighborhood_search.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-step %d", 0 );
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-step %d %d", 0 );
      status = sscanf( argv[i], "%d", &(p->param.step_neighborhood_search.y) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -search-neighborhood-step %d %d", 0 );
      i ++;
      if ( i >= argc) p->param.step_neighborhood_search.z = 0;
      else {
        status = sscanf( argv[i], "%d", &(p->param.step_neighborhood_search.z) );
        if ( status <= 0 ) {
          i--;
          p->param.step_neighborhood_search.z = 0;
        }
      }
    }

    else if ( strcmp ( argv[i], "-similarity-measure" ) == 0
              || strcmp ( argv[i], "-similarity" ) == 0
              || (strcmp ( argv[i], "-si" ) == 0 && argv[i][3] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-similarity-measure", 0 );
      if ( strcmp ( argv[i], "ssd" ) == 0 && argv[i][3] == '\0' ) {
        p->param.similarity_measure = _SSD_;
      }
      else if ( strcmp ( argv[i], "sad" ) == 0 && argv[i][3] == '\0' ) {
        p->param.similarity_measure = _SAD_;
      }
      else if ( strcmp ( argv[i], "cc" ) == 0 && argv[i][2] == '\0' ) {
        p->param.similarity_measure = _SQUARED_CC_;
      }
      else if ( strcmp ( argv[i], "ecc" ) == 0 && argv[i][3] == '\0' ) {
        p->param.similarity_measure = _SQUARED_EXTCC_;
      }
      else {
        fprintf( stderr, "unknown similarity measure: '%s'\n", argv[i] );
        API_ErrorParse_blockmatching( (char*)NULL, "-similarity-measure", 0 );
      }
    }

    else if ( strcmp ( argv[i], "-similarity-measure-threshold" ) == 0
              || strcmp ( argv[i], "-si-th" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-similarity-measure-threshold", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.similarity_measure_threshold) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-similarity-measure-threshold", 0 );
    }



    /* transformation definition and computation
     */
    else if ( ( strcmp ( argv[i], "-elastic-regularization-sigma" ) == 0  && argv[i][29] == '\0' )
              || ( strcmp (argv[i], "-elastic-sigma" ) == 0 && argv[i][14] == '\0' ) ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -elastic-regularization-sigma %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.lowest.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -elastic-regularization-sigma %lf", 0 );
      i ++;
      if ( i >= argc) {
        p->param.elastic_regularization_sigma.lowest.y = p->param.elastic_regularization_sigma.lowest.x;
        p->param.elastic_regularization_sigma.lowest.z = p->param.elastic_regularization_sigma.lowest.x;
      }
      else {
        status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.lowest.y) );
        if ( status <= 0 ) {
          i--;
          p->param.elastic_regularization_sigma.lowest.y = p->param.elastic_regularization_sigma.lowest.x;
          p->param.elastic_regularization_sigma.lowest.z = p->param.elastic_regularization_sigma.lowest.x;
        }
        else {
          i ++;
          if ( i >= argc) p->param.elastic_regularization_sigma.lowest.z = 0;
          else {
            status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.lowest.z) );
            if ( status <= 0 ) {
              i--;
              p->param.elastic_regularization_sigma.lowest.z = 0;
            }
          }
        }
      }
      p->param.elastic_regularization_sigma.highest.x = p->param.elastic_regularization_sigma.lowest.x;
      p->param.elastic_regularization_sigma.highest.y = p->param.elastic_regularization_sigma.lowest.y;
      p->param.elastic_regularization_sigma.highest.z = p->param.elastic_regularization_sigma.lowest.z;
    }
    else if ( strcmp ( argv[i], "-elastic-regularization-sigma-lowest-level" ) == 0
              || strcmp ( argv[i], "-elastic-regularization-sigma-ll" ) == 0
              || strcmp (argv[i], "-elastic-sigma-ll" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -elastic-regularization-sigma-lowest-level %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.lowest.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -elastic-regularization-sigma-lowest-level %lf", 0 );
      i ++;
      if ( i >= argc) {
        p->param.elastic_regularization_sigma.lowest.y = p->param.elastic_regularization_sigma.lowest.x;
        p->param.elastic_regularization_sigma.lowest.z = p->param.elastic_regularization_sigma.lowest.x;
      }
      else {
        status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.lowest.y) );
        if ( status <= 0 ) {
          i--;
          p->param.elastic_regularization_sigma.lowest.y = p->param.elastic_regularization_sigma.lowest.x;
          p->param.elastic_regularization_sigma.lowest.z = p->param.elastic_regularization_sigma.lowest.x;
        }
        else {
          i ++;
          if ( i >= argc) p->param.elastic_regularization_sigma.lowest.z = 0;
          else {
            status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.lowest.z) );
            if ( status <= 0 ) {
              i--;
              p->param.elastic_regularization_sigma.lowest.z = 0;
            }
          }
        }
      }
    }
    else if ( strcmp ( argv[i], "-elastic-regularization-sigma-highest-level" ) == 0
              || strcmp ( argv[i], "-elastic-regularization-sigma-hl" ) == 0
              || strcmp (argv[i], "-elastic-sigma-hl" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -elastic-regularization-sigma-highest-level %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.highest.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -elastic-regularization-sigma-highest-level %lf", 0 );
      i ++;
      if ( i >= argc) {
        p->param.elastic_regularization_sigma.highest.y = p->param.elastic_regularization_sigma.highest.x;
        p->param.elastic_regularization_sigma.highest.z = p->param.elastic_regularization_sigma.highest.x;
      }
      else {
        status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.highest.y) );
        if ( status <= 0 ) {
          i--;
          p->param.elastic_regularization_sigma.highest.y = p->param.elastic_regularization_sigma.highest.x;
          p->param.elastic_regularization_sigma.highest.z = p->param.elastic_regularization_sigma.highest.x;
        }
        else {
          i ++;
          if ( i >= argc) p->param.elastic_regularization_sigma.highest.z = 0;
          else {
            status = sscanf( argv[i], "%lf", &(p->param.elastic_regularization_sigma.highest.z) );
            if ( status <= 0 ) {
              i--;
              p->param.elastic_regularization_sigma.highest.z = 0;
            }
          }
        }
      }
    }



    /* estimator definition and computation
     */
    else if ( strcmp ( argv[i], "-estimator-type") == 0
              || strcmp ( argv[i], "-estimator") == 0
              || strcmp ( argv[i], "-es-type") == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-estimator-type", 0 );
      if ( (strcmp ( argv[i], "ltsw" ) == 0 && argv[i][4] == '\0')
           || (strcmp ( argv[i], "wlts" ) == 0 && argv[i][4] == '\0') ) {
        p->param.estimator.lowest.type = TYPE_WLTS;
      }
      else if ( strcmp ( argv[i], "lts" ) == 0 && argv[i][3] == '\0' ) {
        p->param.estimator.lowest.type = TYPE_LTS;
      }
      else if ( (strcmp ( argv[i], "lsw" ) == 0 && argv[i][3] == '\0')
                || (strcmp ( argv[i], "wls" ) == 0 && argv[i][3] == '\0') ) {
        p->param.estimator.lowest.type = TYPE_WLS;
      }
      else if ( strcmp ( argv[i], "ls" ) == 0 && argv[i][2] == '\0' ) {
        p->param.estimator.lowest.type = TYPE_LS;
      }
      else {
        fprintf( stderr, "unknown estimator type: '%s'\n", argv[i] );
        API_ErrorParse_blockmatching( (char*)NULL, "-estimator-type", 0 );
      }
      p->param.estimator.highest.type = p->param.estimator.lowest.type;
    }

    else if ( strcmp ( argv[i], "-lts-fraction" ) == 0
              || strcmp ( argv[i], "-lts-cut" ) == 0) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-lts-fraction", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.retained_fraction) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-lts-fraction", 0 );
      p->param.estimator.highest.retained_fraction = p->param.estimator.lowest.retained_fraction;
    }

    else if ( strcmp ( argv[i], "-lts-deviation" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-lts-deviation", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.standard_deviation_threshold) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-lts-deviation", 0 );
      p->param.estimator.highest.standard_deviation_threshold = p->param.estimator.lowest.standard_deviation_threshold;
    }

    else if ( strcmp ( argv[i], "-lts-iterations" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-lts-iterations", 0 );
      status = sscanf( argv[i], "%d", &(p->param.estimator.lowest.max_iterations) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-lts-iterations", 0 );
      p->param.estimator.highest.max_iterations = p->param.estimator.lowest.max_iterations;
    }

    else if ( strcmp (argv[i], "-fluid-sigma-lowest-level" ) == 0
              || strcmp (argv[i], "-fluid-sigma-ll" ) == 0
              || strcmp (argv[i], "-lts-sigma-lowest-level" ) == 0
              || strcmp (argv[i], "-lts-sigma-ll" ) == 0  ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -lts-sigma-lowest-level %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.sigma.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -lts-sigma-lowest-level %lf", 0 );
      i ++;
      if ( i >= argc) {
        p->param.estimator.lowest.sigma.y = p->param.estimator.lowest.sigma.x;
        p->param.estimator.lowest.sigma.z = p->param.estimator.lowest.sigma.x;
      }
      else {
        status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.sigma.y) );
        if ( status <= 0 ) {
          i--;
          p->param.estimator.lowest.sigma.y = p->param.estimator.lowest.sigma.x;
          p->param.estimator.lowest.sigma.z = p->param.estimator.lowest.sigma.x;
        }
        else {
          i ++;
          if ( i >= argc) p->param.estimator.lowest.sigma.z = 0;
          else {
            status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.sigma.z) );
            if ( status <= 0 ) {
              i--;
              p->param.estimator.lowest.sigma.z = 0;
            }
          }
        }
      }
    }
    else if ( strcmp (argv[i], "-fluid-sigma-highest-level" ) == 0
              || strcmp (argv[i], "-fluid-sigma-hl" ) == 0
              || strcmp (argv[i], "-lts-sigma-highest-level" ) == 0
              || strcmp (argv[i], "-lts-sigma-hl" ) == 0) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -lts-sigma-highest-level %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.estimator.highest.sigma.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -lts-sigma-highest-level %lf", 0 );
      i ++;
      if ( i >= argc) {
        p->param.estimator.highest.sigma.y = p->param.estimator.highest.sigma.x;
        p->param.estimator.highest.sigma.z = p->param.estimator.highest.sigma.x;
      }
      else {
        status = sscanf( argv[i], "%lf", &(p->param.estimator.highest.sigma.y) );
        if ( status <= 0 ) {
          i--;
          p->param.estimator.highest.sigma.y = p->param.estimator.highest.sigma.x;
          p->param.estimator.highest.sigma.z = p->param.estimator.highest.sigma.x;
        }
        else {
          i ++;
          if ( i >= argc) p->param.estimator.highest.sigma.z = 0;
          else {
            status = sscanf( argv[i], "%lf", &(p->param.estimator.highest.sigma.z) );
            if ( status <= 0 ) {
              i--;
              p->param.estimator.highest.sigma.z = 0;
            }
          }
        }
      }
    }
    else if ( (strcmp (argv[i], "-fluid-sigma" ) == 0 && argv[i][12] == '\0')
              || (strcmp (argv[i], "-lts-sigma" ) == 0 && argv[i][10] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "parsing -lts-sigma %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.sigma.x) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "parsing -lts-sigma %lf", 0 );
      i ++;
      if ( i >= argc) {
        p->param.estimator.lowest.sigma.y = p->param.estimator.lowest.sigma.x;
        p->param.estimator.lowest.sigma.z = p->param.estimator.lowest.sigma.x;
      }
      else {
        status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.sigma.y) );
        if ( status <= 0 ) {
          i--;
          p->param.estimator.lowest.sigma.y = p->param.estimator.lowest.sigma.x;
          p->param.estimator.lowest.sigma.z = p->param.estimator.lowest.sigma.x;
        }
        else {
          i ++;
          if ( i >= argc) p->param.estimator.lowest.sigma.z = 0;
          else {
            status = sscanf( argv[i], "%lf", &(p->param.estimator.lowest.sigma.z) );
            if ( status <= 0 ) {
              i--;
              p->param.estimator.lowest.sigma.z = 0;
            }
          }
        }
      }
      p->param.estimator.highest.sigma = p->param.estimator.lowest.sigma;
    }

    else if ( strcmp (argv[i], "-vector-propagation-distance" ) == 0
               || strcmp (argv[i], "-propagation-distance" ) == 0
               || (strcmp (argv[i], "-pdistance" ) == 0 && argv[i][10] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-vector-propagation-distance", 0 );
      status = sscanf( argv[i], "%f", &(p->param.estimator.lowest.propagation.constant) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-vector-propagation-distance", 0 );
      p->param.estimator.highest.propagation.constant = p->param.estimator.lowest.propagation.constant;
    }
    else if ( strcmp (argv[i], "-vector-fading-distance" ) == 0
               || strcmp (argv[i], "-fading-distance" ) == 0
               || (strcmp (argv[i], "-fdistance" ) == 0 && argv[i][10] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-vector-fading-distance", 0 );
      status = sscanf( argv[i], "%f", &(p->param.estimator.lowest.propagation.fading) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-vector-fading-distance", 0 );
      p->param.estimator.highest.propagation.fading = p->param.estimator.lowest.propagation.fading;
    }



    /* end conditions for matching loop
     */
    else if ( strcmp ( argv[i], "-max-iteration-lowest-level" ) == 0
              || strcmp ( argv[i], "-max-iteration-ll" ) == 0
              || strcmp ( argv[i], "-max-iterations-lowest-level" ) == 0
              || strcmp ( argv[i], "-max-iterations-ll" ) == 0
              || strcmp( argv[i], "-max-iter-lowest-level" ) == 0
              || strcmp( argv[i], "-max-iter-ll" ) == 0  ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-max-iteration-lowest-level", 0 );
      status = sscanf( argv[i], "%d", &(p->param.max_iterations.lowest) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-max-iteration-lowest-level", 0 );
    }
    else if ( strcmp ( argv[i], "-max-iteration-highest-level" ) == 0
              || strcmp ( argv[i], "-max-iteration-hl" ) == 0
              || strcmp ( argv[i], "-max-iterations-highest-level" ) == 0
              || strcmp ( argv[i], "-max-iterations-hl" ) == 0
              || strcmp( argv[i], "-max-iter-highest-level" ) == 0
              || strcmp( argv[i], "-max-iter-hl" ) == 0  ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-max-iteration-highest-level", 0 );
      status = sscanf( argv[i], "%d", &(p->param.max_iterations.highest) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-max-iteration-highest-level", 0 );
    }
    else if ( (strcmp ( argv[i], "-max-iteration" ) == 0 && argv[i][14] == '\0')
              || (strcmp ( argv[i], "-max-iterations" ) == 0 && argv[i][15] == '\0')
              || (strcmp( argv[i], "-max-iter" ) == 0 && argv[i][9] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-max-iteration", 0 );
      status = sscanf( argv[i], "%d", &(p->param.max_iterations.lowest) );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-max-iteration", 0 );
      p->param.max_iterations.highest = p->param.max_iterations.lowest;
    }

    else if ( strcmp ( argv[i], "-corner-ending-condition" ) == 0
              || (strcmp ( argv[i], "-rms") == 0 && argv[i][4] == '\0') ) {
      p->param.rms_ending_condition = 1;
    }


    /* filter type for image smoothing
     */
    else if ( strcmp ( argv[i], "-gaussian-filter-type" ) == 0
              || strcmp ( argv[i], "-filter-type" ) == 0 ) {
      i++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-gaussian-filter-type", 0 );
      if ( strcmp ( argv[i], "deriche" ) == 0 ) {
        BAL_SetFilterType( GAUSSIAN_DERICHE );
      }
      else if ( strcmp ( argv[i], "fidrich" ) == 0 ) {
        BAL_SetFilterType( GAUSSIAN_FIDRICH );
      }
      else if ( strcmp ( argv[i], "young-1995" ) == 0 ) {
        BAL_SetFilterType( GAUSSIAN_YOUNG_1995 );
      }
      else if ( strcmp ( argv[i], "young-2002" ) == 0 ) {
        BAL_SetFilterType( GAUSSIAN_YOUNG_2002 );
      }
      else if ( strcmp ( argv[i], "gabor-young-2002" ) == 0 ) {
        BAL_SetFilterType( GABOR_YOUNG_2002 );
      }
      else if ( strcmp ( argv[i], "convolution" ) == 0  ) {
        BAL_SetFilterType( GAUSSIAN_CONVOLUTION );
      }
      else {
        fprintf( stderr, "unknown filter type: '%s'\n", argv[i] );
        API_ErrorParse_blockmatching( (char*)NULL, "-gaussian-filter-type", 0 );
      }
    }



    /* some general parameters for I/), logs, etc
     */
    else if ( strcmp ( argv[i], "-default-filenames" ) == 0
              || (strcmp ( argv[i], "-df" ) == 0 && argv[i][3] == '\0') ) {
      p->use_default_filename = 1;
    }
    else if ( strcmp ( argv[i], "-no-default-filenames" ) == 0
              || (strcmp ( argv[i], "-ndf" ) == 0 && argv[i][4] == '\0') ) {
      p->use_default_filename = 0;
    }

    else if ( strcmp ( argv[i], "-command-line") == 0 ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -command-line", 0 );
      (void)strcpy( p->command_line_file, argv[i] );
    }
    else if ( strcmp ( argv[i], "-logfile" ) == 0  ) {
      i++;
      if ( i >= argc) API_ErrorParse_blockmatching( (char*)NULL, "parsing -logfile", 0 );
      (void)strcpy( p->log_file, argv[i] );
      if ( p->param.verbose <= 0 ) p->param.verbose = 1;
    }
    else if (strcmp ( argv[i], "-vischeck") == 0){
      p->param.vischeck = 1;
    }
    else if (strcmp ( argv[i], "-write_def") == 0){
      p->param.write_def = 1;
    }

    else if ( strcmp ( argv[i], "-trace") == 0 && argv[i][6] == '\0' ) {
      BAL_IncrementTraceInBalBlockMatching(  );
    }
    else if ( strcmp ( argv[i], "-no-trace") == 0 && argv[i][9] == '\0' ) {
      BAL_DecrementTraceInBalBlockMatching(  );
    }




    /* parallelism
     */
    else if ( strcmp ( argv[i], "-parallel" ) == 0 ) {
      setParallelism( _DEFAULT_PARALLELISM_ );
    }

    else if ( strcmp ( argv[i], "-no-parallel" ) == 0 ) {
       setParallelism( _NO_PARALLELISM_ );
    }

    else if ( strcmp ( argv[i], "-parallelism-type" ) == 0 ||
               strcmp ( argv[i], "-parallel-type" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-parallelism-type", 0 );
      if ( strcmp ( argv[i], "default" ) == 0 ) {
        setParallelism( _DEFAULT_PARALLELISM_ );
      }
      else if ( strcmp ( argv[i], "none" ) == 0 ) {
        setParallelism( _NO_PARALLELISM_ );
      }
      else if ( strcmp ( argv[i], "openmp" ) == 0 || strcmp ( argv[i], "omp" ) == 0 ) {
        setParallelism( _OMP_PARALLELISM_ );
      }
      else if ( strcmp ( argv[i], "pthread" ) == 0 || strcmp ( argv[i], "thread" ) == 0 ) {
        setParallelism( _PTHREAD_PARALLELISM_ );
      }
      else {
        fprintf( stderr, "unknown parallelism type: '%s'\n", argv[i] );
        API_ErrorParse_blockmatching( (char*)NULL, "-parallelism-type", 0 );
      }
    }


    else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-max-chunks", 0 );
      status = sscanf( argv[i], "%d", &maxchunks );
      if ( status <= 0 ) API_ErrorParse_blockmatching( (char*)NULL, "-max-chunks", 0 );
      if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
    }

    else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
              ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][3] == '\0') ) {
      i ++;
      if ( i >= argc)    API_ErrorParse_blockmatching( (char*)NULL, "-parallel-scheduling", 0 );
      if ( strcmp ( argv[i], "default" ) == 0 ) {
        setOmpScheduling( _DEFAULT_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "static" ) == 0 ) {
        setOmpScheduling( _STATIC_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "dynamic-one" ) == 0 ) {
        setOmpScheduling( _DYNAMIC_ONE_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "dynamic" ) == 0 ) {
        setOmpScheduling( _DYNAMIC_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "guided" ) == 0 ) {
        setOmpScheduling( _GUIDED_OMP_SCHEDULING_ );
      }
      else {
        fprintf( stderr, "unknown omp scheduling type: '%s'\n", argv[i] );
        API_ErrorParse_blockmatching( (char*)NULL, "-omp-scheduling", 0 );
      }
    }



    /* general parameters
     */
    else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
              || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
       API_ErrorParse_blockmatching( (char*)NULL, (char*)NULL, 1);
    }
    else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
              || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
       API_ErrorParse_blockmatching( (char*)NULL, (char*)NULL, 0);
    }
    else if ( strcmp ( argv[i], "-verbose" ) == 0
              || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
        if ( _verbose_ <= 0 ) _verbose_ = 1;
        else                  _verbose_ ++;
        if ( p->param.verbose <= 0 ) p->param.verbose = 1;
        else p->param.verbose ++;

        BAL_IncrementVerboseInBalBlockTools(  );
        BAL_IncrementVerboseInBalBlock(  );
        BAL_IncrementVerboseInBalEstimator(  );
        BAL_IncrementVerboseInBalFieldTools(  );
        BAL_IncrementVerboseInBalImageTools(  );
        BAL_IncrementVerboseInBalImage(  );
        BAL_IncrementVerboseInBalLinearTrsf(  );
        BAL_IncrementVerboseInBalBlockMatching(  );
        BAL_IncrementVerboseInBalMatrix(  );
        BAL_IncrementVerboseInBalPyramid(  );
        BAL_IncrementVerboseInBalTransformationTools(  );
        BAL_IncrementVerboseInBalTransformation(  );
        BAL_IncrementVerboseInBalVectorField(  );

        incrementVerboseInChunks(  );
        incrementVerboseInReech4x4();
        incrementVerboseInReechDef();
        incrementVerboseInLinearFiltering();
    }
    else if ( strcmp ( argv[i], "-no-verbose" ) == 0
              || strcmp ( argv[i], "-noverbose" ) == 0
              || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
        _verbose_ = 0;
        p->param.verbose = 0;

        BAL_DecrementVerboseInBalBlockTools(  );
        BAL_DecrementVerboseInBalBlock(  );
        BAL_DecrementVerboseInBalEstimator(  );
        BAL_DecrementVerboseInBalFieldTools(  );
        BAL_DecrementVerboseInBalImageTools(  );
        BAL_DecrementVerboseInBalImage(  );
        BAL_DecrementVerboseInBalLinearTrsf(  );
        BAL_DecrementVerboseInBalBlockMatching(  );
        BAL_DecrementVerboseInBalMatrix(  );
        BAL_DecrementVerboseInBalPyramid(  );
        BAL_DecrementVerboseInBalTransformationTools(  );
        BAL_DecrementVerboseInBalTransformation(  );
        BAL_DecrementVerboseInBalVectorField(  );

        decrementVerboseInChunks(  );
        decrementVerboseInReech4x4();
        decrementVerboseInReechDef();
        setVerboseInLinearFiltering( 0 );
    }
    else if ( (strcmp ( argv[i], "-debug" ) == 0 && argv[i][6] == '\0')
              || (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0') ) {
        if ( _debug_ <= 0 ) _debug_ = 1;
        else                _debug_ ++;
        BAL_IncrementDebugInBalBlockTools(  );
        BAL_IncrementDebugInBalFieldTools(  );
        BAL_IncrementDebugInBalBlockMatching(  );
        incrementDebugInChunks(  );
    }
    else if ( (strcmp ( argv[i], "-no-debug" ) == 0 && argv[i][9] == '\0')
              || (strcmp ( argv[i], "-nodebug" ) == 0 && argv[i][8] == '\0') ) {
        _debug_ = 0;
        BAL_SetDebugInBalBlockTools( 0 );
        BAL_SetDebugInBalFieldTools( 0 );
        BAL_SetDebugInBalBlockMatching( 0 );
        setDebugInChunks( 0 );
    }

    else if ( strcmp ( argv[i], "-print-parameters" ) == 0
              || (strcmp ( argv[i], "-param" ) == 0 && argv[i][6] == '\0') ) {
        p->print_lineCmdParam = 1;
    }

    else if ( strcmp ( argv[i], "-print-time" ) == 0
               || (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
       if ( p->print_time <= 0 ) {
           p->print_time = 1;
       }
       else {
           p->print_time ++;
           BAL_SetTimeInBalBlockMatching( 1 );
       }
    }
    else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')
                || (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
       p->print_time = 0;
       BAL_SetTimeInBalBlockMatching( 0 );
    }

    /* unknown option
     */
    else {
      fprintf(stderr,"unknown option: '%s'\n",argv[i]);
      API_ErrorParse_blockmatching( (char*)NULL, (char*)NULL, 0);
    }
  }

}
