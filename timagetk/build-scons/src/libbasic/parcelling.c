/****************************************************
 * parcelling.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2008
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Tue Apr 15 17:41:49 CEST 2008
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#include <time.h>
#include <stdlib.h>

#include <chamferdistance-mask.h>
#include <chunks.h>
#include <convert.h>
#include <parcelling.h>





/*
 * static global variables
 * verbose,
 * management of ambiguous cases
 * memory management
 * 
 */



static int _verbose_ = 1;

void setVerboseInParcelling( int v )
{
  _verbose_ = v;
}

void incrementVerboseInParcelling(  )
{
  _verbose_ ++;
}

void decrementVerboseInParcelling(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





static int _debug_ = 0;

void setDebugInParcelling( int d )
{
  _debug_ = d;
}

void incrementDebugInParcelling(  )
{
  _debug_ ++;
}

void decrementDebugInParcelling(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}





static long int _random_seed_ = 0;
static int _seed_is_set_ = 0;


long int getRandomSeedInParcelling()
{
    if ( _seed_is_set_ == 0 ) {
        setRandomSeedInParcelling( time(0) );
        _seed_is_set_ = 2;
    }
    return( _random_seed_ );
}

void setRandomSeedInParcelling( long int s )
{
    srandom( s );
    _random_seed_ = s;
    _seed_is_set_ = 1;
}

static double _getRandom( )
{
    if ( _seed_is_set_ == 0 ) {
        setRandomSeedInParcelling( time(0) );
        _seed_is_set_ = 2;
    }
    return( (double)random() / (double)(RAND_MAX) );
}





static int _nPointsToBeAllocated_ = 100;

void parcelling_setNumberOfPointsForAllocation( int n )
{
   if ( n > 0 ) _nPointsToBeAllocated_ = n;
}





static int _max_iterations_ = -1;

void parcelling_setNumberOfIterations( int n )
{
  _max_iterations_ = n;
}





static int _force_exact_center_calculation_ = 1;

void parcelling_ForceExactCenterCalculation()
{
  _force_exact_center_calculation_ = 1;
}

void parcelling_DoNotForceExactCenterCalculation()
{
  _force_exact_center_calculation_ = 0;
}





static int _allows_partial_update_ = 1;

void parcelling_setPartialUpdate( int a )
{
  _allows_partial_update_ = a;
}



/*
 *
 *
 *
 */



















/************************************************************
 *
 * image tools
 *
 ***********************************************************/



static int _count_points( void *theBuf, bufferType theBufType, int *theDim )
{
  char *proc = "_count_points";
  int i, v = theDim[0]*theDim[1]*theDim[2];
  int n = 0;

  switch ( theBufType ) {

  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such label type not handled yet\n", proc );
    }
    return( -1 );

  case UCHAR :
    {
      u8 *buf = (u8*)theBuf;
      for ( i=0; i<v; i++ )
        if ( buf[i] ) n++;
    }
    break;

  case USHORT :
    {
      u16 *buf = (u16*)theBuf;
      for ( i=0; i<v; i++ )
        if ( buf[i] ) n++;
    }
    break;

  case UINT :
    {
      u32 *buf = (u32*)theBuf;
      for ( i=0; i<v; i++ )
        if ( buf[i] ) n++;
    }
    break;

  }

  return( n );
}



static int _check_free_borders( void *theBuf, bufferType theBufType, int *theDim )
{
  char *proc = "_check_free_borders";
  int x, y, z;

  switch ( theBufType ) {

  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such label type not handled yet\n", proc );
    }
    return( -1 );

  case UCHAR :
    {
      u8 *buf = (u8*)theBuf;

      if ( theDim[2] == 1 ) {

        for ( y=0; y<theDim[1]; y++ )
          if ( buf[ y*theDim[0] ] || buf[ theDim[0]-1 + y*theDim[0] ] )
            return( -1 );
        for ( x=0; x<theDim[0]; x++ )
          if ( buf[ x ] || buf[ x + (theDim[1]-1)*theDim[0] ] )
            return( -1 );

      }
      else {

        for ( y=0; y<theDim[1]; y++ )
        for ( x=0; x<theDim[0]; x++ )
          if ( buf[ x + y*theDim[0] ]
               || buf[ x + y*theDim[0] + (theDim[2]-1)*theDim[0]*theDim[1] ] )
            return( -1 );
        for ( z=0; z<theDim[2]; z++ )
        for ( x=0; x<theDim[0]; x++ )
          if ( buf[ x + z*theDim[0]*theDim[1] ]
               || buf[ x + (theDim[1]-1)*theDim[0] + z*theDim[0]*theDim[1] ] )
            return( -1 );
        for ( z=0; z<theDim[2]; z++ )
        for ( y=0; y<theDim[1]; y++ )
          if ( buf[ y*theDim[0] + z*theDim[0]*theDim[1] ]
               || buf[ theDim[0]-1 + y*theDim[0] + z*theDim[0]*theDim[1] ] )
            return( -1 );

      }

    }
    break;
  }

  return( 1 );
}



static void _print_neighorhood( int x, int y, int z, void *buf, bufferType type, int *theDim, char *str )
{
  int i, j, k;

  fprintf( stdout, "Voisinage:" );
  if ( str != (char*)NULL ) fprintf( stdout, "%s", str );
  fprintf( stdout, "\n" );

  switch ( type ) {
  default :
    fprintf( stdout, "type not handled yet\n" );
    break;

  case DOUBLE :
    {
      double *theBuf = (double*)buf;
      for ( j=-1; j<=1; j++ ) {
  for ( k=-1; k<=1; k++ ) {
    for ( i=-1; i<=1; i++ ) {
      if ( x+i < 0 || x+i >= theDim[0]
     || y+j < 0 || y+j >= theDim[1]
     || z+k < 0 || z+k >= theDim[2] )
        fprintf( stdout, " ....." );
      else
        fprintf( stdout, " %9f", theBuf[ ((z+k)*theDim[1]+(y+j))*theDim[0]+(x+i) ] );
  }
    if ( k < 1 ) fprintf( stdout, "  - " );
  }
  fprintf( stdout, "\n" );
      }
      fprintf( stdout, "\n" );
    }
    break;

  case USHORT :
    {
      u16 *theBuf = (u16*)buf;
      for ( j=-1; j<=1; j++ ) {
        for ( k=-1; k<=1; k++ ) {
          for ( i=-1; i<=1; i++ ) {
            if ( x+i < 0 || x+i >= theDim[0]
                 || y+j < 0 || y+j >= theDim[1]
                 || z+k < 0 || z+k >= theDim[2] )
              fprintf( stdout, " ....." );
          else
            fprintf( stdout, " %5d", theBuf[ ((z+k)*theDim[1]+(y+j))*theDim[0]+(x+i) ] );
          }
          if ( k < 1 ) fprintf( stdout, "  - " );
        }
        fprintf( stdout, "\n" );
      }
      fprintf( stdout, "\n" );
    }
    break;

  case UINT :
    {
      u32 *theBuf = (u32*)buf;
      for ( j=-1; j<=1; j++ ) {
        for ( k=-1; k<=1; k++ ) {
          for ( i=-1; i<=1; i++ ) {
            if ( x+i < 0 || x+i >= theDim[0]
                 || y+j < 0 || y+j >= theDim[1]
                 || z+k < 0 || z+k >= theDim[2] )
              fprintf( stdout, " ....." );
          else
            fprintf( stdout, " %9d", theBuf[ ((z+k)*theDim[1]+(y+j))*theDim[0]+(x+i) ] );
          }
          if ( k < 1 ) fprintf( stdout, "  - " );
        }
        fprintf( stdout, "\n" );
      }
      fprintf( stdout, "\n" );
    }
    break;

  }
}










/************************************************************
 *
 * List management
 *
 ***********************************************************/



typedef struct typePoint {
  int x;
  int y;
  int z;
  int i;
  int l;
  int d;
  int inside;
} typePoint;

