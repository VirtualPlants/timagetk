
#include <math.h>

#include <vt_gausssmooth.h>




#define _MAX_HALF_LENGTH_ 11
static double _local_coeff[ _MAX_HALF_LENGTH_ + 1];
static int    _local_offst[ _MAX_HALF_LENGTH_ + 1];
static int _half_length = _MAX_HALF_LENGTH_;





/* si on veut calculer la longueur du masque
   en fonction d'une erreur sur l'integrale de la gaussienne :
   on a 
   int(exp(-(x^2)/2), x=-infinity..infinity ) = sqrt( 2 pi )
   int(exp(-(x^2)/2), x=-a..a)                = sqrt( 2 pi ) erf ( sqrt(2)/2 a )
   int( exp(-(x^2)/(2*s^2)), x=-a..a )        = sqrt( 2 pi ) s erf ( sqrt(2)/2 a/s )
   d'ou
   int( exp(-(x^2)/(2*s^2)), x=-infinity..infinity ) = sqrt( 2 pi ) s 
   */

static void VT_InitCoeff( double sigma, int length )
{
  int l;
  int i;
  double mul = 0.5; /* sigma = 1.0 */
  double sum, s = 0.0;

  /*--- mask size :
        if it is even, make it odd ---*/
  l = length;
  if ( 2 * (l/2) == l ) l--;
  _half_length = l / 2;
  if ( _half_length <= 0 )                _half_length = 0;
  if ( _half_length > _MAX_HALF_LENGTH_ ) _half_length = _MAX_HALF_LENGTH_;

  /*--- initialization ---*/
  for ( i = 0 ; i <= _MAX_HALF_LENGTH_; i ++ ) {
    _local_coeff[i] = 0.0;
    _local_offst[i] = 0;
  }
  if ( sigma > 0.0 ) mul = 0.5 / ( sigma * sigma );
  
  /*--- coefficients ---*/
  _local_coeff[0] = 1.0;
  for ( i = 1 ; i <= _half_length; i ++ ) {
    _local_coeff[ i ] = exp( (double)(-mul * (double)i * (double)i) );
    s += _local_coeff[ i ];
  }
  /*--- normalization ---*/
  sum = 1.0 + 2.0 * s;
  for ( i = 0 ; i <= _half_length; i ++ ) 
    _local_coeff[ i ] /= sum;
}





