/*************************************************************************
 * binary_noise.c
 *
 * $Id: binary_noise.c,v 1.3 2000/10/18 08:45:59 greg Exp $
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
 * 
 * 
 */

#include <stdlib.h>
#include <vt_common.h>
#include <vt_copy.h>


/*-------------------- statistics --------------------*/
#if defined(_STATISTICS_)
extern int TESTS_nb;
#define VAL(tab,index) ( ++TESTS_nb ? tab[index] : tab[index] )
#else
#define VAL(tab,index) tab[index]
#endif
/*----------------------------------------------------*/

int VT_IsSimple2D( const int *V )
{
    /*--- pour eviter les erreurs, j'ai numerote
          le voisinage comme suit :
	  0 1 2
          3 x 4
          5 6 7
     ---*/

    int nb_cc = 0;

    if ( !VAL(V,1) ) {
	nb_cc ++;
	if ( !VAL(V,4) && !VAL(V,2) ) nb_cc --;
    }
    if ( !VAL(V,4) ) {
	nb_cc ++;
	if ( !VAL(V,6) && !VAL(V,7) ) nb_cc --;
    }
    if ( !VAL(V,6) ) {
	nb_cc ++;
	if ( !VAL(V,3) && !VAL(V,5) ) nb_cc --;
    }
    if ( !VAL(V,3) ) {
	nb_cc ++;
	if ( !VAL(V,1) && !VAL(V,0) ) nb_cc --;
    }

    if ( nb_cc == 1 ) return( 1 );
    
    return( 0 );

}

typedef struct local_par {
  vt_names names;
  int iterations;
  float pourcentage;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-i %d] [-p %f]\n\
\t [-inv] [-swap] [-v] [-D] [-help]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
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
 $Revision: 1.3 $ $Date: 2000/10/18 08:45:59 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image, imres;
	double init;
	double max = 2147483647; /* (2^31)-1 */
	register int x, y, dx, dy, i;
	unsigned char *theBuf, *resBuf;
	int v[8];



	(void)srandom( time(0) ); 

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

	/* tests */
	if ( (image->type != UCHAR) || (image->dim.z != 1) ) {
	   VT_FreeImage( image );
	   VT_Free( (void**)&image );
	   VT_ErrorParse("bad input image\n", 0);
	}

	/*--- initialisation de l'image resultat ---*/
        VT_Image( &imres );
        VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, image->type );
        if ( VT_AllocImage( &imres ) != 1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate output image\n", 0);
        }

	(void)VT_CopyImage( image, &imres );
	dx = image->dim.x; dy = image->dim.y;
	for ( i=0; i<par.iterations; i++) {
	  theBuf = (unsigned char*)image->buf;
	  resBuf = (unsigned char*)imres.buf;
	  for ( y = 0; y < dy; y ++ )
	  for ( x = 0; x < dx; x ++, theBuf++, resBuf++ ) {
	    if ( (y==0) || (y==dy-1) || (x==0) || (x==dx-1) ) continue;
	    v[0] = *(theBuf - dx - 1); v[1] = *(theBuf - dx); v[2] = *(theBuf - dx + 1);
	    v[3] = *(theBuf - 1); v[4] = *(theBuf + 1); 
	    v[5] = *(theBuf + dx - 1); v[6] = *(theBuf + dx); v[7] = *(theBuf + dx + 1);
	    if ( VT_IsSimple2D( v ) != 1 ) continue;
	    v[0] = *(resBuf - dx - 1); v[1] = *(resBuf - dx); v[2] = *(resBuf - dx + 1);
	    v[3] = *(resBuf - 1); v[4] = *(resBuf + 1); 
	    v[5] = *(resBuf + dx - 1); v[6] = *(resBuf + dx); v[7] = *(resBuf + dx + 1);
	    if ( VT_IsSimple2D( v ) != 1 ) continue;
	    init = random();
	    if ( init/max > par.pourcentage ) continue;
	    if ( *theBuf == 255 ) *resBuf = 0;
	    if ( *theBuf == 0 ) *resBuf = 255;
	  }
	  (void)VT_CopyImage( &imres, image );
	}
	
	/*--- ecriture de l'image resultat ---*/
	if ( par.names.inv == 1 )  VT_InverseImage( &imres );
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
	    /* */
	    else if ( strcmp ( argv[i], "-i" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -i...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->iterations) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -i...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-p" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -p...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->pourcentage) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -p...\n", 0 );
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
	par->iterations = 1;
	par->pourcentage = 0.5;
}
