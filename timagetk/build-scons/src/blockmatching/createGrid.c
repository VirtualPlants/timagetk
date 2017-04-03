/*************************************************************************
 * createGrid.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
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
#include <unistd.h>

#include <bal-image-tools.h>
#include <bal-createGrid.h>





typedef struct {
  char *resim_name;
  char *template_image_name;
  
  bal_integerPoint dim;
  bal_doublePoint voxel;

  bal_integerPoint offset;
  bal_integerPoint spacing;


} local_parameter;



static char *program = NULL;

static char *usage = "%s [-template %s]\n\
 [-dim %d %d [%d]] [-voxel %lf %lf [%lf]]\n\
 [-offset %d %d [%d]] [-spacing %d %d [%d]]";

static char *detail = "\n\
-template %s         # template image for geometry\n\
-dim %d %d [%d]      # template image dimensions\n\
-voxel %lf %lf [%lf] # voxel sizes of the template image\n\
-offset %d %d [%d]   # offset for lines/planes of the grid\n\
\t value #1          # vertical lines or YZ planes\n\
\t value #2          # horizontal lines or XZ planes\n\
\t value #3          # XY planes\n\
\t specifying a negative value prevents drawing\n\
-spacing %d %d [%d]  # spacing for lines/planes of the grid\n\
\t specifying a null or negative value prevents drawing\n\
";





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
  
  if (createGrid(
		  p.resim_name,
		  p.template_image_name,
		  p.dim,
		  p.voxel,
		  p.offset,
		  p.spacing
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

  program = argv[0];

  for ( i=1; i<argc; i++ ) {

    if ( argv[i][0] == '-' ) {

      if ( strcmp ( argv[i], "-template") == 0
	   || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
	   ||  strcmp ( argv[i], "-reference") == 0
	   || (strcmp ( argv[i], "-ref") == 0 && argv[i][4] == '\0')
	   || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
	i++;
	if ( i >= argc) _ErrorParse( "parsing -template", 0 );
	p->template_image_name = argv[i];
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
      else if ( strcmp (argv[i], "-voxel" ) == 0 ) {
	i ++;
	if ( i >= argc)    _ErrorParse( "parsing -voxel %lf", 0 );
	status = sscanf( argv[i], "%lf", &(p->voxel.x) );
	if ( status <= 0 ) _ErrorParse( "parsing -voxel %lf", 0 );
	i ++;
	if ( i >= argc)    _ErrorParse( "parsing -voxel %lf %lf", 0 );
	status = sscanf( argv[i], "%lf", &(p->voxel.y) );
	if ( status <= 0 ) _ErrorParse( "parsing -voxel %lf %lf", 0 );
	i ++;
	if ( i >= argc) p->voxel.z = 1;
	else {
	  status = sscanf( argv[i], "%lf", &(p->voxel.z) );
	  if ( status <= 0 ) {
	    i--;
	    p->voxel.z = 1;
	  }
	}
      }
      
      else if ( strcmp (argv[i], "-offset" ) == 0 ) {
	i ++;
	if ( i >= argc)    _ErrorParse( "parsing -offset %d", 0 );
	status = sscanf( argv[i], "%d", &(p->offset.x) );
	if ( status <= 0 ) _ErrorParse( "parsing -offset %d", 0 );
	i ++;
	if ( i >= argc)    _ErrorParse( "parsing -offset %d %d", 0 );
	status = sscanf( argv[i], "%d", &(p->offset.y) );
	if ( status <= 0 ) _ErrorParse( "parsing -offset %d %d", 0 );
	i ++;
	if ( i < argc) {
	  status = sscanf( argv[i], "%d", &(p->offset.z) );
	  if ( status <= 0 ) i--;
	}
      }
      else if ( strcmp (argv[i], "-spacing" ) == 0 ) {
	i ++;
	if ( i >= argc)    _ErrorParse( "parsing -spacing %d", 0 );
	status = sscanf( argv[i], "%d", &(p->spacing.x) );
	if ( status <= 0 ) _ErrorParse( "parsing -spacing %d", 0 );
	i ++;
	if ( i >= argc)    _ErrorParse( "parsing -spacing %d %d", 0 );
	status = sscanf( argv[i], "%d", &(p->spacing.y) );
	if ( status <= 0 ) _ErrorParse( "parsing -spacing %d %d", 0 );
	i ++;
	if ( i < argc) {
	  status = sscanf( argv[i], "%d", &(p->spacing.z) );
	  if ( status <= 0 ) i--;
	}
      }
      
      else if ( strcmp ( argv[i], "--help" ) == 0 
		|| ( strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0' )
		|| ( strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0' )
		|| ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) ) {
	_ErrorParse( NULL, 1 );
      }
      
      else {
	fprintf(stderr,"unknown option: '%s'\n",argv[i]);
      }

    }
    else {
      p->resim_name = argv[i];
    }
  }
}



static void _InitParam( local_parameter *p )
{
  p->resim_name = NULL;
  p->template_image_name = NULL;
  
  p->dim.x = 256;
  p->dim.y = 256;
  p->dim.z = 1;

  p->voxel.x = 1.0;
  p->voxel.y = 1.0;
  p->voxel.z = 1.0;
  
  p->offset.x = 0;
  p->offset.y = 0;
  p->offset.z = 0;

  p->spacing.x = 10;
  p->spacing.y = 10;
  p->spacing.z = 10;
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
