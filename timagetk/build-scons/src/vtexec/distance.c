/*************************************************************************
 * distance.c -
 *
 * $Id: distance.c,v 1.3 2000/08/16 16:31:55 greg Exp $
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

#include <vt_distance.h>

typedef struct local_par {
  vt_names names;
  vt_distance dpar;
  int bool_vector;
  int bool_basic;
  int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );





static char *usage = "[image-in] [image-out] [-sb %f] [-2D]\n\
\t [-chamfer | -chamfer2 | -chamfer3 | -eucli | -eucli2 | -d %d |\n\
\t  -user %d %d %d %d %d | -vect] [-euc_type %d] \n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -sb %f : seuil (0.5 par defaut)\n\
\t -chamfer : distance du chanfrein (defaut), cf Borgefors\n\
\t -chamfer2 : distance du chanfrein x 16\n\
\t -chamfer3 : distance du chanfrein x 32, orientee volume\n\
\t -eucli : distance euclidienne, cf Danielsson\n\
\t -eucli2 : distance euclidienne au carre, cf Danielsson\n\
\t -d %d : [4,6,8,10,18,26]-distance\n\
\t -user %d %d %d %d %d : increments specifies par l'utilisateur\n\
\t    dans l'ordre, voisin dans le plan par une face ou une arete,\n\
\t    voisin dans le plan adjacent par une face, une arete ou un sommet.\n\
\t -vect : ecrit les images donnant les coordonnees du point le plus\n\
\t         proche : image-out.x pour la coordonnee selon X, image-out.y\n\
\t         pour la coordonnee selon Y et, si l'image n'est pas 2D,\n\
\t         image-out.z pour la coordonnee selon Z.\n\
\t -2D : restriction dans le plan des distances ci-dessus\n\
\t -euc_type %d : codage pour le calcul de l'euclidean mapping\n\
\t                1 = signed char, 2 = signed short (defaut)\n\
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
 $Revision: 1.3 $ $Date: 2000/08/16 16:31:55 $ $Author: greg $\n";




static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image, imres;
	vt_image imx, imy, imz;

    fprintf(stderr, "\nObsolete version, the user should use the program \"chamfer\" instead of \"distance\"\n\n");

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

	if ( par.bool_vector == 1 ) {
	    /*--- calcul du vecteur pointant vers le plus proche point ---*/
	    /*--- initialisation des images resultat ---*/
	    VT_Image( &imx );
	    VT_InitFromImage( &imx, image, par.names.out, par.dpar.type_image_eucmap );
	    VT_Image( &imy );
	    VT_InitFromImage( &imy, image, par.names.out, par.dpar.type_image_eucmap );
	    VT_Image( &imz );
	    VT_InitFromImage( &imz, image, par.names.out, par.dpar.type_image_eucmap );
	    (void)strcat( imx.name, ".x" );
	    (void)strcat( imy.name, ".y" );
	    (void)strcat( imz.name, ".z" );
	    if ( VT_AllocImage( &imx ) != 1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate first output image\n", 0);
	    }
	    if ( VT_AllocImage( &imy ) != 1 ) {
                VT_FreeImage( &imx );
		VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate first second image\n", 0);
	    }
	    if ( VT_AllocImage( &imz ) != 1 ) {
                VT_FreeImage( &imx );
                VT_FreeImage( &imy );
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate first third image\n", 0);
	    }

	    /*--- calcul ---*/
	    switch ( par.dpar.type_image_eucmap ) {
	    case SCHAR :
		if ( VT_VecteurPPP_SC( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
		    VT_FreeImage( &imx );
		    VT_FreeImage( &imy );
		    VT_FreeImage( &imz );
		    VT_FreeImage( image );
		    VT_Free( (void**)&image );
		    VT_ErrorParse("unable to compute nearest point\n", 0);
		}
		break;
	    case SSHORT :
		if ( VT_VecteurPPP_SS( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
		    VT_FreeImage( &imx );
		    VT_FreeImage( &imy );
		    VT_FreeImage( &imz );
		    VT_FreeImage( image );
		    VT_Free( (void**)&image );
		    VT_ErrorParse("unable to compute nearest point\n", 0);
		}
	    }
	    
            /*--- ecriture des images resultat ---*/
	    if ( VT_WriteInrimage( &imx ) == -1 ) {
		VT_FreeImage( &imx );
		VT_FreeImage( &imy );
		VT_FreeImage( &imz );
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to write first output image\n", 0);
	    }

	    if ( VT_WriteInrimage( &imy ) == -1 ) {
		VT_FreeImage( &imx );
		VT_FreeImage( &imy );
		VT_FreeImage( &imz );
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to write second output image\n", 0);
	    }
	    
	    if ( image->dim.z > 1 ) {
		if ( VT_WriteInrimage( &imz ) == -1 ) {
		    VT_FreeImage( &imx );
		    VT_FreeImage( &imy );
		    VT_FreeImage( &imz );
		    VT_FreeImage( image );
		    VT_Free( (void**)&image );
		    VT_ErrorParse("unable to write third output image\n", 0);
		}
	    }

	    /*--- liberations memoires ---*/
	    VT_FreeImage( &imx );
	    VT_FreeImage( &imy );
	    VT_FreeImage( &imz );

	} 
	else {
	    /*--- initialisation de l'image resultat ---*/
	    VT_Image( &imres );
	    VT_InitFromImage( &imres, image, par.names.out, image->type );
	    if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
	    if ( VT_AllocImage( &imres ) != 1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate output image\n", 0);
	    }
	    
	    if ( par.bool_basic == 1 ) {
	      VT_ErrorParse("in progress ... emergency exit\n", 0);
	      /*
	      switch ( par.dpar.type ) {
	      case VT_DIST_4 :
		SetChamferCoefficientsToDistance04();
		break;
	      case VT_DIST_6 :
		SetChamferCoefficientsToDistance06();
		break;
	      case VT_DIST_8 :
		SetChamferCoefficientsToDistance08();
		break;
	      case VT_DIST_18 :
		SetChamferCoefficientsToDistance18();
		break;
	      case VT_DIST_26 :
		SetChamferCoefficientsToDistance26();
		break;
	      case VT_DIST_CHMFR :
		SetChamferCoefficientsToBorgefors3x3x3();
		break;
	      case VT_DIST_CHMFR2 :
		SetChamferCoefficientsToMalandain3x3x3();
		break;
	      default :
		VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("distance type not handled yet\n", 0);
	      }
	      */
	    } 
	    else {

	      /*--- calcul ---*/
	      if ( VT_Dist( &imres, image, &(par.dpar) ) != 1 ) {
		VT_FreeImage( image );
                VT_FreeImage( &imres );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to compute distance\n", 0);
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
	    VT_FreeImage( &imres );
	}

	/*--- liberations memoires ---*/
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
	int i, nb, status, connexite = 0;
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
                        /*--- seuil ---*/
                        else if ( strcmp ( argv[i], "-sb" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -sb...\n", 0 );
                                status = sscanf( argv[i],"%f",&(par->dpar.seuil) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -sb...\n", 0 );
                        }
			/*--- dimension du traitement ---*/
			else if ( strcmp ( argv[i], "-2D" ) == 0 ) {
                                par->dpar.dim = VT_2D;
			}
                        /*--- type de distance ---*/
			else if ( strcmp ( argv[i], "-d" ) == 0 ) {
			    i += 1;
			    if ( i >= argc)    VT_ErrorParse( "parsing -d...\n", 0 );
			    status = sscanf( argv[i], "%d", &connexite );
			    if ( status <= 0 ) VT_ErrorParse( "parsing -d...\n", 0 );
			}
			else if ( strcmp ( argv[i], "-chamfer" ) == 0 ) {
                                par->dpar.type = VT_DIST_CHMFR;
			}
			else if ( strcmp ( argv[i], "-chamfer2" ) == 0 ) {
                                par->dpar.type = VT_DIST_CHMFR2;
			}
			else if ( strcmp ( argv[i], "-chamfer3" ) == 0 ) {
                                par->dpar.type = VT_DIST_CHMFR3;
			}
			else if ( strcmp ( argv[i], "-eucli" ) == 0 ) {
                                par->dpar.type = VT_DIST_EUCLI;
			}
			else if ( strcmp ( argv[i], "-eucli2" ) == 0 ) {
                                par->dpar.type = VT_DIST_EUCLI_2;
			}
			else if ( strcmp ( argv[i], "-vect" ) == 0 ) {
                                par->bool_vector = 1;
			}
                        else if ( strcmp ( argv[i], "-user" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -user...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->dpar.inc[0]) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -user...\n", 0 );
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -user...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->dpar.inc[1]) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -user...\n", 0 );
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -user...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->dpar.inc[2]) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -user...\n", 0 );
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -user...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->dpar.inc[3]) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -user...\n", 0 );
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -user...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->dpar.inc[4]) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -user...\n", 0 );
                                if ( (par->dpar.inc[0] <= 0) || (par->dpar.inc[1] <= 0) || (par->dpar.inc[2] <= 0) 
				     || (par->dpar.inc[3] <= 0) || (par->dpar.inc[4] <= 0) )
                                        VT_ErrorParse( "parsing -user...\n", 0 );
				par->dpar.type = VT_DIST_USER;
                        }	
		
			else if ( strcmp ( argv[i], "-basic" ) == 0 ) {
			  par->bool_basic = 1;
			}

			/*--- type pour le calcul de l'euclidean mapping ---*/
                        else if ( strcmp ( argv[i], "-euc_type" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -euc_type...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->dpar.type_image_eucmap) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -euc_type...\n", 0 );
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
        if (nb == 0) {
                strcpy( par->names.in,  "<" );  /* standart input */
                strcpy( par->names.out, ">" );  /* standart output */
        }
        if (nb == 1)
                strcpy( par->names.out, ">" );  /* standart output */

	/*--- conversion du type pour le calcul de l'euclidean mapping ---*/
	switch ( par->dpar.type_image_eucmap ) {
	case 1 :
	    par->dpar.type_image_eucmap = SCHAR;
	    break;
	case 2 :
	default :
	    par->dpar.type_image_eucmap = SSHORT;
	}
	
	switch ( connexite ) {
	case 4 :
	    par->dpar.type = VT_DIST_4;
	    break;
	case 6 :
	    par->dpar.type = VT_DIST_6;
	    break;
	case 8 :
	    par->dpar.type = VT_DIST_8;
	    break;
	case 10 :
	    par->dpar.type = VT_DIST_10;
	    break;
	case 18 :
	    par->dpar.type = VT_DIST_18;
	    break;
	case 26 :
	    par->dpar.type = VT_DIST_26;
	}

	/*--- type de l'image resultat ---*/
	if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
	if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
	if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
	if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
	if ( (o == 0) && (s == 0) && (r == 1) )  par->type = FLOAT;
	if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program);
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
	VT_Distance( &(par->dpar) );
    par->bool_vector = 0;
    par->bool_basic = 0;
    par->type = TYPE_UNKNOWN;
}
