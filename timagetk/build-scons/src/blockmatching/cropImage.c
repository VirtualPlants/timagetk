/*************************************************************************
 * cropImage.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Thu Nov 22 18:04:47 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <extract.h>

#include <bal-image.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>

#include <bal-cropImage.h>


typedef struct {
  char *theim_name;
  char *resim_name;

  char *real_transformation_name;
  char *voxel_transformation_name;

  char *template_image_name; /* template */  

  int analyzeFiji;

  bal_integerPoint origin;
  
  bal_integerPoint dim;
  
  bal_integerPoint slice;

} local_parameter;





static char *program = NULL;

static char *usage = "%s %s \n\
 [-origin %d %d [%d] | [-ix %d] [-iy %d] [-iz %d]]\n\
 [-dim %d %d [%d] | [-x %d] [-y %d] [-z %d]] [-template %s]\n\
 [-xy %d | -yz %d | -xz %d]\n\
 [-analyze-fiji]\n\
 [-result-transformation |-res-trsf %s]\n\
 [-result-voxel-transformation |-res-voxel-trsf %s]";

static char *detail = "\
-origin %d %d [%d]   # origin of the output image in the input image\n\
                       default is (0,0,0)\n\
-dim %d %d [%d]      # output image dimensions\n\
-template %s         # template image for the dimensions\n\
                       of the output image\n\
-xy %d   # extract the XY slice #%d\n\
         # this is equivalent to '-origin 0 0 %d -dim dimx dimy 1'\n\
-xz %d   # extract the XZ slice #%d\n\
         # the output image has sizes (dimx, dimz, 1)\n\
-yz %d   # extract the YZ slice #%d\n\
         # the output image has sizes (dimy, dimz, 1)\n\
-result-transformation %s # resampling transformation for 'applyTrsf'\n\
         'applyTrsf %s %s -trsf this-transformation -dim %d %d %d'\n\
         yields then the same result\n\
-result-voxel-transformation %s # idem with a transformation encoded in voxels\n";


static int _verbose_ = 1;
static int _debug_ = 0;


static void _ErrorParse( char *str, int flag );
static void _Parse( int argc, char *argv[], local_parameter *p );
static void _InitParam( local_parameter *p );
static char *_BaseName( char *p );




int main(int argc, char *argv[])
{
  local_parameter p;

  /***************************************************
   *
   * parsing parameters
   *
   ***************************************************/
  program = argv[0];
  
  
  /* no arguments
   */
  if ( argc == 1 ) _ErrorParse( NULL, 0 );

  /* parsing parameters 
   */
  _InitParam( &p );
  _Parse( argc, argv, &p );

  if ( p.theim_name == NULL ) {
    _ErrorParse( "no input image name", 0 );
    exit( -1 );
  }

  if ( p.resim_name == NULL ) {
    _ErrorParse( "no result image name", 0 );
    exit( -1 );
  }

  if (cropImage(
                 p.theim_name,
                 p.resim_name,
                 p.real_transformation_name,
                 p.voxel_transformation_name,
                 p.template_image_name,
                 p.analyzeFiji,
                 p.origin,
                 p.dim,
                 p.slice,
                 _debug_,
                 _verbose_
                 ))
    {
      fprintf( stderr, "%s: Failure.\n",program);
    }

  exit( 0 );
}




/***************************************************
 *
 * 
 *
 ***************************************************/





static void _ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage: %s %s\n",_BaseName(program), usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s\n",detail);
  if ( str != NULL ) (void)fprintf(stderr,"Error: %s\n",str);
  exit( -1 );
}



