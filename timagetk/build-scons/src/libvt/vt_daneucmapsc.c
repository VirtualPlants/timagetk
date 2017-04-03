
#include <vt_daneucmapsc.h>

#define VT_F2I( F ) ( (F) >= 0.0 ? ((int)((F)+0.5)) : ((int)((F)-0.5)) )

/*--- infinis positif et negatif ---*/
#define _VT_SC_PINFINI  127
#define _VT_SC_NINFINI -128
#define _VT_SC_NORME(X,Y,Z) ( (X)*(X) + (Y)*(Y) + (Z)*(Z) )

/*------ Static Functions ------*/
static void _VT_AutreZ( s8 ***vx, s8 ***vy, s8 ***vz, int x, int y, int z, int dim[2], int dz );
static void _VT_AutreY( s8 ***vx, s8 ***vy, s8 ***vz, int x, int y, int z, int dim[2], int dy );
static void _VT_AutreX( s8 ***vx, s8 ***vy, s8 ***vz, int x, int y, int z, int dx );
static void _VT_VectSeuil( vt_image *resIm, vt_image *theIm, vt_distance *par );





/* Calcul des 3 images formant les vecteurs designant les points les plus proches.
   
   Methode de Danielsson.

   Le champ seuil (float) de la structure de parametre definit le seuil pour
   binariser l'image. Les points dont la valeur est superieure ou
   egale au seuil definissent l'objet. 
   Pour les images codees sur des entiers, le seuil flottant est
   change en seuil entier en prenant le plus proche entier.

   Les 3 images resVx, resVy et resVz doivent etre de type
   SCHAR (s8 = signed char).

   Apres calcul, le point le plus proche du point (x,y,z)  de l'image
   theIm est le point de coordonnees
   ( x + (s8)(resVx.buf)[z * resVx.dim.y * resVx.dim.x + y * resVx.dim.x + x], 
     y + (s8)(resVy.buf)[z * resVy.dim.y * resVy.dim.x + y * resVy.dim.x + x],
     z + (s8)(resVz.buf)[z * resVz.dim.y * resVz.dim.x + y * resVz.dim.x + x] ) 
   ou 
   ( x + (s8***)(resVx.array)[z][y][x], 
     y + (s8***)(resVy.array)[z][y][x], 
     z + (s8***)(resVz.array)[z][y][x] )


RETURN
   Retourne FALSE en cas d'erreur.
*/

