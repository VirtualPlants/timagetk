/*************************************************************************
 * minimum.c -
 *
 * $Id: setVoxelValue.c,v 1.1 2003/07/04 08:21:51 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 */

#include <vt_common.h>

typedef struct local_par {
  int x, y, z;
  double v;
  vt_names names;
} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );





static char *usage = "[image-in] [image-out] -x %d -y %d [-z %d] -i %lf\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.1 $ $Date: 2003/07/04 08:21:51 $ $Author: greg $\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image;
  
  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );
  

  if ( par.z == -1 && image->dim.z == 1 ) par.z = 0;
  
  if ( par.x < 0 || par.x >= (int)image->dim.x
       || par.y < 0 || par.x >= (int)image->dim.y ) {
    
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("bad parameters \n", 0);
    
  }

  if ( _VT_VERBOSE_ ) {
    printf( " will set (%d,%d,%d) at %f in ", par.x, par.y, par.z, par.v );
    if ( par.names.out[0] == '\0' || par.names.out[0] == '>' )
      printf( "'%s'\n", image->name );
    else 
      printf( "'%s'\n", par.names.out );
  }

  switch ( image->type ) {
  default :
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("image type not yet implemented \n", 0);

  case UCHAR :
    {
      u8 *** theBuf = (u8***)image->array;
      theBuf[par.z][par.y][par.x] = (u8)( (int)(par.v + 0.5) );
    }
    break;

  case USHORT :
    {
      u16 *** theBuf = (u16***)image->array;
      theBuf[par.z][par.y][par.x] = (u16)( (int)(par.v + 0.5) );
    }
    break;

  case SSHORT :
    {
      s16 *** theBuf = (s16***)image->array;
      if ( par.v >= 0 )
	theBuf[par.z][par.y][par.x] = (s16)( (int)(par.v + 0.5) );
      else 
	theBuf[par.z][par.y][par.x] = (s16)( (int)(par.v - 0.5) );	
    }
    break;
    
  }

  if ( par.names.out[0] != '\0' && par.names.out[0] != '>' )
    sprintf( image->name, "%s", par.names.out );
  
  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( image ) == -1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( image );
  VT_Free( (void**)&image );
  return( 1 );
}








static void VT_Parse( int argc, 
		      char *argv[], 
		      local_par *par )
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
      /*--- arguments generaux ---*/
      else if ( strcmp ( argv[i], "-help" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
	_VT_VERBOSE_ = 1;
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 ) {
	_VT_DEBUG_ = 1;
      }
      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }


      else if ( strcmp ( argv[i], "-x" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-y" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-z" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->z) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-i" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -i...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->v) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -i...\n", 0 );
      }


      /*--- option inconnue ---*/
      else {
	sprintf(text,"unknown option %s\n",argv[i]);
	VT_ErrorParse(text, 0);
      }
    }
    /*--- saisie des noms d'images ---*/
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
  
  /*--- s'il n'y a pas assez de noms ... ---*/
  if (nb == 0) {
    strcpy( par->names.in,  "<" );  /* standart input */
    strcpy( par->names.out, ">" );  /* standart output */
  }
  if (nb == 1)
    strcpy( par->names.out, ">" );  /* standart output */
  
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
  par->x = -1;
  par->y = -1;
  par->z = -1;
  par->v = 0;
}
