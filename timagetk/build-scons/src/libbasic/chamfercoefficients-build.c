/*************************************************************************
 * chamfercoefficients-build.c - computation of chamfer distance coefficients
 *
 * $Id$
 *
 * Copyright (c) INRIA 2016
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Mer 16 mar 2016 16:21:18 CET
 *
 * COMMENTS
 *
 * This file is aimed at being included into an other file, not to
 * be compiled by itself.
 *
 * ADDITIONS, CHANGES
 *
 * -
 *
 *
 *
 *
 */

/**************************************************
 *
 * 2D
 *
 **************************************************/



void _init_2D_mask( VectorList *vl, ConeList *cl, double *s ) 
{
  Vector v;
  int iv1, iv2;
  Cone c;

  _build_vector( &v, 1, 0, 0, s );
  iv1 = _add_vector_to_list( vl, &v );

  _build_vector( &v, 1, 1, 0, s );
  iv2 = _add_vector_to_list( vl, &v );

  _build_2D_cone( vl, &c, iv1, iv2 );
  (void)_add_cone_to_list( cl, &c );
}



void _symmetrise_2D_mask_wrt_vector( VectorList *vl, ConeList *cl, double *s,
				     Vector *normal, int first, int last )
{
  char *proc = "_symmetrise_2D_mask_wrt_vector";
  int i, nv;
  Vector v;
  int iv1, iv2, ic;
  Cone *c, sym;

  for ( i=first; i<=last;  i++ ) {

    nv = vl->n_vectors;

    c = &(cl->allocatedCones[ i ]);

    if ( _trace_ >= 4 ) {
      fprintf( stdout, "%s: cone to be symmetrized = ", proc );
      _print_2D_cone( stdout, vl, c );
      fprintf( stdout, "\n" );
    }

    /* vector #1 */
    _build_3D_symmetric_vector_wrt_vector( &v, &(vl->allocatedVectors[c->iv1]), normal, s );
    iv1 = _add_vector_to_list( vl, &v );

    /* vector #2 */
    _build_3D_symmetric_vector_wrt_vector( &v, &(vl->allocatedVectors[c->iv2]), normal, s );
    iv2 = _add_vector_to_list( vl, &v );

    /* change vector order to ensure
       positivity of the vector product
    */
    _build_2D_cone( vl, &sym, iv2, iv1 );
    ic = _add_cone_to_list( cl, &sym );
    
    if ( _trace_ >= 4 ) {
      fprintf( stdout, "         added vector #1 (index = %2d) = ", iv1 );
      _print_3D_vector( stdout, &(vl->allocatedVectors[iv1]) );
      fprintf( stdout, "\n" );
      fprintf( stdout, "         added vector #2 (index = %2d) = ", iv2 );
      _print_3D_vector( stdout, &(vl->allocatedVectors[iv2]) );
      fprintf( stdout, "\n" );
      fprintf( stdout, "         built cone = " );
      _print_2D_cone( stdout, vl, &sym );
      fprintf( stdout, "\n" );
      fprintf( stdout, "         added cone = " );
      _print_2D_cone( stdout, vl, &(cl->allocatedCones[ic]) );
      fprintf( stdout, "\n" );
    }
    
  }

}



void _symmetrise_2D_mask( VectorList *vl, ConeList *cl, double *voxel_size, int anisotropy_y )
{
  Vector normal;
  
  if ( anisotropy_y ) {
    /* add cones wrt plane (1,1,0) (0,0,1)
	 the vectorial product of the two above vectors 
	 is (1,-1,0)
     */
    normal.x =  1;
    normal.y = -1;
    normal.z =  0;
    _symmetrise_2D_mask_wrt_vector(vl, cl, voxel_size, 
				   &normal, 0, cl->n_cones-1 );
  }
  
  _sort_cones( cl->allocatedCones, 0, cl->n_cones-1 );

}



void _complete_2D_mask_in_quadrant( VectorList *vl, ConeList *cl, double *voxel_size, int anisotropy_y )
{
  Vector normal;
  
  if ( !anisotropy_y ) {
    /* add cones wrt plane (1,1,0) (0,0,1)
	 the vectorial product of the two above vectors 
	 is (1,-1,0)
     */
    normal.x =  1;
    normal.y = -1;
    normal.z =  0;
    _symmetrise_2D_mask_wrt_vector( vl, cl, voxel_size, 
				   &normal, 0, cl->n_cones-1 );

    _sort_cones( cl->allocatedCones, 0, cl->n_cones-1 );
  }
}



