#include <vt_common.h>

#include <vt_distance.h>

typedef struct local_par {
    vt_names names;
    vt_distance dpar;
    int bool_vector;
    int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );





static char *usage = "[image-in] [image-out] [-2D] [-euc_type %d] \n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -2D : restriction dans le plan des distances ci-dessus\n\
\t -euc_type %d : codage pour le calcul de l'euclidean mapping\n\
\t                1 = signed char, 2 = signed short (defaut)\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n";




static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
	local_par par;
	vt_image *image;
	vt_image imx, imy, imz;

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

	
	/*--- calcul du vecteur pointant vers le plus proche point ---*/
	/*--- initialisation des images resultat ---*/
	VT_Image( &imx );
	VT_InitFromImage( &imx, image, par.names.out, par.dpar.type_image_eucmap );
	VT_Image( &imy );
	VT_InitFromImage( &imy, image, par.names.out, par.dpar.type_image_eucmap );
	VT_Image( &imz );
	VT_InitFromImage( &imz, image, par.names.out, par.dpar.type_image_eucmap );
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
	
	/*--- calcul ---*/
	switch ( par.dpar.type_image_eucmap ) {
	case SCHAR :
	  if ( VT_VecteurPPP_SC( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
	    VT_FreeImage( &imx );
	    VT_FreeImage( &imy );
	    VT_FreeImage( &imz );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute nearest point\n", 0);
	  }
	  break;
	case SSHORT :
	  if ( VT_VecteurPPP_SS( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
	    VT_FreeImage( &imx );
	    VT_FreeImage( &imy );
	    VT_FreeImage( &imz );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute nearest point\n", 0);
	  }
	}

	/*--- on propage le numero de l'image d'entree ---*/
	{
	  register int x, y, z, i, j, k;
	  
	  switch ( image->type ) {
	  case UCHAR :
	    {
	      u8 ***theBuf = (u8 ***)(image->array);
	      switch ( par.dpar.type_image_eucmap ) {
	      case SCHAR :
		{
		  s8 ***xbuf = (s8 ***)(imx.array);
		  s8 ***ybuf = (s8 ***)(imy.array);
		  s8 ***zbuf = (s8 ***)(imz.array);
      for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
      for ( x = 0; x < (int)image->dim.x; x ++ ) {
		    k = zbuf[z][y][x];   j = ybuf[z][y][x];   i = xbuf[z][y][x];
		    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
		    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
		  }
		}
	      case SSHORT :
		{
		  s16 ***xbuf = (s16 ***)(imx.array);
		  s16 ***ybuf = (s16 ***)(imy.array);
		  s16 ***zbuf = (s16 ***)(imz.array);
      for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
      for ( x = 0; x < (int)image->dim.x; x ++ ) {
		    k = zbuf[z][y][x];   j = ybuf[z][y][x];   i = xbuf[z][y][x];
		    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
		    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
		  }
		}
	      }
	    }
	    break;
	  case USHORT :
	    {
	      u16 ***theBuf = (u16 ***)(image->array);
	      switch ( par.dpar.type_image_eucmap ) {
	      case SCHAR :
		{
		  s8 ***xbuf = (s8 ***)(imx.array);
		  s8 ***ybuf = (s8 ***)(imy.array);
		  s8 ***zbuf = (s8 ***)(imz.array);
      for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
      for ( x = 0; x < (int)image->dim.x; x ++ ) {
		    k = zbuf[z][y][x];   j = ybuf[z][y][x];   i = xbuf[z][y][x];
		    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
		    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
		  }
		}
	      case SSHORT :
		{
		  s16 ***xbuf = (s16 ***)(imx.array);
		  s16 ***ybuf = (s16 ***)(imy.array);
		  s16 ***zbuf = (s16 ***)(imz.array);
      for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
      for ( x = 0; x < (int)image->dim.x; x ++ ) {
		    k = zbuf[z][y][x];   j = ybuf[z][y][x];   i = xbuf[z][y][x];
		    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
		    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
		  }
		}
	      }
	    }
	    break;
	  case SSHORT :
	    {
	      s16 ***theBuf = (s16 ***)(image->array);
	      switch ( par.dpar.type_image_eucmap ) {
	      case SCHAR :
		{
		  s8 ***xbuf = (s8 ***)(imx.array);
		  s8 ***ybuf = (s8 ***)(imy.array);
		  s8 ***zbuf = (s8 ***)(imz.array);
      for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
      for ( x = 0; x < (int)image->dim.x; x ++ ) {
		    k = zbuf[z][y][x];   j = ybuf[z][y][x];   i = xbuf[z][y][x];
		    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
		    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
		  }
		}
	      case SSHORT :
		{
		  s16 ***xbuf = (s16 ***)(imx.array);
		  s16 ***ybuf = (s16 ***)(imy.array);
		  s16 ***zbuf = (s16 ***)(imz.array);
      for ( z = 0; z < (int)image->dim.z; z ++ )
      for ( y = 0; y < (int)image->dim.y; y ++ )
      for ( x = 0; x < (int)image->dim.x; x ++ ) {
		    k = zbuf[z][y][x];   j = ybuf[z][y][x];   i = xbuf[z][y][x];
		    if ( (k == 0) && (j == 0) && (i == 0) ) continue;
		    theBuf[z][y][x] = theBuf[z+k][y+j][x+i];
		  }
		}
	      }
	    }
	    break;
	  default :
	     VT_FreeImage( &imx );
	     VT_FreeImage( &imy );
	     VT_FreeImage( &imz );
	     VT_FreeImage( image );
	     VT_Free( (void**)&image );
	     VT_ErrorParse("unable to deal with such input image type\n", 0);
	  }
	}

	VT_FreeImage( &imx );
	VT_FreeImage( &imy );
	VT_FreeImage( &imz );
	(void)strcpy( image->name, par.names.out );
	
	/*--- ecriture des images resultat ---*/
	if ( VT_WriteInrimage( image ) == -1 ) {
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to write output image\n", 0);
	}

	VT_FreeImage( image );
	VT_Free( (void**)&image );
	return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
	int i, nb, status;

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
			/*--- dimension du traitement ---*/
			else if ( strcmp ( argv[i], "-2D" ) == 0 ) {
                                par->dpar.dim = VT_2D;
			}
			/*--- type pour le calcul de l'euclidean mapping ---*/
                        else if ( strcmp ( argv[i], "-euc_type" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -euc_type...\n", 0 );
                                status = sscanf( argv[i],"%d",&(par->dpar.type_image_eucmap) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -euc_type...\n", 0 );
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

	/*--- conversion du type pour le calcul de l'euclidean mapping ---*/
	switch ( par->dpar.type_image_eucmap ) {
	case 1 :
	    par->dpar.type_image_eucmap = SCHAR;
	    break;
	case 2 :
	default :
	    par->dpar.type_image_eucmap = SSHORT;
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
	VT_Distance( &(par->dpar) );
	par->bool_vector = 0;
	par->type = TYPE_UNKNOWN;
}
