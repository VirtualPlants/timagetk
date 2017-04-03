/****************************************************
 * drawShape.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Fri Feb  8 15:46:10 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#include <vt_common.h>

#include <drawShapes.h>



typedef enum {
  CYLINDER,
  LINE,
  RECTANGLE,
  SPHERE
} enumShape;



typedef struct local_par {
  vt_names names;
  /* drawing parameter
   */
  enumShape shape;
  double radius;
  double vec[3];
  double size[3];
  double pt1[3];
  double pt2[3];
  double value;
  /* image creation
   */
  vt_4vpt dim;
  vt_fpt voxel;
  ImageType type;
} local_par;



/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out]\n\
 [-shape cylinder|line|rectangle|sphere]\n\
 [-origin %f %f %f] [-end %f %f %f] [-size|-s %f %f %f]\n\
 [-radius|-r %f] [-dir|-vector|-d %f %f %f]\n\
 [-dim %d %d [%d] | [-x %d] [-y %d] [-z %d]] [-v %d] [-template %s]\n\
 [-voxel | -pixel | -vs %f %f [%f] | [-vx %f] [-vy %f] [-vz %f] ]\n\
 [-value %lf]\n\
 [-o|-b|-bytes %d [-r|-f] [-s]]\n\
 [-verbose] [-D] [-help]";

static char *detail = "\
\n\
   Draw a shape\n\
\n\
[image-in]  # if not existing, it is created\n\
[image-out] # if not present, 'image-in' is used\n\
 # image creation parameters\n\
[-dim %d %d [%d]]      # output image dimensions\n\
[-x %d]                # X dimension of the ouput image\n\
[-y %d]                # Y dimension of the ouput image\n\
[-z %d]                # Z dimension of the ouput image\n\
[-template %s]         # template image for the dimensions\n\
                         of the output image\n\
[-voxel %f %f [%f]]    # output image voxel sizes\n\
[-bytes %d]            # number of bytes for encoding\n\
[-r]                   # real\n\
[-f]                   # fixed (ie integer)\n\
[-s]                   # signed\n\
 e.g. '-b 1'    = unsigned char\n\
      '-b 2 -s' = signed short int\n\
      '-b 4 -r' = float\n";



static char program[STRINGLENGTH];