typedef struct typePointList {
  int nPoints;
  int nAllocatedPoints;
  typePoint *pt;
} typePointList;



static void _init_point_list( typePointList *l )
{
  l->nPoints = 0;
  l->nAllocatedPoints = 0;
  l->pt = (typePoint *)NULL;
}



static void _free_point_list( typePointList *l )
{
  l->nPoints = 0;
  l->nAllocatedPoints = 0;
  if ( l->pt != (typePoint *)NULL )
    free( l->pt );
  l->pt = (typePoint *)NULL;
}



/* args =
 * coordinates x, y, z
 * absolute index in volume (i = z*dimx*dimy + y*dimx + x)
 * parcel label
 * distance
 */

static int _add_point_to_list( typePointList *l,
                               int x, int y, int z, int i,
                               int label, int dist, int *theDim, int *theMargin )
{
  char *proc = "_add_point_to_list";
  int n = l->nPoints;
  int newn;
  typePoint *pt = NULL;

  if ( n == l->nAllocatedPoints ) {

    newn = l->nAllocatedPoints + _nPointsToBeAllocated_;

    pt = (typePoint *)malloc( newn * sizeof( typePoint ) );
    if ( pt == NULL ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: can not reallocate point list\n", proc );
        fprintf( stderr, "\t failed to add (%d,%d,%d) in list", x, y, z );
      }
      return( -1 );
    }

    if ( l->pt != NULL ) {
      (void)memcpy( (void*)pt, (void*)l->pt,  l->nAllocatedPoints * sizeof( typePoint ) );
      free( l->pt );
    }

    l->pt = pt;
    l->nAllocatedPoints = newn;

  }

  l->pt[ n ].x = x;
  l->pt[ n ].y = y;
  l->pt[ n ].z = z;
  l->pt[ n ].i = i;
  l->pt[ n ].l = label;
  l->pt[ n ].d = dist;

  l->pt[ n ].inside = 0;
  if ( x >= theMargin[0] && x < theDim[0]-theMargin[0]
       && y >= theMargin[1] && y < theDim[1]-theMargin[1]
       && z >= theMargin[2] && z < theDim[2]-theMargin[2] )
    l->pt[ n ].inside = 1;

  l->nPoints ++;

  return( l->nPoints );
}










/************************************************************
 *
 * List tools
 *
 ***********************************************************/



static int _remove_first_points_from_list( typePointList *thePointList, int npts )
{
  char *proc = "_remove_first_points_from_list";
  int i;
  int o = thePointList->nPoints;

  if ( npts > thePointList->nPoints ) {
    fprintf( stderr, "%s: too many points to be removed\n", proc );
    return( -1 );
  }

  for ( i=npts; i<thePointList->nPoints; i++ )
    thePointList->pt[i-npts] = thePointList->pt[i];
  thePointList->nPoints -= npts;

  if ( 0 && _debug_) {
    fprintf( stderr, "%s: remove %d points from %d -> %d\n", proc, npts, o, thePointList->nPoints );
  }

  return( 1 );
}



static int _get_min_distance( typePointList *thePointList, int *nb )
{
  int f, n, min;
  typePoint aux, *pt = thePointList->pt;

  min = pt[0].d;

  for ( n=1; n<thePointList->nPoints; n++ ) {
    if ( pt[n].d < min ) min = pt[n].d;
  }

  f = 0;
  while ( f < thePointList->nPoints && pt[f].d == min ) f++;

  for ( n=f; n<thePointList->nPoints; n++ ) {
    if ( pt[n].d == min ) {
      aux = pt[n];
      pt[n] = pt[f];
      pt[f] = aux;
      f ++;
    }
  }

  if ( 0 && _verbose_ >= 4 ) {
    fprintf( stderr, "min distance = %d, #pts = %d\n", min, f );
    for ( n=0; n<thePointList->nPoints; n++ ) {
      fprintf( stderr, "D[#%3d] = %9d (%d %d %d)\n", n, pt[n].d, pt[n].x, pt[n].y, pt[n].z );
    }
  }

  *nb = f;
  return( min );
}










/************************************************************
 *
 * parcel list tools
 *
 ***********************************************************/



typedef struct typeParcelCenter {
  int x;
  int y;
  int z;
} typeParcelCenter;

static void _initParcelCenter( typeParcelCenter *c )
{
    c->x = -1;
    c->y = -1;
    c->z = -1;
}

typedef enum enumParcelStatus {
    UNCHANGED,
    CHANGING,
    OSCILLATING,
    NEIGHBOR
} enumParcelStatus;

typedef struct typeParcel {
  /* bounding box
   */
  int xmin, xmax;
  int ymin, ymax;
  int zmin, zmax;
  /* barycenter calculation
   */
  int n;
  double sx, sy, sz;
  int x, y, z;
  /* centers
   */
  enumParcelStatus status;
  typeParcelCenter center;
  typeParcelCenter center1;
  typeParcelCenter center2;
} typeParcel;

static void _initParcel( typeParcel *p )
{
    p->xmin = p->xmax = -1;
    p->ymin = p->ymax = -1;
    p->zmin = p->zmax = -1;
    p->n = 0;
    p->sx = 0.0;
    p->sy = 0.0;
    p->sz = 0.0;
    p->x = 0;
    p->y = 0;
    p->z = 0;
    p->status = CHANGING;
    _initParcelCenter( &(p->center) );
    _initParcelCenter( &(p->center1) );
    _initParcelCenter( &(p->center2) );
}



static int _get_initial_parcel_centers( void* theLabels, bufferType theLabelsType, int *theDim,
                                        typeParcel *parcel, int nparcels )
{
  char *proc = "_get_initial_parcel_centers";
  int n, npoints;
  typePoint *pt, tmp;
  int i, j, x, y, z;

  npoints = _count_points( theLabels, theLabelsType, theDim );

  fprintf(stdout, "npoints = %d\n", npoints);
  fprintf(stdout, "nparcels = %d\n", nparcels);

  if ( npoints <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error or empty input image\n", proc );
    return( -1 );
  }

  if ( npoints <= nparcels ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: too many parcels or not enough points\n", proc );
    return( -1 );
  }

  pt = (typePoint *)malloc( npoints*sizeof( typePoint ) );
  if ( pt == (typePoint *)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate point list\n", proc );
    return( -1 );
  }

  switch( theLabelsType ) {
  default :
      free( pt );
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such label type not handled yet\n", proc );
      }
      return( -1 );
  case UCHAR :
      {
          u8 *theBuf = (u8*)theLabels;
          for ( i=0, j=0, z=0; z<theDim[2]; z++ )
          for ( y=0; y<theDim[1]; y++ )
          for ( x=0; x<theDim[0]; x++, i++ ) {
            if ( theBuf[ i ] ) {
              pt[j].x = x;
              pt[j].y = y;
              pt[j].z = z;
              j ++;
            }
          }
      }
      break;
  case USHORT :
      {
          u16 *theBuf = (u16*)theLabels;
          for ( i=0, j=0, z=0; z<theDim[2]; z++ )
          for ( y=0; y<theDim[1]; y++ )
          for ( x=0; x<theDim[0]; x++, i++ ) {
            if ( theBuf[ i ] ) {
              pt[j].x = x;
              pt[j].y = y;
              pt[j].z = z;
              j ++;
            }
          }
      }
      break;
  case UINT :
      {
          u32 *theBuf = (u32*)theLabels;
          for ( i=0, j=0, z=0; z<theDim[2]; z++ )
          for ( y=0; y<theDim[1]; y++ )
          for ( x=0; x<theDim[0]; x++, i++ ) {
            if ( theBuf[ i ] ) {
              pt[j].x = x;
              pt[j].y = y;
              pt[j].z = z;
              j ++;
            }
          }
      }
      break;
  }

  /* draw random points
   */
  for ( n = npoints, j=0; j<nparcels; j++, n-- ) {
      i = j + (int)((double)n * _getRandom() + 0.5);
      if ( i == j + n )
          i = j + n-1;
      tmp = pt[j];   pt[j] = pt[i];   pt[i] = tmp;
  }

  for ( j=0; j<nparcels; j++ ) {
    parcel[j].center.x = pt[j].x;
    parcel[j].center.y = pt[j].y;
    parcel[j].center.z = pt[j].z;
  }

  free( pt );
  return( 1 );
}



