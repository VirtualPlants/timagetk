/****************************************************
 * topological-tree.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Mar 22 mar 2016 11:12:48 CET
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <string.h>

#include <connexe.h>
#include <topological-tree.h>








/**************************************************
 *
 *
 *
 **************************************************/


static int _verbose_ = 1;
static int _debug_ = 0;

void setVerboseInTopologicalTree( int v )
{
  _verbose_ = v;
}

void incrementVerboseInTopologicalTree(  )
{
  _verbose_ ++;
}

void decrementVerboseInTopologicalTree(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}

void setDebugInTopologicalTree( int d )
{
  _debug_ = d;
}

void incrementDebugInTopologicalTree(  )
{
  _debug_ ++;
}

void decrementDebugInTopologicalTree(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}








/**************************************************
 *
 *
 *
 **************************************************/



int countNeighborsInTreeImage( unsigned char *theBuf, unsigned char *resBuf, int *theDim )
{
  int x, y, z;
  int i, j, k;
  int n;
  int neighbors;

  /* 2D case
   */
  if ( theDim[2] == 1 ) {
    for ( n=0, y=0; y<theDim[1]; y++ )
    for ( x=0; x<theDim[0]; x++, n++ ) {
      if ( theBuf[n] == 0 ) {
        resBuf[n] = 0;
        continue;
      }
      neighbors = 0;
      for ( j=-1; j<=1; j++ ) {
        if ( y+j<0 || y+j>=theDim[1] ) continue;
        for ( i=-1; i<=1; i++ ) {
          if ( x+i<0 || x+i>=theDim[0] ) continue;
          if ( j == 0 && i == 0 ) continue; \
          if ( theBuf[n+j*theDim[0]+i] > 0 ) neighbors++;
        }
      }
      resBuf[n] = neighbors;
    }
  }

  /* 3D case
   */
  else {
    for ( n=0, z=0; z<theDim[2]; z++ )
    for ( y=0; y<theDim[1]; y++ )
    for ( x=0; x<theDim[0]; x++, n++ ) {
      if ( theBuf[n] == 0 ) {
        resBuf[n] = 0;
        continue;
      }
      neighbors = 0;
      for ( k=-1; k<=1; k++ ) {
        if ( z+k<0 || z+k>=theDim[2] ) continue;
        for ( j=-1; j<=1; j++ ) {
          if ( y+j<0 || y+j>=theDim[1] ) continue;
          for ( i=-1; i<=1; i++ ) {
            if ( x+i<0 || x+i>=theDim[0] ) continue;
            if ( k == 0 && j == 0 && i == 0 ) continue; \
            if ( theBuf[n+(k*theDim[1]+j)*theDim[0]+i] > 0 ) neighbors++;
          }
        }
      }
      resBuf[n] = neighbors;
    }
  }

  return( 1 );
}








/**************************************************
 *
 *
 *
 **************************************************/



void initComponentTreeImage( typeComponentTreeImage *c )
{
    c->firstComponentLabel = -1;
    c->lastComponentLabel = -1;
    c->firstJunctionLabel = -1;
    c->lastJunctionLabel = -1;
    c->componentBuf = (void*)NULL;
    c->componentType = TYPE_UNKNOWN;
    c->theDim[0] = 0;
    c->theDim[1] = 0;
    c->theDim[2] = 0;
    c->voxelSize[0] = 1.0;
    c->voxelSize[1] = 1.0;
    c->voxelSize[2] = 1.0;
}



int allocComponentTreeImage( typeComponentTreeImage *c, bufferType type, int *theDim )
{
  char *proc = "allocComponentTreeImage";
  switch( type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    c->componentBuf = (void*)malloc( theDim[0]*theDim[1]*theDim[2]*sizeof(u8) );
    if ( c->componentBuf == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation failed\n", proc );
      return( -1 );
    }
    break;
  case SSHORT :
    c->componentBuf = (void*)malloc( theDim[0]*theDim[1]*theDim[2]*sizeof(s16) );
    if ( c->componentBuf == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation failed\n", proc );
      return( -1 );
    }
    break;
  case USHORT :
    c->componentBuf = (void*)malloc( theDim[0]*theDim[1]*theDim[2]*sizeof(u16) );
    if ( c->componentBuf == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation failed\n", proc );
      return( -1 );
    }
    break;
  }

  c->componentType = type;
  c->theDim[0] = theDim[0];
  c->theDim[1] = theDim[1];
  c->theDim[2] = theDim[2];

  return( 1 );
}



void freeComponentTreeImage( typeComponentTreeImage *c )
{
    if ( c->componentBuf != (void*)NULL )
        free( c->componentBuf );
    initComponentTreeImage( c );
}



