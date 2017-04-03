#include <vt_common.h>

#include <vt_bbox.h>

typedef enum {
  VT_INRIMAGE=1,
  VT_SUBIMAGE=2
} typeProcessing;


typedef struct local_par {
    vt_names names;
    vt_ipt border;
    typeProcessing type_processing;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-m %d %d %d] [-inrimage]\n\
\t [-inv] [-swap] [-v] [-D] [-help]";
static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t -m %d %d %d : definit la marge. L'image image-out contiendra\n\
\t               alors la bounding box d'image-in encadree par des\n\
\t               marges. Cette option est necessaire pour obtenir\n\
\t               la sous-image.\n\
\t -inrimage : donne le coin superieur gauche et la dimension de la\n\
\t             boite englobante (pour ext, par exemple).\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n\
 $Revision: 1.2 $ $Date: 2000/08/16 16:31:54 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image, imres;
	vt_ipt min, max;

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

	/*--- calcul ---*/
	switch( par.type_processing ) {
	case VT_SUBIMAGE :
	    VT_Image( &imres );
	    if ( VT_CopyBBoxWithBorders( image, &imres, &(par.border), &min ) != 1 ) {
		VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute subimage\n", 0);
	    }
	    /* VT_CopyBBoxWithBorders a mis le nom de image dans imres */
	    if ( VT_CopyName( imres.name, par.names.out ) == 0 ) {
		VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to copy image-out name\n", 0);
	    }
	    if ( par.names.inv == 1 )  VT_InverseImage( &imres );
	    if ( VT_WriteInrimage( &imres ) == -1 ) {
		VT_FreeImage( &imres );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to write output image\n", 0);
	    }
	    VT_FreeImage( &imres );
	    break;
	case VT_INRIMAGE :
	    if ( VT_BoundingBox( image, &min, &max ) == 1 ) {
		printf( " Xmin =  %5d, Ymin =  %5d, Zmin =  %5d \n", min.x + 1, min.y + 1, min.z + 1 );
		printf( " dimX =  %5d, dimY =  %5d, dimZ =  %5d \n", max.x - min.x + 1, max.y - min.y + 1, max.z - min.z + 1 );
	    }
	    break;
	default :
	    if ( VT_BoundingBox( image, &min, &max ) == 1 ) {
		printf( " Xmin =  %5d, Ymin =  %5d, Zmin =  %5d \n", min.x, min.y, min.z );
		printf( " Xmax =  %5d, Ymax =  %5d, Zmax =  %5d \n", max.x, max.y, max.z );
	    }
	}
	/*--- liberations memoires ---*/
	VT_FreeImage( image );
	VT_Free( (void**)&image );
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
                                if ( nb == 0 ) {
					/*--- standart input ---*/
					strcpy( par->names.in, "<" );
					nb += 1;
                                }
                        }
                        else if ( strcmp ( argv[i], "-inrimage" ) == 0 ) {
                                par->type_processing = VT_INRIMAGE;
			}
                        else if ( strcmp ( argv[i], "-m" ) == 0 ) {
			    i += 1;
			    if ( i >= argc)    VT_ErrorParse( "parsing -m...\n", 0 );
			    status = sscanf( argv[i],"%d",&(par->border.x) );
			    if ( status <= 0 ) VT_ErrorParse( "parsing -m...\n", 0 );
			    i += 1;
			    if ( i >= argc)    VT_ErrorParse( "parsing -m...\n", 0 );
			    status = sscanf( argv[i],"%d",&(par->border.y) );
			    if ( status <= 0 ) VT_ErrorParse( "parsing -m...\n", 0 );
			    i += 1;
			    if ( i >= argc)    VT_ErrorParse( "parsing -m...\n", 0 );
			    status = sscanf( argv[i],"%d",&(par->border.z) );
			    if ( status <= 0 ) VT_ErrorParse( "parsing -m...\n", 0 );
			    par->type_processing = VT_SUBIMAGE;
			    if ( (par->border.x < 0) || (par->border.y < 0) || (par->border.z < 0) )
				VT_ErrorParse( "parsing -m...\n", 0 );
			}
     			/*--- options generales sur l'image input ---*/
                        else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
                                par->names.inv = 1;
			}
                        else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
                                par->names.swap = 1;
			}
			/*--- options generales ---*/
			else if ( strcmp ( argv[i], "-help" ) == 0 ) {
                                VT_ErrorParse("\n", 1);
                        }
                        else if ( strcmp ( argv[i], "-v" ) == 0 ) {
                                _VT_VERBOSE_ = 1;
			}
                        else if ( strcmp ( argv[i], "-D" ) == 0 ) {
                                _VT_DEBUG_ = 1;
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
	par->border.x = par->border.y = par->border.z = 0;
	par->type_processing = VT_SUBIMAGE;
}