void _build_2D_mask( VectorList *vl, ConeList *cl, double *s, int size ) 
{
  Vector v;
  int d = (size-1)/2;
  int ic, iv, i, k;
  int nx, ny;
  Cone *c, newc;

  /* loop on depth
   */
  for ( k=2; k<=d; k++ ) {

    /* loop on cones
     */
    for ( i=0; i<cl->n_cones; i++ ) {

      /* do not split
       */
      c = &(cl->allocatedCones[i]);
      nx = vl->allocatedVectors[c->iv1].x + vl->allocatedVectors[c->iv2].x;
      ny = vl->allocatedVectors[c->iv1].y + vl->allocatedVectors[c->iv2].y;
      if ( nx > k || ny > k ) continue;

      /* create new vector
       */
      _build_vector( &v, nx, ny, 0, s );
      iv = _add_vector_to_list( vl, &v );
      
      /* create new cone
       */
      _build_2D_cone( vl, &newc, iv, c->iv2 );
      ic =_add_cone_to_list( cl, &newc );
      _move_cone_backward( cl, ic );
      
      /* change the considered cone
	 the previous one should be further in the list (it has a smaller error),
	 so there won't any interaction
      */
      cl->allocatedCones[i].iv2 = iv;
      _compute_2D_cone_error( &(cl->allocatedCones[i]),
			      &(vl->allocatedVectors[cl->allocatedCones[i].iv1]),
			      &(vl->allocatedVectors[cl->allocatedCones[i].iv2]) );
      _move_cone_forward( cl, i );
      
      /* consider it again
       */
      i--;
    }

  }

}



void _build_2D_mask_wrt_error( FILE *fout, 
			       VectorList *vl, ConeList *cl, 
			       double *s, int size, int n_vectors, 
			       double error ) 
{
  char *proc = "_build_2D_mask_wrt_error";
  Vector v;
  int d = (size-1)/2;
  int ic, iv, i=0;
  int nx, ny;
  Cone *c, newc;

  /* loop on cones
   */
  while ( i<cl->n_cones && vl->n_vectors< n_vectors 
	  && cl->allocatedCones[i].error > error ) {

    /* do not split
     */
    c = &(cl->allocatedCones[i]);
    nx = vl->allocatedVectors[c->iv1].x + vl->allocatedVectors[c->iv2].x;
    ny = vl->allocatedVectors[c->iv1].y + vl->allocatedVectors[c->iv2].y;
    if ( nx > d || ny > d ) {
      i++;
      continue;
    }
    
    /* create new vector
     */
    _build_vector( &v, nx, ny, 0, s );
    iv = _add_vector_to_list( vl, &v );
      
    /* create new cone
     */
    _build_2D_cone( vl, &newc, iv, c->iv2 );
    ic =_add_cone_to_list( cl, &newc );
    _move_cone_backward( cl, ic );
      
    /* change the considered cone
     */
    cl->allocatedCones[i].iv2 = iv;
    _compute_2D_cone_error( &(cl->allocatedCones[i]),
			    &(vl->allocatedVectors[cl->allocatedCones[i].iv1]),
			    &(vl->allocatedVectors[cl->allocatedCones[i].iv2]) );
    _move_cone_forward( cl, i );
  
    if ( _trace_ >= 2 ) {
      fprintf( fout, "%s: add vector (%2d %2d)", proc, nx, ny );
      fprintf( fout, " => maximal error becomes %f\n", 
	       cl->allocatedCones[0].error );
    }
    
  }

}










/**************************************************
 *
 * 3D
 *
 **************************************************/



void _init_3D_mask( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  Vector v;
  int iv1, iv2, iv3;
  Cone c;

  _build_vector( &v, 1, 0, 0, s );
  iv1 = _add_vector_to_list( vl, &v );

  _build_vector( &v, 1, 1, 0, s );
  iv2 = _add_vector_to_list( vl, &v );

  _build_vector( &v, 1, 1, 1, s );
  iv3 = _add_vector_to_list( vl, &v );

  _build_3D_cone( vl, &c, iv1, iv2, iv3 );
  (void)_add_cone_to_list( cl, &c );

  _check_3D_mask_naturalconvexity( fout, vl, cl, s );
}



void _symmetrise_3D_mask_wrt_vector( VectorList *vl, ConeList *cl, double *s,
				     Vector *normal, int first, int last )
{
  char *proc = "_symmetrise_3D_mask_wrt_vector";
  int i, nv;
  Vector v;
  int iv1, iv2, iv3, ic;
  Cone *c, sym;

  for ( i=first; i<=last;  i++ ) {

    nv = vl->n_vectors;

    c = &(cl->allocatedCones[ i ]);

    if ( _trace_ >= 4 ) {
      fprintf( stdout, "%s: cone to be symmetrized = ", proc );
      _print_3D_cone( stdout, vl, c );
      fprintf( stdout, "\n" );
    }

    /* vector #1 */
    _build_3D_symmetric_vector_wrt_vector( &v, &(vl->allocatedVectors[c->iv1]), normal, s );
    iv1 = _add_vector_to_list( vl, &v );

    /* vector #2 */
    _build_3D_symmetric_vector_wrt_vector( &v, &(vl->allocatedVectors[c->iv2]), normal, s );
    iv2 = _add_vector_to_list( vl, &v );

    /* vector #3 */
    _build_3D_symmetric_vector_wrt_vector( &v, &(vl->allocatedVectors[c->iv3]), normal, s );
    iv3 = _add_vector_to_list( vl, &v );


    /* change vector order to ensure
       positivity of the vector product
    */
    _build_3D_cone( vl, &sym, iv2, iv1, iv3 );
    ic = _add_cone_to_list( cl, &sym );
    
    if ( _trace_ >= 4 ) {
      fprintf( stdout, "         added vector #1 (index = %2d) = ", iv1 );
      _print_3D_vector( stdout, &(vl->allocatedVectors[iv1]) );
      fprintf( stdout, "\n" );
      fprintf( stdout, "         added vector #2 (index = %2d) = ", iv2 );
      _print_3D_vector( stdout, &(vl->allocatedVectors[iv2]) );
      fprintf( stdout, "\n" );
      fprintf( stdout, "         added vector #3 (index = %2d) = ", iv3 );
      _print_3D_vector( stdout, &(vl->allocatedVectors[iv3]) );
      fprintf( stdout, "\n" );
      fprintf( stdout, "         built cone = " );
      _print_3D_cone( stdout, vl, &sym );
      fprintf( stdout, "\n" );
      fprintf( stdout, "         added cone = " );
      _print_3D_cone( stdout, vl, &(cl->allocatedCones[ic]) );
      fprintf( stdout, "\n" );
    }
    
  }

}



