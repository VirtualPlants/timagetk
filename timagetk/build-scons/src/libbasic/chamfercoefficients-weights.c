/*************************************************************************
 * chamfercoefficients-weights.c - computation of chamfer distance coefficients
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

int _link_cones_to_vectors( WeightedVectorList *wvl, ConeList *cl )
{
  int i, j;

  for ( i=0; i<wvl->n_vectors; i++ ) 
    wvl->allocatedVectors[i].n_cones = 0;

  for ( i=0; i<cl->n_cones; i++ ) {
    for ( j=0; j<wvl->n_vectors; j++ ) {
      if ( j == cl->allocatedCones[i].iv1
	   || j == cl->allocatedCones[i].iv2
	   || j == cl->allocatedCones[i].iv3 )
	wvl->allocatedVectors[j].n_cones ++;
    }
  }

  for ( i=0; i<wvl->n_vectors; i++ ) {
    if ( wvl->allocatedVectors[i].n_cones > 0 ) {
      wvl->allocatedVectors[i].c = (Cone**)malloc( wvl->allocatedVectors[i].n_cones *
                               sizeof( Cone* ) );
      if ( wvl->allocatedVectors[i].c == NULL ) {
	fprintf( stderr, "Allocation error (#cones=%d)\n", 
           wvl->allocatedVectors[i].n_cones );
	return( -1 );
      }
      wvl->allocatedVectors[i].n_cones = 0;
    }
  }
  
  for ( i=0; i<cl->n_cones; i++ ) {
    for ( j=0; j<wvl->n_vectors; j++ ) {
      if ( j == cl->allocatedCones[i].iv1
	   || j == cl->allocatedCones[i].iv2
	   || j == cl->allocatedCones[i].iv3 ) {
	wvl->allocatedVectors[j].c[ wvl->allocatedVectors[j].n_cones ] = &(cl->allocatedCones[i]);
	wvl->allocatedVectors[j].n_cones ++;
      }
    }
  }

  return( 1 );
}



void _sort_2D_vectors( WeightedVector **v, WeightedVectorList *wvl, ConeList *cl )
{
  int i, j, n;
  
  for ( i=0; i<wvl->n_vectors; i++ ) 
    wvl->allocatedVectors[i].flag = 0;
  for ( i=0; i<cl->n_cones; i++ ) 
    cl->allocatedCones[i].flag = 0;
  
  /* first cone
   */

  if ( _norme2v_2D( wvl->allocatedVectors[cl->allocatedCones[0].iv1].vec ) 
       < _norme2v_2D( wvl->allocatedVectors[cl->allocatedCones[0].iv2].vec ) ) {
    v[0] = &( wvl->allocatedVectors[cl->allocatedCones[0].iv1] );
    v[1] = &( wvl->allocatedVectors[cl->allocatedCones[0].iv2] );
  }
  else {
    v[0] = &( wvl->allocatedVectors[cl->allocatedCones[0].iv2] );
    v[1] = &( wvl->allocatedVectors[cl->allocatedCones[0].iv1] );
  }

  cl->allocatedCones[0].flag = 1;
  v[0]->flag = 1;
  v[1]->flag = 1;

  n = 2;

  for ( ; n < wvl->n_vectors;  ) {

    for ( j=0, i=1; i<cl->n_cones && j==0; i++ ) {
      if ( cl->allocatedCones[i].flag == 1 ) continue;
      if ( wvl->allocatedVectors[cl->allocatedCones[i].iv1].flag == 0 
	   && wvl->allocatedVectors[cl->allocatedCones[i].iv2].flag == 0 )
	continue;
      j = i;
    }

    if ( j > 0 ) {
      cl->allocatedCones[j].flag = 1;
      if ( wvl->allocatedVectors[cl->allocatedCones[j].iv1].flag == 0 ) {
	v[n] = &( wvl->allocatedVectors[cl->allocatedCones[j].iv1] );
	v[n]->flag = 1;
	n++;
      }
      if ( wvl->allocatedVectors[cl->allocatedCones[j].iv2].flag == 0 ) {
	v[n] = &( wvl->allocatedVectors[cl->allocatedCones[j].iv2] );
	v[n]->flag = 1;
	n++;
      }
    }      

  }

  if ( 0 && _verbose_ >= 3 ) {
    fprintf( stderr, "#SORTED VECTORS:\n" );
    for (i=0; i<wvl->n_vectors; i++ ) {
      fprintf( stderr, "    v[#%2d] = ", i );
      _print_2D_vector( stderr, v[i]->vec );
       fprintf( stderr, "\n" );
    }
    fprintf( stderr, "\n" );
  }
}