static int _get_parcel_neighbors( typePointList *thePointList, int npts,
                                  void* theLabels, bufferType theLabelsType, int *theDim,
                                  int *theMargin,
                                  int INLIST, int NONINLIST )
{
  char *proc = "_get_parcel_neighbors";
  int x, y, z;
  int i, j, k;
  int n, v;
  int o = thePointList->nPoints;
  int a = 0;

  if ( 0 && _debug_ )
      fprintf( stderr, "%s: look for neighbors of %d points\n", proc, npts );


#define _GETPARCELNEIGHBORS( TYPE ) {                                         \
  TYPE *theBuf = (TYPE*)theLabels;                                            \
  for ( n=0; n<npts; n++ ) {                                                  \
    x = thePointList->pt[n].x;                                                \
    y = thePointList->pt[n].y;                                                \
    z = thePointList->pt[n].z;                                                \
    if ( thePointList->pt[n].inside ) {                                       \
      if ( theDim[2] == 1 ) {                                                 \
        for ( j=-1; j<=1; j++ )                                               \
        for ( i=-1; i<=1; i++ ) {                                             \
          v = (y+j)*theDim[0] + x+i;                                          \
          if ( theBuf[v] != (TYPE)NONINLIST ) continue;                       \
          if ( _add_point_to_list( thePointList, x+i, y+j, z, v, -1, -1, theDim, theMargin ) <= 0 ) { \
            if ( _verbose_ )                                                  \
              fprintf( stderr, "%s: unable to add point to list\n", proc );   \
            return( -1 );                                                     \
          }                                                                   \
          a ++;                                                               \
          theBuf[v] = INLIST;                                                 \
        }                                                                     \
      }                                                                       \
      else {                                                                  \
        for ( k=-1; k<=1; k++ )                                               \
        for ( j=-1; j<=1; j++ )                                               \
        for ( i=-1; i<=1; i++ ) {                                             \
          v = (z+k)*theDim[1]*theDim[0] + (y+j)*theDim[0] + x+i;              \
          if ( theBuf[v] != (TYPE)NONINLIST ) continue;                             \
          if ( _add_point_to_list( thePointList, x+i, y+j, z+k, v, -1, -1, theDim, theMargin ) <= 0 ) { \
            if ( _verbose_ )                                                  \
              fprintf( stderr, "%s: unable to add point to list\n", proc );   \
            return( -1 );                                                     \
          }                                                                   \
          a ++;                                                               \
          theBuf[v] = INLIST;                                                 \
        }                                                                     \
      }                                                                       \
    }                                                                         \
    else {                                                                    \
      for ( k=-1; k<=1; k++ ) {                                               \
        if ( z+k < 0 || z+k >= theDim[2] ) continue;                          \
        for ( j=-1; j<=1; j++ ) {                                             \
          if ( y+j < 0 || y+j >= theDim[1] ) continue;                        \
          for ( i=-1; i<=1; i++ ) {                                           \
            if ( x+i < 0 || x+i >= theDim[0] ) continue;                      \
            v = (z+k)*theDim[1]*theDim[0] + (y+j)*theDim[0] + x+i;            \
            if ( theBuf[v] != (TYPE)NONINLIST ) continue;                     \
            if ( _add_point_to_list( thePointList, x+i, y+j, z+k, v, -1, -1, theDim, theMargin ) <= 0 ) { \
              if ( _verbose_ )                                                \
                fprintf( stderr, "%s: unable to add point to list\n", proc ); \
              return( -1 );                                                   \
            }                                                                 \
            a ++;                                                             \
            theBuf[v] = (TYPE)INLIST;                                         \
          }                                                                   \
        }                                                                     \
      }                                                                       \
    }                                                                         \
  }                                                                           \
}

  switch( theLabelsType ) {
  default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such label type not handled yet\n", proc );
      }
      return( -1 );
  case UCHAR :
      _GETPARCELNEIGHBORS( u8 );
      break;
  case USHORT :
      _GETPARCELNEIGHBORS( u16 );
      break;
  case UINT :
      _GETPARCELNEIGHBORS( u32 );
      break;
  }

  if ( 0 && _debug_ ) {
    fprintf( stderr, "%s: add %d points to %d -> %d\n", proc, a, o, thePointList->nPoints );
  }

  return( 1 );
}



static void _update_distances_in_list( typePointList *thePointList, typeChamferMask *theMask,
                                       int MAXDIST,
                                       u16* theDist,
                                       void* theLabels, bufferType theLabelsType, int *theDim,
                                       int nparcels )
{
  char *proc = "_update_distances_in_list";
  int n, l, d;
  typePoint *pt = thePointList->pt;
  int w, x, y, z, i;

#define _UPDATEDISTANCESINLIST( TYPE ) {                            \
  TYPE *theBuf = (TYPE*)theLabels;                                  \
  for ( n=0; n<thePointList->nPoints; n++ ) {                       \
    if ( theDist[ pt[n].i ] == 0 ) {                                \
      pt[n].d = 0;                                                  \
      pt[n].l = theBuf[ pt[n].i ];                                  \
      continue;                                                     \
    }                                                               \
    x = pt[n].x;                                                    \
    y = pt[n].y;                                                    \
    z = pt[n].z;                                                    \
    d = MAXDIST;                                                    \
    l = 0;                                                          \
    if ( pt[n].inside ) {                                           \
      for ( i=0; i<theMask->nb; i++ ) {                             \
          w = ((z+theMask->list[i].z)*theDim[1] + (y+theMask->list[i].y))*theDim[0] + (x+theMask->list[i].x); \
          if ( theBuf[w] == (TYPE)0 || theBuf[w] > (TYPE)nparcels ) continue; \
          if ( theDist[w] + theMask->list[i].inc < d ) {            \
              d = theDist[w] + theMask->list[i].inc;                \
              l = theBuf[w];                                        \
          }                                                         \
      }                                                             \
    }                                                               \
    else {                                                          \
      for ( i=0; i<theMask->nb; i++ ) {                             \
        if ( pt[n].x + theMask->list[i].x < 0                       \
             || pt[n].x + theMask->list[i].x > theDim[0]-1          \
             || pt[n].y + theMask->list[i].y < 0                    \
             || pt[n].y + theMask->list[i].y > theDim[1]-1          \
             || pt[n].z + theMask->list[i].z < 0                    \
             || pt[n].z + theMask->list[i].z > theDim[2]-1 )        \
          continue;                                                 \
        w = ((z+theMask->list[i].z)*theDim[1] + (y+theMask->list[i].y))*theDim[0] + (x+theMask->list[i].x); \
        if ( theBuf[w] == (TYPE)0 || theBuf[w] > (TYPE)nparcels ) continue; \
        if ( theDist[w] + theMask->list[i].inc < d ) {              \
            d = theDist[w] + theMask->list[i].inc;                  \
            l = theBuf[w];                                          \
        }                                                           \
      }                                                             \
    }                                                               \
    pt[n].d = d;                                                    \
    pt[n].l = l;                                                    \
  }                                                                 \
}

  switch( theLabelsType ) {
  default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such label type not handled yet\n", proc );
      }
      return;
  case UCHAR :
      _UPDATEDISTANCESINLIST( u8 );
      break;
  case USHORT :
      _UPDATEDISTANCESINLIST( u16 );
      break;
  case UINT :
      _UPDATEDISTANCESINLIST( u32 );
      break;
  }
}



