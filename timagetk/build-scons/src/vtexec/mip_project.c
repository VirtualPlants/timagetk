/*************************************************************************
 * mip_project.c -
 *
 * $Id: mip_project.c,v 1.8 2002/12/11 12:06:44 greg Exp $
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

#include <vt_mip.h>

typedef enum {
  _FIRST_,
  _MIN_,
  _MAX_,
  _THRESHOLDED_MAX_,
  _ANIM_
} enumProjection;



typedef struct local_par {
  vt_names names;
  int views;
  int iview;
  enumProjection typeProjection;
  enumDirection  typeDirection;
  enumInterpolation typeInterpolation;
  float rotationDir[3];
  float max;
} local_par;



/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-min] [-first]\n\
\t [-anim] [-x|-y|-z] [-views %d] [-iview %d] [-linear|-nearest]\n\
\t [-rot %f %f %f]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t Calcule les projections MIP de l'image\n\
\t si 'image-in' est '-', on prendra stdin\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -min : projection de l'intensite minimum\n\
\t -anim : creation d'une animation\n\
\t -views %d : nombre de vues de l'animation\n\
\t -iview %d : indice de la seule vue calculee de l'animation\n\
\t -z : projection selon la direction Z\n\
\t      la premiere vue de l'animation est la vue MIP XY\n\
\t      l'axe de rotation est Y\n\
\t -y : projection selon la direction Y\n\
\t      la premiere vue de l'animation est la vue MIP XZ\n\
\t      l'axe de rotation est Z\n\
\t -x : projection selon la direction X\n\
\t      la premiere vue de l'animation est la vue MIP ZY\n\
\t      l'axe de rotation est Y\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n\
 $Revision: 1.8 $ $Date: 2002/12/11 12:06:44 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  char image_name[STRINGLENGTH];
  vt_image *image, imXY, imXZ, imZY;
  vt_image *theXY = (vt_image*)NULL;
  vt_image *theXZ = (vt_image*)NULL;
  vt_image *theZY = (vt_image*)NULL;
  vt_image imres;
  int dimz;

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );


  switch( par.typeProjection ) {
  default :
    break;
    
  case _ANIM_ :

    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read input image\n", 0);

    VT_Image( &imres );
    imres.siz.z = 1;
    dimz = par.views;
    if ( par.iview >= 0 && par.iview < par.views ) {
      dimz = 1;
    }



    switch ( par.typeDirection ) {
    default :
    case _Z_ :
      /* projection dans la direction Z :
	 la premiere vue est la projection MIP sur XY
	 l'axe de rotation est Y (par defaut)
      */
      VT_InitImage( &imres, par.names.out, 
		    (int)(sqrt(image->dim.x*image->dim.x+image->dim.z*image->dim.z))+1,
		    image->dim.y, 
		    dimz,
		    image->type );
      imres.siz.x = image->siz.x;
      imres.siz.y = image->siz.y;
      break;
    case _Y_ :
      /* projection dans la direction Y :
	 la premiere vue est la projection MIP sur XZ
	 l'axe de rotation est Z (par defaut)
      */
      VT_InitImage(  &imres, par.names.out, 
		     (int)(sqrt(image->dim.x*image->dim.x+image->dim.y*image->dim.y))+1,
		     image->dim.z, 
		     dimz,
		     image->type );
      imres.siz.x = image->siz.x;
      imres.siz.y = image->siz.z;
      break;
    case _X_ :
     /* projection dans la direction X :
	 la premiere vue est la projection MIP sur ZY
	 l'axe de rotation est Y (par defaut)
      */
      VT_InitImage(  &imres, par.names.out, 
		     (int)(sqrt(image->dim.y*image->dim.y+image->dim.z*image->dim.z))+1,
		     image->dim.y, 
		     dimz,
		     image->type );
      imres.siz.x = image->siz.z;
      imres.siz.y = image->siz.y;
      break;
    }

    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate output image\n", 0);
    }

    if ( par.iview >= 0 && par.iview < par.views ) {
      int theDim[3];
      int resDim[2];
      theDim[0] = image->dim.x;
      theDim[1] = image->dim.y;
      theDim[2] = image->dim.z;
      resDim[0] = imres.dim.x;
      resDim[1] = imres.dim.y;
      fprintf( stderr, "... compute view %d/%d\n", par.iview, par.views );
      if ( VT_OneViewMIP( image->buf, theDim, imres.buf, resDim,
			  NULL, NULL, image->type,
			  par.iview * 2.0 * 3.1415926536 / (double)par.views,
			  par.rotationDir,
			  par.typeDirection, par.typeInterpolation ) != 1 ) {
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
      }
    }
    else {
      if ( VT_AnimMIP( image, &imres, par.rotationDir,
		       par.typeDirection, par.typeInterpolation ) != 1 ) {
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
      }
    }

    VT_FreeImage( image );
    VT_Free( (void**)&image );

    if ( VT_WriteInrimage( &imres ) == -1 ) {
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to write output image\n", 0);
    }

    VT_FreeImage( &imres );

    exit( 0 );

  }
	  




  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);

  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );
  


  /*--- initialisation des images resultat ---*/
  if ( par.typeDirection == _ALL_ || par.typeDirection == _Z_ ) {
    VT_Image( &imXY );
    if ( par.typeDirection == _ALL_ ) {
      if ( par.names.out[0] == '\0' ) sprintf( image_name, "%s.xymip", par.names.in );
      else                            sprintf( image_name, "%s.xymip.inr", par.names.out );
    } else {
      sprintf( image_name, "%s", par.names.out );
    }
    VT_InitImage( &imXY, image_name, image->dim.x, image->dim.y, (int)1, image->type );
    imXY.siz.x = image->siz.x;
    imXY.siz.y = image->siz.y;
    theXY = &imXY;
    if ( VT_AllocImage( &imXY ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate first output image\n", 0);
    }
  }

  if ( par.typeDirection == _ALL_ || par.typeDirection == _Y_ ) {
    VT_Image( &imXZ );
    if ( par.typeDirection == _ALL_ ) {
      if ( par.names.out[0] == '\0' ) sprintf( image_name, "%s.xzmip", par.names.in );
      else                            sprintf( image_name, "%s.xzmip.inr", par.names.out );
    } else {
      sprintf( image_name, "%s", par.names.out );
    }
    VT_InitImage( &imXZ, image_name, image->dim.x, image->dim.z, (int)1, image->type );
    imXZ.siz.x = image->siz.x;
    imXZ.siz.y = image->siz.z;
    theXZ = &imXZ;
    if ( VT_AllocImage( &imXZ ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate second output image\n", 0);
    }
  }

  if ( par.typeDirection == _ALL_ || par.typeDirection == _X_ ) {
    VT_Image( &imZY );
    if ( par.typeDirection == _ALL_ ) {
      if ( par.names.out[0] == '\0' ) sprintf( image_name, "%s.zymip", par.names.in );
      else                            sprintf( image_name, "%s.zymip.inr", par.names.out );
    } else {
      sprintf( image_name, "%s", par.names.out );
    }
    VT_InitImage( &imZY, image_name, image->dim.z, image->dim.y, (int)1, image->type );
    imZY.siz.x = image->siz.z;
    imZY.siz.y = image->siz.y;
    theZY = &imZY;
    if ( VT_AllocImage( &imZY ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate third output image\n", 0);
    }
  }
  


  

  /*--- calculs ---*/
  switch( par.typeProjection ) {
  default :
    break;
  case _FIRST_ :
    if ( VT_FIP( image, theXY, theXZ, theZY ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to compute output images\n", 0);
    }
    break;
  case _MAX_ :
    if ( VT_MIP( image, theXY, theXZ, theZY ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to compute output images\n", 0);
    }
    break;
  case _MIN_ :
    if ( VT_MinMIP( image, theXY, theXZ, theZY ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to compute output images\n", 0);
    }
    break;
  }



  /*--- ecriture des images resultat ---*/
  if ( par.typeDirection == _ALL_ || par.typeDirection == _Z_ ) {
    if ( VT_WriteInrimage( &imXY ) == -1 ) {
      VT_FreeImage( &imXY );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output images\n", 0);
    }
  }

  if ( par.typeDirection == _ALL_ || par.typeDirection == _Y_ ) {
    if ( VT_WriteInrimage( &imXZ ) == -1 ) {
      VT_FreeImage( &imXZ );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output images\n", 0);
    }
  }

  if ( par.typeDirection == _ALL_ || par.typeDirection == _X_ ) {
    if ( VT_WriteInrimage( &imZY ) == -1 ) {
      VT_FreeImage( &imZY );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output images\n", 0);
    }
  }


  /*--- liberation memoire ---*/
  VT_FreeImage( image );
  VT_Free( (void**)&image );

  exit( 0 );
}









static void VT_Parse( int argc, char *argv[], local_par *par )
{
    int i, nb;
    int status;
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
	    /*--- methode "lente" ---*/


	    else if ( strcmp ( argv[i], "-first" ) == 0 ) {
		par->typeProjection = _FIRST_;
	    }

	    else if ( strcmp ( argv[i], "-min" ) == 0 ) {
		par->typeProjection = _MIN_;
	    }
	    else if ( strcmp ( argv[i], "-anim" ) == 0 ) {
		par->typeProjection = _ANIM_;
	    }

	    
	    else if ( strcmp ( argv[i], "-linear" ) == 0 ) {
		par->typeInterpolation = _LINEAR_;
	    }
	    else if ( strcmp ( argv[i], "-nearest" ) == 0 ) {
		par->typeInterpolation = _NEAREST_;
	    }

	    
	    else if ( strcmp ( argv[i], "-x" ) == 0 ) {
	      par->typeDirection = _X_;
	    }
	    else if ( strcmp ( argv[i], "-y" ) == 0 ) {
	      par->typeDirection = _Y_;
	    }
	    else if ( strcmp ( argv[i], "-z" ) == 0 ) {
	      par->typeDirection = _Z_;
	    }


	    /*---  ---*/
	    else if ( strcmp ( argv[i], "-rot" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -rot...\n", 0 );
	      status = sscanf( argv[i],"%f",&(par->rotationDir[0]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -rot...\n", 0 );
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -rot...\n", 0 );
	      status = sscanf( argv[i],"%f",&(par->rotationDir[1]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -rot...\n", 0 );
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -rot...\n", 0 );
	      status = sscanf( argv[i],"%f",&(par->rotationDir[2]) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -rot...\n", 0 );
	    }




	    else if ( strcmp ( argv[i], "-views" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -views...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->views) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -views...\n", 0 );
	      par->typeProjection = _ANIM_;
	    }

	    else if ( strcmp ( argv[i], "-iview" ) == 0 ) {
	      i += 1;
	      if ( i >= argc)    VT_ErrorParse( "parsing -iview...\n", 0 );
	      status = sscanf( argv[i],"%d",&(par->iview) );
	      if ( status <= 0 ) VT_ErrorParse( "parsing -iview...\n", 0 );
	      par->typeProjection = _ANIM_;
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
    if (nb == 0)
	strcpy( par->names.in,  "<" );  /* standart input */
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

  par->typeProjection = _MAX_;
  par->typeDirection  = _ALL_;
  par->typeInterpolation = _LINEAR_;

  par->views = 24;
  par->iview = -1;

  par->rotationDir[0] = par->rotationDir[1] = par->rotationDir[2] = 0.0;

}
