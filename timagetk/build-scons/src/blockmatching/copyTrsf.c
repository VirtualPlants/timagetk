/*************************************************************************
 * copyTrsf.c -
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

#include <bal-stddef.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>
#include <bal-copyTrsf.h>




typedef struct {

  char *thetrsf_name;
  char *restrsf_name;

  enumUnitTransfo thetrsf_unit;
  enumUnitTransfo restrsf_unit;

  char *template_image_name;
  char *floating_image_name;

  bal_integerPoint dim;
  bal_doublePoint voxel;

  enumTypeTransfo transformation_type;

} local_parameter;





static char *program = NULL;

static char *usage = "%s %s \n\
 [-transformation-type|-transformation|-trsf-type %s]\n\
 [-template %s] [-dim %d %d [%d]] [-voxel %lf %lf [%lf]]\n\
 [-input-unit | -iu %s] [-output-unit | -ou %s]\n\
 [-floating %s]\n\
 [-h|-help|--h|--help]\n";

static char *detail = "\
%s # input transformation \n\
%s # output transformation \n\
[-transformation-type|-transformation|-trsf-type %s] # output transformation type\n\
  translation2D, translation3D, translation-scaling2D, translation-scaling3D,\n\
  rigid2D, rigid3D, rigid, similitude2D, similitude3D, similitude,\n\
  affine2D, affine3D, affine, vectorfield2D, vectorfield3D, vectorfield, vector\n\
-template %s         # template image for the geometry\n\
                     # of the vector field (if required)\n\
-dim %d %d [%d]      # template image dimensions\n\
-voxel %lf %lf [%lf] # voxel sizes of template image\n\
  When copying/converting a matrix into a vectorfield, the vectorial image\n\
  defining the vectorfield will be created with the geometry of the template\n\
  image.\n\
-input-unit  [voxel|real] #\n\
-output-unit [voxel|real] #\n\
-floating %s         # template image for conversion between real and voxel units\n\
  The conversion from real to voxel units is done by calculating\n\
  H^{-1}_floating o T_input o H_template\n\
  while the conversion from voxel to real units is done by calculating\n\
  H_floating o T_input o H^{-1}_template\n\
  H_{image} being the diagonal matrix of the image voxel sizes\n\
[-h|-help|--h|--help]\n";



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

  if (copyTrsf(
		p.thetrsf_name,
		p.restrsf_name,
		p.thetrsf_unit,
		p.restrsf_unit,
		p.template_image_name,
		p.floating_image_name,
		p.dim,
		p.voxel,
		p.transformation_type,
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
      
      else if ( strcmp ( argv[i], "-floating") == 0
		|| (strcmp ( argv[i], "-flo") == 0 && argv[i][4] == '\0') ) {
	i++;
	if ( i >= argc) _ErrorParse( "parsing -floating", 0 );
	p->floating_image_name = argv[i];
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
      
      else if ( strcmp ( argv[i], "-input-unit" ) == 0 
		|| ( strcmp ( argv[i], "-iu" ) == 0 && argv[i][3] == '\0' ) ) {
	i ++;
	if ( i >= argc)    _ErrorParse( "-input-unit", 0 );
	if ( strcmp( argv[i], "real" ) == 0 ) {
	  p->thetrsf_unit = REAL_UNIT;
	} 
	else if ( strcmp( argv[i], "voxel" ) == 0 ) {
	  p->thetrsf_unit = VOXEL_UNIT;
	}
	else {
	  fprintf( stderr, "unknown unit: '%s'\n", argv[i] );
	  _ErrorParse( "-input-unit", 0 );
	}
      }
      
      else if ( strcmp ( argv[i], "-output-unit" ) == 0 
		|| ( strcmp ( argv[i], "-ou" ) == 0 && argv[i][3] == '\0' ) ) {
	i ++;
	if ( i >= argc)    _ErrorParse( "-output-unit", 0 );
	if ( strcmp( argv[i], "real" ) == 0 ) {
	  p->restrsf_unit = REAL_UNIT;
	} 
	else if ( strcmp( argv[i], "voxel" ) == 0 ) {
	  p->restrsf_unit = VOXEL_UNIT;
	}
	else {
	  fprintf( stderr, "unknown unit: '%s'\n", argv[i] );
	  _ErrorParse( "-output-unit", 0 );
	}
      }

      else if ( strcmp ( argv[i], "-transformation-type" ) == 0 
		|| strcmp ( argv[i], "-transformation" ) == 0
		|| strcmp ( argv[i], "-trsf-type" ) == 0 ) {
	i ++;
	if ( i >= argc)    _ErrorParse( "-transformation-type", 0 );
	if ( strcmp ( argv[i], "translation2D" ) == 0 ) {
	  p->transformation_type = TRANSLATION_2D;
	}
	else if ( strcmp ( argv[i], "translation3D" ) == 0 ) {
	  p->transformation_type = TRANSLATION_3D;
	}
	else if ( strcmp ( argv[i], "translation" ) == 0 && argv[i][11] == '\0') {
	  p->transformation_type = TRANSLATION_3D;
	}
	else if ( strcmp ( argv[i], "translation-scaling2D" ) == 0 ) {
	  p->transformation_type = TRANSLATION_SCALING_2D;
	}
	else if ( strcmp ( argv[i], "translation-scaling3D" ) == 0 ) {
	  p->transformation_type = TRANSLATION_SCALING_3D;
	}
	else if ( strcmp ( argv[i], "rigid2D" ) == 0 ) {
	  p->transformation_type = RIGID_2D;
	}
	else if ( strcmp ( argv[i], "rigid3D" ) == 0 ) {
	  p->transformation_type = RIGID_3D;
	}
	else if ( (strcmp ( argv[i], "rigid" ) == 0 && argv[i][5] == '\0') ) {
	  p->transformation_type = RIGID_3D;
	}
	else if ( strcmp ( argv[i], "similitude2D" ) == 0 ) {
	  p->transformation_type = SIMILITUDE_2D;
	}
	else if ( strcmp ( argv[i], "similitude3D" ) == 0 ) {
	  p->transformation_type = SIMILITUDE_3D;
	}
	else if ( strcmp ( argv[i], "similitude" ) == 0 ) {
	  p->transformation_type = SIMILITUDE_3D;
	}
	else if ( strcmp ( argv[i], "affine2D" ) == 0 ) {
	  p->transformation_type = AFFINE_2D;
	}
	else if ( strcmp ( argv[i], "affine3D" ) == 0 ) {
	  p->transformation_type = AFFINE_3D;
	}
	else if ( strcmp ( argv[i], "affine" ) == 0 ) {
	  p->transformation_type = AFFINE_3D;
	}
	/*
	  else if ( strcmp ( argv[i], "spline" ) == 0 ) {
	  p->transformation_type = SPLINE;
	  }
	*/
	else if ( strcmp ( argv[i], "vectorfield" ) == 0 
		  || (strcmp ( argv[i], "vector" ) == 0 && argv[i][6] == '\0') ) {
	  p->transformation_type = VECTORFIELD_3D;
	}
	else if ( strcmp ( argv[i], "vectorfield3D" ) == 0 
		  || strcmp ( argv[i], "vector3D" ) == 0 ) {
	  p->transformation_type = VECTORFIELD_3D;
	}
	else if ( strcmp ( argv[i], "vectorfield2D" ) == 0 
		  || strcmp ( argv[i], "vector2D" ) == 0 ) {
	  p->transformation_type = VECTORFIELD_2D;
	}
	else {
	  fprintf( stderr, "unknown transformation type: '%s'\n", argv[i] );
	  _ErrorParse( "-transformation-type", 0 );
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
    /* ( argv[i][0] == '-' )
     */
    else {
      if ( p->thetrsf_name == NULL ) {
	p->thetrsf_name = argv[i];
      }
      else if ( p->restrsf_name == NULL ) {
	p->restrsf_name = argv[i];
      }
      else {
	fprintf(stderr,"too many file names: '%s'\n",argv[i]);
      }
    }
    
  }
  
}



static void _InitParam( local_parameter *p )
{
  p->thetrsf_name = NULL;
  p->restrsf_name = NULL;

  p->thetrsf_unit = REAL_UNIT;
  p->restrsf_unit = REAL_UNIT;

  p->template_image_name = NULL;
  p->floating_image_name = NULL;
  
  p->dim.x = 256;
  p->dim.y = 256;
  p->dim.z = 256;

  p->voxel.x = 1.0;
  p->voxel.y = 1.0;
  p->voxel.z = 1.0;
  
  p->transformation_type = AFFINE_3D;
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

