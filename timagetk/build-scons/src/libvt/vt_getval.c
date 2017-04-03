
#include <vt_getval.h>

/* Calcul de la valeur entiere a partir d'une valeur double.

*/

#define COMPUTE_INT_FROM_DOUBLE( I, D ) { \
        if ( (D) < 0.0 ) (I) = ((i32)( (D) - 0.5 )); \
        else             (I) = ((i32)( (D) + 0.5 )); }

/* Calcul de la valeur entiere a partir d'une valeur double.

   Si la valeur est inferieure au MIN ou superieure ou MAX,
   on lui affecte MIN ou MAX respectivement.
*/

#define COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( I, D, MIN, MAX ) { \
        if      ( (D) < (MIN) ) (I) = (MIN); \
        else if ( (D) > (MAX) ) (I) = (MAX); \
        else if ( (D) < 0.0 )   (I) = ((i32)( (D) - 0.5 )); \
        else                    (I) = ((i32)( (D) + 0.5 )); }



/*---
  Procedures de lecture de la valeur d'un point.
---*/

/* Getting an image value, knowing x, y and z.

   Given x, y, z coordinates (the v coordinate is
   assumed to be 0), compute the image value.

   The coordinates are tested. If not in the bounds,
   the return value is 0.0.
*/
   
double VT_GetXYZvalue( vt_image *image, int x, int y, int z )
{
    if ( (x < 0) || (x >= (int)image->dim.x) ) return( (double)0.0 );
    if ( (y < 0) || (y >= (int)image->dim.y) ) return( (double)0.0 );
    if ( (z < 0) || (z >= (int)image->dim.z) ) return( (double)0.0 );
    return( _VT_GetVXYZvalue( image, (int)0, x, y, z ) );
}





/* Getting an image value, knowing v, x, y and z.

   Given v, x, y, z coordinates,
   compute the image value.

   The coordinates are tested. If not in the bounds,
   the return value is 0.0.
*/
   
double VT_GetVXYZvalue( vt_image *image, int v, int x, int y, int z )
{
    if ( (v < 0) || (v >= (int)image->dim.v) ) return( (double)0.0 );
    if ( (x < 0) || (x >= (int)image->dim.x) ) return( (double)0.0 );
    if ( (y < 0) || (y >= (int)image->dim.y) ) return( (double)0.0 );
    if ( (z < 0) || (z >= (int)image->dim.z) ) return( (double)0.0 );
    return( _VT_GetVXYZvalue( image, v, x, y, z ) );
}





/* Getting an image value, knowing x, y and z.

   Given x, y, z coordinates (the v coordinate is
   assumed to be 0), compute the image value.

   The coordinates are NOT tested.
*/
   
double _VT_GetXYZvalue( vt_image *image, int x, int y, int z )
{
    return( _VT_GetVXYZvalue( image, (int)0, x, y, z ) );
}





/* Getting an image value, knowing v, x, y and z.

   Given v, x, y, z coordinates,
   compute the image value.

   The coordinates are NOT tested.
*/
   
double _VT_GetVXYZvalue( vt_image *image, int v, int x, int y, int z )
{
    switch ( image->type ) {
    case SCHAR :
        {
	s8 ***buf;
	buf = (s8***)(image->array);
	return( (double)(buf[z][y][x * image->dim.v + v]) );
	}
    case UCHAR :
        {
	u8 ***buf;
	buf = (u8***)(image->array);
	return( (double)(buf[z][y][x * image->dim.v + v]) );
	}
    case SSHORT :
        {
	s16 ***buf;
	buf = (s16***)(image->array);
	return( (double)(buf[z][y][x * image->dim.v + v]) );
	}
    case USHORT :
        {
	u16 ***buf;
	buf = (u16***)(image->array);
	return( (double)(buf[z][y][x * image->dim.v + v]) );
	}
    case FLOAT :
        {
	r32 ***buf;
	buf = (r32***)(image->array);
	return( (double)(buf[z][y][x * image->dim.v + v]) );
	}
    case SINT :
        {
	i32 ***buf;
	buf = (i32***)(image->array);
	return( (double)(buf[z][y][x * image->dim.v + v]) );
	}
    default :
	VT_Error( "unknown image type", "_VT_GetVXYZvalue" );
	return( (double)0.0 );
    }
    return( (double)0.0 );
}





/* Getting an image value, knowing v, x, y and z.

   Given v, x, y, z coordinates,
   compute the image value.

   The coordinates are NOT tested.
*/
   
