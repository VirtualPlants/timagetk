/****************************************************
 * topological-thickening.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun 29 fév 2016 18:28:00 CET
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

#include <string.h>

#include <t04t08.h>
#include <t06t26.h>

#include <topological-operations-common.h>
#include <topological-thickening.h>



static int _verbose_ = 1;

void setVerboseInTopologicalThickening( int v )
{
  _verbose_ = v;
}

void incrementVerboseInTopologicalThickening(  )
{
  _verbose_ ++;
}

void decrementVerboseInTopologicalThickening(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}








/**************************************************
 *
 * parameter management
 *
 **************************************************/


void initTypeThickeningParameters( typeThickeningParameters *p )
{
  p->maxIteration = -1;
  p->connectivity = 26;
  p->theMask = (typeChamferMask*)NULL;
  p->additionalSorting = _NO_SORTING_;
}














/**************************************************
 *
 * interiority tests
 *
 **************************************************/



typedef int (*typeIsInsideFunction)( int, int, int, int * );



/* __attribute__ ((unused))
 * allows to suppress the "warning: unused parameter ‘z’"
 * when compiled with gcc
 */
static int _is2DInside( int x, int y, int z __attribute__ ((unused)), int *theDim )
{
  if ( 0 < x && x < theDim[0]-1 &&
       0 < y && y < theDim[1]-1 )
    return( 1 );
  return( 0 );
}



static int _is3DInside( int x, int y, int z, int *theDim )
{
  if ( 0 < x && x < theDim[0]-1 &&
       0 < y && y < theDim[1]-1 &&
       0 < z && z < theDim[2]-1 )
    return( 1 );
  return( 0 );
}










/**************************************************
 *
 * simplicity tests
 *
 **************************************************/



typedef int (*typeIsPointSimple)( int *,
                                  int *,
                                  int * );



static int _is2DPoint08Simple( int *neighb, int *t04, int *t08 )
{
  int checkT04, checkT08;
  int n;

  for ( n=0; n<9; n++ )
    if ( neighb[n] == _INQUEUE_ )
      neighb[n] = _BACKGROUND_;

  Compute_T04_and_T08( neighb, t04, t08 );

  if ( *t04 != 1 || *t08 != 1 ) return( 0 );

  for ( n=0; n<9; n++ )
    if ( neighb[n] == _WILLBEADDED_ )
      neighb[n] = _BACKGROUND_;

  Compute_T04_and_T08( neighb, &checkT04, &checkT08 );

  if ( checkT04 != 1 || checkT08 != 1 ) return( 0 );

  return( 1 );
}



static int _is3DPoint26Simple( int *neighb, int *t06, int *t26 )
{
  int checkT06, checkT26;
  int n;

  for ( n=0; n<27; n++ )
    if ( neighb[n] == _INQUEUE_ )
      neighb[n] = _BACKGROUND_;

  Compute_T06_and_T26( neighb, t06, t26 );

  if ( *t06 != 1 || *t26 != 1 ) return( 0 );

  for ( n=0; n<27; n++ )
    if ( neighb[n] == _WILLBEADDED_ )
      neighb[n] = _BACKGROUND_;

  Compute_T06_and_T26( neighb, &checkT06, &checkT26 );

  if ( checkT06 != 1 || checkT26 != 1 ) return( 0 );

  return( 1 );
}










/**************************************************
 *
 * thickening initialization
 *
 **************************************************/


/* get the point of maximal value
 */

int InitializeThickeningImage( unsigned char *resBuf,
                               unsigned short *theDistance,
                               int *theDim )
{
  int i, max;
  int v = theDim[0]*theDim[1]*theDim[2];


  max = theDistance[0];
  for ( i=1; i<v; i++ )
    if ( max < theDistance[i] )
      max = theDistance[i];

  for ( i=0; i<v; i++ )
    resBuf[i] = 0;

  for ( i=0; i<v; i++ )
    if ( max == theDistance[i] ) {
      resBuf[i] = 255;
      break;
    }

  return( 1 );
}










/**************************************************
 *
 * thickening tools
 *
 **************************************************/



/* build a first set of points from an image
 * consider all non-zero points
 */