static void _Parse( int argc, char *argv[], local_parameter *p )
{
  int i;
  int status;  
  float t;

  program = argv[0];

  for ( i=1; i<argc; i++ ) {

    if ( argv[i][0] == '-' ) {

      if ( strcmp ( argv[i], "-template") == 0
           || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
           || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
        i++;
        if ( i >= argc) _ErrorParse( "parsing -template", 0 );
        p->template_image_name = argv[i];
      }
      
      else if ( strcmp ( argv[i], "-result-transformation") == 0
                || (strcmp ( argv[i], "-res-trsf") == 0 && argv[i][9] == '\0') ) {
        i++;
        if ( i >= argc) _ErrorParse( "parsing -result-transformation", 0 );
        p->real_transformation_name = argv[i];
      }
      
      else if ( strcmp ( argv[i], "-result-voxel-transformation") == 0
                || strcmp ( argv[i], "-res-voxel-trsf") == 0 ) {
        i++;
        if ( i >= argc) _ErrorParse( "parsing -result-voxel-transformation", 0 );
        p->voxel_transformation_name = argv[i];
      }
      
      else if ( (strcmp (argv[i], "-fiji" ) == 0 && argv[i][5] == '\0')
                || (strcmp (argv[i], "-analyze-fiji" ) == 0) ) {
        p->analyzeFiji = 1;
      }

      else if ( (strcmp (argv[i], "-origin" ) == 0 && argv[i][7] == '\0')
                || (strcmp (argv[i], "-o" ) == 0 && argv[i][2] == '\0') ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -origin %d", 0 );
        status = sscanf( argv[i], "%f", &t );
        if ( status <= 0 ) _ErrorParse( "parsing -origin %d", 0 );
        p->origin.x = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -origin %d %d", 0 );
        status = sscanf( argv[i], "%f", &t );
        if ( status <= 0 ) _ErrorParse( "parsing -origin %d %d", 0 );
        p->origin.y = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
        i ++;
        if ( i >= argc) p->origin.z = 0;
        else {
          status = sscanf( argv[i], "%f", &t );
          if ( status <= 0 ) {
            i--;
            p->origin.z = 0;
          }
          else {
            p->origin.z = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
          }
        }
      }
      
      else if ( strcmp (argv[i], "-ix" ) == 0 && argv[i][3] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -ix %d", 0 );
        status = sscanf( argv[i], "%f", &t );
        if ( status <= 0 ) _ErrorParse( "parsing -ix %d", 0 );
        p->origin.x = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }
      else if ( strcmp (argv[i], "-iy" ) == 0 && argv[i][3] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -iy %d", 0 );
        status = sscanf( argv[i], "%f", &t );
        if ( status <= 0 ) _ErrorParse( "parsing -iy %d", 0 );
        p->origin.y = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }
      else if ( strcmp (argv[i], "-iz" ) == 0 && argv[i][3] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -iz %d", 0 );
        status = sscanf( argv[i], "%f", &t );
        if ( status <= 0 ) _ErrorParse( "parsing -iz %d", 0 );
        p->origin.z = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }

      else if ( strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -dim %d", 0 );
        status = sscanf( argv[i], "%d", &(p->dim.x) );
        if ( status <= 0 ) _ErrorParse( "parsing -dim %d", 0 );
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -dim %d %d", 0 );
        status = sscanf( argv[i], "%d", &(p->dim.y) );
        if ( status <= 0 ) _ErrorParse( "parsing -dim %d %d", 0 );
        i ++;
        if ( i >= argc) p->dim.z = 1;
        else {
          status = sscanf( argv[i], "%d", &(p->dim.z) );
          if ( status <= 0 ) {
            i--;
            p->dim.z = 1;
          }
        }
      }

      else if ( strcmp (argv[i], "-x" ) == 0 && argv[i][2] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -x %d", 0 );
        status = sscanf( argv[i], "%d", &(p->dim.x) );
        if ( status <= 0 ) _ErrorParse( "parsing -x %d", 0 );
      }
      else if ( strcmp (argv[i], "-y" ) == 0 && argv[i][2] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -y %d", 0 );
        status = sscanf( argv[i], "%d", &(p->dim.y) );
        if ( status <= 0 ) _ErrorParse( "parsing -y %d", 0 );
      }
      else if ( strcmp (argv[i], "-z" ) == 0 && argv[i][2] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -z %d", 0 );
        status = sscanf( argv[i], "%d", &(p->dim.z) );
        if ( status <= 0 ) _ErrorParse( "parsing -z %d", 0 );
      }


      else if ( strcmp (argv[i], "-xy" ) == 0 && argv[i][3] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -xy %d", 0 );
        status = sscanf( argv[i], "%d", &(p->slice.z) );
        if ( status <= 0 ) _ErrorParse( "parsing -xy %d", 0 );
      }
      else if ( strcmp (argv[i], "-yz" ) == 0 && argv[i][3] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -yz %d", 0 );
        status = sscanf( argv[i], "%d", &(p->slice.x) );
        if ( status <= 0 ) _ErrorParse( "parsing -yz %d", 0 );
      }
      else if ( strcmp (argv[i], "-xz" ) == 0 && argv[i][3] == '\0' ) {
        i ++;
        if ( i >= argc)    _ErrorParse( "parsing -xz %d", 0 );
        status = sscanf( argv[i], "%d", &(p->slice.y) );
        if ( status <= 0 ) _ErrorParse( "parsing -xz %d", 0 );
      }

      else if ( strcmp ( argv[i], "--help" ) == 0 
                || ( strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0' )
                || ( strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0' )
                || ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) ) {
        _ErrorParse( NULL, 1 );
      }

      else if ( ( strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0' )
                || ( strcmp ( argv[i], "-verbose" ) == 0) ) {
        if ( _verbose_ <= 0 )  _verbose_ = 1;
        else _verbose_ ++;
      }
      else if ( ( strcmp ( argv[i], "-d" ) == 0 && argv[i][2] == '\0' )
                || ( strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0' )) {
        if ( _debug_ <= 0 )  _debug_ = 1;
        else _debug_ ++;
      }

      else {
        fprintf(stderr,"unknown option: '%s'\n",argv[i]);
      }

    }
    /* ( argv[i][0] == '-' )
     */
    else {
      if ( p->theim_name == NULL ) {
        p->theim_name = argv[i];
      }
      else if ( p->resim_name == NULL ) {
        p->resim_name = argv[i];
      }
      else {
        fprintf(stderr,"too many file names: '%s'\n",argv[i]);
      }
    }

  }
  
}



static void _InitParam( local_parameter *p )
{
  p->theim_name = NULL;
  p->resim_name = NULL;

  p->real_transformation_name = NULL;
  p->voxel_transformation_name = NULL;

  p->template_image_name = NULL;
  
  p->analyzeFiji = 0;

  p->origin.x = 0;
  p->origin.y = 0;
  p->origin.z = 0;

  p->dim.x = -1;
  p->dim.y = -1;
  p->dim.z = -1;

  p->slice.x = -1;
  p->slice.y = -1;
  p->slice.z = -1;

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