double _VT_GetVDXYZvalue( vt_image *image, int v, double x, double y, double z )
{
  char *proc = "_VT_GetVDXYZvalue";
  int ix, iy, iz;
  int dimv = image->dim.v;
  int dimx = image->dim.x;
  int dimy = image->dim.y;
  int dimz = image->dim.z;
  double res = 0.0;
  double dx, dy, dz;
  
  ix = (int)x;
  iy = (int)y;
  iz = (int)z;
  dx = x - ix;
  dy = y - iy;
  dz = z - iz;

  if ( ix < 0 ) {
    ix = 0;
    dx = 0; 
  }
  if ( iy < 0 ) { 
    iy = 0;
    dy = 0;
  }
  if ( iz < 0 ) {
    iz = 0;
    dz = 0;
  }
  if ( ix >= dimx - 1 ) {
    ix = dimx-2;
    dx = 1;
  }
  if ( iy >= dimy - 1 ) {
    iy = dimy-2;
    dy = 1;
  }
  if ( iz >= dimz - 1 ) {
    iz = dimz-2;
    dz = 1;
  }

  switch( image->type ) {

  case FLOAT :
    {
      float ***buf = (float***)image->array;
      res = (1-dx) * ( (1-dy)* ( (1-dz)*buf[iz][iy][dimv*ix+v] +
				 (dz)*buf[iz+1][iy][dimv*ix+v] ) +
		       (dy)* ( (1-dz)*buf[iz][iy+1][dimv*ix+v] +
			       (dz)*buf[iz+1][iy+1][dimv*ix+v] ) ) +
	    (dx) * ( (1-dy)* ( (1-dz)*buf[iz][iy][dimv*ix+dimv+v] +
			       (dz)*buf[iz+1][iy][dimv*ix+dimv+v] ) +
		     (dy)* ( (1-dz)*buf[iz][iy+1][dimv*ix+dimv+v] +
			     (dz)*buf[iz+1][iy+1][dimv*ix+dimv+v] ) );
      
    }
    break;

  default :
    VT_Error( "such image type not handled in switch", proc );
    return( (double)0.0 );
  }

  return( res );

}





/* Getting an image value, knowing the point index.

   Given the point index i, compute the image value.

   The index is tested. If not in the bounds,
   the return value is 0.0.
*/

double VT_GetINDvalue( vt_image *image, int i )
{
  if ( (i < 0) || ((size_t)i >= (image->dim.x * image->dim.y * image->dim.z)) )
    return( (double)0.0 );
  return( _VT_GetINDvalue( image, i ) );
}





/* Getting an image value, knowing the point index.

   Given the point index i, compute the image value.

   The index is NOT tested.
*/

double _VT_GetINDvalue( vt_image *image, int i )
{
    switch ( image->type ) {
    case SCHAR :
        {
	s8 *buf;
	buf = (s8*)(image->buf);
	return( (double)(buf[i]) );
	}
    case UCHAR :
        {
	u8 *buf;
	buf = (u8*)(image->buf);
	return( (double)(buf[i]) );
	}
    case SSHORT :
        {
	s16 *buf;
	buf = (s16*)(image->buf);
	return( (double)(buf[i]) );
	}
    case USHORT :
        {
	u16 *buf;
	buf = (u16*)(image->buf);
	return( (double)(buf[i]) );
	}
    case FLOAT :
        {
	r32 *buf;
	buf = (r32*)(image->buf);
	return( (double)(buf[i]) );
	}
    case SINT :
        {
	i32 *buf;
	buf = (i32*)(image->buf);
	return( (double)(buf[i]) );
	}
    default :
	VT_Error( "unknown image type", "_VT_GetINDvalue" );
	return( (double)0.0 );
    }
    return( (double)0.0 );
}





/*---
  Procedures d'ecriture de la valeur d'un point.
---*/

/* Setting an image value, knowing x, y and z.

   Given x, y, z coordinates (the v coordinate is
   assumed to be 0), set the image value.

   The coordinates are tested. If not in the bounds,
   the image is not modified.
*/
   
void VT_SetXYZvalue( vt_image *image, int x, int y, int z, double val )
{
    if ( (x < 0) || (x >= (int)image->dim.x) ) return;
    if ( (y < 0) || (y >= (int)image->dim.y) ) return;
    if ( (z < 0) || (z >= (int)image->dim.z) ) return;
    
    _VT_SetVXYZvalue( image, (int)0, x, y, z, val );
}





/* Setting an image value, knowing v, x, y and z.

   Given v, x, y, z coordinates,
   set the image value.

   The coordinates are tested. If not in the bounds,
   the image is not modified.
*/
   
