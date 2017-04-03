/*************************************************************************
 * execTemplate.c - template for executable creation
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

#include <api-execTemplate.h>







static int _verbose_ = 1;





/* static function definitions
 */

static char *_Array2Str( int argc, char *argv[] );
static char *_BaseName( char *p );
static double _GetTime();
static double _GetClock();






int main( int argc, char *argv[] )
{
  lineCmdParamExecTemplate par;
  bal_image image, imres;
  char *lineoptions;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  /* parameter initialization
   */
  API_InitParam_execTemplate( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_execTemplate( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_execTemplate( 1, argc, argv, &par );
  


  /* input image reading
   */
  BAL_InitImage( &image, NULL, 0, 0, 0, 0, UCHAR );
  if ( BAL_ReadImage( &image, par.input_name, 0 ) != 1 ) {
      API_ErrorParse_execTemplate( _BaseName( argv[0] ), "unable to read input image ...\n", 0 );
  }




  /* output image creation
   */

  BAL_InitImage( &imres, par.output_name, image.ncols, image.nrows, image.nplanes, image.vdim, image.type );
  if ( par.output_type != TYPE_UNKNOWN ) imres.type = par.output_type;
  if ( BAL_AllocImage( &imres ) != 1 ) {
    BAL_FreeImage( &image );
    API_ErrorParse_execTemplate( _BaseName( argv[0] ), "unable to allocate output image ...\n", 0 );
  }

  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL )
      API_ErrorParse_execTemplate( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );

  if ( API_execTemplate( &image, &imres, lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      BAL_FreeImage( &imres );
      BAL_FreeImage( &image );
      API_ErrorParse_execTemplate( _BaseName( argv[0] ), "some error occurs during processing ...\n", -1 );
  }
  free( lineoptions );


  /* output image writing
   */

  if ( BAL_WriteImage( &imres, par.output_name ) != 1 ) {
      BAL_FreeImage( &imres );
      BAL_FreeImage( &image );
    API_ErrorParse_execTemplate( _BaseName( argv[0] ), "unable to write output image ...\n", -1 );
  }
  
  /* memory freeing
   */
  BAL_FreeImage( &imres );
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
