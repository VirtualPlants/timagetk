#include <vt_average.h>

/* Filtrage moyenne 
   
   Si les dimensions de la fenetre sont negatives ou nulles,
   on les prend a 1.
   La fenetre est centree autour du point (x,y,z) de la facon
   suivante :
   [ v - (int)fenetre.v / 2, v + (int)(fenetre - fenetre.v / 2 - 1]
   pour v = {x,y,z}
   
 */
int VT_AverageFilter( vt_image *theIm, vt_image *resIm, vt_ipt *window )
{ 
  char *proc="VT_AverageFilter";
  vt_ipt local_window;
  vt_ipt negOffset, posOffset;
  register double sum;
  register int x, y, z, i, j, k, nb;
  int nbtot;
  
  /*--- tests sur les images ---*/
  if ( VT_Test2Image( resIm, theIm, proc ) == -1 ) return( -1 );
  if ( theIm->type != resIm->type ) {
    VT_Error("images have different types", proc);
    return( -1 );
  }
  if ( (theIm->buf == resIm->buf) || (theIm->array == resIm->array) ) {
    VT_Error("images share same memory", proc);
    return( -1 );
  }

  /*--- determination de la fenetre de calcul ---*/
  local_window = *window;
  if ( local_window.x <= 0 ) local_window.x = 1;
  if ( local_window.y <= 0 ) local_window.y = 1;
  if ( local_window.z <= 0 ) local_window.z = 1;
  if ( local_window.x > (int)theIm->dim.x ) local_window.x = theIm->dim.x;
  if ( local_window.y > (int)theIm->dim.y ) local_window.y = theIm->dim.y;
  if ( local_window.z > (int)theIm->dim.z ) local_window.z = theIm->dim.z;
  if ( (local_window.x == 1) && (local_window.y == 1) && (local_window.z == 1) ) {
    if ( VT_CopyImage( theIm, resIm ) != 1 ) {
      VT_Error( "error while copying", proc );
      return( -1 );
    }
    return( 1 );
  }
  negOffset.x = -(int)(local_window.x / 2);   posOffset.x = local_window.x - 1 + negOffset.x;
  negOffset.y = -(int)(local_window.y / 2);   posOffset.y = local_window.y - 1 + negOffset.y;
  negOffset.z = -(int)(local_window.z / 2);   posOffset.z = local_window.z - 1 + negOffset.z;
  nbtot = local_window.x * local_window.y * local_window.z;

#define _AVERAGE_FILTERING_( TYPE, TAB, TMP ) {                    \
    TYPE ***theBuf = (TYPE***)theIm->array;                \
    TYPE ***resBuf = (TYPE***)resIm->array;                \
                                        \
    for ( z = 0; z < (int)theIm->dim.z; z ++ )                    \
    for ( y = 0; y < (int)theIm->dim.y; y ++ )                    \
    for ( x = 0; x < (int)theIm->dim.x; x ++ ) {                    \
      nb = 0;                                        \
      sum = 0.0;                                \
      /*--- saisie du voisinage ---*/                        \
      if ( (z + negOffset.z < 0) || (z + posOffset.z >= (int)theIm->dim.z) ||    \
     (y + negOffset.y < 0) || (y + posOffset.y >= (int)theIm->dim.y) ||    \
     (x + negOffset.x < 0) || (x + posOffset.x >= (int)theIm->dim.x) ) {    \
    /*--- borders ---*/                            \
    for ( k = negOffset.z; k <= posOffset.z; k ++ ) {            \
    if ( (z + k < 0) || (z + k >= (int)theIm->dim.z) ) continue;        \
      for ( j = negOffset.y; j <= posOffset.y; j ++ ) {            \
      if ( (y + j < 0) || (y + j >= (int)theIm->dim.y) ) continue;        \
        for ( i = negOffset.x; i <= posOffset.x; i ++ ) {            \
        if ( (x + i < 0) || (x + i >= (int)theIm->dim.x) ) continue;        \
          sum += (double)theBuf[z+k][y+j][x+i];                    \
              nb ++;                                                            \
        }                                    \
      }                                    \
    }                                    \
      } else {                                     \
    nb = nbtot;                                                             \
    /*--- deep inside the image ---*/                    \
    for ( k = negOffset.z; k <= posOffset.z; k ++ )                \
    for ( j = negOffset.y; j <= posOffset.y; j ++ )                \
    for ( i = negOffset.x; i <= posOffset.x; i ++ ) {            \
      sum += (double)theBuf[z+k][y+j][x+i];                        \
    }                                    \
      }                                        \
      /*--- computing the median value ---*/                    \
      resBuf[z][y][x] = (TYPE)(sum / (double)nb);                \
    }                                        \
  }
  
  switch ( theIm->type ) {
  case UCHAR :
    _AVERAGE_FILTERING_( u8, tabInt, tmpInt )
    break;
  case SCHAR :
    _AVERAGE_FILTERING_( s8, tabInt, tmpInt )
    break;
  case USHORT :
    _AVERAGE_FILTERING_( u16, tabInt, tmpInt )
    break;
  case SSHORT :
    _AVERAGE_FILTERING_( s16, tabInt, tmpInt )
    break;
  case FLOAT :
    _AVERAGE_FILTERING_( r32, tabFloat, tmpFloat )
    break;
  case SINT :
    _AVERAGE_FILTERING_( i32, tabInt, tmpInt )
    break;
  default :
    VT_Error( "output image type unknown or not supported", proc );
    return( -1 );
  }

  return( 1 );
}
