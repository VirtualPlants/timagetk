/*************************************************************************
 * bal-stddef.h -
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

#ifndef BAL_STDDEF_H
#define BAL_STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif





#include <stdlib.h>
#include <stdio.h>
#include <bal-behavior.h>

/****************************************
 * 
 * miscellaneous defines
 *
 ****************************************/

#define EPSILON 0.0000001

#ifdef max
#undef max
#endif
#define max(a,b) ((a)>(b) ? (a) : (b))

#define RETURNED_VALUE_ON_ERROR -2



/****************************************
 * 
 * Similarity measures between blocks
 * NB: only some of them are implemented
 *
 ****************************************/

typedef enum {
  _SAD_,
  _SSD_,
  _SQUARED_CC_,
  _SQUARED_EXTCC_
} enumTypeSimilarity;



/****************************************
 * 
 * Transformation types
 * NB: only some of them are implemented
 *
 ****************************************/

typedef enum {
  UNDEF_TRANSFORMATION,
  TRANSLATION_2D,
  TRANSLATION_3D,
  TRANSLATION_SCALING_2D,
  TRANSLATION_SCALING_3D,
  RIGID_2D,
  RIGID_3D,
  SIMILITUDE_2D,
  SIMILITUDE_3D,
  AFFINE_2D,
  AFFINE_3D,
  VECTORFIELD_2D,
  VECTORFIELD_3D,
  SPLINE
} enumTypeTransfo;

typedef enum {
  UNDEF_UNIT,
  VOXEL_UNIT,
  REAL_UNIT
} enumUnitTransfo;



/****************************************
 * 
 * miscellaneous structures
 *
 ****************************************/

typedef unsigned char  bal_flag;

typedef struct {
  int x;
  int y;
  int z;
} bal_integerPoint;

typedef struct {
  size_t x;
  size_t y;
  size_t z;
} bal_sizePoint;

typedef struct {
  float x;
  float y;
  float z;
} bal_floatPoint;

typedef struct {
  double x;
  double y;
  double z;
} bal_doublePoint;

typedef struct {
  typeField x;
  typeField y;
  typeField z;
} bal_typeFieldPoint;









/****************************************
 * selection criterium for blocks
   
   Thresholds are considered if they are in the range of image encoding.
   Eg: for images encoded on unsigned char, a low threshold strictly below 0
       is not considered, and a high threshold strictly over 255 is not 
       considered.

   If thresholds are to be considered, only points strictly between the two 
   thresholds are taken into account: the block is valid if the fraction
   of removed voxels (wrt to the number of voxels in the block) is lower than 
   'max_removed_fraction'
 ****************************************/

typedef struct {
  int low_threshold;
  int high_threshold;
  double max_removed_fraction;
} bal_intensitySelection;







/****************************************
 * 
 * these structures allows to defines parameter values for both 
 * the highest and the lowest levels of the pyramid.
 * Recall the level #0 is the original image.
 * Values for the in-between levels will be linearly interpolated.
 *
 ****************************************/

typedef struct {
  double lowest;
  double highest;
} bal_pyramidDouble;

typedef struct {
  int lowest;
  int highest;
} bal_pyramidInt;

typedef struct {
  bal_doublePoint lowest;
  bal_doublePoint highest;
} bal_pyramidDoublePoint;




extern void BAL_PrintTypeSimilarity( FILE *f, enumTypeSimilarity m, char *s );
extern void BAL_PrintTypeTransformation( FILE *f, enumTypeTransfo m, char *s );
extern void BAL_PrintIntegerPoint( FILE *f, bal_integerPoint *p, char *s );
extern void BAL_PrintDoublePoint( FILE *f, bal_doublePoint *p, char *s );
extern void BAL_PrintTypeFieldPoint( FILE *f, bal_typeFieldPoint *p, char *s );
extern void BAL_PrintIntensitySelection( FILE *f, bal_intensitySelection *p, char *s );





#ifdef __cplusplus
}
#endif

#endif