int VT_GaussianSmoothing( vt_image *theIm, vt_image *resIm, double sigma, int length )
{
  vt_image *input;
  char *proc="VT_GaussianSmoothing";
  double *tmp = (double*)NULL;
  register double r, s;
  register int offset, ind, i, j, x, y, z;
  int dx, dy, dz, dxy, dh;

  if ( VT_Test2Image( resIm, theIm, proc ) == -1 ) return( -1 );
  if ( theIm->type != resIm->type ) {
    VT_Error("images have different types",proc);
    return( -1 );
  }
  if ( theIm->buf == resIm->buf ) {
    VT_Error("images share the same buffer",proc);
    return( -1 );
  }

  /*--- initialisation ---*/
  dx = theIm->dim.x;
  dy = theIm->dim.y;
  dz = theIm->dim.z;
  dxy = dx * dy;
  VT_InitCoeff( sigma, length );
  if ( dy > dz ) tmp = (double*)VT_Malloc( (unsigned int)(dy * sizeof(double)) );
  else           tmp = (double*)VT_Malloc( (unsigned int)(dz * sizeof(double)) );
  if ( tmp == (double*)NULL ) {
    VT_Error( "allocation failed for auxiliary buffer", proc );
    return( -1 );
  }
  input = theIm;

#define _X_FILTERING( TYPE ) {                                                          \
  TYPE *theBuf = (TYPE*)(input->buf);                                                   \
  TYPE *resBuf = (TYPE*)(resIm->buf);                                                   \
  for ( z = 0; z < dz ; z ++ ) 								\
  for ( y = 0; y < dy ; y ++ ) {							\
    offset = z * dxy + y * dx;								\
    for ( x = 0; x < _half_length ; x ++ ) {						\
      /*--- the first points ---*/							\
      ind = offset + x;									\
      s = _local_coeff[0];								\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= x; i ++ ) {							\
	r += ( (double)theBuf[ind + i] + (double)theBuf[ind - i] ) * _local_coeff[i];	\
	s += 2.0 * _local_coeff[i];							\
      }											\
      for ( i = x+1; i <= _half_length ; i ++ ) {					\
	r += (double)theBuf[ind + i] * _local_coeff[i];					\
	s += _local_coeff[i];								\
      }											\
      tmp[x] = r/s;		        						\
      /*--- the last points ---*/							\
      j = dx - 1 - x;		        						\
      ind = offset + j;		                   					\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= x; i ++ )							\
	r += ( (double)theBuf[ind + i] + (double)theBuf[ind - i] ) * _local_coeff[i];	\
      for ( i = x+1; i <= _half_length ; i ++ )						\
	r += (double)theBuf[ind - i] * _local_coeff[i];					\
      tmp[j] = r/s;            								\
    }											\
    for ( x = _half_length ; x < dh; x ++ ) {		                 		\
      /*--- the middle points ---*/							\
      ind = offset + x;									\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= _half_length; i ++ ) 						\
	r += ( (double)theBuf[ind + i] + (double)theBuf[ind - i] ) * _local_coeff[i];	\
      tmp[x] = r;	         							\
    }											\
    for ( x = 0, ind = offset; x < dx; x ++, ind ++ ) resBuf[ind] = tmp[y];             \
  }                                                                                     \
}

  if ( dx > 2 ) {
    /*--- filtering along x ---*/
    if ( 2 * _half_length + 1 > dx )
      VT_InitCoeff( sigma, dx );
    dh = dx - _half_length;
    
    switch ( theIm->type ) {
    case UCHAR :
      _X_FILTERING( u8 )
      break;
    case SCHAR :
      _X_FILTERING( s8 )
      break;
    case USHORT :
      _X_FILTERING( u16 )
      break;
    case SSHORT :
      _X_FILTERING( s16 )
      break;
    case FLOAT :
      _X_FILTERING( r32 )
      break;
    case SINT :
      _X_FILTERING( i32 )
      break;
    default :
      VT_Error( "output image type unknown or not supported", proc );
      VT_Free( (void**)&tmp );
      return( -1 );
    }
    input = resIm;
  }

#define _Y_FILTERING( TYPE ) {                                                          \
  TYPE *theBuf = (TYPE*)(input->buf);                                                   \
  TYPE *resBuf = (TYPE*)(resIm->buf);                                                   \
  for ( z = 0; z < dz ; z ++ ) 								\
  for ( x = 0; x < dx ; x ++ ) {							\
    offset = z * dxy + x;								\
    for ( y = 0; y < _half_length ; y ++ ) {						\
      /*--- the first points ---*/							\
      ind = offset + y * dx;						       		\
      s = _local_coeff[0];								\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= y; i ++ ) {							\
	r += ( (double)theBuf[ind + _local_offst[i]] + (double)theBuf[ind - _local_offst[i]] ) * _local_coeff[i]; \
	s += 2.0 * _local_coeff[i];							\
      }											\
      for ( i = y+1; i <= _half_length ; i ++ ) {					\
	r += (double)theBuf[ind + _local_offst[i]] * _local_coeff[i];			\
	s += _local_coeff[i];								\
      }											\
      tmp[y] = r/s;          								\
      /*--- the last points ---*/							\
      j = dy - 1 - y;          								\
      ind = offset + j * dx;               						\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= y; i ++ )							\
	r += ( (double)theBuf[ind + _local_offst[i]] + (double)theBuf[ind - _local_offst[i]] ) * _local_coeff[i]; \
      for ( i = y+1; i <= _half_length ; i ++ )						\
	r += (double)theBuf[ind - _local_offst[i]] * _local_coeff[i];			\
      tmp[j] = r/s;     								\
    }											\
    for ( y = _half_length ; y < dh; y ++ ) {			                	\
      /*--- the middle points ---*/							\
      ind = offset + y * dx;								\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= _half_length; i ++ ) 						\
        r += ( (double)theBuf[ind + _local_offst[i]] + (double)theBuf[ind - _local_offst[i]] ) * _local_coeff[i]; \
      tmp[y] = r;  	        							\
    }											\
    for ( y = 0, ind = offset; y < dy; y ++, ind += dx ) resBuf[ind] = tmp[y];          \
  }                                                                                     \
}

  if ( dy > 2 ) {
    /*--- filtering along y ---*/
    if ( 2 * _half_length + 1 > dy )
      VT_InitCoeff( sigma, dy );
    _local_offst[0] = 0;
    for ( i = 1; i <= _half_length; i ++ )
      _local_offst[i] = i * dx;
    dh = dy - _half_length;

    switch ( theIm->type ) {
    case UCHAR :
      _Y_FILTERING( u8 )
      break;
    case SCHAR :
      _Y_FILTERING( s8 )
      break;
    case USHORT :
      _Y_FILTERING( u16 )
      break;
    case SSHORT :
      _Y_FILTERING( s16 )
      break;
    case FLOAT :
      _Y_FILTERING( r32 )
      break;
    case SINT :
      _Y_FILTERING( i32 )
      break;
    default :
      VT_Error( "output image type unknown or not supported", proc );
      VT_Free( (void**)&tmp );
      return( -1 );
    }
    input = resIm;
  }

