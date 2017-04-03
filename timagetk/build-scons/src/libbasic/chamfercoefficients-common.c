/*************************************************************************
 * chamfercoefficients-common.c - computation of chamfer distance coefficients
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
 * MISC: monitoring
 *
 **************************************************/



static int _debug_   = 0;
static int _trace_   = 0;
static int _verbose_ = 0;



void _set_chamfer_debug( int d ) 
{
  if ( d >= 0 )
    _debug_ = d;
}

void _unset_chamfer_debug() 
{
  _debug_ = 0;
}



void _set_chamfer_trace( int t ) 
{
  if ( t >= 0 )
    _trace_ = t;
}

void _unset_chamfer_trace() 
{
  _trace_ = 0;
}



void _set_chamfer_verbose( int v ) 
{
  if ( v >= 0 )
    _verbose_ = v;
}

void _unset_chamfer_verbose() 
{
  _verbose_ = 0;
}










/**************************************************
 *
 * MISC: global variables
 *
 **************************************************/


static int _alloc_ = 100;
/*
static int _be_strict_ = 0;
*/
/*
static double _tiny_ = 1e-12;
*/










/**************************************************
 *
 * MISC: numerical procedures
 *
 **************************************************/





int _3x3_determinant( int mat[3][3] )
{
  return( mat[0][0]*mat[1][1]*mat[2][2]
	  + mat[0][1]*mat[1][2]*mat[2][0]
	  + mat[0][2]*mat[1][0]*mat[2][1]
	  - mat[2][0]*mat[1][1]*mat[0][2]
	  - mat[2][1]*mat[1][2]*mat[0][0]
	  - mat[2][2]*mat[1][0]*mat[0][1] );
}


int _4x4_determinant( int mat[4][4] )
{
  if ( _debug_ > 2 ) {
    fprintf( stdout, "\n" );
    fprintf( stdout, "[ %3d %3d %3d %3d ]\n",
	     mat[0][0], mat[1][0], mat[2][0], mat[3][0] );
    fprintf( stdout, "[ %3d %3d %3d %3d ]\n",
	     mat[0][1], mat[1][1], mat[2][1], mat[3][1] );
    fprintf( stdout, "[ %3d %3d %3d %3d ]\n",
	     mat[0][2], mat[1][2], mat[2][2], mat[3][2] );
    fprintf( stdout, "[ %3d %3d %3d %3d ]\n",
	     mat[0][3], mat[1][3], mat[2][3], mat[3][3] );
    fprintf( stdout, "\n" );
  }

  return( mat[0][0] * ( mat[1][1]*mat[2][2]*mat[3][3]
			+ mat[1][2]*mat[2][3]*mat[3][1]
			+ mat[1][3]*mat[2][1]*mat[3][2]
			- mat[3][1]*mat[2][2]*mat[1][3]
			- mat[3][2]*mat[2][3]*mat[1][1]
			- mat[3][3]*mat[2][1]*mat[1][2] )
	  - mat[1][0] * ( mat[0][1]*mat[2][2]*mat[3][3]
			+ mat[0][2]*mat[2][3]*mat[3][1]
			+ mat[0][3]*mat[2][1]*mat[3][2]
			- mat[3][1]*mat[2][2]*mat[0][3]
			- mat[3][2]*mat[2][3]*mat[0][1]
			- mat[3][3]*mat[2][1]*mat[0][2] )
	  + mat[2][0] * ( mat[0][1]*mat[1][2]*mat[3][3]
			+ mat[0][2]*mat[1][3]*mat[3][1]
			+ mat[0][3]*mat[1][1]*mat[3][2]
			- mat[3][1]*mat[1][2]*mat[0][3]
			- mat[3][2]*mat[1][3]*mat[0][1]
			- mat[3][3]*mat[1][1]*mat[0][2] )
	  - mat[3][0] * ( mat[0][1]*mat[1][2]*mat[2][3]
			+ mat[0][2]*mat[1][3]*mat[2][1]
			+ mat[0][3]*mat[1][1]*mat[2][2]
			- mat[2][1]*mat[1][2]*mat[0][3]
			- mat[2][2]*mat[1][3]*mat[0][1]
			- mat[2][3]*mat[1][1]*mat[0][2] ) );
}



double _4x4_doubledeterminant( double mat[4][4] )
{
  double r;

  r = mat[0][0] * ( mat[1][1]*mat[2][2]*mat[3][3]
			+ mat[1][2]*mat[2][3]*mat[3][1]
			+ mat[1][3]*mat[2][1]*mat[3][2]
			- mat[3][1]*mat[2][2]*mat[1][3]
			- mat[3][2]*mat[2][3]*mat[1][1]
			- mat[3][3]*mat[2][1]*mat[1][2] )
	  - mat[1][0] * ( mat[0][1]*mat[2][2]*mat[3][3]
			+ mat[0][2]*mat[2][3]*mat[3][1]
			+ mat[0][3]*mat[2][1]*mat[3][2]
			- mat[3][1]*mat[2][2]*mat[0][3]
			- mat[3][2]*mat[2][3]*mat[0][1]
			- mat[3][3]*mat[2][1]*mat[0][2] )
	  + mat[2][0] * ( mat[0][1]*mat[1][2]*mat[3][3]
			+ mat[0][2]*mat[1][3]*mat[3][1]
			+ mat[0][3]*mat[1][1]*mat[3][2]
			- mat[3][1]*mat[1][2]*mat[0][3]
			- mat[3][2]*mat[1][3]*mat[0][1]
			- mat[3][3]*mat[1][1]*mat[0][2] )
	  - mat[3][0] * ( mat[0][1]*mat[1][2]*mat[2][3]
			+ mat[0][2]*mat[1][3]*mat[2][1]
			+ mat[0][3]*mat[1][1]*mat[2][2]
			- mat[2][1]*mat[1][2]*mat[0][3]
			- mat[2][2]*mat[1][3]*mat[0][1]
			- mat[2][3]*mat[1][1]*mat[0][2] );

  if ( _debug_ > 2 ) {
    fprintf( stdout, "\n" );
    fprintf( stdout, "[ %f %f %f %f ]\n",
	     mat[0][0], mat[1][0], mat[2][0], mat[3][0] );
    fprintf( stdout, "[ %f %f %f %f ]\n",
	     mat[0][1], mat[1][1], mat[2][1], mat[3][1] );
    fprintf( stdout, "[ %f %f %f %f ]  =  %f \n",
	     mat[0][2], mat[1][2], mat[2][2], mat[3][2], r );
    fprintf( stdout, "[ %f %f %f %f ]\n",
	     mat[0][3], mat[1][3], mat[2][3], mat[3][3] );
    fprintf( stdout, "\n" );
  }

  return( r );
}