int _check_2D_cone_convexity( WeightedVectorList *wvl, ConeList *cl, Cone *c )
{
  char *proc="_check_2D_cone_convexity";
  WeightedVector *v1, *v2;
  int i;
  int mat[3][3];
  Cone *cc;
  int convexity_1 = -2;
  int convexity_2 = -2;
  int sp, n1, n2;
  Vector sym;


  v1 = &(wvl->allocatedVectors[ c->iv1 ]);
  v2 = &(wvl->allocatedVectors[ c->iv2 ]);

  if ( v1->flag != 1 || v2->flag != 1 )
    return( -1 );

  /* convexity at side iv1
   */
  _init_vector( &sym );
  mat[1][0] = v1->vec->x;
  mat[1][1] = v1->vec->y;
  mat[1][2] = v1->w;
  mat[2][0] = v2->vec->x;
  mat[2][1] = v2->vec->y;
  mat[2][2] = v2->w;

  for ( i=0; i<cl->n_cones && convexity_1 == -2; i++ ) {
    cc = &(cl->allocatedCones[i]);
    if ( cc->iv2 != c->iv1 ) continue;
    if ( wvl->allocatedVectors[cc->iv1].flag != 1 ) {
      convexity_1 = -1;
      continue;
    }
    mat[0][0] = wvl->allocatedVectors[cc->iv1].vec->x;
    mat[0][1] = wvl->allocatedVectors[cc->iv1].vec->y;
    mat[0][2] = wvl->allocatedVectors[cc->iv1].w;
    if ( _3x3_determinant( mat ) < 0 ) {
      return( 0 );
    }
    convexity_1 = 1;
  }

  if ( convexity_1 == -2 ) {
    n1 = v1->vec->x * v1->vec->x + v1->vec->y * v1->vec->y; 
    sp = v1->vec->x * v2->vec->x + v1->vec->y * v2->vec->y; 
    sym.x = 2 * sp * v1->vec->x / n1 - v2->vec->x;
    sym.y = 2 * sp * v1->vec->y / n1 - v2->vec->y;
    if ( v1->vec->x != 1 || v1->vec->y != 0 ) {
      fprintf( stderr, "%s: unexpected symmetry (#1) w.r.t. (%d %d)\n", proc,
	       v1->vec->x, v1->vec->y );
      fprintf( stderr, "\t sym. vector of (%d %d) is (%d %d)\n", 
	       v2->vec->x, v2->vec->y, sym.x, sym.y );
    }
    mat[0][0] = sym.x;
    mat[0][1] = sym.y;
    mat[0][2] = v2->w;
    if ( _3x3_determinant( mat ) < 0 ) {
      return( 0 );
    }
    convexity_1 = 1;
  }


  /* convexity at side iv2
   */
  _init_vector( &sym );
  mat[0][0] = v1->vec->x;
  mat[0][1] = v1->vec->y;
  mat[0][2] = v1->w;
  mat[1][0] = v2->vec->x;
  mat[1][1] = v2->vec->y;
  mat[1][2] = v2->w;

  for ( i=0; i<cl->n_cones && convexity_2 == -2; i++ ) {
    cc = &(cl->allocatedCones[i]);
    if ( cc->iv1 != c->iv2 ) continue;
    if ( wvl->allocatedVectors[cc->iv2].flag != 1 ) {
      convexity_2 = -1;
      continue;
    }
    mat[2][0] = wvl->allocatedVectors[cc->iv2].vec->x;
    mat[2][1] = wvl->allocatedVectors[cc->iv2].vec->y;
    mat[2][2] = wvl->allocatedVectors[cc->iv2].w;
    if ( _3x3_determinant( mat ) < 0 ) {
      return( 0 );
    }
    convexity_2 = 1;
  }

  if ( convexity_2 == -2 ) {
    n2 = v2->vec->x * v2->vec->x + v2->vec->y * v2->vec->y; 
    sp = v1->vec->x * v2->vec->x + v1->vec->y * v2->vec->y; 
    sym.x = 2 * sp * v2->vec->x / n2 - v1->vec->x;
    sym.y = 2 * sp * v2->vec->y / n2 - v1->vec->y;
    if ( (v2->vec->x != 1 || v2->vec->y != 1) && (v2->vec->x != 0 || v2->vec->y != 1) ) {
      fprintf( stderr, "%s: unexpected symmetry (#2) w.r.t. (%d %d)\n", proc,
	       v2->vec->x, v2->vec->y );
    }
    mat[2][0] = sym.x;
    mat[2][1] = sym.y;
    mat[2][2] = v1->w;
    if ( _3x3_determinant( mat ) < 0 ) {
      return( 0 );
    }
    convexity_2 = 1;
  }

  return( 1 );

}



void _compute_2D_cone_max( WeightedVector *v1, WeightedVector *v2, 
               double *dmax )
{
  char *proc = "_compute_2D_cone_max";
  double sp;
  double x, y;

  sp = v1->vec->vx * v2->vec->vx + v1->vec->vy * v2->vec->vy;

  if ( v1->w * v2->vec->vn * v2->vec->vn >= sp *v2->w ) {
    if ( v2->w * v1->vec->vn * v1->vec->vn >= sp *v1->w ) {
      x = v1->w * v2->vec->vx - v2->w * v1->vec->vx;
      y = v1->w * v2->vec->vy - v2->w * v1->vec->vy;
      *dmax = sqrt( x*x + y*y ) / fabs( v1->vec->vx * v2->vec->vy - v1->vec->vy * v2->vec->vx );
    }
    else {
      *dmax = v1->w / v1->vec->vn;
    }
  }
  else {
    if ( v2->w * v1->vec->vn * v1->vec->vn >= sp *v1->w ) {
      *dmax = v2->w / v2->vec->vn;
    }
    else {
      fprintf( stdout, "%s: this case should never occur\n", proc );
    }
  }
}



void _compute_2D_cone_min_max( WeightedVector *v1, WeightedVector *v2, 
              double *dmin, double *dmax )
{
  *dmin = v1->w / v1->vec->vn; 
  if ( *dmin > v2->w / v2->vec->vn ) *dmin = v2->w / v2->vec->vn;

 _compute_2D_cone_max( v1, v2, dmax );
}



void _compute_2D_mask_min_max( WeightedVectorList *vl, ConeList *cl, 
                   double *dmin, double *dmax )
{
  int i;
  double ldmax;
  
  *dmin = vl->allocatedVectors[0].w / vl->allocatedVectors[0].vec->vn;
  
  for ( i=1; i<vl->n_vectors; i++ ) 
    if ( *dmin > vl->allocatedVectors[i].w / vl->allocatedVectors[i].vec->vn )
      *dmin = vl->allocatedVectors[i].w / vl->allocatedVectors[i].vec->vn;

  *dmax = *dmin;
  for ( i=0; i<cl->n_cones; i++ ) {
    _compute_2D_cone_max( &vl->allocatedVectors[cl->allocatedCones[i].iv1],
              &vl->allocatedVectors[cl->allocatedCones[i].iv2],
              &ldmax );
    if ( *dmax < ldmax ) *dmax = ldmax;
  }
}







