/*************************************************************************
 * connexe.c - template for executable creation
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

#include <api-connexe.h>

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
  lineCmdParamConnexe par;
  vt_image *image = (vt_image*)NULL;
  vt_image *imseeds = (vt_image*)NULL;
  vt_image *imout = (vt_image*)NULL;
  vt_image imtmp, imres;
  vt_3m m;
  char *lineoptions;


  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  /* parameter initialization
   */
  API_InitParam_connexe( &par );



  /* parameter parsing
   */
  if ( argc <= 1 )
      API_ErrorParse_connexe( _BaseName( argv[0] ), (char*)NULL, 0 );
  API_ParseParam_connexe( 1, argc, argv, &par );
  


  /* input image reading
   */
  image = _VT_Inrimage( par.input_name );
  if ( image == (vt_image*)NULL ) {
      API_ErrorParse_connexe( _BaseName( argv[0] ), "unable to read input image ...\n", 0 );
  }

  if ( par.input_inv == 1 )  VT_InverseImage( image );
  if ( par.input_swap == 1 ) VT_SwapImage( image );


  if ( par.input_seeds_name != (char*)NULL && par.input_seeds_name[0] != '\0' ) {
      imseeds = _VT_Inrimage( par.input_seeds_name );
      if ( imseeds == (vt_image*)NULL ) {
          VT_FreeImage( image );
          VT_Free( (void**)&image );
          API_ErrorParse_connexe( _BaseName( argv[0] ), "unable to read seeds image ...\n", 0 );
      }
  }


  /* auxiliary image creation
   */

  VT_Image( &imtmp );
  VT_InitFromImage( &imtmp, image, par.output_name, USHORT );
  if ( VT_AllocImage( &imtmp ) != 1 ) {
      if ( imseeds != (vt_image*)NULL ) {
          VT_FreeImage( imseeds );
          VT_Free( (void**)&imseeds );
      }
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      API_ErrorParse_connexe( _BaseName( argv[0] ), "unable to allocate auxiliary image ...\n", 0 );
  }

  /* API call
   */

  lineoptions = _Array2Str( argc, argv );
  if ( lineoptions == (char*)NULL )
      API_ErrorParse_connexe( _BaseName( argv[0] ), "unable to translate command line options ...\n", 0 );

  if ( API_connexe( image, imseeds, &imtmp, lineoptions, (char*)NULL ) != 1 ) {
      free( lineoptions );
      VT_FreeImage( &imres );
      if ( imseeds != (vt_image*)NULL ) {
          VT_FreeImage( imseeds );
          VT_Free( (void**)&imseeds );
      }
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      API_ErrorParse_connexe( _BaseName( argv[0] ), "some error occurs during processing ...\n", 0 );
  }
  free( lineoptions );



  /* freeing some images
   */

  if ( imseeds != (vt_image*)NULL ) {
      VT_FreeImage( imseeds );
      VT_Free( (void**)&imseeds );
  }
  VT_FreeImage( image );
  VT_Free( (void**)&image );



  /* output image: known type
   */
  if ( par.output_type != TYPE_UNKNOWN ) {
      if ( imtmp.type == par.output_type ) {
          imout = &imtmp;
      }
      else {
          VT_InitFromImage( &imres, &imtmp, par.output_name, par.output_type );
          if ( VT_AllocImage( &imres ) != 1 ) {
            VT_FreeImage( &imtmp );
            API_ErrorParse_connexe( _BaseName( argv[0] ), "allocation error of output image ...\n", 0 );
          }
          if ( VT_CopyImage( &imtmp, &imres ) != 1 ) {
            VT_FreeImage( &imres );
            VT_FreeImage( &imtmp );
            API_ErrorParse_connexe( _BaseName( argv[0] ), "copy error of output image ...\n", 0 );
          }
          VT_FreeImage( &imtmp );
          imout = &imres;
      }
  }
  /* output image: unknown type
   */
  else {
      switch( par.cpar.type_output ) {
      default :
        VT_FreeImage( &imtmp );
        API_ErrorParse_connexe( _BaseName( argv[0] ), "embarrassing, output type undefined or not handled ...\n", 0 );
        break;
      case VT_BINAIRE :
          VT_InitFromImage( &imres, &imtmp, par.output_name, UCHAR );
          if ( VT_AllocImage( &imres ) != 1 ) {
            VT_FreeImage( &imtmp );
            API_ErrorParse_connexe( _BaseName( argv[0] ), "allocation error of output image ...\n", 0 );
          }
          if ( VT_Threshold( &imtmp, &imres, (float)1.0 ) != 1 ) {
            VT_FreeImage( &imres );
            VT_FreeImage( &imtmp );
            API_ErrorParse_connexe( _BaseName( argv[0] ), "thresholding error of output image ...\n", 0 );
          }
          VT_FreeImage( &imtmp );
          imout = &imres;
        break;
      case VT_GREY :
      case VT_SIZE :
        if ( VT_3m( &imtmp, (vt_image*)NULL, &m ) == -1 ) {
            VT_FreeImage( &imtmp );
            API_ErrorParse_connexe( _BaseName( argv[0] ), "unable to compute minimum, mean and maximum ...\n", 0 );
         }
        if ( (int)(m.max + 0.5) <= 255 ) {
          par.output_type = UCHAR;
        }
        else if ( (int)(m.max + 0.5) <= 32767 ) {
          par.output_type = SSHORT;
        }
        else {
          par.output_type = USHORT;
        }
        if ( par.output_type != imtmp.type ) {
            VT_InitFromImage( &imres, &imtmp, par.output_name, par.output_type );
            if ( VT_AllocImage( &imres ) != 1 ) {
              VT_FreeImage( &imtmp );
              API_ErrorParse_connexe( _BaseName( argv[0] ), "allocation error of output image ...\n", 0 );
            }
            if ( VT_CopyImage( &imtmp, &imres ) != 1 ) {
              VT_FreeImage( &imres );
              VT_FreeImage( &imtmp );
              API_ErrorParse_connexe( _BaseName( argv[0] ), "copy error of output image ...\n", 0 );
            }
            VT_FreeImage( &imtmp );
            imout = &imres;
        }
        break;
      }
  }

  /* output image writing
   */

  if ( VT_WriteInrimage( imout ) == -1 ) {
    VT_FreeImage( imout );
    API_ErrorParse_connexe( _BaseName( argv[0] ), "unable to write output image ...\n", 0 );
  }
  
  /* memory freeing
   */
  VT_FreeImage( imout );


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