/**************************************************
 *
 * MISC: vectors
 *
 **************************************************/





void _init_vector( Vector *v ) 
{
  v->x = 0;
  v->y = 0;
  v->z = 0;

  v->vx = 0.0;
  v->vy = 0.0;
  v->vz = 0.0;
  
  v->vn = 0.0;  

  v->ux = 0.0;
  v->uy = 0.0;
  v->uz = 0.0;

  v->index = -1;
  v->w = -1;
}



void _copy_vector( Vector *r, Vector *v ) 
{
  r->x = v->x;
  r->y = v->y;
  r->z = v->z;

  r->vx = v->vx;
  r->vy = v->vy;
  r->vz = v->vz;
  
  r->vn = v->vn; 

  r->ux = v->ux;
  r->uy = v->uy;
  r->uz = v->uz;

  r->index = v->index;
  r->w = v->w;
}


void _compute_2D_vector_attributes( Vector *v, double *s ) 
{
  v->z  = 0;

  v->vx = s[0] * v->x;
  v->vy = s[1] * v->y;
  v->vz = 0.0;

  v->vn = sqrt( v->vx*v->vx + v->vy*v->vy );

  v->ux = v->vx / v->vn;
  v->uy = v->vy / v->vn;
  v->uz = 0.0;
}



void _compute_3D_vector_attributes( Vector *v, double *s ) 
{
  v->vx = s[0] * v->x;
  v->vy = s[1] * v->y;
  v->vz = s[2] * v->z;
  v->vn = sqrt( v->vx*v->vx + v->vy*v->vy + v->vz*v->vz );
  v->ux = v->vx / v->vn;
  v->uy = v->vy / v->vn;
  v->uz = v->vz / v->vn;
}



double _norme1v_2D( Vector *v )
{
  return( fabs(v->vx) + fabs(v->vy) );
}



double _normeIv_2D( Vector *v )
{
  if ( fabs(v->vx) > fabs(v->vy) )
    return( fabs(v->vx) );
  else
    return( fabs(v->vy) );
}



double _norme2v_2D( Vector *v )
{
  return( sqrt( v->vx * v->vx + v->vy * v->vy ) );
}



double _norme1v_3D( Vector *v )
{
  return( fabs(v->vx) + fabs(v->vy)  + fabs(v->vz) );
}



double _norme2v_3D( Vector *v )
{
  return( sqrt( v->vx * v->vx + v->vy * v->vy + v->vz * v->vz ) );
}



double _normeIv_3D( Vector *v )
{
  if ( fabs(v->vx) > fabs(v->vy) ) {
    if ( fabs(v->vx) > fabs(v->vz) )
      return( fabs(v->vx) );
    else
      return( fabs(v->vz) );
  }
  else {
    if ( fabs(v->vy) > fabs(v->vz) )
      return( fabs(v->vy) );
    else
      return( fabs(v->vz) );
  }
}



void _build_vector( Vector *v, int x, int y, int z, double *s )
{
  _init_vector( v );

  v->x = x;
  v->y = y;
  v->z = z;
  
  _compute_3D_vector_attributes( v, s );
}



void _build_3D_symmetric_vector_wrt_vector( Vector *res, Vector *v, 
					      Vector *normal, double *s )
{
  int n, ps;
  
  n = normal->x * normal->x + normal->y * normal->y + normal->z * normal->z;
  ps = v->x * normal->x + v->y * normal->y + v->z * normal->z;

  res->x = v->x - 2 * ps * normal->x / n;
  res->y = v->y - 2 * ps * normal->y / n;
  res->z = v->z - 2 * ps * normal->z / n;

  _compute_3D_vector_attributes( res, s );

  res->w = v->w;
}



void _build_3D_symmetric_vector_wrt_vectors( Vector *res, Vector *v, 
					       Vector *v1, Vector *v2, double *s )
{
  Vector normal;

  /* vector product
   */
  normal.x = v1->y * v2->z - v1->z * v2->y;
  normal.y = v1->z * v2->x - v1->x * v2->z;
  normal.z = v1->x * v2->y - v1->y * v2->x;
  _compute_3D_vector_attributes( &normal, s );
  
  _build_3D_symmetric_vector_wrt_vector( res, v, &normal, s );
}



