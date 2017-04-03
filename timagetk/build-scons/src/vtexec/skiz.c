/*************************************************************************
 * skiz.c -
 *
 * $Id: skiz.c,v 1.3 2000/08/16 08:44:06 greg Exp $
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
#include <vt_skiz.h>
#include <vt_distance.h>

typedef enum {
  SKIZ_WITHOUT_MASK = 1,
  SKIZ_WITH_MASK = 2
} typeOperation;
    
typedef struct local_par {
  vt_names names;
  typeOperation operation;
  int chamferSize;
} local_par;


/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-mask image-mask]\n\
\t [-chamfer %d] [-distance image-distance]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
Squelette par zone d'influences.\n\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -mask %s : image de masque. La distance est alors calculee\n\
\t       a l'interieur du masque, et la propagation des etiquettes\n\
\t       se fait en 6-connexite.\n\
\t -chamfer %d : taille du masque du chanfrein (3 ou 5)\n\
\t -distance %s : ecrit l'image de distance dans %s\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s : short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imres, *immask;
  vt_image imx, imy, imz;
  vt_distance dpar;

  int inc[12];
  
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
  

  



  switch ( par.operation ) {
  case SKIZ_WITH_MASK :

    /*--- initialisation de l'image resultat ---*/
    VT_Image( &imres );
    VT_InitFromImage( &imres, image, par.names.out, image->type );

    if ( VT_AllocImage( &imres ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate output image\n", 0);
    }



    /* lecture de l'image masque 
     */
    
    immask = _VT_Inrimage( par.names.ext );
    if ( immask == (vt_image*)NULL ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to read mask image\n", 0);
    }
    /*
     * les coefficients sont donnees par (dx,dy,dz)
     * la 3eme dimension est traitee differement
     * ce qui permettrait de tenir compte d'une eventuelle anisotropie
     */
    switch ( par.chamferSize ) {
    case 3 :
    default :
      inc[0] = 16; /* les 4-voisins dans le plan (dx=1 ou dy=1) */
      inc[1] = 21; /* les 4-voisins dans le plan (dx=1 et dy=1) */

      inc[2] = 16; /* dz=1 et dx=0 et dy=0 */
      inc[3] = 21; /* dz=1 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[4] = 26; /* dz=1 et (dx=1 et dy=1) */
      break;
    case 5 :
      inc[0] = 13; /* dz=0 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[1] = 18; /* dz=0 et (dx=1 et dy=1) */
      inc[2] = 29; /* dz=0 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */

      inc[3] = 13; /* dz=1 et dx=0 et dy=0 */
      inc[4] = 18; /* dz=1 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[5] = 23; /* dz=1 et (dx=1 et dy=1) */
      inc[6] = 29; /* dz=1 et ( (dx=0 et dy=2) ou (dx=2 et dy=0) ) */
      inc[7] = 32; /* dz=1 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */
      inc[8] = 39; /* dz=1 et (dx=2 et dy=2) */

      inc[9] = 29; /* dz=2 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[10] = 32; /* dz=2 et (dx=1 et dy=1) */
      inc[11] = 39; /* dz=2 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */

    }
    
    if ( VT_ConstrainedSkizWithChamfer( image, immask, &imres, par.chamferSize, inc ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( &imres );
      VT_FreeImage( immask );
      VT_Free( (void**)&immask );
      VT_ErrorParse("error in computing\n", 0);
    }

    VT_FreeImage( immask );
    VT_Free( (void**)&immask );


    /*--- ecriture de l'image resultat ---*/
    if ( VT_WriteInrimage( &imres ) == -1 ) {
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output image\n", 0);
    }

    VT_FreeImage( &imres );
    
    break;

    


  case SKIZ_WITHOUT_MASK :
  default :

    /*--- calcul du vecteur pointant vers le plus proche point ---*/
    /*--- initialisation des images resultat ---*/
    VT_Image( &imx );
    VT_InitFromImage( &imx, image, par.names.out, SSHORT );
    VT_Image( &imy );
    VT_InitFromImage( &imy, image, par.names.out, SSHORT );
    VT_Image( &imz );
    VT_InitFromImage( &imz, image, par.names.out, SSHORT );
    (void)strcat( imx.name, ".x" );
    (void)strcat( imy.name, ".y" );
    (void)strcat( imz.name, ".z" );
    if ( VT_AllocImage( &imx ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate first output image\n", 0);
    }
    if ( VT_AllocImage( &imy ) != 1 ) {
      VT_FreeImage( &imx );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate first second image\n", 0);
    }
    if ( VT_AllocImage( &imz ) != 1 ) {
      VT_FreeImage( &imx );
      VT_FreeImage( &imy );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate first third image\n", 0);
    }
    

    
    VT_Distance( &dpar );
    dpar.seuil = 1;


    if ( VT_VecteurPPP_SS( image, &imx, &imy, &imz, &dpar ) != 1 ) {
      VT_FreeImage( &imx );
      VT_FreeImage( &imy );
      VT_FreeImage( &imz );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to compute nearest point\n", 0);
    }

    
    {
      s16 ***theX = (s16 ***)imx.array;
      s16 ***theY = (s16 ***)imy.array;
      s16 ***theZ = (s16 ***)imz.array;
      int x, y, z, i, j, k;

      switch ( image->type ) {
      case UCHAR :
	{
	  u8 ***theBuf = (u8 ***)image->array;
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ ) {
	    k = theZ[z][y][x];   j = theY[z][y][x];   i = theX[z][y][x];
	    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
	    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
	  }
	}
	break;
      case USHORT :
	{
	  u16 ***theBuf = (u16 ***)image->array;
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ ) {
	    k = theZ[z][y][x];   j = theY[z][y][x];   i = theX[z][y][x];
	    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
	    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
	  }
	}
      case SSHORT :
	{
	  s16 ***theBuf = (s16 ***)image->array;
    for ( z = 0; z < (int)image->dim.z; z ++ )
    for ( y = 0; y < (int)image->dim.y; y ++ )
    for ( x = 0; x < (int)image->dim.x; x ++ ) {
	    k = theZ[z][y][x];   j = theY[z][y][x];   i = theX[z][y][x];
	    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
	    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
	  }
	}
	break;
      default :
	VT_FreeImage( &imx );
	VT_FreeImage( &imy );
	VT_FreeImage( &imz );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("can not deal with such input image type\n", 0);
      }
      
    }
    
    VT_FreeImage( &imx );
    VT_FreeImage( &imy );
    VT_FreeImage( &imz );
    
    /*--- ecriture de l'image resultat ---*/
    (void)strcpy( image->name, par.names.out );
    if ( VT_WriteInrimage( image ) == -1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output image\n", 0);
    }

  }





  
  /*--- liberations memoires ---*/
  VT_FreeImage( image );
  VT_Free( (void**)&image );
  return( 1 );
}







static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  int radius;
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
      /*--- image de masque */
      else if ( strcmp ( argv[i], "-mask" ) == 0 ) {
	i += 1;
	strncpy( par->names.ext, argv[i], STRINGLENGTH );
	par->operation = SKIZ_WITH_MASK;
      }
      else if ( strcmp ( argv[i], "-chamfer" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -chamfer...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->chamferSize) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -chamfer...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-radius" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -radius...\n", 0 );
	status = sscanf( argv[i],"%d",&radius );
	if ( status <= 0 ) VT_ErrorParse( "parsing -radius...\n", 0 );
	VT_SetRadiusOfConstrainedSkiz( radius );
      }
      else if ( strcmp ( argv[i], "-distance" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -distance...\n", 0 );
	VT_SetSkizDistanceImageName( argv[i] );
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
  par->operation = SKIZ_WITHOUT_MASK;
  par->chamferSize = 3;
}