int _compute_2D_weights( FILE *fout, WeightsList *wl, 
             VectorList *vl, ConeList *cl,
              int f, int l )
{
  char *proc = "_compute_2D_weights";
  WeightedVectorList wvl;
  WeightedVector **v;
  int i, j;
  double dmin, dmax, eps;
  double perr=1.0;
  double erropt, err;
  int iv;

  int ok;
  Cone * cone;
  double ldmin, ldmax, lerr, leps;

#ifndef WIN32
  struct tms timestruct;
#endif
  double it = 0, ct = 0;



  
  /* weighted vectors
   */
  _init_weighted_vector_list( &wvl );
  _init_weighted_vector_list_from_vector_list( &wvl, vl );


  /* add links from vectors to cones
   */
  (void)_link_cones_to_vectors( &wvl, cl );


  /* allocate and sort the vectors
   */
  v = (WeightedVector**)malloc( vl->n_vectors * sizeof(WeightedVector*) );
  if ( v == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }
  _sort_2D_vectors( v, &wvl, cl );



  /* initialization
     flag = 0 -> no set yet
     flag = 1 -> already set
  */

  for ( i=0; i<wvl.n_vectors; i++ ) 
    wvl.allocatedVectors[i].flag = 0;
  for ( i=0; i<cl->n_cones; i++ ) 
    cl->allocatedCones[i].flag = 0;


  /* the largest error
   */
  erropt = cl->allocatedCones[0].error;


  /* set bounds for the vectors of the first cone
     low bound = f
     high bound = l * |v|_1
   */
  v[0]->f = floor(f * _normeIv_2D( v[0]->vec ) );
  v[0]->l = (int)(l * _norme1v_2D( v[0]->vec ) );

  v[1]->f = floor(f * _normeIv_2D( v[1]->vec ) );
  v[1]->l = (int)(l * _norme1v_2D( v[1]->vec ) );

  v[0]->flag = 1;
  v[1]->flag = 1;


#ifndef WIN32
  it = times( &timestruct );
#endif


  /* loops for the first cone
   */
  for ( v[0]->w  = v[0]->f;
	v[0]->w <= v[0]->l;
	v[0]->w ++ ) {
    if ( _trace_ )
      fprintf( fout,"%3d   [%d %d]\n", v[0]->w, v[0]->f, v[0]->l );

    for ( v[1]->w  = v[1]->f;
	  v[1]->w <= v[1]->l;
	  v[1]->w ++ ) {
      if ( _trace_ )
	fprintf( fout, "%3d %3d   [%d %d]", v[0]->w, v[1]->w, v[1]->f, v[1]->l );

      /* check condition
	 => no more necessary for convexity, 
	 since bounds on v[1] are computed so that it is satisfied
       */
      if ( _check_2D_cone_convexity( &wvl, cl, &(cl->allocatedCones[0]) ) == 0 ) {
	if ( _trace_ )
	  fprintf( fout, " cone #1 not convex\n" );
      }

      if ( _trace_ )
	fprintf( fout, "\n" );



      /* compute min and max distance, epsilon and error
      */      
      _compute_2D_cone_min_max( &(wvl.allocatedVectors[cl->allocatedCones[0].iv1]),
                    &(wvl.allocatedVectors[cl->allocatedCones[0].iv2]),
                    &dmin, &dmax );
      eps = (dmin + dmax)/2.0;
      err = dmax/eps - 1;


      
      /* if we do not perform better,
	 go to next
      */
      if ( err >= perr ) {
	if ( _trace_ ) {
	  fprintf( fout,"%3d %3d too large error", v[0]->w, v[1]->w );
	  fprintf( fout,"\n" );
	}
	continue;
      }

      /* Here, we've got better candidates
	 for the cone of largest error
      */

      /* special case:
	 there is only one cone
      */
      iv = 2;
      if ( iv == wvl.n_vectors ) {
#ifndef WIN32
          ct = times( &timestruct );
#endif
	_add_weights_to_list( wl, &wvl,  err, dmin, dmax, eps, ct, it );
	if ( _verbose_ >= 2 ) {
	  _print_cone_information( fout, erropt, perr, err, dmin, dmax, eps, ct, it );
	  _print_2D_weighted_vector_list( fout, &wvl, 2 );
	}
	perr = err;
	continue;
      }

      cl->allocatedCones[0].flag = 1;

      /* here we have more work to do
	 compute the bounds for the other vectors
	 from the error of the first cone
       */
      for ( i=2, ok=1; i<wvl.n_vectors && ok==1; i++ ) {

	v[i]->f = (int)(floor( eps*(1-perr)*v[i]->vec->vn ) + 0.5 );
	v[i]->l = (int)(ceil(  eps*(1+perr)*v[i]->vec->vn ) + 0.5 );

	if ( v[i]->f <= 0 ) v[i]->f = 1;
	if ( v[i]->l > (int)(l * _norme1v_2D( v[i]->vec )) ) 
	  v[i]->l = (int)(l * _norme1v_2D( v[i]->vec ));
	
	v[i]->w = v[i]->f-1;

	if ( _verbose_ >= 4 ) {
	  fprintf( stdout, "  bounds #%2d: %3d -> %3d\n", i,
             v[i]->f, v[i]->l );
	}
	if ( v[i]->l < v[i]->f ) ok = 0;
      }

      if ( ok == 0 ) {
	if ( _trace_ ) 
	  fprintf( fout,"%3d %3d empty intervals\n", v[0]->w, v[1]->w );
	if ( _verbose_ >= 4 ) {
	  fprintf( stdout, "  empty intervals\n" );
	}
	continue;
      }


      /* bounds have been computed
	 we have to find a weight set that is optimal
       */
      do {
	
	/* find a weight that satisfy conditions
	   for next vector
	*/
	do {
	  v[iv]->w ++;
	  if ( _trace_ ) {
	    if ( v[iv]->w == v[iv]->f ) {
	      fprintf( fout,"%3d %3d", v[0]->w, v[1]->w );
	      for (i=2; i<=iv; i++ )
          fprintf( fout," %3d", v[i]->w );
	      fprintf( fout,"   [%d %d]\n", v[iv]->f, v[iv]->l );
	    }
	  }
	  if ( v[iv]->w <= v[iv]->l ) {
	    v[iv]->flag = 1;
	    ok = 1;
	    /* look at the cones that share the vector
	       we work with
	    */
	    for ( j = 0; j<v[iv]->n_cones && ok == 1; j++ ) {
	      cone = v[iv]->c[j];
	      if ( cone->flag == 1 ) continue;
	      if ( (&(wvl.allocatedVectors[cone->iv1]) == v[iv] 
              && wvl.allocatedVectors[cone->iv2].flag == 1)
             || (wvl.allocatedVectors[cone->iv1].flag == 1
                 && &(wvl.allocatedVectors[cone->iv2]) == v[iv] ) ) {

          ok = ok && _check_2D_cone_convexity( &wvl, cl, cone );

          if ( _trace_ && ok == 0 ) {
              fprintf( fout,"%3d %3d", v[0]->w, v[1]->w );
              for (i=2; i<=iv; i++ )
                fprintf( fout," %3d", v[i]->w );
              fprintf( fout,"   not maximal/convex\n" );
          }
          if ( ok == 1 ) cone->flag = 1;
	      }
	    }
	  }
	} while ( v[iv]->w <= v[iv]->l && ok == 0 );

	/* if not possible
	   get back to previous vector
	*/
	if ( v[iv]->w > v[iv]->l ) {
	  if ( _verbose_ >= 4 ) {
	    fprintf( stdout, "   TEST FAILURE " );
	    for (j=2; j<iv; j++ ) {
	      fprintf( stdout, "#%d=%d ", j, v[j]->w );
	    }
	    fprintf( stdout, "#%d=\n", iv );
	  }
	  v[iv]->w = v[iv]->f-1;
	  for ( j = 0; j<v[iv]->n_cones; j++ )
	    v[iv]->c[j]->flag = 0;
	  v[iv]->flag = 0;
	  iv --;
	}
	/* or get back to next
	 */
	else {
	  if ( iv == wvl.n_vectors-1 ) {
	    /* we got a mask
	     */
	    if ( _verbose_ >= 4 ) {
	      fprintf( stdout, "   TEST SUCCESS " );
	      for (j=2; j<=iv; j++ ) {
          fprintf( stdout, "#%d=%d ", j, v[j]->w );
	      }
	      fprintf( stdout, "\n" );
	    }
	    _compute_2D_mask_min_max( &wvl, cl, &ldmin, &ldmax );
	    leps = (ldmin + ldmax)/2.0;
	    lerr = ldmax/leps - 1;
	    
	    if ( 1.0001 * lerr < perr ) {
#ifndef WIN32
	      ct = times( &timestruct );
#endif
	      _add_weights_to_list( wl, &wvl,  lerr, ldmin, ldmax, leps, ct, it );
	      if ( _verbose_ >= 2 ) {
          _print_mask_information( fout, erropt, perr,
                       err, dmin, dmax,
                       lerr, ldmin, ldmax, leps, ct, it );
          _print_2D_weighted_vector_list( fout, &wvl, wvl.n_vectors );
	      }
	      perr = lerr;
	    }
	  }
	  else {
	    v[iv]->flag = 1;
	    iv ++;
	  }
	}

      } while ( iv > 1 );

      
      /* perr = err; */

    } /* for v[1] */
  }   /* for v[0] */

  _free_weighted_vector_list( &wvl );
  free( v );
  return( wl->n_weights );
}










