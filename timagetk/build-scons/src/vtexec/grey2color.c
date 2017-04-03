/*************************************************************************
 * grey2color -
 *
 * $Id: grey2color.c,v 1.10 2006/04/14 08:37:38 greg Exp $
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
 * - Wed Aug 16 16:30:11 MET DST 2000, Gregoire Malandain
 *   ajout du type FLOAT, pour le passage de 3 images a 
 *   une image vectorielle ou une image 3 plans.
 *
 * - Fri Mar 31 14:24:42 MET DST 2000, Gregoire Malandain
 *   ajout des tables de longueur 65536 pour les images sur 2 octets;
 *
 *
 */

#include <vt_common.h>

typedef enum {
  _TYPE_UNKNOWN = 0,
  VT_VECTORIELLE = 1,
  VT_3PLANS      = 2
} typeImage;

typedef enum {
  VT_GREY2COLOR = 1,
  VT_COLOR2GREY = 2,
  VT_COLOR2RGB  = 3,
  VT_COLOR2DEF  = 4,
  VT_RGB2COLOR  = 5
} typeOperation;

#define VT_COEFF_RED   0.2125
#define VT_COEFF_GREEN 0.7154
#define VT_COEFF_BLUE  0.0721
/*--- il existe aussi 0.299 0.587 0.114 ---*/

typedef struct local_par {
    vt_names names;
    vt_fpt coeff_c2g;
    typeImage type_image;
    typeOperation type_operation;
    char component[3][STRINGLENGTH];
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );
static int _VT_Grey2Color( vt_image *resIm, vt_image *theIm, vt_image *table );
static int _VT_Color2Grey( vt_image *resIm, vt_image *theIm, vt_fpt *coeff );



static char *usage = "[image-in | -r red-im -g green-im -b blue-im] [image-out]\n\
\t [-tc color-table] [-c2g %f %f %f] [-vect|-z] [-rgb|-def] [-v] [-D] [-help]";

static char *detail = "\
This program convert a grey-level image into a color image, and inverse.\n\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -r %s -g %s -b %s : combine 3 images into a color image.\n\
\t -tc %s : %s is an inrimage color image (x=256, y=3, z=1, unsigned char)\n\
\t     1st line = red, 2nd line = green, 3rd line = blue\n\
\t     if specified, convert grey-levels into colors.\n\
\t     the input image must to be not vectorial.\n\
\t -vect : the output color image will be vectorial.\n\
\t -z : the output color image will be on 3 planes (z=3).\n\
\t      This is the default, unless the input image is already a 3D\n\
\t      image (in this case, it will be vectorial).\n\
\t -rgb : convert a color image (vectorial) into 3 images.\n\
\t -c2g %f %f %f : coefficents to convert colors into grey-levels.\n\
\t      the grey level is a linear combination of R, G, B components\n\
\t      respectively multiplied by each cofficients.\n\
\t      Default is 0.2125, 0.7154, 0.0721.\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
If no options are specified, the program considers the input image as a color\n\
image and try to convert it into a grey-level image\n\
\n\
 $Revision: 1.10 $ $Date: 2006/04/14 08:37:38 $ $Author: greg $\n";


static char program[STRINGLENGTH];