static int fprintfComponentTreeImage( FILE *f, typeComponentTreeImage *c )
{
  char *proc = "fprintfComponentTreeImage";
  int *size;
  int i, v;

  size = (int*)malloc( (c->lastJunctionLabel+1) * sizeof(int) );
  if ( size == (int*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }

  v = c->theDim[0] * c->theDim[1] * c->theDim[2];
  for ( i=0; i<=c->lastJunctionLabel; i++ )
    size[i] = 0;

#define FPRINTFCOMPONENTTREEIMAGE( TYPE ) { \
  TYPE *theBuf = (TYPE*)c->componentBuf;    \
  for ( i=0; i<v; i++ ) {                   \
    size[ theBuf[i] ] ++;                   \
  }                                         \
}

  switch( c->componentType ) {
  default :
    free( size );
    if ( _verbose_ )
        fprintf( stderr, "%s: such component type not handled yet\n", proc );
      return( -1 );
  case UCHAR :
    FPRINTFCOMPONENTTREEIMAGE( u8 );
    break;
  case SSHORT :
    FPRINTFCOMPONENTTREEIMAGE( s16 );
    break;
  case USHORT :
    FPRINTFCOMPONENTTREEIMAGE( u16 );
    break;
  }

  fprintf( f, "--- edges ---\n" );
  for ( i=c->firstComponentLabel; i<=c->lastComponentLabel; i++ )
    fprintf( f, "#%3d has size %5d\n", i, size[i] );
  fprintf( f, "--- junctions ---\n" );
  for ( i=c->firstJunctionLabel; i<=c->lastJunctionLabel; i++ )
    fprintf( f, "#%3d has size %5d\n", i, size[i] );

  free( size );
  return( 1 );
}



int labelComponentsInTreeImage( unsigned char *theBuf,
                           typeComponentTreeImage *tree )
{
  char *proc = "labelComponentsInTreeImage";
  int i, v;
  int theDim[3];
  unsigned char *tmpBuf;
  unsigned short int *tmpComponents;
  int r;

  /* allocations
   */
  for (i=0; i<3; i++) theDim[i] = tree->theDim[i];
  v = theDim[2] * theDim[1] * theDim[0];

  tmpBuf = (unsigned char*)malloc( v * sizeof(unsigned char) );
  if ( tmpBuf == (unsigned char*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary input buffer\n", proc );
    return( -1 );
  }

  tmpComponents = (unsigned short int*)malloc( v * sizeof(unsigned short int) );
  if ( tmpComponents == (unsigned short int*)NULL ) {
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary output buffer\n", proc );
    return( -1 );
  }

  /* tree branches
   */
  countNeighborsInTreeImage( theBuf, tmpBuf, theDim );
  for ( i=0; i<v; i++ ) {
      tmpBuf[i] = ( tmpBuf[i] == 1 || tmpBuf[i] == 2 ) ? 255 : 0;
  }
  r = CountConnectedComponents( tmpBuf, UCHAR, tmpComponents, USHORT, theDim );
  if ( r == -1 ) {
    free( tmpComponents );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when counting tree branch components\n", proc );
    return( -1 );
  }
  if ( r == 0 ) {
    free( tmpComponents );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: no tree branch components\n", proc );
    return( -1 );
  }
  if ( _debug_ ) {
      fprintf( stderr, "%s: found %d tree branches\n", proc, r );
  }

  switch( tree->componentType ) {
  default :
    free( tmpComponents );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: such component type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theCC = (unsigned char*)tree->componentBuf;
      if ( r > 255 ) {
        free( tmpComponents );
        free( tmpBuf );
        if ( _verbose_ )
          fprintf( stderr, "%s: too many components for the component type\n", proc );
        return( -1 );
      }
      for ( i=0; i<v; i++ ) {
        theCC[i] = tmpComponents[i];
      }
    }
    break;
  case SSHORT :
    {
      short int *theCC = (short int*)tree->componentBuf;
      if ( r > 32767 ) {
        free( tmpComponents );
        free( tmpBuf );
        if ( _verbose_ )
          fprintf( stderr, "%s: too many components for the component type\n", proc );
        return( -1 );
      }
      for ( i=0; i<v; i++ ) {
        theCC[i] = tmpComponents[i];
      }
    }
    break;
  case USHORT :
    {
      unsigned short int *theCC = (unsigned short int*)tree->componentBuf;
      if ( r > 65535 ) {
        free( tmpComponents );
        free( tmpBuf );
        if ( _verbose_ )
          fprintf( stderr, "%s: too many components for the component type\n", proc );
        return( -1 );
      }
      for ( i=0; i<v; i++ ) {
        theCC[i] = tmpComponents[i];
      }
    }
    break;
  }

  tree->firstComponentLabel = 1;
  tree->lastComponentLabel = r;


  /* tree junctions
   */
  countNeighborsInTreeImage( theBuf, tmpBuf, theDim );
  for ( i=0; i<v; i++ ) {
      tmpBuf[i] = ( tmpBuf[i] >= 3 ) ? 255 : 0;
  }
  r = CountConnectedComponents( tmpBuf, UCHAR, tmpComponents, USHORT, theDim );
  if ( r == -1 ) {
    free( tmpComponents );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when counting tree branch junctions\n", proc );
    return( -1 );
  }
  if ( r == 0 ) {
    free( tmpComponents );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: no tree branch junctions\n", proc );
    return( -1 );
  }
  if ( _debug_ ) {
      fprintf( stderr, "%s: found %d tree junctions\n", proc, r );
  }

  switch( tree->componentType ) {
  default :
    free( tmpComponents );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: such component type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theCC = (unsigned char*)tree->componentBuf;
      if ( r+tree->lastComponentLabel > 255 ) {
        free( tmpComponents );
        free( tmpBuf );
        if ( _verbose_ )
          fprintf( stderr, "%s: too many components for the component type\n", proc );
        return( -1 );
      }
      for ( i=0; i<v; i++ ) {
        if ( tmpComponents[i] > 0 )
          theCC[i] = tmpComponents[i]+tree->lastComponentLabel;
      }
    }
    break;
  case SSHORT :
    {
      short int *theCC = (short int*)tree->componentBuf;
      if ( r+tree->lastComponentLabel > 32767 ) {
        free( tmpComponents );
        free( tmpBuf );
        if ( _verbose_ )
          fprintf( stderr, "%s: too many components for the component type\n", proc );
        return( -1 );
      }
      for ( i=0; i<v; i++ ) {
        if ( tmpComponents[i] > 0 )
          theCC[i] = tmpComponents[i]+tree->lastComponentLabel;
      }
    }
    break;
  case USHORT :
    {
      unsigned short int *theCC = (unsigned short int*)tree->componentBuf;
      if ( r+tree->lastComponentLabel > 65535 ) {
        free( tmpComponents );
        free( tmpBuf );
        if ( _verbose_ )
          fprintf( stderr, "%s: too many components for the component type\n", proc );
        return( -1 );
      }
      for ( i=0; i<v; i++ ) {
        if ( tmpComponents[i] > 0 )
          theCC[i] = tmpComponents[i]+tree->lastComponentLabel;
      }
    }
    break;
  }

  tree->firstJunctionLabel = 1+tree->lastComponentLabel;
  tree->lastJunctionLabel = r+tree->lastComponentLabel;

  if ( _verbose_ >= 2 ) {
      fprintf( stderr, "%s: edge labels in [%d %d]\n", proc,
               tree->firstComponentLabel, tree->lastComponentLabel );
      fprintf( stderr, "\t junctions labels in [%d %d]\n",
               tree->firstJunctionLabel, tree->lastJunctionLabel );
      if ( 0 && _debug_ )
          fprintfComponentTreeImage( stderr, tree );
  }


  free( tmpComponents );
  free( tmpBuf );
  return( 1 );
}








