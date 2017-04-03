/*************************************************************************
 * copy.c - copie d'images
 *
 * $Id: copy.c,v 1.5 2003/06/20 09:05:09 greg Exp $
 *
 * DESCRIPTION: 
 *
 *
 *
 *
 *
 * AUTHOR:
 * Gregoire Malandain
 *
 * CREATION DATE:
 * 1996
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 * - Thu Jul 22 11:07:12 MET DST 1999 (GM)
 *   cas ou l'on voulait juste swapper ou inverser l'image
 *   on ne donne pas de type ou d'options de normalisation
 *
 *
 */

#include <ImageIO.h>
#include <convert.h>
#include <vt_common.h>



typedef enum {
  VT_NONE = 0,
  _NORMA_ = 1,
  _MULT_NORMA_ = 2,
  _INTERVAL_INTO_ = 3,
  _INTERVAL_FROM_ = 4
} TypeOperation;



typedef struct local_par {
  vt_names names;
  bufferType type;
  double min;
  double max;
  TypeOperation type_computation;
} local_par;



/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );




static char *usage = "[image-in] [image-out] [-inv] [-swap] [-v] [-D] [-help]\n\
 [-norma] [-mult_norma] [-min %f] [-max %f] [options-de-type]";



static char *detail = "\
 si 'image-in' est '-', on prendra stdin\n\
 si 'image-out' est absent, on prendra stdout\n\
 si les deux sont absents, on prendra stdin et stdout\n\
 -inv : inverse 'image-in'\n\
 -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
 -v : mode verbose\n\
 -D : mode debug\n\
 -norma : normalisation de 'image-in' avant copie dans 'image-out'\n\
          (transformation lineaire (ax+b) de l'intensite\n\
 -mult_norma : normalisation de 'image-in' avant copie dans 'image-out'\n\
          (transformation multiplicative (ax) de l'intensite\n\
 [-min %f -max %f [-into]] : normalisation de l'image entre min et max\n\
 [-min %f -max %f [-from]] : normalisation de [min,max] dans l'image de sortie\n\
   permet de normaliser plusieurs images de facon coherente.\n\
 options-de-type : -o 1    : unsigned char\n\
                   -o 2    : unsigned short int\n\
                   -o 2 -s : short int\n\
                   -o 4 -s : int\n\
                   -r      : float\n\
si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n";



static char program[STRINGLENGTH];




int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imres;
  
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
  
  
  /* Thu Jul 22 11:07:12 MET DST 1999 (GM)
     si on ne normalise pas, 
     et si le type de sortie est celui d'entree
     alors au mieux on voulait swapper ou inverser
     
     on change le nom de l'image, on l'ecrit et on sort
  */
  if ( par.type_computation == VT_NONE && par.type == TYPE_UNKNOWN ) {
    (void)VT_CopyName( image->name, par.names.out );
    if ( VT_WriteInrimage( image ) == -1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output image\n", 0);
    }
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    return(1);
  }
  
  
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  VT_InitFromImage( &imres, image, par.names.out, image->type );
  if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }

  if ( _VT_DEBUG_ ) {
    fprintf( stderr, "input image type: " );
    printType( stderr, image->type );
    fprintf( stderr, "output image type: " );
    printType( stderr, imres.type );
  }

  
  /*--- copie de l'image d'entree dans l'image resultat ---*/
  switch ( par.type_computation ) {
  case _NORMA_ :

    if ( VT_NormaImage( image, &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to normalize input image\n", 0);
    }
    break;
  case _MULT_NORMA_ :
    if ( VT_NormaMultImage( image, &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to normalize input image\n", 0);
    }
    break;
  case _INTERVAL_INTO_ :
    if ( VT_NormaImageWithinInterval( image, &imres, par.min, par.max ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to normalize input image\n", 0);
    }
    break;
  case _INTERVAL_FROM_ :
    if ( VT_NormaImageFromInterval( image, &imres, par.min, par.max ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to normalize input image\n", 0);
    }
    break;
  case VT_NONE :
  default :
    if ( VT_CopyImage( image, &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to copy input image\n", 0);
    }
  }
  
  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( image );
    VT_FreeImage( &imres );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( image );
  VT_FreeImage( &imres );
  VT_Free( (void**)&image );
  return(1);
}




static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  int o=0, s=0, r=0;
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
      else if ( strcmp ( argv[i], "-help" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }
      else if ( strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0' ) {
	_VT_VERBOSE_ = 1;
        _IncrementVerboseInImageIO();
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0' ) {
	_VT_DEBUG_ = 1;
        _IncrementDebugInImageIO();
      }
      else if ( strcmp ( argv[i], "-norma" ) == 0 ) {
	par->type_computation = _NORMA_;
      }
      else if ( strcmp ( argv[i], "-mult_norma" ) == 0 ) {
	par->type_computation = _MULT_NORMA_;
      }
      else if ( strcmp ( argv[i], "-min" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -min...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->min) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -min...\n", 0 );
	if ( par->type_computation != _INTERVAL_INTO_ || par->type_computation != _INTERVAL_FROM_ )
	  par->type_computation = _INTERVAL_INTO_;
      }
      else if ( strcmp ( argv[i], "-max" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -max...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->max) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -max...\n", 0 );
	if ( par->type_computation != _INTERVAL_INTO_ || par->type_computation != _INTERVAL_FROM_ )
	  par->type_computation = _INTERVAL_INTO_;
      }
      else if ( strcmp ( argv[i], "-into" ) == 0 ) {
	par->type_computation = _INTERVAL_INTO_;
      }
      else if ( strcmp ( argv[i], "-from" ) == 0 ) {
	par->type_computation = _INTERVAL_FROM_;
      }

      /*--- lecture du type de l'image ---*/
      else if ( strcmp ( argv[i], "-r" ) == 0 && argv[i][2] == '\0' ) {
	r = 1;
      }
      else if ( strcmp ( argv[i], "-s" ) == 0 && argv[i][2] == '\0' ) {
	s = 1;
      }
      else if ( strcmp ( argv[i], "-o" ) == 0 && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -o...\n", 0 );
	status = sscanf( argv[i],"%d",&o );
	if ( status <= 0 ) VT_ErrorParse( "parsing -o...\n", 0 );
      }
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
  /*--- noms des images ---*/
  if (nb == 0) {
    strcpy( par->names.in,  "<" );  /* standart input */
    strcpy( par->names.out, ">" );  /* standart output */
  }
  if (nb == 1)
    strcpy( par->names.out, ">" );  /* standart output */
  /*--- type de l'image resultat ---*/
  if ( (o == 1) && (s == 1) && (r == 0) ) par->type = SCHAR;
  if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
  if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
  if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
  if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
  if ( (o == 0 || o == 4) && (s == 0) && (r == 1) )  par->type = FLOAT;
  if ( (o == 8) && (s == 0) && (r == 1) )  par->type = DOUBLE;
  if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program);
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
  par->min = 0;
  par->max = 1;
  par->type = TYPE_UNKNOWN;
  par->type_computation = VT_NONE;
}
