/*************************************************************************
 * buildPyramidImage.c - template for executable creation
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
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <string-tools.h>

#include <api-buildPyramidImage.h>







static int _verbose_ = 1;
static int _debug_ = 0;




/* static function definitions
 */

static char *_Array2Str( int argc, char *argv[] );
static char *_BaseName( char *p );
static double _GetTime();
static double _GetClock();






int main( int argc, char *argv[] )
{
  lineCmdParamBuildPyramidImage par;
  bal_image image;
  stringList output_image_names;
  stringList output_trsf_names;
  char *lineoptions;

  int m, maxlevels;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  /* parameter initialization
   */
  API_InitParam_buildPyramidImage( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_buildPyramidImage( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_buildPyramidImage( 1, argc, argv, &par );
  



  /* input image reading
   */
  BAL_InitImage( &image, NULL, 0, 0, 0, 0, UCHAR );
  if ( BAL_ReadImage( &image, par.input_image_name, 0 ) != 1 ) {
      API_ErrorParse_buildPyramidImage( _BaseName( argv[0] ), "unable to read input image ...\n", 0 );
  }


  m = image.ncols;
  if ( m < (int)image.nrows ) m = image.nrows;
  if ( m < (int)image.nplanes ) m = image.nplanes;
  maxlevels = (int)( log((double)(m)) / log(2.0) ) + 2;

  if ( par.pyramid_lowest_level < 0 )
      par.pyramid_lowest_level = 0;
  if ( par.pyramid_highest_level < 0 || par.pyramid_highest_level > maxlevels )
      par.pyramid_highest_level = maxlevels;



  /* names building
   */

  initStringList( &output_image_names );
  initStringList( &output_trsf_names );

  if ( par.use_default_filename ) {
      if ( par.output_image_format[0] == '\0'
           && par.output_image_prefix[0] == '\0' ) {
          if ( par.output_image_suffix[0] == '\0' )
              (void)strcpy( par.output_image_format, "image_atlevel%02d.inr" );
          else
              sprintf( par.output_image_format, "image_atlevel%%02d.%s", par.output_image_suffix );
      }
      if ( par.output_trsf_format[0] == '\0'
           && par.output_trsf_prefix[0] == '\0' ) {
          if ( par.output_trsf_suffix[0] == '\0' )
              (void)strcpy( par.output_trsf_format, "trsf_atlevel%02d.trsf" );
          else
              sprintf( par.output_trsf_format, "trsf_atlevel%%02d.%s", par.output_trsf_suffix );
      }
  }

  if ( par.output_image_format[0] == '\0'
       && (par.output_image_prefix[0] != '\0' && par.output_image_suffix[0] != '\0' ) ) {
       sprintf( par.output_image_format, "%s%%02d.%s",  par.output_image_prefix, par.output_image_suffix );
  }

  if ( par.output_image_format[0] != '\0' ) {
      if ( buildStringListFromFormat( par.output_image_format,
                                      par.pyramid_lowest_level, par.pyramid_highest_level,
                                      &output_image_names ) != 1 ) {
          freeStringList( &output_image_names );
          BAL_FreeImage( &image );
          API_ErrorParse_buildPyramidImage( _BaseName( argv[0] ), "unable to build output image names ...\n", 0 );
      }
  }

  if ( par.output_trsf_format[0] == '\0'
       && par.output_trsf_prefix[0] != '\0' ) {
      if ( par.output_trsf_suffix[0] != '\0' )
          sprintf( par.output_trsf_format, "%s%%02d.%s",  par.output_trsf_prefix, par.output_trsf_suffix );
      else
          sprintf( par.output_trsf_format, "%s%%02d",  par.output_trsf_prefix );
  }

  if ( par.output_trsf_format[0] != '\0' ) {
      if ( buildStringListFromFormat( par.output_trsf_format,
                                      par.pyramid_lowest_level, par.pyramid_highest_level,
                                      &output_trsf_names ) != 1 ) {
          freeStringList( &output_trsf_names );
          freeStringList( &output_image_names );
          BAL_FreeImage( &image );
          API_ErrorParse_buildPyramidImage( _BaseName( argv[0] ), "unable to build output transformation names ...\n", 0 );
      }
  }


  if ( _debug_ ) {
      fprintf( stderr, "\n" );
      fprintf( stderr, "%s, image names\n", _BaseName( argv[0] ) );
      printStringList( stderr, &output_image_names, (char*)NULL );
      fprintf( stderr, "%s, transformation names\n", _BaseName( argv[0] ) );
      printStringList( stderr, &output_trsf_names, (char*)NULL );
      fprintf( stderr, "\n" );
  }


  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL )
      API_ErrorParse_buildPyramidImage( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );

  if ( API_buildPyramidImage( &image, &output_image_names, &output_trsf_names, lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      freeStringList( &output_trsf_names );
      freeStringList( &output_image_names );
      BAL_FreeImage( &image );
      API_ErrorParse_buildPyramidImage( _BaseName( argv[0] ), "some error occurs during processing ...\n", -1 );
  }
  free( lineoptions );

  
  /* memory freeing
   */
  freeStringList( &output_trsf_names );
  freeStringList( &output_image_names );
  BAL_FreeImage( &image );



  time_exit = _GetTime();
  clock_exit = _GetClock();

  if ( par.print_time ) { 
    fprintf( stderr, "%s: elapsed (real) time = %f\n", _BaseName( argv[0] ), time_exit - time_init );
    fprintf( stderr, "\t       elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
    fprintf( stderr, "\t       ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
  }


  return( 1 );
}





/************************************************************
 *
 * static functions
 *
 ************************************************************/



static char *_Array2Str( int argc, char *argv[] )
{
  char *proc = "_Array2Str";
  int i, l;
  char *s, *t;

  if ( argc <= 1 || argv == (char**)NULL ) {
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: no options in argv[]\n", proc );
    return( (char*)NULL );
  }

  /* there are argc-1 strings
   * compute the sum of string lengths from 1 to argc-1
   * + number of interval between successive strings (argc-2)
   * + 1 to add a trailing '\0'
   */
  for ( l=argc-1, i=1; i<argc; i++ ) {
    l += strlen( argv[i] );
  }

  s = (char*)malloc( l * sizeof( char ) );
  if ( s == (char*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation failed\n", proc );
    return( (char*)NULL );
  }

  for ( t=s, i=1; i<argc; i++ ) {
    (void)strncpy( t, argv[i], strlen( argv[i] ) );
    t += strlen( argv[i] );
    if ( i < argc-1 ) {
      *t = ' ';
      t++;
    }
    else {
      *t = '\0';
    }
  }

  return( s );
}



static char *_BaseName( char *p )
{
  int l;
  if ( p == (char*)NULL ) return( (char*)NULL );
  l = strlen( p ) - 1;
  while ( l >= 0 && p[l] != '/' ) l--;
  if ( l < 0 ) l = 0;
  if ( p[l] == '/' ) l++;
  return( &(p[l]) );
}



static double _GetTime()
{
  struct timeval tv;
  gettimeofday(&tv, (void *)0);
  return ( (double) tv.tv_sec + tv.tv_usec*1e-6 );
}



static double _GetClock() 
{
  return ( (double) clock() / (double)CLOCKS_PER_SEC );
}
