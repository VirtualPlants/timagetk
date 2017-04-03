/*************************************************************************
 * Ext.c -
 *
 * $Id: Ext.c,v 1.8 2006/04/14 08:37:38 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Thu Jul  8 11:48:31 1999
 *
 * ADDITIONS, CHANGES
 *
 * - Thu Aug 31 13:51:37 MEST 2000, Gregoire Malandain
 *   Ajout des options -0 et -1 
 *
 * - Tue Apr 11 19:07:13 MET DST 2000, Gregoire Malandain
 *   propagation de la taille du voxel
 *
 * - Mon Mar 27 19:18:14 MET DST 2000, Gregoire Malandain
 *   extraction de slices
 *
 */

#include <extract.h>

#include <vt_common.h>
#include <vt_extract.h>

typedef enum {
  _XYSLICE_,
  _ALLXYSLICES_,
  _XZSLICE_,
  _YZSLICE_,
  _VOLUME_ 
} enumExtract;  


typedef struct local_par {
  int inrimageLike;
  vt_names names;
  vt_ipt corner;
  vt_ipt dim;
  enumExtract typeExtract;
} local_par;


/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );


static char *usage = "[image-in] [image-out]\n\
\t [-ix %d] [-iy %d] [-iz %d] [-x %d] [-y %d] [-z %d]\n\
\t [-xy %d] [-xz %d] [-yz %d] [-slices] [-0 | -1]\n\
\t [-v] [-help]";

static char *detail = "\
\n\
   Extrait une sous-image.\n\
\n\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -ix, -iy, -iz : origine (>= 1) de la fenetre dans 'image-in'\n\
\t -x, -y, -z    : taille de la fenetre\n\
\t -xy  : extraction of a XY slice as a 2-D image\n\
\t -xz  : extraction of a XZ slice as a 2-D image\n\
\t -yz  : extraction of a YZ slice as a 2-D image\n\
\t -slices : write the 3D image as a series of 2D images\n\
\t -1            : 'ext' like, l'origine de l'image est (1,1,1) [default]\n\
\t -0            : l'origine de l'image est (0,0,0)\n\
\n\
 $Revision: 1.8 $ $Date: 2006/04/14 08:37:38 $ $Author: greg $\n";

static char program[STRINGLENGTH];