static int _get_parcels_from_centers( u16* theDist,
                                      void* theLabels, bufferType theLabelsType, int *theDim,
                                      typeParcel *parcel, int nparcels,
                                      typeChamferMask *theMask,
                                      int PartialUpdate )
{
  char *proc = "_get_parcels_from_centers";
  int MAXDIST = 65535;
  int NONINLIST = 0;
  int INLIST = 0;

  typePointList thePointList;
  int i, j;
  int v = theDim[2]*theDim[1]*theDim[0];
  int nNewPoints, dmin;
  int theMargin[3];



  /* initialize labels image and distance map
     background : 0
     points other than seeds : MAXDIST
     seeds : 0
   */

  switch( theLabelsType ) {
  default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such label type not handled yet\n", proc );
      }
      return( -1 );
  case UCHAR :
      if ( nparcels+2 > 255 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: too many parcels\n", proc );
          return( -1 );
      }
      break;
  case USHORT :
      if ( nparcels+2 > 65535 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: too many parcels\n", proc );
          return( -1 );
      }
      break;
  case UINT :
      break;
  }

  INLIST = nparcels+1;
  NONINLIST = nparcels+2;



#define _GLOBALINITLABELS( TYPE ) { \
  TYPE *theBuf = (TYPE*)theLabels;     \
  for ( i=0; i<v; i++ ) {              \
    if ( theBuf[ i ] == 0 ) {          \
        theDist[ i ] = 0;              \
        continue;                      \
    }                                  \
    theBuf[ i ] = NONINLIST;           \
    theDist[ i ] = MAXDIST;            \
  }                                    \
  for ( i=0; i<nparcels; i++ ) {       \
    j = (parcel[i].center.z * theDim[1] + parcel[i].center.y) * theDim[0] + parcel[i].center.x; \
    theBuf[ j ] = i+1;                 \
    theDist[ j ] = 0;                  \
  }                                    \
}

#define _PARTIALINITLABELS( TYPE ) {           \
  TYPE *theBuf = (TYPE*)theLabels;             \
  for ( i=0; i<v; i++ ) {                      \
      if ( theBuf[ i ] == 0 ) {                \
          theDist[ i ] = 0;                    \
          continue;                            \
      }                                        \
      switch( parcel[ theBuf[i]-1 ].status ) { \
      default :                                \
          if ( _verbose_ ) {                   \
            fprintf( stderr, "%s: such case is not handled yet\n", proc ); \
          }                                    \
          return( -1 );                        \
      case CHANGING :                          \
      case NEIGHBOR :                          \
      case OSCILLATING :                       \
          theBuf[ i ] = NONINLIST;             \
          theDist[ i ] = MAXDIST;              \
          break;                               \
      case UNCHANGED :                         \
          break;                               \
      }                                        \
  }                                            \
  for ( i=0; i<nparcels; i++ ) {               \
      switch( parcel[ i ].status ) {           \
      default :                                \
          break;                               \
      case CHANGING :                          \
      case NEIGHBOR :                          \
      case OSCILLATING :                       \
          j = (parcel[i].center.z * theDim[1] + parcel[i].center.y) * theDim[0] + parcel[i].center.x; \
          theBuf[ j ] = i+1;                   \
          theDist[ j ] = 0;                    \
          break;                               \
      case UNCHANGED :                         \
          break;                               \
      }                                        \
  }                                            \
}

  if ( PartialUpdate ) {
      switch( theLabelsType ) {
      default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such label type not handled yet\n", proc );
          }
          return( -1 );
      case UCHAR :
          _PARTIALINITLABELS( u8 );
          break;
      case USHORT :
          _PARTIALINITLABELS( u16 );
          break;
      case UINT :
          _PARTIALINITLABELS( u32 );
          break;
      }
  }
  else {
      switch( theLabelsType ) {
      default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such label type not handled yet\n", proc );
          }
          return( -1 );
      case UCHAR :
          _GLOBALINITLABELS( u8 );
          break;
      case USHORT :
          _GLOBALINITLABELS( u16 );
          break;
      case UINT :
          _GLOBALINITLABELS( u32 );
          break;
      }
  }



  if ( 0 && _verbose_ >= 3 ) {
    fprintf( stderr, "%s: initial centers\n", proc );
    for ( i=0; i<nparcels; i++ ) {
      fprintf( stderr, "center[#%3d] = %d %d %d\n", i,
               parcel[i].center.x, parcel[i].center.y, parcel[i].center.z );
    }
  }



  /* initialize point list
     get neighbors of parcels
     remove initial points
   */

  theMargin[0] = theMask->list[0].x;
  theMargin[1] = theMask->list[0].y;
  theMargin[2] = theMask->list[0].z;
  for ( i=1; i<theMask->nb; i++ ) {
      if ( theMargin[0] < theMask->list[i].x ) theMargin[0] = theMask->list[i].x;
      if ( theMargin[1] < theMask->list[i].y ) theMargin[1] = theMask->list[i].y;
      if ( theMargin[2] < theMask->list[i].z ) theMargin[2] = theMask->list[i].z;
  }


  /* put centers of parcel into list for distance computation
   */
  _init_point_list( &thePointList );

  if ( PartialUpdate ) {
      for ( i=0; i<nparcels; i++ ) {
          switch( parcel[ i ].status ) {
          default :
              break;
          case CHANGING :
          case NEIGHBOR :
          case OSCILLATING :
              j = (parcel[i].center.z * theDim[1] + parcel[i].center.y) * theDim[0] + parcel[i].center.x;
              if ( _add_point_to_list( &thePointList,
                                       parcel[i].center.x, parcel[i].center.y, parcel[i].center.z,
                                       j, i+1, 0, theDim, theMargin ) <= 0 ) {
                if ( _verbose_ )
                  fprintf( stderr, "%s: unable to add point to list\n", proc );
                _free_point_list( &thePointList );
                return( -1 );
              }
              break;
          case UNCHANGED :
              break;
          }
      }
  }
  else {
      for ( i=0; i<nparcels; i++ ) {
        j = (parcel[i].center.z * theDim[1] + parcel[i].center.y) * theDim[0] + parcel[i].center.x;
        if ( _add_point_to_list( &thePointList,
                                 parcel[i].center.x, parcel[i].center.y, parcel[i].center.z,
                                 j, i+1, 0, theDim, theMargin ) <= 0 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to add point to list\n", proc );
          _free_point_list( &thePointList );
          return( -1 );
        }
      }
  }

  nNewPoints = thePointList.nPoints;

  if ( _get_parcel_neighbors( &thePointList, nNewPoints, theLabels, theLabelsType, theDim, theMargin, INLIST, NONINLIST ) <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to add neighbors to list\n", proc );
    _free_point_list( &thePointList );
    return( -1 );
  }

  if ( 0 && _verbose_ >= 3 ) {
    fprintf( stderr, "%s: %d/%d points at distance 0 in list (%d parcels)\n",
             proc, nNewPoints, thePointList.nPoints, nparcels );
  }

  if ( _remove_first_points_from_list( &thePointList, nNewPoints) <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to remove points from list\n", proc );
    _free_point_list( &thePointList );
    return( -1 );
  }

  do {

    _update_distances_in_list( &thePointList, theMask, MAXDIST, theDist, theLabels, theLabelsType, theDim, nparcels );

    dmin = _get_min_distance( &thePointList, &nNewPoints );

    if ( 0 && _debug_ )
      fprintf( stderr, "%s: %d/%d points at distance %d in list (%d parcels)\n",
               proc, nNewPoints, thePointList.nPoints, dmin, nparcels );

    if ( _get_parcel_neighbors( &thePointList, nNewPoints, theLabels, theLabelsType, theDim, theMargin, INLIST, NONINLIST ) <= 0 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to add neighbors to list\n", proc );
      _free_point_list( &thePointList );
      return( -1 );
    }

    switch( theLabelsType ) {
    default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such label type not handled yet\n", proc );
        }
        _free_point_list( &thePointList );
        return( -1 );
    case UCHAR :
        {
            u8 *theBuf = (u8*)theLabels;
            for ( i=0; i<nNewPoints; i++ ) {
              theDist[ thePointList.pt[i].i ] = thePointList.pt[i].d;
              theBuf[ thePointList.pt[i].i ] = thePointList.pt[i].l;
            }
        }
        break;
    case USHORT :
        {
            u16 *theBuf = (u16*)theLabels;
            for ( i=0; i<nNewPoints; i++ ) {
              theDist[ thePointList.pt[i].i ] = thePointList.pt[i].d;
              theBuf[ thePointList.pt[i].i ] = thePointList.pt[i].l;
            }
        }
        break;
    case UINT :
        {
            u32 *theBuf = (u32*)theLabels;
            for ( i=0; i<nNewPoints; i++ ) {
              theDist[ thePointList.pt[i].i ] = thePointList.pt[i].d;
              theBuf[ thePointList.pt[i].i ] = thePointList.pt[i].l;
            }
        }
        break;
    }

    if ( _remove_first_points_from_list( &thePointList, nNewPoints) <= 0 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to remove points from list\n", proc );
      _free_point_list( &thePointList );
      return( -1 );
    }

  } while ( thePointList.nPoints > 0 );

  _free_point_list( &thePointList );

  return( 1 );
}



