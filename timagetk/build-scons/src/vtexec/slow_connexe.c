#include <vt_common.h>

#include <vt_connexe.h>
#include <vt_slconnexe.h>

typedef enum {
  VT_NONE = 0,
  VT_DELTA = 1
} typeProcessing;

typedef struct local_par {
    vt_names names;
    vt_connexe cpar;
    vt_ipt seuils;
    vt_ipt first_point;
    typeProcessing type_processing;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-con %d] [-tcc %d]\n\
\t [-s %f] [[-delta %d %d] [-first %d %d %d]]\n\
\t [-inv] [-swap] [-v] [-D] [-help]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -con %d : connexite utilisee (4, 6, 8, 10, 18, 26), 26 par defaut\n\
\t -tcc %d : taille minimale des composantes a conserver, 1 par defaut\n\
\t -s %f : seuil (non normalise) pour considerer l'image comme binaire\n\
\t -delta %d %d : seuils a appliquer aux differences\n\
\t -first %d %d %d : coordonnees d'un point de la composante\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image;

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

	/*--- calcul des composantes connexes ---*/
	switch ( par.type_processing ) {
	case VT_DELTA :
	    par.seuils.x = (int)(par.cpar.threshold + 0.5);
	    if ( VT_SlowConnexeWithThresholds( image, &(par.cpar), &(par.seuils), &(par.first_point) ) != 1 ) {
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute connected components (with thresholds)\n", 0);
	    }
	    break;
	case VT_NONE :
	default :
	    if ( VT_SlowConnexe( image, &(par.cpar) ) != 1 ) {
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute connected components\n", 0);
	    }
	}

	/*--- ecriture de l'image resultat ---*/
        if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to copy image-out name\n", 0);
	}
	if ( VT_WriteInrimage( image ) == -1 ) {
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to write output image\n", 0);
        }
		
	/*--- liberations memoires ---*/
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
	int i, nb, status;
	int c;
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
	    /*--- options pour la recherche de composantes connexes ---*/
	    else if ( strcmp( argv[i], "-con" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -con...\n", 0 );
	      status = sscanf( argv[i],"%d",&c );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -con...\n", 0 );
	      switch ( c ) {
	      case 4 :
		par->cpar.type_connexite = N04; break;
	      case 6 :
		par->cpar.type_connexite = N06; break;
	      case 8 :
		par->cpar.type_connexite = N08; break;
	      case 10 :
		par->cpar.type_connexite = N10; break;
	      case 18 :
		par->cpar.type_connexite = N18; break;
	      case 26 :
		par->cpar.type_connexite = N26; break;
	      default :
		VT_ErrorParse(" parsing -con...\n", 0 );
	      }
	    }
	    else if ( strcmp( argv[i], "-tcc" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -tcc...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->cpar.min_size) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -tcc...\n", 0 );
	    }
	    else if ( strcmp( argv[i], "-s" )  == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -s...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->cpar.threshold) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -s...\n", 0 );
	    }
	    else if ( strcmp( argv[i], "-sb" )  == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -sb...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->cpar.threshold) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -sb...\n", 0 );
	    }
	    /*--- options pour la recherche specifique ---*/
	    else if ( strcmp( argv[i], "-first" )  == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -first...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->first_point.x) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -first...\n", 0 );
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -first...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->first_point.y) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -first...\n", 0 );
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -first...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->first_point.z) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -first...\n", 0 );
	    }
	    else if ( strcmp( argv[i], "-delta" )  == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -delta...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->seuils.y) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -delta...\n", 0 );
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse(" parsing -delta...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->seuils.z) );
	      if ( status <= 0 ) VT_ErrorParse(" parsing -delta...\n", 0 );
	      par->type_processing = VT_DELTA;
	    }
	    /*--- options sur image input ---*/
	    else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	      par->names.inv = 1;
	    }
	    else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	      par->names.swap = 1;
	    }
			/*--- options generales ---*/
                        else if ( strcmp ( argv[i], "-v" ) == 0 ) {
			    par->cpar.verbose = 1;
			    _VT_VERBOSE_ = 1;
			}
                        else if ( strcmp ( argv[i], "-D" ) == 0 ) {
                                _VT_DEBUG_ = 1;
			}
			else if ( strcmp ( argv[i], "-help" ) == 0 ) {
                                VT_ErrorParse("\n", 1);
                        }
			else {
				sprintf(text,"unknown option %s\n",argv[i]);
				VT_ErrorParse(text, 0);
			}
		}
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
	VT_Connexe( &(par->cpar) );
	par->seuils.x = par->seuils.y = par->seuils.z = 0;
	par->first_point.x = par->first_point.y = par->first_point.z = 0;
	par->type_processing = VT_NONE;
}
