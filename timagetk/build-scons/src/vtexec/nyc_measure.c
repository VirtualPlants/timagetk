#include <vt_common.h>

#define _COEFF_ 255.0

#define _CORRELATION_     1
#define _INV_CORRELATION_ 2
#define _AVERAGE_         3

typedef struct local_par {
  vt_names names;
  int is_mask;
  int type_computation;
  int sizex;
  int sizey;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-x %d] [-y %d] [-size %d %d]\n\
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
	vt_image *image, imres, *imask;

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
	  imask = _VT_Inrimage( par.names.ext );
	  if ( imask == (vt_image*)NULL ) {
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to read mask image\n", 0);
	  }
	  if ( imask->type != UCHAR ) {
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to deal with such mask image type\n", 0);
	  }
	}
	/*--- operations eventuelles sur l'image d'entree ---*/
	if ( par.names.inv == 1 )  VT_InverseImage( image );
	if ( par.names.swap == 1 ) VT_SwapImage( image );

	/*--- initialisation de l'image resultat ---*/
        VT_Image( &imres );
        VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, image->type );
        if ( VT_AllocImage( &imres ) != 1 ) {
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  if ( par.is_mask == 1 ) {
	    VT_FreeImage( imask );
	    VT_Free( (void**)&imask );
	  }
	  VT_ErrorParse("unable to allocate output image\n", 0);
        }

	switch ( par.type_computation ) {
	case _AVERAGE_ :
	  {
	  register int x, y, z;
	  register int i,j;
	  register u8 ***in = (u8 ***)(image->array);
	  register u8 ***out = (u8 ***)(imres.array);
	  register u8 ***mask = (u8 ***)NULL;
	  register double sum, coeff;
	  int ix, iy;

	  ix = (par.sizex - 1) / 2;
	  if (ix < 1) ix = 1;
	  iy = (par.sizey - 1) / 2;
	  if (iy < 1) iy = 1;
	  coeff = 1.0 / ( (2.0 * ix + 1)*(2.0 * iy + 1) );

	  if ( par.is_mask == 1 ) {
	    mask = (u8 ***)(imask->array);
	  }

	  for ( z=0; z< image->dim.z; z ++ ) {
	    if ( (_VT_VERBOSE_ == 1) || (_VT_DEBUG_ == 1) ) {
	      fprintf( stderr, " ... processing plane # %4d\r", z );
	    }
	    for ( y =0; y < image->dim.y; y ++ ) {
	      /* hors limites */
	      if ( (y < iy) || ( y+iy >= image->dim.y) ) {
		for ( x =0; x < image->dim.x; x ++ ) 
		  out[z][y][x] = 0;
		continue;
	      }
	      for ( x =0; x < image->dim.x; x ++ ) {
		/* hors limites */
		if ( (x < ix) || ( x+ix >= image->dim.x) ) {
		  out[z][y][x] = 0;
		  continue;
		}
		if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
		/* calcul */
		sum = 0.0;
		for ( j = y-iy; j <= y+iy; j++ )
		for ( i = x-ix; i <= x+ix; i++ ){
		  sum += (double)in[z][j][i];
		}
		sum *= coeff;
		out[z][y][x] = (u8)(sum + 0.5);
	      }
	    }
	  }}
	  break;
	case _INV_CORRELATION_ :
	default :
	  {
	  register int x, y, z;
	  register int i,j;
	  register u8 ***in = (u8 ***)(image->array);
	  register u8 ***out = (u8 ***)(imres.array);
	  register u8 ***mask = (u8 ***)NULL;
	  register double sum1, sum2, sumP;
	  int ix, iy;

	  ix = (par.sizex - 1) / 2;
	  if (ix < 1) ix = 1;
	  iy = (par.sizey - 1) / 2;
	  if (iy < 1) iy = 1;

	  if ( par.is_mask == 1 ) {
	    mask = (u8 ***)(imask->array);
	  }

	  for ( y =0; y < image->dim.y; y ++ )
	  for ( x =0; x < image->dim.x; x ++ ) 
	    out[0][y][x] = 0;

	  for ( z=1; z< image->dim.z; z ++ ) {
	    if ( (_VT_VERBOSE_ == 1) || (_VT_DEBUG_ == 1) ) {
	      fprintf( stderr, " ... processing plane # %4d\r", z );
	    }
	    for ( y =0; y < image->dim.y; y ++ ) {
	      /* hors limites */
	      if ( (y < iy) || ( y+iy >= image->dim.y) ) {
		for ( x =0; x < image->dim.x; x ++ ) 
		  out[z][y][x] = 0;
		continue;
	      }
	      for ( x =0; x < image->dim.x; x ++ ) {
		/* hors limites */
		if ( (x < ix) || ( x+ix >= image->dim.x) ) {
		  out[z][y][x] = 0;
		  continue;
		}
		if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
		/* calcul */
		sum1 = sum2 = sumP = 0;
		for ( j = y-iy; j <= y+iy; j++ )
		for ( i = x-ix; i <= x+ix; i++ ){
		  sum1 += (double)in[z][j][i] * (double)in[z][j][i];
		  sum2 += (double)in[z-1][j][i] * (double)in[z-1][j][i];
		  sumP += (double)in[z][j][i] * (double)in[z-1][j][i];
		}
		if ( (sum1 > 0.0) && (sum2 > 0.0) ) {
		  sumP /= sqrt( sum1 ) * sqrt( sum2 );
		  out[z][y][x] = (u8)(_COEFF_ * (1.0-sumP) + 0.5 );
		} else {
		  out[z][y][x] = 0;
		}
	      }
	    }
	  }}
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
	    /*--- taille de la fenetre ---*/
	    else if ( strcmp ( argv[i], "-x" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->sizex) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-y" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->sizey) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-size" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -size...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->sizex) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -size...\n", 0 );
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -size...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->sizey) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -size...\n", 0 );
	    }
	    /* */
	    else if ( strcmp ( argv[i], "-corr" ) == 0 ) {
	      par->type_computation = _CORRELATION_;
	    }
	    else if ( strcmp ( argv[i], "-invcorr" ) == 0 ) {
	      par->type_computation = _INV_CORRELATION_;
	    }
	    else if ( strcmp ( argv[i], "-average" ) == 0 ) {
	      par->type_computation = _AVERAGE_;
	    }
	    else if ( strcmp ( argv[i], "-mask" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -maks...\n", 0 );
	      strncpy( par->names.ext, argv[i], STRINGLENGTH );  
	      par->is_mask = 1;
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
	par->sizex = 3;
	par->sizey = 3;
}