void _sort_3D_vectors( WeightedVector **v, WeightedVectorList *vl, ConeList *cl )
{
  char *proc = "_sort_3D_vectors";
  int i, j, n;
  int t;
  int *tab = NULL;

  for ( i=0; i<vl->n_vectors; i++ ) 
    vl->allocatedVectors[i].flag = 0;
  for ( i=0; i<cl->n_cones; i++ ) 
    cl->allocatedCones[i].flag = 0;


  tab = (int*)malloc( vl->n_vectors * sizeof(int) );
  if ( tab == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    exit( 0 );
  }
  /* first cone
   */

  switch( cl->allocatedCones[0].maximumType ) {
  case _UNSET_ :
    fprintf( stderr, "%s: the first cone has no type\n", proc );
  case _INSIDE_ :
    if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv2].vec )
       < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv3].vec ) ) {
    if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv1].vec ) 
	 < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv2].vec ) ) {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
      v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
    }
    else {
      if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv1].vec ) 
	   < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv3].vec ) ) {
	v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
	v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
	v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
      }
      else {
	v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
	v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
	v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
      }
    }
  }
  else {
    if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv1].vec ) 
	 < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv3].vec ) ) {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
      v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
    }
    else {
      if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv1].vec ) 
	   < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv2].vec ) ) {
	v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
	v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
	v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
      }
      else {
	v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
	v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
	v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
      }
    }
  }
  break;
  case _EDGE_IJ_ :
    if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv1].vec ) 
	 < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv2].vec ) ) {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
    }
    else {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
    }
    v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
    break;
  case _EDGE_JK_ :
    if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv2].vec )
	 < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv3].vec ) ) {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
    }
    else {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
    }
    v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
    break;
  case _EDGE_KI_ :
    if ( _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv1].vec ) 
	 < _norme2v_3D( vl->allocatedVectors[cl->allocatedCones[0].iv3].vec ) ) {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
    }
    else {
      v[0] = &(vl->allocatedVectors[cl->allocatedCones[0].iv3]);
      v[1] = &(vl->allocatedVectors[cl->allocatedCones[0].iv1]);
    }
    v[2] = &(vl->allocatedVectors[cl->allocatedCones[0].iv2]);
    break;
  }

  cl->allocatedCones[0].flag = 1;
  v[0]->flag = 1;
  v[1]->flag = 1;
  v[2]->flag = 1;

  n = 3;


  /* let's go for the other vectors
   */

  for ( ; n < vl->n_vectors;  ) {

    t = 0;
    /* find adjacents cones 
     */

    for ( i=1; i<cl->n_cones; i++ ) {

    /* skip marked cones
     */

      if ( cl->allocatedCones[i].flag == 1 ) continue;
      if ( vl->allocatedVectors[cl->allocatedCones[i].iv1].flag == 1 &&
	   vl->allocatedVectors[cl->allocatedCones[i].iv2].flag == 1 &&
	   vl->allocatedVectors[cl->allocatedCones[i].iv3].flag == 1 ) {
	cl->allocatedCones[i].flag = 1;
	continue;
      }

      /* try to find a cone with two marked vertices
	 ie with only one unmarked vertex
      */
      
      if ( (vl->allocatedVectors[cl->allocatedCones[i].iv1].flag == 0 
	    && vl->allocatedVectors[cl->allocatedCones[i].iv2].flag == 0)
	   || (vl->allocatedVectors[cl->allocatedCones[i].iv2].flag == 0 
	       && vl->allocatedVectors[cl->allocatedCones[i].iv3].flag == 0)
	   || (vl->allocatedVectors[cl->allocatedCones[i].iv3].flag == 0 
	       && vl->allocatedVectors[cl->allocatedCones[i].iv1].flag == 0) )
	continue;

      tab[t] = i;
      t++;
    }

    /* select the cone from the candidates
     */
    switch ( t ) {
    case 0 :
      /* this should not occur
       */
      j = 0 ;
      break;
    case 1 :
      /* take the first cone in the list
       */
      j = tab[0];
      break;
    default :
      /* take the cone of minimal error in the list
       */
      for ( j=tab[0], i=1; i<t; i++ ) 
	if ( cl->allocatedCones[j].error > cl->allocatedCones[tab[i]].error )
	  j = tab[i];
    }
    

    /* add the unmarked vertex into the sorted
       list of vectors
    */

    if ( j > 0 ) {
      cl->allocatedCones[j].flag = 1;
      if ( vl->allocatedVectors[cl->allocatedCones[j].iv1].flag == 0 ) {
	v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv1]);
	v[n]->flag = 1;
	n++;
      }
      if ( vl->allocatedVectors[cl->allocatedCones[j].iv2].flag == 0 ) {
	v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv2]);
	v[n]->flag = 1;
	n++;
      }
      if ( vl->allocatedVectors[cl->allocatedCones[j].iv3].flag == 0 ) {
	v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv3]);
	v[n]->flag = 1;
	n++;
      }
      continue;
    }

    /* do not find a cone with two vectors already in list
     */
    
    fprintf( stderr, "%s: ERROR, looking for a cone with one marked vertex\n", 
	     proc );
    
    for ( j=0, i=1; i<cl->n_cones && j==0; i++ ) {
      if ( cl->allocatedCones[i].flag == 1 ) continue;
      if ( vl->allocatedVectors[cl->allocatedCones[i].iv1].flag == 0 
	   && vl->allocatedVectors[cl->allocatedCones[i].iv2].flag == 0 
	   && vl->allocatedVectors[cl->allocatedCones[i].iv3].flag == 0 )
	continue;
      j = i;
    }
    
    if ( j > 0 ) {

      if ( vl->allocatedVectors[cl->allocatedCones[j].iv1].flag == 1 ) {
	if ( vl->allocatedVectors[cl->allocatedCones[j].iv2].vec->vn 
	     < vl->allocatedVectors[cl->allocatedCones[j].iv3].vec->vn ) {
	  v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv2]);
	  v[n]->flag = 1;
	  n++;
	}
	else {
	  v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv3]);
	  v[n]->flag = 1;
	  n++;
	}
      }
      
      if ( vl->allocatedVectors[cl->allocatedCones[j].iv2].flag == 1 ) {
	if ( vl->allocatedVectors[cl->allocatedCones[j].iv1].vec->vn 
	     < vl->allocatedVectors[cl->allocatedCones[j].iv3].vec->vn ) {
	  v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv1]);
	  v[n]->flag = 1;
	  n++;
	}
	else {
	  v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv3]);
	  v[n]->flag = 1;
	  n++;
	}
      }
      
       if ( vl->allocatedVectors[cl->allocatedCones[j].iv3].flag == 1 ) {
	if ( vl->allocatedVectors[cl->allocatedCones[j].iv1].vec->vn 
	     < vl->allocatedVectors[cl->allocatedCones[j].iv2].vec->vn ) {
	  v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv1]);
	  v[n]->flag = 1;
	  n++;
	}
	else {
	  v[n] = &(vl->allocatedVectors[cl->allocatedCones[j].iv2]);
	  v[n]->flag = 1;
	  n++;
	}
      }
    }

  }

  free( tab );
}




