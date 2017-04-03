
#include <vt_eucmapsc.h>

#define VT_F2I( F ) ( (F) >= 0.0 ? ((int)((F)+0.5)) : ((int)((F)-0.5)) )

/*--- infinis positif et negatif :
      on definit l'infini a 26753 au lieu de 32767, 
      car 3*32767*32767 = 3221028867 depasse les capacites 
      de l'entier 2^31-1 = 2147483647. on choisit donc
      l'infini comme (int)sqrt(2147483647/3)-1
---*/
#define _VT_SS_PINFINI  26753
#define _VT_SS_NINFINI -26753
#define _VT_SS_NORME(X,Y,Z) ( (X)*(X) + (Y)*(Y) + (Z)*(Z) )

#define _TEST_NORME_ min_nr = _VT_SS_NORME( ((int)(vx[zdz][ydy][xdx]) + dx), ((int)(vy[zdz][ydy][xdx]) + dy), ((int)(vz[zdz][ydy][xdx]) + dz) ); \
		     if ( min_nr >= norme )   continue; \
		     min_dx = dx;   min_dy = dy;        \
		     min_dz = dz;   norme = min_nr;     \
                     _NEW_ = 1;

#define _NEW_POINT_ if ( _NEW_ == 0 ) continue; \
                    xdx = x + min_dx;   ydy = y + min_dy;    zdz = z + min_dz; \
                    i = (int)(vx[zdz][ydy][xdx]) + min_dx; \
	            if ( i < _VT_SS_NINFINI )      { vx[z][y][x] = (s16)(_VT_SS_NINFINI); } \
	            else if ( i > _VT_SS_PINFINI ) { vx[z][y][x] = (s16)(_VT_SS_PINFINI); } \
	            else                           { vx[z][y][x] = (s16)(i);              } \
	            i = (int)(vy[zdz][ydy][xdx]) + min_dy; \
	            if ( i < _VT_SS_NINFINI )      { vy[z][y][x] = (s16)(_VT_SS_NINFINI); } \
	            else if ( i > _VT_SS_PINFINI ) { vy[z][y][x] = (s16)(_VT_SS_PINFINI); } \
	            else                           { vy[z][y][x] = (s16)(i);              } \
	            i = (int)(vz[zdz][ydy][xdx]) + min_dz; \
	            if ( i < _VT_SS_NINFINI )      { vz[z][y][x] = (s16)(_VT_SS_NINFINI); } \
	            else if ( i > _VT_SS_PINFINI ) { vz[z][y][x] = (s16)(_VT_SS_PINFINI); } \
	            else                           { vz[z][y][x] = (s16)(i);              } 

/*------ Static Functions ------*/
static void _VT_VectSeuil( vt_image *resIm, vt_image *theIm, vt_distance *par );





/* Calcul des 3 images formant les vecteurs designant les points les plus proches.
   
   Methode de Ragnemalm.

   Le champ seuil (float) de la structure de parametre definit le seuil pour
   binariser l'image. Les points dont la valeur est superieure ou
   egale au seuil definissent l'objet. 
   Pour les images codees sur des entiers, le seuil flottant est
   change en seuil entier en prenant le plus proche entier.

   Les 3 images resVx, resVy et resVz doivent etre de type
   SSHORT (s16 = signed short).

   Apres calcul, le point le plus proche du point (x,y,z)  de l'image
   theIm est le point de coordonnees
   ( x + (s16)(resVx.buf)[z * resVx.dim.y * resVx.dim.x + y * resVx.dim.x + x], 
     y + (s16)(resVy.buf)[z * resVy.dim.y * resVy.dim.x + y * resVy.dim.x + x],
     z + (s16)(resVz.buf)[z * resVz.dim.y * resVz.dim.x + y * resVz.dim.x + x] ) 
   ou 
   ( x + (s16***)(resVx.array)[z][y][x], 
     y + (s16***)(resVy.array)[z][y][x], 
     z + (s16***)(resVz.array)[z][y][x] )


RETURN
   Retourne FALSE en cas d'erreur.
*/

