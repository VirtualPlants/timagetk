#include <math.h>

#include <vt_common.h>
#include <vt_copy.h>

#include <vt_distance.h>
#include <vt_daneucmapsc.h>
#include <vt_daneucmapss.h>

#define _RAGNEMALM_ 1
#define _DANIELSSON_ 2

#define _ANGLE_       1
#define _DISTANCE_    2
#define _PRODUIT_     3
#define _PRODUIT_LOG_ 4
#define _LOG_PRODUIT_ 5
#define _PRODUIT_2_   6
#define _PRODUIT_3_   7
#define _PRODUIT_4_   8
#define _PRODUIT_5_   9
#define _PRODUIT_6_   10
#define _PRODUIT_7_   11

#define _NORME_(n,z,y,x) n  = ((int)vx[z][y][x]) * ((int)vx[z][y][x]); \
			 n += ((int)vy[z][y][x]) * ((int)vy[z][y][x]); \
			 n += ((int)vz[z][y][x]) * ((int)vz[z][y][x])
#define _SCALAIRE_(c,b,a) scalaire  = ((double)vx[z][y][x]) * ((double)vx[c][b][a]); \
			  scalaire += ((double)vy[z][y][x]) * ((double)vy[c][b][a]); \
			  scalaire += ((double)vz[z][y][x]) * ((double)vz[c][b][a]); \
			  scalaire /= sqrt( (double)(norme * n) ); \
                          angle = acos( scalaire ) * 57.2957795
/* 57.2957795 = 180.0 / 3.1415926536 */
#define _DIST_(c,b,a) dx = (double)vx[c][b][a] - (double)vx[z][y][x]; \
                      dy = (double)vy[c][b][a] - (double)vy[z][y][x]; \
                      dz = (double)vz[c][b][a] - (double)vz[z][y][x]
#define _CHOIX_ dist = sqrt( dx*dx + dy*dy + dz*dz ); \
                switch ( par.type_param ) { \
		case  _DISTANCE_ : \
		  r = dist; \
		  break; \
		case _PRODUIT_ : \
		  r = angle * dist; \
		  break; \
		case _PRODUIT_LOG_ : \
		  if ( dist > 0.0 ) r = angle * log( (double)dist ); \
		  else r = 0.0; \
		  break; \
		case _LOG_PRODUIT_ : \
		  if ( angle > 0.0 ) r = log( (double)angle ) * dist ; \
		  else r = 0.0; \
		  break; \
		case _PRODUIT_2_ : \
		  if ( (dist > 0.0) ) \
		    r = angle * exp( (double)( .2630344061 * log( (double)dist ) ) ); \
		  else r = 0.0; \
		  break; \
		case _PRODUIT_3_ : \
		  if ( (dist > 0.0) ) \
		    r = angle * exp( (double)( .3690702462 * log( (double)dist ) ) ); \
		  else r = 0.0; \
		  break; \
		case _PRODUIT_4_ : \
		  if ( (dist > 0.0) ) \
		    r = angle * sqrt( (double)dist ); \
		  else r = 0.0; \
		  break; \
		case _PRODUIT_5_ : \
		  if ( (dist > 0.0) ) \
		    r = angle * exp( (double)( 1.2223924213 * log( (double)dist ) ) ); \
		  else r = 0.0; \
		  break; \
		case _PRODUIT_6_ : \
		  if ( (dist > 0.0) ) \
		    r = angle * exp( (double)( 1.4150374992 * log( (double)dist ) ) ); \
		  else r = 0.0; \
		  break; \
		case _PRODUIT_7_ : \
		  if ( (dist > 0.0) ) \
		    r = angle * exp( (double)( 1.5 * log( (double)dist ) ) ); \
		  else r = 0.0; \
		  break; \
		case _ANGLE_ : \
		default : \
		  r = angle; \
		  break; \
		} \
                if ( r > res[z][y][x] ) res[z][y][x] = (r32)( r )

#if 0
#define TEST_INV_A sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_B sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_C sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_D sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_E sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_F sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )

#define TEST_A sqrt( (double)norme ) + 1.0 > sqrt( (double)n )
#define TEST_B sqrt( (double)norme ) + 1.0 > sqrt( (double)n )
#define TEST_C sqrt( (double)norme ) + 1.0 > sqrt( (double)n )
#define TEST_D sqrt( (double)norme ) + 1.0 > sqrt( (double)n )
#define TEST_E sqrt( (double)norme ) + 1.0 > sqrt( (double)n )
#define TEST_F sqrt( (double)norme ) + 1.0 > sqrt( (double)n )
#endif

