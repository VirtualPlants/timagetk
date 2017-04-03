#include <vt_common.h>

#define _SOUSTRACTION_ 1 
#define _POSITIVE_SOUSTRACTION_ 2

typedef struct local_par {
  vt_names names;
  char name_mask[STRINGLENGTH];
  int is_mask;
  char name_ref[STRINGLENGTH];
  int is_ref;
  int type_computation;
  double coeff;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-mask image-mask]\n\
\t [-sub] [-pos] [-coeff %lf (=0.0)]\n\
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
  vt_image *image, imres, *imask, *iref;
  
  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  if ( image->type != UCHAR ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to deal with such image type\n", 0);
  }
  
  /* mask ? */
  if ( par.is_mask == 1 ) {
    imask = _VT_Inrimage( par.name_mask );
    if ( imask == (vt_image*)NULL ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read mask image\n", 0);
    }
    if ( imask->type != UCHAR ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( imask );
      VT_Free( (void**)&imask );
      VT_ErrorParse("unable to deal with such mask image type\n", 0);
    }
  }

  /* ref ? */
  if ( par.is_ref == 1 ) {
    iref = _VT_Inrimage( par.name_ref );
    if ( iref == (vt_image*)NULL ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read ref image\n", 0);
    }
    if ( iref->type != UCHAR ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( iref );
      VT_Free( (void**)&iref );
      VT_ErrorParse("unable to deal with such ref image type\n", 0);
    }
  }

  

  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );
  
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, SSHORT );
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_FreeImage( imask );
    VT_Free( (void**)&imask );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
  
  if ( par.is_ref == 1 ) {
    register int x, y, z;
    register u8 ***in = (u8 ***)(image->array);
    register s16 ***out = (s16 ***)(imres.array);
    register u8 ***mask = (u8 ***)NULL;
    register u8 ***ref = (u8 ***)(iref->array);

    if ( par.is_mask == 1 ) {
      mask = (u8 ***)(imask->array);
    }

    switch ( par.type_computation ) {
    case _POSITIVE_SOUSTRACTION_ :
      for ( z =0; z < image->dim.z; z ++ ) 
      for ( y =0; y < image->dim.y; y ++ ) 
      for ( x =0; x < image->dim.x; x ++ ) {
	out[z][y][x] = 0.0;
	if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	if ( in[z][y][x] > ref[0][y][x] )
	  out[z][y][x] = (float)in[z][y][x] - (float)ref[0][y][x];
	if ( par.coeff > 0.0 ) out[z][y][x] += par.coeff * out[z-1][y][x];
      }
      break;
    case _SOUSTRACTION_ :
    default :
      for ( z =0; z < image->dim.z; z ++ ) 
      for ( y =0; y < image->dim.y; y ++ ) 
      for ( x =0; x < image->dim.x; x ++ ) {
	out[z][y][x] = 0.0;
	if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	out[z][y][x] = (float)in[z][y][x] - (float)ref[0][y][x];
	if ( par.coeff > 0.0 ) out[z][y][x] += par.coeff * out[z-1][y][x];
      }
    }
  
  } else {
    switch ( par.type_computation ) {
    case _POSITIVE_SOUSTRACTION_ :
      {
	register int x, y, z;
	register u8 ***in = (u8 ***)(image->array);
	register s16 ***out = (s16 ***)(imres.array);
	register u8 ***mask = (u8 ***)NULL;
	
	if ( par.is_mask == 1 ) {
	  mask = (u8 ***)(imask->array);
	}
	
      /* premier plan */
	for ( y =0; y < image->dim.y; y ++ ) 
	  for ( x =0; x < image->dim.x; x ++ ) 
	    out[0][y][x] = 0.0;
	
	/* autres plans */
	for ( z =1; z < image->dim.z; z ++ ) 
	  for ( y =0; y < image->dim.y; y ++ ) 
	    for ( x =0; x < image->dim.x; x ++ ) {
	      out[z][y][x] = 0.0;
	      if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	      if ( in[z][y][x] > in[z-1][y][x] )
		out[z][y][x] = (float)in[z][y][x] - (float)in[z-1][y][x];
	      if ( par.coeff > 0.0 ) out[z][y][x] += par.coeff * out[z-1][y][x];
	    }
      }
    break;
    case _SOUSTRACTION_ :
    default :
      {
	register int x, y, z;
	register u8 ***in = (u8 ***)(image->array);
	register s16 ***out = (s16 ***)(imres.array);
	register u8 ***mask = (u8 ***)NULL;
	
	if ( par.is_mask == 1 ) {
	  mask = (u8 ***)(imask->array);
	}
	
	/* premier plan */
	for ( y =0; y < image->dim.y; y ++ ) 
	  for ( x =0; x < image->dim.x; x ++ ) 
	    out[0][y][x] = 0.0;
	
	/* autres plans */
	for ( z =1; z < image->dim.z; z ++ ) 
	  for ( y =0; y < image->dim.y; y ++ ) 
	    for ( x =0; x < image->dim.x; x ++ ) {
	      out[z][y][x] = 0.0;
	      if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	      out[z][y][x] = (float)in[z][y][x] - (float)in[z-1][y][x];
	      if ( par.coeff > 0.0 ) out[z][y][x] += par.coeff * out[z-1][y][x];
	    }
      }
    }
  }

  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( image );
    VT_FreeImage( &imres );
    VT_Free( (void**)&image );
    if ( par.is_mask == 1 ) {
      VT_FreeImage( imask );
      VT_Free( (void**)&imask );
    }
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( image );
  VT_FreeImage( &imres );
  VT_Free( (void**)&image );
  if ( par.is_mask == 1 ) {
    VT_FreeImage( imask );
    VT_Free( (void**)&imask );
  }
  return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
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
      /*--- mask ---*/
      else if ( strcmp ( argv[i], "-mask" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -mask...\n", 0 );
	strncpy( par->name_mask, argv[i], STRINGLENGTH );  
	par->is_mask = 1;
      }
      /*--- ref ---*/
      else if ( strcmp ( argv[i], "-ref" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -ref...\n", 0 );
	strncpy( par->name_ref, argv[i], STRINGLENGTH );  
	par->is_ref = 1;
      }
      /*--- ---*/
      else if ( strcmp ( argv[i], "-sub" ) == 0 ) {
	par->type_computation = _SOUSTRACTION_;
      }
      else if ( strcmp ( argv[i], "-pos" ) == 0 ) {
	par->type_computation = _POSITIVE_SOUSTRACTION_;
      }
      /*--- coeff ---*/
      else if ( strcmp ( argv[i], "-coeff" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -coeff...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->coeff) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -coeff...\n", 0 );
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
  par->type_computation = TYPE_UNKNOWN;
  par->is_mask = 0;
  par->is_ref = 0;
  par->coeff = 0.0;
}