int VT_DanVecteurPPP_SC( vt_image *theIm /* input image */, 
               vt_image *resVx /* image of X coordinates of vectors pointing towards nearest points */,
               vt_image *resVy /* image of Y coordinates of vectors pointing towards nearest points */,
               vt_image *resVz /* image of Z coordinates of vectors pointing towards nearest points */,
               vt_distance *par /* parameters */ )
{
    int dim[2];
    int dimx, dimy, dimz, dimxy, dimxyz;
    register int i, x, y, z;
    s8 *vlx, *vly, *vlz;
    s8 ***pvlx, ***pvly, ***pvlz;
    char *proc_name = "VT_VecteurPPP_SC";

    /*--- tests sur les images ---*/
    if ( VT_Test2Image( resVx, theIm, proc_name ) == -1 ) return( -1 );
    if ( VT_Test2Image( resVy, theIm, proc_name ) == -1 ) return( -1 );  
    if ( VT_Test2Image( resVz, theIm, proc_name ) == -1 ) return( -1 );  
    if ( ( theIm->dim.x < 3 ) || ( theIm->dim.y < 3 ) ) {
      VT_Error( "images have bad dimensions", proc_name );
      return( -1 );
    }
    if ( resVx->type != SCHAR ) {
      VT_Error( "first output image has a bad type", proc_name );
      return( -1 );
    }
    if ( resVy->type != SCHAR ) {
      VT_Error( "second output image has a bad type", proc_name );
      return( -1 );
    }
    if ( resVz->type != SCHAR ) {
      VT_Error( "third output image has a bad type", proc_name );
      return( -1 );
    }
    
    /*--- initialisation des dimensions ---*/
    dimx  = theIm->dim.x;
    dimy  = theIm->dim.y;
    dimz  = theIm->dim.z;
    dimxy = dimx*dimy;
    dim[0] = dimx - 1;
    dim[1] = dimy - 1;
    dimxyz = dimxy * dimz;
  
    /*--- initialisation des pointeurs ---*/
    vlx = (s8 *)resVx->buf;
    vly = (s8 *)resVy->buf;
    vlz = (s8 *)resVz->buf;
    pvlx = (s8***)resVx->array;
    pvly = (s8***)resVy->array;
    pvlz = (s8***)resVz->array;
    
    /*--- seuillage ---*/
    _VT_VectSeuil( resVx, theIm, par );
    
    /*--- initialisation des autres images ---*/
    for ( i = 0; i < dimxyz ; i ++ )
      *(vly + i) = *(vlz + i) = *(vlx + i);
    
    /*   boucle de la descente   */
    /*****************************/
    for ( z = 0; z < dimz; z++ ) {
      for ( y = 0; y < dimy; y++ ) {

          /*--- premier parcours : sens video ---*/
          if ( z == 0 ) {
            /*--- premier parcours (b) : on parcourt la ligne superieure  ---*/
            /*--- premier parcours (c) : on parcourt la colonne de gauche ---*/
            /*--- x = 0 ---*/
            if ( (pvlx[z][y][0] != 0) || (pvly[z][y][0] != 0) || (pvlz[z][y][0] != 0) ) {
                if ( y > 0 )
                  _VT_AutreY( pvlx, pvly, pvlz, (int)0, y, z, dim, (int)(-1) );
            }
            /*--- x > 0 ---*/
            for ( x = 1; x < dimx; x++ ) {
                if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) ) {
                  if ( y > 0 )
                      _VT_AutreY( pvlx, pvly, pvlz, x, y, z, dim, (int)(-1) );
                  _VT_AutreX( pvlx, pvly, pvlz, x, y, z, (int)(-1) );
                }
            }
          } else {
            /*--- premier parcours (a) : on parcourt le plan superieur    ---*/
            /*--- premier parcours (b) : on parcourt la ligne superieure  ---*/
            /*--- premier parcours (c) : on parcourt la colonne de gauche ---*/
            /*--- x = 0 ---*/
            if ( (pvlx[z][y][0] != 0) || (pvly[z][y][0] != 0) || (pvlz[z][y][0] != 0) ) {
                _VT_AutreZ( pvlx, pvly, pvlz, (int)0, y, z, dim, (int)(-1) );
                if ( y > 0 )
                  _VT_AutreY( pvlx, pvly, pvlz, (int)0, y, z, dim, (int)(-1) );
            }
            /*--- x > 0 ---*/
            for ( x = 1; x < dimx; x++ ) {
                if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) ) {
                  _VT_AutreZ( pvlx, pvly, pvlz, x, y, z, dim, (int)(-1) );
                  if ( y > 0 )
                      _VT_AutreY( pvlx, pvly, pvlz, x, y, z, dim, (int)(-1) );
                  _VT_AutreX( pvlx, pvly, pvlz, x, y, z, (int)(-1) );
                }
            }
          }

          /*--- deuxieme parcours : sens video inverse selon X ---*/
          for ( x = dimx-2; x >= 0; x-- ) {
            if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) )
                _VT_AutreX( pvlx, pvly, pvlz, x, y, z, (int)(1) );
          }
      }

      /*--- troisieme parcours : sens video inverse selon Y et X ---*/
      /*--- y = dimy - 1 = dim[1] ---*/
      for ( x = dimx-2; x >= 0; x-- ) {
          if ( (pvlx[z][dim[1]][x] != 0) || (pvly[z][dim[1]][x] != 0) || (pvlz[z][dim[1]][x] != 0) )
            _VT_AutreX( pvlx, pvly, pvlz, x, dim[1], z, (int)(1) );
      }
      /*--- y < dimy - 1 = dim[1] ---*/
      for ( y = dimy-2; y >= 0; y-- ) {
          /*--- x = dimx - 1 = dim[0] ---*/
          if ( (pvlx[z][y][dim[0]] != 0) || (pvly[z][y][dim[0]] != 0) || (pvlz[z][y][dim[0]] != 0) )
            _VT_AutreY( pvlx, pvly, pvlz, dim[0], y, z, dim, (int)(1) );
          /*--- x < dimx - 1 = dim[0] ---*/
          for ( x = dimx-2; x >= 0; x-- ) {
            if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) ) {
                _VT_AutreY( pvlx, pvly, pvlz, x, y, z, dim, (int)(1) );
                _VT_AutreX( pvlx, pvly, pvlz, x, y, z, (int)(1) );
            }
          }
      }
    }
    
    /*   boucle de la remontee   */
    /*****************************/
    for ( z = dimz-1; z >= 0; z-- ) {
      for ( y = dim[1]; y >= 0; y-- ) {

          /*--- quatrieme parcours : sens video inverse ---*/
          if ( z == dimz-1 ) {
            /*--- quatrieme parcours (b) : on parcourt la ligne inferieure  ---*/
            /*--- quatrieme parcours (c) : on parcourt la colonne de droite ---*/
            /*--- x = dimx - 1 = dim[0] ---*/
            if ( (pvlx[z][y][dim[0]] != 0) || (pvly[z][y][dim[0]] != 0) || (pvlz[z][y][dim[0]] != 0) ) {
                if ( y < dim[1] )
                  _VT_AutreY( pvlx, pvly, pvlz, dim[0], y, z, dim, (int)(1) );
            }
            /*--- x < dimx - 1 = dim[0] ---*/
            for ( x = dimx-2; x >= 0; x-- ) {
                if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) ) {
                  if ( y < dim[1] )
                      _VT_AutreY( pvlx, pvly, pvlz, x, y, z, dim, (int)(1) );
                  _VT_AutreX( pvlx, pvly, pvlz, x, y, z, (int)(1) );
                }
            }
          } else {
            /*--- quatrieme parcours (a) : on parcourt le plan inferieur    ---*/
            /*--- quatrieme parcours (b) : on parcourt la ligne inferieure  ---*/
            /*--- quatrieme parcours (c) : on parcourt la colonne de droite ---*/
            /*--- x = dimx - 1 = dim[0] ---*/
            if ( (pvlx[z][y][dim[0]] != 0) || (pvly[z][y][dim[0]] != 0) || (pvlz[z][y][dim[0]] != 0) ) {
                _VT_AutreZ( pvlx, pvly, pvlz, dim[0], y, z, dim, (int)(1) );
                if ( y < dim[1] )
                  _VT_AutreY( pvlx, pvly, pvlz, dim[0], y, z, dim, (int)(1) );
            }
            /*--- x < dimx - 1 = dim[0] ---*/
            for ( x = dimx-2; x >= 0; x-- ) {
                if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) ) {
                  _VT_AutreZ( pvlx, pvly, pvlz, x, y, z, dim, (int)(1) );
                  if ( y < dim[1] )
                      _VT_AutreY( pvlx, pvly, pvlz, x, y, z, dim, (int)(1) );
                  _VT_AutreX( pvlx, pvly, pvlz, x, y, z, (int)(1) );
                }
            }
          }

          /*--- cinquieme parcours : sens video selon X ---*/
          for ( x = 1; x < dimx; x++ ) {
            if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) )
                _VT_AutreX( pvlx, pvly, pvlz, x, y, z, (int)(-1) );
          }
      }

      /*--- sixieme parcours : sens video selon X et Y ---*/
      /*--- y = 0 ---*/
      for ( x = 1; x < dimx; x++ ) {
          if ( (pvlx[z][0][x] != 0) || (pvly[z][0][x] != 0) || (pvlz[z][0][x] != 0) )
            _VT_AutreX( pvlx, pvly, pvlz, x, (int)0, z, (int)(-1) );
      }
      /*--- y > 0 ---*/
      for ( y = 1; y < dimy; y++ ) {
          /*--- x = 0 ---*/
          if ( (pvlx[z][y][0] != 0) || (pvly[z][y][0] != 0) || (pvlz[z][y][0] != 0) )
              _VT_AutreY( pvlx, pvly, pvlz, dim[0], y, z, dim, (int)(-1) );
          /*--- x > 0 ---*/
          for ( x = 1; x < dimx; x++ ) {
            if ( (pvlx[z][y][x] != 0) || (pvly[z][y][x] != 0) || (pvlz[z][y][x] != 0) ) {
                _VT_AutreY( pvlx, pvly, pvlz, dim[0], y, z, dim, (int)(-1) );
                _VT_AutreX( pvlx, pvly, pvlz, x, (int)0, z, (int)(-1) );
            }
          }
      }
    }
    
    /*--- fin ---*/
    return( 1 );
}




