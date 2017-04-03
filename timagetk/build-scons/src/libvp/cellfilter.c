/*************************************************************************
 * cellfilter.c - template for executable creation
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

#include <api-cellfilter.h>

#include <vt_common.h>





static int _verbose_ = 1;





/* static function definitions
 */

static char *_Array2Str( int argc, char *argv[] );
static char *_BaseName( char *p );
static double _GetTime();
static double _GetClock();






int main( int argc, char *argv[] )
{
  lineCmdParamCellfilter par;
  vt_image *image, imres;
  vt_image imtmp, *imout;
  typeStructuringElement SE;
  char *lineoptions;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  /* parameter initialization
   */
  API_InitParam_cellfilter( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_cellfilter( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_cellfilter( 1, argc, argv, &par );
  


  /* input image reading
   */
  image = _VT_Inrimage( par.input_name );
  if ( image == (vt_image*)NULL ) {
      API_ErrorParse_cellfilter( _BaseName( argv[0] ), "unable to read input image ...\n", 0 );
  }

  if ( par.input_inv == 1 )  VT_InverseImage( image );
  if ( par.input_swap == 1 ) VT_SwapImage( image );



  /* structuring element
   */
  initStructuringElement( &SE );
  if ( par.element_name[0] != '\0' ) {
      if ( readStructuringElement( par.element_name, &SE ) != 1 ) {
          API_ErrorParse_cellfilter( _BaseName( argv[0] ), "unable to read structuring element ...\n", 0 );
      }
  }



  /* output image creation
   */

  VT_Image( &imres );
  VT_InitFromImage( &imres, image, par.output_name, image->type );
  if ( par.output_type != TYPE_UNKNOWN ) imres.type = par.output_type;
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    API_ErrorParse_cellfilter( _BaseName( argv[0] ), "unable to allocate output image ...\n", 0 );
  }

  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL )
      API_ErrorParse_cellfilter( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );

  if ( API_cellfilter( image, &imres, &SE,
                       lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      API_ErrorParse_cellfilter( _BaseName( argv[0] ), "some error occurs during processing ...\n", 0 );
  }



  /* memory freeing
   */
  free( lineoptions );
  freeStructuringElement( &SE );
  VT_FreeImage( image );
  VT_Free( (void**)&image );



  /* output image writing
   */
  imout = &imres;

  if ( par.output_type != TYPE_UNKNOWN ) {
      VT_InitFromImage( &imtmp, &imres, par.output_name, par.output_type  );
      if ( VT_AllocImage( &imtmp ) != 1 ) {
          VT_FreeImage( &imres );
          API_ErrorParse_cellfilter( _BaseName( argv[0] ), "unable to allocate auxiliary image ...\n", 0 );
      }
      if ( VT_CopyImage( &imres, &imtmp ) != 1 ) {
          VT_FreeImage( &imtmp );
          VT_FreeImage( &imres );
          API_ErrorParse_cellfilter( _BaseName( argv[0] ), "unable to copy result image ...\n", 0 );
      }
      VT_FreeImage( &imres );
      imout = &imtmp;
    }

  if ( VT_WriteInrimage( imout ) == -1 ) {
    VT_FreeImage( imout );
    API_ErrorParse_cellfilter( _BaseName( argv[0] ), "unable to write output image ...\n", 0 );
  }



  /* memory freeing
   */
  VT_FreeImage( &imres );




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