/**************************************************
 *
 *
 *
 **************************************************/



typedef enum _enumComponent {
  _UNKNOWN_COMPONENT_,
  _EDGE_,
  _JUNCTION_
} _enumComponent;



typedef struct _internIntPoint {
  int x;
  int y;
  int z;
} _internIntPoint;

typedef struct _internComponent {
  _enumComponent type;
  int n_data;
  int n_allocated_data;
  typeTreeFloatPoint *data;
  typeTreeFloatPoint center;
  int labelFirstNeighbor;
  int labelLastNeighbor;
  int pointIndexOffset;
} _internComponent;

typedef struct _internTree {
  int n_data;
  int n_allocated_data;
  _internComponent *data;
} _internTree;



static void _initInternIntPoint( _internIntPoint *p )
{
  p->x = -1;
  p->y = -1;
  p->z = -1;
}



static void _initInternComponent( _internComponent *c )
{
  c->type = _UNKNOWN_COMPONENT_;
  c->n_data = 0;
  c->n_allocated_data = 0;
  c->data = (typeTreeFloatPoint*)NULL;
  initTreeFloatPoint( &(c->center) );
  c->labelFirstNeighbor = -1;
  c->labelLastNeighbor = -1;
  c->pointIndexOffset = 0;
}



static void _freeInternComponent( _internComponent *c )
{
  if ( c->data != (typeTreeFloatPoint*)NULL )
      free( c->data );
  _initInternComponent( c );
}



static int _size_to_be_allocated_ = 10;

