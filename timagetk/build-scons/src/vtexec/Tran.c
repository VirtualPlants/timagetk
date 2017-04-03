#include <vt_common.h>

typedef enum {
  _NULL_,
  _SX_,
  _SY_,
  _SZ_,
  _DA_,
  _DB_,
  _DC_,
  _DD_,
  _DE_,
  _DF_,
  _TA_,
  _TB_,
  _TC_,
  _YZX_,
  _ZXY_
} enumTransformation;

typedef struct local_par {
  vt_names names;
  enumTransformation typeTransformation;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-sx | -sy | -sz | \n\
\t -da | -db | -dc | -dd | -de | -df | -ta | -tb | -tc | -yzx | -zxy]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -sx : symetrie / ligne mediane\n\
\t -sy : symetrie / colonne mediane\n\
\t -sz : symetrie / plan median\n\
\t -da : symetrie / 1ere diagonale dans le plan XY\n\
\t -db : symetrie / 2eme diagonale dans le plan XY\n\
\t -dc : symetrie / 1ere diagonale dans le plan YZ\n\
\t -dd : symetrie / 2eme diagonale dans le plan YZ\n\
\t -de : symetrie / 1ere diagonale dans le plan XZ\n\
\t -df : symetrie / 2eme diagonale dans le plan XZ\n\
\t -ta : rotation  90 deg. autour du centre (dans le plan XY)\n\
\t -tb : rotation 180 deg. autour du centre (dans le plan XY)\n\
\t -tc : rotation 270 deg. autour du centre (dans le plan XY)\n\
\t -yzx : transposition, le point (x,y,z) de l'image d'entree\n\
\t        est transforme en  (y,z,x) dans l'image de sortie\n\
\t -zxy : transposition, le point (x,y,z) de l'image d'entree\n\
\t        est transforme en  (z,x,y) dans l'image de sortie\n\
\t\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n\
 $Revision: 1.9 $ $Date: 2003/02/14 12:48:23 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imres;
  int x, y, z;
  
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

  
  
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  switch ( par.typeTransformation ) {
  case _SX_ :
  case _SY_ :
  case _SZ_ :
  case _TB_ :
    sprintf( image->name, "%s", par.names.out );
    break;
  case _DA_ :
  case _DB_ :
  case _TA_ :
  case _TC_ :
    VT_InitImage( &imres, par.names.out, 
		  image->dim.y, image->dim.x, image->dim.z, image->type );
    break;
  case _YZX_ :
    VT_InitImage( &imres, par.names.out, 
		  image->dim.y, image->dim.z, image->dim.x, image->type );
    break;
  case _ZXY_ :
    VT_InitImage( &imres, par.names.out, 
		  image->dim.z, image->dim.x, image->dim.y, image->type );
    break;
  case _DC_ :
  case _DD_ :
     VT_InitImage( &imres, par.names.out, 
		  image->dim.x, image->dim.z, image->dim.y, image->type );
   break;
  case _DE_ :
  case _DF_ :
    VT_InitImage( &imres, par.names.out, 
		  image->dim.z, image->dim.y, image->dim.x, image->type );
    break;
  default :
    VT_ErrorParse( "unable to deal with such transformation (1)\n", 0);
  }


  switch ( par.typeTransformation ) {
  case _SX_ :
  case _SY_ :
  case _SZ_ :
  case _TB_ :
    break;
  case _DA_ :
  case _DB_ :
  case _DC_ :
  case _DD_ :
  case _DE_ :
  case _DF_ :
  case _TA_ :
  case _TC_ :
  case _YZX_ :
  case _ZXY_ :
    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate output image\n", 0);
    }
    break;
  default :
    VT_ErrorParse( "unable to deal with such transformation (2)\n", 0);
  }