static int _initiateThickening( topologicalPointListList *pointListList,
                                unsigned char *resBuf,
                                unsigned short *theDistance,
                                int *theDim,
                                typeIsInsideFunction _isInside )
{
    char *proc = "_initiateThickening";
    int nInitialPoints = 0;
    int i, x, y, z;
    topologicalPoint point;



    for ( nInitialPoints=0, i=0, z=0; z<theDim[2]; z++ )
    for ( y=0; y<theDim[1]; y++ )
    for ( x=0; x<theDim[0]; x++, i++ ) {
      if ( resBuf[i] > 0 ) {
        initTopologicalPoint( &point );
        point.x = x;
        point.y = y;
        point.z = z;
        point.i = i;
        point.iteration = 0;
        point.value = 0;
        point.isinside = (*_isInside)( point.x, point.y, point.z, theDim );
        resBuf[i] =  point.type = _WILLBEADDED_;
        if ( addTopologicalPointToTopologicalPointList( &(pointListList->data[theDistance[i]]),
                                                        &point ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to add point (%d,%d,%d) to list\n", proc, x, y, z );
          return( -1 );
        }
        nInitialPoints ++;
      }
    }

    if ( _verbose_ >= 2 ) {
      fprintf( stderr, " ... found %d starting points\n", nInitialPoints );
    }

    return( nInitialPoints );
}





static int _addNeighbors( unsigned char *resBuf,
                          unsigned short *theDistance,
                          int *theDim,
                          topologicalPointListList *pointListList,
                          typeNeighborhood *neighbors,
                          typeIsInsideFunction _isInside,
                          int iteration )
{
  char *proc = "_addNeighbors";
  int d, p, n;
  topologicalPointList *pointList;
  topologicalPoint point;
  int x, y, z, i;
  int xn, yn, zn, in;
  int naddedpoints = 0;

  /* recall that
   * - pointListList.firstindex = 0
   * - pointListList.lastindex = maxPossibleDistance
   */
  for ( d=pointListList->lastindex; d>=1; d-- ) {
    pointList = &(pointListList->data[d]);

    for ( p = 0; p < pointList->n_data; p++ ) {

      if ( pointList->data[p].type != _WILLBEADDED_ ) continue;

      /* the point has just been added
         get its neighbors and put them in queue
       */
      x = pointList->data[p].x;
      y = pointList->data[p].y;
      z = pointList->data[p].z;
      i = pointList->data[p].i;

      if ( pointList->data[p].isinside ) {

        for ( n = 0; n < neighbors->nneighbors; n++ ) {
          in = i + neighbors->neighbors[n].di;
          if ( theDistance[in] > 0 && resBuf[in] == 0 ) {
            initTopologicalPoint( &point );
            point.x = x + neighbors->neighbors[n].dx;
            point.y = y + neighbors->neighbors[n].dy;
            point.z = z + neighbors->neighbors[n].dz;
            point.i = in;
            point.iteration = iteration;
            point.value = 0;
            point.isinside = (*_isInside)( point.x, point.y, point.z, theDim );
            resBuf[in] =  point.type = _INQUEUE_;
            if ( addTopologicalPointToTopologicalPointList( &(pointListList->data[theDistance[in]]),
                                                            &point ) != 1 ) {
              if ( _verbose_ )
                fprintf( stderr, "%s: unable to add point to list\n", proc );
              return( -1 );
            }
            naddedpoints ++;
          }
        }

      }
      else {

        for ( n = 0; n < neighbors->nneighbors; n++ ) {

          xn = x + neighbors->neighbors[n].dx;
          if ( xn < 0 || xn >= theDim[0] ) continue;
          yn = y + neighbors->neighbors[n].dy;
          if ( yn < 0 || yn >= theDim[1] ) continue;
          zn = z + neighbors->neighbors[n].dz;
          if ( zn < 0 || zn >= theDim[2] ) continue;

          in = i + neighbors->neighbors[n].di;
          if ( theDistance[in] > 0 && resBuf[in] == 0 ) {
            initTopologicalPoint( &point );
            point.x = xn;
            point.y = yn;
            point.z = zn;
            point.i = in;
            point.iteration = iteration;
            point.value = 0;
            point.isinside = (*_isInside)( point.x, point.y, point.z, theDim );
            resBuf[in] =  point.type = _INQUEUE_;
            if ( addTopologicalPointToTopologicalPointList( &(pointListList->data[theDistance[in]]),
                                                            &point ) != 1 ) {
              if ( _verbose_ )
                fprintf( stderr, "%s: unable to add point to list\n", proc );
              return( -1 );
            }
            naddedpoints ++;
          }
        }

      }

      /* neighbors have been added
         change the point type
         remove it from the list
       */
      resBuf[ pointList->data[p].i ] = pointList->data[p].type = _ADDED_;

      point = pointList->data[ pointList->n_data-1 ];
      pointList->data[ pointList->n_data-1 ] = pointList->data[p];
      pointList->data[p] = point;
      pointList->n_data --;
      p --;
    }
  }

  return( naddedpoints );
}










/**************************************************
 *
 * thickening
 *
 **************************************************/





int ThickeningImage( unsigned char *resBuf,
                     unsigned short *theDistance,
                     unsigned short *thePropagation,
                     int *theDim,
                     typeThickeningParameters *par )
{
  char *proc = "ThickeningImage";

  int connectivity = par->connectivity;
  
  int maxPossibleDistance = 65535;
  topologicalPointListList pointListList;
  topologicalPointList *pointList;
  int x, y, z;
  int d, i, n, p;
  int v = theDim[0]*theDim[1]*theDim[2];

  typeNeighborhood offsetToAddPoints;
  typeNeighborhood offsetForSimplicity;
  int neighb[27];
  int tback, tfore;

  /* topologicalPoint point; */

  int iteration = 0;
  int nInitialPoints = 0;
  int nAddedPointsQueue, nAddedPointsObjects;

  typeIsInsideFunction _isInside = (typeIsInsideFunction)NULL;
  typeIsPointSimple _isPointSimple = (typeIsPointSimple)NULL;

  enumTypeSort sortingCriterium = par->additionalSorting;
  topologicalPoint firstPoint;
  int apointhasbeenadded, stopparsingpoints;




  /*--------------------------------------------------
   *
   * 
   *
   --------------------------------------------------*/
  
  if ( sortingCriterium == _DISTANCE_SORTING_ ) {
    if ( par->theMask == (typeChamferMask *)NULL ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: no chamfer mask, switch to no additional sorting\n", proc );
      }
      par->additionalSorting = sortingCriterium = _NO_SORTING_;
    }
    else {
      for ( n=0; n<par->theMask->nb; n++ ) {
        par->theMask->list[n].o = par->theMask->list[n].z * theDim[0]*theDim[1]
          + par->theMask->list[n].y * theDim[0]
          + par->theMask->list[n].x;
      }
    }
  }

  if ( _verbose_ >= 3 ) { 
    fprintf( stderr, "%s: sorting criterium = ", proc );
    switch ( sortingCriterium ) {
    default :
      fprintf( stderr, "default \n" ); break;
    case _NO_SORTING_ :
      fprintf( stderr, "none \n" ); break;
    case _ITERATION_SORTING_ :
      fprintf( stderr, "iteration \n" ); break;
    case _DISTANCE_SORTING_ :
      fprintf( stderr, "distance \n" ); break;
    }
  }


  /*--------------------------------------------------
   *
   * connectivity based choices
   *
   --------------------------------------------------*/

  switch( connectivity ) {
  case 4 :
  case 8 :
  case 6 :
  case 18 :
  case 26 :
    break;
  default :
    connectivity = 26;
  }
    
  if ( theDim[2] == 1 ) {
    switch( connectivity ) {
    case 6 :
      connectivity = 4; break;
    case 18 :
    case 26 :
      connectivity = 8; break;
    default :
      break;
    }
  }

  if ( _verbose_ >= 2 )
    fprintf( stderr, "%s: connectivity set to %d\n", proc, connectivity );
    
  switch( connectivity ) {
  default :
    if ( _verbose_ ) 
      fprintf( stderr, "%s: weird situation\n", proc );
    return( -1 );
  case 4 :
    _isInside = &_is2DInside;
    _isPointSimple = &_is2DPoint08Simple;
    break;
  case 8 :
    _isInside = &_is2DInside;
    _isPointSimple = &_is2DPoint08Simple;
    break;
  case 6 :
    _isInside = &_is3DInside;
    _isPointSimple = &_is3DPoint26Simple;
    break;
  case 18 :
    _isInside = &_is3DInside;
    _isPointSimple = &_is3DPoint26Simple;
    break;
  case 26 :
    _isInside = &_is3DInside;
    _isPointSimple = &_is3DPoint26Simple;
    break;
  }


  /*--------------------------------------------------
   *
   * pre-compute offsets for point access speed-up
   *
   --------------------------------------------------*/
 
  defineNeighborsTobBeAdded( &offsetToAddPoints, theDim, connectivity );
  defineNeighborsForSimplicity( &offsetForSimplicity, theDim, connectivity );




  /*--------------------------------------------------
   *
   * build point list
   *
   --------------------------------------------------*/
  
  initTopologicalPointListList( &pointListList );

  if ( allocTopologicalPointListList( &pointListList, 0, maxPossibleDistance ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate point list array\n", proc );
      return( -1 );
  }

  /* looking for start points
     since the value is unknown, be sure it is correctly set
  */
  nInitialPoints = _initiateThickening( &pointListList, resBuf,
                                        theDistance, theDim,
                                        _isInside );
  if ( nInitialPoints <= 0 ) {
      if ( nInitialPoints < 0 )
      if ( _verbose_ ) {
          if ( nInitialPoints < 0 )
              fprintf( stderr, "%s: error when looking for initial points\n", proc );
          else
              fprintf( stderr, "%s: found no initial points\n", proc );
      }
      freeTopologicalPointListList( &pointListList );
      return( -1 );
  }

  if ( _verbose_ >= 2 ) {
    fprintf( stderr, " ... found %d starting points\n", nInitialPoints );
  }

  

  

  /*--------------------------------------------------
   *
   * initialization of propagation image
   *
   --------------------------------------------------*/

  if ( thePropagation != (unsigned short int*) NULL ) {
    for ( i=0; i<v; i++ ) 
      thePropagation[i] = 0;
  }





  /*--------------------------------------------------
   *
   * thickening
   *
   --------------------------------------------------*/

  do {

    iteration ++;
    
    /* update the propgation image with iteration number
     * or distance value
     * recall that
     * - pointListList.firstindex = 0
     * - pointListList.lastindex = maxPossibleDistance
     */
    switch( sortingCriterium ) {
    default :
    case _ITERATION_SORTING_ :
        for ( d=pointListList.firstindex; d<=pointListList.lastindex; d++ ) {
            pointList = &(pointListList.data[d]);
            for ( p=0; p<pointList->n_data; p++ ) {
                if ( pointList->data[p].type == _WILLBEADDED_ ) {
                    thePropagation[ pointList->data[p].i ] = iteration;
                }
            }
        }
        break;
    case _DISTANCE_SORTING_ :
        for ( d=pointListList.firstindex; d<=pointListList.lastindex; d++ ) {
            pointList = &(pointListList.data[d]);
            for ( p=0; p<pointList->n_data; p++ ) {
                if ( pointList->data[p].type == _WILLBEADDED_ ) {
                    thePropagation[ pointList->data[p].i ] = pointList->data[p].value;
                }
            }
        }
        break;
    }


    /* propagation step:
     * turn the _WILLBEADDED_ points into _ADDED_
     * remove them from the list
     * add their neighboring points to the list
     */
    nAddedPointsQueue = _addNeighbors( resBuf, theDistance, theDim,
                                       &pointListList,
                                       &offsetToAddPoints, _isInside, iteration );
    if ( nAddedPointsQueue == -1 ) {
        freeTopologicalPointListList( &pointListList );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to add points at iteration %d\n", proc, iteration );
        return( -1 );
    }
    
    
    /* identify points that can be added to the image
       turn them from _INQUEUE_ into _WILLBEADDED_
     * recall that
     * - pointListList.firstindex = 0
     * - pointListList.lastindex = maxPossibleDistance
     */
    for ( nAddedPointsObjects=0, d=pointListList.lastindex;
          d>=1 && nAddedPointsObjects==0 ; d-- ) {

      pointList = &(pointListList.data[d]);
      if ( pointList->n_data == 0 ) continue;

      /* update points
       * propagation distance for _INQUEUE_ points
       * (only for those that may be added)
       */
      
      switch( sortingCriterium ) {
      default :
      case _ITERATION_SORTING_ :
        break;
      case _DISTANCE_SORTING_ :
        for ( p=0; p<pointList->n_data; p++ ) {
          if ( pointList->data[p].type != _INQUEUE_ ) continue;
          for ( n=0; n < par->theMask->nb; n++ ) {
            x = pointList->data[p].x +  par->theMask->list[n].x;
            if ( x < 0 || theDim[0] <= x ) continue;
            y = pointList->data[p].y +  par->theMask->list[n].y;
            if ( y < 0 || theDim[1] <= y ) continue;
            z = pointList->data[p].z +  par->theMask->list[n].z;
            if ( z < 0 || theDim[2] <= z ) continue;
            if ( resBuf[ pointList->data[p].i + par->theMask->list[n].o ] != _ADDED_ )
              continue;
            if ( pointList->data[p].value > thePropagation[ pointList->data[p].i + par->theMask->list[n].o ] +  par->theMask->list[n].inc )
              pointList->data[p].value = thePropagation[ pointList->data[p].i + par->theMask->list[n].o ] +  par->theMask->list[n].inc;
          }
        }
        break;
      }


      /* sort points
       */
      if ( pointList->n_data > 1 ) {
        sortTopologicalPointList(  pointList, sortingCriterium );
      }

     if ( _verbose_ >= 3 )
        fprintf( stderr, "    parsing %d points at distance %d\n", pointList->n_data, d );
     
     

      /* process points 
       */

      for ( apointhasbeenadded=0, stopparsingpoints=0, p=0; 
            p<pointList->n_data && stopparsingpoints == 0; p++ ) {

        switch( sortingCriterium ) {
        default :
          break;
        case _ITERATION_SORTING_ :
          if ( apointhasbeenadded )
            if ( pointList->data[p].iteration != firstPoint.iteration )
              stopparsingpoints = 1;
          break;
        case _DISTANCE_SORTING_ :
          if ( apointhasbeenadded )
            if ( pointList->data[p].value != firstPoint.value )
              stopparsingpoints = 1;
          break;
        }

        /* seems that a stopping condition is missing
         */
        if ( stopparsingpoints == 1 )
            continue;

        /* get the neighborhood
         */

        extractNeighborhood( neighb, &(pointList->data[p]),
                             resBuf, theDim,
                              &offsetForSimplicity );

        /* test the simplicity
         */

        if ( (*_isPointSimple)( neighb, &tback, &tfore ) == 1 ) {
          /* if there is an end condition for simple points,
             it is to be tested here
          */
          pointList->data[p].type = resBuf[ pointList->data[p].i ] = _WILLBEADDED_;
          nAddedPointsObjects ++;

          if ( apointhasbeenadded == 0 ) {
            apointhasbeenadded = 1;
            firstPoint = pointList->data[p];
          }
        }

      }
      
      if ( _verbose_ >= 4 ) {
        fprintf( stderr, "  distance=%3d - %4d points added to object\n", d, nAddedPointsObjects );
      }

    }
    
    if ( _verbose_ >= 1 ) {
      fprintf( stderr, " #%6d", iteration );
      fprintf( stderr, " Points to queue=%6d", nAddedPointsQueue );
      fprintf( stderr, " Points to object=%6d", nAddedPointsObjects );
      fprintf( stderr, " distance=%d", d+1 );
      if ( _verbose_ >= 2 ) fprintf( stderr, "\n" );
      else                  fprintf( stderr, "\r" );
    }

  } while ( ((par->maxIteration >= 0 && iteration < par->maxIteration) || (par->maxIteration < 0))
            && nAddedPointsObjects > 0 );


  /* transform output image
   */
  for ( i=0; i<v; i++ ) {
    if ( resBuf[i] ==  _ADDED_ )
      resBuf[i] = 255;
    else
      resBuf[i] = 0;
  }

  return( 1 );
}