int VT_VecteurPPP_SS( vt_image *theIm /* input image */, 
		   vt_image *resVx /* image of X coordinates of vectors pointing towards nearest points */, 
		   vt_image *resVy /* image of Y coordinates of vectors pointing towards nearest points */, 
		   vt_image *resVz /* image of Z coordinates of vectors pointing towards nearest points */, 
		   vt_distance *par /* parameters */ )
{
    int dimx, dimy, dimz, dimx1, dimy1, dimz1;
    register int i, x, y, z, _INSIDEX_, _INSIDEY_, _INSIDEZ_, _NEW_;
    register int dx, dy, dz, xdx, ydy, zdz;
    int norme, min_nr, min_dx, min_dy, min_dz;
    s16 *vlx, *vly, *vlz;
    s16 ***vx, ***vy, ***vz;
    char *proc_name = "VT_VecteurPPP_SC";

    /*--- tests sur les images ---*/
    if ( VT_Test2Image( resVx, theIm, proc_name ) == -1 ) return( -1 );
    if ( VT_Test2Image( resVy, theIm, proc_name ) == -1 ) return( -1 );  
    if ( VT_Test2Image( resVz, theIm, proc_name ) == -1 ) return( -1 );  
    if ( ( theIm->dim.x < 3 ) || ( theIm->dim.y < 3 ) ) {
	VT_Error( "images have bad dimensions", proc_name );
	return( -1 );
    }
    if ( resVx->type != SSHORT ) {
	VT_Error( "first output image has a bad type", proc_name );
	return( -1 );
    }
    if ( resVy->type != SSHORT ) {
	VT_Error( "second output image has a bad type", proc_name );
	return( -1 );
    }
    if ( resVz->type != SSHORT ) {
	VT_Error( "third output image has a bad type", proc_name );
	return( -1 );
    }
    
    /*--- initialisation des dimensions ---*/
    dimx  = theIm->dim.x;   dimx1 = dimx - 1;
    dimy  = theIm->dim.y;   dimy1 = dimy - 1;
    dimz  = theIm->dim.z;   dimz1 = dimz - 1;
  
    /*--- initialisation des pointeurs ---*/
    vlx = (s16 *)resVx->buf;
    vly = (s16 *)resVy->buf;
    vlz = (s16 *)resVz->buf;
    vx = (s16***)resVx->array;
    vy = (s16***)resVy->array;
    vz = (s16***)resVz->array;
    
    /*--- seuillage ---*/
    _VT_VectSeuil( resVx, theIm, par );
    
    /*--- initialisation des autres images ---*/
    z = dimx * dimy * dimz;
    for ( i = 0; i < z ; i ++ )
	*(vly + i) = *(vlz + i) = *(vlx + i);

    /*--- premier parcours ---*/
    /*--- 
          X X X   X . .   . . .
          X X X   X O .   . . .
          X X X   X . .   . . . 
    ---*/
    _INSIDEZ_ = 0;
    for ( z = 0; z < dimz; z++ ) {
	if ( z > 0 ) _INSIDEZ_ = 1;
	for ( y = 0; y < dimy; y++ ) {
	    _INSIDEY_ = 1;
	    if ( (_INSIDEZ_ == 0) || (y == 0) || (y == dimy1) ) _INSIDEY_ = 0;
	    for ( x = 0; x < dimx; x++ ) {
		_INSIDEX_ = 1;
		if ( (_INSIDEY_ == 0) || (x == 0) || (x == dimx1) ) _INSIDEX_ = 0;
		/*--- point du fond ---*/
		if ( (vx[z][y][x] == 0) && (vy[z][y][x] == 0) && (vz[z][y][x] == 0) )
		    continue;
		/*--- norme ---*/
		norme = _VT_SS_NORME( (int)(vx[z][y][x]), (int)(vy[z][y][x]), (int)(vz[z][y][x]) );
		min_nr = norme;   _NEW_ = 0;
		min_dx = min_dy = min_dz = 0;
		/*--- tests sur les points ---*/
		if ( _INSIDEX_ == 1 ) {
		    /*--- les 9 points ---*/
		    dz = (-1);   zdz = z - 1;
		    for ( dy = (-1); dy < 2; dy ++ ) {
			ydy = y + dy;
			for ( dx = (-1); dx < 2; dx ++ ) {
			    xdx = x + dx;  
			    _TEST_NORME_
			}
		    }
		    /*--- les 3 points ---*/
		    dx = (-1);   xdx = x - 1;
		    dz = 0;      zdz = z;
		    for ( dy = (-1); dy < 2; dy ++ ) {
			ydy = y + dy;
			_TEST_NORME_
		    }
		} else {
		    /*--- les 9 points ---*/
		    if ( z > 0 ) {
			dz = (-1);   zdz = z - 1;
			for ( dy = (-1); dy < 2; dy ++ ) {
			    ydy = y + dy;
			    if ( (ydy < 0) || (ydy >= dimy) ) continue;
			    for ( dx = (-1); dx < 2; dx ++ ) {
				xdx = x + dx;   
				if ( (xdx < 0) || (xdx >= dimx) )   continue;
				_TEST_NORME_
			    }
			}
		    }
		    if ( x > 0 ) {
			dx = (-1);   xdx = x - 1;
			dz = 0;      zdz = z;
			for ( dy = (-1); dy < 2; dy ++ ) {
			    ydy = y + dy;
			    if ( (ydy < 0) || (ydy >= dimy) )   continue;
			    _TEST_NORME_
	                }
		    }
		}
		/*--- a-t-on un nouveau point ? ---*/
		_NEW_POINT_
	    }
	}
    }
    /*--- 
          X X X   X X X   X X X
          . . X   . O X   . . X
          . . .   . . .   . . . 
    ---*/
    _INSIDEY_ = 0;
    for ( y = 0; y < dimy; y++ ) {
	if ( y > 0 ) _INSIDEY_ = 1;
	for ( z = 0; z < dimz; z++ ) {
	    _INSIDEZ_ = 1;
	    if ( (_INSIDEY_ == 0) || (z == 0) || (z == dimz1) ) _INSIDEZ_ = 0;
	    for ( x = dimx1; x >= 0; x-- ) {
		_INSIDEX_ = 1;
		if ( (_INSIDEZ_ == 0) || (x == 0) || (x == dimx1) ) _INSIDEX_ = 0;
		/*--- point du fond ---*/
		if ( (vx[z][y][x] == 0) && (vy[z][y][x] == 0) && (vz[z][y][x] == 0) )
		    continue;
		/*--- norme ---*/
		norme = _VT_SS_NORME( (int)(vx[z][y][x]), (int)(vy[z][y][x]), (int)(vz[z][y][x]) );
		min_nr = norme;   _NEW_ = 0;
		min_dx = min_dy = min_dz = 0;
		/*--- tests sur les points ---*/
		if ( _INSIDEX_ == 1 ) {
		    /*--- les 9 points ---*/
		    dy = (-1);   ydy = y - 1;
		    for ( dz = (-1); dz < 2; dz ++ ) {
			zdz = z + dz;
			for ( dx = (-1); dx < 2; dx ++ ) {
			    xdx = x + dx;
			    _TEST_NORME_
			}
		    }
		    /*--- les 3 points ---*/
		    dx = 1;   xdx = x + 1;
		    dy = 0;   ydy = y;
		    for ( dz = (-1); dz < 2; dz ++ ) {
			zdz = z + dz;
			_TEST_NORME_
		    }
		} else {
		    /*--- les 9 points ---*/
		    if ( y > 0 ) {
			dy = (-1);   ydy = y - 1;
			for ( dz = (-1); dz < 2; dz ++ ) {
			    zdz = z + dz;
			    if ( (zdz < 0) || (zdz >= dimz) )   continue;
			    for ( dx = (-1); dx < 2; dx ++ ) {
				xdx = x + dx;
				if ( (xdx < 0) || (xdx >= dimx) )   continue;
				_TEST_NORME_
			    }
			}
		    }
		    /*--- les 3 points ---*/
		    if ( x < (dimx-1) ) {
			dx = 1;   xdx = x + 1;
			dy = 0;   ydy = y;
			for ( dz = (-1); dz < 2; dz ++ ) {
			    zdz = z + dz;
			    if ( (zdz < 0) || (zdz >= dimz) )   continue;
			    _TEST_NORME_
			}
		    }
		}
		/*--- a-t-on un nouveau point ? ---*/
		_NEW_POINT_
	    }
	}
    }
    /*--- 
          . . .   X . .   X X X
          . . .   X O .   X X X
          . . .   X . .   X X X 
    ---*/
    _INSIDEZ_ = 0;
    for ( z = dimz1; z >= 0; z-- ) {
	if ( z < dimz1 ) _INSIDEZ_ = 1;
	for ( y = 0; y < dimy; y++ ) {
	    _INSIDEY_ = 1;
	    if ( (_INSIDEZ_ == 0) || (y == 0) || (y == dimy1) ) _INSIDEY_ = 0;
	    for ( x = 0; x < dimx; x++ ) {
		_INSIDEX_ = 1;
		if ( (_INSIDEY_ == 0) || (x == 0) || (x == dimx1) ) _INSIDEX_ = 0;
		/*--- point du fond ---*/
		if ( (vx[z][y][x] == 0) && (vy[z][y][x] == 0) && (vz[z][y][x] == 0) )
		    continue;
		/*--- norme ---*/
		norme = _VT_SS_NORME( (int)(vx[z][y][x]), (int)(vy[z][y][x]), (int)(vz[z][y][x]) );
		min_nr = norme;   _NEW_ = 0;
		min_dx = min_dy = min_dz = 0;
		if ( _INSIDEX_ == 1 ) {
		    /*--- les 9 points ---*/
		    dz = 1;   zdz = z + 1;
		    for ( dy = (-1); dy < 2; dy ++ ) {
			ydy = y + dy;
			for ( dx = (-1); dx < 2; dx ++ ) {
			    xdx = x + dx; 
			    _TEST_NORME_
		        }
		    }
		    /*--- les 3 points ---*/
		    dx = (-1);   xdx = x - 1;
		    dz = 0;      zdz = z;
		    for ( dy = (-1); dy < 2; dy ++ ) {
			ydy = y + dy;
			_TEST_NORME_
		    }
		} else {
		    /*--- les 9 points ---*/
		    if ( z < dimz1 ) {
			dz = 1;   zdz = z + 1;
			for ( dy = (-1); dy < 2; dy ++ ) {
			    ydy = y + dy;
			    if ( (ydy < 0) || (ydy >= dimy) ) continue;
			    for ( dx = (-1); dx < 2; dx ++ ) {
				xdx = x + dx; 
				if ( (xdx < 0) || (xdx >= dimx) )   continue;
				_TEST_NORME_
			    }
			}
		    }
		    /*--- les 3 points ---*/
		    if ( x > 0 ) {
			dx = (-1);   xdx = x - 1;
			dz = 0;      zdz = z;
			for ( dy = (-1); dy < 2; dy ++ ) {
			    ydy = y + dy;
			    if ( (ydy < 0) || (ydy >= dimy) )   continue;
			    _TEST_NORME_
			}
		    }
		}
		/*--- a-t-on un nouveau point ? ---*/
		_NEW_POINT_
	    }
	}
    }

    /*--- 
          . . .   . . .   . . .
          . . X   . O X   . . X
          X X X   X X X   X X X 
    ---*/
    _INSIDEY_ = 0;
    for ( y = dimy1; y >= 0; y-- ) {
	if ( y < dimy1 ) _INSIDEY_ = 1;
	for ( z = 0; z < dimz; z++ ) {
	    _INSIDEZ_ = 1;
	    if ( (_INSIDEY_ == 0) || (z == 0) || (z == dimz1) ) _INSIDEZ_ = 0;
	    for ( x = dimx1; x >= 0; x-- ) {
		_INSIDEX_ = 1;
		if ( (_INSIDEZ_ == 0) || (x == 0) || (x == dimx1) ) _INSIDEX_ = 0;
		/*--- point du fond ---*/
		if ( (vx[z][y][x] == 0) && (vy[z][y][x] == 0) && (vz[z][y][x] == 0) )
		    continue;
		/*--- norme ---*/
		norme = _VT_SS_NORME( (int)(vx[z][y][x]), (int)(vy[z][y][x]), (int)(vz[z][y][x]) );
		min_nr = norme;   _NEW_ = 0;
		min_dx = min_dy = min_dz = 0;
		if ( _INSIDEX_ == 1 ) {
		    /*--- les 9 points ---*/
		    dy = 1;   ydy = y + 1;
		    for ( dz = (-1); dz < 2; dz ++ ) {
			zdz = z + dz;
			for ( dx = (-1); dx < 2; dx ++ ) {
			    xdx = x + dx; 
			    _TEST_NORME_
		        }
		    }
		    /*--- les 3 points ---*/
		    dx = 1;   xdx = x + 1;
		    dy = 0;   ydy = y;
		    for ( dz = (-1); dz < 2; dz ++ ) {
			zdz = z + dz;
			_TEST_NORME_
	            }
		} else {
		    /*--- les 9 points ---*/
		    if ( y < dimy1 ) {
			dy = 1;   ydy = y + 1;
			for ( dz = (-1); dz < 2; dz ++ ) {
			    zdz = z + dz;
			    if ( (zdz < 0) || (zdz >= dimz) )   continue;
			    for ( dx = (-1); dx < 2; dx ++ ) {
				xdx = x + dx; 
				if ( (xdx < 0) || (xdx >= dimx) )   continue;
				_TEST_NORME_
			    }
			}
		    }
		    /*--- les 3 points ---*/
		    if ( x < (dimx-1) ) {
			dx = 1;   xdx = x + 1;
			dy = 0;   ydy = y;
			for ( dz = (-1); dz < 2; dz ++ ) {
			    zdz = z + dz;
			    if ( (zdz < 0) || (zdz >= dimz) )   continue;
			    _TEST_NORME_
			}
		    }
		}
		/*--- a-t-on un nouveau point ? ---*/
		_NEW_POINT_
	    }
	}
    }

    /*--- fin ---*/
    return( 1 );
}





