/****************************************************
 * splitImage.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Thu Mar 21 10:42:48 CET 2013
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

  int nslices;
  int skip;
  
  int firstindex;
  int incindex;
  
} local_par;



/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [format-out]\n\
 [-slices | -z %d] [-skip %d]\n\
 [[-first-index | -fi %d][-1|-0]] [-inc-index | -ii %d] \n\
 [-v] [-help] [-D]";



static char *detail = "\
\n\
   Extract sub-images made from slices from 'image-in'\n\
   If nslices>1: consecutive slices (skip = 1) are extracted\n\
     and dimz/nslices images are created\n\
   if skip>1: skip images are created\n\
\n\
 [format-out]      # in 'printf' format, should contain a %d\n\
 [-slices | -z %d] # number of slices of the ouput image\n\
 [-skip %d]        # number of skipped slices in 'input' between two\n\
                     slices copied in output images. Obviously this\n\
                    is also the number of created images.\n\
 [-first-index %d] # first index to number output images\n\
 [-inc-index %d]   # index increment to number output images\n";


static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image;
  vt_image imres;
  
  char imagename[STRINGLENGTH];

  int theDim[3];
  int resDim[3];
  int i, n, nimages;
  int z, nextz;
  
  

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
    

  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  theDim[0] = resDim[0] = image->dim.x * image->dim.v;
  theDim[1] = resDim[1] = image->dim.y;
  theDim[2] = image->dim.z;


  /* number of created images
   */

  if ( par.skip > 1 ) {
    nimages = par.skip;
    resDim[2] = theDim[2] / nimages;
    if ( par.nslices > 1 && resDim[2] > par.nslices )
      resDim[2] = par.nslices;
    nextz = 1;
  }
  else {
    nimages = theDim[2] / par.nslices;
    resDim[2] = par.nslices;
    nextz = par.nslices;
  }

  fprintf( stderr, "%s: will create %d images of %d slices\n", program, nimages, resDim[2] );
  if ( resDim[2] * nimages < theDim[2] ) 
    fprintf( stderr, "\t the last %d slices of '%s' are skipped\n", theDim[2]-resDim[2]*nimages, par.names.in );


  /* initializing result image
  */
  VT_InitVImage( &imres, (char*)NULL, image->dim.v, image->dim.x, image->dim.y, resDim[2], image->type );
  if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate result image\n", 0);
  }
  imres.siz = image->siz;

  /***************************************************
   *
   * 
   *
   ***************************************************/
  for ( z=0, n=0, i=par.firstindex; n<nimages; z+=nextz, n++, i+=par.incindex ) {
    sprintf( imagename, par.names.out, i );
    if ( par.skip == 1 ) {
      /* copy one block of resDim[2] slices
       */
      ExtractSlicesAndMelt( image->buf, theDim, imres.buf, resDim, 
			    z, par.skip, 0, 1, resDim[2], 1, 
			    image->type );
    }
    else {
      /* copy resDim[2] blocks of 1 slice
       */
      ExtractSlicesAndMelt( image->buf, theDim, imres.buf, resDim, 
			    z, par.skip, 0, 1, 1, resDim[2], 
			    image->type );
    }
    if ( VT_CopyName( imres.name, imagename ) != 1 ) {
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to copy image name\n", 0);
    }
    if ( VT_WriteInrimage( &imres ) == -1 ) {
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output image\n", 0);
    }
  }


  
    /*--- liberations memoires ---*/
  VT_FreeImage( &imres );
  VT_FreeImage( image );
  VT_Free( (void**)&image );
  
  return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  char text[STRINGLENGTH];

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
	par->firstindex = 1;
      }
      else if ( strcmp ( argv[i], "-0" ) == 0 && argv[i][2] == '\0' ) {
	par->firstindex = 0;
      }
      
      else if ( (strcmp (argv[i], "-first-index" ) == 0)
		|| (strcmp (argv[i], "-fi" ) == 0 && argv[i][3] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -first-index %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->firstindex) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -first-index %d\n", 0 );
      }

      else if ( (strcmp (argv[i], "-inc-index" ) == 0)
		|| (strcmp (argv[i], "-ii" ) == 0 && argv[i][3] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -inc-index %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->incindex) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -inc-index %d\n", 0 );
      }

      else if ( (strcmp (argv[i], "-slices" ) == 0)
		|| (strcmp (argv[i], "-z" ) == 0 && argv[i][2] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -slices %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->nslices) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -slices %d\n", 0 );
      }
      
      else if ( (strcmp (argv[i], "-skip" ) == 0) ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -skip %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->skip) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -skip %d\n", 0 );
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

  par->nslices=1;
  par->skip=1;
  par->firstindex=1;
  par->incindex=1;
}