static int _addPointToInternComponent( _internComponent *l, _internIntPoint *p )
{
  char *proc = "_addPointToInternComponent";
  int s =  l->n_allocated_data;
  typeTreeFloatPoint *data;

  if ( l->n_data == l->n_allocated_data ) {
    s += _size_to_be_allocated_;
    data = (typeTreeFloatPoint*)malloc( s * sizeof(typeTreeFloatPoint) );
    if ( data == (typeTreeFloatPoint*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
    }
    if ( l->n_allocated_data > 0 ) {
      (void)memcpy( data, l->data, l->n_allocated_data*sizeof(typeTreeFloatPoint) );
      free( l->data );
    }
    l->n_allocated_data = s;
    l->data = data;
  }

  initTreeFloatPoint( &(l->data[l->n_data]) );
  l->data[l->n_data].x = p->x;
  l->data[l->n_data].y = p->y;
  l->data[l->n_data].z = p->z;
  l->n_data ++;

  return( 1 );
}



static void _initInternTree( _internTree *t )
{
  t->n_data = 0;
  t->n_allocated_data = 0;
  t->data = (_internComponent*)NULL;
}



static void _freeInternTree( _internTree *t )
{
  int i;
  if ( t->data != (_internComponent*)NULL ) {
    for ( i=0; i<t->n_data; i++ )
      _freeInternComponent( &(t->data[i]) );
    free( t->data );
  }
  _initInternTree( t );
}



static int _allocInternTree( _internTree *t, int n )
{
  char *proc = "_allocInternTree";
  int i;

  t->data = (_internComponent*)malloc( n*sizeof(_internComponent) );
  if ( t->data == (_internComponent*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }

  t->n_data = n;
  t->n_allocated_data = n;
  for ( i=0; i<n; i++ )
    _initInternComponent( &(t->data[i]) );

  return( 1 );
}



static void _printInternTree( FILE *f, _internTree *tree )
{
  int nedges;
  int njunctions;
  int i, n;

  for ( i=1, nedges=0, njunctions=0; i<tree->n_data; i++ ) {
    if ( tree->data[i].type == _EDGE_) nedges++;
    if ( tree->data[i].type == _JUNCTION_) njunctions++;
  }

  fprintf( f, "=== internal tree\n" );
  fprintf( f, "=== %d components, %d edges, %d junctions\n",
           tree->n_data, nedges, njunctions );
  fprintf( f, "--- edges ---\n" );
  for ( i=1; i<tree->n_data; i++ ) {
    if ( tree->data[i].type != _EDGE_ ) continue;
    n = tree->data[i].n_data - 1;
    fprintf( f, "    #%3d: %d points, ",
             i, tree->data[i].n_data );
    if ( tree->data[i].labelFirstNeighbor == -1 )
        fprintf( f, "NULL" );
    else
        fprintf( f, "%4d", tree->data[i].labelFirstNeighbor );
    fprintf( f, " <- " );
    fprintf( f, "[(%.1f %.1f %.1f), ..., (%.1f %.1f %.1f)]",
             tree->data[i].data[0].x, tree->data[i].data[0].y, tree->data[i].data[0].z,
             tree->data[i].data[n].x, tree->data[i].data[n].y, tree->data[i].data[n].z );
    fprintf( f, " -> " );
    if ( tree->data[i].labelLastNeighbor == -1 )
        fprintf( f, "NULL" );
    else
        fprintf( f, "%4d", tree->data[i].labelLastNeighbor );
    fprintf( f, "\n" );
  }
  fprintf( f, "--- junctions ---\n" );
  for ( i=1; i<tree->n_data; i++ ) {
    if ( tree->data[i].type != _JUNCTION_ ) continue;
    fprintf( f, "    #%3d: %d points (%.1f %.1f %.1f)\n",
             i, tree->data[i].n_data,
             tree->data[i].center.x, tree->data[i].center.y, tree->data[i].center.z );
  }
  fprintf( f, "--- others ---\n" );
  for ( i=1; i<tree->n_data; i++ ) {
    if ( tree->data[i].type == _EDGE_ || tree->data[i].type == _JUNCTION_ ) continue;
    fprintf( f, "    #%3d: %d points\n",
             i, tree->data[i].n_data );
  }
}



static int _countSameNeighors( typeComponentTreeImage *treeImage,
                               _internIntPoint *p )
{
  char *proc = "_countSameNeighors";
  int n = 0;
  int i, j, k;
  int a, b;
  int x = p->x;
  int y = p->y;
  int z = p->z;

  a = (z * treeImage->theDim[1] + y) * treeImage->theDim[0] + x;

#define _COUNTSAMENEIGHBORS( TYPE ) {                         \
  TYPE *theCC = (TYPE*)treeImage->componentBuf;               \
  if ( treeImage->theDim[2] == 1 ) {                          \
    for ( n=0, j=-1; j<=1; j++ ) {                            \
      if ( y+j<0 || y+j>=treeImage->theDim[1] ) continue;     \
      for ( i=-1; i<=1; i++ ) {                               \
        if ( x+i<0 || x+i>=treeImage->theDim[0] ) continue;   \
        b = a + j * treeImage->theDim[0] + i;                 \
        if ( theCC[b] == theCC[a] ) n++;                      \
      }                                                       \
    }                                                         \
  }                                                           \
  else {                                                      \
    for ( n=0, k=-1; k<=1; k++ ) {                            \
      if ( z+k<0 || z+k>=treeImage->theDim[2] ) continue;     \
      for ( j=-1; j<=1; j++ ) {                               \
        if ( y+j<0 || y+j>=treeImage->theDim[1] ) continue;   \
        for ( i=-1; i<=1; i++ ) {                             \
          if ( x+i<0 || x+i>=treeImage->theDim[0] ) continue; \
          b = a + (k * treeImage->theDim[1] + j) * treeImage->theDim[0] + i; \
          if ( theCC[b] == theCC[a] ) n++;                    \
        }                                                     \
      }                                                       \
    }                                                         \
  }                                                           \
}

  switch( treeImage->componentType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such tree image type not handled yet\n", proc );
    return( -1 );

  case UCHAR :
    _COUNTSAMENEIGHBORS( u8 );
    return( n-1 );
    break;

  case SSHORT :
    _COUNTSAMENEIGHBORS( s16 );
    return( n-1 );
    break;

  case USHORT :
    _COUNTSAMENEIGHBORS( u16 );
    return( n-1 );
    break;
  }

  return( -1 );
}



static int _extractEdgeNeighbors( typeComponentTreeImage *treeImage,
                              _internIntPoint *center,
                              _internIntPoint *p,
                              _internIntPoint *l )
{
  char *proc = "_extractEdgeNeighbors";
  int n = 0;
  int i, j, k;
  int a, b, label;
  int x = center->x;
  int y = center->y;
  int z = center->z;

  a = (z * treeImage->theDim[1] + y) * treeImage->theDim[0] + x;

#define _EXTRACTEDGENEIGHBORS( TYPE ) {                       \
  TYPE *theCC = (TYPE*)treeImage->componentBuf;               \
  label = theCC[a];                                           \
  if ( treeImage->theDim[2] == 1 ) {                          \
    for ( n=0, j=-1; j<=1; j++ ) {                            \
      if ( y+j<0 || y+j>=treeImage->theDim[1] ) continue;     \
      for ( i=-1; i<=1; i++ ) {                               \
        if ( x+i<0 || x+i>=treeImage->theDim[0] ) continue;   \
        if ( j == 0 && i == 0 ) continue;                     \
        b = a + j * treeImage->theDim[0] + i;                 \
        if ( theCC[b] == label ) {                            \
          if ( n == 0 ) {                                     \
            p->x = x+i;   p->y = y+j;   p->z = z;             \
          }                                                   \
          else if ( n == 1 ) {                                \
            l->x = x+i;   l->y = y+j;   l->z = z;             \
          }                                                   \
          n++;                                                \
        }                                                     \
      }                                                       \
    }                                                         \
  }                                                           \
  else {                                                      \
    for ( n=0, k=-1; k<=1; k++ ) {                            \
      if ( z+k<0 || z+k>=treeImage->theDim[2] ) continue;     \
      for ( j=-1; j<=1; j++ ) {                               \
        if ( y+j<0 || y+j>=treeImage->theDim[1] ) continue;   \
        for ( i=-1; i<=1; i++ ) {                             \
          if ( x+i<0 || x+i>=treeImage->theDim[0] ) continue; \
          if ( k == 0 && j == 0 && i == 0 ) continue;         \
          b = a + (k * treeImage->theDim[1] + j) * treeImage->theDim[0] + i; \
          if ( theCC[b] == label ) {                          \
            if ( n == 0 ) {                                   \
              p->x = x+i;   p->y = y+j;   p->z = z + k;       \
            }                                                 \
            else if ( n == 1 ) {                              \
              l->x = x+i;   l->y = y+j;   l->z = z + k;       \
            }                                                 \
            n++;                                              \
          }                                                   \
        }                                                     \
      }                                                       \
    }                                                         \
  }                                                           \
}

  switch( treeImage->componentType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such tree image type not handled yet\n", proc );
    return( -1 );

  case UCHAR :
    _EXTRACTEDGENEIGHBORS( u8 );
    return( n );
    break;

  case SSHORT :
    _EXTRACTEDGENEIGHBORS( s16 );
    return( n );
    break;

  case USHORT :
    _EXTRACTEDGENEIGHBORS( u16 );
    return( n );
    break;
  }

  return( -1 );
}



