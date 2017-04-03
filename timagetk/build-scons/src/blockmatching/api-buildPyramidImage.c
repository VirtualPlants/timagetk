/*************************************************************************
 * api-buildPyramidImage.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Jeu  1 oct 2015 14:43:43 CEST
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>
#include <linearFiltering.h>

#include <api-buildPyramidImage.h>
#include <bal-pyramid.h>





static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_buildPyramidImage( char *str, lineCmdParamBuildPyramidImage *p );



/************************************************************
 *
 * main API
 *
 * This API acts differently than others. Here, all output images
 * are not allocated beforehand. Instead, the list of output images
 * is passed as a parameter, and images will be written by
 * the procedure.
 *
 ************************************************************/



int API_buildPyramidImage( bal_image *image,
                           stringList *output_image_names,
                           stringList *output_trsfs_names,
                           char *param_str_1, char *param_str_2 )
{
  char *proc = "API_buildPyramidImage";
  lineCmdParamBuildPyramidImage par;



  /* parameter initialization
   */
  API_InitParam_buildPyramidImage( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_buildPyramidImage( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_buildPyramidImage( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_buildPyramidImage( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( BAL_BuildPyramidImage( image, output_image_names, output_trsfs_names,
                              par.pyramid_lowest_level,
                              par.pyramid_highest_level,
                              par.pyramid_gaussian_filtering ) != 1 ) {
    if ( _verbose_ )
        fprintf( stderr, "%s: unable to build pyramid\n", proc );
    return( -1 );
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



static char *usage = "[image-in]\n\
 [[-res-image|-res-image-format] format-image-out] \n\
 [-res-image-prefix|-image-prefix prefix-image-out]\n\
 [-res-image-suffix|-image-suffix suffix-image-out]\n\
 [-res-trsf|-res-trsf-format] format-trsf-out] \n\
 [-res-trsf-prefix|-trsf-prefix prefix-trsf-out] [-res-trsf-suffix|-trsf-suffix suffix-trsf-out]\n\
 [-normalisation|-norma|-rescale|-no-normalisation|-no-norma|-no-rescale]\n\
 [-pyramid-lowest-level | -py-ll %d] [-pyramid-highest-level | -py-hl %d]\n\
 [-pyramid-gaussian-filtering | -py-gf]\n\
 [-gaussian-filter-type|-filter-type deriche|fidrich|young-1995|young-2002|...\n\
  ...|gabor-young-2002|convolution]\n\
 [-default-filenames|-df] [-no-default-filenames|-ndf]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'image-in' is equal to '-', stdin will be used\n\
# file names\n\
 -res-image|-res-image-format %s: format 'a la printf' for output images\n\
   must contain a '%d' that will be used for #level\n\
 -res-image-prefix|-image-prefix %s:\n\
 -res-image-suffix|-image-suffix %s:\n\
   output image names will be built by 'image-prefix'%02d.'image-suffix'\n\
 -res-trsf|-res-trsf-format %s: format 'a la printf' for output transformations\n\
   must contain a '%d' that will be used for #level\n\
 -res-trsf-prefix|-trsf-prefix %s:\n\
 -res-trsf-suffix|-trsf-suffix %s:\n\
   output transformations names will be built by 'trsf-prefix'%02d.'trsf-suffix'\n\
# pre-processing\n\
 -normalisation|-norma|-rescale: input image is normalized on one byte\n\
 -no-normalisation|-no-norma|-no-rescale: input image is not normalized on one byte\n\
# pyramid building\n\
 -pyramid-lowest-level | -py-ll %d: pyramid lowest level\n\
  (0 = original dimension)\n\
 -pyramid-highest-level | -py-hl %d: pyramid highest level\n\
 -pyramid-gaussian-filtering | -py-gf: before subsampling, the image \n\
 is filtered (ie smoothed) by a gaussian kernel.\n\
# filter type\n\
 -gaussian-filter-type|-filter-type deriche|fidrich|young-1995|young-2002|...\n\
  ...|gabor-young-2002|convolution: type of filter for gaussian filtering\n\
# misc writing stuff\n\
 -default-filenames|-df: use default filename names\n\
 -no-default-filenames|-ndf: do not use default filename names\n\
# parallelism parameters\n\
 -parallel|-no-parallel:\n\
 -max-chunks %d:\n\
 -parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread:\n\
 -omp-scheduling|-omps default|static|dynamic-one|dynamic|guided:\n\
# general parameters \n\
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
";





char *API_Help_buildPyramidImage( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_buildPyramidImage( char *program, char *str, int flag )
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



void API_InitParam_buildPyramidImage( lineCmdParamBuildPyramidImage *p )
{
    (void)strncpy( p->input_image_name, "\0", 1 );

    (void)strncpy( p->output_image_format, "\0", 1 );
    (void)strncpy( p->output_image_prefix, "\0", 1 );
    (void)strncpy( p->output_image_suffix, "\0", 1 );

    (void)strncpy( p->output_trsf_format, "\0", 1 );
    (void)strncpy( p->output_trsf_prefix, "\0", 1 );
    (void)strncpy( p->output_trsf_suffix, "trsf", 4 );

    p->normalisation = 0;

    p->pyramid_lowest_level = 0;
    p->pyramid_highest_level = -1;
    p->pyramid_gaussian_filtering = 0;

    p->use_default_filename = 0;

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_buildPyramidImage( FILE *theFile, char *program,
                                  lineCmdParamBuildPyramidImage *p, char *str )
{
  FILE *f = theFile;
  if ( theFile == (FILE*)NULL ) f = stderr;

  fprintf( f, "==================================================\n" );
  fprintf( f, "= in line command parameters" );
  if ( program != (char*)NULL )
    fprintf( f, " for '%s'", program );
  if ( str != (char*)NULL )
    fprintf( f, "= %s\n", str );
  fprintf( f, "\n"  );
  fprintf( f, "==================================================\n" );


  fprintf( f, "# file names\n" );

  fprintf( f, "- input image is " );
  if ( p->input_image_name != (char*)NULL && p->input_image_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_image_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output image format is " );
  if ( p->output_image_format != (char*)NULL && p->output_image_format[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_image_format );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output image prefix is " );
  if ( p->output_image_prefix != (char*)NULL && p->output_image_prefix[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_image_prefix );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output image suffix is " );
  if ( p->output_image_suffix != (char*)NULL && p->output_image_suffix[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_image_suffix );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation format is " );
  if ( p->output_trsf_format != (char*)NULL && p->output_trsf_format[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_trsf_format );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation prefix is " );
  if ( p->output_trsf_prefix != (char*)NULL && p->output_trsf_prefix[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_trsf_prefix );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation suffix is " );
  if ( p->output_trsf_suffix != (char*)NULL && p->output_trsf_suffix[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_trsf_suffix );
  else
    fprintf( f, "'NULL'\n" );


  fprintf( f, "# specific parameters\n" );
  fprintf( f, "- normalisation = %d\n", p->normalisation );
  fprintf( f, "- lowest level of pyramid to be written out = %d\n", p->pyramid_lowest_level );
  fprintf( f, "- highest level of pyramid to be written out = %d\n", p->pyramid_highest_level );
  fprintf( f, "- gaussian filtering for pyramid building = %d\n", p->pyramid_gaussian_filtering );

  fprintf( f, "# writing stuff\n" );
  fprintf( f, "- p->use_default_filename = %d\n", p->use_default_filename );

  fprintf( f, "==================================================\n" );
}





/************************************************************
 *
 * parameters parsing
 *
 ************************************************************/



static void _API_ParseParam_buildPyramidImage( char *str, lineCmdParamBuildPyramidImage *p )
{
  char *proc = "_API_ParseParam_buildPyramidImage";
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

  API_ParseParam_buildPyramidImage( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_buildPyramidImage( int firstargc, int argc, char *argv[],
                                  lineCmdParamBuildPyramidImage *p )
{
  int i;
  int inputisread = 0;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;



  /* option line parsing
   */
  for ( i=firstargc; i<argc; i++ ) {

      /* strings beginning with '-'
       */
      if ( argv[i][0] == '-' ) {
          if ( argv[i][1] == '\0' ) {
            if ( inputisread == 0 ) {
              (void)strcpy( p->input_image_name,  "<" );  /* standart input */
              inputisread = 1;
            }
            else {
              API_ErrorParse_buildPyramidImage( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* file names
           */
          else if ( (strcmp( argv[i], "-res-image") == 0 && argv[i][10] == '\0')
                    || strcmp( argv[i], "-res-image-format") == 0 ) {
              (void)strcpy( p->output_image_format, argv[i] );
          }

          /* pre-processing
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

          /* pyramid building
           */
          else if ( strcmp ( argv[i], "-pyramid-lowest-level" ) == 0
                    || (strcmp( argv[i], "-py-ll") == 0 && argv[i][6] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -pyramid-lowest-level", 0 );
            status = sscanf( argv[i], "%d", &(p->pyramid_lowest_level) );
            if ( status <= 0 ) API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -pyramid-lowest-level", 0 );
          }
          else if ( strcmp ( argv[i], "-pyramid-highest-level" ) == 0
                    || (strcmp( argv[i], "-py-hl") == 0 && argv[i][6] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -pyramid-highest-level", 0 );
            status = sscanf( argv[i], "%d", &(p->pyramid_highest_level) );
            if ( status <= 0 ) API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -pyramid-highest-level", 0 );
          }
          else if ( strcmp ( argv[i], "-pyramid-gaussian-filtering" ) == 0
                    || (strcmp( argv[i], "-py-gf") == 0 && argv[i][6] == '\0') ) {
            p->pyramid_gaussian_filtering = 1;
          }

          /* filter type for image smoothing
           */
          else if ( strcmp ( argv[i], "-gaussian-filter-type" ) == 0
                    || strcmp ( argv[i], "-filter-type" ) == 0 ) {
            i++;
            if ( i >= argc)    API_ErrorParse_buildPyramidImage( (char*)NULL, "-gaussian-filter-type", 0 );
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
              API_ErrorParse_buildPyramidImage( (char*)NULL, "-gaussian-filter-type", 0 );
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

          /* parallelism parameters
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
             if ( i >= argc)    API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_buildPyramidImage( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_buildPyramidImage( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_buildPyramidImage( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;

              BAL_IncrementVerboseInBalImage(  );
              BAL_IncrementVerboseInBalPyramid(  );

              incrementVerboseInChunks(  );
              incrementVerboseInLinearFiltering();
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              _verbose_ = 0;

              BAL_SetVerboseInBalImage( 0 );
              BAL_SetVerboseInBalPyramid( 0 );

              decrementVerboseInChunks(  );
              setVerboseInLinearFiltering( 0 );
          }
          else if ( (strcmp ( argv[i], "-debug" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0') ) {
              if ( _debug_ <= 0 ) _debug_ = 1;
              else                _debug_ ++;

              BAL_IncrementDebugInBalPyramid( );
              incrementDebugInChunks(  );
          }
          else if ( (strcmp ( argv[i], "-no-debug" ) == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-nodebug" ) == 0 && argv[i][8] == '\0') ) {
              _debug_ = 0;

              BAL_SetDebugInBalPyramid( 0 );
              setDebugInChunks( 0 );
          }

          else if ( strcmp ( argv[i], "-print-parameters" ) == 0
                    || (strcmp ( argv[i], "-param" ) == 0 && argv[i][6] == '\0') ) {
             p->print_lineCmdParam = 1;
          }

          else if ( strcmp ( argv[i], "-print-time" ) == 0
                     || (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
             p->print_time = 1;
          }
          else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')
                      || (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
             p->print_time = 0;
          }

          /* unknown option
           */
          else {
              sprintf(text,"unknown option %s\n",argv[i]);
              API_ErrorParse_buildPyramidImage( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_buildPyramidImage( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( inputisread == 0 ) {
              (void)strcpy( p->input_image_name, argv[i] );
              inputisread = 1;
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_image_format, argv[i] );
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_buildPyramidImage( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
  if ( inputisread == 0 ) {
    (void)strcpy( p->input_image_name,  "<" );  /* standart input */
    inputisread = 1;
  }


}
