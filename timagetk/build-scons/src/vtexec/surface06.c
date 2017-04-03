#include <vt_common.h>

#include <vt_amincir.h>
#include <vt_amliste.h>

#include <vt_boolean.h>
#include <vt_t06.h>
#include <vt_t26.h>

typedef struct local_par {
  vt_names names;
  int bool_amincir;
  int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-thining | -amincir]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -thining | -amincir : 6-amincissement de l'image\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n";

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
	if ( image->type != UCHAR ) {
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("input image type should be unsigned char\n", 0);
	}

	/*--- operations eventuelles sur l'image d'entree ---*/
	if ( par.names.inv == 1 )  VT_InverseImage( image );
	if ( par.names.swap == 1 ) VT_SwapImage( image );

	/*--- initialisation de l'image resultat ---*/
        VT_Image( &imres );
        VT_InitImage( &imres, par.names.out, image->dim.x, image->dim.y, image->dim.z, (int)UCHAR );
        if ( VT_AllocImage( &imres ) != 1 ) {
                VT_FreeImage( image );
                VT_Free( (void**)&image );
                VT_ErrorParse("unable to allocate output image\n", 0);
        }

	if ( par.bool_amincir == 1 ) {
	  register int i, j, k, l, x, z;
	  unsigned char *theBuf, *resBuf;
	  vt_pt_amincir *liste;
	  int ind, dx, dxy, lnb, ln;
	  int indice[6][2], vois[3][3][3], offset[3][3][3], voisins[26];
	  int nb_cycle, del_cycle, del_sscycle;
	  char message[256];
	  /*--- on recopie image dans imres ---*/
	  z = image->dim.x * image->dim.y * image->dim.z;
	  theBuf = (unsigned char*)image->buf;
	  resBuf = (unsigned char*)imres.buf;
	  for ( x = 0; x < z; x++ ) {
	    if ( *theBuf > 0 ) *resBuf = (u8)VT_DELETABLE;
	    else               *resBuf = (u8)0;
	    theBuf++;           resBuf++;
	  }
	  /*--- on cree la liste de point a effacer ---*/
	  liste = _VT_ThinPtList( &imres, (int)VT_3D, &lnb );
	  if ( (liste == (vt_pt_amincir*)NULL) || (lnb <= 0) ) {
	    VT_FreeImage( image );
	    VT_FreeImage( &imres );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute list of points\n", 0);
	  }
	  /*--- on prepare 
	        la numerotation du voisinage est :
		 0  1  2    9 10 11   17 18 19
		 3  4  5   12 .. 13   20 21 22
		 6  7  8   14 15 16   23 24 25
	  ---*/
	  theBuf = (unsigned char*)imres.buf;
	  dx = imres.dim.x;    dxy = imres.dim.x * imres.dim.y;
	  ln = lnb;
	  indice[0][0] =  4;   indice[0][1] = 21;
	  indice[1][0] = 21;   indice[1][1] =  4;
	  indice[2][0] = 10;   indice[2][1] = 15;
	  indice[3][0] = 15;   indice[3][1] = 10;
	  indice[4][0] = 12;   indice[4][1] = 13;
	  indice[5][0] = 13;   indice[5][1] = 12;
	  offset[1][1][1] = 0;
	  offset[1][0][1] = - dx;
	  offset[1][2][1] = dx;
	  offset[0][1][1] = - 1;
	  offset[0][0][1] = offset[1][0][1] - 1;
	  offset[0][2][1] = offset[1][2][1] - 1;
	  offset[2][1][1] = 1;
	  offset[2][0][1] = offset[1][0][1] + 1;
	  offset[2][2][1] = offset[1][2][1] + 1;
	  for ( i = 0; i < 3; i++ )
	  for ( j = 0; j < 3; j++ ) {
	    offset[i][j][0] = offset[i][j][1] - dxy;
	    offset[i][j][2] = offset[i][j][1] + dxy;
	  }
	  /*--- boucle des cycles ---*/
	  nb_cycle = 0;
	  do {
	    del_cycle = 0;
	    /*--- boucle des sous-cycles ---*/
	    for ( x = 0; x < 6; x ++ ) {
	      del_sscycle = 0;
	      /*--- liste des points ---*/
	      for ( l = 0; l < ln; l++ ) {
		/*--- saisie du voisinage ---*/
		ind = liste[l].pt.z * dxy + liste[l].pt.y * dx + liste[l].pt.x;
		if ( liste[l].inside == 1 ) {
		    for ( i = 0; i < 3; i++ )
		    for ( j = 0; j < 3; j++ )
		    for ( k = 0; k < 3; k++ )
			vois[i][j][k] = (int)(theBuf[ ind + offset[i][j][k] ]);
		} else {
		    for ( i = -1; i < 2; i++ )
		    for ( j = -1; j < 2; j++ )
		    for ( k = -1; k < 2; k++ ) {
			if ( (liste[l].pt.x+i >= 0) && (liste[l].pt.x+i < dx) &&
           (liste[l].pt.y+j >= 0) && (liste[l].pt.y+j < (int)imres.dim.y) &&
           (liste[l].pt.z+k >= 0) && (liste[l].pt.z+k < (int)imres.dim.z) )
			    vois[1+i][1+j][1+k] = (int)(theBuf[ ind + offset[1+i][1+j][1+k] ]);
			else
			    vois[1+i][1+j][1+k] = (int)0;
		    }
		}
		/*--- on change de representation de voisinage ---*/
		voisins[ 0] = vois[0][0][0];  voisins[ 1] = vois[1][0][0];   voisins[ 2] = vois[2][0][0];  
		voisins[ 3] = vois[0][1][0];  voisins[ 4] = vois[1][1][0];   voisins[ 5] = vois[2][1][0];  
		voisins[ 6] = vois[0][2][0];  voisins[ 7] = vois[1][2][0];   voisins[ 8] = vois[2][2][0];  
		voisins[ 9] = vois[0][0][1];  voisins[10] = vois[1][0][1];   voisins[11] = vois[2][0][1];  
		voisins[12] = vois[0][1][1];                                 voisins[13] = vois[2][1][1];  
		voisins[14] = vois[0][2][1];  voisins[15] = vois[1][2][1];   voisins[16] = vois[2][2][1];  
		voisins[17] = vois[0][0][2];  voisins[18] = vois[1][0][2];   voisins[19] = vois[2][0][2];  
		voisins[20] = vois[0][1][2];  voisins[21] = vois[1][1][2];   voisins[22] = vois[2][1][2];  
		voisins[23] = vois[0][2][2];  voisins[24] = vois[1][2][2];   voisins[25] = vois[2][2][2];  
		/*--- effacable ? ---*/
		if ( voisins[ indice[x][0] ] >  0 ) continue;
		if ( voisins[ indice[x][1] ] == 0 ) continue;
		if ( IsSimple3D_6NEP( voisins ) != 1 ) continue;
		/*--- a effacer ---*/
		liste[l].status = VT_TOBEDELETED;
	      }
	      /*--- fin du traitement de la liste ---*/
	      /*--- on modifie l'image et on rearrange la liste ---*/
	      i = 0;
	      for ( l = 0; l < ln; l++ ) {
		switch ( liste[l].status ) {
		case VT_DELETABLE :
		  /*--- on garde le point dans la liste ---*/
		  liste[i++] = liste[l];
		  break;
		case VT_TOBEDELETED :
		  /*--- on efface le point ---*/
		  ind = liste[l].pt.z * dxy + liste[l].pt.y * dx + liste[l].pt.x;
		  theBuf[ind] = (unsigned char)0;
		  del_sscycle ++;
		  break;
		case VT_UNDELETABLE :
		default :
		  break;
		}
	      }
	      ln = i;
	      del_cycle += del_sscycle;
	    }
	    /*--- fin des sous-cycles ---*/
	    sprintf( message," thinning : cycle %3d, deleted points = %8d", nb_cycle, del_cycle );
	    VT_Message( message, program );
	    nb_cycle ++;
	  } while ( del_cycle > 0 );
	  /*--- fin des cycles ---*/
	  /*--- on inverse l'image resultat ---*/
	  z = image->dim.x * image->dim.y * image->dim.z;
	  for ( x = 0; x < z; x++ ) {
	    if ( *theBuf > 0 ) *theBuf = (u8)0;
	    else               *theBuf = (u8)255;
	    theBuf++;  
	  }
	} else {
	  register int x, z;
	  unsigned char *theBuf, *resBuf;
	  /*--- on recopie et on inverse image dans imres ---*/
	  z = image->dim.x * image->dim.y * image->dim.z;
	  theBuf = (unsigned char*)image->buf;
	  resBuf = (unsigned char*)imres.buf;
	  for ( x = 0; x < z; x++ ) {
	    if ( *theBuf > 0 ) *resBuf = (u8)0;
	    else               *resBuf = (u8)255;
	    theBuf++;           resBuf++;
	  }
	}
		
	/*--- on peut liberer image ---*/
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	
	/*--- caracterisation complete ---*/
	{
	  register int x, y, z, i, j, k;
	  int dx, dy, dz, dxy, dx1, dy1, dz1;
	  int t06, t26, ind;
	  int offset[3][3][3], vois[3][3][3], voisins[27];
	  int histo[7][9];
	  unsigned char *theBuf;
	  /*--- preparation ---*/
	  theBuf = (unsigned char*)imres.buf;
	  dx = imres.dim.x;   dx1 = dx - 1;
	  dy = imres.dim.y;   dy1 = dy - 1;
	  dz = imres.dim.z;   dz1 = dz - 1;
	  dxy = dx * dy;
	  for ( i = 0; i < 3; i++ )
	    offset[i][1][1] = i - 1; 
	  for ( i = 0; i < 3; i++ ) {
	    offset[i][0][1] = offset[i][1][1] - dx;
	    offset[i][2][1] = offset[i][1][1] + dx;
	  }
	  for ( i = 0; i < 3; i++ )
          for ( j = 0; j < 3; j++ ) {
	    offset[i][j][0] = offset[i][j][1] - dxy;
	    offset[i][j][2] = offset[i][j][1] + dxy;
	  }
	  for ( i = 0; i < 7; i++ )
          for ( j = 0; j < 9; j++ ) 
	    histo[i][j] = 0;
	  ind = 0;
	  /*--- traitement ---*/
	  for ( z = 0; z < dz; z++ )
	  for ( y = 0; y < dy; y++ )
	  for ( x = 0; x < dx; x++ ) {
	    /*--- point du fond ---*/
	    if ( theBuf[ind] == 255 ) {
	      ind ++;
	      continue;
	    }
	    /*--- on separe les cas "bord de l'image" des autres ---*/
	    /*--- a l'interieur de l'image, on ne teste pas      ---*/
	    if ( ( x > 0 ) && ( x < dx1 ) && 
		 ( y > 0 ) && ( y < dy1 ) &&
		 ( z > 0 ) && ( z < dz1 ) ) {
	      for ( i = 0; i < 3; i++ )
	      for ( j = 0; j < 3; j++ ) 
	      for ( k = 0; k < 3; k++ ) {
		vois[i][j][k] = (int)(theBuf[ ind + offset[i][j][k] ]);
	      }
	    } else {
	      for ( i = -1; i < 2; i++ )
              for ( j = -1; j < 2; j++ ) 
              for ( k = -1; k < 2; k++ ) {
		if ( (x+i >= 0) && (x+i < dx) && 
		     (y+j >= 0) && (y+j < dy) &&
		     (z+k >= 0) && (z+k < dz) ) {
		  vois[1+i][1+j][1+k] = (int)(theBuf[ ind + offset[1+i][1+j][1+k] ]);
		} else {
		  vois[1+i][1+j][1+k] = (int)255;
		}
	      }
	    }
	    /*--- le "fond" est a 255, le reste est l'objet ---*/
	    for ( i = 0; i < 3; i++ )
	    for ( j = 0; j < 3; j++ ) 
	    for ( k = 0; k < 3; k++ )
	      if ( vois[i][j][k] < 255 ) vois[i][j][k] = 0;
	    /*--- on change de representation de voisinage ---*/
	    voisins[ 0] = vois[0][0][0];  voisins[ 1] = vois[1][0][0];   voisins[ 2] = vois[2][0][0];  
	    voisins[ 3] = vois[0][1][0];  voisins[ 4] = vois[1][1][0];   voisins[ 5] = vois[2][1][0];  
	    voisins[ 6] = vois[0][2][0];  voisins[ 7] = vois[1][2][0];   voisins[ 8] = vois[2][2][0];  
	    voisins[ 9] = vois[0][0][1];  voisins[10] = vois[1][0][1];   voisins[11] = vois[2][0][1];  
	    voisins[12] = vois[0][1][1];  voisins[13] = vois[1][1][1];   voisins[14] = vois[2][1][1];  
	    voisins[15] = vois[0][2][1];  voisins[16] = vois[1][2][1];   voisins[17] = vois[2][2][1];  
	    voisins[18] = vois[0][0][2];  voisins[19] = vois[1][0][2];   voisins[20] = vois[2][0][2];  
	    voisins[21] = vois[0][1][2];  voisins[22] = vois[1][1][2];   voisins[23] = vois[2][1][2];  
	    voisins[24] = vois[0][2][2];  voisins[25] = vois[1][2][2];   voisins[26] = vois[2][2][2];  
	    /*--- les 2 nombres ---*/
	    t06 = Compute_T06( voisins );
	    t26 = Compute_T26( voisins );
	    histo[t06][t26] ++;
	    i = t06 * 10 + t26;
	    theBuf[ind] = (u8)i;
	    ind++;
	  }
	  if ( _VT_VERBOSE_ == 1 ) {
	    fprintf( stderr, " --- First caracterisation ---\n" );
	    for ( i = 0; i < 7; i++ )
	    for ( j = 0; j < 9; j++ )
	      if ( histo[i][j] > 0 ) 
		fprintf( stderr, " t06 = %d, t26 = %d, # points = %8d\n", i, j, histo[i][j] );
	  }
	  /*--- on remet le fond a zero ---*/
	  z = imres.dim.x * imres.dim.y * imres.dim.z;
	  for ( x = 0; x < z; x++ )
	    if ( theBuf[x] == 255 ) theBuf[x] = 0;
	}
	/*--- caracterisation reduite ---*/
	{
	  register int i, j, v;
	  unsigned char *theBuf;
	  v = imres.dim.x * imres.dim.y * imres.dim.z;
	  theBuf = (unsigned char*)imres.buf;
	  for ( i = 0; i < v; i ++ ) {
	    j = (int)theBuf[i];
	    switch ( j ) {
	    case 11 : /* point frontiere */
	      theBuf[i] = 100;   break;
	    case 12 : /* point surface */
	      theBuf[i] = 200;   break;
	    case 1 :
	    case 10 :
	    case 13 :
	    case 14 :
	    case 15 :
	    case 16 :
	    case 17 :
	    case 18 :
	    case 21 :
	    case 22 :
	    case 23 :
	    case 24 :
	    case 25 :
	    case 31 :
	    case 32 :
	    case 33 :
	    case 41 :
	    case 42 :
	    case 51 :
	    case 61 :
	      theBuf[i] = 255;
	    }
	  }
	}
	
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
    int i, nb; 
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
	    /*--- amincissement ? ---*/
	    else if ( strcmp ( argv[i], "-amincir" ) == 0 ) {
		par->bool_amincir = 1;
	    }
	    else if ( strcmp ( argv[i], "-thinning" ) == 0 ) {
		par->bool_amincir = 1;
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
	par->bool_amincir = 0;
	par->type = UCHAR;
}