#if 0
#define TEST_INV_A (norme + 2.0 * (double)vx[z][y][x] + 1.0) <= n
#define TEST_INV_B (norme - 2.0 * (double)vx[z][y][x] + 1.0) <= n
#define TEST_INV_C (norme + 2.0 * (double)vy[z][y][x] + 1.0) <= n
#define TEST_INV_D (norme - 2.0 * (double)vy[z][y][x] + 1.0) <= n
#define TEST_INV_E (norme + 2.0 * (double)vz[z][y][x] + 1.0) <= n
#define TEST_INV_F (norme - 2.0 * (double)vz[z][y][x] + 1.0) <= n 

#define TEST_A (norme + 2.0 * (double)vx[z][y][x] + 1.0) > n
#define TEST_B (norme - 2.0 * (double)vx[z][y][x] + 1.0) > n
#define TEST_C (norme + 2.0 * (double)vy[z][y][x] + 1.0) > n
#define TEST_D (norme - 2.0 * (double)vy[z][y][x] + 1.0) > n
#define TEST_E (norme + 2.0 * (double)vz[z][y][x] + 1.0) > n
#define TEST_F (norme - 2.0 * (double)vz[z][y][x] + 1.0) > n 
#endif

#if 1
#define TEST_INV_A sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_B sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_C sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_D sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_E sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )
#define TEST_INV_F sqrt( (double)norme ) + 1.0 <= sqrt( (double)n )

#define TEST_A (norme + 2.0 * (double)vx[z][y][x] + 1.0) > n
#define TEST_B (norme - 2.0 * (double)vx[z][y][x] + 1.0) > n
#define TEST_C (norme + 2.0 * (double)vy[z][y][x] + 1.0) > n
#define TEST_D (norme - 2.0 * (double)vy[z][y][x] + 1.0) > n
#define TEST_E (norme + 2.0 * (double)vz[z][y][x] + 1.0) > n
#define TEST_F (norme - 2.0 * (double)vz[z][y][x] + 1.0) > n 
#endif



typedef struct local_par {
  vt_names names;
  vt_distance dpar;
  typeBoolean rec;
  int type_distance;
  int type_param;
  int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );





static char *usage = "[image-in] [image-out] [-rec %s] [-sb %f] [-euc_type %d] \n\
\t [-angle | -distance | -produit | -prodlog | -logprod]\n\
\t [-ragnemalm | -danielsson]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -sb %f : seuil (0.5 par defaut)\n\
\t -euc_type %d : codage pour le calcul de l'euclidean mapping\n\
\t                1 = signed char, 2 = signed short (defaut)\n\
\t -angle    : calcul de l'angle (en degres)\n\
\t -distance : calcul de la distance (en pixels)\n\
\t -produit  : produit = distance * angle\n\
\t -prodlog  : angle * log( distance )\n\
\t -logprod  : log( angle ) * distance\n\
\t -produit2 : angle * distance^(.2630344061). On a (180,1) = (150,2) = 180\n\
\t -produit3 : angle * distance^(.3690702462). On a (180,1) = (120,3) = 180\n\
\t -produit4 : angle * sqrt(distance). On a (180,1) = (90,4) = 180\n\
\t -produit5 : angle * distance^(1.2223924213). On a (180,1) = 180 et (90,2) = 210\n\
\t -produit6 : angle * distance^(1.4150374992). On a (180,1) = 180 et (90,2) = 240\n\
\t -produit7 : angle * distance^(1.5)\n\
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
	vt_image *image, imaux, imrec, imres;
	vt_image imx, imy, imz;
	int x, y, z, i;
	int dimx, dimy, dimz, dimx1, dimy1, dimz1;
	r32 ***res, ***rec = (r32***)NULL;
	register double r, scalaire, angle, dist;
	double dx, dy, dz;
	register int norme, n;

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
	
	/*--- initialisation de l'image auxiliaire ---*/
	VT_Image( &imaux );
	VT_InitFromImage( &imaux, image, par.names.out, FLOAT );
	if ( VT_AllocImage( &imaux ) != 1 ) {
	    VT_FreeImage( &imx );
	    VT_FreeImage( &imy );
	    VT_FreeImage( &imz );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to allocate auxiliary image\n", 0);
	}
	/* image reconstruction */
	VT_Image( &imrec );
	if ( par.rec == True ) {
	  VT_InitFromImage( &imrec, image, par.names.ext, FLOAT );
	  if ( VT_AllocImage( &imrec ) != 1 ) {
	    VT_FreeImage( &imaux );
	    VT_FreeImage( &imx );
	    VT_FreeImage( &imy );
	    VT_FreeImage( &imz );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to allocate auxiliary image\n", 0);
	  }
	  rec = (r32***)imrec.array;
	}

	dimx = image->dim.x;   dimx1 = dimx - 1;
	dimy = image->dim.y;   dimy1 = dimy - 1;
	dimz = image->dim.z;   dimz1 = dimz - 1;
	res = (r32***)imaux.array;

	/*--- calcul ---*/
	switch ( par.dpar.type_image_eucmap ) {
	case SCHAR :
	    {
	    s8 ***vx, ***vy, ***vz;
	    switch ( par.type_distance ) {
	    case _DANIELSSON_ :
	      if ( VT_DanVecteurPPP_SC( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
		VT_FreeImage( &imx );
		VT_FreeImage( &imy );
		VT_FreeImage( &imz );
		VT_FreeImage( &imaux );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute nearest point\n", 0);
	      }
	      break;
	    case _RAGNEMALM_ :
	    default :
	      if ( VT_VecteurPPP_SC( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
		VT_FreeImage( &imx );
		VT_FreeImage( &imy );
		VT_FreeImage( &imz );
		VT_FreeImage( &imaux );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute nearest point\n", 0);
	      }
	    }

	    vx = ( s8***)imx.array;
	    vy = ( s8***)imy.array;
	    vz = ( s8***)imz.array;

	    for ( z = 0; z < dimz; z ++ )
	    for ( y = 0; y < dimy; y ++ )
            for ( x = 0; x < dimx; x ++ ) {
		res[z][y][x] = 0.0;
		if ( (vx[z][y][x] == 0) && (vy[z][y][x] == 0) && (vz[z][y][x] == 0) ) continue;
		_NORME_(norme,z,y,x);

		/* check it's a possible maximal ball */
		if ( x > 0 ) {
		  i = x - 1;
		  if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		    _NORME_(n,z,y,i);
		    if ( TEST_INV_A ) continue;
		  }
		}
		if ( x < dimx1 ) {
		  i = x + 1;
		  if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		    _NORME_(n,z,y,i);
		    if ( TEST_INV_B ) continue;
		  }
		}
		if ( y > 0 ) {
		  i = y - 1;
		  if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		    _NORME_(n,z,i,x);
		    if ( TEST_INV_C ) continue;
		  }
		}
		if ( y < dimy1 ) {
		  i = y + 1;
		  if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		    _NORME_(n,z,i,x);
		    if ( TEST_INV_D ) continue;
		  }
		}
		if ( z > 0 ) {
		  i = z - 1;
		  if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		    _NORME_(n,i,y,x);
		    if ( TEST_INV_E ) continue;
		  }
		}
		if ( z < dimz1 ) {
		  i = z + 1;
		  if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		    _NORME_(n,i,y,x);
		    if ( TEST_INV_F ) continue;
		  }
		}

		/* compute the value */
		if ( x > 0 ) {
		  i = x - 1;
		  if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		    _NORME_(n,z,y,i);
		    if ( TEST_A ) {
		      _SCALAIRE_(z,y,i);
		      if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		      _DIST_(z,y,i);   dx -= 1.0;
		      _CHOIX_;
		    }
		  }
		}
		
		if ( x < dimx1 ) {
		  i = x + 1;
		  if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		    _NORME_(n,z,y,i);
		    if ( TEST_B ) {
		      _SCALAIRE_(z,y,i);
		      if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		      _DIST_(z,y,i);   dx += 1.0;
		      _CHOIX_;
		    }
		  }
		}

		if ( y > 0 ) {
		  i = y - 1;
		  if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		    _NORME_(n,z,i,x);
		    if ( TEST_C ) {
		      _SCALAIRE_(z,i,x);
		      if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		      _DIST_(z,i,x);   dy -= 1.0;
		      _CHOIX_;
		    }
		  }
		}

		if ( y < dimy1 ) {
		  i = y + 1;
		  if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		    _NORME_(n,z,i,x);
		    if ( TEST_D ) {
		      _SCALAIRE_(z,i,x);
		      if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		      _DIST_(z,i,x);   dy += 1.0;
		      _CHOIX_;
		    }
		  }
		}

		if ( z > 0 ) {
		  i = z - 1;
		  if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		    _NORME_(n,i,y,x);
		    if ( TEST_E ) {
		      _SCALAIRE_(i,y,x);
		      if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		      _DIST_(i,y,x);   dz -= 1.0;
		      _CHOIX_;
		    }
		  }
		}

		if ( z < dimz1 ) {
		  i = z + 1;
		  if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		    _NORME_(n,i,y,x);
		    if ( TEST_F ) {
		      _SCALAIRE_(i,y,x);
		      if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		      _DIST_(i,y,x);   dz += 1.0;
		      _CHOIX_;
		    }
		  }
		}

	    }

	    /* reconstruction */
	    if ( par.rec == True ) {
	      int ix, iy, iz;
	      
	      for ( z = 0; z < dimz; z ++ )
	      for ( y = 0; y < dimy; y ++ )
	      for ( x = 0; x < dimx; x ++ )
		rec[z][y][x] = 0.0;
	
	      for ( z = 0; z < dimz; z ++ )
	      for ( y = 0; y < dimy; y ++ )
	      for ( x = 0; x < dimx; x ++ ) {
		if ( res[z][y][x] > 0.0 ) {
		  _NORME_(norme,z,y,x);
		  dx = (int)( sqrt( (double)norme ) + 1 );
		  dy = dz = dx;
		  for ( iz = (-dz); iz <= dz; iz ++ ) {
		    if ( (z+iz < 0) || (z+iz > dimz1) ) continue;
		    for ( iy = (-dy); iy <= dy; iy ++ ) {
		      if ( (y+iy < 0) || (y+iy > dimy1) ) continue;
		      for ( ix = (-dx); ix <= dx; ix ++ ) {
			if ( (x+ix < 0) || (x+ix > dimx1) ) continue;
			n = ix*ix + iy*iy + iz*iz;
			if ( n >= norme ) continue;
			if ( rec[z+iz][y+iy][x+ix] < res[z][y][x] )
			  rec[z+iz][y+iy][x+ix] = res[z][y][x];
		      }
		    }
		  }
		}
	      }
	    }
	    
	    }
	    break;
	case SSHORT :
	  {
	  s16 ***vx, ***vy, ***vz;
	  switch ( par.type_distance ) {
	  case _DANIELSSON_ :
	    if ( VT_DanVecteurPPP_SS( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
	      VT_FreeImage( &imx );
	      VT_FreeImage( &imy );
	      VT_FreeImage( &imz );
	      VT_FreeImage( &imaux );
	      VT_FreeImage( image );
	      VT_Free( (void**)&image );
	      VT_ErrorParse("unable to compute nearest point\n", 0);
	    }
	    break;
	  case _RAGNEMALM_ :
	  default :
	    if ( VT_VecteurPPP_SS( image, &imx, &imy, &imz, &(par.dpar) ) != 1 ) {
		VT_FreeImage( &imx );
		VT_FreeImage( &imy );
		VT_FreeImage( &imz );
		VT_FreeImage( &imaux );
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_ErrorParse("unable to compute nearest point\n", 0);
	    }
	  }

	  vx = ( s16***)imx.array;
	  vy = ( s16***)imy.array;
	  vz = ( s16***)imz.array;
	  
	  for ( z = 0; z < dimz; z ++ )
	  for ( y = 0; y < dimy; y ++ )
	  for ( x = 0; x < dimx; x ++ ) {
	    res[z][y][x] = 0.0;
	    if ( (vx[z][y][x] == 0) && (vy[z][y][x] == 0) && (vz[z][y][x] == 0) ) continue;
	    _NORME_(norme,z,y,x);
	    
		/* check it's a possible maximal ball */
		if ( x > 0 ) {
		  i = x - 1;
		  if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		    _NORME_(n,z,y,i);
		    if ( TEST_INV_A ) continue;
		  }
		}
		if ( x < dimx1 ) {
		  i = x + 1;
		  if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		    _NORME_(n,z,y,i);
		    if ( TEST_INV_B ) continue;
		  }
		}
		if ( y > 0 ) {
		  i = y - 1;
		  if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		    _NORME_(n,z,i,x);
		    if ( TEST_INV_C ) continue;
		  }
		}
		if ( y < dimy1 ) {
		  i = y + 1;
		  if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		    _NORME_(n,z,i,x);
		    if ( TEST_INV_D ) continue;
		  }
		}
		if ( z > 0 ) {
		  i = z - 1;
		  if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		    _NORME_(n,i,y,x);
		    if ( TEST_INV_E ) continue;
		  }
		}
		if ( z < dimz1 ) {
		  i = z + 1;
		  if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		    _NORME_(n,i,y,x);
		    if ( TEST_INV_F ) continue;
		  }
		}

		/* compute the value */
	    if ( x > 0 ) {
	      i = x - 1;
	      if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		_NORME_(n,z,y,i);
		if ( TEST_A ) {
		  _SCALAIRE_(z,y,i);
		    if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		    _DIST_(z,y,i);   dx -= 1.0;
		    _CHOIX_;
		}
	      }
	    }
	    
	    if ( x < dimx1 ) {
	      i = x + 1;
	      if ( (vx[z][y][i] != 0) || (vy[z][y][i] != 0) || (vz[z][y][i] != 0) ) {
		_NORME_(n,z,y,i);
		if ( TEST_B ) {
		  _SCALAIRE_(z,y,i);
		  if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		  _DIST_(z,y,i);   dx += 1.0;
		  _CHOIX_;
		}
	      }
	    }
	    
	    if ( y > 0 ) {
	      i = y - 1;
	      if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		_NORME_(n,z,i,x);
		if ( TEST_C ) {
		  _SCALAIRE_(z,i,x);
		  if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		  _DIST_(z,i,x);   dy -= 1.0;
		  _CHOIX_;
		}
	      }
	    }
	    
	    if ( y < dimy1 ) {
	      i = y + 1;
	      if ( (vx[z][i][x] != 0) || (vy[z][i][x] != 0) || (vz[z][i][x] != 0) ) {
		_NORME_(n,z,i,x);
		if ( TEST_D ) {
		  _SCALAIRE_(z,i,x);
		  if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		  _DIST_(z,i,x);   dy += 1.0;
		  _CHOIX_;
		}
	      }
	    }
	    
	    if ( z > 0 ) {
	      i = z - 1;
	      if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		_NORME_(n,i,y,x);
		if ( TEST_E ) {
		  _SCALAIRE_(i,y,x);
		  if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		  _DIST_(i,y,x);   dz -= 1.0;
		  _CHOIX_;
		}
	      }
	    }
	    
	    if ( z < dimz1 ) {
	      i = z + 1;
	      if ( (vx[i][y][x] != 0) || (vy[i][y][x] != 0) || (vz[i][y][x] != 0) ) {
		_NORME_(n,i,y,x);
		if ( TEST_F ) {
		  _SCALAIRE_(i,y,x);
		  if ( (angle < 91.0) && (norme == 1) && (n == 1) ) angle = 0.0;
		  _DIST_(i,y,x);   dz += 1.0;
		    _CHOIX_;
		}
	      }
	    }

	  }
	  
	    /* reconstruction */
	    if ( par.rec == True ) {
	      int ix, iy, iz;
	      
	      for ( z = 0; z < dimz; z ++ )
	      for ( y = 0; y < dimy; y ++ )
	      for ( x = 0; x < dimx; x ++ )
		rec[z][y][x] = 0.0;
	
	      for ( z = 0; z < dimz; z ++ )
	      for ( y = 0; y < dimy; y ++ )
	      for ( x = 0; x < dimx; x ++ ) {
		if ( res[z][y][x] > 0.0 ) {
		  _NORME_(norme,z,y,x);
		  dx = (int)( sqrt( (double)norme ) + 1 );
		  dy = dz = dx;
		  for ( iz = (-dz); iz <= dz; iz ++ ) {
		    if ( (z+iz < 0) || (z+iz > dimz1) ) continue;
		    for ( iy = (-dy); iy <= dy; iy ++ ) {
		      if ( (y+iy < 0) || (y+iy > dimy1) ) continue;
		      for ( ix = (-dx); ix <= dx; ix ++ ) {
			if ( (x+ix < 0) || (x+ix > dimx1) ) continue;
			n = ix*ix + iy*iy + iz*iz;
			if ( n >= norme ) continue;
			if ( rec[z+iz][y+iy][x+ix] < res[z][y][x] )
			  rec[z+iz][y+iy][x+ix] = res[z][y][x];
		      }
		    }
		  }
		}
	      }
	    }
	  
	  }
	
	}
	    
	/*--- liberations memoires ---*/
	VT_FreeImage( &imx );
	VT_FreeImage( &imy );
	VT_FreeImage( &imz );

	/*--- initialisation de l'image resultat ---*/
	VT_Image( &imres );
	VT_InitFromImage( &imres, image, par.names.out, image->type );
	if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
	if ( VT_AllocImage( &imres ) != 1 ) {
	    VT_FreeImage( &imaux );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to allocate output image\n", 0);
	}
	if ( VT_CopyImage( &imaux, &imres ) != 1 ) {
	    VT_FreeImage( &imaux );
	    VT_FreeImage( &imres );
	    VT_ErrorParse("unable to copy output image\n", 0);
	}

	/*--- liberations memoires ---*/ 
	VT_FreeImage( &imaux );

	/*--- ecriture de l'image resultat ---*/
	if ( VT_WriteInrimage( &imres ) == -1 ) {
	    VT_FreeImage( &imres );
	    VT_ErrorParse("unable to write output image\n", 0);
	}
	/*--- liberations memoires ---*/
	VT_FreeImage( &imres );
	
	if ( par.rec == True ) {
	  VT_Image( &imres );
	  VT_InitFromImage( &imres, image, par.names.ext, image->type );
	  if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
	  if ( VT_AllocImage( &imres ) != 1 ) {
	    VT_FreeImage( &imrec );
	    VT_FreeImage( image );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to allocate output image\n", 0);
	  }
	  if ( VT_CopyImage( &imrec, &imres ) != 1 ) {
	    VT_FreeImage( &imrec );
	    VT_FreeImage( &imres );
	    VT_ErrorParse("unable to copy output image\n", 0);
	  }
	  VT_FreeImage( &imrec );
	  if ( VT_WriteInrimage( &imres ) == -1 ) {
	    VT_FreeImage( &imres );
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
                        /*--- seuil ---*/
                        else if ( strcmp ( argv[i], "-sb" ) == 0 ) {
                                i += 1;
                                if ( i >= argc)    VT_ErrorParse( "parsing -sb...\n", 0 );
                                status = sscanf( argv[i],"%f",&(par->dpar.seuil) );
                                if ( status <= 0 ) VT_ErrorParse( "parsing -sb...\n", 0 );
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
			/*--- traitement ---*/
			else if ( strcmp ( argv[i], "-angle" ) == 0 ) {
                                par->type_param = _ANGLE_;
                        }
			else if ( strcmp ( argv[i], "-distance" ) == 0 ) {
                                par->type_param = _DISTANCE_;
                        }
			else if ( strcmp ( argv[i], "-produit" ) == 0 ) {
			  par->type_param = _PRODUIT_;
                        }
			else if ( strcmp ( argv[i], "-produit2" ) == 0 ) {
			  par->type_param = _PRODUIT_2_;
                        }
			else if ( strcmp ( argv[i], "-produit3" ) == 0 ) {
			  par->type_param = _PRODUIT_3_;
                        }
			else if ( strcmp ( argv[i], "-produit4" ) == 0 ) {
			  par->type_param = _PRODUIT_4_;
                        }
			else if ( strcmp ( argv[i], "-produit5" ) == 0 ) {
			  par->type_param = _PRODUIT_5_;
                        }
			else if ( strcmp ( argv[i], "-produit6" ) == 0 ) {
			  par->type_param = _PRODUIT_6_;
                        }
			else if ( strcmp ( argv[i], "-produit7" ) == 0 ) {
			  par->type_param = _PRODUIT_7_;
                        }
			else if ( strcmp ( argv[i], "-logprod" ) == 0 ) {
                                par->type_param = _LOG_PRODUIT_;
                        }
			else if ( strcmp ( argv[i], "-prodlog" ) == 0 ) {
                                par->type_param = _PRODUIT_LOG_;
                        }
			else if ( strcmp ( argv[i], "-ragnemalm" ) == 0 ) {
                                par->type_distance = _RAGNEMALM_;
                        }
			else if ( strcmp ( argv[i], "-danielsson" ) == 0 ) {
                                par->type_distance = _DANIELSSON_;
                        }
			/* reconstruction */
			else if ( strcmp ( argv[i], "-rec" ) == 0 ) {
			  i += 1;
			  strncpy( par->names.ext, argv[i], STRINGLENGTH );  
			  par->rec = True;
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

	/*--- conversion du type pour le calcul de l'euclidean mapping ---*/
	switch ( par->dpar.type_image_eucmap ) {
	case 1 :
	    par->dpar.type_image_eucmap = SCHAR;
	    break;
	case 2 :
	default :
	    par->dpar.type_image_eucmap = SSHORT;
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
	VT_Distance( &(par->dpar) );
	par->rec = False;
	par->type_distance = _RAGNEMALM_;
	par->type_param = TYPE_UNKNOWN;
	par->type = TYPE_UNKNOWN;
}
