/*************************************************************************
 * topological-operations-common.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Dim 28 fév 2016 14:10:05 CET
 *
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <topological-operations-common.h>





static int _verbose_ = 1;

void setVerboseInTopologicalOperationsCommon( int v )
{
  _verbose_ = v;
}

void incrementVerboseInTopologicalOperationsCommon(  )
{
  _verbose_ ++;
}

void decrementVerboseInTopologicalOperationsCommon(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}










/**************************************************
 *
 * point management
 *
 **************************************************/



void initTopologicalPoint( topologicalPoint *p )
{
  p->x = -1;
  p->y = -1;
  p->z = -1;
  p->i = -1;

  p->isinside = -1;
  p->type = _BACKGROUND_;

  p->iteration = -1;
  p->value = -1;
}



#ifdef _UNUSED_
static void _printtopologicalPoint( FILE *f, int s )
{
  switch ( s ) {
  default : fprintf( f, "unknown" );
  case _BACKGROUND_   : fprintf( f, "BACKGROUND" ); break;
  case _TOBEDELETED_  :
  /* case _INQUEUE_ : */
      fprintf( f, "TOBEDELETED (thinnig) / INQUEUE (thickening)" ); break;
  case _CANBEDELETED_ :
  /* case _WILLBEADDED_  : */
      fprintf( f, "CANBEDELETED (thinnig) / WILLBEADDED (thickening)" ); break;
  case _ENDPOINT_     : fprintf( f, "ENDPOINT" ); break;
  case _ANCHOR_       :
  /* case _ADDED_        : */
      fprintf( f, "ANCHOR (thinnig) / ADDED (thickening)" ); break;
  }
}
#endif










/**************************************************
 *
 * point list management
 *
 **************************************************/



static void initTopologicalPointList( topologicalPointList *l )
{
  l->data = (topologicalPoint*)NULL;
  l->n_data = 0;
  l->n_allocated_data = 0;
}



static void freeTopologicalPointList( topologicalPointList *l )
{
  if ( l->data != (topologicalPoint*)NULL )
    free( l->data );
  initTopologicalPointList( l );
}



static int _size_to_be_allocated_ = 100;

