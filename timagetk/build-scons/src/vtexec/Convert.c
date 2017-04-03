#include <vt_common.h>
#include <iopnm.h>
#include <iobmp.h>

typedef enum {
  _UNKNOWN_=0,
  _INR_=1,
  _PPM_=2,
  _PNM_=2,
  _BMP_=3
} enumFileType;

typedef enum {
  _GREY_,
  _MAX_,
  _NONE_
} enumConvertType;

typedef enum {
  _PLANES_,
  _VECT_
} enumColorType;


typedef struct local_par {
  vt_names names;
  enumFileType inputType;
  enumFileType outputType;
  enumConvertType conversion;
  enumColorType color;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out]\n\
\t [-inr2ppm | -ppm2inr | -bmp2inr | -inr2bmp] [-grey|-max]\n\
\t [-vect|-planes]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\n\
-grey: compute luminosity\n\
-max:  project (along Z) the maximum intensity\n\
-planes:  create 3 slices instead of a vectorial image\n\
-vect: create a vectorial image instead of 3 slices\n\
\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.5 $ $Date: 2006/04/14 08:37:38 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image image, imres, imwrite, *imageToBeWritten;

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );

  
  /* faire les tests sur les noms */
  if ( par.inputType == _UNKNOWN_ ) {

    if ( strcmp( (par.names.in + strlen( par.names.in ) - 3), "bmp" ) == 0 ) {
      par.inputType = _BMP_;
    } 
    else if ( strcmp( (par.names.in + strlen( par.names.in ) - 3), "BMP" ) == 0 ) {
      par.inputType = _BMP_;
    } 
    else if ( strcmp( (par.names.in + strlen( par.names.in ) - 3), "ppm" ) == 0 ) {
      par.inputType = _PPM_;
    } 
    else if ( strcmp( (par.names.in + strlen( par.names.in ) - 3), "pgm" ) == 0 ) {
      par.inputType = _PPM_;
    } 
    else if ( strcmp( (par.names.in + strlen( par.names.in ) - 3), "pnm" ) == 0 ) {
      par.inputType = _PPM_;
    } 
    else if ( strcmp( (par.names.in + strlen( par.names.in ) - 3), "inr" ) == 0 ) {
      par.inputType = _INR_;
    } 

  }
  
  if ( par.outputType == _UNKNOWN_ ) {

    if ( strcmp( (par.names.out + strlen( par.names.out ) - 3), "bmp" ) == 0 ) {
      par.outputType = _BMP_;
    } 
    else if ( strcmp( (par.names.out + strlen( par.names.out ) - 3), "BMP" ) == 0 ) {
      par.outputType = _BMP_;
    } 
    else if ( strcmp( (par.names.out + strlen( par.names.out ) - 3), "ppm" ) == 0
	      || strcmp( (par.names.out + strlen( par.names.out ) - 6), "ppm.gz" ) == 0 ) {
      par.outputType = _PPM_;
    } 
    else if ( strcmp( (par.names.out + strlen( par.names.out ) - 3), "pgm" ) == 0
	      || strcmp( (par.names.out + strlen( par.names.out ) - 6), "pgm.gz" ) == 0 ) {
      par.outputType = _PPM_;
    } 
    else if ( strcmp( (par.names.out + strlen( par.names.out ) - 3), "pnm" ) == 0
	      || strcmp( (par.names.out + strlen( par.names.out ) - 6), "pnm.gz" ) == 0 ) {
      par.outputType = _PPM_;
    } 
    else if ( strcmp( (par.names.out + strlen( par.names.out ) - 3), "inr" ) == 0
	      || strcmp( (par.names.out + strlen( par.names.out ) - 6), "inr.gz" ) == 0 ) {
      par.outputType = _INR_;
    }
    
  }
  


  (void)VT_ReadInrimage( &image, par.names.in );
  imageToBeWritten = &image;


  /* some processing
   */

  
  switch ( par.conversion ) {
  
  default :
    break;
  
  case _MAX_ :

    if ( image.dim.x != 1 ) {
      VT_ErrorParse( "can not convert to max from vectorial image\n", 0 );
    }


    VT_InitImage( &imres, par.names.out, image.dim.x, image.dim.y, 
		  1, image.type );
    (void)VT_AllocImage( &imres );
    
    /* conversion a faire */
    {
      int x, y, z, c;
      switch( image.type ) {
      case UCHAR :
	{
	  unsigned char ***theBuf = (unsigned char ***)image.array;
	  unsigned char ***resBuf = (unsigned char ***)imres.array;
    for (y=0; y<(int)image.dim.y; y++)
      for (x=0; x<(int)image.dim.x; x++) {
	      c = theBuf[0][y][x];
        for (z=1; z<(int)image.dim.z; z++)
		if ( c < theBuf[z][y][x] )  c = theBuf[z][y][x];
	      resBuf[0][y][x] = c;
	    }
	}
	break;
      default :
	VT_ErrorParse( "can not convert to max with such image type\n", 0 );
      }
      
      imageToBeWritten = &imres;
      VT_FreeImage( &image );
    }
    break;

  case _GREY_ :
  
    if ( (image.dim.z != 3 || image.dim.v != 1) && 
	 (image.dim.z != 1 || image.dim.v != 3) ) {
      fprintf( stderr, " do not recognize a color input image (dim Z = %lu, dim V = %lu)\n", 
	       image.dim.z, image.dim.v );
      imageToBeWritten = &image;
      break;
    }

    VT_InitImage( &imres, par.names.out, image.dim.x, image.dim.y, 
		  1, image.type );
    (void)VT_AllocImage( &imres );

    if ( image.dim.z == 3 && image.dim.v == 1 ) {
      int x, y, c;
      switch( image.type ) {
      case UCHAR :
	{
	  unsigned char ***theBuf = (unsigned char ***)image.array;
	  unsigned char ***resBuf = (unsigned char ***)imres.array;
    for (y=0; y<(int)image.dim.y; y++)
      for (x=0; x<(int)image.dim.x; x++) {
	      c = (int)( 0.2125 * (double)theBuf[0][y][x] +
			 0.7154 * (double)theBuf[1][y][x] +
			 0.0721 * (double)theBuf[2][y][x] + 0.5 );
	      if ( c > 255 )    resBuf[0][y][x] = 255;
	      else if ( c < 0 ) resBuf[0][y][x] = 0;
	      else              resBuf[0][y][x] = c;
	    }
	}
	break;
      default :
	VT_ErrorParse( "can not convert to grey such 3-planes image type\n", 0 );
      }

      imageToBeWritten = &imres;
      VT_FreeImage( &image );
    }

    if ( image.dim.z == 1 && image.dim.v == 3 ) {
      int x, y, c;
      switch( image.type ) {
      case UCHAR :
	{
	  unsigned char ***theBuf = (unsigned char ***)image.array;
	  unsigned char ***resBuf = (unsigned char ***)imres.array;
    for (y=0; y<(int)image.dim.y; y++)
      for (x=0; x<(int)image.dim.x; x++) {
	      c = (int)( 0.2125 * (double)theBuf[0][y][3*x] +
			 0.7154 * (double)theBuf[0][y][3*x+1] +
			 0.0721 * (double)theBuf[0][y][3*x+2] + 0.5 );
	      if ( c > 255 )    resBuf[0][y][x] = 255;
	      else if ( c < 0 ) resBuf[0][y][x] = 0;
	      else              resBuf[0][y][x] = c;
	    }
	}
	break;
      default :
	VT_ErrorParse( "can not convert to grey such vectorial image type\n", 0 );
      }

      imageToBeWritten = &imres;
      VT_FreeImage( &image );
    }

    break;

  }



  /* preparing the output
   */

  switch ( par.color ) {
  default :
    break;
  case _PLANES_ :
    switch ( par.outputType ) {
    default :
    case _INR_ :
      break;
    case _PNM_ :
    case _BMP_ :
      par.color = _VECT_;
    }
    break;
  }


  if ( par.color == _VECT_ &&
       ( imageToBeWritten->dim.v == 1 && imageToBeWritten->dim.z > 1 ) ) {

    VT_InitVImage( &imwrite, par.names.out, imageToBeWritten->dim.z, imageToBeWritten->dim.x, 
		   imageToBeWritten->dim.y, 1, imageToBeWritten->type );
    (void)VT_AllocImage( &imwrite );

    {
      int x, y, z;
      int v = imageToBeWritten->dim.z;
      
      switch( image.type ) {
      case UCHAR :
	{
	  unsigned char ***theBuf = (unsigned char ***)imageToBeWritten->array;
	  unsigned char ***resBuf = (unsigned char ***)imwrite.array;
    for (z=0; z<(int)imageToBeWritten->dim.z; z++)
      for (y=0; y<(int)imageToBeWritten->dim.y; y++)
        for (x=0; x<(int)imageToBeWritten->dim.x; x++) {
		resBuf[0][y][v*x+z] = theBuf[z][y][x];
		resBuf[0][y][v*x+z] = theBuf[z][y][x];
		resBuf[0][y][v*x+z] = theBuf[z][y][x];
	      }
	}
	break;
      default :
	VT_ErrorParse( "can not convert to vectorial image such image type\n", 0 );
      }
    }

    VT_FreeImage( imageToBeWritten );
    imageToBeWritten = &imwrite;

  }


  if ( par.color == _PLANES_ &&
       ( imageToBeWritten->dim.z == 1 && imageToBeWritten->dim.v > 1 ) ) {

    VT_InitVImage( &imwrite, par.names.out, 1, imageToBeWritten->dim.x, 
		   imageToBeWritten->dim.y, imageToBeWritten->dim.v, imageToBeWritten->type );
    (void)VT_AllocImage( &imwrite );

    {
      int x, y, v;
      int z = imageToBeWritten->dim.v;
      
      switch( image.type ) {
      case UCHAR :
	{
	  unsigned char ***theBuf = (unsigned char ***)imageToBeWritten->array;
	  unsigned char ***resBuf = (unsigned char ***)imwrite.array;
    for (y=0; y<(int)imageToBeWritten->dim.y; y++)
      for (x=0; x<(int)imageToBeWritten->dim.x; x++)
        for (v=0; v<(int)imageToBeWritten->dim.v; v++) {
		resBuf[v][y][x] = theBuf[0][y][z*x+v];
		resBuf[v][y][x] = theBuf[0][y][z*x+v];
		resBuf[v][y][x] = theBuf[0][y][z*x+v];
	      }
	}
	break;
      default :
	VT_ErrorParse( "can not convert to slices image such image type\n", 0 );
      }
    }

    VT_FreeImage( imageToBeWritten );
    imageToBeWritten = &imwrite;

  }


  (void)VT_CopyName( imageToBeWritten->name, par.names.out );


  /* ecriture 
   */

  (void)VT_WriteInrimage( imageToBeWritten );
  VT_FreeImage( imageToBeWritten );

  exit( 0 );
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

      else if ( strcmp ( argv[i], "-inr2ppm" ) == 0 ) {
	par->inputType = _INR_;
	par->outputType = _PPM_;
      }
      else if ( strcmp ( argv[i], "-ppm2inr" ) == 0 ) {
	par->inputType = _PPM_;
	par->outputType = _INR_;
      }
      else if ( strcmp ( argv[i], "-inr2bmp" ) == 0 ) {
	par->inputType = _INR_;
	par->outputType = _BMP_;
      }
      else if ( strcmp ( argv[i], "-bmp2inr" ) == 0 ) {
	par->inputType = _BMP_;
	par->outputType = _INR_;
      }

      else if ( strcmp ( argv[i], "-grey" ) == 0 ) {
	par->conversion = _GREY_;
      }
      else if ( strcmp ( argv[i], "-max" ) == 0 ) {
	par->conversion = _MAX_;
      }

      else if ( strcmp ( argv[i], "-planes" ) == 0 ) {
	par->color = _PLANES_;
      }
      else if ( strcmp ( argv[i], "-vect" ) == 0 ) {
	par->color = _VECT_;
      }
     

      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
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
  par->inputType = _UNKNOWN_;
  par->outputType = _UNKNOWN_;
  par->conversion = _NONE_;
  par->color = _VECT_;
}
