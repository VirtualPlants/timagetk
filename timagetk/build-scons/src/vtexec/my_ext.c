#include <vt_common.h>

#include <vt_extract.h>

typedef struct local_par {
    vt_names names;
    vt_ipt corner;
    vt_ipt dim;
    int bool_slow;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-slow]\n\
\t [-ix %d] [-iy %d] [-iz %d] [-x %d] [-y %d] [-z %d] [-v] [-help]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -ix, -iy, -iz : origine (>= 1) de la fenetre dans 'image-in'\n\
\t -x, -y, -z    : taille de la fenetre\n\
\t -slow         : lecture des plans au fur et a mesure\n";
static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
    local_par par;
    vt_image *image, imres;

    /*--- initialisation des parametres ---*/
    VT_InitParam( &par );
    /*--- lecture des parametres ---*/
    VT_Parse( argc, argv, &par );
    


    /*
     * to be "ext" like
     */
    par.corner.x --;
    par.corner.y --;
    par.corner.z --;




    /*--- parametres ---*/
    if ( par.corner.x < 0 ) par.corner.x = 0;
    if ( par.corner.y < 0 ) par.corner.y = 0;
    if ( par.corner.z < 0 ) par.corner.z = 0;
    if ( par.dim.x <= 0 ) par.dim.x = 1;
    if ( par.dim.y <= 0 ) par.dim.y = 1;
    if ( par.dim.z <= 0 ) par.dim.z = 1;

#ifndef _gis_
    /*--- methode "lente" ---*/
    if ( par.bool_slow == 1 ) {
	if ( VT_SlowExtract( &(par.names), &(par.corner), &(par.dim) ) != 1 ) {
	    VT_ErrorParse("unable to extract sub-image (slow)\n", 0);
	}
	return( 1 );
    }
#endif

    /*--- lecture de l'image d'entree ---*/
    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) 
	VT_ErrorParse("unable to read input image\n", 0);

    if ( ((par.corner.x + par.dim.x) > image->dim.x) ||
	 ((par.corner.y + par.dim.y) > image->dim.y) ||
	 ((par.corner.z + par.dim.z) > image->dim.z) ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("invalid block dimensions or origin\n", 0);
    }

    /*--- initialisation de l'image resultat ---*/
    imres = *image;
    VT_InitImage( &imres, par.names.out, par.dim.x, par.dim.y, par.dim.z, image->type );
    if ( VT_AllocImage( &imres ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate output image\n", 0);
    }

    /*--- methode normale ---*/
    if ( VT_Extract( &imres, image, &(par.corner) ) != 1 ) {
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to extract sub-image\n", 0);
    }
#if defined(_gis_)
    imres.gis.ref_bool = 1;
    if ( VT_CopyName( imres.gis.ref_name, image->name ) == 0 )
      VT_Error("unable to reference name into image header", program);
    imres.gis.ref.x = par.corner.x;
    imres.gis.ref.y = par.corner.y;
    imres.gis.ref.z = par.corner.z;
    imres.gis.ref.t = 0;
#endif    

    /*--- liberations memoires ---*/
    VT_FreeImage( image );
    VT_Free( (void**)&image );

    /*--- ecriture de l'image resultat ---*/
    if ( VT_WriteInrimage( &imres ) == -1 ) {
	VT_FreeImage( &imres );
	VT_ErrorParse("unable to write output image\n", 0);
    }
		
    /*--- liberations memoires ---*/
    VT_FreeImage( &imres );
    
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
		else if ( strcmp ( argv[i], "-help" ) == 0 ) {
		    VT_ErrorParse("\n", 1);
		}
		else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
		    par->names.inv = 1;
		}
		else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
		    par->names.swap = 1;
		}
		else if ( strcmp ( argv[i], "-v" ) == 0 ) {
		    _VT_VERBOSE_ = 1;
		}
		else if ( strcmp ( argv[i], "-D" ) == 0 ) {
		    _VT_DEBUG_ = 1;
		}
		/*--- lecture du coin ---*/
		else if ( strcmp ( argv[i], "-ix" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -ix...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.x) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -ix...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-iy" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -iy...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.y) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -iy...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-iz" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -iz...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.z) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -iz...\n", 0 );
		}
		/*--- lecture du coin ---*/
		else if ( strcmp ( argv[i], "-ix" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -ix...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.x) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -ix...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-iy" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -iy...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.y) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -iy...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-iz" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -iz...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.z) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -iz...\n", 0 );
		}
		/*--- lecture du coin ---*/
		else if ( strcmp ( argv[i], "-ix" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -ix...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.x) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -ix...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-iy" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -iy...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.y) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -iy...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-iz" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -iz...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.z) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -iz...\n", 0 );
		}
		/*--- lecture des dimensions ---*/
		else if ( strcmp ( argv[i], "-x" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->dim.x) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-y" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->dim.y) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-z" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->dim.z) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
		}
		/*--- methode "lente" ---*/
		else if ( strcmp ( argv[i], "-slow" ) == 0 ) {
		    par->bool_slow = 1;
		}
		/*--- option inconnue ---*/
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
	par->corner.x = par->corner.y = par->corner.z = 0;
	par->dim.x = par->dim.y = par->dim.z = 0;
	par->bool_slow = 0;
}