static int _get_parcel_center( typeParcel *parcel,
                               int label,
                               u16* theDist,
                               void* theLabels, bufferType theLabelsType,
                               int *theDim, typeChamferMask *theMask )
{
  char *proc = "_get_parcel_center";
  u32 *theLocalLabels = NULL;
  u16 *theLocalDist = NULL;
  double *theLocalCriteria = NULL;
  int theLocalDim[3];

  typeParcel theParcel;
  typeParcelCenter theBestCenter;
  int indexBestCenter, indexCenter;

  int x, y, z;
  int lx, ly, lz, i, j, k, v;
  int flag;

  int localverbose = 0;


  theLocalDim[0] = parcel[0].xmax - parcel[0].xmin + 1;
  theLocalDim[1] = parcel[0].ymax - parcel[0].ymin + 1;
  theLocalDim[2] = parcel[0].zmax - parcel[0].zmin + 1;

  /* allocations
   */
  theLocalLabels = (u32*)malloc( theLocalDim[0]*theLocalDim[1]*theLocalDim[2]*sizeof(u32) );
  if ( theLocalLabels == (void*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate 1st auxiliary array\n", proc );
    return( -1 );
  }

  theLocalDist = (u16*)malloc( theLocalDim[0]*theLocalDim[1]*theLocalDim[2]*sizeof(u16) );
  if ( theLocalDist == (void*)NULL ) {
      free( theLocalLabels );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate distance auxiliary array\n", proc );
      return( -1 );
  }

  theLocalCriteria = (double*)malloc( theLocalDim[0]*theLocalDim[1]*theLocalDim[2]*sizeof(double) );
  if ( theLocalCriteria == (double*)NULL ) {
      free( theLocalDist );
      free( theLocalLabels );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate 2nd auxiliary array\n", proc );
      return( -1 );
  }


  /* get the parcel and the distances associated to the actual center
   */

#define _FILLLOCALPARCEL( TYPE ) {                          \
  TYPE *theBuf = (TYPE*)theLabels;                          \
  for ( i=0, lz=0; lz<theLocalDim[2]; lz++ ) {              \
    z = lz + parcel[0].zmin;                                \
    for ( ly=0; ly<theLocalDim[1]; ly++ ) {                 \
      y = ly + parcel[0].ymin;                              \
      j = (z * theDim[1] + y) * theDim[0] + parcel[0].xmin; \
      for ( lx=0; lx<theLocalDim[0]; lx++, i++, j++ ) {     \
        theLocalCriteria[i] = 0.0;                          \
        if ( theBuf[j] == (TYPE)label ) {                   \
          theLocalLabels[i] = label;                        \
          theLocalDist[i] = theDist[j];                     \
        }                                                   \
        else {                                              \
          theLocalLabels[i] = 0;                            \
          theLocalDist[i] = 0;                              \
        }                                                   \
      }                                                     \
    }                                                       \
  }                                                         \
}

  switch( theLabelsType ) {
  default :
      free( theLocalCriteria );
      free( theLocalDist );
      free( theLocalLabels );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate 2nd auxiliary array\n", proc );
      return( -1 );
  case UCHAR :
      _FILLLOCALPARCEL( u8 );
      break;
  case USHORT :
      _FILLLOCALPARCEL( u16 );
      break;
  case UINT :
      _FILLLOCALPARCEL( u32 );
      break;
  }


  /* computed criterium for the actual center
   */
  theBestCenter.z = z = parcel[0].center.z - parcel[0].zmin;
  theBestCenter.y = y = parcel[0].center.y - parcel[0].ymin;
  theBestCenter.x = x = parcel[0].center.x - parcel[0].xmin;

  indexBestCenter = (z*theLocalDim[1] + y)*theLocalDim[0] + x;

  theLocalCriteria[ indexBestCenter ] = 0.0;
  for ( v=0, lz=0; lz<theLocalDim[2]; lz++ )
  for ( ly=0; ly<theLocalDim[1]; ly++ )
  for ( lx=0; lx<theLocalDim[0]; lx++, v++ ) {
    theLocalCriteria[ indexBestCenter ] += (int)theLocalDist[v]*(int)theLocalDist[v];
  }

  if ( localverbose )
    fprintf( stdout, "   1st Criterium (%d %d %d)= %f\n",
             theBestCenter.x, theBestCenter.y, theBestCenter.z,
             theLocalCriteria[ indexBestCenter ] );

  do {

    flag = 0;
    for ( k=-1; !flag && k<=1; k++ ) {

      theParcel.center.z = theBestCenter.z+k;
      if ( theParcel.center.z < 0 || theParcel.center.z >= theLocalDim[2] ) continue;

      for ( j=-1; !flag && j<=1; j++ ) {

        theParcel.center.y = theBestCenter.y+j;
        if ( theParcel.center.y < 0 || theParcel.center.y >= theLocalDim[1] ) continue;

        for ( i=-1; !flag && i<=1; i++ ) {

          theParcel.center.x = theBestCenter.x+i;
          if ( theParcel.center.x < 0 || theParcel.center.x >= theLocalDim[0] ) continue;

          indexCenter = (theParcel.center.z*theLocalDim[1] + theParcel.center.y)*theLocalDim[0] + theParcel.center.x;

          if ( theLocalCriteria[ indexCenter ] > 0.0 ) continue;

          /* not in the parcel
           */
          if ( theLocalLabels[ indexCenter ] == 0 ) continue;

          if ( _get_parcels_from_centers( theLocalDist, theLocalLabels, UINT,
                                          theLocalDim, &theParcel, 1, theMask, 0 ) <= 0 ) {
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to compute criteria\n", proc );
            free( theLocalCriteria );
            free( theLocalDist );
            free( theLocalLabels );
            return( -1 );
          }

          theLocalCriteria[ indexCenter ] = 0.0;
          for ( v=0, lz=0; lz<theLocalDim[2]; lz++ )
            for ( ly=0; ly<theLocalDim[1]; ly++ )
              for ( lx=0; lx<theLocalDim[0]; lx++, v++ ) {
                theLocalCriteria[ indexCenter ] += (int)theLocalDist[v]*(int)theLocalDist[v];
              }

          if ( localverbose )
            fprintf( stdout, "      test  (%d %d %d)= %f",
                     theParcel.center.x, theParcel.center.y, theParcel.center.z,
                     theLocalCriteria[ indexCenter ] );

          if ( theLocalCriteria[ indexCenter ] < theLocalCriteria[ indexBestCenter ] ) {
            flag = 1;
            theBestCenter = theParcel.center;
            indexBestCenter = indexCenter;
            if ( localverbose )
              fprintf( stdout, " => SUCCESS\n");
          }
          else {
            if ( localverbose )
              fprintf( stdout, " -> failure\n");
          }
          if ( localverbose )
            _print_neighorhood( theParcel.center.x, theParcel.center.y, theParcel.center.z, theLocalCriteria, DOUBLE, theLocalDim, NULL );

        }
      }
    }


  } while ( flag );


  parcel[0].center.x = theBestCenter.x + parcel[0].xmin;
  parcel[0].center.y = theBestCenter.y + parcel[0].ymin;
  parcel[0].center.z = theBestCenter.z + parcel[0].zmin;

  free( theLocalCriteria );
  free( theLocalDist );
  free( theLocalLabels );

  return( 1 );
}