void VT_SetVXYZvalue( vt_image *image, int v, int x, int y, int z, double val )
{
    if ( (v < 0) || (v >= (int)image->dim.v) ) return;
    if ( (x < 0) || (x >= (int)image->dim.x) ) return;
    if ( (y < 0) || (y >= (int)image->dim.y) ) return;
    if ( (z < 0) || (z >= (int)image->dim.z) ) return;
 
    _VT_SetVXYZvalue( image, v, x, y, z, val );
}





/* Setting an image value, knowing x, y and z.

   Given x, y, z coordinates (the v coordinate is
   assumed to be 0), set the image value.

   The coordinates are NOT tested.
*/
   
void _VT_SetXYZvalue( vt_image *image, int x, int y, int z, double val )
{
    _VT_SetVXYZvalue( image, (int)0, x, y, z, val );
}





/* Setting an image value, knowing v, x, y and z.

   Given v, x, y, z coordinates,
   set the image value.

   The coordinates are NOT tested.
*/

void _VT_SetVXYZvalue( vt_image *image, int v, int x, int y, int z, double val )
{
    i32 iv;
    
    switch ( image->type ) {
    case SCHAR :
        {
	s8 ***buf;
	buf = (s8***)(image->array);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, -128, 127 )
	buf[z][y][x * image->dim.v + v] = (s8)iv;
	return;
	}
    case UCHAR :
        {
	u8 ***buf;
	buf = (u8***)(image->array);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, 0, 255 )
	buf[z][y][x * image->dim.v + v] = (u8)iv;
	return;
	}
    case SSHORT :
        {
	s16 ***buf;
	buf = (s16***)(image->array);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, -32768, 32767 )
	buf[z][y][x * image->dim.v + v] = (s16)iv;
	return;
	}
    case USHORT :
        {
	u16 ***buf;
	buf = (u16***)(image->array);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, 0, 65535 )
	buf[z][y][x * image->dim.v + v] = (u16)iv;
	return;
	}
    case FLOAT :
        {
	r32 ***buf;
	buf = (r32***)(image->array);
	buf[z][y][x * image->dim.v + v] = (r32)val;
	return;
	}
    case SINT :
        {
	i32 ***buf;
	buf = (i32***)(image->array);
	COMPUTE_INT_FROM_DOUBLE( iv, val )
	buf[z][y][x * image->dim.v + v] = (i32)iv;
	return;
	}
    default :
	VT_Error( "unknown image type", "_VT_SetVXYZvalue" );
	return;
    }
    return;
}





/* Setting an image value, knowing the point index.

   Given the point index i, set the image value.

   The index is tested. If not in the bounds,
   the image is not modified.
*/

void VT_SetINDvalue( vt_image *image, int i, double val )
{
    if ( (i < 0) || ((size_t)i >= (image->dim.x * image->dim.y * image->dim.z)) )
      return;
    _VT_SetINDvalue( image, i, val );
}





/* Setting an image value, knowing the point index.

   Given the point index i, set the image value.

   The index is NOT tested.
*/

void _VT_SetINDvalue( vt_image *image, int i, double val )
{
    i32 iv = 0;

    switch ( image->type ) {
    case SCHAR :
        {
	s8 *buf;
	buf = (s8*)(image->buf);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, -128, 127 )
	buf[i] = (s8)iv;
	return;
	}
    case UCHAR :
        {
	u8 *buf;
	buf = (u8*)(image->buf);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, 0, 255 )
	buf[i] = (u8)iv;
	return;
	}
    case SSHORT :
        {
	s16 *buf;
	buf = (s16*)(image->buf);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, -32768, 32767 )
	buf[i] = (s16)iv;
	return;
	}
    case USHORT :
        {
	u16 *buf;
	buf = (u16*)(image->buf);
	COMPUTE_INT_FROM_DOUBLE_WITH_MIN_AND_MAX( iv, val, 0, 65535 )
	buf[i] = (u16)iv;
	return;
	}
    case FLOAT :
        {
	r32 *buf;
	buf = (r32*)(image->buf);
	buf[i] = (r32)val;
	return;
	}
    case SINT :
        {
	i32 *buf;
	buf = (i32*)(image->buf);
	COMPUTE_INT_FROM_DOUBLE( iv, val )
	buf[i] = (i32)iv;
	return;
	}
    default :
	VT_Error( "unknown image type", "_VT_SetINDvalue" );
	return;
    }
    return;
}