int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imres, *theIm;
  vt_image *imtemplate;
  int theDim[3];
  int thePt1[3], thePt2[3];

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );





  /* creation de l'image
   */

  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) {
    
    /* initializing result image
       - with reference image, if any
       - with parameters, if any
    */
    
    VT_Image( &imres );
    
    if ( par.names.ext[0] != '\0' ) {
      imtemplate = _VT_Inrimage( par.names.ext );
      if ( imtemplate == (vt_image*)NULL ) {
	VT_ErrorParse("unable to read template image\n", 0);
      }
      
      if ( _VT_VERBOSE_ )
	fprintf( stderr, "... initializing with template image '%s'\n", par.names.ext );

      VT_InitVImage( &imres, par.names.out, imtemplate->dim.v, 
		     imtemplate->dim.x, imtemplate->dim.y, imtemplate->dim.z, 
		     imtemplate->type );
      imres.siz.x = imtemplate->siz.x;
      imres.siz.y = imtemplate->siz.y;
      imres.siz.z = imtemplate->siz.z;
      
      VT_FreeImage( imtemplate );
      VT_Free( (void**)&imtemplate );
      
      if ( par.dim.v > 0 ) imres.dim.v = par.dim.v;
      if ( par.dim.x > 0 ) imres.dim.x = par.dim.x;
      if ( par.dim.y > 0 ) imres.dim.y = par.dim.y;
      if ( par.dim.z > 0 ) imres.dim.z = par.dim.z;
      
      if ( par.voxel.x > 0.0 ) imres.siz.x = par.voxel.x;
      if ( par.voxel.y > 0.0 ) imres.siz.y = par.voxel.y;
      if ( par.voxel.z > 0.0 ) imres.siz.z = par.voxel.z;
      
      if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
    }
    
    else {
      
      if ( _VT_VERBOSE_ )
	fprintf( stderr, "... initializing dimensions\n" );

      if ( par.dim.v < 0 ) par.dim.v = 1;
      if ( par.dim.x < 0 ) par.dim.x = 256;
      if ( par.dim.y < 0 ) par.dim.y = 256;
      if ( par.dim.z < 0 ) par.dim.z = 1;
      
      if ( par.voxel.x < 0.0 ) par.voxel.x = 1.0;
      if ( par.voxel.y < 0.0 ) par.voxel.y = 1.0;
      if ( par.voxel.z < 0.0 ) par.voxel.z = 1.0;
      
      if ( par.type == TYPE_UNKNOWN ) par.type = UCHAR;
      
      VT_InitVImage( &imres, par.names.out, par.dim.v, 
		     par.dim.x, par.dim.y, par.dim.z, 
		     par.type );
      
      imres.siz.x = par.voxel.x;
      imres.siz.y = par.voxel.y;
      imres.siz.z = par.voxel.z;
    }

    if ( par.names.in[0] != '\0' && par.names.out[0] == '\0' ) {
      if ( 0 )
	fprintf( stderr, "%s: set '%s' as output image name\n", program, par.names.in );
      if ( VT_CopyName( imres.name, par.names.in ) == 0 ) {
	VT_ErrorParse("unable to copy image name\n", 0);
      }
    }
    
    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_ErrorParse("unable to allocate output image\n", 0);
    }
	 
    /* VT_AllocImage met l'image a zero
       VT_FillImage( &imres, 0.0 );
    */

    theIm = &imres;

  }
  else {
    theIm = image;
    if ( par.names.out[0] != '\0' ) {
      if ( 0 )
	fprintf( stderr, "%s: set '%s' as output image name\n", program, par.names.out );
      if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
	VT_ErrorParse("unable to copy image name\n", 0);
      }
    }
  }

  
  /* ... */

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  switch( par.shape ) {

  default :
    VT_FreeImage( theIm );
    if ( image != (vt_image*)NULL ) 
      VT_Free( (void**)&image );
    VT_ErrorParse( "such shape not handled yet\n", 0 );

  case CYLINDER :
    if ( drawCylinder( theIm->buf, theIm->buf, theDim, theIm->type,
		       par.pt1, par.vec, par.radius, par.value ) != 1 ) {
      VT_FreeImage( theIm );
      if ( image != (vt_image*)NULL ) 
	VT_Free( (void**)&image );
      VT_ErrorParse( "error when drawing cylinder\n", 0 );
    }
    break;

  case LINE :
    thePt1[0] = (par.pt1[0] > 0) ? (int)(par.pt1[0]+0.5) : (int)(par.pt1[0]-0.5);
    thePt1[1] = (par.pt1[1] > 0) ? (int)(par.pt1[1]+0.5) : (int)(par.pt1[1]-0.5);
    thePt1[2] = (par.pt1[2] > 0) ? (int)(par.pt1[2]+0.5) : (int)(par.pt1[2]-0.5);
    if ( par.pt2[0] < 0.0 || par.pt2[1] < 0.0 ) {
      par.pt2[0] = par.pt1[0] + par.size[0];
      par.pt2[1] = par.pt1[1] + par.size[1];
      par.pt2[2] = par.pt1[2] + par.size[2];
    }
    thePt2[0] = (par.pt2[0] > 0) ? (int)(par.pt2[0]+0.5) : (int)(par.pt2[0]-0.5);
    thePt2[1] = (par.pt2[1] > 0) ? (int)(par.pt2[1]+0.5) : (int)(par.pt2[1]-0.5);
    thePt2[2] = (par.pt2[2] > 0) ? (int)(par.pt2[2]+0.5) : (int)(par.pt2[2]-0.5);
    if ( drawLine( theIm->buf, theIm->buf, theDim, theIm->type,
		   thePt1, thePt2, par.value ) != 1 ) {
      VT_FreeImage( theIm );
      if ( image != (vt_image*)NULL ) 
	VT_Free( (void**)&image );
      VT_ErrorParse( "error when drawing line\n", 0 );
    }
    break;

  case RECTANGLE :
    thePt1[0] = (par.pt1[0] > 0) ? (int)(par.pt1[0]+0.5) : (int)(par.pt1[0]-0.5);
    thePt1[1] = (par.pt1[1] > 0) ? (int)(par.pt1[1]+0.5) : (int)(par.pt1[1]-0.5);
    thePt1[2] = (par.pt1[2] > 0) ? (int)(par.pt1[2]+0.5) : (int)(par.pt1[2]-0.5);
    if ( par.pt2[0] < 0.0 || par.pt2[1] < 0.0 ) {
      par.pt2[0] = par.pt1[0] + par.size[0];
      par.pt2[1] = par.pt1[1] + par.size[1];
      par.pt2[2] = par.pt1[2] + par.size[2];
    }
    thePt2[0] = (par.pt2[0] > 0) ? (int)(par.pt2[0]+0.5) : (int)(par.pt2[0]-0.5);
    thePt2[1] = (par.pt2[1] > 0) ? (int)(par.pt2[1]+0.5) : (int)(par.pt2[1]-0.5);
    thePt2[2] = (par.pt2[2] > 0) ? (int)(par.pt2[2]+0.5) : (int)(par.pt2[2]-0.5);

    if ( _VT_VERBOSE_ )
      fprintf( stderr, "... draw rectangle from [%d %d %d] to [%d %d %d]\n",
	       thePt1[0], thePt1[1], thePt1[2], thePt2[0], thePt2[1], thePt2[2] );

    if ( drawRectangle( theIm->buf, theIm->buf, theDim, theIm->type,
		   thePt1, thePt2, par.value ) != 1 ) {
      VT_FreeImage( theIm );
      if ( image != (vt_image*)NULL ) 
	VT_Free( (void**)&image );
      VT_ErrorParse( "error when drawing rectangle\n", 0 );
    }
    break;

  case SPHERE :
    if ( drawSphere( theIm->buf, theIm->buf, theDim, theIm->type,
		     par.pt1, par.radius, par.value ) != 1 ) {
      VT_FreeImage( theIm );
      if ( image != (vt_image*)NULL ) 
	VT_Free( (void**)&image );
      VT_ErrorParse( "error when drawing sphere\n", 0 );
    }
    break;

  }


  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( theIm ) == -1 ) {
    VT_FreeImage( theIm );
    if ( image != (vt_image*)NULL ) 
      VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( theIm );
  if ( image != (vt_image*)NULL ) 
    VT_Free( (void**)&image );
  
  return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  int f=0, o=0, s=0, r=0;
  char text[STRINGLENGTH];
  
  if ( VT_CopyName( program, argv[0] ) != 1 )
    VT_Error("Error while copying program name", (char*)NULL);
  if ( argc == 1 ) VT_ErrorParse("\n", 0 );
  
  /*--- lecture des parametres ---*/
  i = 1; nb = 0;
  while ( i < argc ) {
    if ( argv[i][0] == '-' ) {
      if ( argv[i][1] == '\0' ) {
	VT_ErrorParse("parsing - \n", 1);
      }
      
      
      /* some general options
       */

      else if ( strcmp ( argv[i], "--help" ) == 0 
		|| ( strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0' )
		|| ( strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0' )
		|| ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-verbose" ) == 0
		|| (strcmp ( argv[i], "-w" ) == 0 && argv[i][2] == '\0') ) {
	if ( _VT_VERBOSE_ <= 0 )
	  _VT_VERBOSE_ = 1;
	else 
	  _VT_VERBOSE_ ++;
      }
      else if ( strcmp ( argv[i], "-debug" ) == 0
		|| (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0') ) {
	if ( _VT_DEBUG_  <= 0 ) 
	  _VT_DEBUG_ = 1;
	else 
	  _VT_DEBUG_ ++;
      }

      /*--- ---*/
      else if ( strcmp ( argv[i], "-shape" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -shape...\n", 0 );
	if ( strcmp ( argv[i], "cylinder" ) == 0 ) {
	  par->shape = CYLINDER;
	}
	else if ( strcmp ( argv[i], "line" ) == 0 ) {
	  par->shape = LINE;
	}
	else if ( strcmp ( argv[i], "rectangle" ) == 0 ) {
	  par->shape = RECTANGLE;
	}
	else if ( strcmp ( argv[i], "sphere" ) == 0 ) {
	  par->shape = SPHERE;
	}
	else if ( strcmp ( argv[i], "rectangle" ) == 0 ) {
	  par->shape = RECTANGLE;
	}
      }
      
      else if ( strcmp ( argv[i], "-radius" ) == 0 ||
		(strcmp ( argv[i], "-r" ) == 0 && argv[i][2] != '\0') ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -radius...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->radius) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -radius...\n", 0 );
      }


      else if ( strcmp ( argv[i], "-vector" ) == 0 ||
		strcmp ( argv[i], "-dir" ) == 0  ||
		(strcmp ( argv[i], "-d" ) == 0 && argv[i][2] != '\0') ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -dir...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->vec[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -dir...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -dir...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->vec[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -dir...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -dir...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->vec[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -dir...\n", 0 );
      }
      
      else if ( strcmp ( argv[i], "-size" ) == 0  ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -size...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->size[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -size...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -size...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->size[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -size...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -size...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->size[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -size...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-origin" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -origin...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->pt1[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -origin...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -origin...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->pt1[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -origin...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -origin...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->pt1[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -origin...\n", 0 );
      }
      
      else if ( strcmp ( argv[i], "-end" ) == 0 ||
		(strcmp ( argv[i], "-e" ) == 0 && argv[i][2] != '\0') ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -end...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->pt2[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -end...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -end...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->pt2[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -end...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -end...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->pt2[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -end...\n", 0 );
      }

      /* image value
       */
      else if ( strcmp ( argv[i], "-value" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -value...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->value) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -value...\n", 0 );
      }



      /*--- dimension de l'image ---*/
      else if ( strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0' ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -dim %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->dim.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -dim %d\n", 0 );
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -dim %d %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->dim.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -dim %d %d\n", 0 );
	i ++;
	if ( i < argc ) {
	  status = sscanf( argv[i], "%d", &(par->dim.z) );
	  if ( status <= 0 ) {
	    i--;
	  }
	}
      }
      
      else if ( strcmp ( argv[i], "-x") == 0 && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->dim.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-y" ) == 0  && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->dim.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-z" ) == 0  && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->dim.z) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-v" ) == 0   && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -v...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->dim.v) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -v...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-voxel" ) == 0
		|| strcmp ( argv[i], "-pixel" ) == 0
		|| (strcmp ( argv[i], "-vs" ) == 0  && argv[i][3] == '\0') ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -voxel..\n", 0 );
	status = sscanf( argv[i],"%f",&(par->voxel.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -voxel...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -voxel..\n", 0 );
	status = sscanf( argv[i],"%f",&(par->voxel.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -voxel...\n", 0 );
	i += 1;
	if ( i < argc ) {
	  status = sscanf( argv[i],"%f",&(par->voxel.z) );
	  if ( status <= 0 ) {
	    i--;
	  }
	}
      }

      else if ( strcmp ( argv[i], "-vx" ) == 0   && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vx...\n", 0 );
	status = sscanf( argv[i], "%f", &(par->voxel.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vx...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-vy" ) == 0   && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vy...\n", 0 );
	status = sscanf( argv[i], "%f", &(par->voxel.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vy...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-vz" ) == 0   && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vz...\n", 0 );
	status = sscanf( argv[i], "%f", &(par->voxel.z) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vz...\n", 0 );
      }

      

      /* template
       */

      else if ( strcmp ( argv[i], "-template") == 0
	   || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
	   || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
	i++;
	if ( i >= argc) VT_ErrorParse( "parsing -template\n", 0 );
	strncpy( par->names.ext, argv[i], STRINGLENGTH ); 
      }



      /* image encoding type
       */

      else if ( strcmp ( argv[i], "-f" ) == 0 && argv[i][2] == '\0' ) {
	f = 1;
      }
      else if ( strcmp ( argv[i], "-r" ) == 0 ) {
	r = 1;
      }
      else if ( strcmp ( argv[i], "-s" ) == 0 ) {
	s = 1;
      }
      else if ( strcmp ( argv[i], "-bytes" ) == 0
		|| (strcmp ( argv[i], "-b" ) == 0 && argv[i][2] == '\0')
		|| (strcmp ( argv[i], "-o" ) == 0 && argv[i][2] == '\0') ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -bytes...\n", 0 );
	status = sscanf( argv[i],"%d",&o );
	if ( status <= 0 ) VT_ErrorParse( "parsing -bytes...\n", 0 );
      }
      


      /*--- option inconnue ---*/
      
      else {
	sprintf(text,"unknown option %s\n",argv[i]);
	VT_ErrorParse(text, 0);
      }
    }
    else if ( argv[i][0] != 0 ) {
      if ( nb == 0 ) { 
	strncpy( par->names.in, argv[i], STRINGLENGTH );  
	nb += 1;
      } else if ( nb == 1 ) { 
	strncpy( par->names.out, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else 
	VT_ErrorParse("too much file names when parsing\n", 0 );
    }
    i += 1;
  }
  if (nb == 0) {
    strcpy( par->names.out, ">" );  /* standart output */
  }
  
  /*--- type de l'image resultat ---*/
  if ( r == 1 ) {
    switch( o ) {
    default :
      VT_ErrorParse( "such byte size not handled for floating types\n", 0 );
      break;
    case 0 :
    case 4 :
      par->type = FLOAT; break;
    case 8 :
      par->type = FLOAT; break;
    }
  }
  else {
    switch( o ) {
    default :
      VT_ErrorParse( "such byte size not handled for integer types\n", 0 );
      break;
    case 0 :
    case 1 :
      par->type = ( s == 1 ) ? SCHAR : UCHAR;
      break;
    case 2 :
      par->type = ( s == 1 ) ? SSHORT : USHORT;
      break;
    case 4 :
      par->type = ( s == 1 ) ? SINT : UINT;
      break;
    case 8 :
      if ( s == 1 )
	VT_ErrorParse( "signed long int not handled yet\n", 0 );
      else 
	par->type = ULINT;
      break;
    }
  }
}



static void VT_ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage : %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",detail);
  (void)fprintf(stderr,"Erreur : %s",str);
  exit(0);
}



static void VT_InitParam( local_par *par )
{
  VT_Names( &(par->names) );

  par->shape = SPHERE;
  par->radius = 64.0 ;

  par->vec[0] = 0.0;
  par->vec[1] = 0.0;
  par->vec[2] = 1.0;

  par->size[0] = 64.0;
  par->size[1] = 64.0;
  par->size[2] = 0.0;

  par->pt1[0] = 127.0;
  par->pt1[1] = 127.0;
  par->pt1[2] = 0.0;

  par->pt2[0] = -1.0;
  par->pt2[1] = -1.0;
  par->pt2[2] = 0.0;

  par->value = 255;

  par->dim.x = -1;
  par->dim.y = -1;
  par->dim.z = -1;
  par->dim.v = -1;
  par->voxel.x = par->voxel.y = par->voxel.z = -1.0;
  par->type = UCHAR;
}