void _symmetrise_3D_mask( FILE *fout, VectorList *vl, ConeList *cl, 
			  double *voxel_size,
			  int anisotropy_y, int anisotropy_z )
{
  int nc1, nc0 = cl->n_cones;
  Vector normal;

  if ( anisotropy_z ) {
    /* add cones wrt plane (1,0,0) (0,1,1)
       the vectorial product of the two above vectors 
       is (0,-1,1)
    */
    normal.x =  0;
    normal.y = -1;
    normal.z =  1;
    _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				    &normal, 0, nc0-1 );
    nc1 = cl->n_cones;
    
    /* add cones wrt plane (0,1,0) (1,0,1)
       the vectorial product of the two above vectors 
       is (1,0,-1)
    */
    normal.x =  1;
    normal.y =  0;
    normal.z = -1;
    _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				    &normal, nc0, nc1-1 );
    if ( anisotropy_y ) {
      /* add cones wrt plane (1,1,0) (0,0,1)
	 the vectorial product of the two above vectors 
	 is (1,-1,0)
      */
      normal.x =  1;
      normal.y = -1;
      normal.z =  0;
      _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				      &normal, 0, cl->n_cones-1 );
    }
  }
  else {
    if ( anisotropy_y ) {
      /* add cones wrt plane (1,0,0) (0,1,1)
       the vectorial product of the two above vectors 
       is (0,-1,1)
      */
      normal.x =  0;
      normal.y = -1;
      normal.z =  1;
      _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				      &normal, 0, nc0-1 );

      /* add cones wrt plane (1,1,0) (0,0,1)
	 the vectorial product of the two above vectors 
	 is (1,-1,0)
      */
      normal.x =  1;
      normal.y = -1;
      normal.z =  0;
      _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				      &normal, 0, nc0-1 );
    }
  }

  _check_3D_mask_naturalconvexity( fout, vl, cl, voxel_size );

  _sort_cones( cl->allocatedCones, 0, cl->n_cones-1 );

}



void _complete_3D_mask_in_octant( VectorList *vl, ConeList *cl, 
			  double *voxel_size,
			  int anisotropy_y, int anisotropy_z )
{
  int nc1, nc0 = cl->n_cones;
  Vector normal;

  if ( !anisotropy_y && !anisotropy_z ) {
    /* add cones wrt plane (1,0,0) (0,1,1)
       the vectorial product of the two above vectors 
       is (0,-1,1)
    */
    normal.x =  0;
    normal.y = -1;
    normal.z =  1;

    nc0 = cl->n_cones;
    _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				    &normal, 0, nc0-1 );
    
    /* add cones wrt plane (0,1,0) (1,0,1)
       the vectorial product of the two above vectors 
       is (1,0,-1)
    */
    normal.x =  1;
    normal.y =  0;
    normal.z = -1;

    nc1 = cl->n_cones;
    _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				    &normal, nc0, nc1-1 );
    /* add cones wrt plane (1,1,0) (0,0,1)
       the vectorial product of the two above vectors 
       is (1,-1,0)
    */
    normal.x =  1;
    normal.y = -1;
    normal.z =  0;
    _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				    &normal, 0, cl->n_cones-1 );
  }

  if ( !anisotropy_y && anisotropy_z ) {
    /* add cones wrt plane (1,1,0) (0,0,1)
       the vectorial product of the two above vectors 
       is (1,-1,0)
    */
    normal.x =  1;
    normal.y = -1;
    normal.z =  0;
    _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				    &normal, 0, cl->n_cones-1 );
  }

  if ( anisotropy_y && !anisotropy_z ) {
    /* add cones wrt plane (0,1,0) (1,0,1)
       the vectorial product of the two above vectors 
       is (1,0,-1)
    */
    normal.x =  1;
    normal.y =  0;
    normal.z = -1;
    _symmetrise_3D_mask_wrt_vector( vl, cl, voxel_size, 
				    &normal, 0, cl->n_cones-1 );
  }
  
  if ( !(anisotropy_y && anisotropy_z) ) {
    _sort_cones( cl->allocatedCones, 0, cl->n_cones-1 );
  }

}



