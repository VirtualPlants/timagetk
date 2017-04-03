/****************************************************
 * mergeSlices.c - 
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
  int firstslice;
  int lastslice;
  
} local_par;



/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[format-in] [image-out]\n\
 [-slices | -nz | -z %d] [-fz %d] [-lz %d]\n\
 [-verbose|-v | -no-verbose|-nv] [-help] [-D]";



static char *detail = "\
\n\
   Merge slices\n\
\n\
 [format-in]       # in 'printf' format, should contain a %d\n\
 [-slices | -z %d] # number of slices (consecutively numbered) \n\
 [-fz %d]          # index of the first slice\n\
 [-lz %d]          # index of the last slice\n";


static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image;
  vt_image imres;
  
  char imagename[STRINGLENGTH];

  int theDim[4];
  int resDim[4];

  int z, iz, lz, nimages;
  size_t size;
  unsigned char *resBuf;

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
    

  iz = par.firstslice;
  if ( par.nslices >= 0 ) {
    lz = iz + par.nslices - 1;
  }
  else if ( par.lastslice >= par.firstslice ) {
    lz = par.lastslice;
  }
  nimages = lz - iz + 1;

  /*--- lecture de la premiere image d'entree ---*/
  sprintf( imagename, par.names.in, iz );
  image = _VT_Inrimage( imagename );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  theDim[0] = resDim[0] = image->dim.x;
  theDim[1] = resDim[1] = image->dim.y;
  theDim[2] = image->dim.z;
  resDim[2] = image->dim.z * nimages;
  theDim[3] = resDim[3] = image->dim.v;

  
  switch ( image->type ) {
  case UCHAR  : size = sizeof( u8 );  break;
  case SCHAR  : size = sizeof( s8 );  break;
  case USHORT : size = sizeof( u16 ); break;
  case SSHORT : size = sizeof( s16 ); break;
  case UINT   : size = sizeof( u32 ); break;
  case SINT    : size = sizeof( i32 ); break;
  case ULINT  : size = sizeof( u64 ); break;
  case FLOAT  : size = sizeof( r32 ); break;
  case DOUBLE : size = sizeof( r64 ); break;
  default :
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("buffer type not handled yet\n", 0);
  }  
  size *= image->dim.x * image->dim.y * image->dim.z * image->dim.v;


  if ( _VT_VERBOSE_  || _VT_DEBUG_ )
    fprintf( stderr, "%s: will merge %d images of %d slices\n", program, nimages, theDim[2] );

 
  VT_InitVImage( &imres, par.names.out, resDim[3], resDim[0], resDim[1], resDim[2], image->type );
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate result image\n", 0);
  }
  resBuf = (unsigned char*)imres.buf;


  VT_FreeImage( image );
  VT_Free( (void**)&image );
  
  /***************************************************
   *
   * 
   *
   ***************************************************/
  theDim[0] *= theDim[3];
  resDim[0] *= resDim[3];

  for ( z=iz; z<=lz; z++, resBuf += size ) {

    sprintf( imagename, par.names.in, z );
    image = _VT_Inrimage( imagename );
    if ( image == (vt_image*)NULL ) {
      VT_FreeImage( &imres );
      fprintf( stderr, "... reading '%s'\n", imagename );
      VT_ErrorParse("unable to read input image\n", 0);
    }


    ExtractSlicesAndMelt( image->buf, theDim, resBuf, theDim, 
			  0, 0, 0, 0, theDim[2], 1, 
			  image->type );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
  }

  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( &imres );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( &imres );
  
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

      else if ( strcmp ( argv[i], "-no-verbose" ) == 0
		|| (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
        _VT_VERBOSE_ = 0;
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
	par->firstslice = 1;
      }
      else if ( strcmp ( argv[i], "-0" ) == 0 && argv[i][2] == '\0' ) {
	par->firstslice = 0;
      }

      else if ( (strcmp (argv[i], "-slices" ) == 0)
		|| (strcmp (argv[i], "-nz" ) == 0 && argv[i][3] == '\0')
		|| (strcmp (argv[i], "-z" ) == 0 && argv[i][2] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -slices %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->nslices) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -slices %d\n", 0 );
      }
      
      else if ( (strcmp (argv[i], "-fz" ) == 0 && argv[i][3] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -fz %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->firstslice) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -fz %d\n", 0 );
      }

      else if ( (strcmp (argv[i], "-lz" ) == 0 && argv[i][3] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -lz %d\n", 0 );
	status = sscanf( argv[i], "%d", &(par->lastslice) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -lz %d\n", 0 );
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

  par->nslices = -1;
  par->firstslice = 1;
  par->lastslice = -1;

}
