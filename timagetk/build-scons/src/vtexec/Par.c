/*************************************************************************
 * Par.c - 
 *
 * $Id: Par.c,v 1.5 2001/04/13 18:12:12 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Apr 26 10:04:51 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *
 */

#include <vt_common.h>

#include <sys/types.h>
#include <sys/stat.h>




typedef struct local_par {
  int standardInputIsAnArg;
  int nbImages;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );




static void _PrintImageInfo( vt_image *image /* image whose header is to be printed */,
			     char *name,
			     int nh /* number of blocks of 256 bytes in the header */ );



static char *usage = "[images]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.5 $ $Date: 2001/04/13 18:12:12 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image image;
  int nb_header;
  int imageSize;
  
  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  if ( par.nbImages > 0 ) {
    char message[256];
    struct stat stbuf;
    int i;


    /* a faire :
       image compressee ou non
       cf vt_inrimage.c
    */



    for ( i=1; i<argc; i++ ) {
      /* attention, on n'evite que les args "-*"
       */
      if ( argv[i][0] == '-' ) continue;
      
      VT_Image( &image );
      if ( VT_ReadInrimHeader( &image, argv[i] ) == -1 ) {
	sprintf( message, "Unable to read '%s' header \n", argv[i] );
	VT_ErrorParse( message, 0 );
      }
      
      _PrintImageInfo( &image, argv[i], 1 );
      imageSize = VT_SizeImage( &image );
      
      if ( 0 ) {
	if ( stat( argv[i], &stbuf ) != 0 ) {
	  sprintf( message, "Unable to fill '%s' file info \n", argv[i] );
	  VT_ErrorParse( message, 0 );
	}
	
	if ( stbuf.st_size != nb_header*256+imageSize ) {
	  printf(" ... warning, image size is %d (instead of %d)\n",
		 (int)stbuf.st_size, nb_header*256+imageSize );
	}
      }
      
    }

  }
  



  return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i;
  char text[STRINGLENGTH];
  
  if ( VT_CopyName( program, argv[0] ) != 1 )
    VT_Error("Error while copying program name", (char*)NULL);
  if ( argc == 1 ) VT_ErrorParse("\n", 0 );
  
  /*--- lecture des parametres ---*/
  i = 1;
  while ( i < argc ) {
    if ( argv[i][0] == '-' ) {
      if ( argv[i][1] == '\0' ) {
	/*--- standart input ---*/
	par->standardInputIsAnArg = 1;
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

      /*--- option inconnue ---*/
      else {
	sprintf(text,"unknown option %s\n",argv[i]);
	VT_ErrorParse(text, 0);
      }
    }
    /*--- saisie des noms d'images ---*/
    else if ( argv[i][0] != 0 ) {
      par->nbImages ++;
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
  par->standardInputIsAnArg = 0;
  par->nbImages = 0;
}

















static void _PrintImageInfo( vt_image *image /* image whose header is to be printed */,
			     char *name,
			     int nh /* number of blocks of 256 bytes in the header */ )
{
  int length;
  int baselength = 10;
  int divlength = 10;
  char nameFormat[10];
  


  if ( name != (char*)NULL &&  name[0] != '<' && name[0] != '\0' ) {
    length = strlen( name );
    if ( length <= baselength ) {
      sprintf( nameFormat, "%%-%ds", baselength );
    } else {
      if ( (length - baselength) % divlength == 0 ) 
	sprintf( nameFormat, "%%-%ds", length );
      else
	sprintf( nameFormat, "%%-%ds", baselength+(1+(length - baselength)/divlength)*divlength);
    }
    printf( nameFormat, name );
  } else {
    sprintf( nameFormat, "%%-%ds", baselength );
    printf( nameFormat,"standard input");
  }
  
  printf( " hdr=%-2d",nh);
  printf( " x=%-5lu",image->dim.x);
  printf( " y=%-5lu",image->dim.y);
  printf( " z=%-5lu",image->dim.z);
  if ( image->dim.v != 1 )
    printf( " v=%-5lu",image->dim.v);

  switch ( image->cpu ) {
    case LITTLEENDIAN :
	printf( " cpu=decm    ");
	break;
    case BIGENDIAN :
	printf( " cpu=sun     ");
	break;
    case TYPE_UNKNOWN :
    default :	
	printf( " cpu=inconnu");
    }
    printf( " type=");
    switch ( image->type ) {
    case SCHAR :
	printf( "signed char");
	break;
    case UCHAR :
	printf( "unsigned char");
	break;
    case SSHORT :
	printf( "signed short int");
	break;
    case USHORT :
	printf( "unsigned short int");
	break;
    case FLOAT :
	printf( "float");
	break;
    case DOUBLE :
	printf( "double");
	break;
    case SINT :
	printf( "signed int");
	break;
    case ULINT :
	printf( "unsigned long int");
	break;
    case TYPE_UNKNOWN :
    default :
	printf( "inconnu");
    }
    printf( " vs = %g %g %g",
		  image->siz.x, image->siz.y, image->siz.z );
    if ( image->off.x != 0 || image->off.y != 0 || image->off.z != 0 )
      printf( " offset = %g %g %g",
	      image->off.x, image->off.y, image->off.z );
    if ( image->rot.x != 0 || image->rot.y != 0 || image->rot.z != 0 )
      printf( " rotation = %g %g %g",
	      image->rot.x, image->rot.y, image->rot.z );
    if ( image->ctr.x != 0 || image->ctr.y != 0 || image->ctr.z != 0 )
      printf( " center = %d %d %d",
	      image->ctr.x, image->ctr.y, image->ctr.z );

    printf( "\n");
}
