/*************************************************************************
 * interpolateImages.c - template for executable creation
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

#include <api-interpolateImages.h>
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
  lineCmdParamInterpolateImages par;
  bal_image image0, image1, imres;
  bal_transformation T_0_1, T_1_t;
  bal_image templateImage;
  char *lineoptions;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  int i;
  double t;
  char name[STRINGLENGTH];


  /* parameter initialization
   */
  API_InitParam_interpolateImages( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_interpolateImages( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_interpolateImages( 1, argc, argv, &par );
  


  /* input images reading
   */
  BAL_InitImage( &image0, NULL, 0, 0, 0, 0, UCHAR );
  if ( BAL_ReadImage( &image0, par.input_name_0, 0 ) != 1 ) {
      API_ErrorParse_interpolateImages( _BaseName( argv[0] ), "unable to read input image 0...\n", 0 );
  }

  BAL_InitImage( &image1, NULL, 0, 0, 0, 0, UCHAR );
  if ( BAL_ReadImage( &image1, par.input_name_1, 0 ) != 1 ) {
      BAL_FreeImage(&image0);
      API_ErrorParse_interpolateImages( _BaseName( argv[0] ), "unable to read input image 0...\n", 0 );
  }

  /* input transformation reading
   */
  BAL_InitTransformation( &T_0_1 );
  if ( BAL_ReadTransformation( &T_0_1, par.input_trsf ) != 1 ) {
    BAL_FreeImage(&image0);
    BAL_FreeImage(&image1);
    if ( _verbose_ )
      API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "some error occurs during input transformation reading ...\n", -1 );
    return( -1 );
  }


  /* output image creation
   */

  BAL_InitImage( &imres, (char *)NULL, T_0_1.vx.ncols, T_0_1.vx.nrows, T_0_1.vx.nplanes, image0.vdim, image0.type );
  if ( par.output_type != TYPE_UNKNOWN ) imres.type = par.output_type;
  if ( BAL_AllocImage( &imres ) != 1 ) {
    BAL_FreeImage( &image0 );
    BAL_FreeImage( &image1 );
    BAL_FreeTransformation( &T_0_1 );
    API_ErrorParse_interpolateImages( _BaseName( argv[0] ), "unable to allocate output image ...\n", 0 );
  }

  /* intermediary transformation creation
   */

  BAL_InitTransformation( &T_1_t );
  switch ( T_0_1.type ) {

  default :

    if ( _verbose_ )
      BAL_PrintTransformation( stderr, &T_0_1, "read transformation" );
    BAL_FreeImage( &image0 );
    BAL_FreeImage( &image1 );
    BAL_FreeTransformation( &T_0_1 );
    API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "input transformation type is not handled yet ...\n", -1 );
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

    if ( BAL_AllocTransformation( &T_1_t, T_0_1.type, (bal_image *)NULL ) != 1 ) {
      BAL_FreeImage( &image0 );
      BAL_FreeImage( &image1 );
      BAL_FreeTransformation( &T_0_1 );
      if ( _verbose_ )
        API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to allocate output transformation ...\n", -1 );
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
        BAL_FreeImage( &image0 );
        BAL_FreeImage( &image1 );
        BAL_FreeTransformation( &T_0_1 );
        if ( _verbose_ )
          API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to read the template image ...\n", -1 );
        return( -1 );
      }
    }

    /* initializing result image
       - with parameters, if any
    */
    else if ( par.template_dim.x > 0 && par.template_dim.y > 0 ) {
      if ( par.template_dim.z > 0 ) {
        if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, par.template_dim.z, 1, UCHAR ) != 1 ) {
          BAL_FreeImage( &image0 );
          BAL_FreeImage( &image1 );
          BAL_FreeTransformation( &T_0_1 );
          if ( _verbose_ )
            API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to initialize the auxiliary image ...\n", -1 );
          return( -1 );
        }
      }
      else {
        if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, 1, 1, UCHAR ) != 1 ) {
          BAL_FreeImage( &image0 );
          BAL_FreeImage( &image1 );
          BAL_FreeTransformation( &T_0_1 );
          if ( _verbose_ )
            API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to initialize the auxiliary image (dimz=1) ...\n", -1 );
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
                          T_0_1.vx.ncols, T_0_1.vx.nrows, T_0_1.vx.nplanes,
                          1, UCHAR ) != 1 ) {
        BAL_FreeImage( &image0 );
        BAL_FreeImage( &image1 );
        BAL_FreeTransformation( &T_0_1 );
        API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to initialize the auxiliary image (vectorfield) ...\n", -1 );
        return( -1 );
      }
      templateImage.vx = T_0_1.vx.vx;
      templateImage.vy = T_0_1.vx.vy;
      templateImage.vz = T_0_1.vx.vz;
    }

    if ( T_0_1.type == VECTORFIELD_2D ) {
      templateImage.nplanes = T_0_1.vx.nplanes;
    }


    if ( BAL_AllocTransformation( &T_1_t, T_0_1.type, &templateImage ) != 1 ) {
      BAL_FreeImage( &templateImage );
      BAL_FreeImage( &image0 );
      BAL_FreeImage( &image1 );
      BAL_FreeTransformation( &T_0_1 );
      if ( _verbose_ )
        API_ErrorParse_intermediaryTrsf( _BaseName( argv[0] ), "unable to allocate the result transformation (vectorfield) ...\n", -1 );
      return( -1 );
    }

    BAL_FreeImage( &templateImage );

  }

  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL )
      API_ErrorParse_interpolateImages( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );

  for (i = 0 ; i < par.n_img ; i++) {

    t = ((double)i+1)/(par.n_img+1);

    /* output image name */

    sprintf( name, par.output_format, t );

    if (_verbose_)
        fprintf(stdout, "Computing %s ...\n", name);

    /* Here is the stuff */

    /* INTERMEDIARY TRANSFORMATION */
    if ( API_intermediaryTrsfStuff(&T_0_1, &T_1_t, t, 0) != 1 ) {
        free( lineoptions );
        BAL_FreeImage( &imres );
        BAL_FreeImage( &image0 );
        BAL_FreeImage( &image1 );
        BAL_FreeTransformation( &T_1_t );
        BAL_FreeTransformation( &T_0_1 );
        API_ErrorParse_interpolateImages( _BaseName( argv[0] ), "some error occurs during intermediary transformation processing ...\n", -1 );
    }

    /* INTERPOLATION */
    if ( API_interpolateImages( &image0, &image1, &T_1_t, &imres, t, lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      BAL_FreeImage( &imres );
      BAL_FreeImage( &image0 );
      BAL_FreeImage( &image1 );
      BAL_FreeTransformation( &T_1_t );
      BAL_FreeTransformation( &T_0_1 );
      API_ErrorParse_interpolateImages( _BaseName( argv[0] ), "some error occurs during processing ...\n", -1 );
    }

    /* output image writing
     */

    if ( BAL_WriteImage( &imres, name ) != 1 ) {
        free( lineoptions );
        BAL_FreeImage( &imres );
        BAL_FreeImage( &image0 );
        BAL_FreeImage( &image1 );
        BAL_FreeTransformation( &T_1_t );
        BAL_FreeTransformation( &T_0_1 );
        API_ErrorParse_interpolateImages( _BaseName( argv[0] ), "unable to write output image ...\n", -1 );
    }
  }

  free( lineoptions );


  
  /* memory freeing
   */
  BAL_FreeImage( &imres );
  BAL_FreeImage( &image0 );
  BAL_FreeImage( &image1 );
  BAL_FreeTransformation( &T_1_t );
  BAL_FreeTransformation( &T_0_1 );


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