void _split_3D_cone_along_JK( VectorList *vl, ConeList *cl, double *s, int i )
{
  Cone *c, newc;
  Vector v;
  int iv, ic;
  
  /* get or create new vector 
   */
  c = &(cl->allocatedCones[i]);
  _build_vector( &v, vl->allocatedVectors[c->iv2].x + vl->allocatedVectors[c->iv3].x,
		 vl->allocatedVectors[c->iv2].y + vl->allocatedVectors[c->iv3].y, 
		 vl->allocatedVectors[c->iv2].z + vl->allocatedVectors[c->iv3].z, s );
  iv = _add_vector_to_list( vl, &v ); 
  
  /* create new cone 
   */
  _build_3D_cone( vl, &newc, c->iv1, iv, c->iv3 );
  ic = _add_cone_to_list( cl, &newc );
  _move_cone_backward( cl, ic );
  
  /* change the considered cone 
     the previous one should be further in the list (it has a smaller error),
     so there won't any interaction
   */
  cl->allocatedCones[i].iv3 = iv;
  _compute_3D_cone_error( &(cl->allocatedCones[i]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv1]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv2]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv3]) );
  _move_cone_forward( cl, i );
}



void _split_3D_cone_along_KI( VectorList *vl, ConeList *cl, double *s, int i )
{
  Cone *c, newc;
  Vector v;
  int iv, ic;

  /* get or create new vector 
   */
  c = &(cl->allocatedCones[i]);
  _build_vector( &v, vl->allocatedVectors[c->iv3].x + vl->allocatedVectors[c->iv1].x,
		 vl->allocatedVectors[c->iv3].y + vl->allocatedVectors[c->iv1].y, 
		 vl->allocatedVectors[c->iv3].z + vl->allocatedVectors[c->iv1].z, s );
  iv = _add_vector_to_list( vl, &v );
  
  /* create new cone 
   */
  _build_3D_cone( vl, &newc, c->iv1, c->iv2, iv );
  ic = _add_cone_to_list( cl, &newc );
  _move_cone_backward( cl, ic );
	
  /* change the considered cone 
     the previous one should be further in the list (it has a smaller error),
     so there won't any interaction
   */
  cl->allocatedCones[i].iv1 = iv;
  _compute_3D_cone_error( &(cl->allocatedCones[i]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv1]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv2]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv3]) );
  _move_cone_forward( cl, i );
}



void _split_3D_cone_along_IJ( VectorList *vl, ConeList *cl, double *s, int i )
{
  Cone *c, newc;
  Vector v;
  int iv, ic;

  /* get or create new vector 
   */
  c = &(cl->allocatedCones[i]);
  _build_vector( &v, vl->allocatedVectors[c->iv1].x + vl->allocatedVectors[c->iv2].x,
		 vl->allocatedVectors[c->iv1].y + vl->allocatedVectors[c->iv2].y, 
		 vl->allocatedVectors[c->iv1].z + vl->allocatedVectors[c->iv2].z, s );
  iv = _add_vector_to_list( vl, &v );
  
  /* create new cone 
   */
  _build_3D_cone( vl, &newc, iv, c->iv2, c->iv3 );
  ic = _add_cone_to_list( cl, &newc );
  _move_cone_backward( cl, ic );

  /* change the considered cone 
     the previous one should be further in the list (it has a smaller error),
     so there won't any interaction
   */
  cl->allocatedCones[i].iv2 = iv;
  _compute_3D_cone_error( &(cl->allocatedCones[i]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv1]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv2]),
			  &(vl->allocatedVectors[cl->allocatedCones[i].iv3]) );
  _move_cone_forward( cl, i );
}



void _split_3D_cone( FILE *fout, VectorList *vl, ConeList *cl, 
		     double *s, int index_cone, LocalisationType edge ) 
{
  char *proc = "_split_3D_cone";
  Cone *cc;
  int iv1=0, iv2=0, iv3=0;
  int ncones, adjcone;
  LocalisationType adjside;

  ncones = cl->n_cones;
  cc = &(cl->allocatedCones[index_cone]);

  switch( edge ) {

  default :
    break;

  case _EDGE_JK_ :

    iv1 = cc->iv2;
    iv2 = cc->iv3;
    iv3 = cc->iv1;

    _split_3D_cone_along_JK( vl, cl, s, index_cone );

    break;

  case _EDGE_KI_ :

    iv1 = cc->iv3;
    iv2 = cc->iv1;
    iv3 = cc->iv2;

    _split_3D_cone_along_KI( vl, cl, s, index_cone );

    break;

  case _EDGE_IJ_ :

    iv1 = cc->iv1;
    iv2 = cc->iv2;
    iv3 = cc->iv3;

    _split_3D_cone_along_IJ( vl, cl, s, index_cone );

    break;

  }

  adjcone = _search_adjacent_3D_cone( cl, iv1, iv2, iv3, &adjside );

  switch( adjside ) {
  default : 
    break;
  case _EDGE_JK_ :
    _split_3D_cone_along_JK( vl, cl, s, adjcone );
    break;
  case _EDGE_KI_ :
    _split_3D_cone_along_KI( vl, cl, s, adjcone );
    break;
  case _EDGE_IJ_ :
    _split_3D_cone_along_IJ( vl, cl, s, adjcone );
    break;
  }

  if ( _trace_ >= 2 ) {
    fprintf( fout, "%s: add vector (%2d %2d %2d)", proc, 
	     vl->allocatedVectors[iv1].x + vl->allocatedVectors[iv2].x,
	     vl->allocatedVectors[iv1].y + vl->allocatedVectors[iv2].y,
	     vl->allocatedVectors[iv1].z + vl->allocatedVectors[iv2].z );
    fprintf( fout, " => maximal error becomes %f\n", 
	     cl->allocatedCones[0].error );
    fprintf( fout, "                add %d cone(s) => #cones becomes %d\n", 
	     cl->n_cones-ncones, cl->n_cones );
  }

  _check_3D_mask_naturalconvexity( fout, vl, cl, s );

  if ( _trace_ >= 3 ) {
    _print_cone_list( fout, vl, cl );
  }
  

}



