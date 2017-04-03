#include <vt_common.h>

#include <vt_mescorr.h>

#define VT_MIN(A,B) ( (A) < (B) ? (A) : (B) )

typedef struct local_par {
	vt_names names;
	int local;
	vt_ipt min;
	vt_ipt max;
	vt_ipt flag;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in1] [image-in2] [-inv] [-swap] [-v] [-D] [-help]\n\
\t [-x %d %d] [-y %d %d] [-z %d %d]";
static char *detail = "\
\t -x %d %d : definit les bornes selon X pour le calcul de la mesure\n\
\t -y %d %d : definit les bornes selon Y pour le calcul de la mesure\n\
\t -z %d %d : definit les bornes selon Z pour le calcul de la mesure\n\
\t    si les bornes ne sont pas definies, on les choisit maximales\n\
\t -inv : inverse 'image-in1' et 'image-in2' \n\
\t -swap : swap 'image-in1' et 'image-in2' (si codees sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n\
 $Revision: 1.2 $ $Date: 2000/08/16 16:31:56 $ $Author: greg $\n";


static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image1, *image2;
	vt_mescorr param;
	int xmin, ymin, zmin, xmax, ymax, zmax;
	
	/*--- initialisation des parametres ---*/
	VT_InitParam( &par );
	/*--- lecture des parametres ---*/
	VT_Parse( argc, argv, &par );

	/*--- lecture de l'image d'entree ---*/
	image1 = _VT_Inrimage( par.names.in );
	if ( image1 == (vt_image*)NULL ) 
		VT_ErrorParse("unable to read input image 1 \n", 0);
	image2 = _VT_Inrimage( par.names.ext );
	if ( image2 == (vt_image*)NULL ) {
		VT_FreeImage( image1 );
		VT_Free( (void**)&image1 );
		VT_ErrorParse("unable to read input image 2 \n", 0);
	}

	/*--- operations eventuelles sur l'image d'entree ---*/
	if ( par.names.inv == 1 )  {
		VT_InverseImage( image1 );
		VT_InverseImage( image2 );
	}
	if ( par.names.swap == 1 ) {
		VT_SwapImage( image1 );
		VT_SwapImage( image2 );
	}

	VT_Mescorr( &param );
	/*--- verification des dimensions ---*/
	xmin = ymin = zmin = 0;
	xmax = VT_MIN( (image1->dim.x - 1), (image2->dim.x - 1) );
	ymax = VT_MIN( (image1->dim.y - 1), (image2->dim.y - 1) );
	zmax = VT_MIN( (image1->dim.z - 1), (image2->dim.z - 1) );
	if ( par.flag.x == 1 ) {
		if ( (par.min.x < 0) || (par.min.x > xmax) || (par.min.x > par.max.x) )
			VT_ErrorParse("Coordinate X of the first corner not valide\n", 0);
		if ( (par.max.x < 0) || (par.max.x > xmax) )
			VT_ErrorParse("Coordinate X of the second corner not valide\n", 0);
		param.min.x = par.min.x;   param.max.x = par.max.x;   
		}
	else    param.min.x = xmin;   param.max.x = xmax;   
	if ( par.flag.y == 1 ) {
		if ( (par.min.y < 0) || (par.min.y > ymax) || (par.min.y > par.max.y) )
			VT_ErrorParse("Coordinate Y of the first corner not valide\n", 0);
		if ( (par.max.y < 0) || (par.max.y > ymax) )
			VT_ErrorParse("Coordinate Y of the second corner not valide\n", 0);
		param.min.y = par.min.y;   param.max.y = par.max.y;   
		}
	else    param.min.y = ymin;   param.max.y = ymax;   
	if ( par.flag.z == 1 ) {
		if ( (par.min.z < 0) || (par.min.z > zmax) || (par.min.z > par.max.z) )
			VT_ErrorParse("Coordinate Z of the first corner not valide\n", 0);
		if ( (par.max.z < 0) || (par.max.z > zmax) )
			VT_ErrorParse("Coordinate Z of the second corner not valide\n", 0);
		param.min.z = par.min.z;   param.max.z = par.max.z;   
		}
	else    param.min.z = zmin;   param.max.z = zmax;   
	
	/*--- calcul des parametres ---*/
	if ( VT_MesureCorrelation( image1, image2, &param ) != 1 ) {
		VT_FreeImage( image1 );
		VT_Free( (void**)&image1 );
		VT_FreeImage( image2 );
		VT_Free( (void**)&image2 );
		VT_ErrorParse("Error during computation\n", 0);
	}

	/*--- ecriture des resultats ---*/
	printf(" ----- Mesure de la correlation -----\n");
	printf(" image 1 : %s ;   image 2 : %s \n", par.names.in, par.names.ext );
	printf("       i2 = %g * i1 + %g \n", param.a, param.b );
	printf(" moyenne des distances a la droite : %g \n", param.mean );
	printf(" sigma (moyenne des distances au carre a la droite) : %g \n", param.sigma );

	/*--- liberations memoires ---*/
	VT_FreeImage( image1 );
	VT_Free( (void**)&image1 );
	VT_FreeImage( image2 );
	VT_Free( (void**)&image2 );
	return(1);
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
                                sprintf(text,"unknown option %s\n",argv[i]);
				VT_ErrorParse(text, 0);
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
			else if ( strcmp ( argv[i], "-x" ) == 0 ) {
				i += 1;
				if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->min.x) );
				if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
				i += 1;
				if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->max.x) );
				if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
				par->flag.x = 1;
			}
			else if ( strcmp ( argv[i], "-y" ) == 0 ) {
				i += 1;
				if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->min.y) );
				if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
				i += 1;
				if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->max.y) );
				if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
				par->flag.y = 1;
			}
			else if ( strcmp ( argv[i], "-z" ) == 0 ) {
				i += 1;
				if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->min.z) );
				if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
				i += 1;
				if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->max.z) );
				if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
				par->flag.z = 1;
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
                                strncpy( par->names.ext, argv[i], STRINGLENGTH );  
                                nb += 1;
                        }
                        else 
                                VT_ErrorParse("too much file names when parsing\n", 0 );
                }
                i += 1;
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
	par->local = 0;
	par->min.x = par->min.y = par->min.z = 0;
	par->max.x = par->max.y = par->max.z = 0;
	par->flag.x = par->flag.y = par->flag.z = 0;
}
