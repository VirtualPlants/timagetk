/****************************************************
 * histogram.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Feb  6 12:06:18 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#ifndef _histogram_h_
#define _histogram_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>

#include <typedefs.h>



typedef enum {
  _MATLAB_,
  _SCILAB_,
  _IMAGE_,
  _STDERR_,
  _STDOUT_,
  _TXT_,
  _UNDEFINED_HISTOGRAM_FILE_
} enumHistogramFile;



/* the 'bin/index' value of a sample is given
   by (int)((sample-min)/binlength + 0.5)
*/
typedef struct {
  unionValues min;
  unionValues max;
  float binlength;
  bufferType typeIndex;
  int dim;
  void *index;
} typeHistogramHeader;




typedef struct {
  typeHistogramHeader xaxis;
  typeHistogramHeader yaxis;
  bufferType typeHisto;
  void* data;
} typeHistogram;





typedef struct {
  typeHistogram *data;
  int n;
} typeHistogramList;





extern void setVerboseInHistogram( int v );
extern void incrementVerboseInHistogram(  );
extern void decrementVerboseInHistogram(  );
 

/* histogram header construction tools
 */

extern bufferType typeHistogramIndex( float binlength, bufferType typeImage );

extern void initHistogramHeader( typeHistogramHeader *h ); 

extern void freeHistogramHeader( typeHistogramHeader *h );

extern int allocHistogramHeader( typeHistogramHeader *h, 
				 unionValues *min, unionValues *max,
				 float binlength, bufferType typeImage );



/* histogram construction tools
 */

extern void initHistogram( typeHistogram *h );

extern void freeHistogram( typeHistogram *h );

extern int allocHistogramData( typeHistogram *h, bufferType typeHisto );

extern void zeroHistogram( typeHistogram *h );

extern int allocHistogramFromHistogram( typeHistogram *h,
					typeHistogram *template,
					bufferType typeHisto );



/* 1D histogram construction tools
 */

extern int alloc1DHistogramFromType( typeHistogram *h, bufferType typeImage );

extern int alloc1DHistogramFromImage( typeHistogram *h, 
				      void *theIm,
				      bufferType theType,
				      void *theMask,
				      bufferType theMaskType,
				      double *matrix, 
				      int *theDim );

extern int alloc1DHistogramFromImageAndBinLength( typeHistogram *h, 
						  void *theIm,
						  bufferType theType,
						  void *theMask,
						  bufferType theMaskType,
						  double *matrix, 
						  int *theDim,
						  float binlength );

extern int alloc1DDifferenceHistogramFromJointHistogram( typeHistogram *hdiff,
							 typeHistogram *hjoint );



/* 2D histogram construction tools
 */

extern int alloc2DHistogramFromTypes( typeHistogram *h, bufferType typeImage1, bufferType typeImage2 );

extern int alloc2DHistogramFromHistogramList( typeHistogram *h, 
					     typeHistogramList *hl );

extern int alloc2DHistogramFromImages( typeHistogram *h, 
				       void *theIm1,
				       bufferType theType1,
				       void *theMask1,
				       bufferType theMaskType1,
				       double *mat1,
				       void *theIm2,
				       bufferType theType2,
				       void *theMask2,
				       bufferType theMaskType2,
				       double *mat2,
				       int *theDim );

extern int alloc2DHistogramFromImagesAndBinLength( typeHistogram *h, 
						   void *theIm1,
						   bufferType theType1,
						   void *theMask1,
						   bufferType theMaskType1,
						   double *mat1,
						   void *theIm2,
						   bufferType theType2,
						   void *theMask2,
						   bufferType theMaskType2,
						   double *mat2,
						   int *theDim,
						   float binlength1,
						   float binlength2 );



/* histogram list construction tools
 */

extern void initHistogramList( typeHistogramList *hl );
extern void freeHistogramList( typeHistogramList *hl );
extern int allocHistogramList( typeHistogramList *hl, int n );



/* 1D histogram filling tools
 */

extern int fill1DHistogramFromImage( typeHistogram *h, 
				     void *theIm,
				     bufferType theType,
				     void *theMask,
				     bufferType theMaskType,
				     double *matrix,
				     int *theDim );