int _split_best_3D_cone( FILE *fout, VectorList *vl, ConeList *cl, 
			  double *s, int index_cone, int depth )
{
  int ok[3];
  Vector *v1, *v2, *v3;
  double error[3], minerror = 0;
  int imin = -1;
  LocalisationType split = _UNSET_;
  Vector sum;
  Cone *cc, csum;

  cc = &(cl->allocatedCones[index_cone]);

  v1 = &(vl->allocatedVectors[ cc->iv1 ]);
  v2 = &(vl->allocatedVectors[ cc->iv2 ]);
  v3 = &(vl->allocatedVectors[ cc->iv3 ]);

  ok[0] = ok[1] = ok[2] = 0;

  if ( v1->x + v2->x <= depth && v1->y + v2->y <= depth && v1->z + v2->z <= depth  ) {

    ok[2] = 2;

    _build_vector( &sum, v1->x + v2->x, v1->y + v2->y, v1->z + v2->z, s );

    _init_cone( &csum );
    _compute_3D_cone_error( &csum, &sum, v2, v3 );
    error[2] = csum.error;
    
    _compute_3D_cone_error( &csum, v1, &sum, v3 );
    if ( error[2] < csum.error ) error[2] = csum.error;
    
    if ( split != _UNSET_ ) {
      if ( minerror > error[2] ) {
	split = _EDGE_IJ_;
	minerror = error[2];
	imin = 2;
      }
    }
    else {
      split = _EDGE_IJ_;
      minerror = error[2];
      imin = 2;
    }

  }

  if ( v2->x + v3->x <= depth && v2->y + v3->y <= depth && v2->z + v3->z <= depth  ) {

    ok[0] = 2;

    _build_vector( &sum, v2->x + v3->x, v2->y + v3->y, v2->z + v3->z, s );

    _init_cone( &csum );
    _compute_3D_cone_error( &csum, v1, &sum, v3 );
    error[0] = csum.error;
    
    _compute_3D_cone_error( &csum, v1, v2, &sum );
    if ( error[0] < csum.error ) error[0] = csum.error;
    
    if ( split != _UNSET_ ) {
      if ( minerror > error[0] ) {
	split = _EDGE_JK_;
	minerror = error[0];
	imin = 0;
      }
    }
    else {
      split = _EDGE_JK_;
      minerror = error[0];
      imin = 0;
    }

  }

  if ( v3->x + v1->x <= depth && v3->y + v1->y <= depth && v3->z + v1->z <= depth  ) {

    ok[1] = 2;

    _build_vector( &sum, v3->x + v1->x, v3->y + v1->y, v3->z + v1->z, s );

    _init_cone( &csum );
    _compute_3D_cone_error( &csum, v1, v2, &sum );
    error[1] = csum.error;
    
    _compute_3D_cone_error( &csum, &sum, v2, v3 );
    if ( error[1] < csum.error ) error[1] = csum.error;
    
    if ( split != _UNSET_ ) {
      if ( minerror > error[1] ) {
	split = _EDGE_KI_;
	minerror = error[1];
	imin = 1;
      }
    }
    else {
      split = _EDGE_KI_;
      minerror = error[1];
      imin = 1;
    }

  }

  if ( ok[0] == 0 && ok[1] == 0 && ok[2] == 0 ) {
    return( 0 );
  }

  _split_3D_cone( fout, vl, cl, s, index_cone, split );
  
  return( 1 );
}