static int _extractComponentNeighbors( typeComponentTreeImage *treeImage,
                                       _internIntPoint *center,
                                       int *l1, int *l2 )
{
  char *proc = "_extractComponentNeighbors";
  int n = 0;
  int i, j, k;
  int a, b, label;
  int x = center->x;
  int y = center->y;
  int z = center->z;

  *l1 = *l2 = -1;
  a = (z * treeImage->theDim[1] + y) * treeImage->theDim[0] + x;

#define _EXTRACTCOMPONENTNEIGHBORS( TYPE ) {                  \
  TYPE *theCC = (TYPE*)treeImage->componentBuf;               \
  label = theCC[a];                                           \
  if ( treeImage->theDim[2] == 1 ) {                          \
    for ( n=0, j=-1; j<=1; j++ ) {                            \
      if ( y+j<0 || y+j>=treeImage->theDim[1] ) continue;     \
      for ( i=-1; i<=1; i++ ) {                               \
        if ( x+i<0 || x+i>=treeImage->theDim[0] ) continue;   \
        if ( j == 0 && i == 0 ) continue;                     \
        b = a + j * treeImage->theDim[0] + i;                 \
        if ( theCC[b] > 0 && theCC[b] != label ) {            \
          if ( n == 0 ) {                                     \
            *l1 = theCC[b];                                   \
          }                                                   \
          else if ( n == 1 ) {                                \
            *l2 = theCC[b];                                   \
          }                                                   \
          n++;                                                \
        }                                                     \
      }                                                       \
    }                                                         \
  }                                                           \
  else {                                                      \
    for ( n=0, k=-1; k<=1; k++ ) {                            \
      if ( z+k<0 || z+k>=treeImage->theDim[2] ) continue;     \
      for ( j=-1; j<=1; j++ ) {                               \
        if ( y+j<0 || y+j>=treeImage->theDim[1] ) continue;   \
        for ( i=-1; i<=1; i++ ) {                             \
          if ( x+i<0 || x+i>=treeImage->theDim[0] ) continue; \
          if ( k == 0 && j == 0 && i == 0 ) continue;         \
          b = a + (k * treeImage->theDim[1] + j) * treeImage->theDim[0] + i; \
          if ( theCC[b] > 0 && theCC[b] != label ) {          \
            if ( n == 0 ) {                                   \
              *l1 = theCC[b];                                 \
            }                                                 \
            else if ( n == 1 ) {                              \
              *l2 = theCC[b];                                 \
            }                                                 \
            n++;                                              \
          }                                                   \
        }                                                     \
      }                                                       \
    }                                                         \
  }                                                           \
}

  switch( treeImage->componentType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such tree image type not handled yet\n", proc );
    return( -1 );

  case UCHAR :
    _EXTRACTCOMPONENTNEIGHBORS( u8 );
    return( n );
    break;

  case SSHORT :
    _EXTRACTCOMPONENTNEIGHBORS( s16 );
    return( n );
    break;

  case USHORT :
    _EXTRACTCOMPONENTNEIGHBORS( u16 );
    return( n );
    break;
  }

  return( -1 );
}