typedef struct _getCentersFromParcelsParam {
    u16 *theDist;
    void * theLabels;
    bufferType theLabelsType;
    int *theDim;
    typeParcel *parcel;
    typeChamferMask *theMask;
} _getCentersFromParcelsParam;



static void * _getCentersFromParcels( void* par )
{
    char *proc = "_getCentersFromParcels";
    typeChunk *chunk = (typeChunk *)par;
    void *parameter = chunk->parameters;
    size_t first = chunk->first;
    size_t last = chunk->last;

    u16 *theDist = ((_getCentersFromParcelsParam*)parameter)->theDist;
    void * theLabels = ((_getCentersFromParcelsParam*)parameter)->theLabels;
    bufferType theLabelsType = ((_getCentersFromParcelsParam*)parameter)->theLabelsType;
    int *theDim = ((_getCentersFromParcelsParam*)parameter)->theDim;
    typeParcel *parcel = ((_getCentersFromParcelsParam*)parameter)->parcel;
    typeChamferMask *theMask = ((_getCentersFromParcelsParam*)parameter)->theMask;

    size_t i;
    int v;


#define _GETCENTERSFROMPARCELS( TYPE ) {                                                                \
    TYPE *theBuf = (TYPE*)theLabels;                                                                    \
    for ( i=first; i<=last; i++ ) {                                                                     \
      parcel[i].x = (int)( parcel[i].sx/(double)parcel[i].n + 0.5 );                                    \
      parcel[i].y = (int)( parcel[i].sy/(double)parcel[i].n + 0.5 );                                    \
      parcel[i].z = (int)( parcel[i].sz/(double)parcel[i].n + 0.5 );                                    \
      v = (parcel[i].z * theDim[1] + parcel[i].y) * theDim[0] + parcel[i].x;                            \
      if ( !_force_exact_center_calculation_ && theBuf[v] == (i+1) ) {                                  \
        parcel[i].center.x = parcel[i].x;                                                               \
        parcel[i].center.y = parcel[i].y;                                                               \
        parcel[i].center.z = parcel[i].z;                                                               \
      }                                                                                                 \
      else {                                                                                            \
        if ( _verbose_ >= 2 )                                                                           \
          fprintf( stderr, "%s: switch to iterative method for parcel %lu\n", proc, i);                 \
        if ( _get_parcel_center( &(parcel[i]), i+1,                                                     \
                                 theDist, theLabels, theLabelsType, theDim,                             \
                                 theMask ) <= 0 ) {                                                     \
          if ( _verbose_ )                                                                              \
            fprintf( stderr, "%s: iterative calculation of center failed for parcel %lu\n", proc, i+1 ); \
          chunk->ret = -1;                                                                              \
          return( (void*)NULL );                                                                        \
        }                                                                                               \
      }                                                                                                 \
    }                                                                                                   \
}

    switch( theLabelsType ) {
    default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such label type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
    case UCHAR :
        _GETCENTERSFROMPARCELS( u8 );
        break;
    case USHORT :
        _GETCENTERSFROMPARCELS( u16 );
        break;
    case UINT :
        _GETCENTERSFROMPARCELS( u32 );
        break;
    }

    chunk->ret = 1;
    return( (void*)NULL );
}



static int _get_centers_from_parcels( u16* theDist, void* theLabels, bufferType theLabelsType, int *theDim,
                                      typeParcel *parcel, int nparcels, typeChamferMask *theMask)
{
  char *proc = "_get_centers_from_parcels";
  int i, x, y, z, l;

  typeChunks chunks;
  int n;
  size_t first = 0;
  size_t last = nparcels - 1;
  _getCentersFromParcelsParam p;

  /* initialization of the bounding box and sums
   */
  for ( i=0; i<nparcels; i++ ) {
    parcel[i].xmin = parcel[i].xmax = parcel[i].center.x;
    parcel[i].ymin = parcel[i].ymax = parcel[i].center.y;
    parcel[i].zmin = parcel[i].zmax = parcel[i].center.z;
    parcel[i].n = 0;
    parcel[i].sx = parcel[i].sy = parcel[i].sz = 0.0;
  }

#define _BOUNDINGBOXSUMS( TYPE ) {                          \
  TYPE *theBuf = (TYPE*)theLabels;                          \
  for ( i=0, z=0; z<theDim[2]; z++ )                        \
  for ( y=0; y<theDim[1]; y++ )                             \
  for ( x=0; x<theDim[0]; x++, i++ ) {                      \
    if ( theBuf[i] == (TYPE)0 || theBuf[i] > (TYPE)nparcels ) continue; \
    l = theBuf[i] - 1;                                      \
    if ( parcel[l].xmin > x ) parcel[l].xmin = x;           \
    if ( parcel[l].xmax < x ) parcel[l].xmax = x;           \
    if ( parcel[l].ymin > y ) parcel[l].ymin = y;           \
    if ( parcel[l].ymax < y ) parcel[l].ymax = y;           \
    if ( parcel[l].zmin > z ) parcel[l].zmin = z;           \
    if ( parcel[l].zmax < z ) parcel[l].zmax = z;           \
    parcel[l].n ++;                                         \
    parcel[l].sx += x;                                      \
    parcel[l].sy += y;                                      \
    parcel[l].sz += z;                                      \
  }                                                         \
}

  switch( theLabelsType ) {
  default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such label type not handled yet\n", proc );
      }
      return( -1 );
  case UCHAR :
      _BOUNDINGBOXSUMS( u8 );
      break;
  case USHORT :
      _BOUNDINGBOXSUMS( u16 );
      break;
  case UINT :
      _BOUNDINGBOXSUMS( u32 );
      break;
  }

  p.theDist = theDist;
  p.theLabels = theLabels;
  p.theLabelsType = theLabelsType;
  p.theDim = theDim;
  p.parcel = parcel;
  p.theMask = theMask;

  initChunks( &chunks );

  if ( 0 && _debug_ ) {
      if ( allocBuildOneChunk( &chunks, first, last ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to compute chunks\n", proc );
          return( -1 );
      }
  }
  else {
      if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute chunks\n", proc );
        return( -1 );
      }
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    chunks.data[n].parameters = (void*)(&p);
  }

  if ( processChunks( &_getCentersFromParcels, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute centers from parcels\n", proc );
    freeChunks( &chunks );
    return( -1 );
  }

  freeChunks( &chunks );

  return( 1 );
}





