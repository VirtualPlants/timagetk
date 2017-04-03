/*************************************************************************
 * topological-operations-common.h -
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


#ifndef _topological_operations_common_h_
#define _topological_operations_common_h_

#ifdef __cplusplus
extern "C" {
#endif



extern void setVerboseInTopologicalOperationsCommon( int v );
extern void incrementVerboseInTopologicalOperationsCommon(  );
extern void decrementVerboseInTopologicalOperationsCommon(  );



/**************************************************
 *
 * point management
 *
 **************************************************/



/**************************************************
 *
 * point management
 *
 **************************************************/

/* definition of some values
 * warning: ordering is important!
 * _BACKGROUND_VALUE_ = 0
 * < _CANBECHANGED_VALUE_
 * < _TOBECHANGED_
 * < _ENDPOINT_
 * < _FOREGROUND_VALUE_ = _ANCHOR_
 */
#define _BACKGROUND_VALUE_ 0
#define _CANBECHANGED_VALUE_ 150
#define _TOBECHANGED_VALUE_ 200
#define _ENDPOINT_VALUE_ 225
#define _ANCHOR_VALUE_ 255
#define _FOREGROUND_VALUE_ 255

typedef enum enumTypeTopologicalPoint {
    _BACKGROUND_ = _BACKGROUND_VALUE_,

  /* these were for the thinning operation
   */
  _CANBECHANGED_ = _CANBECHANGED_VALUE_,
  _TOBECHANGED_ = _TOBECHANGED_VALUE_,
  _ENDPOINT_ = _ENDPOINT_VALUE_,
  _ANCHOR_ = _ANCHOR_VALUE_,

  /* these were for the thickening (by growing) operation
   */
  _INQUEUE_ = 150,      /* in the list, but not queued for addition */
  _WILLBEADDED_ = 200,  /* in the list, and queued for addition */
  _ADDED_ = 255
} enumTypeTopologicalPoint;



typedef struct topologicalPoint {

  /* point location
   */
  int x;
  int y;
  int z;
  int i;

  /* point characterisation
   */
  char isinside;
  enumTypeTopologicalPoint type;

  /* additional attributes
   */
  int iteration;
  int value;

} topologicalPoint;

extern void initTopologicalPoint( topologicalPoint *p );



typedef struct topologicalPointList {
  topologicalPoint *data;
  int n_data;
  int n_allocated_data;
} topologicalPointList;

extern int addTopologicalPointToTopologicalPointList( topologicalPointList *l,
                                                      topologicalPoint *p );

typedef enum {
  _NO_SORTING_=0,
  _ITERATION_SORTING_=1,
  _DISTANCE_SORTING_=2,
  _VALUE_SORTING_=2,
} enumTypeSort;

extern void sortTopologicalPointList( topologicalPointList *l, enumTypeSort sortingCriterium );



typedef struct topologicalPointListList {
  topologicalPointList *data;
  int firstindex;
  int lastindex;
  int n_data;
  int n_allocated_data;
} topologicalPointListList;

extern void initTopologicalPointListList( topologicalPointListList *l );
extern void freeTopologicalPointListList( topologicalPointListList *l );
extern int allocTopologicalPointListList( topologicalPointListList *l,
                                          int firstindex, int lastindex );

/**************************************************
 *
 * neighborhood management
 *
 **************************************************/



typedef struct typeOffset {
  int dx;
  int dy;
  int dz;
  int di;
} typeOffset;



typedef struct typeNeighborhood {
  int nneighbors;
  typeOffset neighbors[27];
} typeNeighborhood;

/* build the offsets array for the given
 * neighborhood. The central point is not included.
 * To be used for propagation strategies
 */
extern void defineNeighborsTobBeAdded( typeNeighborhood *n,
                                       int *theDim,
                                       int connectivity );

/* build the offsets array for the 9- or the 27-
 * neighborhood, depending on the connectivity.
 * The central point is included.
 */
extern void defineNeighborsForSimplicity( typeNeighborhood *n,
                                          int *theDim,
                                          int connectivity );

/* extract the neighborhood. Useful for 9- and 27-
 * neighborhoods.
 */
extern void extractNeighborhood( int *neighb,
                                 topologicalPoint *pt,
                                 unsigned char *resBuf,
                                 int *theDim,
                                 typeNeighborhood *neighbors );

extern void fprintfNeighborhood( FILE *f, int *neighb, int n );

#ifdef __cplusplus
}
#endif


#endif