static void _VT_AutreZ( s8 ***vx, s8 ***vy, s8 ***vz, int x, int y, int z, int dim[2], int dz )
{
    /*--- on teste les 9 points d'un autre plan : dz = {-1, +1} :
          le test ( (z+dz >= 0) && (z+dz < dimz) ) est suppose vrai ---*/

    register int xdx, ydy, zdz;
    int n, m, dx=0, dy, ddx, ddy, ddz;
  
    m = n = _VT_SC_NORME( (int)(vx[z][y][x]), (int)(vy[z][y][x]), (int)(vz[z][y][x]) );
    ddx = ddy = ddz = 0;
    zdz = z + dz;
  
    /*--- test des 9 points dans le plan au-dessus ou au-dessous du point courant ---*/
    for ( dy = -1; dy < 2; dy ++ ) {
      ydy = y + dy;
      if ( ydy < 0 )      continue;
      if ( ydy > dim[1] ) continue;

      for ( dx = -1; dx < 2; dx ++ ) {
          xdx = x + dx;
          if ( xdx < 0 )      continue;
          if ( xdx > dim[0] ) continue;

          m = _VT_SC_NORME( ((int)(vx[zdz][ydy][xdx]) + dx), ((int)(vy[zdz][ydy][xdx]) + dy), ((int)(vz[zdz][ydy][xdx]) + dz) );
          if (m >= n)         continue;

          ddx = dx;
          ddy = dy;
          ddz = dz;
          n   = m;
      }
    }
    /*--- a-t-on trouve un point ? si oui => ddz == dz :
          on verifie que le nouveau vecteur est OK       ---*/
    if ( ddz == dz ) {
      ydy = y + ddy;
      xdx = x + ddx;
      n = (int)(vx[zdz][ydy][xdx]) + ddx;
      if ( n < _VT_SC_NINFINI )      { vx[z][y][x] = (s8)(_VT_SC_NINFINI); }
      else if ( n > _VT_SC_PINFINI ) { vx[z][y][x] = (s8)(_VT_SC_PINFINI); }
      else                           { vx[z][y][x] = (s8)(n);              }
      n = (int)(vy[zdz][ydy][xdx]) + ddy;
      if ( n < _VT_SC_NINFINI )      { vy[z][y][x] = (s8)(_VT_SC_NINFINI); }
      else if ( n > _VT_SC_PINFINI ) { vy[z][y][x] = (s8)(_VT_SC_PINFINI); }
      else                           { vy[z][y][x] = (s8)(n);              }
      n = (int)(vz[zdz][ydy][xdx]) + dz;
      if ( n < _VT_SC_NINFINI )      { vz[z][y][x] = (s8)(_VT_SC_NINFINI); }
      else if ( n > _VT_SC_PINFINI ) { vz[z][y][x] = (s8)(_VT_SC_PINFINI); }
      else                           { vz[z][y][x] = (s8)(n);              }
    }
}