#define _Z_FILTERING( TYPE ) {                                                          \
  TYPE *theBuf = (TYPE*)(resIm->buf);                                                   \
  TYPE *resBuf = (TYPE*)(resIm->buf);                                                   \
  for ( y = 0; y < dy ; y ++ ) 								\
  for ( x = 0; x < dx ; x ++ ) {							\
    offset = y * dx + x;								\
    for ( z = 0; z < _half_length ; z ++ ) {						\
      /*--- the first points ---*/							\
      ind = offset + z * dxy;						       		\
      s = _local_coeff[0];								\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= z; i ++ ) {							\
	r += ( (double)theBuf[ind + _local_offst[i]] + (double)theBuf[ind - _local_offst[i]] ) * _local_coeff[i]; \
	s += 2.0 * _local_coeff[i];							\
      }											\
      for ( i = z+1; i <= _half_length ; i ++ ) {					\
	r += (double)theBuf[ind + _local_offst[i]] * _local_coeff[i];			\
	s += _local_coeff[i];								\
      }											\
      tmp[z] = r/s;								        \
      /*--- the last points ---*/							\
      j = dz - 1 - z;                                                                   \
      ind = offset + j * dxy;            						\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= z; i ++ )							\
	r += ( (double)theBuf[ind + _local_offst[i]] + (double)theBuf[ind - _local_offst[i]] ) * _local_coeff[i]; \
      for ( i = z+1; i <= _half_length ; i ++ )						\
	r += (double)theBuf[ind - _local_offst[i]] * _local_coeff[i];			\
      tmp[j] = r/s;          								\
    }											\
    for ( z = _half_length ; z < dh; z ++ ) {			                	\
      /*--- the middle points ---*/							\
      ind = offset + z * dxy;								\
      r = (double)theBuf[ind] * _local_coeff[0];					\
      for ( i = 1; i <= _half_length; i ++ ) 						\
        r += ( (double)theBuf[ind + _local_offst[i]] + (double)theBuf[ind - _local_offst[i]] ) * _local_coeff[i]; \
      tmp[z] = r;  								\
    }											\
    for ( z = 0, ind = offset; z < dz; z ++, ind += dxy ) resBuf[ind] = tmp[z];         \
  }                                                                                     \
}

  if ( dz > 2 ) {
    /*--- filtering along z ---*/
    if ( 2 * _half_length + 1 > dz )
      VT_InitCoeff( sigma, dz );
    _local_offst[0] = 0;
    for ( i = 1; i <= _half_length; i ++ )
      _local_offst[i] = i * dxy;
    dh = dz - _half_length;

    switch ( theIm->type ) {
    case UCHAR :
      _Z_FILTERING( u8 )
      break;
    case SCHAR :
      _Z_FILTERING( s8 )
      break;
    case USHORT :
      _Z_FILTERING( u16 )
      break;
    case SSHORT :
      _Z_FILTERING( s16 )
      break;
    case FLOAT :
      _Z_FILTERING( r32 )
      break;
    case SINT :
      _Z_FILTERING( i32 )
      break;
    default :
      VT_Error( "output image type unknown or not supported", proc );
      VT_Free( (void**)&tmp );
      return( -1 );
    }
  }

  VT_Free( (void**)&tmp );

  if ( (dx <= 2) && (dy <= 2) && (dz <= 2) )
    if ( VT_CopyImage( theIm, resIm ) != 1 ) {
      VT_Error( "error while copying", proc );
      return( -1 );
    }
  
  return( 1 );
}