void _correct_3D_mask_wrt_naturalconvexity( FILE *f, VectorList *vl, 
					    ConeList *cl, double *s )
{
  char *proc = "_correct_3D_mask_wrt_naturalconvexity";
  int i, j, k;
  LocalisationType side;
  int nc;
  int icv1c1, icv2c1, iovc1;
  int icv1c2, icv2c2, iovc2;
  double error1, error2;
  int dontstop;

  if ( _trace_ >= 2 ) {
    fprintf( f, "%s:\n", proc );
  }

  do {
    
    dontstop = 0;

    if ( _debug_ >= 2 ) 
      _check_3D_mask_naturalconvexity( f, vl, cl, s );

    /* count cones that are not naturally convex
     */
    nc = 0;
    for ( i=0; i<cl->n_cones; i++ ) {
      (void)_check_3D_cone_naturalconvexity( f, vl, cl, &(cl->allocatedCones[i]), s );
      if ( cl->allocatedCones[i].convexity == 0 ) {
	nc += 3;
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_IJ_
		|| cl->allocatedCones[i].convexity == _EDGE_JK_
		|| cl->allocatedCones[i].convexity == _EDGE_KI_ ) {
	nc += 2;
      }      
      else if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_JK_
		|| cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_KI_
		|| cl->allocatedCones[i].convexity == _EDGE_JK_ + _EDGE_KI_ ){
	nc += 1;
      }
    }

    
    if ( _trace_ >= 2 ) {
      fprintf( f, "  natural convexity is unverified %d times\n", nc );
    }


    if ( nc == 0 || nc == 1 )
      continue;
    

    for ( i=0; i<cl->n_cones && dontstop == 0; i++ ) {

      /* look for cone with ONE convexity problem
       */
      if ( cl->allocatedCones[i].convexity != _EDGE_IJ_ + _EDGE_JK_ 
	   && cl->allocatedCones[i].convexity != _EDGE_IJ_ + _EDGE_KI_ 
	   && cl->allocatedCones[i].convexity != _EDGE_JK_ + _EDGE_KI_ )
	continue;


      
      if ( _trace_ >= 3 ) {
	fprintf( f, "  ... process cone #%2d\n", i );
      }

      /* indices of common vectors and of other vector for cone #1
       */
      icv1c1 = icv2c1 = iovc1 = -1;
      if ( cl->allocatedCones[i].convexity == _EDGE_JK_ + _EDGE_KI_ ) {
	icv1c1 = cl->allocatedCones[i].iv1;
	icv2c1 = cl->allocatedCones[i].iv2;
	iovc1  = cl->allocatedCones[i].iv3;
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_KI_ ) {
	icv1c1 = cl->allocatedCones[i].iv2;
	icv2c1 = cl->allocatedCones[i].iv3;
	iovc1  = cl->allocatedCones[i].iv1;
      } 
      else if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_JK_ ) {
	icv1c1 = cl->allocatedCones[i].iv3;
	icv2c1 = cl->allocatedCones[i].iv1;
	iovc1  = cl->allocatedCones[i].iv2;
      }

      /* search adjacent cone 
       */
      j = _search_adjacent_3D_cone( cl, icv1c1, icv2c1, iovc1, &side );

      if ( j == -1 ) {
	if ( _trace_ >= 3 ) {
	  fprintf( f, "      there is no adjacent cone for edge #%2d - #%2d\n",
		   icv1c1, icv2c1 );
	}
	continue;
      }

      if ( _trace_ >= 3 ) {
	fprintf( f, "      adjacent cone = #%2d\n", j );
      }
      

      /* indices of common vectors and of other vector for cone #2
       */

      if ( cl->allocatedCones[j].iv1 == icv2c1 &&
	   cl->allocatedCones[j].iv2 == icv1c1 &&
	   ( cl->allocatedCones[j].convexity == _EDGE_JK_ + _EDGE_KI_
	     || cl->allocatedCones[j].convexity == _EDGE_JK_
	     || cl->allocatedCones[j].convexity == _EDGE_KI_
	     || cl->allocatedCones[j].convexity == 0 ) ) {
	icv1c2 = cl->allocatedCones[j].iv1;
	icv2c2 = cl->allocatedCones[j].iv2;
	iovc2  = cl->allocatedCones[j].iv3;
      }
      else if ( cl->allocatedCones[j].iv2 == icv2c1 &&
		cl->allocatedCones[j].iv3 == icv1c1 &&
	   ( cl->allocatedCones[j].convexity == _EDGE_IJ_ + _EDGE_KI_
	     || cl->allocatedCones[j].convexity == _EDGE_IJ_
	     || cl->allocatedCones[j].convexity == _EDGE_KI_
	     || cl->allocatedCones[j].convexity == 0 ) ) {
	icv1c2 = cl->allocatedCones[j].iv2;
	icv2c2 = cl->allocatedCones[j].iv3;
	iovc2  = cl->allocatedCones[j].iv1;
      }
      else if ( cl->allocatedCones[j].iv3 == icv2c1 &&
		cl->allocatedCones[j].iv1 == icv1c1 &&
	   ( cl->allocatedCones[j].convexity == _EDGE_IJ_ + _EDGE_JK_
	     || cl->allocatedCones[j].convexity == _EDGE_IJ_
	     || cl->allocatedCones[j].convexity == _EDGE_JK_
	     || cl->allocatedCones[j].convexity == 0 ) ) {
	icv1c2 = cl->allocatedCones[j].iv3;
	icv2c2 = cl->allocatedCones[j].iv1;
	iovc2  = cl->allocatedCones[j].iv2;
      }
      else {
	if ( _trace_ >= 3 ) {
	  fprintf( f, "      its concavities do not correspond -> do nothing\n" );
	}
	continue;
      }



      /* switch edges
       */
      cl->allocatedCones[i].iv1 = icv1c1;
      cl->allocatedCones[i].iv2 = iovc2;
      cl->allocatedCones[i].iv3 = iovc1;
      cl->allocatedCones[j].iv1 = iovc2;
      cl->allocatedCones[j].iv2 = icv2c1;
      cl->allocatedCones[j].iv3 = iovc1;

      (void)_check_3D_cone_naturalconvexity( f, vl, cl, &(cl->allocatedCones[i]), s );
      (void)_check_3D_cone_naturalconvexity( f, vl, cl, &(cl->allocatedCones[j]), s );

      error1 = cl->allocatedCones[i].error;
      _compute_3D_cone_error( &(cl->allocatedCones[i]),
			      &(vl->allocatedVectors[ cl->allocatedCones[i].iv1 ]),
			      &(vl->allocatedVectors[ cl->allocatedCones[i].iv2 ]),
			      &(vl->allocatedVectors[ cl->allocatedCones[i].iv3 ]) );
      if ( error1 > cl->allocatedCones[i].error ) {
	_move_cone_forward( cl, i );
      }
      else if ( error1 < cl->allocatedCones[i].error ) {
	_move_cone_backward( cl, i );
      }
      
      /* find again #j, it may have moved
       */
      for ( j=-1, k=0; k<cl->n_cones && j == -1; k++ ) {
	if ( cl->allocatedCones[k].iv1 == iovc2 &&
	     cl->allocatedCones[k].iv2 == icv2c1 &&
	     cl->allocatedCones[k].iv3 == iovc1 )
	  j = k;
      }
      error2 = cl->allocatedCones[j].error;
      _compute_3D_cone_error( &(cl->allocatedCones[j]),
			      &(vl->allocatedVectors[ cl->allocatedCones[j].iv1 ]),
			      &(vl->allocatedVectors[ cl->allocatedCones[j].iv2 ]),
			      &(vl->allocatedVectors[ cl->allocatedCones[j].iv3 ]) );
      if ( error2 > cl->allocatedCones[j].error ) {
	_move_cone_forward( cl, j );
      }
      else if ( error2 < cl->allocatedCones[j].error ) {
	_move_cone_backward( cl, j );
      }
     
      dontstop = 1;
    }

    
  } while ( dontstop == 1 );

}