static int _getChangingNeighbors( void* theLabels, bufferType theLabelsType, int *theDim,
                                  typeParcel *parcel )
{
    char *proc = "_getChangingNeighbors";
    int i, j;
    int x, y, z;
    int u, v, w;

#define _GETCHANGINGNEIGHBORS( TYPE ) {                                         \
  TYPE *theBuf = (TYPE*)theLabels;                                              \
  if ( theDim[2] > 1 ) {                                                        \
      for ( i=0, z=0; z<theDim[2]; z++ )                                        \
      for ( y=0; y<theDim[1]; y++ )                                             \
      for ( x=0; x<theDim[0]; x++, i++ ) {                                      \
          if ( theBuf[i] == 0 ) continue;                                       \
          if ( parcel[ theBuf[i]-1 ].status == NEIGHBOR                         \
               || parcel[ theBuf[i]-1 ].status == UNCHANGED ) continue;         \
          if ( x > 0 && x < theDim[0]-1                                         \
               && y > 0 && y < theDim[1]-1                                      \
               && z > 0 && z < theDim[2]-1 ) {                                  \
              for ( w=-1; w<=1; w++ )                                           \
              for ( v=-1; v<=1; v++ )                                           \
              for ( u=-1; u<=1; u++ ) {                                         \
                  if ( w == 0 && v == 0 && u == 0 ) continue;                   \
                  j = ((z+w)*theDim[1] + y+v)*theDim[0] + x+u;                  \
                  if ( theBuf[j] == 0 ) continue;                               \
                  if ( parcel[ theBuf[j]-1 ].status == UNCHANGED )              \
                      parcel[ theBuf[j]-1 ].status = NEIGHBOR;                  \
              }                                                                 \
          }                                                                     \
          else {                                                                \
              for ( w=-1; w<=1; w++ ) {                                         \
                  if ( z+w < 0 || z+w >= theDim[2] ) continue;                  \
                  for ( v=-1; v<=1; v++ ) {                                     \
                      if ( y+v < 0 || y+v >= theDim[1] ) continue;              \
                      for ( u=-1; u<=1; u++ ) {                                 \
                          if ( x+u < 0 || x+u >= theDim[0] ) continue;          \
                          if ( w == 0 && v == 0 && u == 0 ) continue;           \
                          j = ((z+w)*theDim[1] + y+v)*theDim[0] + x+u;          \
                          if ( theBuf[j] == 0 ) continue;                       \
                          if ( parcel[ theBuf[j]-1 ].status == UNCHANGED )      \
                              parcel[ theBuf[j]-1 ].status = NEIGHBOR;          \
                      }                                                         \
                  }                                                             \
              }                                                                 \
          }                                                                     \
      }                                                                         \
  }                                                                             \
  else {                                                                        \
      for ( i=0, y=0; y<theDim[1]; y++ )                                        \
      for ( x=0; x<theDim[0]; x++, i++ ) {                                      \
          if ( theBuf[i] == 0 ) continue;                                       \
          if ( parcel[ theBuf[i]-1 ].status == NEIGHBOR                         \
               || parcel[ theBuf[i]-1 ].status == UNCHANGED ) continue;         \
          if ( x > 0 && x < theDim[0]-1                                         \
               && y > 0 && y < theDim[1]-1 ) {                                  \
              for ( v=-1; v<=1; v++ )                                           \
              for ( u=-1; u<=1; u++ ) {                                         \
                  if ( v == 0 && u == 0 ) continue;                             \
                  j = (y+v)*theDim[0] + x+u;                                    \
                  if ( theBuf[j] == 0 ) continue;                               \
                  if ( parcel[ theBuf[j]-1 ].status == UNCHANGED )              \
                      parcel[ theBuf[j]-1 ].status = NEIGHBOR;                  \
              }                                                                 \
          }                                                                     \
          else {                                                                \
              for ( v=-1; v<=1; v++ ) {                                         \
                  if ( y+v < 0 || y+v >= theDim[1] ) continue;                  \
                  for ( u=-1; u<=1; u++ ) {                                     \
                      if ( x+u < 0 || x+u >= theDim[0] ) continue;              \
                      if ( v == 0 && u == 0 ) continue;                         \
                      j = (y+v)*theDim[0] + x+u;                                \
                      if ( theBuf[j] == 0 ) continue;                           \
                      if ( parcel[ theBuf[j]-1 ].status == UNCHANGED )          \
                          parcel[ theBuf[j]-1 ].status = NEIGHBOR;              \
                  }                                                             \
              }                                                                 \
          }                                                                     \
      }                                                                         \
  }                                                                             \
}

    switch( theLabelsType ) {
    default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such label type not handled yet\n", proc );
        }
        return( -1 );
    case UCHAR :
        _GETCHANGINGNEIGHBORS( u8 );
        break;
    case USHORT :
        _GETCHANGINGNEIGHBORS( u16 );
        break;
    case UINT :
        _GETCHANGINGNEIGHBORS( u32 );
        break;
    }

    return( 1 );
}










/************************************************************
 *
 *
 *
 ************************************************************/

