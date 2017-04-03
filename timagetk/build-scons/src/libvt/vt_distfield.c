#include <vt_distfield.h>

static int _VERBOSE_ = 1;

distanceField *VT_CreateDistanceField( int dimx,
				       int dimy,
				       int dimz,
				       bufferType type )
{
  char *proc="VT_CreateDistanceField";
  distanceField *dfield = (distanceField *)NULL;
  int size = 0;
  
  if ( (dimx < 1) || (dimy < 1) || (dimz < 1) ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: bad dimensions.\n", proc );
    }
    return( (distanceField *)NULL );
  }

  dfield = (distanceField *)malloc( sizeof(distanceField) );
  if ( dfield == (distanceField *)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to allocate structure.\n", proc );
    }
    return( (distanceField *)NULL );
  }

  /*
   * initialisation.
   */
  dfield->x = (void*)NULL;
  dfield->y = (void*)NULL;
  dfield->z = (void*)NULL;
  dfield->type = type;
  dfield->dimx = dimx;
  dfield->dimy = dimy;
  dfield->dimz = dimz;
  dfield->dimxy = dimx * dimy;
  
  /* 
   * allocation
   */
  switch( type ) {
  case SCHAR :
    size = dimx*dimy*dimz*sizeof( s8 );
    break;
  case SSHORT :
    size = dimx*dimy*dimz*sizeof( s16 );
    break;
  case FLOAT :
    size = dimx*dimy*dimz*sizeof( r32 );
    break;
  default :
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to deal with such type.\n", proc );
    }
    free( (void*)dfield );
    return( (distanceField *)NULL );
  }
  dfield->x = (void*)malloc( size );
  if ( dfield->x == (void*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to allocate first coordinate's buffer.\n", proc );
    }
    free( (void*)dfield );
    return( (distanceField *)NULL );
  }  
  dfield->y = (void*)malloc( size );
  if ( dfield->y == (void*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to allocate second coordinate's buffer.\n", proc );
    }
    free( (void*)(dfield->x) );
    free( (void*)dfield );
    return( (distanceField *)NULL );
  }
  dfield->z = (void*)malloc( size );
  if ( dfield->z == (void*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to allocate third coordinate's buffer.\n", proc );
    }
    free( (void*)(dfield->y) );
    free( (void*)(dfield->x) );
    free( (void*)dfield );
    return( (distanceField *)NULL );
  }

  return( dfield );
}

void VT_FreeDistanceField( distanceField *dfield )
{
  if ( dfield == (distanceField *)NULL ) return;
  if ( dfield->x != (void*)NULL ) free( (void*)(dfield->x) );
  if ( dfield->y != (void*)NULL ) free( (void*)(dfield->y) );
  if ( dfield->z != (void*)NULL ) free( (void*)(dfield->z) );
  free( (void*)dfield );
  return;
}