void _build_3D_mask( FILE *fout, VectorList *vl, ConeList *cl, double *s, int size, int correction ) 
{
  int d = (size-1)/2;
  int i, k;

  if ( _trace_ >= 3 ) {
    _print_cone_list( fout, vl, cl );
  }

  /* loop on depth
   */
  for ( k=2; k<=d; k++ ) {
    
    /* loop on cones
     */
    for ( i=0; i<cl->n_cones; i++ ) {

      /* if not possible to split, go to next cone
       */
      if ( _split_best_3D_cone( fout, vl, cl, s, i, k ) == 0 ) {
	continue;
      }

      /* else go ahead
       */

      if ( correction )
	_correct_3D_mask_wrt_naturalconvexity( fout, vl, cl, s );

      i--;
      
    }
  }
}



void _build_3D_mask_wrt_error( FILE *fout, VectorList *vl, ConeList *cl, 
			       double *s, int size, int n_vectors,
			       double mask_error, int correction ) 
{
  int d = (size-1)/2;
  int i=0;


  if ( _trace_ >= 3 ) {
    _print_cone_list( fout, vl, cl );
  }

  /* loop on cones
   */
  while ( i<cl->n_cones && vl->n_vectors < n_vectors 
	  && cl->allocatedCones[i].error > mask_error ) {
    
    /* is it possible to split ?
     */
    if ( _split_best_3D_cone( fout, vl, cl, s, i, d ) == 0 ) {
      i++;
      continue;
    }

    if ( correction )
      _correct_3D_mask_wrt_naturalconvexity( fout, vl, cl, s );
  }
}



void _build_3D_mask_XxXx5( FILE *fout, VectorList *vl, ConeList *cl, double *s, int size ) 
{
  int ic;
  
  ic = _search_3D_cone( vl, cl, 1, 0, 0, 1, 1, 0, 1, 1, 1 );
  _split_3D_cone( fout, vl, cl, s, ic, _EDGE_KI_ );
  
  if ( size == 335 ) return;

  ic = _search_3D_cone( vl, cl, 1, 0, 0, 1, 1, 0, 2, 1, 1 );
  _split_3D_cone( fout, vl, cl, s, ic, _EDGE_IJ_ );


  if ( size == 355 ) return;

  _split_3D_cone( fout, vl, cl, s, 1, _EDGE_JK_ );

}



void _build_3D_mask_3x3x5( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  _build_3D_mask_XxXx5( fout, vl, cl, s, 335 );
}

void _build_3D_mask_3x5x5( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  _build_3D_mask_XxXx5( fout, vl, cl, s, 355 );
}

void _build_3D_mask_5x5x5( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  _build_3D_mask_XxXx5( fout, vl, cl, s, 555 );
}



void _build_3D_mask_XxXx5_2( FILE *fout, VectorList *vl, ConeList *cl, double *s, int size ) 
{
  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_IJ_ );

  if ( size == 3352 ) return;

  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_KI_ );

  if ( size == 3552 ) return;

  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_JK_ ); 
}



void _build_3D_mask_3x3x5_2( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  _build_3D_mask_XxXx5_2( fout, vl, cl, s, 3352 );
}

