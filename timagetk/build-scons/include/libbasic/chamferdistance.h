/*************************************************************************
 * chamfer.h - computation of chamfer distances
 *
 * $Id: chamfer.h,v 1.4 2000/08/08 14:48:26 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Tue Oct  7 15:33:09 CEST 2008
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 *
 */

#ifndef _chamferdistance_h_
#define _chamferdistance_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <math.h>

#include <stdio.h>

#include <typedefs.h>
#include <convert.h>


extern void setVerboseInChamferDistance( int v );
extern void incrementVerboseInChamferDistance(  );
extern void decrementVerboseInChamferDistance(  );


typedef enum {
  _DISTANCE04_,
  _DISTANCE08_,
  _CHAMFER3x3_,
  _CHAMFER5x5_,
  _DISTANCE06_,
  _DISTANCE10_,
  _DISTANCE18_,
  _DISTANCE26_,
  _CHAMFER3x3x3_,
  _CHAMFER5x5x5_,
  _UNDEFINED_DISTANCE_
} enumDistance;


typedef struct {
  int x;
  int y;
  int z;
  int o; /* offset */
  int inc;
} typeChamferPoint;

typedef struct {
  int nb;
  typeChamferPoint *list;
  double normalizationfactor;
} typeChamferMask;






extern void initChamferMask( typeChamferMask *theMask );
extern void freeChamferMask( typeChamferMask *theMask );
extern int allocChamferMask( typeChamferMask *theMask, int n );
extern void sortChamferMask( typeChamferMask* theMask );

extern int getChamferMaskMin( typeChamferMask* theMask );
extern int getForwardChamferMask( typeChamferMask* theMask, typeChamferMask* theForwardMask );
extern int getBackwardChamferMask( typeChamferMask* theMask, typeChamferMask* theForwardMask );


extern int printChamferMaskAsImage( FILE* fout, typeChamferMask* theMask );
extern void printChamferMaskAsList( FILE* fout, typeChamferMask* theMask );


extern int buildChamferMaskFromImage( void *theBuf,
                                          bufferType type,
                                          int *theDim,
                                          typeChamferMask *theMask );

extern int buildPredefinedChamferMask( enumDistance type, 
                                           typeChamferMask *theMask );

extern int buildShapeBasedInterpolationMasks( typeChamferMask* theMask, 
                                                   typeChamferMask* resInitMask,
                                                   typeChamferMask* resMask );



extern int _ComputePositiveChamferMap( void *inputBuf,
                                              bufferType typeIn,
                                              void *outputBuf,
                                              bufferType typeOut,
                                              int *theDim,
                                              double threshold,
                                              typeChamferMask *inputMask,
                                              int _divide_ );

extern int _NormalizePositiveChamferMap( void *inputBuf,
                                              bufferType typeIn,
                                              int *theDim,
                                              typeChamferMask *inputMask );



extern int Compute2DNormalizedChamfer3x3WithThreshold( void *inputBuf,
                                                  bufferType typeIn,
                                                  void *outputBuf,
                                                  bufferType typeOut,
                                                  int *theDim,
                                                  double threshold );

extern int Compute2DNormalizedChamfer5x5WithThreshold( void *inputBuf,
                                                  bufferType typeIn,
                                                  void *outputBuf,
                                                  bufferType typeOut,
                                                  int *theDim,
                                                  double threshold );

extern int Compute3DNormalizedChamfer3x3x3( void *inputBuf,
                                      bufferType typeIn,
                                      void *outputBuf,
                                      bufferType typeOut,
                                      int *theDim );

extern int Compute3DNormalizedChamfer5x5x5( void *inputBuf,
                                      bufferType typeIn,
                                      void *outputBuf,
                                      bufferType typeOut,
                                      int *theDim );


extern int _ComputePositiveNegativeChamferMap( void *inputBuf,
                                      bufferType typeIn,
                                      void *outputBuf,
                                      bufferType typeOut,
                                      int *theDim,
                                      double threshold,
                                      typeChamferMask *inputMask,
                                                    int _divide_ );



extern int skizWithChamferDistance( void *labelBuf,
                                    bufferType typeLabel,
                                    void *distBuf,
                                    bufferType typeDist,
                                    int *theDim,
                                    typeChamferMask *inputMask,
                                    int _divide_ );



extern int morphologicalDilationWithDistance( void *inputBuf,
                                                   void *resultBuf,
                                                   bufferType type,
                                                   int *theDim,
                                                   int radius, int dimension );

extern int morphologicalErosionWithDistance( void *inputBuf,
                                                  void *resultBuf,
                                                  bufferType type,
                                                  int *theDim,
                                                  int radius, int dimension );

extern int morphologicalClosingWithDistance( void *inputBuf,
                                                   void *resultBuf,
                                                   bufferType type,
                                                   int *theDim,
                                                   int radius, int dimension );

extern int morphologicalOpeningWithDistance( void *inputBuf,
                                                  void *resultBuf,
                                                  bufferType type,
                                                  int *theDim,
                                                  int radius, int dimension );


#ifdef __cplusplus
}
#endif

#endif

