#include <vt_common.h>
#include <sour-filter.h>


typedef struct local_par {
  double radius;
  double sigma;
  vt_names names;
  int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] -w image-weights\n\
\t [-r | -radius] %f [-s | -sigma] %f\n\
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
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, *imweights, imres;


  typeMask theMask;
  int theDim[3];
  

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  imweights = _VT_Inrimage( par.names.ext );
  if ( imweights == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read weights image\n", 0);
  
  if ( image->type != FLOAT || imweights->type != FLOAT ) {
    VT_ErrorParse("images should be float\n", 0);
  }
  if ( image->dim.x != imweights->dim.x ||
       image->dim.y != imweights->dim.y ||
       image->dim.z != imweights->dim.z ) {
    VT_ErrorParse("images should have same dimmensions\n", 0);
  }



  /*--- operations eventuelles sur l'image d'entree ---*/
  /*
  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );
  */


  


  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, image->type );
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_FreeImage( imweights );
    VT_Free( (void**)&imweights );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
  

  
  _InitMask( &theMask );
  if ( _CreateGaussianMask( &theMask, par.radius, par.sigma ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_FreeImage( imweights );
    VT_Free( (void**)&imweights );
    VT_ErrorParse("unable to create mask\n", 0);
  }
  theDim[0] = image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;
  (void)_FilterWithMaskAndWeights( (float*)image->buf,
				   (float*)imres.buf,
				   (float*)imweights->buf,
				   theDim,
				   &theMask );
  

  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( &imres );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_FreeImage( imweights );
    VT_Free( (void**)&imweights );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( &imres );
  VT_FreeImage( image );
  VT_Free( (void**)&image );
    VT_FreeImage( imweights );
    VT_Free( (void**)&imweights );
  return( 1 );
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

      
      else if ( strcmp ( argv[i], "-radius" ) == 0 ||
		strcmp ( argv[i], "-r" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -radius...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->radius) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -radius...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-sigma" ) == 0 ||
		strcmp ( argv[i], "-s" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -sigma...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->sigma) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -sigma...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-w" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -w...\n", 0 );
	strncpy( par->names.ext, argv[i], STRINGLENGTH );
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
      else {
	fprintf( stderr, " quid de %s\n",  argv[i] );
	VT_ErrorParse("too much file names when parsing\n", 0 );
      }
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
}