#define TRANSFOIMAGE( TYPE ) {           \
  TYPE ***theBuf=(TYPE ***)image->array; \
  TYPE ***resBuf=(TYPE ***)NULL;         \
  TYPE tmp;                              \
  switch ( par.typeTransformation ) {    \
  case _SX_ :                            \
    for ( z=0; z<(int)image->dim.z; z ++ )    \
    for ( y=0; y<(int)image->dim.y/2; y ++ )  \
    for ( x=0; x<(int)image->dim.x; x ++ ) {	 \
      tmp = theBuf[z][y][x];             \
      theBuf[z][y][x] = theBuf[z][image->dim.y-1-y][x]; \
      theBuf[z][image->dim.y-1-y][x] = tmp; \
    }                                    \
    break;                               \
  case _SY_ :                            \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x/2; x++ ) { \
      tmp = theBuf[z][y][x];             \
      theBuf[z][y][x] = theBuf[z][y][image->dim.x-1-x]; \
      theBuf[z][y][image->dim.x-1-x] = tmp; \
    }                                    \
    break;                               \
  case _SZ_ :                            \
    for ( z=0; z<(int)image->dim.z/2; z++ )   \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      tmp = theBuf[z][y][x];             \
      theBuf[z][y][x] = theBuf[image->dim.z-1-z][y][x]; \
      theBuf[image->dim.z-1-z][y][x] = tmp; \
    }                                    \
    break;                               \
  case _TB_ :                            \
    for ( z=0; z<(int)image->dim.z; z++ ) {   \
      for ( y=0; y<(int)image->dim.y/2; y++ ) \
      for ( x=0; x<(int)image->dim.x; x++ ) { \
        tmp = theBuf[z][y][x];           \
        theBuf[z][y][x] = theBuf[z][image->dim.y-1-y][image->dim.x-1-x]; \
        theBuf[z][image->dim.y-1-y][image->dim.x-1-x] = tmp; \
      }                                  \
      if ( (image->dim.y+1)/2 > image->dim.y/2 ) { \
        for ( x=0; x<(int)image->dim.x/2; x++ ) {       \
          tmp = theBuf[z][image->dim.y/2][x];      \
          theBuf[z][image->dim.y/2][x] = theBuf[z][image->dim.y/2][image->dim.x-1-x]; \
          theBuf[z][image->dim.y/2][image->dim.x-1-x] = tmp; \
        }                                \
      }                                  \
    }                                    \
    break;                               \
  case _DA_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[z][x][y] = theBuf[z][y][x]; \
    }                                    \
    break;                               \
  case _DB_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[z][x][y] = theBuf[z][image->dim.y-1-y][image->dim.x-1-x]; \
    }                                    \
    break;                               \
  case _DC_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[y][z][x] = theBuf[z][y][x]; \
    }                                    \
    break;                               \
  case _DD_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[y][z][x] = theBuf[image->dim.z-1-z][image->dim.y-1-y][x]; \
    }                                    \
    break;                               \
  case _DE_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[x][y][z] = theBuf[z][y][x]; \
    }                                    \
    break;                               \
  case _DF_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[x][y][z] = theBuf[image->dim.z-1-z][y][image->dim.x-1-x]; \
    }                                    \
    break;                               \
  case _TA_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[z][x][image->dim.y-1-y] = theBuf[z][y][x]; \
    }                                    \
    break;                               \
  case _TC_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[z][image->dim.x-1-x][y] = theBuf[z][y][x]; \
    }                                    \
    break;                               \
  case _YZX_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[x][z][y] = theBuf[z][y][x]; \
    }                                    \
    break;                               \
  case _ZXY_ :                            \
    resBuf = (TYPE ***)imres.array;      \
    for ( z=0; z<(int)image->dim.z; z++ )     \
    for ( y=0; y<(int)image->dim.y; y++ )     \
    for ( x=0; x<(int)image->dim.x; x++ ) {   \
      resBuf[y][x][z] = theBuf[z][y][x]; \
    }                                    \
    break;                               \
  default :                              \
    VT_ErrorParse( "unable to deal with such transformation (3)\n", 0); \
  }                                      \
}

  
  switch ( image->type ) {
  case UCHAR :
    TRANSFOIMAGE( u8 )
    break;
  case USHORT :
    TRANSFOIMAGE( u16 )
    break;
  case SSHORT :
    TRANSFOIMAGE( s16 )
    break;
  case FLOAT :
    TRANSFOIMAGE( r32 )
    break;
  default :
      VT_ErrorParse( "unable to deal with such image type\n", 0);
  }


  switch ( par.typeTransformation ) {
  case _SX_ :
  case _SY_ :
  case _SZ_ :
  case _TB_ :
    break;
  case _DA_ :
  case _DB_ :
  case _TA_ :
  case _TC_ : 
    imres.siz.x = image->siz.y;
    imres.siz.y = image->siz.x;
    imres.siz.z = image->siz.z;
    break;
  case _DC_ :
  case _DD_ :
    imres.siz.x = image->siz.x;
    imres.siz.y = image->siz.z;
    imres.siz.z = image->siz.y;
    break;
  case _DE_ :
  case _DF_ :
    imres.siz.x = image->siz.z;
    imres.siz.y = image->siz.y;
    imres.siz.z = image->siz.x;
     break;
  case _YZX_ : 
    imres.siz.x = image->siz.y;
    imres.siz.y = image->siz.z;
    imres.siz.z = image->siz.x;
    break;
  case _ZXY_ : 
    imres.siz.x = image->siz.z;
    imres.siz.y = image->siz.x;
    imres.siz.z = image->siz.y;
    break;
  default :
    VT_ErrorParse( "unable to deal with such transformation (4)\n", 0);
  }


  switch ( par.typeTransformation ) {
  case _SX_ :
  case _SY_ :
  case _SZ_ :
  case _TB_ :
    if ( VT_WriteInrimage( image ) == -1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output image\n", 0);
    }
    break;
  case _DA_ :
  case _DB_ :
  case _DC_ :
  case _DD_ :
  case _DE_ :
  case _DF_ :
  case _TA_ :
  case _TC_ : 
  case _YZX_ : 
  case _ZXY_ : 
    if ( VT_WriteInrimage( &imres ) == -1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output image\n", 0);
    }
    VT_FreeImage( &imres );
    break;
  default :
    VT_ErrorParse( "unable to deal with such transformation (5)\n", 0);
  }

  
  /*--- liberations memoires ---*/
  VT_FreeImage( &imres );
  VT_Free( (void**)&image );
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


      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }


      /*--- lecture du type de l'image de sortie ---*/
      else if ( strcmp ( argv[i], "-sx" ) == 0 ) {
	par->typeTransformation = _SX_;
      }
      else if ( strcmp ( argv[i], "-sy" ) == 0 ) {
	par->typeTransformation = _SY_;
      }
      else if ( strcmp ( argv[i], "-sz" ) == 0 ) {
	par->typeTransformation = _SZ_;
      }
      else if ( strcmp ( argv[i], "-da" ) == 0 ) {
	par->typeTransformation = _DA_;
      }
      else if ( strcmp ( argv[i], "-db" ) == 0 ) {
	par->typeTransformation = _DB_;
      }
      else if ( strcmp ( argv[i], "-dc" ) == 0 ) {
	par->typeTransformation = _DC_;
      }
      else if ( strcmp ( argv[i], "-dd" ) == 0 ) {
	par->typeTransformation = _DD_;
      }
      else if ( strcmp ( argv[i], "-de" ) == 0 ) {
	par->typeTransformation = _DE_;
      }
      else if ( strcmp ( argv[i], "-df" ) == 0 ) {
	par->typeTransformation = _DF_;
      }
      else if ( strcmp ( argv[i], "-ta" ) == 0 ) {
	par->typeTransformation = _TA_;
      }
      else if ( strcmp ( argv[i], "-tb" ) == 0 ) {
	par->typeTransformation = _TB_;
      }
      else if ( strcmp ( argv[i], "-tc" ) == 0 ) {
	par->typeTransformation = _TC_;
      }
      else if ( strcmp ( argv[i], "-yzx" ) == 0 ) {
	par->typeTransformation = _YZX_;
      }
      else if ( strcmp ( argv[i], "-zxy" ) == 0 ) {
	par->typeTransformation = _ZXY_;
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
  par->typeTransformation = _NULL_;
}
