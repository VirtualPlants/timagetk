/*************************************************************************
 * chamferdistance-mask.c - computation of chamfer masks
 *
 * $Id$
 *
 * Copyright (c) INRIA 2014
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer  4 jui 2014 11:07:28 CEST
 *
 * ADDITIONS, CHANGES
 *
 * - 
 *
 *
 *
 *
 */

#include <chamfercoefficients.h>

#include <chamferdistance-mask.h>



static int _verbose_ = 1;

void setVerboseInChamferDistanceMask ( int v )
{
  _verbose_ = v;
}

void incrementVerboseInChamferDistanceMask(  )
{
  _verbose_ ++;
}

void decrementVerboseInChamferDistanceMask(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}








int buildChamferMask( double *theVoxelSize, /* must be 3 values */
		      int theMaskDim, /* 2 or 3 */
		      int theMaskSize, /* size of mask: 3, 5, 7, etc. */
		      int maxvalueofmincoefficient, /* maximal value for the minimal coefficient value */
		      enumDistance typePredefinedMask, /* predefined masks */
		      typeChamferMask *theMask )
{
  char *proc = "buildChamferMask";
  enumDistance type = typePredefinedMask;
  
  VectorList vl;
  ConeList cl;
  WeightsList wl;
  
  int anisotropy_y = 0;
  int anisotropy_z = 0;
  double voxel_size[3];
  
  int minvalueofmincoefficient = 1;
  int dims[3], c[3];
  int i, n, x, y, z;
  int *btxt = (int*)NULL;


  /* arguments processing
   */

  switch ( theMaskDim ) {
  case 2 :
  case 3 :
    break;
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: weird mask dimension (%d)\n", proc, theMaskDim );
    return( -1 );
  }

  if ( theMaskDim == 2  ) {
    switch ( type ) {
    default :
      break;
    case _DISTANCE06_ :
      type = _DISTANCE04_;
      break;
    case _DISTANCE10_ :
    case _DISTANCE18_ :
    case _DISTANCE26_ :
      type = _DISTANCE08_;
      break;
    case _CHAMFER3x3x3_ :
      type = _CHAMFER3x3_;
      break;
    case _CHAMFER5x5x5_ :
      type = _CHAMFER5x5_;
      break;
    }
  }

  

  initChamferMask( theMask );

  

  /* predefined distance
   */

  switch ( type ) {
  case _UNDEFINED_DISTANCE_ :
    break;
  default :

    if ( _verbose_ )
      fprintf( stderr, "%s: fills predefined chamfer mask\n", proc );

    if ( buildPredefinedChamferMask( type, theMask ) != 1 ) {
      freeChamferMask( theMask );
      if ( _verbose_ )
	fprintf( stderr, "%s: error when filling predefined chamfer mask\n", proc );
      return( -1 );
    }
    return( 1 );
  }


  if ( _verbose_ >= 2 )
    fprintf( stderr, "%s: computes image-dependent chamfer mask\n", proc );


  /* calculated chamfer mask
   */
  _init_vector_list( &vl );
  _init_cone_list( &cl );

  voxel_size[0] = 1.0;
  voxel_size[1] = theVoxelSize[1] / theVoxelSize[0];
  voxel_size[2] = theVoxelSize[2] / theVoxelSize[0];
  if ( voxel_size[1] < 0.99 || voxel_size[1] > 1.01 )
    anisotropy_y = 1;
  if ( theMaskDim == 3 ) {
    if ( voxel_size[2] < 0.99 || voxel_size[2] > 1.01 )
      anisotropy_z = 1;
  }
  


  switch ( theMaskDim ) {
    
  default :

    if ( _verbose_ )
      fprintf( stderr, "%s: weird mask dimension (%d)\n", proc, theMaskDim );
    return( -1 );

  case 2 :

    /* mask initialization 
     */
    _init_2D_mask( &vl, &cl, voxel_size );
    _build_2D_mask( &vl, &cl, voxel_size, theMaskSize );
    _symmetrise_2D_mask( &vl, &cl, voxel_size, anisotropy_y );
    if ( _verbose_ >= 2 )
      _print_2D_vector_list( stdout, &vl );
    
    /* compute weights
     */
    _init_weights_list( &wl );
    wl.error_optimal = cl.allocatedCones[0].error;
    (void) _compute_2D_weights( stdout, &wl, &vl, &cl, 
				minvalueofmincoefficient, maxvalueofmincoefficient );
    
    break;

  case 3 :
    
    /* mask initialization 
     */
    _init_3D_mask( stdout, &vl, &cl, voxel_size ); 
    _build_3D_mask( stdout, &vl, &cl, voxel_size, theMaskSize, (int)0 );
    _symmetrise_3D_mask( stdout, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );
    if ( _verbose_ >= 2 )
      _print_3D_vector_list( stdout, &vl );
    _check_3D_mask_naturalconvexity( stdout, &vl, &cl, voxel_size );
    _check_3D_mask_fareyregularity( stdout, &vl, &cl );
    
    /* compute weights
     */
    _init_weights_list( &wl );
    wl.error_optimal = cl.allocatedCones[0].error;
    (void) _compute_3D_weights( stdout, &wl, &vl, &cl, voxel_size, 
				minvalueofmincoefficient, maxvalueofmincoefficient );
    
  }
  

  
  /* mask extraction
   */   
  if ( wl.n_weights > 0 ) {
    
    if ( _verbose_ >= 2 )
      _print_weights_list( stdout, &wl, &vl );
    
    for ( i=0; i<vl.n_vectors; i++ )
      vl.allocatedVectors[i].w = wl.allocatedWeights[wl.n_weights - 1].w[i];
    
    btxt = _build_buffer_mask( dims, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );
    
    c[0] = (dims[0]-1)/2;
    c[1] = (dims[1]-1)/2;
    c[2] = (dims[2]-1)/2;
    if ( allocChamferMask( theMask, dims[0] * dims[1] * dims[2] ) <= 0 ) {
      fprintf( stderr, "error in mask allocation\n" );
      _free_cone_list( &cl );
      _free_vector_list( &vl );
      _free_weights_list( &wl );
      return( -1 );
    }
    
    n = 0;
    for ( z=0; z<dims[2]; z++ )
    for ( y=0; y<dims[1]; y++ )
    for ( x=0; x<dims[0]; x++ ) {
      if ( btxt[z*dims[0]*dims[1] + y*dims[0] + x] > 0 ) {
	theMask->list[n].x = x - c[0];
	theMask->list[n].y = y - c[1];
	theMask->list[n].z = z - c[2];
	theMask->list[n].inc = btxt[z*dims[0]*dims[1] + y*dims[0] + x];
	n++;
      }
    }
    theMask->nb = n;
    theMask->normalizationfactor = wl.allocatedWeights[wl.n_weights - 1].epsilon;
    

    if ( _verbose_ >= 2 )
      (void)printChamferMaskAsImage( stdout, theMask );
    
    if ( btxt != NULL ) free( btxt );
  }
  
  _free_cone_list( &cl );
  _free_vector_list( &vl );
  _free_weights_list( &wl );

  return( 1 );
  
}