void _sort_vectors_in_lexicographic_order( Vector **v, int left, int right )
{
  int i, last;
  Vector *t;
  int max, lmax;
  int sum, lsum;

  if ( left >= right ) return;

  t = v[left];   v[left] = v[(left+right)/2];   v[(left+right)/2] = t;
  
  last = left;

  lmax = v[left]->x;
  if ( lmax < v[left]->y) lmax = v[left]->y;
  if ( lmax < v[left]->z) lmax = v[left]->z;
  lsum = v[left]->x + v[left]->y + v[left]->z;

  for ( i = left+1; i <= right; i++ ) {
    max = v[i]->x;
    if ( max < v[i]->y) max = v[i]->y;
    if ( max < v[i]->z) max = v[i]->z;
    sum = v[i]->x + v[i]->y + v[i]->z;
    if ( (max < lmax) ||
	 (max == lmax && sum < lsum) ||
	 (max == lmax && sum == lsum
	  && ((v[i]->x > v[left]->x)
	      || (v[i]->x == v[left]->x && v[i]->y > v[left]->y)
	      || (v[i]->x == v[left]->x && v[i]->y == v[left]->y 
		  && v[i]->z > v[left]->z))) ) {
      t = v[++last];   v[last] = v[i];   v[i] = t;
    }
  }

  t = v[left];   v[left] = v[last];   v[last] = t;

  _sort_vectors_in_lexicographic_order( v, left, last-1 );
  _sort_vectors_in_lexicographic_order( v, last+1, right );
}












/**************************************************
 *
 * MISC: vectors, print
 *
 **************************************************/





void _print_2D_vector( FILE *f, Vector *v )
{
  fprintf( f, "(%2d %2d)", v->x, v->y );
}



void _print_3D_vector( FILE *f, Vector *v )
{
  fprintf( f, "(%2d %2d %2d) [%d]", v->x, v->y, v->z, v->w );
}










/**************************************************
 *
 * MISC: vector list
 *
 **************************************************/





void _init_vector_list( VectorList *vl )
{
  vl->allocatedVectors = NULL;
  vl->n_allocated_vectors = 0;
  vl->n_vectors = 0;
}



void _free_vector_list( VectorList *vl )
{
  if ( vl->allocatedVectors != NULL ) free( vl->allocatedVectors );
  _init_vector_list( vl );
}




int _add_vector_to_list( VectorList *vl, Vector *v )
{
  char *proc = "_add_vector_to_list";
  Vector *newv;
  int i, nalloc;
  
  /* look for the vector
   */
  for ( i=0; i<vl->n_vectors; i++ ) {
    if ( vl->allocatedVectors[i].x == v->x 
	 && vl->allocatedVectors[i].y == v->y 
	 && (v->z < 0 || vl->allocatedVectors[i].z == v->z) ) {
      return ( i );
    }
  }

  /* is allocated space sufficient ?
(   */
  if ( vl->n_vectors >= vl->n_allocated_vectors ) {

    if ( vl->n_allocated_vectors <= 0 ) 
      nalloc = _alloc_;
    else
      nalloc = vl->n_allocated_vectors + _alloc_;

    /* allocation
     */
    newv = (Vector*)malloc( nalloc * sizeof( Vector ) );
    if ( newv == NULL ) {
      fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
    }
    
    /* copy and/or initialization
     */
    if ( vl->n_allocated_vectors <= 0 ) {
      for ( i=0; i<nalloc; i++ ) {
	_init_vector( &(newv[i]) );
      }
    }
    else {
      memcpy( newv, vl->allocatedVectors, 
	      vl->n_allocated_vectors * sizeof( Vector ) );
      for ( i=vl->n_vectors; i<nalloc; i++ ) {
	_init_vector( &(newv[i]) );
      }
      free( vl->allocatedVectors );
    }

    vl->allocatedVectors = newv;
    vl->n_allocated_vectors = nalloc;
  }
  

  /* create the vector
   */
  vl->n_vectors ++;
  _copy_vector( &(vl->allocatedVectors[ vl->n_vectors-1 ]), v );

  return( vl->n_vectors-1 );

}













/**************************************************
 *
 * MISC: vector list, print
 *
 **************************************************/




void _print_2D_vector_list( FILE *f, VectorList *vl )
{
  int i;

  fprintf( f, "\n" );
  fprintf( f, "#VECTORS = %d", vl->n_vectors );
  fprintf( f, "\n" );
  for ( i=0; i<vl->n_vectors; i++ ) {
    fprintf( f, "v[#%3d] = ", i );
    _print_2D_vector( f, &(vl->allocatedVectors[i]) );
    /*
    fprintf( f, " c=%d", vl->allocatedVectors[i].n_cones );
    fprintf( f, " " );
    for ( j=0; j<vl->allocatedVectors[i].n_cones; j++ )
      _print_2D_cone( f, vl, vl->allocatedVectors[i].c[j] );
    */
    fprintf( f, "\n" );
  }
  fprintf( f, "\n" );
}



void _print_3D_vector_list( FILE *f, VectorList *vl )
{
  int i;

  fprintf( f, "\n" );
  fprintf( f, "#VECTORS = %d", vl->n_vectors );
  fprintf( f, "\n" );
  for ( i=0; i<vl->n_vectors; i++ ) {
    fprintf( f, "v[#%3d] = ", i );
    _print_3D_vector( f, &(vl->allocatedVectors[i]) );
    /*
    fprintf( f, " c=%d", vl->allocatedVectors[i].n_cones );
    fprintf( f, " " );
    for ( j=0; j<vl->allocatedVectors[i].n_cones; j++ ) {
      _print_3D_cone( f, vl, vl->allocatedVectors[i].c[j] );
      if ( j<vl->allocatedVectors[i].n_cones-1 && j>0 && (j+1)%2 == 0 )
	fprintf( f, "\n                        ");
    }
    */
    fprintf( f, "\n" );
  }
  fprintf( f, "\n" );
}










/**************************************************
 *
 * MISC: cones
 *
 **************************************************/





void _init_cone( Cone *c )
{
  c->iv1 = -1;
  c->iv2 = -1;
  c->iv3 = -1;
  
  c->maximumType = _UNSET_;

  c->convexity = _UNSET_;

  c->error  = 1.0;
}



