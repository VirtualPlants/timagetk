#include <vt_common.h>

#include <vt_copy.h>
#include <vt_gausssmooth.h>
#include <vt_localsmooth.h>

typedef enum {
  VT_NONE = 0,
  _GAUSS_ = 1,
  _LOCAL_ = 2,
} typeComputation;

#define _MAX_LOCAL_ 4

typedef struct local_par {
  typeComputation type_computation;
  vt_names names;
  /*--- gauss ---*/
  float sigma;
  int length;
  /*--- local ---*/
  int nb_local;
  int def_local[2][_MAX_LOCAL_];
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out]\n\
\t [-sigma %f -length %d] [-06 %d] [-26 %d]\n\
\t [-inv] [-swap] [-v] [-D] [-help]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image, imres, *theIm;
	int i;
	Neighborhood c;
	
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
        VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, image->type );
        if ( VT_AllocImage( &imres ) != 1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate output image\n", 0);
        }

	switch ( par.type_computation ) {
	case _LOCAL_ :
	  if ( par.nb_local <= 0 ) {
	    if ( VT_CopyImage( image, &imres ) != 1 ) {
	      VT_ErrorParse( "error while copying\n", 0);
	    }
	    break;
	  }
	  theIm = image;
	  for ( i = 0; i < par.nb_local; i ++ ) {
	    switch ( par.def_local[0][i] ) {
	    case N04 :
	      c = C_04;
	      if ( VT_SmoothingWithConnectivity( theIm, &imres, c, par.def_local[1][i] ) != 1 ) {
                VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute result with 04 smoothing\n", 0);
	      }
	      break;
	    case N06 :
	      c = C_06;
	      if ( VT_SmoothingWithConnectivity( theIm, &imres, c, par.def_local[1][i] ) != 1 ) {
                VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute result with 06 smoothing\n", 0);
	      }
	      break;
	    case N08 :
	      c = C_08;
	      if ( VT_SmoothingWithConnectivity( theIm, &imres, c, par.def_local[1][i] ) != 1 ) {
                VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute result with 08 smoothing\n", 0);
	      }
	      break;
	    case N10 :
	      c = C_10;
	      if ( VT_SmoothingWithConnectivity( theIm, &imres, c, par.def_local[1][i] ) != 1 ) {
                VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute result with 10 smoothing\n", 0);
	      }
	      break;
	    case N18 :
	      c = C_18;
	      if ( VT_SmoothingWithConnectivity( theIm, &imres, c, par.def_local[1][i] ) != 1 ) {
                VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute result with 18 smoothing\n", 0);
	      }
	      break;
	    case N26 :
	      c = C_26;
	      if ( VT_SmoothingWithConnectivity( theIm, &imres, c, par.def_local[1][i] ) != 1 ) {
                VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute result with 26 smoothing\n", 0);
	      }
	    }
	    theIm = &imres;
	  }
	  break;
	case _GAUSS_ :
	default :
	  if ( VT_GaussianSmoothing( image, &imres, (double)(par.sigma), (int)(par.length) ) != 1 ) {
                VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute result with gaussian smoothing\n", 0);
	  }
	}

	/*--- ecriture de l'image resultat ---*/
        if ( VT_WriteInrimage( &imres ) == -1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to write output image\n", 0);
        }
		
	/*--- liberations memoires ---*/
	VT_FreeImage( image );
        VT_FreeImage( &imres );
	VT_Free( (void**)&image );
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
	    /*--- traitement eventuel de l'image d'entree ---*/
	    else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
		par->names.inv = 1;
	    }
	    else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
		par->names.swap = 1;
	    }
	    /*--- lissage gaussien ---*/
	    else if ( strcmp ( argv[i], "-sigma" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -sigma...\n", 0 );
	      status = sscanf( argv[i],"%f",&(par->sigma) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -sigma...\n", 0 );
	      par->type_computation = _GAUSS_;
	    }
	    else if ( strcmp ( argv[i], "-length" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -length...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->length) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -length...\n", 0 );
	    }
	    /*--- lissage local ---*/
	    else if ( strcmp ( argv[i], "-04" ) == 0 ) {
	      if ( par->nb_local == _MAX_LOCAL_ )
		VT_ErrorParse( "parsing -04: too many local definitions...\n", 0 );
	      par->def_local[0][par->nb_local] = N04;
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -04...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->def_local[1][par->nb_local]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -04...\n", 0 );
	      par->nb_local ++;
	      par->type_computation = _LOCAL_;
	    }
	    else if ( strcmp ( argv[i], "-06" ) == 0 ) {
	      if ( par->nb_local == _MAX_LOCAL_ )
		VT_ErrorParse( "parsing -06: too many local definitions...\n", 0 );
	      par->def_local[0][par->nb_local] = N06;
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -06...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->def_local[1][par->nb_local]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -06...\n", 0 );
	      par->nb_local ++;
	      par->type_computation = _LOCAL_;
	    }
	    else if ( strcmp ( argv[i], "-08" ) == 0 ) {
	      if ( par->nb_local == _MAX_LOCAL_ )
		VT_ErrorParse( "parsing -08: too many local definitions...\n", 0 );
	      par->def_local[0][par->nb_local] = N08;
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -08...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->def_local[1][par->nb_local]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -08...\n", 0 );
	      par->nb_local ++;
	      par->type_computation = _LOCAL_;
	    }
	    else if ( strcmp ( argv[i], "-10" ) == 0 ) {
	      if ( par->nb_local == _MAX_LOCAL_ )
		VT_ErrorParse( "parsing -10: too many local definitions...\n", 0 );
	      par->def_local[0][par->nb_local] = N10;
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -10...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->def_local[1][par->nb_local]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -10...\n", 0 );
	      par->nb_local ++;
	      par->type_computation = _LOCAL_;
	    }
	    else if ( strcmp ( argv[i], "-18" ) == 0 ) {
	      if ( par->nb_local == _MAX_LOCAL_ )
		VT_ErrorParse( "parsing -18: too many local definitions...\n", 0 );
	      par->def_local[0][par->nb_local] = N18;
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -18...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->def_local[1][par->nb_local]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -18...\n", 0 );
	      par->nb_local ++;
	      par->type_computation = _LOCAL_;
	    }
	    else if ( strcmp ( argv[i], "-26" ) == 0 ) {
	      if ( par->nb_local == _MAX_LOCAL_ )
		VT_ErrorParse( "parsing -26: too many local definitions...\n", 0 );
	      par->def_local[0][par->nb_local] = N26;
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -26...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->def_local[1][par->nb_local]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -26...\n", 0 );
	      par->type_computation = _LOCAL_;
	      par->nb_local ++;
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
  int i;
  par->type_computation = VT_NONE;
  VT_Names( &(par->names) );

  par->sigma = 1.0;
  par->length = 7;

  par->nb_local = 0;
  for ( i = 0; i < _MAX_LOCAL_; i ++ ) {
    par->def_local[0][i] = VT_NONE;
    par->def_local[1][i] = 0;
  }

}







