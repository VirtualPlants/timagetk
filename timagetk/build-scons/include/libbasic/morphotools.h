/*************************************************************************
 * morphotools.h -
 *
 * $Id: morphotools.h,v 1.3 2006/04/14 08:38:55 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Mar 14 08:28:36 MET 2001
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#ifndef _morphotools_h_
#define _morphotools_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>

#include <string.h>


#include <typedefs.h>
#include <morpho.h>


typedef enum enumMorphologicalOperation {
    MORPHOLOGICAL_DILATION = 1,
    MORPHOLOGICAL_EROSION = 2,
    MORPHOLOGICAL_CLOSING = 3,
    MORPHOLOGICAL_OPENING  = 4,
    MORPHOLOGICAL_HATCLOSING  = 5,
    MORPHOLOGICAL_HATOPENING  = 6,
    MORPHOLOGICAL_CONTRAST = 7,
    MORPHOLOGICAL_GRADIENT = 8
} enumMorphologicalOperation;


typedef struct {
  int x;
  int y;
  int z;
  int o;
} typeMorphoToolsPoint;

typedef struct {
  int nb;
  typeMorphoToolsPoint *list;
} typeMorphoToolsList;


typedef struct {
  /* morphologie de base:
     - nombre d'iterations
     - connexite 
       celle-ci est ramenee au cas 2D si dimension = 2
  */
  int nbIterations;
  int connectivity;
  /* element structurant donne par l'utilisateur
   */
  typeMorphoToolsList userDefinedSE;
  int radius;
  int dimension;
} typeStructuringElement;


extern void printPseudoSphereDecomposition( FILE *f, int radius, int dim, char *str );


extern int morphologicalDilation( void *inputBuf,
				  void *resultBuf,
				  bufferType type,
				  int *theDim,
				  typeStructuringElement *theSE );

extern int morphologicalErosion( void *inputBuf,
				 void *resultBuf,
				 bufferType type,
				 int *theDim,
				 typeStructuringElement *theSE );



extern void useBinaryMorphologicalOperations();
extern void useGreyLevelMorphologicalOperations();


extern void initStructuringElement ( typeStructuringElement *SE );
extern void freeStructuringElement ( typeStructuringElement *SE );
extern void printStructuringElement ( FILE *f, typeStructuringElement *SE, char *str );

extern int buildStructuringElementAs3DSphere( typeStructuringElement *SE, int radius );
extern int buildStructuringElementFromImage( unsigned char *theBuf,
					     int *theDim,
					     int *theCenter,
					     typeStructuringElement *SE );
extern int readStructuringElement( char *fileName,
			    typeStructuringElement *SE );

extern void MorphoTools_verbose();
extern void MorphoTools_noverbose();

#ifdef __cplusplus
}
#endif

#endif