int main( int argc, char *argv[] )
{
    local_par par;
    vt_image *image, imres;
    int z, l, el;
    char fmt[10];
    char ext[10];
    char num[10];
    char tmp[STRINGLENGTH];

    int thePt[3] = {0,0,0};
    int theDim[3];
    int resDim[3];


    fprintf( stderr, "Deprecated\n" );

    /*--- initialisation des parametres ---*/
    VT_InitParam( &par );
    /*--- lecture des parametres ---*/
    VT_Parse( argc, argv, &par );
    


    /*
     * to be "ext" like
     */
    if ( par.inrimageLike != 0 ) {
      par.corner.x --;
      par.corner.y --;
      par.corner.z --;
    }



    /*--- parametres ---*/
    if ( par.corner.x < 0 ) par.corner.x = 0;
    if ( par.corner.y < 0 ) par.corner.y = 0;
    if ( par.corner.z < 0 ) par.corner.z = 0;
    if ( par.dim.x <= 0 ) par.dim.x = 1;
    if ( par.dim.y <= 0 ) par.dim.y = 1;
    if ( par.dim.z <= 0 ) par.dim.z = 1;


    /*--- lecture de l'image d'entree ---*/
    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) 
	VT_ErrorParse("unable to read input image\n", 0);

    if ( ((par.corner.x + par.dim.x) > (int)image->dim.x) ||
   ((par.corner.y + par.dim.y) > (int)image->dim.y) ||
   ((par.corner.z + par.dim.z) > (int)image->dim.z) ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("invalid block dimensions or origin\n", 0);
    }

    /*--- initialisation de l'image resultat ---*/
    VT_Image( &imres );
    switch ( par.typeExtract ) {
    default :
    case _VOLUME_ :
      VT_InitImage( &imres, par.names.out, 
		    par.dim.x, par.dim.y, par.dim.z, image->type );
      imres.siz.x = image->siz.x;
      imres.siz.y = image->siz.y;
      imres.siz.z = image->siz.z;
      break;
    case _XYSLICE_ :
    case _ALLXYSLICES_ :
      VT_InitImage( &imres, par.names.out, 
		    image->dim.x, image->dim.y, 1, image->type );
      imres.siz.x = image->siz.x;
      imres.siz.y = image->siz.y;
      imres.siz.z = image->siz.z;
      break;
    case _XZSLICE_ :
      VT_InitImage( &imres, par.names.out, 
		    image->dim.x, image->dim.z, 1, image->type );
      imres.siz.x = image->siz.x;
      imres.siz.y = image->siz.z;
      imres.siz.z = image->siz.y;
      break;
    case _YZSLICE_ :
      VT_InitImage( &imres, par.names.out, 
		    image->dim.y, image->dim.z, 1, image->type );
      imres.siz.x = image->siz.y;
      imres.siz.y = image->siz.z;
      imres.siz.z = image->siz.x;
    }

    if ( VT_AllocImage( &imres ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate output image\n", 0);
    }

    
    
    theDim[0] = image->dim.x;
    theDim[1] = image->dim.y;
    theDim[2] = image->dim.z;
    
    resDim[0] = imres.dim.x;
    resDim[1] = imres.dim.y;
    resDim[2] = imres.dim.z;
    

  
    switch ( par.typeExtract ) { 
    default : 
	break;
    case _ALLXYSLICES_ :
      if ( image->dim.z < 10 ) l = 1;
      else if ( image->dim.z < 100 ) l = 2;
      else if ( image->dim.z < 1000 ) l = 3;
      else if ( image->dim.z < 10000 ) l = 4;
      else l = 5;
      sprintf( fmt, ".%%0%dd", l );
      l = strlen( par.names.out );
      el = 0;
      if ( strncmp( par.names.out + l - 4, ".inr", 4 ) == 0 ) {
	sprintf( ext, ".inr" );
	el = 4;
      }
      else if ( strncmp( par.names.out + l - 4, ".ppm", 4 ) == 0 ) {
	sprintf( ext, ".ppm" );
	el = 4;
      }
      else if ( strncmp( par.names.out + l - 4, ".pgm", 4 ) == 0 ) {
	sprintf( ext, ".pgm" );
	el = 4;
      }
      else {
	fprintf( stderr, "%s: extension of '%s' not recognized", 
		 program, par.names.out );
      }
      strncpy( tmp, par.names.out, l-el );
      for ( z=0; z<(int)image->dim.z; z++ ) {
	tmp[l-el] = '\0';
	if ( par.inrimageLike == 0 ) sprintf( num, fmt, z );
	else sprintf( num, fmt, z+1 );
	strcat( tmp, num );
	if ( el > 0 ) strcat( tmp, ext );
	/* printf( "%s %s %s %s\n", tmp, fmt, num, ext ); */
	thePt[2] = z;
	ExtractFromBuffer( image->buf, theDim, imres.buf, resDim, thePt, image->type );
	(void)VT_CopyName( imres.name, tmp );
	(void)VT_WriteInrimage( &imres );
      }

      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
    
      return( 1 );
    }


    switch ( par.typeExtract ) {
    default :
    case _VOLUME_ :
      thePt[0] = par.corner.x;
      thePt[1] = par.corner.y;
      thePt[2] = par.corner.z;
      break;
    case _XYSLICE_ :
      thePt[2] = par.corner.z;
      resDim[0] = image->dim.x;
      resDim[1] = image->dim.y;
      resDim[2] = 1;
      break;
    case _XZSLICE_ :
      thePt[1] = par.corner.y;
      resDim[0] = image->dim.x;
      resDim[1] = 1;
      resDim[2] = image->dim.z;
      break;
    case _YZSLICE_ :
      thePt[0] = par.corner.x;
      resDim[0] = 1;
      resDim[1] = image->dim.y;
      resDim[2] = image->dim.z;
    }
    
    ExtractFromBuffer( image->buf, theDim, imres.buf, resDim, thePt, image->type );

    if ( 0 ) {
      /*--- methode normale ---*/
      if ( VT_Extract( &imres, image, &(par.corner) ) != 1 ) {
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to extract sub-image\n", 0);
      }
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


		
		else if ( strcmp ( argv[i], "-1" ) == 0 ) {
		  par->inrimageLike = 1;
		}
		else if ( strcmp ( argv[i], "-0" ) == 0 ) {
		  par->inrimageLike = 0;
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


		else if ( strcmp ( argv[i], "-slices" ) == 0 ) {
		  par->typeExtract = _ALLXYSLICES_;
		}


		/*--- lecture du coin ---*/
		else if ( strncmp ( argv[i], "-yz", 3 ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -yz...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.x) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -yz...\n", 0 );
		    par->typeExtract = _YZSLICE_;
		}
		else if ( strncmp ( argv[i], "-xz", 3 ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -xz...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.y) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -xz...\n", 0 );
		    par->typeExtract = _XZSLICE_;
		}
		else if ( strncmp ( argv[i], "-xy", 3 ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -xy...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->corner.z) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -xy...\n", 0 );
		    par->typeExtract = _XYSLICE_;
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
	par->inrimageLike = 1;
	par->corner.x = par->corner.y = par->corner.z = 0;
	par->dim.x = par->dim.y = par->dim.z = 0;
	par->typeExtract = _VOLUME_;
}
