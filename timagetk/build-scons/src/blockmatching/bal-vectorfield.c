/*************************************************************************
 * bal-vectorfield.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <chunks.h>
#include <chamferdistance.h>
#include <chamferdistance-mask.h>

#include <bal-vectorfield.h>
#include <bal-field-tools.h>
#include <bal-behavior.h>



/************************************************************
 *
 * static variables
 *
 ************************************************************/


/* for some generic writing
 */

static int _verbose_ = 1;

void BAL_SetVerboseInBalVectorField( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalVectorField(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalVectorField(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}










/************************************************************
 *
 *
 *
 ************************************************************/



int BAL_SmoothVectorField( bal_transformation* theTrsf, bal_doublePoint *theSigma )
{
  char *proc="BAL_SmoothVectorField";

  switch ( theTrsf->type ) {
  default :
    if ( _verbose_ )
         fprintf( stderr, "%s: transformation is not a vector field\n", proc );
    return( -1 );
    
  case VECTORFIELD_3D :
    if ( BAL_SmoothImage( &(theTrsf->vz), theSigma ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to smooth Z component\n", proc );
      return( -1 );
    }

  case VECTORFIELD_2D :
    if ( BAL_SmoothImage( &(theTrsf->vx), theSigma ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to smooth X component\n", proc );
      return( -1 );
    }
    if ( BAL_SmoothImage( &(theTrsf->vy), theSigma ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to smooth Y component\n", proc );
      return( -1 );
    }
  }
  return( 1 );
}





/************************************************************
 *
 * procedure to parallelize vector field initialization
 * and division by the weights
 *
 ************************************************************/

static double _min_weight_ = 0.00001;

typedef struct {
  float *bufVx;
  float *bufVy;
  float *bufVz;
  float *bufWeights;
  double v;
} _VectorFieldAuxiliaryParam;



static void _InitVectorFieldAuxiliaryParam( _VectorFieldAuxiliaryParam *p )
{
  p->bufVx = (float*)NULL;
  p->bufVy = (float*)NULL;
  p->bufVz = (float*)NULL;
  p->bufWeights = (float*)NULL;
  p->v = _min_weight_;
}





static void *_Init2DVectorFieldSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  float *bufVx = ((_VectorFieldAuxiliaryParam*)parameter)->bufVx;
  float *bufVy = ((_VectorFieldAuxiliaryParam*)parameter)->bufVy;
  float *bufWe = ((_VectorFieldAuxiliaryParam*)parameter)->bufWeights;
  size_t i;

  for ( i=first; i<=last; i++ ) {
    bufVx[i] = 0.0;
    bufVy[i] = 0.0;
    bufWe[i] = 0.0;
  }
  chunk->ret = 1;
  return( (void*)NULL );
}



static void *_Init3DVectorFieldSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  float *bufVx = ((_VectorFieldAuxiliaryParam*)parameter)->bufVx;
  float *bufVy = ((_VectorFieldAuxiliaryParam*)parameter)->bufVy;
  float *bufVz = ((_VectorFieldAuxiliaryParam*)parameter)->bufVz;
  float *bufWe = ((_VectorFieldAuxiliaryParam*)parameter)->bufWeights;
  size_t i;

  for ( i=first; i<=last; i++ ) {
    bufVx[i] = 0.0;
    bufVy[i] = 0.0;
    bufVz[i] = 0.0;
    bufWe[i] = 0.0;
  }
  chunk->ret = 1;
  return( (void*)NULL );
}



static void *_Normalize2DVectorFieldSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  float *bufVx = ((_VectorFieldAuxiliaryParam*)parameter)->bufVx;
  float *bufVy = ((_VectorFieldAuxiliaryParam*)parameter)->bufVy;
  float *bufWe = ((_VectorFieldAuxiliaryParam*)parameter)->bufWeights;
  double v = ((_VectorFieldAuxiliaryParam*)parameter)->v;
  size_t i;

  for ( i=first; i<=last; i++ ) {
    if ( bufWe[i] > v ) {
      bufVx[i] /= bufWe[i];
      bufVy[i] /= bufWe[i];
    }
    else {
      bufVx[i] /= v;
      bufVy[i] /= v;
    }
  }
  chunk->ret = 1;
  return( (void*)NULL );
}



static void *_Normalize3DVectorFieldSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  float *bufVx = ((_VectorFieldAuxiliaryParam*)parameter)->bufVx;
  float *bufVy = ((_VectorFieldAuxiliaryParam*)parameter)->bufVy;
  float *bufVz = ((_VectorFieldAuxiliaryParam*)parameter)->bufVz;
  float *bufWe = ((_VectorFieldAuxiliaryParam*)parameter)->bufWeights;
  double v = ((_VectorFieldAuxiliaryParam*)parameter)->v;
  size_t i;

  for ( i=first; i<=last; i++ ) {
    if ( bufWe[i] > v ) {
      bufVx[i] /= bufWe[i];
      bufVy[i] /= bufWe[i];
      bufVz[i] /= bufWe[i];
    }
    else {
      bufVx[i] /= v;
      bufVy[i] /= v;
      bufVz[i] /= v;
    }
  }
  chunk->ret = 1;
  return( (void*)NULL );
}



static int _VectorField_Estimation_Initialization( bal_transformation *theTrsf,
                                                   bal_image *theWeights )
{
  char *proc = "_VectorField_Estimation_Initialization";
  typeChunks chunks;
  size_t first, last;
  _VectorFieldAuxiliaryParam aux;
  int n;

  /* parallelization stuff
   */
  first = 0;
  last = theTrsf->vx.ncols*theTrsf->vx.nrows*theTrsf->vx.nplanes - 1;
  _InitVectorFieldAuxiliaryParam( &aux );

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( -1 );
  }
  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    chunks.data[n].parameters = (void*)(&aux);
  }

  switch ( theTrsf->type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
    freeChunks( &chunks );
    return( -1 );

  case VECTORFIELD_2D :
    aux.bufVx = (float*)(theTrsf->vx.data);
    aux.bufVy = (float*)(theTrsf->vy.data);
    aux.bufWeights = (float*)(theWeights->data);
    if ( processChunks( &_Init2DVectorFieldSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to initialize vectorfield (2D case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;

  case VECTORFIELD_3D :
    aux.bufVx = (float*)(theTrsf->vx.data);
    aux.bufVy = (float*)(theTrsf->vy.data);
    aux.bufVz = (float*)(theTrsf->vz.data);
    aux.bufWeights = (float*)(theWeights->data);
    if ( processChunks( &_Init3DVectorFieldSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to initialize vectorfield (3D case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;
  }

  freeChunks( &chunks );
  return( 1 );
}



static int _VectorField_Estimation_Smoothing( bal_transformation *theTrsf,
                                              bal_image *theWeights,
                                              bal_doublePoint *sigma )
{
  char *proc = "_VectorField_Estimation_Smoothing";

  switch ( theTrsf->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
    return( -1 );
  case VECTORFIELD_2D :
    sigma->z = -1.0;
    break;
  case VECTORFIELD_3D :
    break;
  }

  if ( BAL_SmoothVectorField( theTrsf, sigma ) != 1 ) {
    if ( _verbose_ )
        fprintf( stderr, "%s: unable to smooth vectorfield\n", proc );
    return( -1 );
  }

  if ( BAL_SmoothImage( theWeights, sigma ) != 1 ) {
    if ( _verbose_ )
        fprintf( stderr, "%s: unable to smooth weights\n", proc );
    return( -1 );
  }

  return( 1 );
}



static int _VectorField_Estimation_Normalization( bal_transformation *theTrsf,
                                                  bal_image *theWeights )
{
  char *proc = "_VectorField_Estimation_Normalization";
  typeChunks chunks;
  size_t first, last;
  _VectorFieldAuxiliaryParam aux;
  int n;

  /* parallelization stuff
   */
  first = 0;
  last = theTrsf->vx.ncols*theTrsf->vx.nrows*theTrsf->vx.nplanes - 1;
  _InitVectorFieldAuxiliaryParam( &aux );

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( -1 );
  }
  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    chunks.data[n].parameters = (void*)(&aux);
  }

  switch ( theTrsf->type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
    freeChunks( &chunks );
    return( -1 );

  case VECTORFIELD_2D :
    aux.bufVx = (float*)(theTrsf->vx.data);
    aux.bufVy = (float*)(theTrsf->vy.data);
    aux.bufWeights = (float*)(theWeights->data);
    if ( processChunks( &_Normalize2DVectorFieldSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to normalize vectorfield (2D case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;

  case VECTORFIELD_3D :
    aux.bufVx = (float*)(theTrsf->vx.data);
    aux.bufVy = (float*)(theTrsf->vy.data);
    aux.bufVz = (float*)(theTrsf->vz.data);
    aux.bufWeights = (float*)(theWeights->data);
    if ( processChunks( &_Normalize3DVectorFieldSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to normalize vectorfield (3D case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;
  }

  freeChunks( &chunks );
  return( 1 );
}





/************************************************************
 *
 * vector field filling, direct propagation of field
 *
 ************************************************************/



typedef struct typeFieldPoint {
  int i;
  int j;
  int k;
  typeScalarWeightedDisplacement *displacement;
  float distanceToOrigin;
} typeFieldPoint;

static void _initTypeFieldPoint ( typeFieldPoint *p )
{
    p->i = -1;
    p->j = -1;
    p->k = -1;
    p->displacement = (typeScalarWeightedDisplacement*)NULL;
    p->distanceToOrigin = -1.0;
}



typedef struct typeFieldPointList {
  typeFieldPoint *data;
  int first_index;
  int n_allocated_data;
  int n_data;
} typeFieldPointList;

static void _initTypeFieldPointList ( typeFieldPointList *l )
{
    l->data = (typeFieldPoint*)NULL;
    l->first_index = 0;
    l->n_allocated_data = 0;
    l->n_data = 0;
}

static void _freeTypeFieldPointList ( typeFieldPointList *l )
{
    if ( l->data != (typeFieldPoint*)NULL ) {
        free( l->data );
    }
    _initTypeFieldPointList( l );
}

static int _size_to_be_allocated_ = 100;

static int _addTypeFieldPointToList( typeFieldPointList *l,
                                     typeFieldPoint *p )
{
    char *proc = "_addTypeFieldPointToList";
    int s =  l->n_allocated_data;
    typeFieldPoint *data;

    if ( l->n_data == l->n_allocated_data ) {
      s += _size_to_be_allocated_;
      data = (typeFieldPoint*)malloc( s * sizeof(typeFieldPoint) );
      if ( data == (typeFieldPoint*)NULL ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: allocation error\n", proc );
        return( -1 );
      }
      if ( l->n_allocated_data > 0 ) {
        (void)memcpy( data, l->data, l->n_allocated_data*sizeof(typeFieldPoint) );
        free( l->data );
      }
      l->n_allocated_data = s;
      l->data = data;
    }

    l->data[l->n_data] = *p;
    l->n_data ++;

    return( 1 );
}



/* conventions
 * weight = -1.0 means that point is in list
 * weight > 0.0 means that point has already been marked
 */
static int _VectorField_Direct_Propagation_Initialization( typeFieldPointList *l,
                                                    bal_image *theWeights,
                                                    FIELD *field )
{
    char *proc = "_VectorField_Direct_Propagation_Initialization";
    float ***bufWeights;
    size_t n;
    typeFieldPoint p;

    bufWeights = (float***)theWeights->array;

    for ( n=0; n<field->n_selected_pairs; n++ ) {
        _initTypeFieldPoint( &p );
        p.i = (int)(field->pointer[n]->origin.x + 0.5);
        p.j = (int)(field->pointer[n]->origin.y + 0.5);
        p.k = (int)(field->pointer[n]->origin.z + 0.5);
        p.displacement = field->pointer[n];
        p.distanceToOrigin = 0.0;
        if ( _addTypeFieldPointToList( l, &p ) != 1 ) {
            if ( _verbose_ )
                fprintf( stderr, "%s: error when adding point #%lu/%lu to list\n",
                         proc, n, field->n_selected_pairs );
            return( -1 );
        }
        bufWeights[p.k][p.j][p.i] = -1.0;
    }

    return( 1 );
}



static int _VectorField_Direct_Propagation_PointMarking( typeFieldPointList *l,
                                                  bal_transformation* theTrsf,
                                                  bal_image *theWeights,
                                                  enumTypeEstimator type,
                                                  float distance,
                                                  float maxDistance,
                                                  float fadingCoefficient )
{
    char *proc = "_VectorField_Direct_Propagation_PointMarking";
    int first = l->first_index;
    int last = l->n_data-1;
    int n, m, f;
    float errorDistance;
    float ***bufWeights;
    float ***bufVx = (float***)NULL;
    float ***bufVy = (float***)NULL;
    float ***bufVz = (float***)NULL;
    int i, j, k;
    int x, y, z;
    float dx, dy, dz, d;
    typeFieldPoint p;


    bufWeights = (float***)theWeights->array;
    switch ( theTrsf->type ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
      return( -1 );
    break;
    case VECTORFIELD_3D :
      bufVz = (float***)(theTrsf->vz.array);
    case VECTORFIELD_2D :
      bufVx = (float***)(theTrsf->vx.array);
      bufVy = (float***)(theTrsf->vy.array);
      break;
    }


    /* admissible error, based on voxel size
     */
    errorDistance = theTrsf->vx.vx;
    if ( errorDistance < theTrsf->vx.vx ) errorDistance = theTrsf->vx.vy;
    if ( errorDistance < theTrsf->vx.vz ) errorDistance = theTrsf->vx.vz;
    errorDistance /= 4.0;

    for ( n=first; n<=last; n++ ) {

        /* go to next point
         */
        if ( l->data[n].distanceToOrigin >= distance+errorDistance ) {
            continue;
        }

        /* add point #n
         */
        i = l->data[n].i;
        j = l->data[n].j;
        k = l->data[n].k;

        if ( bufWeights[k][j][i] > 0.0 ) {
            if ( _verbose_ )
                fprintf( stderr, "%s: weird, point (%d,%d,%d) has already been added\n",
                         proc, i, j, k );
            continue;
        }

        switch ( type ) {
        default :
          if ( _verbose_ )
            fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
          return( -1 );

        case TYPE_LS :
        case TYPE_LTS :

          switch ( theTrsf->type ) {
          default :
            if ( _verbose_ )
              fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
            return( -1 );
          case VECTORFIELD_2D :
            bufVx[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.x;
            bufVy[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.y;
            bufWeights[k][j][i] = 1.0;
            break;
          case VECTORFIELD_3D :
            bufVx[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.x;
            bufVy[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.y;
            bufVz[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.z;
            bufWeights[k][j][i] = 1.0;
            break;
          }
          break;

        case TYPE_WLS :
        case TYPE_WLTS :

            switch ( theTrsf->type ) {
            default :
              if ( _verbose_ )
                fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
              return( -1 );
            case VECTORFIELD_2D :
              bufVx[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.x;
              bufVy[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.y;
              bufWeights[k][j][i] = l->data[n].displacement->rho;
              break;
            case VECTORFIELD_3D :
              bufVx[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.x;
              bufVy[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.y;
              bufVz[k][j][i] = fadingCoefficient * l->data[n].displacement->vector.z;
              bufWeights[k][j][i] = l->data[n].displacement->rho;
              break;
            }
            break;
        }

        /* add neighbors of point #n
         * pay attention to the fact that a point could
         * already have been added
         */
        for ( z=-1; z<=1; z++ ) {
          if ( k+z < 0 || k+z >= (int)theTrsf->vx.nplanes ) continue;
          for ( y=-1; y<=1; y++ ) {
            if ( j+y < 0 || j+y >= (int)theTrsf->vx.nrows ) continue;
            for ( x=-1; x<=1; x++ ) {
              if ( i+x < 0 || i+x >= (int)theTrsf->vx.ncols ) continue;

              /* only keep 6-neighbors
               */
              if ( ((z == -1 || z == 1) && (y == -1 || y == 1))
                   || ((z == -1 || z == 1) && (x == -1 || x == 1))
                   || ((y == -1 || y == 1) && (x == -1 || x == 1)) )
                  continue;

              /* point already marked
               */
              if ( bufWeights[k+z][j+y][i+x] > 0.0 ) continue;
              if ( bufWeights[k+z][j+y][i+x] < -1.5 ) continue;
              /* compute distance to origin
               */
              dz = (k+z) - l->data[n].displacement->origin.z;
              dy = (j+y) - l->data[n].displacement->origin.y;
              dx = (i+x) - l->data[n].displacement->origin.x;
              /* distance in world unit
              d = sqrt( (dx*theTrsf->vx.vx)*(dx*theTrsf->vx.vx)
                        + (dy*theTrsf->vx.vy)*(dy*theTrsf->vx.vy)
                        + (dz*theTrsf->vx.vz)*(dz*theTrsf->vx.vz)
              */
              d = sqrt( dx*dx + dy*dy + dz*dz );
              if ( d > maxDistance ) {
                  bufWeights[k+z][j+y][i+x] = -2.0;
                  continue;
              }
              _initTypeFieldPoint( &p );
              p.i = i+x;
              p.j = j+y;
              p.k = k+z;
              p.displacement = l->data[n].displacement;
              p.distanceToOrigin = d;
              if ( bufWeights[k+z][j+y][i+x] < 0.0 ) {
                  /* point already in list
                   * replace if new distance is shorter
                   */
                  for ( m=0, f=0; m<l->n_data && f==0; m++ ) {
                      if ( l->data[m].i != i+x
                           || l->data[m].j != j+y
                           || l->data[m].k != k+z )
                          continue;
                      if ( l->data[m].distanceToOrigin > d )
                          l->data[m] = p;
                      f = 1;
                  }
              }
              else {
                  /* new point
                   */
                  if ( _addTypeFieldPointToList( l, &p ) != 1 ) {
                      if ( _verbose_ )
                          fprintf( stderr, "%s: error when adding new point to list\n",
                                   proc );
                      return( -1 );
                  }
                  bufWeights[k+z][j+y][i+x] = -1.0;
              }
            }
          }
        }

        /* neigbors have been added
         * put this point at the beginning of list
         */
        p = l->data[l->first_index];
        l->data[l->first_index] = l->data[n];
        l->data[n] = p;
        l->first_index ++;
    }

    return( l->first_index - first );
}



static int _VectorField_Direct_Propagation( bal_transformation* theTrsf,
                                            bal_image *theWeights,
                                            FIELD *field,
                                            bal_estimator *estimator )
{
    char *proc = "_VectorField_Direct_Propagation";
    typeFieldPointList list;
    int iteration = 0;
    int i, j, k;
    int nmarkedpoints;
    float distance;
    float propagationConstant = estimator->propagation.constant;
    float propagationFading = estimator->propagation.fading;
    float fadingCoefficient = 1.0;

    if ( propagationConstant < 0.0 ) propagationConstant = 0.0;
    if ( propagationFading < 0.0 ) propagationFading = 0.0;

    _initTypeFieldPointList( &list );
    if ( _VectorField_Direct_Propagation_Initialization( &list, theWeights, field ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to init propagation\n", proc );
        _freeTypeFieldPointList( &list );
        return( -1 );
    }

    /* propagation
     * first distance value = 0.0
     *   =
     */
    for ( distance=0.0;
          distance<=propagationConstant+propagationFading && list.n_data>0;
          ) {

        if ( distance <= propagationConstant ) {
            fadingCoefficient = 1.0;
        }
        else if ( propagationFading > 0.0 ) {
            fadingCoefficient = ( propagationFading + propagationConstant - distance ) / propagationFading;
        }


        if ( _verbose_ >= 3 ) {
            fprintf( stderr, "%s: iteration #%2d, process distance=%f, fading coefficient=%f\n",
                     proc, iteration, distance, fadingCoefficient );
            fprintf( stderr, "\t ... process list = [%d %d]\n",
                     list.first_index, list.n_data-1 );
        }

        /* mark points
         */

        nmarkedpoints = _VectorField_Direct_Propagation_PointMarking( &list, theTrsf,
                                                               theWeights, estimator->type,
                                                               distance,
                                                               propagationConstant+propagationFading,
                                                               fadingCoefficient );

        if ( nmarkedpoints < 0 ) {
            if ( _verbose_ )
                fprintf( stderr, "%s: error when marking points at distance %f\n", proc, distance );
            _freeTypeFieldPointList( &list );
            return( -1 );
        }

        if ( _verbose_ >= 3 ) {
            fprintf( stderr, "\t ... result list = [%d %d]\n",
                     list.first_index, list.n_data-1 );
            fprintf( stderr, "\t ... marks %d points at distance %f\n",
                     nmarkedpoints, distance );
        }

        /* get new distance
         */
        for ( i=0, j=list.first_index; j<list.n_data; i++, j++ )
            list.data[ i ] = list.data[ j ];
        list.n_data -= list.first_index;
        list.first_index = 0;

        distance = list.data[ list.first_index ].distanceToOrigin;
        for ( i=list.first_index+1; i<list.n_data; i++ ) {
            if ( distance > list.data[i].distanceToOrigin )
                distance = list.data[i].distanceToOrigin;
        }

        iteration ++;
    }

    _freeTypeFieldPointList( &list );

    /* weights at -1.0 have to be set back to 0.0
     * they denote points in the list that have not been marked
     */

    switch ( theWeights->type ) {
    default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such weight image type not handled yet\n", proc );
        return( -1 );
    case FLOAT :
      {
        float ***theBuf = (float***)theWeights->array;
        for ( k=0; k<(int)theWeights->nplanes; k++ )
        for ( j=0; j<(int)theWeights->nrows; j++ )
        for ( i=0; i<(int)theWeights->ncols; i++ ) {
            if ( theBuf[k][j][i] < 0.0 ) {
                theBuf[k][j][i] = 0.0;
            }
        }
      }
      break;
    }
    return( 1 );
}





/************************************************************
 *
 * vector field filling, propagation of field by influence zone
 *
 ************************************************************/



#define _SKIZ_UNWEIGHTED_FILLING( TYPE ) {                              \
  TYPE *theLabel = (TYPE*)bufLabel;                                     \
  for ( i=0, z=0; z<theDim[2]; z++ )                                    \
  for ( y=0; y<theDim[1]; y++ )                                         \
  for ( x=0; x<theDim[0]; x++, i++ ) {                                  \
    if ( theDist[i] == 0 ) {                                            \
        bufVx[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.x;     \
        bufVy[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.y;     \
        if ( theTrsf->type == VECTORFIELD_3D )                          \
          bufVz[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.z;   \
        bufWeights[z][y][x] = 1.0;                                      \
    }                                                                   \
    else {                                                              \
      distance = (double)theDist[i] / nfactor;                          \
      if ( distance >= estimator->propagation.fading + estimator->propagation.constant ) { \
        bufVx[z][y][x] = 0.0;                                           \
        bufVy[z][y][x] = 0.0;                                           \
        if ( theTrsf->type == VECTORFIELD_3D )                          \
          bufVz[z][y][x] = 0.0;                                         \
        bufWeights[z][y][x] = 0.0;                                      \
        continue;                                                       \
      }                                                                 \
      if ( distance <= estimator->propagation.constant )                \
        c = 1.0;                                                        \
      else  if ( estimator->propagation.fading > 0.0 )                  \
        c = ( estimator->propagation.fading + estimator->propagation.constant - distance ) / estimator->propagation.fading; \
      bufVx[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.x;   \
      bufVy[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.y;   \
      if ( theTrsf->type == VECTORFIELD_3D )                            \
        bufVz[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.z; \
      bufWeights[z][y][x] = 1.0;                                        \
    }                                                                   \
  }                                                                     \
}



#define _SKIZ_WEIGHTED_FILLING( TYPE ) {                                \
  TYPE *theLabel = (TYPE*)bufLabel;                                     \
  for ( i=0, z=0; z<theDim[2]; z++ )                                    \
  for ( y=0; y<theDim[1]; y++ )                                         \
  for ( x=0; x<theDim[0]; x++, i++ ) {                                  \
    if ( theDist[i] == 0 ) {                                            \
        bufVx[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.x;     \
        bufVy[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.y;     \
        if ( theTrsf->type == VECTORFIELD_3D )                          \
          bufVz[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.z;   \
        bufWeights[z][y][x] = field->pointer[ theLabel[i]-1 ]->rho;     \
    }                                                                   \
    else {                                                              \
      distance = (double)theDist[i] / nfactor;                          \
      if ( distance >= estimator->propagation.fading + estimator->propagation.constant ) { \
        bufVx[z][y][x] = 0.0;                                           \
        bufVy[z][y][x] = 0.0;                                           \
        if ( theTrsf->type == VECTORFIELD_3D )                          \
          bufVz[z][y][x] = 0.0;                                         \
        bufWeights[z][y][x] = 0.0;                                      \
        continue;                                                       \
      }                                                                 \
      if ( distance <= estimator->propagation.constant )                \
        c = 1.0;                                                        \
      else  if ( estimator->propagation.fading > 0.0 )                  \
        c = ( estimator->propagation.fading + estimator->propagation.constant - distance ) / estimator->propagation.fading; \
      bufVx[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.x;   \
      bufVy[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.y;   \
      if ( theTrsf->type == VECTORFIELD_3D )                            \
        bufVz[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.z; \
      bufWeights[z][y][x] = field->pointer[ theLabel[i]-1 ]->rho;       \
    }                                                                   \
  }                                                                     \
}



static int _VectorField_Skiz_Propagation( bal_transformation* theTrsf,
                                          bal_image *theWeights,
                                          FIELD *field,
                                          bal_estimator *estimator )
{
  char *proc = "_VectorField_Skiz_Propagation";
  void *bufLabel = (void*)NULL;
  bufferType typeLabel = TYPE_UNKNOWN;
  unsigned short int *theDist = (unsigned short int *)NULL;
  size_t i, n, v;
  float ***bufWeights = (float***)NULL;
  float ***bufVx = (float***)NULL;
  float ***bufVy = (float***)NULL;
  float ***bufVz = (float***)NULL;
  int x, y, z;

  int theDim[3];

  typeChamferMask theMask;
  int chamfercoef_dim = 0;
  int chamfercoef_size = 5;
  int chamfercoef_max = 5;
  double chamfercoef_voxel_size[3];
  double nfactor, distance, c;



  theDim[0] = theTrsf->vx.ncols;
  theDim[1] = theTrsf->vx.nrows;
  theDim[2] = theTrsf->vx.nplanes;
  v = theTrsf->vx.ncols * theTrsf->vx.nrows * theTrsf->vx.nplanes;



  /* labels allocation and initialization
   */
  if ( field->n_selected_pairs <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs <= 255 ) {
    bufLabel = (void*)malloc( v*sizeof(u8) );
    if ( bufLabel == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: label buffer allocation error\n", proc );
      return( -1 );
    }
    typeLabel = UCHAR;
  }
  else if ( field->n_selected_pairs <= 65535 ) {
    bufLabel = (void*)malloc( v*sizeof(u16) );
    if ( bufLabel == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: label buffer allocation error\n", proc );
      return( -1 );
    }
    typeLabel = USHORT;
  }
  else {
    if ( _verbose_ )
      fprintf( stderr, "%s: too many elements in field\n", proc );
    return( -1 );
  }

  switch ( typeLabel ) {
  default :
    free( bufLabel );
    if ( _verbose_ )
      fprintf( stderr, "%s: weird, this should not be reached\n", proc );
    return( -1 );
  case UCHAR :
    {
      u8 *theLabel = (u8*)bufLabel;
      for ( i=0; i<v; i++ ) theLabel[i] = 0;
      for ( n=0; n<field->n_selected_pairs; n++ ) {
          i = (int)(field->pointer[n]->origin.z + 0.5) * theDim[0] * theDim[1];
          i += (int)(field->pointer[n]->origin.y + 0.5) * theDim[0];
          i += (int)(field->pointer[n]->origin.x + 0.5);
          theLabel[i] = n+1;
      }
    }
    break;
  case USHORT :
    {
      u16 *theLabel = (u16*)bufLabel;
      for ( i=0; i<v; i++ ) theLabel[i] = 0;
      for ( n=0; n<field->n_selected_pairs; n++ ) {
          i = (int)(field->pointer[n]->origin.z + 0.5) * theDim[0] * theDim[1];
          i += (int)(field->pointer[n]->origin.y + 0.5) * theDim[0];
          i += (int)(field->pointer[n]->origin.x + 0.5);
          theLabel[i] = n+1;
      }
    }
    break;
  }



  /* chamfer mask computation
   */
  initChamferMask( &theMask );
  switch ( theTrsf->type ) {
  default :
    free( bufLabel );
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
    return( -1 );
  break;
  case VECTORFIELD_2D :
    chamfercoef_dim = 2;
    break;
  case VECTORFIELD_3D :
    chamfercoef_dim = 3;
    break;
  }
  chamfercoef_voxel_size[0] = 1.0;
  chamfercoef_voxel_size[1] = theTrsf->vx.vy / theTrsf->vx.vx;
  chamfercoef_voxel_size[2] = theTrsf->vx.vz / theTrsf->vx.vx;

  if ( buildChamferMask( chamfercoef_voxel_size, chamfercoef_dim, chamfercoef_size,
                         chamfercoef_max, _UNDEFINED_DISTANCE_, &theMask ) != 1 ) {
    free( bufLabel );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when computing masks\n", proc );
    return( -1 );
  }

  /* distance buffer allocation
   */
  theDist = (unsigned short int*)malloc( v * sizeof(u16) );
  if ( theDist == (unsigned short int*)NULL ) {
    freeChamferMask( &theMask );
    free( bufLabel );
    if ( _verbose_ )
      fprintf( stderr, "%s: distance buffer allocation error\n", proc );
    return( -1 );
  }



  /* skiz
   */
  if ( skizWithChamferDistance( bufLabel, typeLabel, theDist, USHORT,
                                theDim, &theMask, 0 ) != 1 ) {
    free( theDist );
    freeChamferMask( &theMask );
    free( bufLabel );
    if ( _verbose_ )
      fprintf( stderr, "%s: error in skiz computation\n", proc );
    return( -1 );
  }



  /* transformation filling
   */
  if ( theMask.normalizationfactor > 0.0 ) {
    nfactor = theMask.normalizationfactor;
  }
  else {
    nfactor = getChamferMaskMin( &theMask );
  }
  nfactor /= theTrsf->vx.vx;

  bufWeights = (float***)theWeights->array;
  switch ( theTrsf->type ) {
  default :
    free( theDist );
    freeChamferMask( &theMask );
    free( bufLabel );
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
    return( -1 );
  break;
  case VECTORFIELD_3D :
    bufVz = (float***)(theTrsf->vz.array);
  case VECTORFIELD_2D :
    bufVx = (float***)(theTrsf->vx.array);
    bufVy = (float***)(theTrsf->vy.array);
    break;
  }



  switch ( estimator->type ) {
  default :
    free( theDist );
    freeChamferMask( &theMask );
    free( bufLabel );
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
    return( -1 );
  case TYPE_LS :
  case TYPE_LTS :
    switch( typeLabel ) {
    default :
      free( theDist );
      freeChamferMask( &theMask );
      free( bufLabel );
      if ( _verbose_ )
        fprintf( stderr, "%s: weird, this should not be reached\n", proc );
      return( -1 );
    case UCHAR :
        _SKIZ_UNWEIGHTED_FILLING( u8 );
        break;
    case USHORT :
        _SKIZ_UNWEIGHTED_FILLING( u16 );
        break;
    }
    break;
  case TYPE_WLS :
  case TYPE_WLTS :
    switch( typeLabel ) {
    default :
      free( theDist );
      freeChamferMask( &theMask );
      free( bufLabel );
      if ( _verbose_ )
        fprintf( stderr, "%s: weird, this should not be reached\n", proc );
      return( -1 );
    case UCHAR :
        _SKIZ_WEIGHTED_FILLING( u8 );
        break;
    case USHORT :
        _SKIZ_WEIGHTED_FILLING( u16 );
        break;
    }
    break;

  }

  switch( typeLabel ) {
  default :
    free( theDist );
    freeChamferMask( &theMask );
    free( bufLabel );    if ( _verbose_ )
      fprintf( stderr, "%s: weird, this should not be reached\n", proc );
    return( -1 );
  case UCHAR :
    {
      u8 *theLabel = (u8*)bufLabel;
      for ( i=0, z=0; z<theDim[2]; z++ )
      for ( y=0; y<theDim[1]; y++ )
      for ( x=0; x<theDim[0]; x++, i++ ) {
        if ( theDist[i] == 0 ) {
            bufVx[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.x;
            bufVy[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.y;
            if ( theTrsf->type == VECTORFIELD_3D )
                bufVz[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.z;
            switch ( estimator->type ) {
            default :
              free( theDist );
              freeChamferMask( &theMask );
              free( bufLabel );
              if ( _verbose_ )
                fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
              return( -1 );
            case TYPE_LS :
            case TYPE_LTS :
              bufWeights[z][y][x] = 1.0;
              break;
            case TYPE_WLS :
            case TYPE_WLTS :
              bufWeights[z][y][x] = field->pointer[ theLabel[i]-1 ]->rho;
              break;
            }
        }
        else {
          distance = (double)theDist[i] / nfactor;
          if ( distance >= estimator->propagation.fading + estimator->propagation.constant ) {
              bufVx[z][y][x] = 0.0;
              bufVy[z][y][x] = 0.0;
              if ( theTrsf->type == VECTORFIELD_3D )
                  bufVz[z][y][x] = 0.0;
              bufWeights[z][y][x] = 0.0;
              continue;
          }
          if ( distance <= estimator->propagation.constant )
            c = 1.0;
          else  if ( estimator->propagation.fading > 0.0 )
            c = ( estimator->propagation.fading + estimator->propagation.constant - distance ) / estimator->propagation.fading;
          bufVx[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.x;
          bufVy[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.y;
          if ( theTrsf->type == VECTORFIELD_3D )
              bufVz[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.z;
          switch ( estimator->type ) {
          default :
            free( theDist );
            freeChamferMask( &theMask );
            free( bufLabel );
            if ( _verbose_ )
              fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
            return( -1 );
          case TYPE_LS :
          case TYPE_LTS :
            bufWeights[z][y][x] = 1.0;
            break;
          case TYPE_WLS :
          case TYPE_WLTS :
            bufWeights[z][y][x] = field->pointer[ theLabel[i]-1 ]->rho;
            break;
          }

        }
      }
    }
    break;
  }


if ( 0 ) {
  switch( typeLabel ) {
  default :
    free( theDist );
    freeChamferMask( &theMask );
    free( bufLabel );    if ( _verbose_ )
      fprintf( stderr, "%s: weird, this should not be reached\n", proc );
    return( -1 );
  case UCHAR :
    {
      u8 *theLabel = (u8*)bufLabel;
      for ( i=0, z=0; z<theDim[2]; z++ )
      for ( y=0; y<theDim[1]; y++ )
      for ( x=0; x<theDim[0]; x++, i++ ) {
        if ( theDist[i] == 0 ) {
            bufVx[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.x;
            bufVy[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.y;
            if ( theTrsf->type == VECTORFIELD_3D )
                bufVz[z][y][x] = field->pointer[ theLabel[i]-1 ]->vector.z;
            switch ( estimator->type ) {
            default :
              free( theDist );
              freeChamferMask( &theMask );
              free( bufLabel );
              if ( _verbose_ )
                fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
              return( -1 );
            case TYPE_LS :
            case TYPE_LTS :
              bufWeights[z][y][x] = 1.0;
              break;
            case TYPE_WLS :
            case TYPE_WLTS :
              bufWeights[z][y][x] = field->pointer[ theLabel[i]-1 ]->rho;
              break;
            }
        }
        else {
          distance = (double)theDist[i] / nfactor;
          if ( distance >= estimator->propagation.fading + estimator->propagation.constant ) {
              bufVx[z][y][x] = 0.0;
              bufVy[z][y][x] = 0.0;
              if ( theTrsf->type == VECTORFIELD_3D )
                  bufVz[z][y][x] = 0.0;
              bufWeights[z][y][x] = 0.0;
              continue;
          }
          if ( distance <= estimator->propagation.constant )
            c = 1.0;
          else  if ( estimator->propagation.fading > 0.0 )
            c = ( estimator->propagation.fading + estimator->propagation.constant - distance ) / estimator->propagation.fading;
          bufVx[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.x;
          bufVy[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.y;
          if ( theTrsf->type == VECTORFIELD_3D )
              bufVz[z][y][x] = c * field->pointer[ theLabel[i]-1 ]->vector.z;
          switch ( estimator->type ) {
          default :
            free( theDist );
            freeChamferMask( &theMask );
            free( bufLabel );
            if ( _verbose_ )
              fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
            return( -1 );
          case TYPE_LS :
          case TYPE_LTS :
            bufWeights[z][y][x] = 1.0;
            break;
          case TYPE_WLS :
          case TYPE_WLTS :
            bufWeights[z][y][x] = field->pointer[ theLabel[i]-1 ]->rho;
            break;
          }

        }
      }
    }
    break;
  }
}

  free( theDist );
  freeChamferMask( &theMask );
  free( bufLabel );
  return( 1 );
}






/************************************************************
 *
 * vector field filling, mapping of field
 *
 ************************************************************/



static int _VectorField_Estimation_Filling( bal_transformation* theTrsf,
                                            bal_image *theWeights,
                                            FIELD *field,
                                            enumTypeEstimator type )
{
  char *proc = "_VectorField_Estimation_Filling";
  float ***bufWeights, ***bufVx, ***bufVy, ***bufVz;
  int i, j, k;
  size_t n;

  bufWeights = (float***)theWeights->array;

  switch ( type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
    return( -1 );

  case TYPE_LS :
  case TYPE_LTS :

    switch ( theTrsf->type ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
      return( -1 );
    break;
    case VECTORFIELD_2D :
      bufVx = (float***)(theTrsf->vx.array);
      bufVy = (float***)(theTrsf->vy.array);

      for ( n=0; n<field->n_selected_pairs; n++ ) {
        i = (int)(field->pointer[n]->origin.x + 0.5);
        j = (int)(field->pointer[n]->origin.y + 0.5);
        k = (int)(field->pointer[n]->origin.z + 0.5);
        bufVx[k][j][i] += field->pointer[n]->vector.x;
        bufVy[k][j][i] += field->pointer[n]->vector.y;
        bufWeights[k][j][i] ++;
      }
      break;
    case VECTORFIELD_3D :
      bufVx = (float***)(theTrsf->vx.array);
      bufVy = (float***)(theTrsf->vy.array);
      bufVz = (float***)(theTrsf->vz.array);

      for ( n=0; n<field->n_selected_pairs; n++ ) {
        i = (int)(field->pointer[n]->origin.x + 0.5);
        j = (int)(field->pointer[n]->origin.y + 0.5);
        k = (int)(field->pointer[n]->origin.z + 0.5);
        bufVx[k][j][i] += field->pointer[n]->vector.x;
        bufVy[k][j][i] += field->pointer[n]->vector.y;
        bufVz[k][j][i] += field->pointer[n]->vector.z;
        bufWeights[k][j][i] ++;
      }
      break;
    }
    break;

  case TYPE_WLS :
  case TYPE_WLTS :

      switch ( theTrsf->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such transformation type not handled yet (but weird)\n", proc );
        return( -1 );
      break;
      case VECTORFIELD_2D :
        bufVx = (float***)(theTrsf->vx.array);
        bufVy = (float***)(theTrsf->vy.array);

        for ( n=0; n<field->n_selected_pairs; n++ ) {
          i = (int)(field->pointer[n]->origin.x + 0.5);
          j = (int)(field->pointer[n]->origin.y + 0.5);
          k = (int)(field->pointer[n]->origin.z + 0.5);
          bufVx[k][j][i] += field->pointer[n]->rho * field->pointer[n]->vector.x;
          bufVy[k][j][i] += field->pointer[n]->rho * field->pointer[n]->vector.y;
          bufWeights[k][j][i] += field->pointer[n]->rho;
        }
        break;
      case VECTORFIELD_3D :
        bufVx = (float***)(theTrsf->vx.array);
        bufVy = (float***)(theTrsf->vy.array);
        bufVz = (float***)(theTrsf->vz.array);

        for ( n=0; n<field->n_selected_pairs; n++ ) {
          i = (int)(field->pointer[n]->origin.x + 0.5);
          j = (int)(field->pointer[n]->origin.y + 0.5);
          k = (int)(field->pointer[n]->origin.z + 0.5);
          bufVx[k][j][i] += field->pointer[n]->rho * field->pointer[n]->vector.x;
          bufVy[k][j][i] += field->pointer[n]->rho * field->pointer[n]->vector.y;
          bufVz[k][j][i] += field->pointer[n]->rho * field->pointer[n]->vector.z;
          bufWeights[k][j][i] += field->pointer[n]->rho;
        }
        break;
      }
      break;

  }

  return( 1 );
}






/************************************************************
 *
 *
 *
 ************************************************************/





static int _VectorField_Estimation( bal_transformation *theTrsf,
                                       FIELD *field,
                                       bal_estimator *estimator )
{
  char *proc="_VectorField_Estimation";
  bal_image theWeights;
  bal_doublePoint sigma = estimator->sigma;


  /* initialization
   */
  if ( BAL_InitAllocImage( &theWeights, "weights.inr", 
                              theTrsf->vx.ncols, theTrsf->vx.nrows, theTrsf->vx.nplanes, 1, FLOAT) == -1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate weight image\n", proc );
    return ( -1 );
  }


  /* initialization
   */
  if ( _VectorField_Estimation_Initialization( theTrsf, &theWeights ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error in initialization\n", proc );
      BAL_FreeImage( &theWeights );
      return( -1 );
  }


  /* vector filling
   */
  if ( estimator->propagation.constant > 0.0 || estimator->propagation.fading > 0.0 ) {
      switch ( estimator->propagation.type ) {
      default :
      case TYPE_DIRECT_PROPAGATION :
          if ( _VectorField_Direct_Propagation( theTrsf, &theWeights, field, estimator ) != 1 ) {
              if ( _verbose_ )
                fprintf( stderr, "%s: error in direct propagation\n", proc );
              BAL_FreeImage( &theWeights );
              return( -1 );
          }
          break;
      case TYPE_SKIZ_PROPAGATION :
          if ( _VectorField_Skiz_Propagation( theTrsf, &theWeights, field, estimator ) != 1 ) {
              if ( _verbose_ )
                fprintf( stderr, "%s: error in direct propagation\n", proc );
              BAL_FreeImage( &theWeights );
              return( -1 );
          }
          break;
      }


  }
  else {
      if ( _VectorField_Estimation_Filling( theTrsf, &theWeights, field, estimator->type ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: error in initialization\n", proc );
          BAL_FreeImage( &theWeights );
          return( -1 );
      }
  }


  /* smoothing
   * ie fluid regularization
   */
  if ( _VectorField_Estimation_Smoothing( theTrsf, &theWeights, &sigma ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error in normalization\n", proc );
      BAL_FreeImage( &theWeights );
      return( -1 );
  }


  /* normalization
   */
  if ( _VectorField_Estimation_Normalization( theTrsf, &theWeights ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error in normalization\n", proc );
      BAL_FreeImage( &theWeights );
      return( -1 );
  }

  BAL_FreeImage( &theWeights );

  return( 1 );
}





/************************************************************
 *
 * residuals
 *
 ************************************************************/



typedef struct {
  bal_transformation *theTrsf;
  FIELD *field;
} _VectorFieldResidualsParam;





static void *_VectorField_Residuals_2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  FIELD *field = ((_VectorFieldResidualsParam*)parameter)->field;
  float ***bufVx = (float***)(((_VectorFieldResidualsParam*)parameter)->theTrsf->vx.array);
  float ***bufVy = (float***)(((_VectorFieldResidualsParam*)parameter)->theTrsf->vy.array);

  size_t n;
  int i, j, k;
  double dx, dy;

  for ( n = first; n <= last; n ++ ) {
    i = (int)(field->pointer[n]->origin.x + 0.5);
    j = (int)(field->pointer[n]->origin.y + 0.5);
    k = (int)(field->pointer[n]->origin.z + 0.5);
    dx = bufVx[k][j][i] - field->pointer[n]->vector.x;
    dy = bufVy[k][j][i] - field->pointer[n]->vector.y;
    field->pointer[n]->error = dx*dx + dy*dy;
  }
  chunk->ret = 1;
  return( (void*)NULL );
}



static void *_VectorField_Residuals_3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  FIELD *field = ((_VectorFieldResidualsParam*)parameter)->field;
  float ***bufVx = (float***)(((_VectorFieldResidualsParam*)parameter)->theTrsf->vx.array);
  float ***bufVy = (float***)(((_VectorFieldResidualsParam*)parameter)->theTrsf->vy.array);
  float ***bufVz = (float***)(((_VectorFieldResidualsParam*)parameter)->theTrsf->vz.array);

  size_t n;
  int i, j, k;
  double dx, dy, dz;

  for ( n = first; n <= last; n ++ ) {
    i = (int)(field->pointer[n]->origin.x + 0.5);
    j = (int)(field->pointer[n]->origin.y + 0.5);
    k = (int)(field->pointer[n]->origin.z + 0.5);
    dx = bufVx[k][j][i] - field->pointer[n]->vector.x;
    dy = bufVy[k][j][i] - field->pointer[n]->vector.y;
    dz = bufVz[k][j][i] - field->pointer[n]->vector.z;
    field->pointer[n]->error = dx*dx + dy*dy + dz*dz; 
  }
  chunk->ret = 1;
  return( (void*)NULL );
}



int BAL_VectorField_Residuals( bal_transformation* theTrsf, 
                                      FIELD * field )
{
  char *proc = "BAL_VectorField_Residuals";
  int n;

  _VectorFieldResidualsParam p;
  typeChunks chunks;
  size_t first, last;
  first = 0;
  last = field->n_computed_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( -1 );
  }
  
  p.theTrsf = theTrsf;
  p.field = field;

  for ( n=0; n<chunks.n_allocated_chunks; n++ )
    chunks.data[n].parameters = (void*)(&p);





  switch( theTrsf->type ) {
  default :
    if ( _verbose_ )
      fprintf (stderr, "%s: such transformation not handled in switch\n", proc );
    freeChunks( &chunks );
    return( -1 );
    
  case VECTORFIELD_2D :
    
    if ( processChunks( &_VectorField_Residuals_2DSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to compute residuals (2D vector field case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;
      
  case VECTORFIELD_3D :
      
    if ( processChunks( &_VectorField_Residuals_3DSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to compute residuals (3D vector field case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;
    
  }
  
  freeChunks( &chunks );
  return( 1 );

}





/************************************************************
 *
 * trimmed estimation
 *
 ************************************************************/



static int _VectorField_Trimmed_Estimation( bal_transformation* theTrsf, 
                                                 FIELD * field,
                                                 bal_estimator *estimator )
{
  char *proc="_VectorField_Trimmed_Estimation";

  int nretainedpairs;

  int iter;



  /* initial transformation estimation
   */
  field->n_selected_pairs = field->n_computed_pairs;
  if ( _VectorField_Estimation( theTrsf, field, estimator ) != 1 ) {
    if ( _verbose_ )
      fprintf ( stderr, "%s: error when estimating initial transformation\n", proc );
    return( -1 );
  }



  /* loop initialisation
   */
  for ( iter = 0; iter < estimator->max_iterations; iter ++ ) {

    /* calcul des residus
     */
    if ( BAL_VectorField_Residuals( theTrsf, field ) != 1 ) {
      if ( _verbose_ )
         fprintf (stderr, "%s: unable to compute residuals\n", proc );
      return( -1 );
    }


    /* sort residuals
     */
    nretainedpairs = BAL_SelectSmallestResiduals( field, estimator );
    if ( nretainedpairs <= 0 ) {
      if ( _verbose_ )
         fprintf (stderr, "%s: no retained residuals? Returned value was %d\n", proc, nretainedpairs );
      return( -1 );
    }
    field->n_selected_pairs = nretainedpairs;
    
    
    
    /* transformation estimation
     */
    if ( _VectorField_Estimation( theTrsf, field, estimator ) != 1 ) {
      if ( _verbose_ ) {
         fprintf( stderr, "%s: something goes wrong in the transformation estimation\n", proc );
         fprintf( stderr, "\t iteration #%d of the iterated (trimmed) estimation\n", iter );
      }
      return( -1 );
    }


    
    if ( _verbose_ >= 3 ) {
#ifdef _ORIGINAL_BALADIN_PRINTS_
      switch ( estimator->type ) {
      default: break;
      case TYPE_LS :
      case TYPE_LTS :
         fprintf( stderr, "      LTS Iteration n. %d \r", iter);
         break;
      case TYPE_WLS :
      case TYPE_WLTS :
         fprintf( stderr, "      WLTS Iteration n. %d \r", iter);
         break;
      }
#else
      switch ( estimator->type ) {
      default: break;
      case TYPE_LS :
      case TYPE_LTS :
         fprintf( stderr, "      LTS: iteration #%2d ... \r", iter );
         break;
      case TYPE_WLS :
      case TYPE_WLTS :
         fprintf( stderr, "      WLTS: iteration #%2d ... \r", iter );
         break;
      }
#endif
    }

  }
    
  return( 1 );
}





/************************************************************
 *
 * estimation
 *
 ************************************************************/



/* Compute the transformation from the reference image 
   towards the floating one, thus allows to resample the floating
   in the reference frame.
   
   Pairings are in voxel units.
*/
int BAL_ComputeVectorFieldTransformation( bal_transformation* theTrsf, 
                                               FIELD * field,
                                               bal_estimator *estimator )
{
  char *proc= "BAL_ComputeVectorFieldTransformation";
  switch ( estimator->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
    return( -1 );
  case TYPE_LS :
  case TYPE_WLS :
    return( _VectorField_Estimation( theTrsf, field, estimator ) );
  case TYPE_LTS :
  case TYPE_WLTS :
    return( _VectorField_Trimmed_Estimation( theTrsf, field, estimator ) );
  }
  
  return( 1 );
}
