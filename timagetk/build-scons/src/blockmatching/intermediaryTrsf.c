/*************************************************************************
 * intermediaryTrsf.c - template for executable creation
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

#include <api-intermediaryTrsf.h>







static int _verbose_ = 1;





/* static function definitions
 */

static char *_Array2Str( int argc, char *argv[] );
static char *_BaseName( char *p );
static double _GetTime();
static double _GetClock();






int main( int argc, char *argv[] )
{
  lineCmdParamIntermediaryTrsf par;
  bal_transformation theTrsf;
  bal_transformation resTrsf;
  bal_image templateImage;

  char *lineoptions;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  /* parameter initialization
   */
  API_InitParam_intermediaryTrsf( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_intermediaryTrsf( 1, argc, argv, &par );
  


  /***************************************************
   *
   *
   *
   ***************************************************/
  BAL_InitTransformation( &theTrsf );
  BAL_InitTransformation( &resTrsf );

  /* input transformation reading
   */

  if ( BAL_ReadTransformation( &theTrsf, par.input_name ) != 1 ) {
    if ( _verbose_ )
      API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "some error occurs during input transformation reading ...\n", -1 );
    return( -1 );
  }



  /* output transformation creation
   */


  switch ( theTrsf.type ) {

  default :

    if ( _verbose_ )
      BAL_PrintTransformation( stderr, &theTrsf, "read transformation" );
    API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "input transformation type is not handled yet ...\n", -1 );
    BAL_FreeTransformation( &theTrsf );
    return( -1 );

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

    if ( BAL_AllocTransformation( &resTrsf, theTrsf.type, (bal_image *)NULL ) != 1 ) {
      if ( _verbose_ )
        API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to allocate output transformation ...\n", -1 );
      BAL_FreeTransformation( &theTrsf );
      return( -1 );
    }
    break;

  case VECTORFIELD_2D :
  case VECTORFIELD_3D :

    /* initializing result image
       - with reference image, if any
    */
    if ( par.flag_template == 1 ) {
      if ( BAL_ReadImage( &templateImage, par.template_name, 1 ) != 1 ) {
        if ( _verbose_ )
          API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to read the template image ...\n", -1 );
        BAL_FreeTransformation( &theTrsf );
        return( -1 );
      }
    }

    /* initializing result image
       - with parameters, if any
    */
    else if ( par.template_dim.x > 0 && par.template_dim.y > 0 ) {
      if ( par.template_dim.z > 0 ) {
        if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, par.template_dim.z, 1, UCHAR ) != 1 ) {
          if ( _verbose_ )
            API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to initialize the auxiliary image ...\n", -1 );
          BAL_FreeTransformation( &theTrsf );
          return( -1 );
        }
      }
      else {
        if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, 1, 1, UCHAR ) != 1 ) {
          if ( _verbose_ )
            API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to initialize the auxiliary image (dimz=1) ...\n", -1 );
          BAL_FreeTransformation( &theTrsf );
          return( -1 );
        }
      }
      if ( par.template_voxel.x > 0.0 ) templateImage.vx = par.template_voxel.x;
      if ( par.template_voxel.y > 0.0 ) templateImage.vy = par.template_voxel.y;
      if ( par.template_voxel.z > 0.0 ) templateImage.vz = par.template_voxel.z;
    }

    /* initialisation with transformation
     */
    else {
      if ( BAL_InitImage( &templateImage, (char*)NULL,
                          theTrsf.vx.ncols, theTrsf.vx.nrows, theTrsf.vx.nplanes,
                          1, UCHAR ) != 1 ) {
        BAL_FreeTransformation( &theTrsf );
        API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to initialize the auxiliary image (vectorfield) ...\n", -1 );
        return( -1 );
      }
      templateImage.vx = theTrsf.vx.vx;
      templateImage.vy = theTrsf.vx.vy;
      templateImage.vz = theTrsf.vx.vz;
    }

    if ( theTrsf.type == VECTORFIELD_2D ) {
      templateImage.nplanes = theTrsf.vx.nplanes;
    }


    if ( BAL_AllocTransformation( &resTrsf, theTrsf.type, &templateImage ) != 1 ) {
      if ( _verbose_ )
        API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to allocate the result transformation (vectorfield) ...\n", -1 );
      BAL_FreeImage( &templateImage );
      BAL_FreeTransformation( &theTrsf );
      return( -1 );
    }

    BAL_FreeImage( &templateImage );

  }


  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeTransformation( &resTrsf );
      API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );
  }

  if ( API_intermediaryTrsf( &theTrsf, &resTrsf, par.t, lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeTransformation( &resTrsf );
      API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "some error occurs during processing ...\n", -1 );
  }
  free( lineoptions );


  /* output transformation writing
   */

  if ( BAL_WriteTransformation(&resTrsf, par.output_name) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeTransformation( &resTrsf );
      if ( _verbose_ )
        API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to write transformation ...\n", -1 );
      return( -1 );
  }

  /* memory freeing
   */
  BAL_FreeTransformation( &theTrsf );
  BAL_FreeTransformation( &resTrsf );


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