int _check_4vectors_convexity( WeightedVector *v1, WeightedVector *v2, WeightedVector *v3, WeightedVector *v4 )
{
  int mat[4][4];
  
  mat[0][0] = v1->vec->x; mat[0][1] = v1->vec->y; mat[0][2] = v1->vec->z; mat[0][3] = v1->w;
  mat[1][0] = v2->vec->x; mat[1][1] = v2->vec->y; mat[1][2] = v2->vec->z; mat[1][3] = v2->w;
  mat[2][0] = v3->vec->x; mat[2][1] = v3->vec->y; mat[2][2] = v3->vec->z; mat[2][3] = v3->w;
  mat[3][0] = v4->vec->x; mat[3][1] = v4->vec->y; mat[3][2] = v4->vec->z; mat[3][3] = v4->w;
  if ( _4x4_determinant( mat ) < 0 )
    return( 0 );
  return( 1 );
}



int _check_3D_cone_convexity_wrt_edge( WeightedVectorList *vl, ConeList *cl, double *s,
                           int iv1, int iv2, int iv3, char *desc )
{
  WeightedVector *v1, *v2, *v3, *v4 = NULL;
  int adjcone;
  LocalisationType adjside;
  Vector sym;
  WeightedVector wsym;

  v1 = &(vl->allocatedVectors[ iv1 ]);
  v2 = &(vl->allocatedVectors[ iv2 ]);
  v3 = &(vl->allocatedVectors[ iv3 ]);

  if ( v1->flag != 1 || v2->flag != 1 || v3->flag != 1 )
    return( -1 );
    
  adjcone = _search_adjacent_3D_cone( cl, iv2, iv3, iv1, &adjside );

  if ( adjcone >= 0 ) {

    /* there exist an adjacent cone, get the vector 
    */
    switch ( adjside ) {
    default :
      break;
    case _EDGE_IJ_ :
      v4 = &(vl->allocatedVectors[ cl->allocatedCones[adjcone].iv3 ]);
      break;
    case _EDGE_JK_ :
      v4 = &(vl->allocatedVectors[ cl->allocatedCones[adjcone].iv1 ]);
      break;
    case _EDGE_KI_ :
      v4 = &(vl->allocatedVectors[ cl->allocatedCones[adjcone].iv2 ]);
      break;
    }

  }
  else {

    /* no adjacent cone, symmetrise cone wrt JK
     */
    _build_3D_symmetric_vector_wrt_vectors( &sym, v1->vec, v2->vec, v3->vec, s );
    wsym.w = v1->w;
    wsym.vec = &sym;
    wsym.flag = v1->flag;
    v4 = &wsym;
    
    if ( _debug_ ) {
      fprintf( stdout, 
	       "SYM (%s) of (%d %d %d) wrt (%d %d %d),(%d %d %d) is (%d %d %d)\n",
	       desc,
	       v1->vec->x, v1->vec->y, v1->vec->z,
	       v2->vec->x, v2->vec->y, v2->vec->z,
	       v3->vec->x, v3->vec->y, v3->vec->z,
	       sym.x, sym.y, sym.z );
    }
    
  }

  if ( v4->flag == 1 ) {
    if ( _check_4vectors_convexity( v1, v2, v3, v4 ) == 0 ) {
      if ( _debug_ ) {
	fprintf( stdout, "convexity: unsuccessful convexity test (%s)\n", desc );
	fprintf( stdout, "           vector (inside) was (%2d %2d %2d)\n", v4->vec->x, v4->vec->y, v4->vec->z );
      }
      return( 0 );
    }
    if ( _debug_ ) 
      fprintf( stdout, "convexity: successful convexity test (%s)\n", desc );
  }
  else {
    if ( _debug_ ) 
      fprintf( stdout, "convexity: unchecked convexity test (%s)\n", desc );
  }

  return( 1 );
}



int _check_3D_cone_convexity( WeightedVectorList *vl, ConeList *cl, double *s, Cone *c )
{
  WeightedVector *v1, *v2, *v3;

  v1 = &(vl->allocatedVectors[ c->iv1 ]);
  v2 = &(vl->allocatedVectors[ c->iv2 ]);
  v3 = &(vl->allocatedVectors[ c->iv3 ]);

  if ( v1->flag != 1 || v2->flag != 1 || v3->flag != 1 )
    return( -1 );

  /* check convexity for JK 
   */
  if ( _check_3D_cone_convexity_wrt_edge( vl, cl, s, c->iv1, c->iv2, c->iv3, "JK" ) != 1 ) 
    return( 0 );
  
  /* check convexity for KI 
   */
  if ( _check_3D_cone_convexity_wrt_edge( vl, cl, s, c->iv2, c->iv3, c->iv1, "KI" ) != 1 ) 
    return( 0 );
    
  /* check convexity for IJ
   */
  if ( _check_3D_cone_convexity_wrt_edge( vl, cl, s, c->iv3, c->iv1, c->iv2, "IJ" ) != 1 ) 
    return( 0 );
    
  return( 1 );

}