static void _VT_AutreY( s8 ***vx, s8 ***vy, s8 ***vz, int x, int y, int z, int dim[2], int dy )
{
    /*--- on teste les 3 points d'une autre ligne : dy = {-1, +1} :
          le test ( (y+dy >= 0) && (y+dy < dimy) ) est suppose vrai ---*/

    register int xdx, ydy;
    int n, m, dx, ddx, ddy;
  
    m = n = _VT_SC_NORME( (int)(vx[z][y][x]), (int)(vy[z][y][x]), (int)(vz[z][y][x]) );
    ddx = ddy = 0;
    ydy = y + dy;

    /*--- test des 3 points dans la ligne au-dessus ou au-dessous du point courant ---*/
    for ( dx = -1; dx < 2; dx ++ ) {
      xdx = x + dx;
      if ( xdx < 0 )      continue;
      if ( xdx > dim[0] ) continue;

      m = _VT_SC_NORME( ((int)(vx[z][ydy][xdx]) + dx), ((int)(vy[z][ydy][xdx]) + dy), (int)(vz[z][ydy][xdx]) );
      if (m >= n)         continue;

      ddx = dx;
      ddy = dy;
      n   = m;
    }

    /*--- a-t-on trouve un point ? si oui => ddy == dy :
          on verifie que le nouveau vecteur est OK       ---*/
    if ( ddy == dy ) {
      xdx = x + ddx;
      n = (int)(vx[z][ydy][xdx]) + ddx;
      if ( n < _VT_SC_NINFINI )      { vx[z][y][x] = (s8)(_VT_SC_NINFINI); }
      else if ( n > _VT_SC_PINFINI ) { vx[z][y][x] = (s8)(_VT_SC_PINFINI); }
      else                           { vx[z][y][x] = (s8)(n);              }
      n = (int)(vy[z][ydy][xdx]) + dy;
      if ( n < _VT_SC_NINFINI )      { vy[z][y][x] = (s8)(_VT_SC_NINFINI); }
      else if ( n > _VT_SC_PINFINI ) { vy[z][y][x] = (s8)(_VT_SC_PINFINI); }
      else                           { vy[z][y][x] = (s8)(n);              }
      vz[z][y][x] = vz[z][ydy][xdx];
    }
}





