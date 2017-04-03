#include <vt_common.h>

#include <vt_seuil.h>

typedef struct local_par {
    vt_names names;
    float sb;
    float sh;
    int flag;
    int grey;
    int type;
  int norma;
} local_par;

/*-------Definition des fonctions statiques----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-sb %f] [-sh %f] [-inv] [-swap]\n\
\t [-grey] [-v] [-D] [-help] [options-de-type]";
static char *detail = "\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -sb %f : seuil bas\n\
\t -sh %f : seuil haut \n\
\t Les points dont la valeur est superieure ou egale au seuil bas, et,\n\
\t si un seuil haut est donne, inferieure ou egale au seuil haut, sont mis\n\
\t a 255, 32765, 65535, 2147483647 ou 1.0; les autres a 0.0 (defaut);\n\
\t -grey : les points dont la valeur est superieure ou egale au seuil bas, et,\n\
\t si un seuil haut est donne, inferieure ou egale au seuil haut, gardent\n\
\t valeur, les autres sont mis a 0;\n\
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
	vt_image *image, imres;

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
	VT_InitFromImage( &imres, image, par.names.out, image->type );
	if ( par.grey != 1 )
	    if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
	if ( VT_AllocImage( &imres ) != 1 ) {
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to allocate output image\n", 0);
	}
	
	/*--- seuillage --*/

	if ( par.norma == 1 ) {
	  if ( par.flag != 0 ) {
	    VT_FreeImage( &imres );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("must speicfy two thresholds\n", 0);
	  }
	  if ( imres.type != UCHAR ) {
	    VT_FreeImage( &imres );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("result image type must be unsigned char\n", 0);
	  }
	  switch ( image->type ) {
	  default :
	    VT_FreeImage( &imres );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("can not deal with such image type\n", 0);
	    break;

	  case USHORT :
	    {
	      unsigned short int ***theBuf = (unsigned short int ***)image->array;
	      unsigned char ***resBuf = (unsigned char ***)imres.array;
	      int x, y, z;
        for ( z=0; z<(int)image->dim.z; z++ )
        for ( y=0; y<(int)image->dim.y; y++ )
        for ( x=0; x<(int)image->dim.x; x++ ) {
		if (theBuf[z][y][x] <= par.sb) {
		  resBuf[z][y][x] = 0;
		} else if (theBuf[z][y][x] >= par.sh) {
		  resBuf[z][y][x] = 255;
		} else {
		  resBuf[z][y][x] = (int)( (double)(theBuf[z][y][x] -  par.sb) * 255.0 / 
					   (double)(par.sh -  par.sb) + 0.5);
		}
	      }
	    }
	    break;

	  case SSHORT :
	    {
	      short int ***theBuf = (short int ***)image->array;
	      unsigned char ***resBuf = (unsigned char ***)imres.array;
	      int x, y, z;
        for ( z=0; z<(int)image->dim.z; z++ )
        for ( y=0; y<(int)image->dim.y; y++ )
        for ( x=0; x<(int)image->dim.x; x++ ) {
		if (theBuf[z][y][x] <= par.sb) {
		  resBuf[z][y][x] = 0;
		} else if (theBuf[z][y][x] >= par.sh) {
		  resBuf[z][y][x] = 255;
		} else {
		  resBuf[z][y][x] = (int)( (double)(theBuf[z][y][x] -  par.sb) * 255.0 / 
					   (double)(par.sh -  par.sb) + 0.5);
		}
	      }
	    }
	    break;
	  }


	} else {

	  if ( par.grey == 1 ) {
	    if ( par.flag == 1 )
	      VT_GreyThreshold( image, &imres, par.sb );
	    else
	      VT_GreyThreshold2( image, &imres, par.sb, par.sh );
	  } else {
	    if ( par.flag == 1 )
	      VT_Threshold( image, &imres, par.sb );
	    else
	      VT_Threshold2( image, &imres, par.sb, par.sh );
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
	return(1);
}




static void VT_Parse( int argc, char *argv[], local_par *par )
{
	int i, nb, status;
	int isb=0, ish=0;
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
		/*--- grey ---*/
		else if ( strcmp ( argv[i], "-grey" ) == 0 ) {
		    par->grey = (int)1;
		}
		else if ( strcmp ( argv[i], "-norma" ) == 0 ) {
		    par->norma = (int)1;
		}
		/*--- seuils ---*/
		else if ( strcmp ( argv[i], "-sb" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -sb...\n", 0 );
		    status = sscanf( argv[i],"%f",&(par->sb) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -sb...\n", 0 );
		    isb = 1;
		}
		else if ( strcmp ( argv[i], "-sh" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -sh...\n", 0 );
		    status = sscanf( argv[i],"%f",&(par->sh) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -sh...\n", 0 );
		    ish = 1;
		}
		/*--- lecture du type de l'image ---*/
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
	/*--- noms des images ---*/
        if (nb == 0) {
                strcpy( par->names.in,  "<" );  /* standart input */
                strcpy( par->names.out, ">" );  /* standart output */
        }
        if (nb == 1)
                strcpy( par->names.out, ">" );  /* standart output */
	/*--- type de l'image resultat ---*/
	if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
	if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
	if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
	if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
	if ( (o == 0) && (s == 0) && (r == 1) )  par->type = FLOAT;
	if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program);
	/*--- controles ---*/
	if ( (isb == 0) && (ish == 0 ) )
		VT_ErrorParse("no specified thresholds\n", 0 );
	if ( (isb == 0) && (ish == 1 ) )
		VT_ErrorParse("low threshold not specified\n", 0 );
	if ( (isb == 1) && (ish == 0 ) )
		par->flag = (int)1;
	if ( (isb == 1) && (ish == 1 ) ) {
		par->flag = (int)0;
		if ( par->sb > par->sh ) 
			VT_ErrorParse("low threshold greater than high one\n", 0 );
	}
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
	par->sb = (float)0.0;
	par->sh = (float)0.0;
	par->flag = (int)0;
	par->grey = (int)0;
        par->type = UCHAR;
	par->norma = 0;
}