int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, *table, imres;
  int x,y,z;


  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );










  if ( par.type_operation == VT_RGB2COLOR ) {
    

    /*--- lecture red ---*/
    image = _VT_Inrimage( par.component[0] );
    if ( image == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read red image\n", 0);
    
    if ( image->dim.z > 1 ) {
      if ( par.type_image == VT_3PLANS ) { 
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to deal with 3D red image\n", 0);
      }
      par.type_image = VT_VECTORIELLE;
    }




    /*--- initialisation output : attention il faut prendre par.names.in ---*/
    switch ( par.type_image ) {
    case VT_VECTORIELLE :
      VT_InitFromImage( &imres, image, par.names.in, image->type );
      imres.dim.v = 3;
      break;
    case VT_3PLANS :
    default :
      VT_InitFromImage( &imres, image, par.names.in, image->type );
      imres.dim.z = 3;
    }
    
    /*--- allocation de l'image resultat ---*/	
    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate output image\n", 0);
    }	  




    
    if ( (_VT_DEBUG_ != False) || (_VT_VERBOSE_ != False) )
      fprintf(stderr," ... incorporating red image into result ...");
    
    /*--- transfert red ---*/
    switch ( image->type ) {
    default :
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres  );
      VT_ErrorParse("such type not handled yet (red)\n", 0 );
    case UCHAR :
      {
	u8 ***theBuf = (u8***)image->array;
	u8 ***resBuf = (u8***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[0][y][x] = theBuf[0][y][x];
	}
      }
      break;
    case USHORT :
      {
	u16 ***theBuf = (u16***)image->array;
	u16 ***resBuf = (u16***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[0][y][x] = theBuf[0][y][x];
	}
      }
      break;
    case FLOAT :
      {
	r32 ***theBuf = (r32***)image->array;
	r32 ***resBuf = (r32***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[0][y][x] = theBuf[0][y][x];
	}
      }
    }

    VT_FreeImage( image );
    VT_Free( (void**)&image );
    
    if ( (_VT_DEBUG_ != False) || (_VT_VERBOSE_ != False) )
      fprintf(stderr," done.\n" );
	  







    /*--- lecture green ---*/
    image = _VT_Inrimage( par.component[1] );
    if ( image == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read green image\n", 0);
    if ( image->type != imres.type ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
      VT_ErrorParse("bad type for green image\n", 0);
    }
    
    if ( (image->dim.x != imres.dim.x) || (image->dim.y != imres.dim.y) ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
      VT_ErrorParse("bad X, Y dimensions for green image\n", 0);
    }
    
    switch ( par.type_image ) {
    case VT_VECTORIELLE :
      if ( image->dim.z != imres.dim.z ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres );
	VT_ErrorParse("bad Z dimensions for green image\n", 0);
      }
      break;
    case VT_3PLANS :
    default :
      if ( image->dim.z != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres );
	VT_ErrorParse("bad Z dimensions for green image\n", 0);
      }
      break;
    }





    if ( (_VT_DEBUG_ != False) || (_VT_VERBOSE_ != False) )
      fprintf(stderr," ... incorporating green image into result ...");
    
    /*--- transfert green ---*/
    switch ( image->type ) {
    default :
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres  );
      VT_ErrorParse("such type not handled yet (blue)\n", 0 );
    case UCHAR :
      {
	u8 ***theBuf = (u8***)image->array;
	u8 ***resBuf = (u8***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x+1] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[1][y][x] = theBuf[0][y][x];
	}
      }
      break;
    case USHORT :
      {
	u16 ***theBuf = (u16***)image->array;
	u16 ***resBuf = (u16***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x+1] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[1][y][x] = theBuf[0][y][x];
	}
      }
      break;
    case FLOAT :
      {
	r32 ***theBuf = (r32***)image->array;
	r32 ***resBuf = (r32***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x+1] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[1][y][x] = theBuf[0][y][x];
	}
      }
    }


	  
    if ( (_VT_DEBUG_ != False) || (_VT_VERBOSE_ != False) )
      fprintf(stderr," done.\n" );
    


    /*--- lecture blue ---*/
    image = _VT_Inrimage( par.component[2] );
    if ( image == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read blue image\n", 0);
    if ( image->type != imres.type ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
      VT_ErrorParse("bad type for blue image\n", 0);
    }
    

    if ( (image->dim.x != imres.dim.x) || (image->dim.y != imres.dim.y) ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
      VT_ErrorParse("bad X, Y dimensions for blue image\n", 0);
    }
    
    switch ( par.type_image ) {
    case VT_VECTORIELLE :
      if ( image->dim.z != imres.dim.z ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres );
	VT_ErrorParse("bad Z dimensions for blue image\n", 0);
      }
      break;
    case VT_3PLANS :
    default :
      if ( image->dim.z != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres );
	VT_ErrorParse("bad Z dimensions for blue image\n", 0);
      }
      break;
    }
    


    if ( (_VT_DEBUG_ != False) || (_VT_VERBOSE_ != False) )
      fprintf(stderr," ... incorporating blue image into result ...");
    
    /*--- transfert blue ---*/
    switch ( image->type ) {
    default :
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres  );
      VT_ErrorParse("such type not handled yet (blue)\n", 0 );
    case UCHAR :
      {
	u8 ***theBuf = (u8***)image->array;
	u8 ***resBuf = (u8***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x+2] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[2][y][x] = theBuf[0][y][x];
	}
      }
      break;
    case USHORT :
      {
	u16 ***theBuf = (u16***)image->array;
	u16 ***resBuf = (u16***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x+2] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[2][y][x] = theBuf[0][y][x];
	}
      }
      break;
    case FLOAT :
      {
	r32 ***theBuf = (r32***)image->array;
	r32 ***resBuf = (r32***)imres.array;
	switch ( par.type_image ) {
	case VT_VECTORIELLE :
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[z][y][3*x+2] = theBuf[z][y][x];
	  break;
	case VT_3PLANS :
	default :
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ )
	    resBuf[2][y][x] = theBuf[0][y][x];
	}
      }
    }


    
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    
    if ( (_VT_DEBUG_ != False) || (_VT_VERBOSE_ != False) )
      fprintf(stderr," done.\n" );
    

    
    /*--- ecriture de l'image resultat ---*/
    if ( VT_WriteInrimage( &imres ) == -1 ) {
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to write output image\n", 0);
    }
    
    /*--- liberations memoires ---*/
    VT_FreeImage( &imres );
    return( 1 );
  }











  /* on a traite le cas ( par.type_operation == VT_RGB2COLOR )
     on traite maintenant les autres cas 
  */









  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );











	
  switch ( par.type_operation ) {
  case VT_COLOR2RGB :
  case VT_COLOR2DEF :

    /*--- verifications ---*/
    if ( image->dim.v == 3 ) {
      par.type_image = VT_VECTORIELLE;
    } else {
      if ( image->dim.v != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to convert color image to rgb ones\n", 0);
      }
      if ( image->dim.z != 3 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to convert color image to rgb ones\n", 0);
      }
      par.type_image = VT_3PLANS;
    }







    if ( par.type_image == VT_3PLANS ) {
      VT_InitFromImage( &imres, image, par.names.out, image->type );
      imres.dim.v = 3;
      imres.dim.z = 1;
      /*--- allocation de l'image resultat ---*/	
      if ( VT_AllocImage( &imres ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate output image\n", 0);
      }

      switch( image->type ) {
      default :
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres  );
	VT_ErrorParse("such type not handled yet (red)\n", 0 );
      case UCHAR :
	{
	  u8 ***theBuf = (u8***)(image->array);
	  u8 ***resBuf = (u8***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[0][y][3*x+z] = theBuf[z][y][x];
	      }
	}
	break;
      }
      
    }
    else {


      /* cas VT_COLOR2RGB ou l'image d'entree peut etre 3D
       */
      
      
      VT_InitFromImage( &imres, image, "", image->type );
      imres.dim.v = 1;
      /*--- allocation de l'image resultat ---*/	
      if ( VT_AllocImage( &imres ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate output image\n", 0);
      }
      
      /*--- red ---*/
      switch ( par.type_operation ) {
      default :
      case VT_COLOR2RGB :
	/* sprintf( imres.name, "%s.red", par.names.out ); */
	sprintf( imres.name, "%s.r.inr", par.names.out );
	break;
      case VT_COLOR2DEF :
	sprintf( imres.name, "%s.x", par.names.out );
      }
      
      
      switch( image->type ) {
      default :
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres  );
	VT_ErrorParse("such type not handled yet (red)\n", 0 );
      case UCHAR :
	{
	  u8 ***theBuf = (u8***)(image->array);
	  u8 ***resBuf = (u8***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x];
	      }
	}
	break;
      case USHORT :
	{
	  u16 ***theBuf = (u16***)(image->array);
	  u16 ***resBuf = (u16***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x];
	      }
	}
	break;
      case FLOAT :
	{
	  r32 ***theBuf = (r32***)(image->array);
	  r32 ***resBuf = (r32***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x];
	      }
	}
	break;
      }
      (void)VT_WriteInrimage( &imres );
      
      
      
      /*--- green ---*/
      switch ( par.type_operation ) {
      default :
      case VT_COLOR2RGB :
	/* sprintf( imres.name, "%s.green", par.names.out ); */
	sprintf( imres.name, "%s.g.inr", par.names.out );
	break;
      case VT_COLOR2DEF :
	sprintf( imres.name, "%s.y", par.names.out );
      }
      
      
      switch( image->type ) {
      default :
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres  );
	VT_ErrorParse("such type not handled yet (green)\n", 0 );
      case UCHAR :
	{
	  u8 ***theBuf = (u8***)(image->array);
	  u8 ***resBuf = (u8***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x+1];
	      }
	}
	break;
      case USHORT :
	{
	  u16 ***theBuf = (u16***)(image->array);
	  u16 ***resBuf = (u16***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x+1];
	      }
	}
	break;
      case FLOAT :
	{
	  r32 ***theBuf = (r32***)(image->array);
	  r32 ***resBuf = (r32***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x+1];
	      }
	}
	break;
      }
      (void)VT_WriteInrimage( &imres );
      
      
      
      /*--- blue ---*/
      switch ( par.type_operation ) {
      default :
      case VT_COLOR2RGB :
	/* sprintf( imres.name, "%s.blue", par.names.out ); */
	sprintf( imres.name, "%s.b.inr", par.names.out );
	break;
      case VT_COLOR2DEF :
	sprintf( imres.name, "%s.z", par.names.out );
      }
      
      
      switch( image->type ) {
      default :
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres  );
	VT_ErrorParse("such type not handled yet (blue)\n", 0 );
      case UCHAR :
	{
	  u8 ***theBuf = (u8***)(image->array);
	  u8 ***resBuf = (u8***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x+2];
	      }
	}
	break;
      case USHORT :
	{
	  u16 ***theBuf = (u16***)(image->array);
	  u16 ***resBuf = (u16***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x+2];
	      }
	}
	break;
      case FLOAT :
	{
	  r32 ***theBuf = (r32***)(image->array);
	  r32 ***resBuf = (r32***)(imres.array);
    for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
        for ( x = 0; x < (int)image->dim.x; x ++ ) {
		resBuf[z][y][x] = theBuf[z][y][3*x+2];
	      }
	}
	break;
      }
    }

    (void)VT_WriteInrimage( &imres );
    
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_FreeImage( &imres );
    return( 1 );
    
    break;







    
  case VT_COLOR2GREY :
    /*
    if ( image->type != UCHAR ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("bad type for input image (case VT_COLOR2GREY)\n", 0);
    }
    */
    /*--- verifications ---*/
    if ( image->dim.v == 3 ) {
      par.type_image = VT_VECTORIELLE;
      VT_InitFromImage( &imres, image, par.names.out, image->type );
      VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, image->type );

    } else if ( (image->dim.v == 1) && (image->dim.z == 3) ) {
      par.type_image = VT_3PLANS;
      VT_InitFromImage( &imres, image, par.names.out, image->type );
      imres.dim.z = 1;
    } else {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to convert color image to grey-levels one\n", 0);
    }

    /*--- allocation de l'image resultat ---*/	
    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate output image\n", 0);
    }
    
    /*--- calcul ---*/	
    if ( _VT_Color2Grey( &imres, image, &(par.coeff_c2g) ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to convert color image to grey one (_VT_Color2Grey returns)\n", 0);
    }
    
    break;
    










  case VT_GREY2COLOR :
  if ( image->type != UCHAR && image->type != USHORT ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("bad type for input image (case VT_GREY2COLOR)\n", 0);
  }
    /*--- verifications ---*/
    if ( image->dim.v != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to convert grey-levels image to color one\n", 0);
    }
    if ( image->dim.z != 1 ) {
      if ( par.type_image == VT_3PLANS ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to convert grey-levels image to color one\n", 0);
      } else {
	par.type_image = VT_VECTORIELLE;
      }
    } else {
      if ( par.type_image == _TYPE_UNKNOWN ) par.type_image = VT_3PLANS;
    }
    
    /*--- lecture de la table couleur ---*/
    table = _VT_Inrimage( par.names.ext );
    if ( (table->type != UCHAR) || (table->dim.y != 3)  ||
	 (table->dim.z != 1) || (table->dim.v != 1) ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( table );
      VT_Free( (void**)&table );
      VT_ErrorParse("bad color table\n", 0);
    }
    if ( ((image->type == UCHAR) && (table->dim.x < 256)) ||
	 ((image->type == USHORT) && (table->dim.x < 65536)) ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( table );
      VT_Free( (void**)&table );
      VT_ErrorParse("color table not adapted to input image\n", 0);
    }
	 
    /*--- initialisation de l'image resultat ---*/
    switch ( par.type_image ) {
    case VT_VECTORIELLE :
      VT_InitFromImage( &imres, image, par.names.out, UCHAR );
      imres.dim.v = 3;
      break;
    case VT_3PLANS :
      VT_InitFromImage( &imres, image, par.names.out, UCHAR );
      imres.dim.z = 3;
      break;
    default :
      VT_ErrorParse("bad typeImage\n", 0);
    }
    
    /*--- allocation de l'image resultat ---*/	
    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( table );
      VT_Free( (void**)&table );
      VT_ErrorParse("unable to allocate output image\n", 0);
    }

    /*--- calcul ---*/	
    if ( _VT_Grey2Color( &imres, image, table ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( table );
      VT_Free( (void**)&table );
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to convert grey image to color one\n", 0);
    }
    VT_FreeImage( table );
    VT_Free( (void**)&table );
    
    break;
    
  default :
    VT_ErrorParse("bad typeOperation\n", 0);
    
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
    int check_r, check_g, check_b;
    char text[STRINGLENGTH];
    
    check_r = check_g = check_b = 0;

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
	    /*--- 3 images ---*/
	    else if ( strcmp ( argv[i], "-r" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -r...\n", 0 );
		strncpy( par->component[0], argv[i], STRINGLENGTH );
		par->type_operation = VT_RGB2COLOR;
		check_r = 1;
	    }
	    else if ( strcmp ( argv[i], "-g" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -g...\n", 0 );
		strncpy( par->component[1], argv[i], STRINGLENGTH );
		par->type_operation = VT_RGB2COLOR;
		check_g = 1;
	    }
	    else if ( strcmp ( argv[i], "-b" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -b...\n", 0 );
		strncpy( par->component[2], argv[i], STRINGLENGTH );
		par->type_operation = VT_RGB2COLOR;
		check_b = 1;
	    }	      
	    /*--- vectorielle ou non ---*/
	    else if ( strcmp ( argv[i], "-vect" ) == 0 ) {
		par->type_image = VT_VECTORIELLE;
	    }
	    else if ( strcmp ( argv[i], "-z" ) == 0 ) {
		par->type_image = VT_3PLANS;
	    }
	    /*--- type de conversion ---*/
	    else if ( strcmp ( argv[i], "-rgb" ) == 0 ) {
		par->type_operation = VT_COLOR2RGB;
	    }
	    /*--- type de conversion ---*/
	    else if ( strcmp ( argv[i], "-def" ) == 0 ) {
		par->type_operation = VT_COLOR2DEF;
	    }
	    else if ( strcmp ( argv[i], "-tc" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -tc...\n", 0 );
		strncpy( par->names.ext, argv[i], STRINGLENGTH );
		par->type_operation = VT_GREY2COLOR;
	    }
	    else if ( strcmp ( argv[i], "-c2g" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -c2g...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->coeff_c2g.x) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -c2g...\n", 0 );
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -c2g...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->coeff_c2g.y) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -c2g...\n", 0 );
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -c2g...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->coeff_c2g.z) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -c2g...\n", 0 );
  		par->type_operation = VT_COLOR2GREY;
	    }
	    /*--- option inconnue ---*/
	    else {
		sprintf(text,"unknown option %s\n",argv[i]);
		VT_ErrorParse(text, 0);
	    }
	}
	/*--- noms d'image ---*/
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

    /*--- standard input et output ---*/
    if (nb == 0) {
	strcpy( par->names.in,  "<" );  /* standart input */
	strcpy( par->names.out, ">" );  /* standart output */
    }
    if (nb == 1)
	strcpy( par->names.out, ">" );  /* standart output */

    /*--- ---*/
    if ( par->type_operation == VT_RGB2COLOR ) {
      if ( check_r != 1 )
	VT_ErrorParse("red component not present\n", 0 );
      if ( check_g != 1 )
	VT_ErrorParse("green component not present\n", 0 );
      if ( check_b != 1 )
	VT_ErrorParse("blue component not present\n", 0 );
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
        int i;
	VT_Names( &(par->names) );
	par->coeff_c2g.x = (float)VT_COEFF_RED;
	par->coeff_c2g.y = (float)VT_COEFF_GREEN;
	par->coeff_c2g.z = (float)VT_COEFF_BLUE;
	par->type_image = _TYPE_UNKNOWN;
	par->type_operation = VT_COLOR2GREY;
	for ( i = 0; i < STRINGLENGTH; i++ )
	  par->component[0][i] = par->component[1][i] = par->component[2][i] = '\0';
}








static int _VT_Grey2Color( vt_image *resIm, vt_image *theIm, vt_image *table )
{
  register int i, j=0, d, out_offset = 0;
  typeImage type_image = _TYPE_UNKNOWN;
  unsigned char *out_red, *out_green,  *out_blue;
  unsigned char *table_red, *table_green,  *table_blue;

    /*--- tests generaux ---*/
    if ( VT_Test1VImage( theIm, "_VT_Grey2Color" ) == -1 ) return( -1 );
    if ( VT_Test1VImage( resIm, "_VT_Grey2Color" ) == -1 ) return( -1 );
    if ( VT_Test1Image( table, "_VT_Grey2Color" ) == -1 ) return( -1 );

    /*--- tests input/output ---*/
    if ( (resIm->dim.x != theIm->dim.x) ||
	 (resIm->dim.y != theIm->dim.y) ) {
	VT_Error("incompatible dimensions of images", "_VT_Grey2Color");
	return( -1 );
    }
    if ( (theIm->dim.z == 1) && (resIm->dim.z == 3) ) {
	if ( (theIm->dim.v == 1) && (resIm->dim.v == 1) ) {
	    type_image = VT_3PLANS;
	} else {
	    VT_Error("incompatible dimensions of images", "_VT_Grey2Color");
	    return( -1 );
	}
    } else if ( theIm->dim.z != resIm->dim.z ) {
	VT_Error("incompatible dimensions of images", "_VT_Grey2Color");
	return( -1 );
    } else if ( (theIm->dim.v == 1) && (resIm->dim.v == 3) ) {
	type_image = VT_VECTORIELLE;
    } else {
	VT_Error("incompatible dimensions of images", "_VT_Grey2Color");
	return( -1 );
    }
    if ( (theIm->type != UCHAR) && (theIm->type != USHORT) ) {
      VT_Error("bad type for input image", "_VT_Grey2Color");
	return( -1 );
    }
    if ( (resIm->type != UCHAR) ) {
      VT_Error("bad type for output image", "_VT_Grey2Color");
	return( -1 );
    }
    /*--- test table de couleur ---*/
    if ( ((table->type == UCHAR) && (table->dim.x < 256)) 
	 || ((table->type == USHORT) && (table->dim.x < 65536)) 
	 || (table->type != UCHAR)
	 || (table->dim.y != 3)  ||
	 (table->dim.z != 1) || (table->dim.v != 1) ) {
	VT_Error("bad color table", "_VT_Grey2Color");
	return( -1 );
    }

    /*--- initialisations ---*/
    d = theIm->dim.x * theIm->dim.y * theIm->dim.z;


    table_red    = (unsigned char*)(table->buf);
    table_green  = table_red;
    table_green += table->dim.x;
    table_blue   = table_red;
    table_blue  += 2*table->dim.x;
    
    out_red = out_green = out_blue = (unsigned char*)NULL;
    switch ( type_image ) {
    case VT_3PLANS :
	out_red   = (unsigned char*)(resIm->buf);
	out_green = out_red + (theIm->dim.x * theIm->dim.y);
	out_blue  = out_red + 2 * (theIm->dim.x * theIm->dim.y);
	out_offset = 1;
	break;
    case VT_VECTORIELLE :
	out_red   = (unsigned char*)(resIm->buf);
	out_green = out_red + 1;
	out_blue  = out_red + 2;
	out_offset = 3;
	break;
    default :
      VT_ErrorParse("bad typeImage\n", 0);
    }
	
    /*--- c'est parti ---*/
    switch ( theIm->type ) {
    case UCHAR :
      {
	unsigned char* theBuf = (unsigned char*)(theIm->buf);
	for ( i = 0; i < d; i++, out_red += out_offset, out_green += out_offset, out_blue += out_offset ) {
	  j = (int)(*theBuf++);
	  *out_red   = table_red[j];
	  *out_green = table_green[j];
	  *out_blue  = table_blue[j];
	}
      }
      break;
    case USHORT :
      {
	unsigned short int * theBuf = (unsigned short int*)(theIm->buf);
	for ( i = 0; i < d; i++, out_red += out_offset, out_green += out_offset, out_blue += out_offset ) {
	  j = (int)(*theBuf++);
	  *out_red   = table_red[j];
	  *out_green = table_green[j];
	  *out_blue  = table_blue[j];
	}
      }
      break;
    default :
      VT_ErrorParse("bad type Image\n", 0);
    }
    
    return( 1 );
}



static int _VT_Color2Grey( vt_image *resIm, vt_image *theIm, vt_fpt *coeff )
{
    register double val, c_red, c_green, c_blue;
    register int i, d, in_offset = 0;
    typeImage type_image = _TYPE_UNKNOWN;

    /*--- tests generaux ---*/
    if ( VT_Test1VImage( theIm, "_VT_Color2Grey" ) == -1 ) return( -1 );
    if ( VT_Test1VImage( resIm, "_VT_Color2Grey" ) == -1 ) return( -1 );
    
    /*--- tests input/output ---*/
    if ( (resIm->dim.x != theIm->dim.x) ||
	 (resIm->dim.y != theIm->dim.y) ) {
	VT_Error("incompatible dimensions of images", "_VT_Color2Grey");
	return( -1 );
    }
    if ( (resIm->dim.z == 1) && (theIm->dim.z == 3) ) {
	if ( (theIm->dim.v == 1) && (resIm->dim.v == 1) ) {
	    type_image = VT_3PLANS;
	} else {
	    VT_Error("incompatible dimensions of images", "_VT_Color2Grey");
	    return( -1 );
	}
    } else if ( theIm->dim.z != resIm->dim.z ) {
	VT_Error("incompatible dimensions of images", "_VT_Color2Grey");
	return( -1 );
    } else if ( (resIm->dim.v == 1) && (theIm->dim.v == 3) ) {
	type_image = VT_VECTORIELLE;
    } else {
	VT_Error("incompatible dimensions of images", "_VT_Color2Grey");
	return( -1 );
    }
    /*
    if ( (theIm->type != UCHAR) || (resIm->type != UCHAR) ) {
	VT_Error("bad type for both input and output images", "_VT_Color2Grey");
	return( -1 );
    }
    */
    if ( (theIm->type != resIm->type) ) {
      VT_Error("differents types for both input and output images", "_VT_Color2Grey");
      return( -1 );
    }

    /*--- tests coefficients ---*/
    c_red   = (double)coeff->x;
    c_green = (double)coeff->y;
    c_blue  = (double)coeff->z;
    val = c_red + c_green + c_blue;
    if ( val <= 0.0 ) {
	VT_Error("bad coefficients for conversion", "_VT_Color2Grey");
	return( -1 );
    }
    c_red   /= val;
    c_green /= val;
    c_blue  /= val;
    
    /*--- initialisations ---*/
    d = resIm->dim.x * resIm->dim.y * resIm->dim.z;
    switch (theIm->type) {
    default :
            VT_Error("image type not handled yet", "_VT_Color2Grey");
            return( -1 );
    case UCHAR :
      {
            register unsigned char *in_red, *in_green, *in_blue;
            register unsigned char *resBuf;
            resBuf = (unsigned char*)(resIm->buf);
            in_red = in_green = in_blue = (unsigned char*)NULL;
            switch ( type_image ) {
            case VT_3PLANS :
                in_red   = (unsigned char*)(theIm->buf);
                in_green = in_red + (theIm->dim.x * theIm->dim.y);
                in_blue  = in_red + 2 * (theIm->dim.x * theIm->dim.y);
                in_offset = 1;
                break;
            case VT_VECTORIELLE :
                in_red   = (unsigned char*)(theIm->buf);
                in_green = in_red + 1;
                in_blue  = in_red + 2;
                in_offset = 3;
                break;
            default :
              VT_ErrorParse("bad type_image\n", 0);
            }
            /*--- c'est parti ---*/
            for ( i = 0; i < d; i++, in_red += in_offset, in_green += in_offset, in_blue += in_offset ) {
                val = c_red * (double)(*in_red) + c_green * (double)(*in_green) + c_blue * (double)(*in_blue);
                *resBuf++ = (unsigned char)( (int)(val + 0.5) );
            }
      }
      break;
      case FLOAT :
          {
                register float *in_red, *in_green, *in_blue;
                register float *resBuf;
                resBuf = (float*)(resIm->buf);
                in_red = in_green = in_blue = (float*)NULL;
                switch ( type_image ) {
                case VT_3PLANS :
                    in_red   = (float*)(theIm->buf);
                    in_green = in_red + (theIm->dim.x * theIm->dim.y);
                    in_blue  = in_red + 2 * (theIm->dim.x * theIm->dim.y);
                    in_offset = 1;
                    break;
                case VT_VECTORIELLE :
                    in_red   = (float*)(theIm->buf);
                    in_green = in_red + 1;
                    in_blue  = in_red + 2;
                    in_offset = 3;
                    break;
                default :
                  VT_ErrorParse("bad type_image\n", 0);
                }
                /*--- c'est parti ---*/
                for ( i = 0; i < d; i++, in_red += in_offset, in_green += in_offset, in_blue += in_offset ) {
                    val = c_red * (double)(*in_red) + c_green * (double)(*in_green) + c_blue * (double)(*in_blue);
                    *resBuf++ = val;
                }
          }
      break;
    }





    return( 1 );
}