static void _VT_AutreX( s8 ***vx, s8 ***vy, s8 ***vz, int x, int y, int z, int dx )
{
    /*--- on teste le point d'une autre colonne : dx = {-1, +1} :
          le test ( (x+dx >= 0) && (x+dx < dimx) ) est suppose vrai ---*/

    register int xdx;
    int n, m;
  
    n = _VT_SC_NORME( (int)(vx[z][y][x]), (int)(vy[z][y][x]), (int)(vz[z][y][x]) );
    xdx = x + dx;

    m = _VT_SC_NORME( ((int)(vx[z][y][xdx]) + dx), (int)(vy[z][y][xdx]), (int)(vz[z][y][xdx]) );

    if (m >= n)  return;
    
    /*--- a-t-on trouve un point ? 
          on verifie que le nouveau vecteur est OK       ---*/
    n = (int)(vx[z][y][xdx]) + dx;
    if ( n < _VT_SC_NINFINI )      { vx[z][y][x] = (s8)(_VT_SC_NINFINI); }
    else if ( n > _VT_SC_PINFINI ) { vx[z][y][x] = (s8)(_VT_SC_PINFINI); }
    else                           { vx[z][y][x] = (s8)(n);              }
    vy[z][y][x] = vy[z][y][xdx];
    vz[z][y][x] = vz[z][y][xdx];
}





static void _VT_VectSeuil( vt_image *resIm, vt_image *theIm, vt_distance *par )
{
      register int is, i;
      int v;
      register float fs;
      s8 *vl;

      fs = par->seuil;
      is = VT_F2I( fs );
      v = theIm->dim.x * theIm->dim.y * theIm->dim.z;
      vl = (s8*)(resIm->buf);

      switch ( theIm->type ) {
      case UCHAR :
            {
            u8 *buf;
            buf = (u8 *)(theIm->buf);
            for (i=0; i<v; i++)
                  *vl++ = ( *buf++ < is ) ? _VT_SC_PINFINI : 0;
              }
            break;
      case SCHAR :
            {
            s8 *buf;
            buf = (s8 *)(theIm->buf);
            for (i=0; i<v; i++)
                  *vl++ = ( *buf++ < is ) ? _VT_SC_PINFINI : 0;
              }
            break;
      case USHORT :
            {
            u16 *buf;
            buf = (u16 *)(theIm->buf);
            for (i=0; i<v; i++)
                  *vl++ = ( *buf++ < is ) ? _VT_SC_PINFINI : 0;
              }
            break;
      case SSHORT :
            {
            s16 *buf;
            buf = (s16 *)(theIm->buf);
            for (i=0; i<v; i++)
                  *vl++ = ( *buf++ < is ) ? _VT_SC_PINFINI : 0;
              }
            break;
      case SINT :
            {
            i32 *buf;
            buf = (i32 *)(theIm->buf);
            for (i=0; i<v; i++)
                  *vl++ = ( *buf++ < is ) ? _VT_SC_PINFINI : 0;
              }
            break;
      case FLOAT :
            {
            r32 *buf;
            buf = (r32 *)(theIm->buf);
            for (i=0; i<v; i++)
                  *vl++ = ( *buf++ < fs ) ? _VT_SC_PINFINI : 0;
              }
      default :
              for ( i = 0; i < v; i ++ )
                *vl++ = ( VT_GetINDvalue( theIm, i ) < fs ) ? _VT_SC_PINFINI : 0;
      }
}


