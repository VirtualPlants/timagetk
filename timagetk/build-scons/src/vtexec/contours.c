/*************************************************************************
 * contours.c -
 *
 * $Id: contours.c,v 1.3 2000/08/16 16:31:54 greg Exp $
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
#include <vt_neighborhood.h>
#include <vt_contours.h>

typedef enum {
  EXTREMA = 1,
  NORME = 2
} OutputType;


typedef struct local_par {
    vt_names names;
    vt_contours rpar;
    int type;
  OutputType output;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out]\n\
\t [-alpha %f | -a %f | [-ax %f] [-ay %f] [-az %f]]\n\
\t [-sigma %f | [-sx %f] [-sy %f] [-sz %f]] [-norme]\n\
\t [-cont %d | [-xcont %d] [-ycont %d] [-zcont %d]]\n\
\t [-2D] [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -alpha | -a  : alpha pour le filtre recursif de Deriche\n\
\t -a[x|y|z]    : alpha selon X, Y ou Z\n\
\t -sigma       : sigma pour l'approximation de la gaussienne\n\
\t -s[x|y|z]    : sigma selon X, Y ou Z\n\
\t -cont        : points ajoutes aux bords\n\
\t -[x|y|z]cont : points ajoutes aux bords selon X, Y ou Z\n\
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
 $Revision: 1.3 $ $Date: 2000/08/16 16:31:54 $ $Author: greg $\n";

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
        if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
        if ( VT_AllocImage( &imres ) != 1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate output image\n", 0);
        }



	switch ( par.output ) {
	case EXTREMA :
	default :

	  /*--- calcul ---*/
	  if ( VT_ExtractEdges( image, &imres, &(par.rpar) ) != 1 ) {
	    VT_FreeImage( image );
	    VT_FreeImage( &imres );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to extract edges from input image\n", 0);
	  }

	  break;

	case NORME :
	  if ( VT_NormeGradient( image, &imres, &(par.rpar), VT_DERIVATIVE_1_CONTOURS ) != 1 ) {
	    VT_FreeImage( image );
	    VT_FreeImage( &imres );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute norme from input image\n", 0);
	  }

	  break;
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
	    /*--- dimension du traitement ---*/
	    else if ( strcmp ( argv[i], "-2D" ) == 0 ) {
		par->rpar.dim = VT_2D;
	    }
	    /*--- alpha ---*/
	    else if ( (strcmp ( argv[i], "-alpha" ) == 0) || (strcmp ( argv[i], "-a" ) == 0) ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing [-alpha|-a]...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.x) );
                if ( status <= 0 ) VT_ErrorParse( "parsing [-alpha|-a]...\n", 0 );
		par->rpar.value_coefficient.z = par->rpar.value_coefficient.y = par->rpar.value_coefficient.x;
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECFILTERS_DERICHE :
		    par->rpar.type_filter = VT_RECFILTERS_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing [-alpha|-a]...\n", 0 );
		}
	    }
	    else if ( strcmp ( argv[i], "-ax" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -ax...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.x) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -ax...\n", 0 );
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECFILTERS_DERICHE :
		    par->rpar.type_filter = VT_RECFILTERS_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing -ax...\n", 0 );
		}
	    }
	    else if ( strcmp ( argv[i], "-ay" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -ay...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.y) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -ay...\n", 0 );
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECFILTERS_DERICHE :
		    par->rpar.type_filter = VT_RECFILTERS_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing -ay...\n", 0 );
		}
	    }
	    else if ( strcmp ( argv[i], "-az" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -az...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.z) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -az...\n", 0 );
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECFILTERS_DERICHE :
		    par->rpar.type_filter = VT_RECFILTERS_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing -az...\n", 0 );
		}
	    }
	    /*--- sigma ---*/
	    else if ( strcmp ( argv[i], "-sigma" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -sigma...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.x) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -sigma...\n", 0 );
		par->rpar.value_coefficient.z = par->rpar.value_coefficient.y = par->rpar.value_coefficient.x;
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECGAUSSIAN_DERICHE :
		    par->rpar.type_filter = VT_RECGAUSSIAN_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing -sigma...\n", 0 );
		}
	    }
	    else if ( strcmp ( argv[i], "-sx" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -sx...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.x) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -sx...\n", 0 );
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECGAUSSIAN_DERICHE :
		    par->rpar.type_filter = VT_RECGAUSSIAN_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing -sx...\n", 0 );
		}
	    }
	    else if ( strcmp ( argv[i], "-sy" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -sy...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.y) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -sy...\n", 0 );
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECGAUSSIAN_DERICHE :
		    par->rpar.type_filter = VT_RECGAUSSIAN_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing -sy...\n", 0 );
		}
	    }
	    else if ( strcmp ( argv[i], "-sz" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -sz...\n", 0 );
                status = sscanf( argv[i],"%f",&(par->rpar.value_coefficient.z) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -sz...\n", 0 );
		switch ( par->rpar.type_filter ) {
		case TYPE_UNKNOWN :
		case VT_RECGAUSSIAN_DERICHE :
		    par->rpar.type_filter = VT_RECGAUSSIAN_DERICHE;
		    break;
		default :
		    VT_ErrorParse( "parsing -sz...\n", 0 );
		}
	    }
	    /*--- bordure ---*/
	    else if ( strcmp ( argv[i], "-cont" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -cont...\n", 0 );
                status = sscanf( argv[i],"%d",&(par->rpar.length_continue.x) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -cont...\n", 0 );
		par->rpar.length_continue.z = par->rpar.length_continue.y = par->rpar.length_continue.x;
	    }
	    else if ( strcmp ( argv[i], "-xcont" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -xcont...\n", 0 );
                status = sscanf( argv[i],"%d",&(par->rpar.length_continue.x) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -xcont...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-ycont" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -ycont...\n", 0 );
                status = sscanf( argv[i],"%d",&(par->rpar.length_continue.y) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -ycont...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-zcont" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -zcont...\n", 0 );
                status = sscanf( argv[i],"%d",&(par->rpar.length_continue.z) );
                if ( status <= 0 ) VT_ErrorParse( "parsing -zcont...\n", 0 );
	    }



	    else if ( strcmp ( argv[i], "-norme" ) == 0 ) {
	      par->output = NORME;
	    }



	    /*--- marta ? ---*/
	    else if ( strcmp ( argv[i], "-marta" ) == 0 ) {
		par->rpar.type_filter = VT_RECGAUSSIAN_MARTA;
	    }

	    /*--- lecture du type de l'image de sortie ---*/
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

    /*--- type de l'image resultat ---*/
    if ( (o == 1) && (s == 1) && (r == 0) )  par->type = SCHAR;
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
    VT_Contours( &(par->rpar) );
    par->type = TYPE_UNKNOWN;
    par->output = EXTREMA;
}
