/*************************************************************************
 * linearFilter.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Fri Nov 30 21:58:59 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <api-linearFilter.h>

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
  lineCmdParamLinearFilter par;
  vt_image *image, imres;
  char *lineoptions;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  /* parameter initialization
   */
  API_InitParam_linearFilter( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_linearFilter( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_linearFilter( 1, argc, argv, &par );
  


  /* input image reading
   */
  image = _VT_Inrimage( par.input_name );
  if ( image == (vt_image*)NULL ) {
      API_ErrorParse_linearFilter( _BaseName( argv[0] ), "unable to read input image ...\n", 0 );
  }

  if ( par.input_inv == 1 )  VT_InverseImage( image );
  if ( par.input_swap == 1 ) VT_SwapImage( image );



  /* output image creation
   */

  if ( par.output_type == TYPE_UNKNOWN ) {
    switch ( par.typeOutput ) {
    default :
    case _FILTER_ :
        if ( (par.filter[0].derivative == NODERIVATIVE || par.filter[0].derivative == DERIVATIVE_0)
             && (par.filter[1].derivative == NODERIVATIVE || par.filter[1].derivative == DERIVATIVE_0)
             && (par.filter[2].derivative == NODERIVATIVE || par.filter[2].derivative == DERIVATIVE_0) )
            par.output_type = image->type;
        else
            par.output_type = FLOAT;
        break;
    case _GRADIENT_MODULUS_ :   par.output_type = FLOAT; break;
    case _HESSIAN_ :            par.output_type = FLOAT; break;
    case _LAPLACIAN_ :          par.output_type = FLOAT; break;
    case _ZCROSSINGS_HESSIAN_ :   par.output_type = UCHAR; break;
    case _ZCROSSINGS_LAPLACIAN_ : par.output_type = UCHAR; break;
    case _GRADIENT_HESSIAN_ :   par.output_type = FLOAT; break;
    case _GRADIENT_LAPLACIAN_ : par.output_type = FLOAT; break;
    case _EXTREMA_GRADIENT_ :   par.output_type = FLOAT; break;
    }
  }

  VT_Image( &imres );
  VT_InitFromImage( &imres, image, par.output_name, image->type );
  if ( par.output_type != TYPE_UNKNOWN ) imres.type = par.output_type;
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    API_ErrorParse_linearFilter( _BaseName( argv[0] ), "unable to allocate output image ...\n", 0 );
  }

  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL )
      API_ErrorParse_linearFilter( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );

  if ( API_linearFilter( image, &imres, lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      API_ErrorParse_linearFilter( _BaseName( argv[0] ), "some error occurs during processing ...\n", 0 );
  }
  free( lineoptions );


  /* output image writing
   */

  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( &imres );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    API_ErrorParse_linearFilter( _BaseName( argv[0] ), "unable to write output image ...\n", 0 );
  }
  
  /* memory freeing
   */
  VT_FreeImage( &imres );
  VT_FreeImage( image );
  VT_Free( (void**)&image );



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
