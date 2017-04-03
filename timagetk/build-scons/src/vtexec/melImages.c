/****************************************************
 * melImages.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sun Feb 10 16:59:14 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */



#include <extract.h>
#include <vt_common.h>






typedef struct local_par {
  vt_names names;
  char out[STRINGLENGTH];

  int origin_definition;
  vt_ipt origin;
  vt_ipt dim;
  vt_ipt slice;

  vt_ipt out_origin;
  
} local_par;



/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [[image-aux] image-out]\n\
 [-origin %d %d [%d] | [-ix %d] [-iy %d] [-iz %d]] [-0 | -1]\n\
 [-dim %d %d [%d] | [-x %d] [-y %d] [-z %d]] [-template %s]\n\
 [-xy %d | -yz %d | -xz %d]\n\
 [-out-origin %d %d [%d] | [-ox %d] [-oy %d] [-oz %d]]\n\
 [-v] [-help] [-D]";



static char *detail = "\
\n\
   Extract a sub-image from 'image-in' and melt it in 'image-aux'.\n\
\n\
 [-origin %d %d [%d]]   # origin of the extracted image in the input image\n\
                          default is (0,0,0)\n\
 [-ix %d]               # X coordinates of the origin in the input image\n\
 [-iy %d]               # Y coordinates of the origin in the input image\n\
 [-iz %d]               # Z coordinates of the origin in the input image\n\
 [-0]                   # first coordinate is 0 (default)\n\
 [-1]                   # first coordinate is 1\n\
 [-dim %d %d [%d]]      # extracted image dimensions\n\
 [-x %d]                # X dimension of the extracted image\n\
 [-y %d]                # Y dimension of the extracted image\n\
 [-z %d]                # Z dimension of the extracted image\n\
 [-template %s]         # template image for the dimensions\n\
                          of the extracted image\n\
 [-xy %d]   # extract the XY slice #%d\n\
            # this is equivalent to '-origin 0 0 %d -dim dimx dimy 1'\n\
 [-xz %d]   # extract the XZ slice #%d\n\
            # this is equivalent to '-origin 0 %d 0 -dim dimx 1 dimz'\n\
 [-yz %d]   # extract the YZ slice #%d\n\
            # this is equivalent to '-origin %d 0 0 -dim 1 dimy dimz'\n\
 [-out-origin %d %d [%d]]   # origin of the extracted image in the output image\n\
                          default is (0,0,0)\n\
 [-ox %d]               # X coordinates of the origin in the output image\n\
 [-oy %d]               # Y coordinates of the origin in the output image\n\
 [-oz %d]               # Z coordinates of the origin in the output image\n";