void _copy_cone( Cone *r, Cone *c )
{
  r->iv1 = c->iv1;
  r->iv2 = c->iv2;
  r->iv3 = c->iv3;
  
  r->maximumType = c->maximumType;

  r->convexity = c->convexity;

  r->error = c->error;
}



void _compute_2D_cone_error( Cone *c, Vector *v1, Vector *v2 )
{
  double x, y, h;

  x = v1->ux + v2->ux;
  y = v1->uy + v2->uy;

  h = sqrt( x*x + y*y )/2;

  c->maximumType = _INSIDE_;
  c->error = (1-h)/(1+h);
}



void _compute_3D_cone_error( Cone *c, Vector *v1, Vector *v2, Vector *v3 )
{
  char *proc = "_compute_3D_cone_error";
  double sp12, sp23, sp31;
  double sign12, sign23, sign31;
  int nn = 0;
  double h, n, x, y, z;

  sp12 = v1->ux * v2->ux + v1->uy * v2->uy + v1->uz * v2->uz;

  sp23 = v2->ux * v3->ux + v2->uy * v3->uy + v2->uz * v3->uz;

  sp31 = v3->ux * v1->ux + v3->uy * v1->uy + v3->uz * v1->uz;

  sign12 = 1.0 + sp12 - sp23 - sp31;
  sign23 = 1.0 - sp12 + sp23 - sp31;
  sign31 = 1.0 - sp12 - sp23 + sp31;

  if ( sign12 < 0 ) nn++;
  if ( sign23 < 0 ) nn++;
  if ( sign31 < 0 ) nn++;

  if ( nn >= 2 ) {
    fprintf( stderr, "%s: two negative scalar products !?!\n", proc );
    fprintf( stderr, "    this should never occur\n" );
    exit( 0 );
  }

  if ( nn == 0 ) {
    c->maximumType = _INSIDE_;

    n = v1->ux * v2->uy * v3->uz - v1->ux * v2->uz * v3->uy
      + v1->uy * v2->uz * v3->ux - v1->uy * v2->ux * v3->uz
      + v1->uz * v2->ux * v3->uy - v1->uz * v2->uy * v3->ux;

    x = v2->uy * v3->uz - v2->uz * v3->uy + v3->uy * v1->uz 
      - v3->uz * v1->uy + v1->uy * v2->uz - v1->uz * v2->uy;
    
    y = v2->uz * v3->ux - v2->ux * v3->uz + v3->uz * v1->ux 
      - v3->ux * v1->uz + v1->uz * v2->ux - v1->ux * v2->uz;
    
    z = v2->ux * v3->uy - v2->uy * v3->ux + v3->ux * v1->uy 
      - v3->uy * v1->ux + v1->ux * v2->uy - v1->uy * v2->ux;

    h = n /sqrt( x*x + y*y + z*z );
    c->error = (1-h)/(1+h);
    return; 
  }

  if ( sign12 < 0 ) {
    c->maximumType = _EDGE_IJ_;

    x = v1->ux + v2->ux;
    y = v1->uy + v2->uy;
    z = v1->uz + v2->uz;

    h = sqrt( x*x + y*y + z*z )/2;
    c->error = (1-h)/(1+h);
    return;
  }

  if ( sign23 < 0 ) {
    c->maximumType = _EDGE_JK_;

    x = v2->ux + v3->ux;
    y = v2->uy + v3->uy;
    z = v2->uz + v3->uz;

    h = sqrt( x*x + y*y + z*z )/2;
    c->error = (1-h)/(1+h);
    return;
  }

  if ( sign31 < 0 ) {
    c->maximumType = _EDGE_KI_;

    x = v3->ux + v1->ux;
    y = v3->uy + v1->uy;
    z = v3->uz + v1->uz;

    h = sqrt( x*x + y*y + z*z )/2;
    c->error = (1-h)/(1+h);
    return;
  }

  fprintf( stderr, "%s: unexpected end\n", proc );
  fprintf( stderr, "    this should never occur\n" );
  exit( 0 );
}



void _build_2D_cone( VectorList *vl, Cone *c, int iv1, int iv2 )
{
  char *proc="_build_2D_cone";
  Vector *v1, *v2;
  
  _init_cone( c );

  c->iv1 = iv1;
  c->iv2 = iv2;
  c->iv3 = -1;

  v1 = &(vl->allocatedVectors[iv1]);
  v2 = &(vl->allocatedVectors[iv2]);

  if ( v1->x*v2->y - v1->y*v2->x < 0 ) {
    if ( _verbose_ >= 1  || _trace_ )
      fprintf( stderr, "%s: warning, negative vector product. Construction error?\n", proc );
    c->iv1 = iv2;
    c->iv2 = iv1;
  }

  _compute_2D_cone_error( c, &(vl->allocatedVectors[c->iv1]),
			  &(vl->allocatedVectors[c->iv2]) );
}



void _build_3D_cone( VectorList *vl, Cone *c, int iv1, int iv2, int iv3 )
{
  _init_cone( c );

  c->iv1 = iv1;
  c->iv2 = iv2;
  c->iv3 = iv3;

  _compute_3D_cone_error( c, &(vl->allocatedVectors[iv1]),
			  &(vl->allocatedVectors[iv2]),
			  &(vl->allocatedVectors[iv3]) );
}



int _check_4vectors_naturalconvexity( Vector *v1, Vector *v2, 
				      Vector *v3, Vector *v4 )
{
  double mat[4][4];
  
  mat[0][0] = v1->x; mat[0][1] = v1->y; mat[0][2] = v1->z; mat[0][3] = v1->vn;
  mat[1][0] = v2->x; mat[1][1] = v2->y; mat[1][2] = v2->z; mat[1][3] = v2->vn;
  mat[2][0] = v3->x; mat[2][1] = v3->y; mat[2][2] = v3->z; mat[2][3] = v3->vn;
  mat[3][0] = v4->x; mat[3][1] = v4->y; mat[3][2] = v4->z; mat[3][3] = v4->vn;
  if ( _4x4_doubledeterminant( mat ) < 0 )
    return( 0 );
  return( 1 );
}













