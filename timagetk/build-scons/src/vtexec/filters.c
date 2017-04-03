#include <vt_common.h>

#include <vt_filters.h>

typedef struct local_par {
    vt_names names;
    vt_fpt vz;
    vt_filters fp;
    int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out]\n\
\t [-x %d] [-y %d] [-z %d] [-sigma %f] [-sx %f] [-sy %f] [-sz %f]\n\
\t [-cont %d] [-xcont %d] [-ycont %d] [-zcont %d]\n\
\t [-vz %f %f %f] [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -x %d     : ordre de derivation selon X (de 0 a 3)\n\
\t -y %d     : ordre de derivation selon Y (de 0 a 3)\n\
\t -z %d     : ordre de derivation selon Z (de 0 a 3)\n\
\t -sigma %f : coefficient pour le filtrage (selon les 3 directions)\n\
\t -sx %f    : coefficient pour le filtrage selon X\n\
\t -sy %f    : coefficient pour le filtrage selon Y\n\
\t -sz %f    : coefficient pour le filtrage selon Z\n\
\t -cont %d  : poursuite par continuite aux bords (selon les 3 directions)\n\
\t -xcont %d : poursuite par continuite aux bords selon X\n\
\t -ycont %d : poursuite par continuite aux bords selon Y\n\
\t -zcont %d : poursuite par continuite aux bords selon Z\n\
\t -vz %f %f %f : taille du voxel\n\
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
 $Revision: 1.2 $ $Date: 2000/08/16 16:31:55 $ $Author: greg $\n";



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
        VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, image->type );
        if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
        if ( VT_AllocImage( &imres ) != 1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate output image\n", 0);
        }
	imres.siz.x = image->siz.x;
	imres.siz.y = image->siz.y;
	imres.siz.z = image->siz.z;

	/*--- filtrage ---*/
	if ( VT_RecursiveGaussian( &imres, image, &(par.fp) ) != 1 ) {
                VT_FreeImage( image );
                VT_FreeImage( &imres );
                VT_Free( (void**)&image );
                VT_ErrorParse("error when filtering\n", 0);
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
			/*--- sigma pour le filtrage ---*/
			else if ( strcmp ( argv[i], "-sigma" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -sigma...\n", 0 );
				status = sscanf( argv[i],"%f",&(par->fp.coeff_filter.x) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -sigma...\n", 0 );
				par->fp.coeff_filter.y = par->fp.coeff_filter.x;
				par->fp.coeff_filter.z = par->fp.coeff_filter.x;
			}
			else if ( strcmp ( argv[i], "-sx" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -sx...\n", 0 );
				status = sscanf( argv[i],"%f",&(par->fp.coeff_filter.x) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -sx...\n", 0 );
			}
			else if ( strcmp ( argv[i], "-sy" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -sy...\n", 0 );
				status = sscanf( argv[i],"%f",&(par->fp.coeff_filter.y) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -sy...\n", 0 );
			}
			else if ( strcmp ( argv[i], "-sz" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -sz...\n", 0 );
				status = sscanf( argv[i],"%f",&(par->fp.coeff_filter.z) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -sz...\n", 0 );
			}
			/*--- ordre du filtrage ---*/
			else if ( strcmp ( argv[i], "-x" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
				status = sscanf( argv[i],"%d",&(par->fp.order_filter.x) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
			}
			else if ( strcmp ( argv[i], "-y" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
				status = sscanf( argv[i],"%d",&(par->fp.order_filter.y) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
			}
			else if ( strcmp ( argv[i], "-z" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
				status = sscanf( argv[i],"%d",&(par->fp.order_filter.z) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
			}
			/*--- continuite aux bords ---*/
			else if ( strcmp ( argv[i], "-cont" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -cont...\n", 0 );
				status = sscanf( argv[i],"%d",&(par->fp.long_bord.x) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -cont...\n", 0 );
				par->fp.long_bord.y = par->fp.long_bord.x;
				par->fp.long_bord.z = par->fp.long_bord.x;
				par->fp.type_bord.x = _VT_BORD_CONT;
				par->fp.type_bord.y = _VT_BORD_CONT;
				par->fp.type_bord.z = _VT_BORD_CONT;
			}
			else if ( strcmp ( argv[i], "-xcont" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -xcont...\n", 0 );
				status = sscanf( argv[i],"%d",&(par->fp.long_bord.x) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -xcont...\n", 0 );
				par->fp.type_bord.x = _VT_BORD_CONT;
			}
			else if ( strcmp ( argv[i], "-ycont" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -ycont...\n", 0 );
				status = sscanf( argv[i],"%d",&(par->fp.long_bord.y) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -ycont...\n", 0 );
				par->fp.type_bord.y = _VT_BORD_CONT;
			}
			else if ( strcmp ( argv[i], "-zcont" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -zcont...\n", 0 );
				status = sscanf( argv[i],"%d",&(par->fp.long_bord.z) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -zcont...\n", 0 );
				par->fp.type_bord.z = _VT_BORD_CONT;
			}
                        /*--- lecture de la taille du voxel ---*/
                        else if ( strcmp ( argv[i], "-vz" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -vz...\n", 0 );
                                status = sscanf( argv[i],"%f",&(par->vz.x) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -vz...\n", 0 );
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -vz...\n", 0 );
                                status = sscanf( argv[i],"%f",&(par->vz.y) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -vz...\n", 0 );
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -vz...\n", 0 );
                                status = sscanf( argv[i],"%f",&(par->vz.z) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -vz...\n", 0 );
                                if ( (par->vz.x <= 0.0) || (par->vz.y <= 0.0) || (par->vz.z <= 0.0) )
                                        VT_ErrorParse( "parsing -vz...\n", 0 );
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

	/*--- type de l'image resultat ---*/
	if ( (o == 1) && (s == 1) && (r == 0) ) par->type = SCHAR;
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
    VT_Filters( &(par->fp) );
    par->vz.x = par->vz.y = par->vz.z = 1.0;
    par->type = TYPE_UNKNOWN;
}