extern int fill1DHistogramFromBuffer( typeHistogram *h, 
				      void *theIm,
				      bufferType theType,
				      size_t v );

extern int fill1DHistogramFromMaskedBuffer( typeHistogram *h, 
					    void *theIm,
					    bufferType theType,
					    void *theMaskIm,
					    bufferType theMaskType,
					    size_t v );

extern int fill1DHistogramFromResampledImage( typeHistogram *h, 
					      void *theIm,
					      bufferType theType,
					      double *mat, 
					      int *theDim );

extern int fill1DHistogramFromMaskedResampledImage( typeHistogram *h, 
						    void *theIm,
						    bufferType theType,
						    void *theMaskIm,
						    bufferType theMaskType,
						    double *mat,
						    int *theDim );

extern int fill1DDifferenceHistogram( typeHistogram *hdiff,
				      typeHistogram *hjoint );



/* 2D histogram filling tools
 */

extern int fill2DHistogramFromImages( typeHistogram *h, 
				      void *theIm1, 
				      bufferType theType1,
				      void *theMask1,
				      bufferType theMaskType1,
				      double *mat1,
				      void *theIm2,
				      bufferType theType2,
				      void *theMask2,
				      bufferType theMaskType2,
				      double *mat2,
				      int *theDim );

extern int fill2DHistogramFromBuffers( typeHistogram *h, 
				       void *theIm1,
				       bufferType theType1,
				       void *theIm2,
				       bufferType theType2,
				       size_t v );

extern int fill2DHistogramFromMaskedBuffers( typeHistogram *h, 
					     void *theIm1,
					     bufferType theType1,
					     void *theMaskIm1,
					     bufferType theMaskType1,
					     void *theIm2,
					     bufferType theType2,
					     void *theMaskIm2,
					     bufferType theMaskType2,
					     size_t v );

extern int fill2DHistogramFromResampledImages( typeHistogram *h, 
					       void *theIm1,
					       bufferType theType1,
					       double *mat1,
					       void *theIm2,
					       bufferType theType2,
					       double *mat2,
					       int *theDim );

extern int fill2DHistogramFromMaskedResampledImages( typeHistogram *h, 
						     void *theIm1,
						     bufferType theType1,
						     void *theMaskIm1,
						     bufferType theMaskType1,
						     double *mat1,
						     void *theIm2,
						     bufferType theType2,
						     void *theMaskIm2,
						     bufferType theMaskType2,
						     double *mat2,
						     int *theDim );



/* histogram tools
 */

extern float maxhistogram( typeHistogram *h );

extern void fprintfHistogramHeaderInfo( FILE *f, typeHistogram *h, char *desc );



/* 1D histogram tools
 */

extern int minMeanMaxStddev1DHistogramWithRange( typeHistogram *h,
						 double fbound,
						 double lbound,
						 double *min,
						 double *moy,
						 double *max,
						 double *ect );



/* histogram transformation tools
 */

extern int pdfHistogram( typeHistogram *pdf, typeHistogram *h );



/* 1D histogram transformation tools
 */

extern int cumulative1DHistogram( typeHistogram *cumul, typeHistogram *h );



/* 1D histogram I/O procedures
 */

extern int fprintf1DHistogramMatlab( FILE *f, int fd, 
				     typeHistogram *h, char *s );
extern int fprintf1DHistogramScilab( FILE *f, int fd, 
				     typeHistogram *h, char *s );
extern int fprintf2DHistogramScilab( FILE *f, int fd, 
				     typeHistogram *h, char *s );

extern void writeHistogramXxxlab( char *name, typeHistogram *h, char *s, 
				  enumHistogramFile xxxlab );
extern void writeHistogramMatlab( char *name, typeHistogram *h, char *s );
extern void writeHistogramScilab( char *name, typeHistogram *h, char *s );

extern void write1DHistogramTxt( char *name, typeHistogram *h );

extern void fprintf1DHistogram( FILE *theFile, typeHistogram *h );



/* buffer/image statistics tools
 */

extern int minMeanMaxStddevValues( void *theIm, bufferType theType, 
				   void *theMask, bufferType theMaskType, 
				   int *theDim, 
				   double *theMin, double *theMean,  
				   double *theMax, double *theStandardDeviation );


#ifdef __cplusplus
}
#endif

#endif