static void _VT_VectSeuil( vt_image *resIm, vt_image *theIm, vt_distance *par )
{
	register int is, i;
	int v;
	register float fs;
	s16 *vl;

	fs = par->seuil;
	is = VT_F2I( fs );
	v = theIm->dim.x * theIm->dim.y * theIm->dim.z;
	vl = (s16*)(resIm->buf);

	switch ( theIm->type ) {
	case UCHAR :
		{
		u8 *buf;
		buf = (u8 *)(theIm->buf);
		for (i=0; i<v; i++) 
			*vl++ = ( *buf++ < is ) ? _VT_SS_PINFINI : 0;
	        }
		break;
	case SCHAR :
		{
		s8 *buf;
		buf = (s8 *)(theIm->buf);
		for (i=0; i<v; i++) 
			*vl++ = ( *buf++ < is ) ? _VT_SS_PINFINI : 0;
	        }
		break;
	case USHORT :
		{
		u16 *buf;
		buf = (u16 *)(theIm->buf);
		for (i=0; i<v; i++) 
			*vl++ = ( *buf++ < is ) ? _VT_SS_PINFINI : 0;
	        }
		break;
	case SSHORT :
		{
		s16 *buf;
		buf = (s16 *)(theIm->buf);
		for (i=0; i<v; i++) 
			*vl++ = ( *buf++ < is ) ? _VT_SS_PINFINI : 0;
	        }
		break;
	case SINT :
		{
		i32 *buf;
		buf = (i32 *)(theIm->buf);
		for (i=0; i<v; i++) 
			*vl++ = ( *buf++ < is ) ? _VT_SS_PINFINI : 0;
	        }
		break;
	case FLOAT :
		{
		r32 *buf;
		buf = (r32 *)(theIm->buf);
		for (i=0; i<v; i++) 
			*vl++ = ( *buf++ < fs ) ? _VT_SS_PINFINI : 0;
	        }
	default :
	        for ( i = 0; i < v; i ++ )
		    *vl++ = ( VT_GetINDvalue( theIm, i ) < fs ) ? _VT_SS_PINFINI : 0;
	}
}