/**************************************************
 *
 * MISC: cone, print
 *
 **************************************************/





void _print_2D_cone( FILE *f, VectorList *vl, Cone *c )
{
  fprintf( f, "[" );
  _print_2D_vector( f, &(vl->allocatedVectors[c->iv1]) );
  fprintf( f, " " );
  _print_2D_vector( f, &(vl->allocatedVectors[c->iv2]) );
  fprintf( f, "]" );
}



void _print_3D_cone( FILE *f, VectorList *vl, Cone *c )
{
  fprintf( f, "[" );
  _print_3D_vector( f, &(vl->allocatedVectors[c->iv1]) );
  fprintf( f, " " );
  _print_3D_vector( f, &(vl->allocatedVectors[c->iv2]) );
  fprintf( f, " " );
  _print_3D_vector( f, &(vl->allocatedVectors[c->iv3]) );
  fprintf( f, "]" );
}










/**************************************************
 *
 * MISC: cone list
 *
 **************************************************/





void _init_cone_list( ConeList *cl )
{
  cl->allocatedCones = NULL;
  cl->n_allocated_cones = 0;
  cl->n_cones = 0;
}



void _free_cone_list( ConeList *cl )
{
  free( cl->allocatedCones );
  _init_cone_list( cl );
}



void _sort_cones( Cone *cl, int left, int right )
{
  int i, last;
  Cone c;

  if ( left >= right ) return;

  c = cl[left];   cl[left] = cl[(left+right)/2];   cl[(left+right)/2] = c;
  
  last = left;

  for ( i = left+1; i <= right; i++ )
    if ( cl[i].error > cl[left].error ) {
      c = cl[++last];   cl[last] = cl[i];   cl[i] = c;
    }

  c = cl[left];   cl[left] = cl[last];   cl[last] = c;

  _sort_cones( cl, left, last-1 );
  _sort_cones( cl, last+1, right );
  
}



int _add_cone_to_list( ConeList *cl, Cone *c )
{
  char *proc = "_add_cone_to_list";
  Cone *newc;
  int i, nalloc;
  
  /* is allocated space sufficient ?
   */
  if ( cl->n_cones >= cl->n_allocated_cones ) {

    if ( cl->n_allocated_cones <= 0 ) 
      nalloc = _alloc_;
    else
      nalloc = cl->n_allocated_cones + _alloc_;

    /* allocation
     */
    newc = (Cone*)malloc( nalloc * sizeof( Cone ) );
    if ( newc == NULL ) {
      fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
    }
    
    /* copy and/or initialization
     */
    if ( cl->n_allocated_cones <= 0 ) {
      for ( i=0; i<nalloc; i++ ) {
	_init_cone( &(newc[i]) );
      }
    }
    else {
      memcpy( newc, cl->allocatedCones, 
	      cl->n_allocated_cones * sizeof( Cone ) );
      for ( i=cl->n_cones; i<nalloc; i++ ) {
	_init_cone( &(newc[i]) );
      }
      free( cl->allocatedCones );
    }

    cl->allocatedCones = newc;
    cl->n_allocated_cones = nalloc;
  }


  /* create the cone
   */
  cl->n_cones ++;
  _copy_cone( &(cl->allocatedCones[ cl->n_cones-1 ]), c );

  return( cl->n_cones-1 );

}



void _move_cone_backward( ConeList *cl, int n )
{
  int i=n;
  Cone c;
  
  while ( i > 0 
	  && cl->allocatedCones[i].error > cl->allocatedCones[i-1].error ) {
    c = cl->allocatedCones[i];
    cl->allocatedCones[i] = cl->allocatedCones[i-1];
    cl->allocatedCones[i-1] = c;
    i --;
  }
}



void _move_cone_forward( ConeList *cl, int n )
{
  int i=n;
  Cone c;
  
  while ( i < cl->n_cones-1 
	  && cl->allocatedCones[i].error < cl->allocatedCones[i+1].error ) {
    c = cl->allocatedCones[i];
    cl->allocatedCones[i] = cl->allocatedCones[i+1];
    cl->allocatedCones[i+1] = c;
    i ++;
  }
}



int _search_3D_cone( VectorList *vl, ConeList *cl, 
		     int x1, int y1, int z1,
		     int x2, int y2, int z2,
		     int x3, int y3, int z3 )
{
  int i;
  Cone *cc;
  
  for ( i=0; i<cl->n_cones; i++ ) {
    cc = &(cl->allocatedCones[i]);
    if ( vl->allocatedVectors[ cc->iv1 ].x != x1 
	 || vl->allocatedVectors[ cc->iv1 ].y != y1 
	 || vl->allocatedVectors[ cc->iv1 ].z != z1 )
      continue;
    if ( vl->allocatedVectors[ cc->iv2 ].x != x2 
	 || vl->allocatedVectors[ cc->iv2 ].y != y2 
	 || vl->allocatedVectors[ cc->iv2 ].z != z2 )
      continue;
    if ( vl->allocatedVectors[ cc->iv3 ].x != x3 
	 || vl->allocatedVectors[ cc->iv3 ].y != y3 
	 || vl->allocatedVectors[ cc->iv3 ].z != z3 )
      continue;
    return( i );
  }
  return( -1 );
}



