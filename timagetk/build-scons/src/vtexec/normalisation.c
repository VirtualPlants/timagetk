/*************************************************************************
 * minimum.c -
 *
 * $Id: normalisation.c,v 1.2 2003/07/04 08:16:07 greg Exp $
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
#include <math.h>
#include <vt_common.h>
#include <convolution.h>

typedef struct local_par {
  double sigma;
  int borderLengths[3];
  vt_names names;
  int type;
} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );





static char *usage = "[image-in] [image-out]\n\
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
 $Revision: 1.2 $ $Date: 2003/07/04 08:16:07 $ $Author: greg $\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imdiff, imres;
  int theDims[3];
  double theCoefs[3];
  size_t i;
  double *tmp1Buf;
  double *tmp2Buf;
  double *mask[3] = { NULL, NULL, NULL };
  int maskLength[3];

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  fprintf( stderr, "type of '%s' is %d\n", par.names.in, image->type );
  
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  VT_InitFromImage( &imres, image, par.names.out, DOUBLE );
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
  VT_Image( &imdiff );
  VT_InitFromImage( &imdiff, image, par.names.out, DOUBLE );
  if ( VT_AllocImage( &imdiff ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate auxiliary image\n", 0);
  }
  
  theDims[0] = image->dim.x;
  theDims[1] = image->dim.y;
  theDims[2] = image->dim.z;

  theCoefs[0] = par.sigma / image->siz.x;
  theCoefs[1] = par.sigma / image->siz.y;
  theCoefs[2] = par.sigma / image->siz.z;

  for ( i=0; i<3; i++ ) {
    maskLength[i] = 1 + 2 * (int)(2.5 * theCoefs[i] + 0.5);
    mask[i] = _buildGaussianMask( theCoefs[i], maskLength[i] );
    if ( mask[i] == NULL ) 
       VT_ErrorParse("unable to allocate convolution mask\n", 0 );
  }

  for ( i=0; i<3; i++ ) {
    if ( par.borderLengths[i] < 2.0 * theCoefs[i] ) 
      par.borderLengths[i] =(int)(2.0 * theCoefs[i] + 0.5);
  }

  fprintf( stderr, "... filter input image with sigma = (%f %f %f)\n",
	   theCoefs[0], theCoefs[1], theCoefs[2] );
  fprintf( stderr, " \t \t length = (%d %d %d)\n", 
	   maskLength[0], maskLength[1], maskLength[2] );
  if ( SeparableConvolution( image->buf, image->type, 
				imres.buf, imres.type, theDims,
				par.borderLengths, mask, maskLength ) != 1 ) {
    VT_ErrorParse("unable to filter input image\n", 0);
  }
  
  fprintf( stderr, "... substract filtered image from input image\n" );
  tmp1Buf = (double*)imres.buf;
  tmp2Buf = (double*)imdiff.buf;
  switch( image->type ) {
  default :
    VT_ErrorParse("image type not handled in switch\n", 0);
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)image->buf;
      for ( i=0; i<imres.dim.x*imres.dim.y*imres.dim.z; i++ )
	tmp2Buf[i] = theBuf[i] - tmp1Buf[i];
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)image->buf;
      for ( i=0; i<imres.dim.x*imres.dim.y*imres.dim.z; i++ )
	tmp2Buf[i] = theBuf[i] - tmp1Buf[i];
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)image->buf;
      for ( i=0; i<imres.dim.x*imres.dim.y*imres.dim.z; i++ )
	tmp2Buf[i] = theBuf[i] - tmp1Buf[i];
    }
    break;
  }

  VT_FreeImage( image );
  VT_Free( (void**)&image );




  if ( 0 ) {
    VT_FreeImage( &imres );
    VT_Image( &imres );
    VT_InitFromImage( &imres, &imdiff, par.names.out, FLOAT );
    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate output image\n", 0);
    }
    
    if ( VT_CopyImage( &imdiff, &imres ) != 1 ) {
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to copy input image\n", 0);
    }
    
    VT_FreeImage( &imdiff );
    
    
    /*--- ecriture de l'image resultat ---*/
    if ( VT_WriteInrimage( &imres ) == -1 ) {
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to write output image\n", 0);
    }
    exit( 0 );
  }









  fprintf( stderr, "... square substracted image\n" );
  tmp1Buf = (double*)imres.buf;
  tmp2Buf = (double*)imdiff.buf;
  for ( i=0; i<imres.dim.x*imres.dim.y*imres.dim.z; i++ )
    tmp1Buf[i] = tmp2Buf[i] * tmp2Buf[i];
  
  fprintf( stderr, "... filter square substracted image with sigma = (%f %f %f)\n",
	   theCoefs[0], theCoefs[1], theCoefs[2] );
  if ( SeparableConvolution( imres.buf, imres.type,
				imres.buf, imres.type, theDims,
				par.borderLengths, mask, maskLength ) != 1 ) {
    VT_ErrorParse("unable to filter substracted image\n", 0);
  }
  
  fprintf( stderr, "... square root\n" );
  tmp1Buf = (double*)imres.buf;
  for ( i=0; i<imres.dim.x*imres.dim.y*imres.dim.z; i++ )
    tmp1Buf[i] = sqrt( tmp1Buf[i] );
  
  tmp1Buf = (double*)imres.buf;
  tmp2Buf = (double*)imdiff.buf;  
  fprintf( stderr, "... dividing\n" );
  for ( i=0; i<imres.dim.x*imres.dim.y*imres.dim.z; i++ ) {
    if ( tmp1Buf[i] > 0.0 ) tmp2Buf[i] = tmp2Buf[i] / tmp1Buf[i];
    else                    tmp2Buf[i] = 0.0;
  }

  VT_FreeImage( &imres );
  VT_Image( &imres );
  VT_InitFromImage( &imres, &imdiff, par.names.out, FLOAT );
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
  
  if ( VT_CopyImage( &imdiff, &imres ) != 1 ) {
    VT_FreeImage( &imres );
    VT_ErrorParse("unable to copy input image\n", 0);
  }

  VT_FreeImage( &imdiff );


  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( &imres );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( &imres );
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
	_VT_VERBOSE_ = 1;
	Convolution_verbose ( );
      }
      else if ( strcmp ( argv[i], "-nv" ) == 0 ) {
	_VT_VERBOSE_ = 0;
	Convolution_noverbose ( );
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

      else if ( strcmp ( argv[i], "-sigma" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -sigma...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->sigma) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -sigma...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-cont" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -cont...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->borderLengths[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -cont...\n", 0 );
	par->borderLengths[2] = par->borderLengths[1] = par->borderLengths[0];
      }

      else if ( strcmp ( argv[i], "-xcont" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -xcont...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->borderLengths[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -xcont...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-ycont" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -ycont...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->borderLengths[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -ycont...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-zcont" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -zcont...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->borderLengths[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -zcont...\n", 0 );
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

  par->borderLengths[0] = 10;
  par->borderLengths[1] = 10;
  par->borderLengths[2] = 10;
  par->sigma = 60.0;

  par->type = TYPE_UNKNOWN;
}