static int _extractEdge( typeComponentTreeImage *treeImage,
                         _internComponent *c, _internIntPoint *firstPoint )
{
  char *proc = "_extractEdge";
  _internIntPoint current;
  _internIntPoint prev;
  _internIntPoint pt1;
  _internIntPoint pt2;
  int label1, label2;
  int nneighbors;
  int ncomponents;


  _initInternIntPoint( &pt1 );
  _initInternIntPoint( &pt2 );

  current = *firstPoint;

  /* add first point
   */
  if ( _addPointToInternComponent( c, &current ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to add first point to component\n", proc );
    return( -1 );
  }

  /* identify edge neighbor for first point
   * at most one neighbor (r=0,1)
   * r=0 means this is a one-point edge
   */
  nneighbors = _extractEdgeNeighbors( treeImage, &current, &pt1, &pt2 );
  if ( nneighbors == -1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to extract edge neighbor(s) for first point\n", proc );
    return( -1 );
  }
  else if ( nneighbors > 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: first point has multiple neighbors\n", proc );
    return( -1 );
  }


  /* identify component neighbor for first point
   * at most one component (r=0,1)
   * -1 means no neighbor
   */
  ncomponents = _extractComponentNeighbors( treeImage, &current, &label1, &label2 );
  if ( ncomponents == -1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to extract component neighbor for first point\n", proc );
    return( -1 );
  }
  else if ( (nneighbors == 0 && ncomponents > 2)
            || (nneighbors == 1 && ncomponents > 1) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: component has too many neighbors\n", proc );
    return( -1 );
  }

  /* case of one-point edge
   */
  if ( nneighbors == 0 ) {
    if ( ncomponents == 0 ) {
      c->labelFirstNeighbor = -1;
      c->labelLastNeighbor = -1;
    }
    else if ( ncomponents == 1 ) {
      c->labelFirstNeighbor = label1;
      c->labelLastNeighbor = -1;
    }
    else if ( ncomponents == 2 ) {
      c->labelFirstNeighbor = label1;
      c->labelLastNeighbor = label2;
    }
    else {
      if ( _verbose_ )
        fprintf( stderr, "%s: weird situation for one-point edge\n", proc );
      return( -1 );
    }
    return( 1 );
  }

  /* general case
   * nneighbors == 1
   * set component neighbor
   */
  if ( ncomponents == 0 ) {
    c->labelFirstNeighbor = -1;
  }
  else if ( ncomponents == 1 ) {
    c->labelFirstNeighbor = label1;
  }
  else {
    if ( _verbose_ )
      fprintf( stderr, "%s: weird situation for multiple-point edge\n", proc );
    return( -1 );
  }

  prev = current;
  current = pt1;

  do {
    if ( _addPointToInternComponent( c, &current ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to add point to component\n", proc );
      return( -1 );
    }

    nneighbors = _extractEdgeNeighbors( treeImage, &current, &pt1, &pt2 );
    if ( nneighbors == -1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to extract edge neighbor(s) for point\n", proc );
      return( -1 );
    }
    else if ( nneighbors > 2 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: point (%d %d %d) has multiple (%d) neighbors\n",
                 proc, current.x, current.y, current.z, nneighbors );
      return( -1 );
    }

    if ( nneighbors == 2 ) {
      if ( pt1.x == prev.x && pt1.y == prev.y && pt1.z == prev.z ) {
        prev = current;
        current = pt2;
      }
      else if ( pt2.x == prev.x && pt2.y == prev.y && pt2.z == prev.z ) {
        prev = current;
        current = pt1;
      }
      else {
        if ( _verbose_ )
          fprintf( stderr, "%s: previous point is not found among neighbors\n", proc );
        return( -1 );
      }
    }

  } while ( nneighbors == 2 );

  ncomponents = _extractComponentNeighbors( treeImage, &current, &label1, &label2 );
  if ( ncomponents == -1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to extract component neighbor for last point\n", proc );
    return( -1 );
  }
  else if ( ncomponents > 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: component has too many neighbors\n", proc );
    return( -1 );
  }

  if ( ncomponents == 0 ) {
    c->labelLastNeighbor = -1;
  }
  else if ( ncomponents == 1 ) {
    c->labelLastNeighbor = label1;
  }
  else {
    if ( _verbose_ )
      fprintf( stderr, "%s: weird situation for multiple-point edge\n", proc );
    return( -1 );
  }

  return( 1 );
}



static int _TreeImageToInternTree( typeComponentTreeImage *treeImage,
                                   _internTree *tree )
{
  char *proc = "_TreeImageToInternTree";
  int x, y, z, i, j;
  int r;
  _internIntPoint pt;

  _initInternTree( tree );
  if ( _allocInternTree( tree, treeImage->lastJunctionLabel+1 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }

#define _TREEIMAGETOINTERNTREE( TYPE ) {                                     \
  TYPE *theCC = (TYPE*)treeImage->componentBuf;                              \
  for ( i=0, z=0; z<treeImage->theDim[2]; z++ )                              \
  for ( y=0; y<treeImage->theDim[1]; y++ )                                   \
  for ( x=0; x<treeImage->theDim[0]; x++, i++ ) {                            \
    if ( theCC[i] == 0 ) continue;                                           \
    pt.x = x;   pt.y = y;   pt.z = z;                                        \
    if ( treeImage->firstJunctionLabel <= theCC[i]                           \
         && theCC[i] <= treeImage->lastJunctionLabel ) {                     \
      tree->data[theCC[i]].type = _JUNCTION_;                                \
      if ( _addPointToInternComponent( &(tree->data[theCC[i]]), &pt ) != 1 ) { \
        _freeInternTree( tree );                                             \
        return( -1 );                                                        \
      }                                                                      \
    }                                                                        \
    else if ( treeImage->firstComponentLabel <= theCC[i]                     \
              && theCC[i] <= treeImage->lastComponentLabel ) {               \
      if ( tree->data[theCC[i]].n_data > 0 ) continue;                       \
      r = _countSameNeighors( treeImage, &pt );                              \
      if ( theCC[i] == 110 ) \
         fprintf( stderr, "label=%d, pt=(%d,%d,%d), #neighbors=%d\n", theCC[i], x, y, z, r ); \
      if ( r == -1 ) {                                                       \
        _freeInternTree( tree );                                             \
        if ( _verbose_ )                                                     \
          fprintf( stderr, "%s: error when counting neigbors of (%d,%d,%d)\n", proc, x, y, z ); \
        return( -1 );                                                        \
      }                                                                      \
      if ( r == 1 || r == 0 ) {                                              \
        tree->data[theCC[i]].type = _EDGE_;                                  \
        if ( _extractEdge( treeImage, &(tree->data[theCC[i]]), &pt ) != 1 ) { \
            _freeInternTree( tree );                                         \
            if ( _verbose_ )                                                 \
              fprintf( stderr, "%s: error when extracting edge #%d from (%d,%d,%d)\n", proc, theCC[i], x, y, z ); \
            return( -1 );                                                    \
        }                                                                    \
      }                                                                      \
    }                                                                        \
    else {                                                                   \
      _freeInternTree( tree );                                               \
      if ( _verbose_ )                                                       \
        fprintf( stderr, "%s: weird case, %d is not a valid label\n", proc, theCC[i] ); \
      return( -1 );                                                          \
    }                                                                        \
  }                                                                          \
}

  switch( treeImage->componentType ) {
  default :
    _freeInternTree( tree );
    if ( _verbose_ )
      fprintf( stderr, "%s: such tree image type not handled yet\n", proc );
    return( -1 );

  case UCHAR :
      _TREEIMAGETOINTERNTREE( u8 );
      break;

  case SSHORT :
      _TREEIMAGETOINTERNTREE( s16 );
      break;

  case USHORT :
      _TREEIMAGETOINTERNTREE( u16 );
      break;
  }

  for ( i=1; i<tree->n_data; i++ ) {
    if ( tree->data[i].type == _EDGE_) continue;
    initTreeFloatPoint( &(tree->data[i].center) );
    tree->data[i].center.x = 0.0;
    tree->data[i].center.y = 0.0;
    tree->data[i].center.z = 0.0;
    for ( j=0; j<tree->data[i].n_data; j++ ) {
      tree->data[i].center.x += tree->data[i].data[j].x;
      tree->data[i].center.y += tree->data[i].data[j].y;
      tree->data[i].center.z += tree->data[i].data[j].z;
    }
    tree->data[i].center.x /= tree->data[i].n_data;
    tree->data[i].center.y /= tree->data[i].n_data;
    tree->data[i].center.z /= tree->data[i].n_data;
  }

  return( 1 );
}








/**************************************************
 *
 *
 *
 **************************************************/



void initTreeFloatPoint( typeTreeFloatPoint *p )
{
  p->x = -1.0;
  p->y = -1.0;
  p->z = -1.0;
}



static void initTreeEdge( typeTreeEdge *e )
{
  e->n_data = 0;
  e->n_allocated_data = 0;
  e->data = (int*)NULL;
}



static void freeTreeEdge( typeTreeEdge *e )
{
  if ( e->data != (int*)NULL )
    free( e->data );
  initTreeEdge( e );
}



void initTree( typeTree *t )
{
  t->n_points = 0;
  t->n_allocated_points = 0;
  t->points = (typeTreeFloatPoint*)NULL;
  t->n_edges = 0;
  t->n_allocated_edges = 0;
  t->edges = (typeTreeEdge*)NULL;
}



void freeTree( typeTree *t )
{
  int i;
  if ( t->points != (typeTreeFloatPoint*)NULL )
    free( t->points );
  if ( t->n_edges > 0 && t->edges != (typeTreeEdge*)NULL ) {
    for ( i=0; i<t->n_edges; i++ )
      freeTreeEdge( &(t->edges[i]) );
    free( t->edges );
  }
  initTree( t );
}



int treeImageToTree( typeComponentTreeImage *treeImage, typeTree *tree )
{
  char *proc = "treeImageToTree";
  _internTree iTree;
  int npointsedges;
  int npoints;
  int nedges;
  int njunctions;
  int nelements;
  int i, j, k, v, e;

  /* build internal tree
   */
  _initInternTree( &iTree );
  if ( _TreeImageToInternTree( treeImage, &iTree ) != 1 ) {
    _freeInternTree( &iTree );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when building internal tree\n", proc );
    return( -1 );
  }

  /* extract information
   */

  for ( i=1, nedges=0, njunctions=0; i<iTree.n_data; i++ ) {
    if ( iTree.data[i].type == _EDGE_) nedges++;
    if ( iTree.data[i].type == _JUNCTION_) njunctions++;
  }

  if ( _debug_  ) {
    fprintf( stderr, "%s: found %d edges, %d junctions\n",
             proc, nedges, njunctions );
    _printInternTree( stderr, &iTree );
  }


  /* recall that iTree.data[0] remains unknown
   */
  if ( nedges+njunctions != iTree.n_data-1 ) {
     _freeInternTree( &iTree );
     if ( _verbose_ )
       fprintf( stderr, "%s: unknown components in internal tree\n", proc );
     return( -1 );
  }

  for ( i=1, npointsedges=0; i<iTree.n_data; i++ ) {
    if ( iTree.data[i].type != _EDGE_) continue;
    npointsedges += iTree.data[i].n_data;
  }
  npoints = npointsedges + njunctions;

  if ( _verbose_ >= 2 ) {
    fprintf( stderr, "%s: found %d points, %d edges, %d junctions\n",
             proc, npoints, nedges, njunctions );
  }
  if ( _verbose_ >= 3 ) {
    _printInternTree( stderr, &iTree );
  }

  /* allocate tree
   */
  tree->points = (typeTreeFloatPoint*)malloc( npoints*sizeof(typeTreeFloatPoint) );
  if ( tree->points == (typeTreeFloatPoint*)NULL ) {
    _freeInternTree( &iTree );
    if ( _verbose_ )
      fprintf( stderr, "%s: tree points allocation failed\n", proc );
    return( -1 );
  }
  tree->n_points = npoints;
  tree->n_allocated_points = npoints;
  for ( i=0; i<tree->n_points; i++ )
    initTreeFloatPoint( &(tree->points[i]) );

  tree->edges = (typeTreeEdge*)malloc( nedges*sizeof(typeTreeEdge) );
  if ( tree->edges == (typeTreeEdge*)NULL ) {
    freeTree( tree );
    _freeInternTree( &iTree );
    if ( _verbose_ )
      fprintf( stderr, "%s: tree points allocation failed\n", proc );
    return( -1 );
  }
  tree->n_edges = nedges;
  tree->n_allocated_edges = nedges;
  for ( i=0; i<tree->n_edges; i++ )
    initTreeEdge( &(tree->edges[i]) );

  /* fill tree
   */
  for ( i=1, v=0; i<iTree.n_data; i++ ) {
    switch( iTree.data[i].type ) {
    default :
      break;
    case _EDGE_ :
      iTree.data[i].pointIndexOffset = v;
      for ( j=0; j<iTree.data[i].n_data; j++, v++ ) {
        tree->points[v].x = iTree.data[i].data[j].x * treeImage->voxelSize[0];
        tree->points[v].y = iTree.data[i].data[j].y * treeImage->voxelSize[1];
        tree->points[v].z = iTree.data[i].data[j].z * treeImage->voxelSize[2];
      }
      break;
    case _JUNCTION_ :
      iTree.data[i].pointIndexOffset = v;
      tree->points[v].x = iTree.data[i].center.x * treeImage->voxelSize[0];
      tree->points[v].y = iTree.data[i].center.y * treeImage->voxelSize[1];
      tree->points[v].z = iTree.data[i].center.z * treeImage->voxelSize[2];
      v++;
      break;
    }
  }
  for ( i=1, e=0; i<iTree.n_data; i++ ) {
    if ( iTree.data[i].type != _EDGE_ ) continue;
    nelements = iTree.data[i].n_data;
    if ( iTree.data[i].labelFirstNeighbor > 0 ) nelements++;
    if ( iTree.data[i].labelLastNeighbor > 0 ) nelements++;
    tree->edges[e].data = (int*)malloc( nelements*sizeof(int) );
    if ( tree->edges[e].data == (int*)NULL ) {
      freeTree( tree );
      _freeInternTree( &iTree );
      if ( _verbose_ )
        fprintf( stderr, "%s: tree edge allocation failed for edge #%d\n", proc, e );
      return( -1 );
    }
    tree->edges[e].n_data = nelements;
    tree->edges[e].n_allocated_data = nelements;
    k = 0;
    if ( iTree.data[i].labelFirstNeighbor > 0 ) {
      tree->edges[e].data[k] = iTree.data[ iTree.data[i].labelFirstNeighbor ].pointIndexOffset;
      k++;
    }
    for ( j=0; j<iTree.data[i].n_data; j++, k++ ) {
      tree->edges[e].data[k] = iTree.data[i].pointIndexOffset + j;
    }
    if ( iTree.data[i].labelLastNeighbor > 0 ) {
      tree->edges[e].data[k] = iTree.data[ iTree.data[i].labelLastNeighbor ].pointIndexOffset;
      k++;
    }
    e++;
  }

  _freeInternTree( &iTree );
  return( 1 );
}








/**************************************************
 *
 *
 *
 **************************************************/



int writeVTKLegacyFile( char *name, typeTree *tree, char *desc )
{
  char *proc = "writeVTKLegacyFile";
  FILE *f;
  int i, j, n;

  f = fopen( name, "w" );
  if ( f == (FILE*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when opening '%s'\n", proc, name );
    return( -1 );
  }

  fprintf( f, "# vtk DataFile Version 2.0\n" );

  if ( desc != (char*)NULL )
      fprintf( f, "%s", desc );
  fprintf( f, "\n" );

  fprintf( f, "ASCII\n" );
  fprintf( f, "DATASET POLYDATA\n" );

  fprintf( f, "POINTS %d float\n", tree->n_points );
  for ( i=0; i<tree->n_points; i++ )
    fprintf( f, "%f %f %f\n", tree->points[i].x, tree->points[i].y, tree->points[i].z );
  fprintf( f, "\n" );

  for ( i=0, n=0; i<tree->n_edges; i++ )
    n += tree->edges[i].n_data + 1;

  fprintf( f, "LINES %d %d\n", tree->n_edges, n);
  for ( i=0; i<tree->n_edges; i++ ) {
    fprintf( f, "%d", tree->edges[i].n_data );
    for ( j=0; j<tree->edges[i].n_data; j++ )
      fprintf( f, " %d", tree->edges[i].data[j] );
    fprintf( f, "\n" );
  }


  fclose( f );
  return( 1 );
}






