int parcelling( void *theBuf, bufferType theBufType, 
                  int **theSeeds, int nparcels,
                  void *theOutputLabels, bufferType theOutputLabelsType,
                  void *theOutputDistance, bufferType theOutputDistanceType,
                  int *theDim, int inSeeds, typeChamferMask *chamferMask )
{
  char *proc = "parcelling";
  typeParcel *theParcel;
  /*
  typeParcelCenter *centers;
  typeParcelCenter *theCenter, *oldCenter;
  */
  void *theLabels;
  u16 *theDist;
  bufferType theLabelsType = TYPE_UNKNOWN;


  int i;
  int iteration;
  int nchanges = 0;
  int noscillations = 0;
  int nunchanges = 0;
  int nneighbors = 0;
  int partialupdate = 0;

  typeChamferMask theMask, *ptrMask;
  double voxel_size[3] = {1.0, 1.0, 1.0};

  if ( chamferMask == NULL ) {
      initChamferMask( &theMask );
      if ( buildChamferMask( voxel_size, (theDim[2] == 1) ? 2 : 3,
                             3, 3, _CHAMFER3x3x3_, &theMask ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to build chamfer mask\n", proc );
          return( -1 );
      }
      ptrMask = &theMask;
  }
  else {
      ptrMask = chamferMask;
  }


  /*
  int inc[3][3][3] = {{{5,4,5},{4,3,4},{5,4,5}},{{4,3,4},{3,0,3},{4,3,4}},{{5,4,5},{4,3,4},{5,4,5}}};
  */


  if ( 0 && _check_free_borders( theBuf, theBufType, theDim ) != 1 ) {
    /* il faudrait en fait encapsuler cette fonction et l'appeler
       avec des images plus grandes si necessaire
    */
      if ( chamferMask == NULL ) freeChamferMask( &theMask );
      if ( _verbose_ )
        fprintf( stderr, "%s: non-empty borders not handled yet\n", proc );
      return( -1 );
  }


  /* offsets for chamfer mask
   */
  for ( i=0; i<ptrMask->nb; i++ ) {
    ptrMask->list[i].o = ptrMask->list[i].z * theDim[1]*theDim[0]  + ptrMask->list[i].y * theDim[0] + ptrMask->list[i].x;
  }


  /* some allocations
   * - 2 arrays of parcel centers
   * - 1 data array of labels (2 more labels are required)
   * - 1 data array of distance
   */

  theParcel = (typeParcel *)malloc( nparcels*sizeof( typeParcel ) );
  if ( theParcel == (typeParcel *)NULL ) {
      if ( chamferMask == NULL ) freeChamferMask( &theMask );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate parcel list\n", proc );
      return( -1 );

  }
  for ( i=0; i<nparcels; i++ ) {
      _initParcel( &(theParcel[i]) );
  }

  /*
  theCenter = centers;
  oldCenter = centers + nparcels;
  */
  
  if ( nparcels+2 <= 255 ) {
      theLabelsType = UCHAR;
      theLabels = malloc( theDim[2]*theDim[1]*theDim[0]*sizeof(u8) );
      if ( theLabels == (void*)NULL ) {
          free( theParcel );
          if ( chamferMask == NULL ) freeChamferMask( &theMask );
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to allocate label array\n", proc );
          return( -1 );
      }
  }
  else if ( nparcels+2 <= 65535 ) {
      theLabelsType = USHORT;
      theLabels = malloc( theDim[2]*theDim[1]*theDim[0]*sizeof(u16) );
      if ( theLabels == (void*)NULL ) {
          free( theParcel );
          if ( chamferMask == NULL ) freeChamferMask( &theMask );
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to allocate label array\n", proc );
          return( -1 );
      }
  }
  else {
      theLabelsType = UINT;
      theLabels = malloc( theDim[2]*theDim[1]*theDim[0]*sizeof(u32) );
      if ( theLabels == (void*)NULL ) {
          free( theParcel );
          if ( chamferMask == NULL ) freeChamferMask( &theMask );
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to allocate label array\n", proc );
          return( -1 );
      }
  }

  theDist = (u16*)malloc( theDim[2]*theDim[1]*theDim[0]*sizeof(u16) );
  if ( theDist == (u16*)NULL ) {
      free( theLabels );
      free( theParcel );
      if ( chamferMask == NULL ) freeChamferMask( &theMask );
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate distance array\n", proc );
      return( -1 );
  }



  /* put initial data into data array
  */
  if ( ConvertBuffer( theBuf, theBufType, theLabels, theLabelsType, theDim[2]*theDim[1]*theDim[0] ) != 1 ) {
    free( theDist );
    free( theLabels );
    free( theParcel );
    if ( chamferMask == NULL ) freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to convert buffer\n", proc );
    return( -1 );
  }


  
  /* draw random initial seeds (if necessary)
     (assume that foreground points are non-null points)
  */

  if ( theSeeds != (int**) NULL && inSeeds == 1 ) {
    for ( i=0; i<nparcels; i++ ) {
      theParcel[i].center.x = theSeeds[i][0];
      theParcel[i].center.y = theSeeds[i][1];
      if ( theDim[2] > 1 )
         theParcel[i].center.z = theSeeds[i][2];
      else 
         theParcel[i].center.z = 0;
    }
  }
  else {
      if ( _get_initial_parcel_centers( theLabels, theLabelsType, theDim, theParcel, nparcels ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to get initial centers\n", proc );
        free( theDist );
        free( theLabels );
        free( theParcel );
        if ( chamferMask == NULL ) freeChamferMask( &theMask );
        return( -1 );
      }
  }

 
  


  iteration = 0;
  do {

      /* compute geodesic influence areas of parcels
       */
      if ( _get_parcels_from_centers( theDist, theLabels, theLabelsType, theDim,
                                      theParcel, nparcels, ptrMask, partialupdate ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to get parcels from centers\n", proc );
        free( theDist );
        free( theLabels );
        free( theParcel );
        if ( chamferMask == NULL ) freeChamferMask( &theMask );
        return( -1 );
      }

      /* if some iterations remains to do
         -> update centers
      */

      if ( (_max_iterations_ < 0) ||
           ( _max_iterations_ > 0 && iteration < _max_iterations_ ) ) {


          /* copy of parcel centers
           */
          for ( i=0; i<nparcels; i++ ) {
              theParcel[i].center2 = theParcel[i].center1;
              theParcel[i].center1 = theParcel[i].center;
          }

          /* recompute centers
           */
          if ( _get_centers_from_parcels( theDist, theLabels, theLabelsType, theDim,
                                          theParcel, nparcels, ptrMask ) != 1 ) {
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to get centers from parcels\n", proc );
            free( theDist );
            free( theLabels );
            free( theParcel );
            if ( chamferMask == NULL ) freeChamferMask( &theMask );
            return( -1 );
          }



          if ( 0 && _debug_ ) {
              for ( i=0; i<nparcels; i++ ) {
                  if ( theParcel[i].center.x != theParcel[i].center1.x
                       || theParcel[i].center.y != theParcel[i].center1.y
                       || theParcel[i].center.z != theParcel[i].center1.z ) {
                      fprintf( stderr, "#%3d: ", i );
                      fprintf( stderr, "%3d %3d %3d ", theParcel[i].center2.x,
                               theParcel[i].center2.y, theParcel[i].center2.z );
                      fprintf( stderr, "-> " );
                      fprintf( stderr, "%3d %3d %3d ", theParcel[i].center1.x,
                               theParcel[i].center1.y, theParcel[i].center1.z );
                      fprintf( stderr, "-> " );
                      fprintf( stderr, "%3d %3d %3d ", theParcel[i].center.x,
                               theParcel[i].center.y, theParcel[i].center.z );
                      fprintf( stderr, "\n" );

                  }
              }
          }


          nchanges = 0;
          noscillations = 0;
          for ( i=0; i<nparcels; i++ ) {
              theParcel[i].status = UNCHANGED;
              if ( theParcel[i].center.x != theParcel[i].center1.x
                   || theParcel[i].center.y != theParcel[i].center1.y
                   || theParcel[i].center.z != theParcel[i].center1.z ) {
                  if ( theParcel[i].center.x != theParcel[i].center2.x
                       || theParcel[i].center.y != theParcel[i].center2.y
                       || theParcel[i].center.z != theParcel[i].center2.z ) {
                      theParcel[i].status = CHANGING;
                      nchanges ++;
                  }
                  else {
                      theParcel[i].status = OSCILLATING;
                      noscillations ++;
                  }
              }
          }




          /* look at parcels in case of partial updates
           */
          /* if ( _allows_partial_update_ && nchanges < nparcels / 10 ) { */
          if ( _allows_partial_update_ ) {
              partialupdate = 0;
              if ( _getChangingNeighbors( theLabels, theLabelsType, theDim,
                                          theParcel ) != 1 ) {
                  if ( _verbose_ )
                    fprintf( stderr, "%s: unable to compute neighbors\n", proc );
                  free( theDist );
                  free( theLabels );
                  free( theParcel );
                  if ( chamferMask == NULL ) freeChamferMask( &theMask );
                  return( -1 );
              }
              nchanges = 0;
              noscillations = 0;
              nunchanges = 0;
              nneighbors = 0;
              for ( i=0; i<nparcels; i++ ) {
                  switch( theParcel[i].status ) {
                  default :
                      if ( _verbose_ )
                        fprintf( stderr, "%s: unknown parcel status, this is weird\n", proc );
                      free( theDist );
                      free( theLabels );
                      free( theParcel );
                      return( -1 );
                  case UNCHANGED :
                      nunchanges ++; break;
                  case CHANGING :
                      nchanges ++; break;
                  case OSCILLATING :
                      noscillations ++; break;
                  case NEIGHBOR :
                      nneighbors ++; break;
                  }
              }
              /* should we make a test here ?
               * if ( 1 || nunchanges > nparcels/4 )
               */
              if ( nunchanges > 0 )
                  partialupdate = 1;
          } else {
              for ( i=0; i<nparcels; i++ ) {
                  theParcel[i].status = CHANGING;
              }
          }

          if ( _verbose_ ) {
            fprintf( stderr, "%s: iteration #%2d, %4d changes, %4d oscillations", proc, iteration, nchanges, noscillations );
            if ( _allows_partial_update_ ) {
                fprintf( stderr, ", %4d neighbors, %4d unchanges", nneighbors, nunchanges );
            }
            fprintf( stderr, "\n" );
          }


      }






      iteration ++;


      /* stop if
         _max_iterations_ == 0 (we just want geodesic influence areas)
          OR (maximal number of iterations is positive and is reached)

          OR (there are no changes)
      */



  } while ( !( (_max_iterations_ == 0)
                || (_max_iterations_ > 0 && iteration > _max_iterations_)
                || (nchanges == 0) ) );
    

  if ( chamferMask == NULL ) freeChamferMask( &theMask );

  /* export results :
     - parcel centers
     - labels 
     - distance
  */
  

  if ( theSeeds != (int**) NULL ) {
    for ( i=0; i<nparcels; i++ ) {
      theSeeds[i][0] = theParcel[i].center.x;
      theSeeds[i][1] = theParcel[i].center.y;
      if ( theDim[2] > 1 )
         theSeeds[i][2] = theParcel[i].center.z;
      else
         theSeeds[i][2] = 0;
    }
  }

  free( theParcel );


  
  /* copy auxiliary label array
   */
  if ( theOutputLabels != NULL ) {
      if ( ConvertBuffer( theLabels, theLabelsType, theOutputLabels, theOutputLabelsType, theDim[2]*theDim[1]*theDim[0] ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to convert buffer\n", proc );
        free( theDist );
        free( theLabels );
        return( -1 );
      }
  }
  

  if ( theOutputDistance != NULL ) {
      if ( ConvertBuffer( theDist, USHORT, theOutputDistance, theOutputDistanceType, theDim[2]*theDim[1]*theDim[0] ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to convert buffer\n", proc );
        free( theDist );
        free( theLabels );
        return( -1 );
      }
  }

  free( theDist );
  free( theLabels );

  return( 1 );
}