int _search_adjacent_3D_cone( ConeList *cl, int iv1, int iv2, int iv3, LocalisationType *side )
{
  int i;
  Cone *cc;
  
  *side = _UNSET_;
  
  for ( i=0; i<cl->n_cones; i++ ) {
    cc = &(cl->allocatedCones[i]);
    /* IJ case
     */
    if ( (cc->iv1 == iv1 && cc->iv2 == iv2 && cc->iv3 != iv3) 
	 || (cc->iv2 == iv1 && cc->iv1 == iv2 && cc->iv3 != iv3) ) {
      *side = _EDGE_IJ_;
      return( i );
    }
    /* JK case
     */
    if ( (cc->iv2 == iv1 && cc->iv3 == iv2 && cc->iv1 != iv3) 
	 || (cc->iv3 == iv1 && cc->iv2 == iv2 && cc->iv1 != iv3) ) {
      *side = _EDGE_JK_;
      return( i );
    }
    /* KI case
     */
    if ( (cc->iv3 == iv1 && cc->iv1 == iv2 && cc->iv2 != iv3) 
	 || (cc->iv1 == iv1 && cc->iv3 == iv2 && cc->iv2 != iv3) ) {
      *side = _EDGE_KI_;
      return( i );
    }
  }
  return( -1 );
}



int _check_3D_cone_naturalconvexity_wrt_edge( FILE *f, VectorList *vl, ConeList *cl, 
					      int iv1, int iv2, int iv3, 
					      char *desc, double *s )
{
  Vector *v1, *v2, *v3, *v4 = NULL;
  int adjcone;
  LocalisationType adjside;
  Vector sym;


  v1 = &(vl->allocatedVectors[ iv1 ]);
  v2 = &(vl->allocatedVectors[ iv2 ]);
  v3 = &(vl->allocatedVectors[ iv3 ]);
    
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

    /* no adjacent cone, symmetrise vector v1 wrt v2, v3
     */
    _build_3D_symmetric_vector_wrt_vectors( &sym, v1, v2, v3, s );
    v4 = &sym;
    if ( _debug_ >= 2 ) {
      fprintf( f, 
	       "SYM (%s) of (%d %d %d) wrt (%d %d %d),(%d %d %d) is (%d %d %d)\n",
	       desc,
	       v1->x, v1->y, v1->z,
	       v2->x, v2->y, v2->z,
	       v3->x, v3->y, v3->z,
	       sym.x, sym.y, sym.z );
    }
  }

  if ( _check_4vectors_naturalconvexity( v1, v2, v3, v4 ) == 0 ) {
    if ( _debug_ >= 2 ) {
      fprintf( f, "convexity: unsuccessful natural convexity test (%s)\n", desc );
      fprintf( f, "           vector (inside) was (%2d %2d %2d)\n", v4->x, v4->y, v4->z );
    }
    return( 0 );
  }
  if ( _debug_ >= 2 ) 
    fprintf( f, "convexity: successful natural convexity test (%s)\n", desc );

  return( 1 );

}



int _check_3D_cone_naturalconvexity( FILE *f, VectorList *vl, ConeList *cl, Cone *c, double *s )
{
  int cjk, cki, cij;
  
  c->convexity = _EDGE_JK_ + _EDGE_KI_ + _EDGE_IJ_;
  
  /* check convexities
   */
  
  cjk =_check_3D_cone_naturalconvexity_wrt_edge( f, vl, cl, c->iv1, c->iv2, c->iv3, "JK", s );

  cki =_check_3D_cone_naturalconvexity_wrt_edge( f, vl, cl, c->iv2, c->iv3, c->iv1, "KI", s );

  cij =_check_3D_cone_naturalconvexity_wrt_edge( f, vl, cl, c->iv3, c->iv1, c->iv2, "IJ", s );

  if ( cjk != 1 ) c->convexity -= _EDGE_JK_;
  if ( cki != 1 ) c->convexity -= _EDGE_KI_;
  if ( cij != 1 ) c->convexity -= _EDGE_IJ_;

  if ( c->convexity == 0 ) return( 0 );
  
  return( 1 );

}



void _check_3D_mask_naturalconvexity( FILE *f, VectorList *vl, ConeList *cl, double *s )
{
  int i;
  for ( i=0; i<cl->n_cones; i++ )
    _check_3D_cone_naturalconvexity( f, vl, cl, &(cl->allocatedCones[i]), s );
}



void _check_3D_mask_fareyregularity( FILE *f, VectorList *vl, ConeList *cl )
{
  char *proc = "_check_3D_mask_fareyregularity";
  int i;
  int d, mat[3][3];
  Vector *v1, *v2, *v3;

  for ( i=0; i<cl->n_cones; i++ ) {
    v1 = &(vl->allocatedVectors[cl->allocatedCones[i].iv1]);
    v2 = &(vl->allocatedVectors[cl->allocatedCones[i].iv2]);
    v3 = &(vl->allocatedVectors[cl->allocatedCones[i].iv3]);
    mat[0][0] = v1->x; mat[0][1] = v1->y; mat[0][2] = v1->z;
    mat[1][0] = v2->x; mat[1][1] = v2->y; mat[1][2] = v2->z;
    mat[2][0] = v3->x; mat[2][1] = v3->y; mat[2][2] = v3->z;
    if ( (d = _3x3_determinant( mat )) != 1 ) {
      fprintf( f, "%s: cone c[#%3d] ", proc, i );
      _print_3D_cone( f, vl, &(cl->allocatedCones[i]) );
      fprintf( f, " has determinant <> 1" );
      fprintf( f, " (det=%d)", d );
      fprintf( f, "\n" );
    }
  }
}









/**************************************************
 *
 * MISC: cone list, print
 *
 **************************************************/