void _compute_3D_cone_max( WeightedVector *v1, WeightedVector *v2, WeightedVector *v3, 
               double *dmax )
{
  char *proc = "_compute_3D_cone_max";
  double u[3], n;
  double v12[3], v23[3], v31[3], d[3];
  double uv12, uv23, uv31;

  u[0] = v1->w*(v2->vec->vy*v3->vec->vz - v2->vec->vz*v3->vec->vy)
       + v2->w*(v3->vec->vy*v1->vec->vz - v3->vec->vz*v1->vec->vy)
       + v3->w*(v1->vec->vy*v2->vec->vz - v1->vec->vz*v2->vec->vy);
  u[1] = v1->w*(v2->vec->vz*v3->vec->vx - v2->vec->vx*v3->vec->vz)
       + v2->w*(v3->vec->vz*v1->vec->vx - v3->vec->vx*v1->vec->vz)
       + v3->w*(v1->vec->vz*v2->vec->vx - v1->vec->vx*v2->vec->vz);
  u[2] = v1->w*(v2->vec->vx*v3->vec->vy - v2->vec->vy*v3->vec->vx)
       + v2->w*(v3->vec->vx*v1->vec->vy - v3->vec->vy*v1->vec->vx)
       + v3->w*(v1->vec->vx*v2->vec->vy - v1->vec->vy*v2->vec->vx);
	
  v23[0] = v2->vec->vy * v3->vec->vz - v2->vec->vz * v3->vec->vy;
  v23[1] = v2->vec->vz * v3->vec->vx - v2->vec->vx * v3->vec->vz;
  v23[2] = v2->vec->vx * v3->vec->vy - v2->vec->vy * v3->vec->vx;

  v31[0] = v3->vec->vy * v1->vec->vz - v3->vec->vz * v1->vec->vy;
  v31[1] = v3->vec->vz * v1->vec->vx - v3->vec->vx * v1->vec->vz;
  v31[2] = v3->vec->vx * v1->vec->vy - v3->vec->vy * v1->vec->vx;

  v12[0] = v1->vec->vy * v2->vec->vz - v1->vec->vz * v2->vec->vy;
  v12[1] = v1->vec->vz * v2->vec->vx - v1->vec->vx * v2->vec->vz;
  v12[2] = v1->vec->vx * v2->vec->vy - v1->vec->vy * v2->vec->vx;

  uv12 = u[0] * v12[0] + u[1] * v12[1] + u[2] * v12[2];
  uv23 = u[0] * v23[0] + u[1] * v23[1] + u[2] * v23[2];
  uv31 = u[0] * v31[0] + u[1] * v31[1] + u[2] * v31[2];


  if ( uv12 >= 0 ) {
    if ( uv23 >= 0 ) {
      if ( uv31 >= 0 ) {

	*dmax = sqrt( u[0]*u[0] + u[1]*u[1] + u[2]*u[2] );
  
	n = v1->vec->vx * v2->vec->vy * v3->vec->vz
	  + v1->vec->vy * v2->vec->vz * v3->vec->vx
	  + v1->vec->vz * v2->vec->vx * v3->vec->vy
	  - v1->vec->vx * v2->vec->vz * v3->vec->vy
	  - v1->vec->vy * v2->vec->vx * v3->vec->vz
	  - v1->vec->vz * v2->vec->vy * v3->vec->vx;
	*dmax /= n;

      }
      else {
	d[0] = v1->w * v3->vec->vx - v3->w * v1->vec->vx;
	d[1] = v1->w * v3->vec->vy - v3->w * v1->vec->vy;
	d[2] = v1->w * v3->vec->vz - v3->w * v1->vec->vz;
	*dmax = sqrt( (d[0]*d[0]+d[1]*d[1]+d[2]*d[2])
                / (v31[0]*v31[0]+v31[1]*v31[1]+v31[2]*v31[2]) );
      }
    }
    else {
      if ( uv31 >= 0 ) {
	d[0] = v3->w * v2->vec->vx - v2->w * v3->vec->vx;
	d[1] = v3->w * v2->vec->vy - v2->w * v3->vec->vy;
	d[2] = v3->w * v2->vec->vz - v2->w * v3->vec->vz;
	*dmax = sqrt( (d[0]*d[0]+d[1]*d[1]+d[2]*d[2])
                / (v23[0]*v23[0]+v23[1]*v23[1]+v23[2]*v23[2]) );
      }
      else {
	*dmax = v3->w / v3->vec->vn;	
      }
    }
  }
  else {
    if ( uv23 >= 0 ) {
      if ( uv31 >= 0 ) {
	d[0] = v2->w * v1->vec->vx - v1->w * v2->vec->vx;
	d[1] = v2->w * v1->vec->vy - v1->w * v2->vec->vy;
	d[2] = v2->w * v1->vec->vz - v1->w * v2->vec->vz;
	*dmax = sqrt( (d[0]*d[0]+d[1]*d[1]+d[2]*d[2])
                / (v12[0]*v12[0]+v12[1]*v12[1]+v12[2]*v12[2]) );
      }
      else {
	*dmax = v1->w / v1->vec->vn;	
      }
    }
    else {
      if ( uv31 >= 0 ) {
	*dmax = v2->w / v2->vec->vn;	
      }
      else {
	fprintf( stdout, "%s: this case should never occur\n", proc );
      }
    }
  }
  
}



void _compute_3D_cone_min_max( WeightedVector *v1, WeightedVector *v2, WeightedVector *v3, 
                   double *dmin, double *dmax )
{
  *dmin = v1->w / v1->vec->vn;
  if ( *dmin > v2->w / v2->vec->vn ) *dmin = v2->w / v2->vec->vn;
  if ( *dmin > v3->w / v3->vec->vn ) *dmin = v3->w / v3->vec->vn;

  _compute_3D_cone_max( v1, v2, v3, dmax );
}



void _compute_3D_mask_min_max( WeightedVectorList *vl, ConeList *cl, 
                   double *dmin, double *dmax )
{
  int i;
  WeightedVector *v1, *v2, *v3;
  double ldmax;
  
  *dmin = vl->allocatedVectors[0].w / vl->allocatedVectors[0].vec->vn;
  
  for ( i=1; i<vl->n_vectors; i++ ) 
    if ( *dmin > vl->allocatedVectors[i].w / vl->allocatedVectors[i].vec->vn )
      *dmin = vl->allocatedVectors[i].w / vl->allocatedVectors[i].vec->vn;

  *dmax = *dmin;
  for ( i=0; i<cl->n_cones; i++ ) {
    v1 = &(vl->allocatedVectors[ cl->allocatedCones[i].iv1 ]);
    v2 = &(vl->allocatedVectors[ cl->allocatedCones[i].iv2 ]);
    v3 = &(vl->allocatedVectors[ cl->allocatedCones[i].iv3 ]);

    _compute_3D_cone_max( v1, v2, v3, &ldmax );
    if ( *dmax < ldmax ) *dmax = ldmax;

  }
}