distanceField *VT_FillDistanceField( char* imageX,
				     char* imageY,
				     char* imageZ )
{
  char *proc="VT_FillDistanceField";
  vt_image *tmpImage = (vt_image *)NULL;
  distanceField *dfield = (distanceField *)NULL;
  int size=0;
  
  /*
   * 1ere image 
   */
  tmpImage =  _VT_Inrimage( imageX );
  if ( tmpImage == (vt_image*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to read %s.\n", proc, imageX );
    }
    return( (distanceField *)NULL );
  }
  /*
   * creation du champ de distance 
   */
  dfield = VT_CreateDistanceField( tmpImage->dim.x, tmpImage->dim.y,
				   tmpImage->dim.z, tmpImage->type );
  if ( dfield == (distanceField *)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: error when creating distance field.\n", proc );
    }
    VT_FreeImage( tmpImage );
    VT_Free( (void**)&tmpImage );
    return( (distanceField *)NULL );
  }
  /*
   * Taille des buffers
   */
  switch( tmpImage->type ) {
  case SCHAR :
    size = dfield->dimx * dfield->dimy * dfield->dimz * sizeof( s8 );
    break;
  case SSHORT :
    size = dfield->dimx * dfield->dimy * dfield->dimz * sizeof( s16 );
    break;
  case FLOAT :
    size = dfield->dimx * dfield->dimy * dfield->dimz * sizeof( r32 );
    break;
  default :
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to deal with such type.\n", proc );
    }
    VT_FreeImage( tmpImage );
    VT_Free( (void**)&tmpImage );
    VT_FreeDistanceField( dfield );
    return( (distanceField *)NULL );
  }
  /*
   * copie du buffer
   */
  memcpy( (void*)dfield->x, (void*)tmpImage->buf, size );
  VT_FreeImage( tmpImage );
  VT_Free( (void**)&tmpImage );
  if ( _VERBOSE_ != 0 ) {
    fprintf( stderr, " %s: lecture de %s achevee.\n", proc, imageX );
  }

  /*
   * deuxieme image
   */ 
  tmpImage =  _VT_Inrimage( imageY );
  if ( tmpImage == (vt_image*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to read %s.\n", proc, imageY );
    }
    VT_FreeDistanceField( dfield );
    return( (distanceField *)NULL );
  }
  if ( ((int)tmpImage->dim.x != dfield->dimx) ||
       ((int)tmpImage->dim.y != dfield->dimy) ||
       ((int)tmpImage->dim.z != dfield->dimz) ||
       (tmpImage->type != dfield->type) ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: incompatible dimensions or type for second image %s.\n", proc, imageY );
    }
    VT_FreeImage( tmpImage );
    VT_Free( (void**)&tmpImage );
    VT_FreeDistanceField( dfield );
    return( (distanceField *)NULL );
  }
  /*
   * copie du buffer
   */
  memcpy( (void*)dfield->y, (void*)tmpImage->buf, size );
  VT_FreeImage( tmpImage );
  VT_Free( (void**)&tmpImage );
  if ( _VERBOSE_ != 0 ) {
    fprintf( stderr, " %s: lecture de %s achevee.\n", proc, imageY );
  }
  
  
  /*
   * troisieme image
   */ 
  tmpImage =  _VT_Inrimage( imageZ );
  if ( tmpImage == (vt_image*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: unable to read %s.\n", proc, imageZ );
    }
    VT_FreeDistanceField( dfield );
    return( (distanceField *)NULL );
  }
  if ( ((int)tmpImage->dim.x != dfield->dimx) ||
       ((int)tmpImage->dim.y != dfield->dimy) ||
       ((int)tmpImage->dim.z != dfield->dimz) ||
       (tmpImage->type != dfield->type) ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: incompatible dimensions or type for second image %s.\n", proc, imageZ );
    }
    VT_FreeImage( tmpImage );
    VT_Free( (void**)&tmpImage );
    VT_FreeDistanceField( dfield );
    return( (distanceField *)NULL );
  }
  /*
   * copie du buffer
   */
  memcpy( (void*)dfield->z, (void*)tmpImage->buf, size );
  VT_FreeImage( tmpImage );
  VT_Free( (void**)&tmpImage );
  if ( _VERBOSE_ != 0 ) {
    fprintf( stderr, " %s: lecture de %s achevee.\n", proc, imageZ );
  }

  return( dfield );
}

void getDistanceFieldValues( float *v, 
			     int x, 
			     int y, 
			     int z,
			     distanceField *dfield )
{
  int i, j, k;
  
  i = x;
  if ( i < 0 ) i = 0;
  if ( i >= dfield->dimx ) i = dfield->dimx - 1;
  j = y;
  if ( j < 0 ) j = 0;
  if ( j >= dfield->dimy ) k = dfield->dimy - 1;
  k = z;
  if ( k < 0 ) j = 0;
  if ( k >= dfield->dimz ) k = dfield->dimz - 1;

  switch( dfield->type ) {
  case SCHAR :
    v[0] = (float)( ((s8*)dfield->x)[i + j*dfield->dimx + k*dfield->dimxy ] );
    v[1] = (float)( ((s8*)dfield->y)[i + j*dfield->dimx + k*dfield->dimxy ] );
    v[2] = (float)( ((s8*)dfield->z)[i + j*dfield->dimx + k*dfield->dimxy ] );
    break;
  case SSHORT :
    v[0] = (float)( ((s16*)dfield->x)[i + j*dfield->dimx + k*dfield->dimxy ] );
    v[1] = (float)( ((s16*)dfield->y)[i + j*dfield->dimx + k*dfield->dimxy ] );
    v[2] = (float)( ((s16*)dfield->z)[i + j*dfield->dimx + k*dfield->dimxy ] );
    break;
  case FLOAT :
    v[0] = (float)( ((r32*)dfield->x)[i + j*dfield->dimx + k*dfield->dimxy ] );
    v[1] = (float)( ((r32*)dfield->y)[i + j*dfield->dimx + k*dfield->dimxy ] );
    v[2] = (float)( ((r32*)dfield->z)[i + j*dfield->dimx + k*dfield->dimxy ] );
    break;
  default :
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " unkown distance field type.\n" );
    }
  }
}