void _print_cone_list( FILE *f, VectorList *vl, ConeList *cl )
{
  int i;
  double emin, emax;
  
  emin = emax= cl->allocatedCones[0].error;
  for ( i=1; i<cl->n_cones; i++ ) {
    if ( emin > cl->allocatedCones[i].error )
      emin = cl->allocatedCones[i].error;
    if ( emax < cl->allocatedCones[i].error )
      emax = cl->allocatedCones[i].error;
  }

   
  fprintf( f, "\n" );
  fprintf( f, "#CONES = %d", cl->n_cones );
  fprintf( f, "   ERR MIN = %f", emin );
  fprintf( f, "   ERR MAX = %f", emax );
  fprintf( f, "\n" );

  for ( i=0; i<cl->n_cones; i++ ) {

    fprintf( f, "c[#%3d] = ", i );

    if ( cl->allocatedCones[i].iv3 < 0 )
      _print_2D_cone( f, vl, &(cl->allocatedCones[i]) );
    else
      _print_3D_cone( f, vl, &(cl->allocatedCones[i]) );
    
    fprintf( f, ", E=%f", cl->allocatedCones[i].error );

    if ( _verbose_ >= 4 ) {
      fprintf( f, ", H=%f", (1-cl->allocatedCones[i].error)/(1+cl->allocatedCones[i].error) );
    }

    if ( _verbose_ >= 3 && cl->allocatedCones[i].iv3 >= 0 ) {
      fprintf( f, ", max=" );
      switch ( cl->allocatedCones[i].maximumType ) {
      case _UNSET_  :  fprintf( f, "---" ); break;
      case _INSIDE_ :  fprintf( f, "IJK" ); break;
      case _EDGE_IJ_ : fprintf( f, "IJ-" ); break;
      case _EDGE_JK_ : fprintf( f, "-JK" ); break;
      case _EDGE_KI_ : fprintf( f, "I-K" ); break;
      }
      fprintf( f, ", conv=" );
      if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ +_EDGE_JK_ + _EDGE_KI_ ) {
	fprintf( f, "IJ,JK,KI" );
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_JK_ ) {
	fprintf( f, "IJ,JK,--" );
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_KI_ ) {
	fprintf( f, "IJ,--,KI" );
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_JK_ + _EDGE_KI_ ) {
	fprintf( f, "--,JK,KI" );
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ ) {
	fprintf( f, "IJ,--,--" );
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_JK_ ) {
	fprintf( f, "--,JK,--" );
      }
      else if ( cl->allocatedCones[i].convexity == _EDGE_KI_ ) {
	fprintf( f, "--,--,KI" );
      }
      else {
	fprintf( f, "--,--,--" );
      }
    }
    fprintf( f, "\n" );
  }

  fprintf( f, "\n" );
}





/* this was the first version of the complexity calculation
   however, it always symmetrises the mask, although it may already
   be constructed in the first quadrant -> overestimation of the complexity
*/

int _old_compute_mask_vector_number( VectorList *vl )
{
  char *proc = "_old_compute_mask_vector_number";
  VectorList nl;
  int i, n;
  Vector normal, res;
  double voxel_size[3] = { 1.0, 1.0, 1.0 };
  int z, nb = 0;
  
  _init_vector_list( &nl );

  z = 0;
  for ( i=0; i<vl->n_vectors; i++ ) {
    (void)_add_vector_to_list( &nl, &(vl->allocatedVectors[i]) );
    if ( vl->allocatedVectors[i].z != 0 ) z++;
  }

  if ( _trace_ >= 4 ) {
    fprintf( stdout, "%s: initial vectors", proc );
    _print_3D_vector_list( stdout, &nl );
    fprintf( stdout, "\n" );
  }

  n = nl.n_vectors;

  normal.x =  1;
  normal.y = -1;
  normal.z =  0;


  /* 2D
   */
  for ( i=0; i<n; i++ ) {
    _build_3D_symmetric_vector_wrt_vector( &res, &(nl.allocatedVectors[i]), &normal, voxel_size );
    (void)_add_vector_to_list( &nl, &res );
  }

  /* 3D
   */
  if ( z > 0 ) {

    n = nl.n_vectors;

    normal.x =  0;
    normal.y = -1;
    normal.z =  1;
    
    for ( i=0; i<n; i++ ) {
      _build_3D_symmetric_vector_wrt_vector( &res, &(nl.allocatedVectors[i]), &normal, voxel_size );
      (void)_add_vector_to_list( &nl, &res );
    }
    
    n = nl.n_vectors;
    
    normal.x =  1;
    normal.y =  0;
    normal.z = -1;
  
    for ( i=0; i<n; i++ ) {
      _build_3D_symmetric_vector_wrt_vector( &res, &(nl.allocatedVectors[i]), &normal, voxel_size );
      (void)_add_vector_to_list( &nl, &res );
    }
    
  }


  for (i=0; i<nl.n_vectors; i++ ) {
    z = (nl.allocatedVectors[i].x == 0) 
      + (nl.allocatedVectors[i].y == 0) 
      + (nl.allocatedVectors[i].z == 0);
    switch ( z ) {
    default : break;
    case 0 : nb += 8; break;
    case 1 : nb += 4; break;
    case 2 : nb += 2; break;
    }
  }

  if ( _trace_ >= 4 ) {
    fprintf( stdout, "%s: vectors in the first quadrant/octant", proc );
    _print_3D_vector_list( stdout, &nl );
    fprintf( stdout, "%s: total complexity = %d\n", proc, nb );
  }

  return( nb );
}










