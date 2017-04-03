/*************************************************************************
 * minimum.c -
 *
 * $Id: detrend.c,v 1.2 2002/09/27 13:29:45 greg Exp $
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
#include <convolution.h>

typedef struct local_par {
  vt_names names;
  int type;
  int fwhm[3];
} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );





static char *usage = "[image-in] [image-out]\n\
\t [-fwhm %d] [-mask %s]\n\
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
 $Revision: 1.2 $ $Date: 2002/09/27 13:29:45 $ $Author: greg $\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imres;
  vt_image *mask, maskres;
  int i, dim[3];
  float *res, *mres;

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
  VT_InitFromImage( &imres, image, par.names.out, FLOAT );

  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }

  fprintf( stderr, "detrend by convolution\n" );

  dim[0] = image->dim.x;
  dim[1] = image->dim.y;
  dim[2] = image->dim.z;
  
  res = (float*)imres.buf;

  if ( par.names.ext[0] == '\0' ) {

    ConvolutionWithFwhm( image->buf, image->type,
			 imres.buf, imres.type, dim, par.fwhm );
    
    switch( image->type ) {
    default :
      VT_ErrorParse(" image type not handled in switch\n", 0 );
    case UCHAR :
      {
	u8 *buf = (u8*)image->buf;
	for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) res[i] = buf[i] - res[i];
      }
      break;
    case USHORT :
      {
	u16 *buf = (u16*)image->buf;
	for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) res[i] = buf[i] - res[i];
      }
      break;
    case FLOAT :
      {
	float *buf = (float*)image->buf;
	for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) res[i] = buf[i] - res[i];
      }
      break;
    case DOUBLE :
      {
	double *buf = (double*)image->buf;
	for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) res[i] = buf[i] - res[i];
      }
      break;
    }

  }
  else {
    
    mask = _VT_Inrimage( par.names.ext );
    if ( mask == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read mask image\n", 0);
    VT_Image( &maskres );
    VT_InitFromImage( &maskres, mask, "mask-conv.inr", FLOAT );
    if ( VT_AllocImage( &maskres ) != 1 )
      VT_ErrorParse("unable to allocate mask output image\n", 0);
    
    dim[0] = mask->dim.x;
    dim[1] = mask->dim.y;
    dim[2] = mask->dim.z;
    
    switch( mask->type ) {
    default :
      VT_ErrorParse(" mask image type not handled in switch\n", 0 );
    case UCHAR :
      {
	u8 *mbuf = (u8*)mask->buf;
	for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	  mbuf[i] = ( mbuf[i] > 0 ) ? 1 : 0;

	switch( image->type ) {
	default :
	  VT_ErrorParse(" image type not handled in switch\n", 0 );
	case UCHAR :
	  {
	    u8 *buf = (u8*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      if ( mbuf[i] <= 0 ) buf[i] = 0;
	  }
	  break;
	case USHORT :
	  {
	    u16 *buf = (u16*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      if ( mbuf[i] <= 0 ) buf[i] = 0;
	  }
	  break;
	case FLOAT :
	  {
	    float *buf = (float*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      if ( mbuf[i] <= 0 ) buf[i] = 0;
	  }
	  break;
	case DOUBLE :
	  {
	    double *buf = (double*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      if ( mbuf[i] <= 0 ) buf[i] = 0;
	  }
	  break;
	}
	
      }
      break;
    }
    
    ConvolutionWithFwhm( image->buf, image->type,
			 imres.buf, imres.type, dim, par.fwhm );
    
    ConvolutionWithFwhm( mask->buf, mask->type,
			 maskres.buf, maskres.type, dim, par.fwhm );

    mres= (float*)maskres.buf;
    
    switch( mask->type ) {
    default :
      VT_ErrorParse(" mask image type not handled in switch\n", 0 );
    case UCHAR :
      {
	u8 *mbuf = (u8*)mask->buf;

	switch( image->type ) {
	default :
	  VT_ErrorParse(" image type not handled in switch\n", 0 );
	case UCHAR :
	  {
	    u8 *buf = (u8*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      res[i] = ( mbuf[i] > 0 ) ? buf[i] - res[i]/mres[i] : 0;
	  }
	  break;
	case USHORT :
	  {
	    u16 *buf = (u16*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      res[i] = ( mbuf[i] > 0 ) ? buf[i] - res[i]/mres[i] : 0;
	  }
	  break;
	case FLOAT :
	  {
	    float *buf = (float*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      res[i] = ( mbuf[i] > 0 ) ? buf[i] - res[i]/mres[i] : 0;
	  }
	  break;
	case DOUBLE :
	  {
	    double *buf = (double*)image->buf;
	    for (i=0; i<dim[0]*dim[1]*dim[2]; i++ ) 
	      res[i] = ( mbuf[i] > 0 ) ? buf[i] - res[i]/mres[i] : 0;
	  }
	  break;
	}
      }
      break;
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



      else if ( strcmp ( argv[i], "-fwhm" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -fwhm...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->fwhm[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -fwhm...\n", 0 );
	par->fwhm[0] = par->fwhm[1] = par->fwhm[2];
      }

      else if ( strcmp ( argv[i], "-xfwhm" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -xfwhm...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->fwhm[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -xfwhm...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-yfwhm" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -yfwhm...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->fwhm[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -yfwhm...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-zfwhm" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -zfwhm...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->fwhm[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -zfwhm...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-mask" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -mask...\n", 0 );
	strncpy( par->names.ext, argv[i], STRINGLENGTH );  
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
  par->type = TYPE_UNKNOWN;
  par->fwhm[0] = par->fwhm[1] = par->fwhm[2] = 10;
}