static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image;
  vt_image *imres;
  vt_image *imtemplate;
  
  int theDim[4];
  int resDim[4];
  int winDim[3] = {-1, -1, -1};
  int theLeftCorner[3] = {0, 0, 0};
  int theRightCorner[3] = {0, 0, 0};
  int resLeftCorner[3] = {0, 0, 0};
  int resRightCorner[3] = {0, 0, 0};
  
  

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
    

  
  /*
   * if the indices do not begin with 0
   */
  if ( par.origin_definition > 0 ) {
    par.origin.x -= par.origin_definition;
    par.origin.y -= par.origin_definition;
    par.origin.z -= par.origin_definition;

    par.out_origin.x -= par.origin_definition;
    par.out_origin.y -= par.origin_definition;
    par.out_origin.z -= par.origin_definition;
 
    par.slice.x -= par.origin_definition;
    par.slice.y -= par.origin_definition;
    par.slice.z -= par.origin_definition;
  }



  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  theDim[0] = image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;
  theDim[3] = image->dim.v;



  /* initializing window image
     - with reference image, if any
     - with parameters, if any
  */

  if ( par.names.ext[0] != '\0' ) {
    imtemplate = _VT_Inrimage( par.names.ext );
    if ( imtemplate == (vt_image*)NULL ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read template image\n", 0);
    }

    theLeftCorner[0] = par.origin.x;
    theLeftCorner[1] = par.origin.y;
    theLeftCorner[2] = par.origin.z;

    theRightCorner[0] = par.origin.x + imtemplate->dim.x;
    theRightCorner[1] = par.origin.y + imtemplate->dim.y;
    theRightCorner[2] = par.origin.z + imtemplate->dim.z;

    VT_FreeImage( imtemplate );
    VT_Free( (void**)&imtemplate );

    if ( theLeftCorner[0] < 0 ) theLeftCorner[0] = 0;
    if ( theLeftCorner[1] < 0 ) theLeftCorner[1] = 0;
    if ( theLeftCorner[2] < 0 ) theLeftCorner[2] = 0;
    
    if ( theRightCorner[0] > (int)image->dim.x ) theRightCorner[0] = image->dim.x;
    if ( theRightCorner[1] > (int)image->dim.y ) theRightCorner[1] = image->dim.y;
    if ( theRightCorner[2] > (int)image->dim.z ) theRightCorner[2] = image->dim.z;

    winDim[0] = theRightCorner[0] - theLeftCorner[0];
    winDim[1] = theRightCorner[1] - theLeftCorner[1];
    winDim[2] = theRightCorner[2] - theLeftCorner[2];
  }
      


  /* initialisation with dimension parameters
   */
  else if ( par.dim.x > 0 && par.dim.y > 0 ) {

    theLeftCorner[0] = par.origin.x;
    theLeftCorner[1] = par.origin.y;

    theRightCorner[0] = par.origin.x + par.dim.x;
    theRightCorner[1] = par.origin.y + par.dim.y;

    if ( theLeftCorner[0] < 0 ) theLeftCorner[0] = 0;
    if ( theLeftCorner[1] < 0 ) theLeftCorner[1] = 0;
    
    if ( theRightCorner[0] > (int)image->dim.x ) theRightCorner[0] = image->dim.x;
    if ( theRightCorner[1] > (int)image->dim.y ) theRightCorner[1] = image->dim.y;

    if ( par.dim.z > 0 ) {
      
      theLeftCorner[2] = par.origin.z;
      theRightCorner[2] = par.origin.z + par.dim.z;

      if ( theLeftCorner[2] < 0 ) theLeftCorner[2] = 0;
      if ( theRightCorner[2] > (int)image->dim.z ) theRightCorner[2] = image->dim.z;
      
    }
    else {

      theLeftCorner[2] = 0;
      theRightCorner[2] = 1;

    }

    winDim[0] = theRightCorner[0] - theLeftCorner[0];
    winDim[1] = theRightCorner[1] - theLeftCorner[1];
    winDim[2] = theRightCorner[2] - theLeftCorner[2];
  }



  /* initialisation with slice information
   */
  else if ( par.slice.z >= 0 && par.slice.z < (int)image->dim.z ) {

    winDim[0] = image->dim.x;
    winDim[1] = image->dim.y;
    winDim[2] = 1;

    theLeftCorner[2] = par.slice.z;

  }

  else if ( par.slice.y >= 0 && par.slice.y < (int)image->dim.y ) {

    winDim[0] = image->dim.x;
    winDim[1] = 1;
    winDim[2] = image->dim.z;

    theLeftCorner[1] = par.slice.y;

  }

  else if ( par.slice.x >= 0 && par.slice.x < (int)image->dim.x ) {

    winDim[0] = 1;
    winDim[1] = image->dim.y;
    winDim[2] = image->dim.z;

    theLeftCorner[0] = par.slice.x;

  }

  /* no dimension parameters
   */

  else {
    
    winDim[0] = image->dim.x;
    winDim[1] = image->dim.y;
    winDim[2] = image->dim.z;
    
    theLeftCorner[0] = 0;
    theLeftCorner[1] = 0;
    theLeftCorner[2] = 0;
  }


  if ( _VT_DEBUG_ ) {
    fprintf( stderr, "... will copy window of size [%d %d %d] from (%d %d %d)\n",
	     winDim[0], winDim[1], winDim[2], theLeftCorner[0], theLeftCorner[1], theLeftCorner[2] );
  }


  /*---  target image ---*/
  imres = _VT_Inrimage( par.names.out );
  if ( imres == (vt_image*)NULL ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to read second input image\n", 0);
  }
  
  resDim[0] = imres->dim.x;
  resDim[1] = imres->dim.y;
  resDim[2] = imres->dim.z;
  resDim[3] = imres->dim.v;

  resLeftCorner[0] = par.out_origin.x;
  resLeftCorner[1] = par.out_origin.y;
  resLeftCorner[2] = par.out_origin.z;

  theRightCorner[0] = par.out_origin.x + winDim[0];
  theRightCorner[1] = par.out_origin.y + winDim[1];
  theRightCorner[2] = par.out_origin.z + winDim[2];

  /* the point is:
     should we decrease both theLeftCorner and resLeftCorner?
  */
  if ( resLeftCorner[0] < 0 ) resLeftCorner[0] = 0;
  if ( resLeftCorner[1] < 0 ) resLeftCorner[1] = 0;
  if ( resLeftCorner[2] < 0 ) resLeftCorner[2] = 0;

  if ( resRightCorner[0] > (int)imres->dim.x )
    winDim[0] -= resRightCorner[0]-imres->dim.x;
  if ( resRightCorner[1] > (int)imres->dim.y )
    winDim[1] -= resRightCorner[1]-imres->dim.y;
  if ( resRightCorner[2] > (int)imres->dim.z )
    winDim[2] -= resRightCorner[2]-imres->dim.z;
  

  if ( imres->type != image->type 
       || imres->dim.v != image->dim.v ) {
      VT_FreeImage( imres );
      VT_Free( (void**)&imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "input images have different charecteristics\n", 0);
  }

  if ( winDim[0] + par.out_origin.x > (int)imres->dim.x
       || winDim[1] + par.out_origin.y > (int)imres->dim.y
       || winDim[2] + par.out_origin.z > (int)imres->dim.z ) {
      if ( _VT_VERBOSE_ ) {
	fprintf( stderr, "\t [%d,%d,%d] does not fit in [%lu,%lu,%lu] from (%d,%d,%d)\n",
		 winDim[0], winDim[1], winDim[2],
		 imres->dim.x, imres->dim.y, imres->dim.z,
		 par.out_origin.x, par.out_origin.y, par.out_origin.z );
      }
      VT_FreeImage( imres );
      VT_Free( (void**)&imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("too large subimage (2)\n", 0);
    }

  /***************************************************
   *
   * 
   *
   ***************************************************/

  ExtractFromBufferAndMelt( image->buf, theDim, imres->buf, resDim, theLeftCorner, resLeftCorner, winDim, image->type );

  /***************************************************
   *
   * 
   *
   ***************************************************/
    
    /*--- liberations memoires ---*/
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    
    if ( par.out[0] != '\0' ) 
      strncpy( imres->name, par.out, STRINGLENGTH );  

    if ( VT_WriteInrimage( imres ) == -1 ) {
      VT_FreeImage( imres );
      VT_Free( (void**)&imres );
      VT_ErrorParse("unable to write output image\n", 0);
    }

    /*--- liberations memoires ---*/
    VT_FreeImage( imres );
    VT_Free( (void**)&imres );

    return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  char text[STRINGLENGTH];
  float t;

  if ( VT_CopyName( program, argv[0] ) != 1 )
    VT_Error("Error while copying program name", (char*)NULL);
  if ( argc == 1 ) VT_ErrorParse("\n", 0 );
  
  /*--- lecture des parametres ---*/
  i = 1; nb = 0;
  while ( i < argc ) {
    if ( argv[i][0] == '-' ) {
      if ( argv[i][1] == '\0' ) {
	if ( nb == 0 ) {
	  /*--- standart input ---*/
	  strcpy( par->names.in, "<" );
	  nb += 1;
	}
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
		|| (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
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

      /* origin of the subblock in input
       */
      
      else if ( strcmp ( argv[i], "-1" ) == 0 && argv[i][2] == '\0' ) {
	par->origin_definition = 1;
      }
      else if ( strcmp ( argv[i], "-0" ) == 0 && argv[i][2] == '\0' ) {
	par->origin_definition = 0;
      }
      
      else if ( (strcmp (argv[i], "-origin" ) == 0 && argv[i][7] == '\0')
		|| (strcmp (argv[i], "-o" ) == 0 && argv[i][2] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -origin %d\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -origin %d\n", 0 );
	par->origin.x = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -origin %d %d\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -origin %d %d\n", 0 );
	par->origin.y = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
	i ++;
	if ( i >= argc) par->origin.z = (par->origin_definition == 1) ? 1 : 0;
	else {
	  status = sscanf( argv[i], "%f", &t );
	  if ( status <= 0 ) {
	    i--;
	    par->origin.z = (par->origin_definition == 1) ? 1 : 0;
	  }
	  else {
	    par->origin.z = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
	  }
	}
      }

      else if ( strcmp ( argv[i], "-ix" ) == 0 && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -ix...\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -ix...\n", 0 );
	par->origin.x = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }
      else if ( strcmp ( argv[i], "-iy" ) == 0 && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -iy...\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -iy...\n", 0 );
	par->origin.y = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }
      else if ( strcmp ( argv[i], "-iz" ) == 0  && argv[i][3] == '\0') {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -iz...\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -iz...\n", 0 );
	par->origin.z = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }

      /* origin of the subblock in output
       */
      
      else if ( (strcmp (argv[i], "-out-origin" ) == 0 && argv[i][11] == '\0')
		|| (strcmp (argv[i], "-oo" ) == 0 && argv[i][3] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -out-origin %d\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -out-origin %d\n", 0 );
	par->out_origin.x = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -out-origin %d %d\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -out-origin %d %d\n", 0 );
	par->out_origin.y = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
	i ++;
	if ( i >= argc) par->out_origin.z = (par->origin_definition == 1) ? 1 : 0;
	else {
	  status = sscanf( argv[i], "%f", &t );
	  if ( status <= 0 ) {
	    i--;
	    par->out_origin.z = (par->origin_definition == 1) ? 1 : 0;
	  }
	  else {
	    par->out_origin.z = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
	  }
	}
      }

      else if ( strcmp ( argv[i], "-ox" ) == 0 && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -ox...\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -ox...\n", 0 );
	par->out_origin.x = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }
      else if ( strcmp ( argv[i], "-oy" ) == 0 && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -oy...\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -oy...\n", 0 );
	par->out_origin.y = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }
      else if ( strcmp ( argv[i], "-oz" ) == 0  && argv[i][3] == '\0') {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -oz...\n", 0 );
	status = sscanf( argv[i], "%f", &t );
	if ( status <= 0 ) VT_ErrorParse( "parsing -oz...\n", 0 );
	par->out_origin.z = (t > 0.0) ? (int)(t+0.5) : (int)(t-0.5);
      }

      /* dimensions of the subblock
       */

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
	if ( i >= argc) par->dim.z = 1;
	else {
	  status = sscanf( argv[i], "%d", &(par->dim.z) );
	  if ( status <= 0 ) {
	    i--;
	    par->dim.z = 1;
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

      /* slices
       */

      else if ( strncmp ( argv[i], "-yz", 3 ) == 0 && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -yz...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->slice.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -yz...\n", 0 );
      }
      else if ( strncmp ( argv[i], "-xz", 3 ) == 0 && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -xz...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->slice.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -xz...\n", 0 );
      }
      else if ( strncmp ( argv[i], "-xy", 3 ) == 0 && argv[i][3] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -xy...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->slice.z) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -xy...\n", 0 );
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
      }
      else if ( nb == 1 ) {
	strncpy( par->names.out, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else if ( nb == 2 ) {
	strncpy( par->out, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else 
	VT_ErrorParse("too much file names when parsing\n", 0 );
    }
    i += 1;
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
  par->out[0] = '\0';

  par->origin_definition = 0;
  par->origin.x = par->origin.y = par->origin.z = 0;
  par->dim.x = par->dim.y = par->dim.z = -1;
  par->slice.x = par->slice.y = par->slice.z = -1;

  par->out_origin.x = par->out_origin.y = par->out_origin.z = 0;
}
