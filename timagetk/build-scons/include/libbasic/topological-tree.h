/****************************************************
 * topological-tree.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Mar 22 mar 2016 11:08:03 CET
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#ifndef _topological_tree_h_
#define _topological_tree_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <typedefs.h>


extern void setVerboseInTopologicalTree( int v );
extern void incrementVerboseInTopologicalTree( );
extern void decrementVerboseInTopologicalTree( );

extern void setDebugInTopologicalTree( int d );
extern void incrementDebugInTopologicalTree(  );
extern void decrementDebugInTopologicalTree(  );


extern int countNeighborsInTreeImage( unsigned char *theBuf,
                           unsigned char *resBuf, int *theDim );


typedef struct typeComponentTreeImage {
  int firstComponentLabel;
  int lastComponentLabel;
  int firstJunctionLabel;
  int lastJunctionLabel;
  void *componentBuf;
  bufferType componentType;
  int theDim[3];
  float voxelSize[3];
} typeComponentTreeImage;

extern void initComponentTreeImage( typeComponentTreeImage *c );
extern int allocComponentTreeImage( typeComponentTreeImage *c,
                                    bufferType type, int *theDim );
extern void freeComponentTreeImage( typeComponentTreeImage *c );

extern int labelComponentsInTreeImage( unsigned char *theBuf,
                                  typeComponentTreeImage *tree );



typedef struct typeTreeFloatPoint {
  float x;
  float y;
  float z;
} typeTreeFloatPoint;

extern void initTreeFloatPoint( typeTreeFloatPoint *p );



typedef struct typeTreeEdge {
  int n_data;
  int n_allocated_data;
  int *data;
} typeTreeEdge;

typedef struct typeTree {
  int n_points;
  int n_allocated_points;
  typeTreeFloatPoint *points;
  int n_edges;
  int n_allocated_edges;
  typeTreeEdge *edges;
} typeTree;

extern void initTree( typeTree *t );
extern void freeTree( typeTree *t );

extern int treeImageToTree( typeComponentTreeImage *treeImage, typeTree *t );

extern int writeVTKLegacyFile( char *name, typeTree *tree, char *desc );

#ifdef __cplusplus
}
#endif

#endif
