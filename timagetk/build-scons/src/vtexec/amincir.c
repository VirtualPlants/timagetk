
#include <vt_common.h>
#include <vt_amincir.h>


typedef struct local_par {
    vt_names names;
    vt_amincir apar;
    int type;
} local_par;

/*-------Definition des fonctions statiques----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-s %f] [-inv] [-swap]\n\
\t [-gong | -grg | -grgc | -a2D ] [-old | -bdd | -dist] [-sh] \n\
\t [-chamfer | -chamfer2 | -chamfer3 | -eucli -eucli2 | -d6 | -d10 | -d18 | -d26]\n\
\t [-v] [-D] [-help] [options-de-type]";
static char *detail = "\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -s %f : seuil (1.0 par defaut)\n\
\t Les points dont la valeur est superieure ou egale au seuil\n\
\t definissent l'objet a amincir.\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -gong : algorithme de Gong-Bertrand\n\
\t -grg  : definition exacte des points simples\n\
\t -grgc : definition exacte des points simples (ne conserve que les courbes)\n\
\t -bdd  : utilisation des bdd (si existante)\n\
\t -old  : implementation historique\n\
\t -dist : tri des points selon la distance\n\
\t -2D : traitement 2D (dans le plan)\n\
\t -sh : shrinking (pas de condition d'arret)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s : short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.2 $ $Date: 2000/08/16 16:31:53 $ $Author: greg $\n";


static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image, imres;

	/*--- lecture des parametres ---*/
	VT_InitParam( &par );
	VT_Parse( argc, argv, &par );

	/*--- lecture de l'image d'entree ---*/
	image = _VT_Inrimage( par.names.in );
	if ( image == (vt_image*)NULL ) 
		VT_ErrorParse("unable to read input image\n", 0);

	/*--- operations eventuelles sur l'image d'entree ---*/
	if ( par.names.inv == 1 )  VT_InverseImage( image );
	if ( par.names.swap == 1 ) VT_SwapImage( image );

	if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to copy output image name\n", 0);
	}

	/*--- initialisation de l'image resultat ---*/
	VT_Image( &imres );
	VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, image->type );
	if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
	if ( VT_AllocImage( &imres ) != 1 ) {
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to allocate output image\n", 0);
	}
	
	/*--- seuillage --*/
	if ( VT_Thinning( &imres, image, &(par.apar) ) != 1 ) {
		VT_FreeImage( image );
		VT_FreeImage( &imres );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to thin input image\n", 0);
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
			/*--- methode d'amincissement ---*/
                        else if ( strcmp ( argv[i], "-gong" ) == 0 ) {
			    par->apar.type_method = VT_GONGBERTRAND;
			}
                        else if ( strcmp ( argv[i], "-grg" ) == 0 ) {
			    par->apar.type_method = VT_GRG_PLANES;
			}
                        else if ( strcmp ( argv[i], "-grgc" ) == 0 ) {
			    par->apar.type_method = VT_GRG_CURVES;
			}
                        else if ( strcmp ( argv[i], "-a2D" ) == 0 ) {
			    par->apar.type_method = VT_GRG_2D;
			}
			/*--- implementation ---*/
                        else if ( strcmp ( argv[i], "-bdd" ) == 0 ) {
			    par->apar.type_option = VT_BDD;
			}
                        else if ( strcmp ( argv[i], "-old" ) == 0 ) {
			    par->apar.type_option = VT_OLD;
			}
                        else if ( strcmp ( argv[i], "-dist" ) == 0 ) {
			    par->apar.type_option = VT_DIST;
			}
			/*--- shrinking ---*/
                        else if ( strcmp ( argv[i], "-sh" ) == 0 ) {
			    par->apar.bool_shrink = 1;
			}
                        /*--- type de distance ---*/
			else if ( strcmp ( argv[i], "-d6" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_6;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-d10" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_10;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-d18" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_18;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-d26" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_26;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-chamfer" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_CHMFR;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-chamfer2" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_CHMFR2;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-chamfer3" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_CHMFR3;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-eucli" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_EUCLI;
			    par->apar.type_option = VT_DIST;
			}
			else if ( strcmp ( argv[i], "-eucli2" ) == 0 ) {
			    par->apar.par_dist.type = VT_DIST_EUCLI_2;
			    par->apar.type_option = VT_DIST;
			}
			/*--- option 2D generale ---*/
                        else if ( strcmp ( argv[i], "-2D" ) == 0 ) {
			    par->apar.type_method = VT_GRG_2D;
			    par->apar.par_dist.dim = VT_2D;
			}
			/*--- seuils ---*/
                        else if ( strcmp ( argv[i], "-s" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -s...\n", 0 );
                                status = sscanf( argv[i],"%f",&(par->apar.seuil) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -s...\n", 0 );
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
	if ( (o == 1) && (s == 1)  && (r == 0) ) par->type = SCHAR;
	if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
	if ( (o == 2) && (s == 1)  && (r == 0) ) par->type = SSHORT;
	if ( (o == 4) && (s == 1)  && (r == 0) ) par->type = SINT;
	if ( (o == 0) && (s == 0) && (r == 1)  ) par->type = FLOAT;
	if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program);
}



static void VT_ErrorParse( char *str, int flag )
{
	(void)fprintf(stderr,"Usage : %s %s\n",program, usage);
        if ( flag == 1 ) (void)fprintf(stderr,"%s",detail);
        (void)fprintf(stderr,"Erreur :  %s",str);
        exit(0);
}



static void VT_InitParam( local_par *par )
{
    VT_Names( &(par->names) );
    VT_Amincir( &(par->apar) );
    par->type = TYPE_UNKNOWN;
}