int addTopologicalPointToTopologicalPointList( topologicalPointList *l, topologicalPoint *p )
{
  char *proc = "addTopologicalPointToTopologicalPointList";
  int s =  l->n_allocated_data;
  topologicalPoint *data;

  if ( l->n_data == l->n_allocated_data ) {
    s += _size_to_be_allocated_;
    data = (topologicalPoint*)malloc( s * sizeof(topologicalPoint) );
    if ( data == (topologicalPoint*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
    }
    if ( l->n_allocated_data > 0 ) {
      (void)memcpy( data, l->data, l->n_allocated_data*sizeof(topologicalPoint) );
      free( l->data );
    }
    l->n_allocated_data = s;
    l->data = data;
  }

  initTopologicalPoint( &(l->data[l->n_data]) );
  l->data[l->n_data] = *p;
  l->n_data ++;
  return( 1 );
}



static void _sortTopologicalPointListWrtIteration( topologicalPointList *l,
                                           int left,
                                           int right )
{
  int i, last;
  topologicalPoint tmp;

  if ( left >= right ) return;

  tmp = l->data[left];   l->data[left] = l->data[(left+right)/2];   l->data[(left+right)/2] = tmp;

  last = left;
  for ( i = left+1; i <= right; i++ )
    if ( l->data[i].iteration < l->data[left].iteration ) {
      tmp = l->data[++last];   l->data[last] = l->data[i];   l->data[i] = tmp;
    }

  tmp = l->data[left];   l->data[left] = l->data[last];   l->data[last] = tmp;

  _sortTopologicalPointListWrtIteration( l, left, last-1 );
  _sortTopologicalPointListWrtIteration( l, last+1, right );
}



static void _sortTopologicalPointListWrtValue( topologicalPointList *l,
                                       int left,
                                       int right )
{
  int i, last;
  topologicalPoint tmp;

  if ( left >= right ) return;

  tmp = l->data[left];   l->data[left] = l->data[(left+right)/2];   l->data[(left+right)/2] = tmp;

  last = left;
  for ( i = left+1; i <= right; i++ )
    if ( l->data[i].value < l->data[left].value ) {
      tmp = l->data[++last];   l->data[last] = l->data[i];   l->data[i] = tmp;
    }

  tmp = l->data[left];   l->data[left] = l->data[last];   l->data[last] = tmp;

  _sortTopologicalPointListWrtValue( l, left, last-1 );
  _sortTopologicalPointListWrtValue( l, last+1, right );
}



void sortTopologicalPointList( topologicalPointList *l, enumTypeSort sortingCriterium )
{
  switch( sortingCriterium ) {
  default :
    break;
  case _ITERATION_SORTING_ :
    _sortTopologicalPointListWrtIteration( l, 0, l->n_data-1 );
    break;
  case _DISTANCE_SORTING_ :
    _sortTopologicalPointListWrtValue( l, 0, l->n_data-1 );
    break;
  }
}










/**************************************************
 *
 * point list list management
 *
 **************************************************/



void initTopologicalPointListList( topologicalPointListList *l )
{
  l->data = (topologicalPointList*)NULL;
  l->firstindex = 0;
  l->lastindex = 0;
  l->n_data = 0;
  l->n_allocated_data = 0;
}



void freeTopologicalPointListList( topologicalPointListList *l )
{
  int i;
  if ( l->data != (topologicalPointList*)NULL ) {
    for ( i=0; i<l->n_allocated_data; i++ ) {
        freeTopologicalPointList( &(l->data[i]) );
    }
    free( l->data );
  }
  initTopologicalPointListList( l );
}



int allocTopologicalPointListList( topologicalPointListList *l, int firstindex, int lastindex )
{
    char *proc = "allocTopologicalPointListList";
    topologicalPointList *data;
    int i, n = lastindex-firstindex+1;

    if ( n <= 0 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: no list to be allocated\n", proc );
        return( -1 );
    }

    data = (topologicalPointList*)malloc( n * sizeof(topologicalPointList) );
    if ( data == (topologicalPointList*)NULL ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: allocation failed\n", proc );
        return( -1 );
    }

    for ( i=0; i<n; i++ )
        initTopologicalPointList( &(data[i]) );

    l->data = data;
    l->firstindex = firstindex;
    l->lastindex = lastindex;
    l->n_data = n;
    l->n_allocated_data = n;

    return( 1 );
}










/**************************************************
 *
 * neighborhood management
 *
 **************************************************/





void defineNeighborsTobBeAdded( typeNeighborhood *n,
                                 int *theDim,
                                 int connectivity )
{
  int i = 0;

  for ( i=0; i<27; i++ ) {
    n->neighbors[i].dx = 0;
    n->neighbors[i].dy = 0;
    n->neighbors[i].dz = 0;
    n->neighbors[i].di = 0;
  }

  i = 0;

  /* 04-neighbors
   */
  n->neighbors[i].dy = -1;   i++;
  n->neighbors[i].dx = -1;   i++;
  n->neighbors[i].dx =  1;   i++;
  n->neighbors[i].dy =  1;   i++;

  switch ( connectivity ) {
  case 4 :
    break;
  case 6 :
    n->neighbors[i].dz = -1;   i++;
    n->neighbors[i].dz =  1;   i++;
    break;
  case 8 :
  case 18 :
  case 26 :
  default :
    n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
    break;
  }

  switch ( connectivity ) {
  case 4 :
  case 8 :
  case 6 :
    break;
  case 26 :
  default :
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
  case 18 :
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   i++;
  }

  n->nneighbors = i;

  for ( i=0; i<n->nneighbors; i++ ) {
    n->neighbors[i].di = n->neighbors[i].dz * theDim[0] * theDim[1]
      + n->neighbors[i].dy * theDim[0]
      + n->neighbors[i].dx;
  }
}



void defineNeighborsForSimplicity( typeNeighborhood *n,
                                    int *theDim,
                                    int connectivity )
{
  int i = 0;

  for ( i=0; i<27; i++ ) {
    n->neighbors[i].dx = 0;
    n->neighbors[i].dy = 0;
    n->neighbors[i].dz = 0;
    n->neighbors[i].di = 0;
  }

  i = 0;

  if ( connectivity == 6 || connectivity == 18 || connectivity == 26 ) {
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
  }

  n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
  n->neighbors[i].dy = -1;   i++;
  n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
  n->neighbors[i].dx = -1;   i++;
  i++;
  n->neighbors[i].dx =  1;   i++;
  n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
  n->neighbors[i].dy =  1;   i++;
  n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;

  if ( connectivity == 6 || connectivity == 18 || connectivity == 26 ) {
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
  }

  n->nneighbors = i;

  for ( i=0; i<n->nneighbors; i++ ) {
    n->neighbors[i].di += n->neighbors[i].dz * theDim[0] * theDim[1]
      + n->neighbors[i].dy * theDim[0]
      + n->neighbors[i].dx;
  }
}



void extractNeighborhood( int *neighb,
                           topologicalPoint *pt,
                           unsigned char *resBuf,
                           int *theDim,
                           typeNeighborhood *neighbors )
{
  int n;

  if ( pt->isinside ) {

    for ( n=0; n<neighbors->nneighbors; n++ ) {
      neighb[n] = resBuf[ pt->i + neighbors->neighbors[n].di ];
    }

  }
  else {

    for ( n=0; n<neighbors->nneighbors; n++ ) {
      if ( pt->x+neighbors->neighbors[n].dx < 0 || pt->x+neighbors->neighbors[n].dx >= theDim[0] ||
           pt->y+neighbors->neighbors[n].dy < 0 || pt->y+neighbors->neighbors[n].dy >= theDim[1] ||
           pt->z+neighbors->neighbors[n].dz < 0 || pt->z+neighbors->neighbors[n].dz >= theDim[2] )
        neighb[n] = 0;
      else
        neighb[n] = resBuf[ pt->i + neighbors->neighbors[n].di ];
    }

  }
}



void fprintfNeighborhood( FILE *f, int *neighb, int n )
{
    switch( n ) {
    default :
        break;
    case 9 :
        fprintf( f, "   %3d %3d %3d\n", neighb[0], neighb[1], neighb[2] );
        fprintf( f, "   %3d %3d %3d\n", neighb[3], neighb[4], neighb[5] );
        fprintf( f, "   %3d %3d %3d\n", neighb[6], neighb[7], neighb[8] );
        break;
    case 27 :
        fprintf( f, "   %3d %3d %3d", neighb[0], neighb[1], neighb[2] );
        fprintf( f, "   %3d %3d %3d", neighb[ 9], neighb[10], neighb[11] );
        fprintf( f, "   %3d %3d %3d", neighb[18], neighb[19], neighb[20] );
        fprintf( f, "\n" );
        fprintf( f, "   %3d %3d %3d", neighb[3], neighb[4], neighb[5] );
        fprintf( f, "   %3d %3d %3d", neighb[12], neighb[13], neighb[14] );
        fprintf( f, "   %3d %3d %3d", neighb[21], neighb[22], neighb[23] );
        fprintf( f, "\n" );
        fprintf( f, "   %3d %3d %3d", neighb[6], neighb[7], neighb[8] );
        fprintf( f, "   %3d %3d %3d", neighb[15], neighb[16], neighb[17] );
        fprintf( f, "   %3d %3d %3d", neighb[24], neighb[25], neighb[26] );
        fprintf( f, "\n" );
        break;
    }
}

