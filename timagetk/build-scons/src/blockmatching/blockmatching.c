/*************************************************************************
 * blockmatching.c - template for executable creation
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Jeu  2 jul 2015 13:53:42 CEST
 *
 * ADDITIONS, CHANGES
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <api-blockmatching.h>







static int _verbose_ = 1;
static char *defaultprefix = "blockmatching";




/* static function definitions
 */

static char *_Array2Str( int argc, char *argv[] );
static char *_BaseName( char *p );
static double _GetTime();
static double _GetClock();






int main( int argc, char *argv[] )
{
  lineCmdParamBlockmatching par;
  char *lineoptions;
  FILE *fcmdl = (FILE*)NULL;
  int i;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;




  /* parameter initialization
   */
  API_InitParam_blockmatching( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_blockmatching( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_blockmatching( 1, argc, argv, &par );


  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL )
      API_ErrorParse_blockmatching( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );

  if ( par.command_line_file[0] == '\0' && par.use_default_filename == 1 ) {
    sprintf( par.command_line_file, "%s-%d-cmdline.log", defaultprefix, getpid() );
  }
  if ( par.command_line_file[0] != '\0' ) {
    if ( strlen( par.command_line_file ) == 6 && strcmp( par.command_line_file, "stderr" ) == 0 )
      fcmdl = stderr;
    else if ( strlen( par.command_line_file ) == 6 && strcmp( par.command_line_file, "stdout" ) == 0 )
      fcmdl = stdout;
    else {
      fcmdl = fopen( par.command_line_file, "w" );
      if ( fcmdl == NULL ) {
        if ( _verbose_ ) {
            fprintf( stderr, "%s: unable to open '%s' for writing, switch to 'stderr'\n",
                     _BaseName( argv[0] ), par.command_line_file );
        }
        fcmdl = stderr;
      }
    }
  }
  if ( fcmdl != (FILE*)NULL ) {
    for ( i=0; i<argc; i++ ) {
      fprintf( fcmdl, "%s", argv[i] );
      if ( i < argc-1 )
        fprintf( fcmdl, " " );
    }
    fprintf( fcmdl, "\n" );
    if ( fcmdl != stdout && fcmdl != stderr )
      fclose( fcmdl );
  }



  if ( API_INTERMEDIARY_blockmatching( par.floating_image,
                                       par.reference_image,
                                       par.result_image,
                                       par.left_real_transformation,
                                       par.left_voxel_transformation,
                                       par.initial_result_real_transformation,
                                       par.initial_result_voxel_transformation,
                                       par.result_real_transformation,
                                       par.result_voxel_transformation,
                                       lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      API_ErrorParse_blockmatching( _BaseName( argv[0] ), "some error occurs during processing ...\n", -1 );
  }
  free( lineoptions );





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