int _compute_3D_weights( FILE *fout, WeightsList *wl, 
              VectorList *vl, ConeList *cl,
              double *s, int f, int l )
{
  char *proc = "_compute_3D_weights";
  WeightedVectorList wvl;
  WeightedVector **v;
  int i, j;
  double dmin, dmax, eps;
  double perr=1.0;
  double erropt, err;
  int iv;

  int ok;
  Cone * cone;
  double ldmin, ldmax, lerr, leps;

  double v12[3], v23[3], v31[3];
#ifndef WIN32
  struct tms timestruct;
#endif

  double v12_12, v12_23, v12_31, v23_23, v23_31, v31_31;
  double it = 0, ct = 0;




  /* weighted vectors
   */
  _init_weighted_vector_list( &wvl );
  _init_weighted_vector_list_from_vector_list( &wvl, vl );


  /* add links from vectors to cones
   */
  (void)_link_cones_to_vectors( &wvl, cl );

  /* allocate and sort the vectors
   */
  v = (WeightedVector**)malloc( vl->n_vectors * sizeof(WeightedVector*) );
  if ( v == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }
  _sort_3D_vectors( v, &wvl, cl );

  if ( _trace_ || _debug_ ) {
    for ( i=0; i<vl->n_vectors; i++ ) {
      fprintf( fout, "v[#%3d] = ", i );
      _print_3D_vector( fout, v[i]->vec );
      fprintf( fout, "\n" );
    }
  }

  /* initialization
     flag = 0 -> no set yet
     flag = 1 -> already set
  */

  for ( i=0; i<vl->n_vectors; i++ ) 
    wvl.allocatedVectors[i].flag = 0;
  for ( i=0; i<cl->n_cones; i++ ) 
    cl->allocatedCones[i].flag = 0;


  /* some vectorial and scalar products
   */
  v23[0] = v[1]->vec->vy * v[2]->vec->vz - v[1]->vec->vz * v[2]->vec->vy;
  v23[1] = v[1]->vec->vz * v[2]->vec->vx - v[1]->vec->vx * v[2]->vec->vz;
  v23[2] = v[1]->vec->vx * v[2]->vec->vy - v[1]->vec->vy * v[2]->vec->vx;

  v31[0] = v[2]->vec->vy * v[0]->vec->vz - v[2]->vec->vz * v[0]->vec->vy;
  v31[1] = v[2]->vec->vz * v[0]->vec->vx - v[2]->vec->vx * v[0]->vec->vz;
  v31[2] = v[2]->vec->vx * v[0]->vec->vy - v[2]->vec->vy * v[0]->vec->vx;

  v12[0] = v[0]->vec->vy * v[1]->vec->vz - v[0]->vec->vz * v[1]->vec->vy;
  v12[1] = v[0]->vec->vz * v[1]->vec->vx - v[0]->vec->vx * v[1]->vec->vz;
  v12[2] = v[0]->vec->vx * v[1]->vec->vy - v[0]->vec->vy * v[1]->vec->vx;
  
  v12_12 = v12[0]*v12[0] + v12[1]*v12[1] + v12[2]*v12[2];
  v12_23 = v12[0]*v23[0] + v12[1]*v23[1] + v12[2]*v23[2];
  v12_31 = v12[0]*v31[0] + v12[1]*v31[1] + v12[2]*v31[2];

  v23_23 = v23[0]*v23[0] + v23[1]*v23[1] + v23[2]*v23[2];
  v23_31 = v23[0]*v31[0] + v23[1]*v31[1] + v23[2]*v31[2];

  v31_31 = v31[0]*v31[0] + v31[1]*v31[1] + v31[2]*v31[2];


  /* the largest error
   */
  erropt = cl->allocatedCones[0].error;


  /* set bounds for the vectors of the first cone
     low bound = f
     high bound = l * |v|_1
   */
  v[0]->f = floor(f * _normeIv_3D( v[0]->vec ) );
  v[0]->l = (int)(l * _norme1v_3D( v[0]->vec ) );

  v[1]->f = floor(f * _normeIv_3D( v[1]->vec ) );
  v[1]->l = (int)(l * _norme1v_3D( v[1]->vec ) );

  v[2]->f = floor(f * _normeIv_3D( v[2]->vec ) );
  v[2]->l = (int)(l * _norme1v_3D( v[2]->vec ) );

  v[0]->flag = 1;
  v[1]->flag = 1;
  v[2]->flag = 1;

#ifndef WIN32
  it = times( &timestruct );
#endif


  /* loops for the first cone
   */
  for ( v[0]->w  = v[0]->f;
	v[0]->w <= v[0]->l;
	v[0]->w ++ ) {

    if ( _trace_ )
      fprintf( fout,"%3d   [%d %d]\n", v[0]->w, v[0]->f, v[0]->l );



    for ( v[1]->w  = v[1]->f;
	  v[1]->w <= v[1]->l;
	  v[1]->w ++ ) {
      
      if ( _trace_ )
	fprintf( fout,"%3d %3d   [%d %d]\n", v[0]->w, v[1]->w, v[1]->f, v[1]->l );
      
      
  
      for ( v[2]->w  = v[2]->f;
	    v[2]->w <= v[2]->l;
	    v[2]->w ++ ) {

	if ( _trace_ )
	  fprintf( fout,"%3d %3d %3d   [%d %d]\n", 
             v[0]->w, v[1]->w, v[2]->w, v[2]->f, v[2]->l );

	/* check condition
	 */
	if ( _check_3D_cone_convexity( &wvl, cl, s, &(cl->allocatedCones[0]) ) == 0 ) {
	  if ( _trace_ )
	    fprintf( fout, " cone #0 not convex\n" );
	}

	
	/* compute min and max distance, epsilon and error
	 */      
	_compute_3D_cone_min_max( &(wvl.allocatedVectors[cl->allocatedCones[0].iv1]),
                      &(wvl.allocatedVectors[cl->allocatedCones[0].iv2]),
                      &(wvl.allocatedVectors[cl->allocatedCones[0].iv3]),
                      &dmin, &dmax );

	eps = (dmin + dmax)/2.0;
	err = dmax/eps - 1;

	/* if we do not perform better,
	   go to next
	*/
	if ( err >= perr ) {
	  if ( _trace_ ) 
	    fprintf( fout,"%3d %3d %3d too large error\n", v[0]->w, v[1]->w, v[2]->w );
	  continue;
	}
	
	/* Here, we've got better candidates
	   for the cone of largest error
	*/

	/* special case:
	   there is only one cone
	*/
	iv = 3;
	if ( iv == wvl.n_vectors ) {
#ifndef WIN32
          ct = times( &timestruct );
#endif
	  _add_weights_to_list( wl, &wvl,  err, dmin, dmax, eps, ct, it );
	  if ( _verbose_ >= 2 ) {
	    _print_cone_information( fout, erropt, perr, err, dmin, dmax, eps, ct, it );
	    _print_3D_weighted_vector_list( fout, &wvl, 3 );
	  }
	  perr = err;
	  continue;
	}
	
	if ( _verbose_ >= 4 ) {
	  fprintf( fout, "\n" );
	  fprintf( fout, "* v[0]=%d in [%d %d] v[1]=%d in [%d %d] v[2]=%d in [%d %d]\n", 
             v[0]->w, v[0]->f, v[0]->l,
             v[1]->w, v[1]->f, v[1]->l,
             v[2]->w, v[2]->f, v[2]->l );
	}

	cl->allocatedCones[0].flag = 1;

	/* here we have more work to do
	   compute the bounds for the other vectors
	   from the error of the first cone
	*/
	for ( i=3, ok=1; i<wvl.n_vectors && ok==1; i++ ) {

	  v[i]->f = (int)(floor( eps*(1-perr)*v[i]->vec->vn ) + 0.5 );
	  v[i]->l = (int)(ceil(  eps*(1+perr)*v[i]->vec->vn ) + 0.5 );
	  
	  if ( v[i]->f <= 0 ) v[i]->f = 1;
	  if ( v[i]->l > (int)(l * _norme1v_3D( v[i]->vec )) ) 
	    v[i]->l = (int)(l * _norme1v_3D( v[i]->vec ));

	  v[i]->w = v[i]->f-1;

	  if ( _verbose_ >= 4 ) {
	    fprintf( stdout, "  bounds #%2d: %3d -> %3d\n", i,
               v[i]->f, v[i]->l );
	  }
	  if ( v[i]->l < v[i]->f ) ok = 0;
	}

	if ( ok == 0 ) {
	  if ( _trace_ ) 
	    fprintf( fout,"%3d %3d %3d empty interval\n", v[0]->w, v[1]->w, v[2]->w );
	  if ( _verbose_ >= 4 ) {
	    fprintf( stdout, "  empty interval\n" );
	  }
	  continue;
	}


	
	
	
	/* bounds have been computed
	   we have to find a weight set that is optimal
	*/
	do {
	  
	  /* find a weight that satisfy convexity conditions
	     for next vector
	  */
	  do {
	    v[iv]->w ++;
	    if ( _trace_ ) {
	      if ( v[iv]->w == v[iv]->f ) {
          fprintf( fout,"%3d %3d %3d", v[0]->w, v[1]->w, v[2]->w );
          for (i=3; i<=iv; i++ )
            fprintf( fout," %3d", v[i]->w );
          fprintf( fout,"   [%d %d]\n", v[iv]->f, v[iv]->l );
	      }
	    }
	    if ( v[iv]->w <= v[iv]->l ) {
	      v[iv]->flag = 1;
	      ok = 1;
	      for ( j = 0; j<v[iv]->n_cones && ok == 1; j++ ) {
          cone = v[iv]->c[j];
          if ( (&(wvl.allocatedVectors[cone->iv1]) == v[iv]
                && wvl.allocatedVectors[cone->iv2].flag == 1
                && wvl.allocatedVectors[cone->iv3].flag == 1)
               || (wvl.allocatedVectors[cone->iv1].flag == 1
             && &(wvl.allocatedVectors[cone->iv2]) == v[iv]
             && wvl.allocatedVectors[cone->iv3].flag == 1)
               || (wvl.allocatedVectors[cone->iv1].flag == 1
             && wvl.allocatedVectors[cone->iv2].flag == 1
             && &(wvl.allocatedVectors[cone->iv3]) == v[iv]) ) {

            ok = ok && _check_3D_cone_convexity( &wvl, cl, s, cone );

            if ( _trace_ && ok == 0 ) {
              fprintf( fout,"%3d %3d %3d", v[0]->w, v[1]->w, v[2]->w );
              for (i=3; i<=iv; i++ )
                fprintf( fout," %3d", v[i]->w );
              fprintf( fout,"   not maximal/convex\n" );
            }
          }
	      }
	    }
	    
	  } while ( v[iv]->w <= v[iv]->l && ok == 0 );
	  
	  
	  /* if not possible
	     get back to previous vector
	  */
	  if ( v[iv]->w > v[iv]->l ) {
	    if ( _verbose_ >= 4 ) {
	      fprintf( stdout, "   TEST FAILURE " );
	      for (j=3; j<iv; j++ ) {
          fprintf( stdout, "#%d=%d ", j, v[j]->w );
	      }
	      fprintf( stdout, "#%d=\n", iv );
	    }
	    v[iv]->w = v[iv]->f-1;
	    for ( j = 0; j<v[iv]->n_cones; j++ )
	      v[iv]->c[j]->flag = 0;
	    v[iv]->flag = 0;
	    iv --;
	  }
	  /* or get back to next
	   */
	  else {
	    if ( iv == wvl.n_vectors-1 ) {
	      /* we got a mask
	       */
	      if ( _verbose_ >= 4 ) {
          fprintf( stdout, "   TEST SUCCESS " );
          for (j=2; j<=iv; j++ ) {
            fprintf( stdout, "#%d=%d ", j, v[j]->w );
          }
          fprintf( stdout, "\n" );
	      }
	      _compute_3D_mask_min_max( &wvl, cl, &ldmin, &ldmax );
	      leps = (ldmin + ldmax)/2.0;
	      lerr = ldmax/leps - 1;
	      
	      if ( 1.0001 * lerr < perr ) {
#ifndef WIN32
          ct = times( &timestruct );
#endif
          _add_weights_to_list( wl, &wvl,  lerr, ldmin, ldmax, leps, ct, it );
          if ( _verbose_ >= 2 ) {
            _print_mask_information( fout, erropt, perr,
                         err, dmin, dmax,
                         lerr, ldmin, ldmax, leps, ct, it );
            _print_3D_weighted_vector_list( fout, &wvl, wvl.n_vectors );
          }
          perr = lerr;
	      }
	    }
	    else {
	      v[iv]->flag = 1;
	      iv ++;
	    }
	  }
	  
	  
	} while ( iv > 2 );





      
      } /* for v[2] */
    }   /* for v[0] */
  }     /* for v[0] */

  _free_weighted_vector_list( &wvl );
  free( v );
  return( wl->n_weights );
}
