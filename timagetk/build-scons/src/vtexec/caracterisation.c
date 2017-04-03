/*************************************************************************
 * caracterisation.c -
 *
 * $Id: caracterisation.c,v 1.3 2000/08/16 16:31:54 greg Exp $
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
#include <vt_amincir.h>
#include <vt_seuil.h>
#include <vt_caracterisation.h>

#define _VT_CURVES   1
#define _VT_SURFACES 2
#define _VT_COMPLETE 3

typedef struct local_par {
    vt_names names;
    float seuil;
    int bool_thinning;
    int type_caracterisation;
    vt_caracterisation cpar;
    int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-seuil %f]\n\
\t[-curves | -surfaces | -complete] [-con %d] [-ej,-extended_junctions] \n\
\t[-thinning, -amincir] [-inv] [-swap] [-v] [-D] [-help]";

#ifndef _CC_
static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -s %f : seuil (1.0 par defaut)\n\
\t -curves     : caracterisation topologique dediee aux courbes\n\
\t -surfaces   : caracterisation topologique dediee aux surfaces (cas general et defaut)\n\
\t -complete   : a chaque type tres particulier correspond une valeur\n\
\t               (voir sources pour detail)\n\
\t -con %d     : connexite utilisee pour la recherche des faux points de\n\
                 surfaces (en fait points de jonction), et des points de\n\
                 frontiere voisins des points de jonction (option -ej).\n\
                 Les connexites permises sont 18 et 26 (defaut). Cette option\n\
                 n'est utilisee que pour les caracterisations -complete ou -surfaces.\n\
\t -ej         : les points frontieres voisins d'un point de jonctions entre\n\
                 surfaces sont consideres comme points de jonctions (pour la\n\
                 caracterisation dediee aux surfaces).\n\
\t -extended_junctions : id.\n\
\t -thinning   : un amincissement est fait avant la caracterisation\n\
\t -aminicir   : id.\n\
\t -inv  : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t -----------------------------------------------------------------------\n\
\t C'est un programme de caracterisation topologique.\n\
\t 1. l'image d'entree est (eventuellement) amincie :\n\
\t    - par `amincir -2D` pour les images plan (2D)\n\
\t    - par `amincir -bdd` pour les images volumiques et l'option -surfaces\n\
\t    - par `amincir -bddc` pour les images volumiques et l'option -curves\n\
\t 2. chaque point est caracterise topologiquement :\n\
\t    - en comptant le nombre de voisins en 2D et pour l'option -curves\n\
\t    - completement sinon\n\
\t Dans l'image resultat, les points ont les valeurs suivantes :\n\
\t    - VT_JUNCTION (255) : pour les points de jonction\n\
\t    - VT_SIMPLE   (200) : pour les points simples (frontieres)\n\
\t    - VT_CC       (100) : pour les points autres\n\
\n\
 $Revision: 1.3 $ $Date: 2000/08/16 16:31:54 $ $Author: greg $\n";

#else
static char *detail = "";
#endif

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
    local_par par;
    vt_image *image, imres;
    vt_amincir apar;
    
    /*--- initialisation des parametres ---*/
    VT_InitParam( &par );
    /*--- lecture des parametres ---*/
    VT_Parse( argc, argv, &par );
    
    /*--- ---*/
    if ( (par.cpar.connexite != N18) && (par.cpar.connexite != N26) ) {
	VT_ErrorParse("Bad connectivity\n", 0);
    }
    
    /*--- lecture de l'image d'entree ---*/
    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) 
	VT_ErrorParse("unable to read input image\n", 0);
    
    /*--- operations eventuelles sur l'image d'entree ---*/
    if ( par.names.inv == 1 )  VT_InverseImage( image );
    if ( par.names.swap == 1 ) VT_SwapImage( image );
    
    /*--- initialisation de l'image resultat ---*/
    VT_Image( &imres );
    VT_InitFromImage( &imres, image, par.names.out, (int)UCHAR );
    if ( VT_AllocImage( &imres ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate result image\n", 0);
    }
    
    /*--- amincissement eventuel ---*/
    if ( par.bool_thinning == 1 ) {
	VT_Amincir( &apar );
	apar.seuil = par.seuil;
	apar.type_option = VT_BDD;
	switch ( par.type_caracterisation ) {
	case _VT_CURVES :
	    apar.type_method = VT_GRG_CURVES;
	    break;
	case _VT_SURFACES :
	default :
	    apar.type_method = VT_GRG_PLANES;
	}
	if ( image->dim.z == 1 ) apar.type_method = VT_GRG_2D;
	VT_Thinning( &imres, image, &apar );
    }
    else
	VT_Threshold( image, &imres, par.seuil );
    
    /*--- Caracterisation ---*/
    switch ( par.type_caracterisation ) {
    case _VT_CURVES :
	VT_CaracterisationCurves( &imres, &imres );
	break;
    case _VT_COMPLETE :
	VT_CaracterisationComplete( &imres, &imres, &(par.cpar) );
	break;
    case _VT_SURFACES :
    default :
	VT_Caracterisation( &imres, &imres, &(par.cpar) );
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
    int connexite;
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
	    /*--- type de caracterisation ---*/
	    else if ( strcmp ( argv[i], "-curves" ) == 0 ) {
		par->type_caracterisation = _VT_CURVES;
	    }
	    else if ( strcmp ( argv[i], "-surfaces" ) == 0 ) {
		par->type_caracterisation = _VT_SURFACES;
	    }
	    else if ( strcmp ( argv[i], "-complete" ) == 0 ) {
		par->type_caracterisation = _VT_COMPLETE;
	    }
	    /*--- amincissement eventuel ---*/
	    else if ( strcmp ( argv[i], "-thinning" ) == 0 ) {
		par->bool_thinning = 1;
	    }
	    else if ( strcmp ( argv[i], "-amincir" ) == 0 ) {
		par->bool_thinning = 1;
	    }
	    /*--- options de caracterisation ---*/
	    else if ( strcmp ( argv[i], "-ej" ) == 0 ) {
		par->cpar.bool_extended_junctions = 1;
	    }
	    else if ( strcmp ( argv[i], "extended_junctions" ) == 0 ) {
		par->cpar.bool_extended_junctions = 1;
	    }
	    else if ( strcmp ( argv[i], "-con" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -con...\n", 0 );
		status = sscanf( argv[i],"%d",&connexite );
		if ( status <= 0 ) VT_ErrorParse( "parsing -con...\n", 0 );
	    }
	    /*--- seuils ---*/
	    else if ( strcmp ( argv[i], "-seuil" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -seuil...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->seuil) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -seuil...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-sb" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -sb...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->seuil) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -sb...\n", 0 );
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

    /*--- conversion de la connexite ---*/
    switch ( connexite ) {
    case 4 :
	par->cpar.connexite = N04;
	break;
    case 8 :
	par->cpar.connexite = N08;
	break;
    case 6 :
	par->cpar.connexite = N06;
	break;
    case 10 :
	par->cpar.connexite = N10;
	break;
    case 18 :
	par->cpar.connexite = N18;
	break;
    case 26 :
	par->cpar.connexite = N26;
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
    par->seuil = (float)1.0;
    par->bool_thinning = 0;
    par->type_caracterisation = _VT_SURFACES;
    VT_InitCaracterisation( &(par->cpar) );
    par->type = TYPE_UNKNOWN;
}