int _compute_mask_vector_number( VectorList *vl, ConeList *cl, double *voxel_size, 
			  int anisotropy_y, int anisotropy_z )
{
  char *proc = "_compute_mask_vector_number";
  VectorList nvl;
  ConeList ncl;
  int notinplane;

  int i, n;
  int nb = 0;
  
  _init_vector_list( &nvl );
  _init_cone_list( &ncl );
  
  notinplane = 0;
  for ( i=0; i<vl->n_vectors; i++ ) {
    (void)_add_vector_to_list( &nvl, &(vl->allocatedVectors[i]) );
    if ( vl->allocatedVectors[i].z != 0 ) notinplane++;
  }
  for ( i=0; i<cl->n_cones; i++ ) {
    (void)_add_cone_to_list( &ncl, &(cl->allocatedCones[i]) );
  }
  
  if ( !notinplane ) 
    _complete_2D_mask_in_quadrant( &nvl, &ncl, voxel_size, anisotropy_y );
  else 
    _complete_3D_mask_in_octant( &nvl, &ncl, voxel_size, anisotropy_y, anisotropy_z );


  for (i=0; i<nvl.n_vectors; i++ ) {
    n = (nvl.allocatedVectors[i].x == 0) 
      + (nvl.allocatedVectors[i].y == 0) 
      + (nvl.allocatedVectors[i].z == 0);
    switch ( n ) {
    default : break;
    case 0 : nb += 8; break;
    case 1 : nb += 4; break;
    case 2 : nb += 2; break;
    }
  }

  if ( _trace_ >= 4 ) {
    fprintf( stdout, "%s: vectors in the first quadrant/octant", proc );
    _print_3D_vector_list( stdout, &nvl );
    fprintf( stdout, "%s: total complexity = %d\n", proc, nb );
  }

  _free_vector_list( &nvl );
  _free_cone_list( &ncl );

  return( nb );
}










/**************************************************
 *
 * MISC: weighted vectors
 *
 **************************************************/





void _init_weighted_vector_list( WeightedVectorList *wvl )
{
  wvl->allocatedVectors = NULL;
  wvl->n_allocated_vectors = 0;
  wvl->n_vectors = 0;
}

void _free_weighted_vector_list( WeightedVectorList *wvl )
{
  int i;
  for ( i=0; i<wvl->n_allocated_vectors; i++ )
    if ( wvl->allocatedVectors[i].c != NULL )
      free( wvl->allocatedVectors[i].c );
  if ( wvl->allocatedVectors != NULL ) free( wvl->allocatedVectors );
  _init_weighted_vector_list( wvl );
}

int _init_weighted_vector_list_from_vector_list( WeightedVectorList *wvl,
						  VectorList *vl )
{
  char *proc = "_init_weighted_vector_list_from_vector_list";
  int i;

  _free_weighted_vector_list( wvl );

  wvl->allocatedVectors = (WeightedVector*)malloc( vl->n_vectors * sizeof(WeightedVector) );
  if ( wvl->allocatedVectors == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }
  for ( i=0; i<vl->n_vectors; i++ ) {
    wvl->allocatedVectors[i].vec = &(vl->allocatedVectors[i]);
    wvl->allocatedVectors[i].n_cones = 0;
    wvl->allocatedVectors[i].c = NULL;
    
    wvl->allocatedVectors[i].w = -1;
    wvl->allocatedVectors[i].f = -1;
    wvl->allocatedVectors[i].l = -1;
  }

  wvl->n_allocated_vectors = vl->n_vectors;
  wvl->n_vectors = vl->n_vectors;
  return( 1 );
}



/**************************************************
 *
 * MISC: weights list
 *
 **************************************************/





void _init_weights_list( WeightsList *rl )
{
  rl->error_optimal = 0.0;
  rl->n_weights = 0;
  rl->n_allocated_weights = 0;
  rl->allocatedWeights = NULL;
}

void _free_weights_list( WeightsList *rl )
{ 
  int i;
  if ( rl->allocatedWeights != NULL) {
    if ( rl->n_weights > 0 ) {
      for ( i=0; i< rl->n_weights; i++ )
	if ( rl->allocatedWeights[i].w != NULL ) 
	  free( rl->allocatedWeights[i].w );
    }
  }
  free( rl->allocatedWeights );
  _init_weights_list( rl );
}


void _add_weights_to_list( WeightsList *rl,  WeightedVectorList *vl,
			   double err, double dmin, double dmax, double eps,
			   double time, double init_time )
{
  char *proc = "_add_weights_to_list";
  Weights *newr;
  int nalloc;
  int i;

  if (  rl->n_weights >= rl->n_allocated_weights ) {

    if ( rl->n_allocated_weights  <= 0 ) 
      nalloc = _alloc_;
    else
      nalloc = rl->n_allocated_weights + _alloc_;

    newr = (Weights*)malloc( nalloc * sizeof(Weights) );
    if ( newr == NULL ) {
      fprintf( stderr, "%s: allocation error\n", proc );
      exit( -1 );
    }
    if ( rl->n_allocated_weights  > 0 ) {
      memcpy( newr, rl->allocatedWeights, 
	      rl->n_allocated_weights * sizeof(Weights) );
      free( rl->allocatedWeights );
    }

    rl->allocatedWeights = newr;
    rl->n_allocated_weights = nalloc;
  }

  rl->allocatedWeights[ rl->n_weights ].w = (int*)malloc( vl->n_vectors * sizeof(int) );
  if ( rl->allocatedWeights[ rl->n_weights ].w == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    exit( -1 );
  }

  for ( i=0; i<vl->n_vectors; i++ )
    rl->allocatedWeights[rl->n_weights ].w[i] = vl->allocatedVectors[i].w;

  rl->allocatedWeights[rl->n_weights ].error = err;
  rl->allocatedWeights[rl->n_weights ].dmin  = dmin;
  rl->allocatedWeights[rl->n_weights ].dmax  = dmax;
  rl->allocatedWeights[rl->n_weights ].epsilon = eps;
  rl->allocatedWeights[rl->n_weights ].elapsed_time = 
    (time - init_time) * 10000 * 1000 / (double)CLOCKS_PER_SEC;

  rl->n_weights ++;
}



