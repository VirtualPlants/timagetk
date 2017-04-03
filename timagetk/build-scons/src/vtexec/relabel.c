/*************************************************************************
 * removeCcOnBorder.c - met a zero les composantes connexes numerotees qui
 *                      "touchent" un bord (en X ou en Y)
 *
 * $Id: relabel.c,v 1.2 2000/03/28 07:28:05 greg Exp $
 *
 * DESCRIPTION: 
 *
 *
 *
 *
 *
 * AUTHOR:
 * Gregoire Malandain
 *
 * CREATION DATE:
 * May 1999
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 * - Mon Mar 27 21:32:00 MEST 2000, Gregoire Malandain
 *   Les labels en-dehors d'un cercle sont enleves.
 *
 *
 */

#include <vt_common.h>

typedef struct local_par {
  vt_names names;

  int removeLabelsOutsideXYCircle;
  double xycenter[2];
  double xyradius;

  int removeLabelsOnXBorder;
  int removeLabelsOnYBorder;
  int removeLabelsOnZBorder;
  int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out]\n\
\t [-border | -xborder | -yborder | -zborder]\n\
\t [-xycenter %lf %lf] [-xyradius %lf]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

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
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image;
  int x, y, z, i, j, n;
  int *v = (int*)NULL;
  double d, r;

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
  


  /* get max
   */
  n = 0;
  switch ( image->type ) {
  case UCHAR :
    {
      u8 *** theBuf = (u8 ***)image->array;
      for ( z=0; z<(int)image->dim.z ; z++ )
      for ( y=0; y<(int)image->dim.y ; y++ )
      for ( x=0; x<(int)image->dim.x ; x++ )
	if ( theBuf[z][y][x] > n ) n = theBuf[z][y][x];
    }
    break;
  case USHORT :
    {
      unsigned short int *** theBuf = (unsigned short int ***)image->array;
      for ( z=0; z<(int)image->dim.z ; z++ )
      for ( y=0; y<(int)image->dim.y ; y++ )
      for ( x=0; x<(int)image->dim.x ; x++ )
	if ( theBuf[z][y][x] > n ) n = theBuf[z][y][x];
    }
    break;
  default :
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse( "unable to deal with such image type\n", 0);
  }


  /* allocation du tableau
   */
  v = (int*)malloc( (n+1) * sizeof(int) );
  if ( v == (int*)NULL ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse( "unable to allocate auxiliary array\n", 0);
  }
  for (x=0; x<=n; x++ ) v[x] = 0;
  


  /* capture des labels
   */
  
  
  switch ( image->type ) {
  case USHORT :
    {
      unsigned short int *** theBuf = (unsigned short int ***)image->array;
      
      for ( z=0; z<(int)image->dim.z ; z++ )
      for ( y=0; y<(int)image->dim.y ; y++ )
      for ( x=0; x<(int)image->dim.x ; x++ )
	v[ theBuf[z][y][x] ] = theBuf[z][y][x];


      if ( par.removeLabelsOnZBorder != 0 ) {
	if ( image->dim.z >= 3 ) {
    for ( y=0; y<(int)image->dim.y ; y++ )
    for ( x=0; x<(int)image->dim.x ; x++ ) {
	    v[ (int)theBuf[0][y][x] ] = 0;
	    v[ (int)theBuf[image->dim.z-1][y][x] ] = 0;
	  }
	}
      }
      
      if ( par.removeLabelsOnYBorder != 0 ) {
	if ( image->dim.y >= 3 ) {
    for ( z=0; z<(int)image->dim.z ; z++ )
    for ( x=0; x<(int)image->dim.x ; x++ ) {
	    v[ (int)theBuf[z][0][x] ] = 0;
	    v[ (int)theBuf[z][image->dim.y-1][x] ] = 0;
	  }
	}
      }

      if ( par.removeLabelsOnXBorder != 0 ) {
	if ( image->dim.x >= 3 ) {
    for ( z=0; z<(int)image->dim.z ; z++ )
    for ( y=0; y<(int)image->dim.y ; y++ ) {
	    v[ (int)theBuf[z][y][0] ] = 0;
	    v[ (int)theBuf[z][y][image->dim.x-1] ] = 0;
	  }
	}
      }

      if ( par.removeLabelsOutsideXYCircle != 0 ) {
	r = par.xyradius * par.xyradius;
  for ( z=0; z<(int)image->dim.z ; z++ )
  for ( y=0; y<(int)image->dim.y ; y++ )
  for ( x=0; x<(int)image->dim.x ; x++ ) {
	  d = (x-par.xycenter[0])*(x-par.xycenter[0]) 
	    + (y-par.xycenter[1])*(y-par.xycenter[1]);
	  if ( d <= r ) continue;
	  v[ (int)theBuf[z][y][x] ] = 0;
	}
      }

    }
    break;
  case UCHAR :
    {
      u8 *** theBuf = (u8 ***)image->array;
      
      for ( z=0; z<(int)image->dim.z ; z++ )
      for ( y=0; y<(int)image->dim.y ; y++ )
      for ( x=0; x<(int)image->dim.x ; x++ )
	v[ theBuf[z][y][x] ] = theBuf[z][y][x];


      if ( par.removeLabelsOnZBorder != 0 ) {
	if ( image->dim.z >= 3 ) {
    for ( y=0; y<(int)image->dim.y ; y++ )
    for ( x=0; x<(int)image->dim.x ; x++ ) {
	    v[ (int)theBuf[0][y][x] ] = 0;
	    v[ (int)theBuf[image->dim.z-1][y][x] ] = 0;
	  }
	}
      }
      
      if ( par.removeLabelsOnYBorder != 0 ) {
	if ( image->dim.y >= 3 ) {
    for ( z=0; z<(int)image->dim.z ; z++ )
    for ( x=0; x<(int)image->dim.x ; x++ ) {
	    v[ (int)theBuf[z][0][x] ] = 0;
	    v[ (int)theBuf[z][image->dim.y-1][x] ] = 0;
	  }
	}
      }
      
      if ( par.removeLabelsOnXBorder != 0 ) {
	if ( image->dim.x >= 3 ) {
    for ( z=0; z<(int)image->dim.z ; z++ )
    for ( y=0; y<(int)image->dim.y ; y++ ) {
	    v[ (int)theBuf[z][y][0] ] = 0;
	    v[ (int)theBuf[z][y][image->dim.x-1] ] = 0;
	  }
	}
      }

      if ( par.removeLabelsOutsideXYCircle != 0 ) {
	r = par.xyradius * par.xyradius;
  for ( z=0; z<(int)image->dim.z ; z++ )
  for ( y=0; y<(int)image->dim.y ; y++ )
  for ( x=0; x<(int)image->dim.x ; x++ ) {
	  d = (x-par.xycenter[0])*(x-par.xycenter[0]) 
	    + (y-par.xycenter[1])*(y-par.xycenter[1]);
	  if ( d <= r ) continue;
	  v[ (int)theBuf[z][y][x] ] = 0;
	}
      }

    }
    break;
    
  default :
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse( "unable to deal with such image type\n", 0);
  }
  
      
  j = 0;
  for ( i=1; i<=n; i++ )
    if ( v[i] > 0 ) v[i] = ++j;
  fprintf( stderr," %d valid components out of %d\n", j, n );
  

  switch ( image->type ) {
  case USHORT :
    {
      unsigned short int *** theBuf = (unsigned short int ***)image->array;

     for ( z=0; z<(int)image->dim.z ; z++ )
     for ( y=0; y<(int)image->dim.y ; y++ )
     for ( x=0; x<(int)image->dim.x ; x++ ) {
       if ( theBuf[z][y][x] == 0 ) continue;
       theBuf[z][y][x] = (unsigned short int)v[ (int)theBuf[z][y][x] ];
     }
     
    }
    break;

  case UCHAR :
    {
      u8 *** theBuf = (u8 ***)image->array;
      for ( z=0; z<(int)image->dim.z ; z++ )
     for ( y=0; y<(int)image->dim.y ; y++ )
     for ( x=0; x<(int)image->dim.x ; x++ ) {
       if ( theBuf[z][y][x] == 0 ) continue;
       theBuf[z][y][x] = (u8)v[ (int)theBuf[z][y][x] ];
     }
     
    }
    break;
    
  default :
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse( "unable to deal with such image type\n", 0);
  }

  free( v );



  /*--- initialisation de l'image resultat ---*/
  if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
    VT_ErrorParse( "unable to copy name into image header", 0 );
  }




  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( image ) == -1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( image );
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
      


      else if ( strcmp ( argv[i], "-border" ) == 0 ) {
	par->removeLabelsOnXBorder = 1;
	par->removeLabelsOnYBorder = 1;
	par->removeLabelsOnZBorder = 1;
      }
      else if ( strcmp ( argv[i], "-xborder" ) == 0 ) {
	par->removeLabelsOnXBorder = 1;
      }
      else if ( strcmp ( argv[i], "-yborder" ) == 0 ) {
	par->removeLabelsOnYBorder = 1;
      }
      else if ( strcmp ( argv[i], "-zborder" ) == 0 ) {
	par->removeLabelsOnZBorder = 1;
      }


      else if ( strcmp ( argv[i], "-xycenter" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -xycenter...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->xycenter[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -xycenter...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -xycenter...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->xycenter[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -xycenter...\n", 0 );
	par->removeLabelsOutsideXYCircle = 1;
      }

      else if ( strcmp ( argv[i], "-xyradius" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -xyradius...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->xyradius) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -xyradius...\n", 0 );
	par->removeLabelsOutsideXYCircle = 1;
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
  par->type = TYPE_UNKNOWN;

  par->removeLabelsOutsideXYCircle = 0;
  par->xycenter[0] = par->xycenter[1] = 0.0;
  par->xyradius = 0.0;

  par->removeLabelsOnXBorder = 0;
  par->removeLabelsOnYBorder = 0;
  par->removeLabelsOnZBorder = 0;
}
