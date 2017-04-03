/*************************************************************************
 * Logic.c - 
 *
 * $Id: Logic.c,v 1.3 2000/10/20 13:40:23 greg Exp $
 *
 * Copyright (c) INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * ?
 *
 * ADDITIONS, CHANGES
 *
 */


#include <vt_common.h>

#include <pixel-operation.h>


typedef struct local_par {
    vt_names names;
    int type_operation;
} local_par;

#define _NONE_ 0
#define _INV_  1
#define _OU_   2
#define _XOU_  3
#define _ET_   4
#define _MASK_ 5

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[-inv | [-et|-and] | [-ou|-or] | [-xou|-xor] | -mask]\n\
image-in [image-in2] image-out [-v] [-D] [-help]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -inv : inverse 'image-in'\n\
\t -et |-and : ET logic entre image-in et image-in2 (doivent etre de meme type)\n\
\t -ou |-or  : OU logic entre image-in et image-in2 (doivent etre de meme type)\n\
\t -xou|-xor : OU exclusif logic entre image-in et image-in2 (doivent etre de meme type)\n\
\t -mask : si image-in est de type unsigned char, met a zero dans image-in2 les\n\
\t         points correspondants aux zeros de image-in.\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n\
 $Revision: 1.3 $ $Date: 2000/10/20 13:40:23 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, *imaux, *aux;
  int theDim[3];

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );

  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );

  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);

  /*--- lecture de la seconde image et realisation de l'inversion ---*/
  switch ( par.type_operation ) {
  case _ET_ :
  case _OU_ :
  case _XOU_ :
  case _MASK_ :
    /*--- lecture d'une autre image ---*/
    imaux = _VT_Inrimage( par.names.ext );
    if ( imaux == (vt_image*)NULL ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read second input image\n", 0);
    }
    break;

  case _INV_ :
    /*--- inversion ---*/
    VT_InverseImage( image );
    
    if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to copy output image name\n", 0);
    }
    if ( VT_WriteInrimage( image ) == -1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output image\n", 0);
    }
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    return( 1 );

  default :
    /*--- pas de choix ---*/
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("no choice for the logical operation\n", 0 );
  }

  /*--- verification des dimensions ---*/
  if ( image->dim.x != imaux->dim.x 
       || image->dim.y != imaux->dim.y
       || image->dim.z != imaux->dim.z 
       || image->dim.v != imaux->dim.v ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_FreeImage( imaux );
    VT_Free( (void**)&imaux );
    VT_ErrorParse("the two input images have different dimensions\n", 0 );
  }
  
  /*--- verification des types ---*/
  switch ( par.type_operation ) {
  case _ET_ :
  case _OU_ :
  case _XOU_ :

    if ( image->type != imaux->type ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( imaux );
      VT_Free( (void**)&imaux );
      VT_ErrorParse("the two input images have different type\n", 0 );
    }
    break;
  case _MASK_ :
    if ( (image->type != UCHAR) && (imaux->type != UCHAR) ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( imaux );
      VT_Free( (void**)&imaux );
      VT_ErrorParse("none of both input images is unsigned char\n", 0 );
    }
    if ( image->type != UCHAR ) {
      aux = image;
      image = imaux;
      imaux = aux;
    }
  }

  /*--- operations ---*/
  switch ( par.type_operation ) {
  case _ET_ :
    VT_LogicEt( image, imaux, imaux );
    break;
  case _OU_ :
    VT_LogicOu( image, imaux, imaux );
    break;
  case _XOU_ :
    VT_LogicXou( image, imaux, imaux );
    break;
  case _MASK_ :
    theDim[0] = image->dim.x;
    theDim[1] = image->dim.y;
    theDim[2] = image->dim.z;
    if ( maskImage( imaux->buf, imaux->type,
			image->buf, image->type, 
			imaux->buf, imaux->type, theDim ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( imaux );
      VT_Free( (void**)&imaux );
      VT_ErrorParse( "error when masking image\n", 0 );
    }
    break;
  }
  
  VT_FreeImage( image );
  VT_Free( (void**)&image );

  /*--- ecriture de l'image resultat ---*/
  if ( VT_CopyName( imaux->name, par.names.out ) == 0 ) {  
    VT_FreeImage( imaux );
    VT_Free( (void**)&imaux );
    VT_ErrorParse("unable to copy output image name\n", 0);
  }
  if ( VT_WriteInrimage( imaux ) == -1 ) {
    VT_FreeImage( imaux );
    VT_Free( (void**)&imaux );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
    int i, nb;
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
	    /*--- operations logiques ---*/
	    else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
		par->type_operation = _INV_;
	    }
	    else if ( strcmp ( argv[i], "-et" ) == 0 ) {
		par->type_operation = _ET_;
	    }
	    else if ( strcmp ( argv[i], "-and" ) == 0 ) {
		par->type_operation = _ET_;
	    }
	    else if ( strcmp ( argv[i], "-ou" ) == 0 ) {
		par->type_operation = _OU_;
	    }
	    else if ( strcmp ( argv[i], "-or" ) == 0 ) {
		par->type_operation = _OU_;
	    }
	    else if ( strcmp ( argv[i], "-xou" ) == 0 ) {
		par->type_operation = _XOU_;
	    }
	    else if ( strcmp ( argv[i], "-xor" ) == 0 ) {
		par->type_operation = _XOU_;
	    }
	    else if ( strcmp ( argv[i], "-mask" ) == 0 ) {
		par->type_operation = _MASK_;
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
		strncpy( par->names.ext, argv[i], STRINGLENGTH );  
		nb += 1;
	    }
	    else if ( nb == 2 ) {
		strncpy( par->names.out, argv[i], STRINGLENGTH );  
		nb += 1;
	    }
	    else 
		VT_ErrorParse("too much file names when parsing\n", 0 );
	}
	i += 1;
    }

    /*--- s'il n'y a pas assez de noms ... ---*/
    switch ( par->type_operation ) {
    case _ET_ :
    case _OU_ :
    case _XOU_ :
    case _MASK_ :
      if ( nb == 0 ) 
	VT_ErrorParse("not enough file names when parsing\n", 0 );
      if ( nb == 1 ) {
	strcpy( par->names.ext, par->names.in );
	strcpy( par->names.in,  "<" );  /* standart input */
	strcpy( par->names.out, ">" );  /* standart output */
      }
      if ( nb == 2 )
	strcpy( par->names.out, ">" );  /* standart output */
      break;
    case _INV_ :
      if (nb == 0) {
	strcpy( par->names.in,  "<" );  /* standart input */
	strcpy( par->names.out, ">" );  /* standart output */
      }
      if (nb == 1)
	strcpy( par->names.out, ">" );  /* standart output */
      if (nb == 2) 
	strcpy( par->names.out, par->names.ext );
      if (nb == 3)
	VT_ErrorParse("too much file names when parsing\n", 0 );
      break;
    default :
      VT_ErrorParse("no choice for the logical operation\n", 0 );
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
	par->type_operation = _NONE_;
}