void _build_3D_mask_5x5x5_2( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  _build_3D_mask_XxXx5_2( fout, vl, cl, s, 5552 );
}



void _build_3D_mask_XxXx5_3( FILE *fout, VectorList *vl, ConeList *cl, double *s, int size ) 
{
  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_JK_ );

  if ( size == 3353 ) return;

  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_KI_ );

  if ( size == 3553 ) return;

  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_IJ_ );
}



void _build_3D_mask_3x3x5_3( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  _build_3D_mask_XxXx5_3( fout, vl, cl, s, 3353 );
}

void _build_3D_mask_5x5x5_3( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{
  _build_3D_mask_XxXx5_3( fout, vl, cl, s, 5553 );
}



void _build_3D_mask_7x7x7( FILE *fout, VectorList *vl, ConeList *cl, double *s ) 
{

  _build_3D_mask_5x5x5( fout, vl, cl, s );

  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_KI_ );

  _split_3D_cone( fout, vl, cl, s, 2, _EDGE_IJ_ );

  _split_3D_cone( fout, vl, cl, s, 4, _EDGE_JK_ );

  _split_3D_cone( fout, vl, cl, s, 4, _EDGE_KI_ );

  _split_3D_cone( fout, vl, cl, s, 1, _EDGE_JK_ );

  _split_3D_cone( fout, vl, cl, s, 0, _EDGE_JK_ );

  _split_3D_cone( fout, vl, cl, s, 3, _EDGE_IJ_ );

}



/* this depends how the mask was symmetrised
 */
int _symmetrise_vector_list( VectorList *nl, VectorList *vl,
			     int anisotropy_y, int anisotropy_z )
{
  char *proc = "_symmetrise_vector_list";
  int notinplane;
  int i, n, nv0, nv1;
  Vector normal, res;
  double voxel_size[3] = { 1.0, 1.0, 1.0 };

  _init_vector_list( nl );

  notinplane = 0;
  for ( i=0; i<vl->n_vectors; i++ ) {
    (void)_add_vector_to_list( nl, &(vl->allocatedVectors[i]) );
    if ( vl->allocatedVectors[i].z != 0 ) notinplane++;
  }
  

  if ( _trace_ >= 4 ) {
    fprintf( stdout, "%s: initial vectors", proc );
    _print_3D_vector_list( stdout, nl );
    fprintf( stdout, "\n" );
  }

  n = nl->n_vectors;

  
  /* 2D case 
   */
  if ( !notinplane ) {
    /* symmetry w.r.t (1,-1,0)
     */
    normal.x =  1;
    normal.y = -1;
    normal.z =  0;
    
    n = nl->n_vectors;
    for ( i=0; i<n; i++ ) {
      _build_3D_symmetric_vector_wrt_vector( &res, &(nl->allocatedVectors[i]), &normal, voxel_size );
      (void)_add_vector_to_list( nl, &res );
    }
  }

  /* 3D case 
   */
  else {

    if ( !anisotropy_z && !anisotropy_y ) {
      nv0 = nl->n_vectors;
      /* symmetry w.r.t (0,-1,1)
       */
      normal.x =  0;
      normal.y = -1;
      normal.z =  1;
      for ( i=0; i<nv0; i++ ) {
	_build_3D_symmetric_vector_wrt_vector( &res, &(nl->allocatedVectors[i]), &normal, voxel_size );
	(void)_add_vector_to_list( nl, &res );
      }
      nv1 = nl->n_vectors;
      /* symmetry w.r.t (1,0,-1)
       */
      normal.x =  1;
      normal.y =  0;
      normal.z = -1;
      for ( i=nv0; i<nv1; i++ ) {
	_build_3D_symmetric_vector_wrt_vector( &res, &(nl->allocatedVectors[i]), &normal, voxel_size );
	(void)_add_vector_to_list( nl, &res );
      }
      n = nl->n_vectors;
      /* symmetry w.r.t (1,-1,0)
       */
      normal.x =  1;
      normal.y = -1;
      normal.z =  0;
      for ( i=0; i<n; i++ ) {
	_build_3D_symmetric_vector_wrt_vector( &res, &(nl->allocatedVectors[i]), &normal, voxel_size );
	(void)_add_vector_to_list( nl, &res );
      }
    }
    
    if ( anisotropy_z && !anisotropy_y ) {
      n = nl->n_vectors;
      /* symmetry w.r.t (1,-1,0)
       */
      normal.x =  1;
      normal.y = -1;
      normal.z =  0;
      for ( i=0; i<n; i++ ) {
	_build_3D_symmetric_vector_wrt_vector( &res, &(nl->allocatedVectors[i]), &normal, voxel_size );
	(void)_add_vector_to_list( nl, &res );
      }
    }

    if ( !anisotropy_z && anisotropy_y ) {
      n = nl->n_vectors;
      /* symmetry w.r.t (1,0,-1)
       */
      normal.x =  1;
      normal.y =  0;
      normal.z = -1;
      for ( i=0; i<n; i++ ) {
	_build_3D_symmetric_vector_wrt_vector( &res, &(nl->allocatedVectors[i]), &normal, voxel_size );
	(void)_add_vector_to_list( nl, &res );
      }
    }
  }
  
  return( 1 );
}
