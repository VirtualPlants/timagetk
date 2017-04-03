/*************************************************************************
 * minimum.c -
 *
 * $Id: Arit.c,v 1.1 2006/04/14 08:37:11 greg Exp $
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

#include <pixel-operation.h>

#include <vt_common.h>

typedef enum {
  _NULL_,
  _ADD_,
  _SUB_,
  _MUL_,
  _DIV_,
  _MIN_,
  _MAX_
} enumOperation;

typedef enum {
  _IMAGE_,
  _IMAGE_WITH_IMAGE_,
  _IMAGE_WITH_SCALAR_
} enumTypeOperation;

typedef struct local_par {
  vt_names names;
  bufferType type;
  enumOperation operation;
} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );

static int _isNumber( char *s, double *d );



static char *usage = "image-in [image-in2] image-out\n\
\t [-add|-sub|-mul|-div|-min|-max]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s : short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.1 $ $Date: 2006/04/14 08:37:11 $ $Author: greg $\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image = NULL, *imaux = NULL, imtmp, *imres = NULL;
  double scalar = 0;
  enumTypeOperation typeOperation = _IMAGE_WITH_IMAGE_;
  int theDim[3] = {0,0,0};

  /*
  float *tmpbuf = NULL, *buf;
  int z, i, s;
  int opwithnb = 0;
  double nb = 0.0;
  */

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  

  /* reading images
   */

  if ( _isNumber( par.names.in, &scalar ) == 1 ) {

    image = _VT_Inrimage( par.names.ext );
    if ( image == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read input image\n", 0);
    typeOperation = _IMAGE_WITH_SCALAR_;

    if ( _VT_VERBOSE_ )
        fprintf( stderr, "   %s: scalar %f with image\n", program, scalar );
  }
  else if ( _isNumber( par.names.ext, &scalar ) == 1 ) {

    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read input image\n", 0);
    typeOperation = _IMAGE_WITH_SCALAR_;

    if ( _VT_VERBOSE_ )
        fprintf( stderr, "   %s: image with scalar %f\n", program, scalar );

  }
  else {
    
    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read input image\n", 0);
    imaux = _VT_Inrimage( par.names.ext );
    if ( imaux == (vt_image*)NULL ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read second input image\n", 0);
    }

    if ( par.names.inv == 1 )  VT_InverseImage( imaux );
    if ( par.names.swap == 1 ) VT_SwapImage( imaux );

    if ( image->dim.x != imaux->dim.x || image->dim.y != imaux->dim.y || 
         image->dim.z != imaux->dim.z || image->dim.v != imaux->dim.v ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      if ( imaux != (vt_image*)NULL ) {
        VT_FreeImage( imaux );
        VT_Free( (void**)&imaux );
      }
      VT_ErrorParse("input images have different dimensions\n", 0);
    }

    typeOperation = _IMAGE_WITH_IMAGE_;
    
  }

  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );

  theDim[0] = image->dim.v * image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;
  

  /* output image
   */
  VT_Image( &imtmp );
  if ( par.type == image->type ) {
    imres = image;
    if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      if ( imaux != NULL ) {
        VT_FreeImage( imaux );
        VT_Free( (void**)&imaux );
      }
      VT_ErrorParse("unable to copy output image name (1)\n", 0);
    }
  }
  else if ( imaux != NULL && par.type == imaux->type ) {
    imres = imaux;
    if ( VT_CopyName( imaux->name, par.names.out ) == 0 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( imaux );
      VT_Free( (void**)&imaux );
      VT_ErrorParse("unable to copy output image name (2)\n", 0);
    }
  }
  else {
    VT_InitFromImage( &imtmp, image, par.names.out, par.type );
    if ( VT_AllocImage( &imtmp ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      if ( imaux != NULL ) {
        VT_FreeImage( imaux );
        VT_Free( (void**)&imaux );
      }
      VT_ErrorParse("unable to allocate output image\n", 0);
    }
    imres = &imtmp;
  }


  



  /* processing
   */

#define _FREEING_IMAGES {           \
  if ( imres == &imtmp )            \
    VT_FreeImage( &imtmp );         \
  if ( imaux != (vt_image*)NULL ) { \
    VT_FreeImage( imaux );          \
    VT_Free( (void**)&imaux );      \
  }                                 \
  VT_FreeImage( image );            \
  VT_Free( (void**)&image );        \
}


  switch ( typeOperation ) {

  default :
    _FREEING_IMAGES;
    VT_ErrorParse("such operation type not handled yet\n", 0);


  case _IMAGE_WITH_IMAGE_ :
    switch ( par.operation ) {
    default :
      _FREEING_IMAGES;
      VT_ErrorParse("such operation not handled yet\n", 0);
    case _ADD_ :
      if ( addImages( image->buf, image->type, imaux->buf, imaux->type,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _SUB_ :
      if ( subtractImages( image->buf, image->type, imaux->buf, imaux->type,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _MUL_ :
      if ( multiplyImages( image->buf, image->type, imaux->buf, imaux->type,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _DIV_ :
      if ( divideImages( image->buf, image->type, imaux->buf, imaux->type,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _MIN_ :
      if ( minImages( image->buf, image->type, imaux->buf, imaux->type,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _MAX_ :
      if ( maxImages( image->buf, image->type, imaux->buf, imaux->type,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    }
    break;

  case _IMAGE_WITH_SCALAR_ :
    switch ( par.operation ) {
    default :
      _FREEING_IMAGES;
      VT_ErrorParse("such operation not handled yet\n", 0);
    case _ADD_ :
      if ( addImageDouble( image->buf, image->type, scalar,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _SUB_ :
      if ( subtractImageDouble( image->buf, image->type, scalar,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _MUL_ :
      if ( multiplyImageDouble( image->buf, image->type, scalar,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _DIV_ :
      if ( divideImageDouble( image->buf, image->type, scalar,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _MIN_ :
      if ( minImageDouble( image->buf, image->type, scalar,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    case _MAX_ :
      if ( maxImageDouble( image->buf, image->type, scalar,
                      imres->buf, imres->type, theDim ) != 1 ) {
        _FREEING_IMAGES;
        VT_ErrorParse("error when computing \n", 0);
      }
      break;
    }
    break;

  }

   


  
  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( imres ) == -1 ) {
    _FREEING_IMAGES;
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  _FREEING_IMAGES;
  return( 1 );
}








static void VT_Parse( int argc, 
                      char *argv[],
                      local_par *par )
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
      /*--- arguments generaux ---*/
      else if ( strcmp ( argv[i], "-help" ) == 0 ) {
        VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
        if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
        else _VT_VERBOSE_  ++;
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


      else if ( strcmp ( argv[i], "-add" ) == 0 ) {
        par->operation = _ADD_;
      }
      else if ( strcmp ( argv[i], "-sub" ) == 0 
                || (strcmp ( argv[i], "-so" ) == 0 && argv[3] == '\0') ) {
        par->operation = _SUB_;
      }
      else if ( strcmp ( argv[i], "-mul" ) == 0 ) {
        par->operation = _MUL_;
      }      
      else if ( strcmp ( argv[i], "-div" ) == 0 ) {
        par->operation = _DIV_;
      }
      else if ( strcmp ( argv[i], "-min" ) == 0 ) {
        par->operation = _MIN_;
      }
      else if ( strcmp ( argv[i], "-max" ) == 0 ) {
        par->operation = _MAX_;
      }

      /*--- lecture du type de l'image de sortie ---*/
      else if ( strcmp ( argv[i], "-r" ) == 0 ) {
        r = 1;
      }
      else if ( strcmp ( argv[i], "-s" ) == 0 ) {
        s = 1;
      }
      else if ( strcmp ( argv[i], "-o" ) == 0 ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -o...\n", 0 );
        status = sscanf( argv[i],"%d",&o );
        if ( status <= 0 ) VT_ErrorParse( "parsing -o...\n", 0 );
      }
      /*--- option inconnue ---*/
      else {
        if ( argv[i][1] == '0' || argv[i][1] == '1' || argv[i][1] == '2' || argv[i][1] == '3' || argv[i][1] == '4'
             || argv[i][1] == '5' || argv[i][1] == '6' || argv[i][1] == '7' || argv[i][1] == '8' || argv[i][1] == '9' ) {
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
        else {
            sprintf(text,"unknown option %s\n",argv[i]);
            VT_ErrorParse(text, 0);
        }
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
  switch ( par->operation ) {
  case _ADD_ :
  case _SUB_ :
  case _MUL_ :
  case _DIV_ :
  case _MIN_ :
  case _MAX_ :
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

  default :
    VT_ErrorParse("no choice for the arithmetic operation\n", 0 );
  }

  /*--- type de l'image resultat ---*/
  if ( (o == 1) && (s == 1) && (r == 0) )  par->type = SCHAR;
  if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
  if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
  if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
  if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
  if ( (o == 0) && (s == 0) && (r == 1) )  par->type = FLOAT;
  /* if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program); */
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
  par->type = FLOAT;
  par->operation = _NULL_;
}



static int _isNumber( char *s, double *d )
{
  int f=0;
  int i, l;
  int p=0;
  
  if ( s == (char*)NULL ) return( 0 );
  l =  strlen( s );
  if ( l == 0 ) return( 0 );
  if ( s[0] == '-' ) {
    if ( l == 1 ) return( 0 );
    f = 1;
  }

  for ( i=f; i<l; i++ ) {
    if ( s[i] >= '0' && s[i] <= '9' ) continue;
    if ( s[i] == '.' ) {
      p++;
      if ( p > 1 ) return( 0 );
      continue;
    }
    return( 0 );
  }

  if ( sscanf( s, "%lf", d ) != 1 ) return( 0 );
  
  return( 1 );

}
