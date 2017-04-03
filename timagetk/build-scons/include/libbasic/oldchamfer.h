/*************************************************************************
 * chamfer.h - computation of chamfer distances
 *
 * $Id: chamfer.h,v 1.5 2001/03/19 17:11:47 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Feb, 8 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 *
 */

#ifndef _chamfer_h_
#define _chamfer_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h> /* int abs(int j); */
#include <math.h>

#include <stdio.h>


#include <typedefs.h>
#include <convert.h>



typedef enum {
  _BORGEFORS3x3x3_ = 1,
  _CHAMFER3x3x3_ = 2,
  _MALANDAIN3x3x3_ = 2,
  _DISTANCE04_ = 3,
  _DISTANCE08_ = 4,
  _DISTANCE06_ = 5,
  _DISTANCE18_ = 6,
  _DISTANCE26_ = 7,
  _CHAMFER5x5x5_ = 8
} enumDistance;



/* Compute a 3x3 chamfer 2D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * Increments are (16, 21) and the result
 * image is obtained by shifting bits (to divide by 16).
 * The forward propagation mask is then
 *
 *     21 16 21
 *     16  X  .
 *      .  .  .
 *
 * For a 3D image, the distance is computed slice
 * by slice independantly.
 * The image is binarized with a threshold = 1.0.
 * 
 * 1. the coefficients are set
 * 2. call of a static function (_ComputeChamferDistance())
 *    where
 *    a. the distance map is initialized
 *       by InitializeChamferMap()
 *       if possible (if its type is (un)signed short int)
 *       we use the output buffer as auxiliary distance
 *       map (a 2 bytes buffer is required).
 *    b. according to the parameters one of the following
 *       is called
 *       - Compute2DChamfer3InUShortImage()
 *       - Compute3DChamfer3InUShortImage()
 *       - Compute2DChamfer5InUShortImage()
 *       - Compute3DChamfer5InUShortImage()
 *    c. If necessary, the distance map is updated
 *       (division  by the smallest coefficient)
 *    d. If necessary, the auxiliary distance map
 *       is converted into the type of output buffer.
 *  
 */
extern int Compute2DChamfer3x3( void *inputBuf,     /* input buffer */
				bufferType typeIn,  /* type of the input buffer */
				void *outputBuf,    /* output buffer */
				bufferType typeOut, /* type of the output buffer */
				const int *bufferDims     /* buffers' dimensions */
				);
  
/* Compute a 3x3x3 chamfer 3D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * Increments are (16, 21, 26) and the result
 * image is obtained by shifting bits (to divide by 16).
 * The forward propagation mask is then
 *
 *     26 21 26      21 16 21
 *     21 16 21      16  X  .
 *     26 21 26       .  .  .
 *
 *       z-1             z
 *
 * The image is binarized with a threshold = 1.0.
 * 
 * 1. the coefficients are set
 * 2. call of a static function (_ComputeChamferDistance())
 *    where
 *    a. the distance map is initialized
 *       by InitializeChamferMap()
 *       if possible (if its type is (un)signed short int)
 *       we use the output buffer as auxiliary distance
 *       map (a 2 bytes buffer is required).
 *    b. according to the parameters one of the following
 *       is called
 *       - Compute2DChamfer3InUShortImage()
 *       - Compute3DChamfer3InUShortImage()
 *       - Compute2DChamfer5InUShortImage()
 *       - Compute3DChamfer5InUShortImage()
 *    c. If necessary, the distance map is updated
 *       (division  by the smallest coefficient)
 *    d. If necessary, the auxiliary distance map
 *       is converted into the type of output buffer.
 *  
 *  
 */
extern int Compute3DChamfer3x3x3( void *inputBuf,     /* input buffer */
				  bufferType typeIn,  /* type of the input buffer */
				  void *outputBuf,    /* output buffer */
				  bufferType typeOut, /* type of the output buffer */
				  const int *bufferDims     /* buffers' dimensions */
				  );
  

/* Compute a 5x5 chamfer 2D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * Increments are (13, 18, 29).
 * The forward propagation mask is then
 *
 *     29 .. 29   
 *  29 18 13 18 29
 *  .. 13  X      
 *                
 * For a 3D image, the distance is computed slice
 * by slice independantly.
 * The image is binarized with a threshold = 1.0.
 * 
 * 1. the coefficients are set
 * 2. call of a static function (_ComputeChamferDistance())
 *    where
 *    a. the distance map is initialized
 *       by InitializeChamferMap()
 *       if possible (if its type is (un)signed short int)
 *       we use the output buffer as auxiliary distance
 *       map (a 2 bytes buffer is required).
 *    b. according to the parameters one of the following
 *       is called
 *       - Compute2DChamfer3InUShortImage()
 *       - Compute3DChamfer3InUShortImage()
 *       - Compute2DChamfer5InUShortImage()
 *       - Compute3DChamfer5InUShortImage()
 *    c. If necessary, the distance map is updated
 *       (division  by the smallest coefficient)
 *    d. If necessary, the auxiliary distance map
 *       is converted into the type of output buffer.
 *  
 */
extern int Compute2DChamfer5x5( void *inputBuf,     /* input buffer */
				bufferType typeIn,  /* type of the input buffer */
				void *outputBuf,    /* output buffer */
				bufferType typeOut, /* type of the output buffer */
				const int *bufferDims     /* buffers' dimensions */
				);
  
/* Compute a 5x5 chamfer 2D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * The forward propagation mask is then
 *
 *  . c . c .   f e c e f   . f . f .
 *  c b a b c   e d b d e   f e c e f
 *  . a X       c b a b c   . c . c .
 *              e d b d e   f e c e f
 *              f e c e f   . f . f .
 *
 *     z=0         z=1         z=2
 *
 *     29 .. 29      39 32 29 32 39   .. 39 .. 39 ..
 *  29 18 13 18 29   32 23 18 23 32   39 32 29 32 39
 *  .. 13 XX         29 18 13 18 29   .. 29 .. 29 ..
 *                   32 23 18 23 32   39 32 29 32 39
 *                   39 32 29 32 39   .. 39 .. 39 ..
 *                
 * For a 3D image, the distance is computed slice
 * by slice independantly.
 * The image is binarized with a threshold = 1.0.
 * 
 * 1. the coefficients are set
 * 2. call of a static function (_ComputeChamferDistance())
 *    where
 *    a. the distance map is initialized
 *       by InitializeChamferMap()
 *       if possible (if its type is (un)signed short int)
 *       we use the output buffer as auxiliary distance
 *       map (a 2 bytes buffer is required).
 *    b. according to the parameters one of the following
 *       is called
 *       - Compute2DChamfer3InUShortImage()
 *       - Compute3DChamfer3InUShortImage()
 *       - Compute2DChamfer5InUShortImage()
 *       - Compute3DChamfer5InUShortImage()
 *    c. If necessary, the distance map is updated
 *       (division  by the smallest coefficient)
 *    d. If necessary, the auxiliary distance map
 *       is converted into the type of output buffer.
 *  
 */
extern int Compute3DChamfer5x5x5( void *inputBuf,     /* input buffer */
				  bufferType typeIn,  /* type of the input buffer */
				  void *outputBuf,    /* output buffer */
				  bufferType typeOut, /* type of the output buffer */
				  const int *bufferDims     /* buffers' dimensions */
				  );
    

  





/* Compute a 3x3 chamfer 2D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * idem Compute2DChamfer3x3() but the threshold to binarize
 * the input buffer is provided by the user.
 */

extern int Compute2DChamfer3x3WithThreshold( void *inputBuf,     /* input buffer */
				bufferType typeIn,  /* type of the input buffer */
				void *outputBuf,    /* output buffer */
				bufferType typeOut, /* type of the output buffer */
				const int *bufferDims,    /* buffers' dimensions */
					     float threshold
				);
  
/* Compute a 3x3x3 chamfer 3D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * idem Compute3DChamfer3x3x3() but the threshold to binarize
 * the input buffer is provided by the user.
 */

extern int Compute3DChamfer3x3x3WithThreshold( void *inputBuf,     /* input buffer */
				  bufferType typeIn,  /* type of the input buffer */
				  void *outputBuf,    /* output buffer */
				  bufferType typeOut, /* type of the output buffer */
				  const int *bufferDims,    /* buffers' dimensions */
					       float threshold
				  );
  
/* Compute a 5x5 chamfer 2D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * idem Compute2DChamfer5x5() but the threshold is binarized
 * the input buffer is provided by the user.
 */

extern int Compute2DChamfer5x5WithThreshold( void *inputBuf,     /* input buffer */
				bufferType typeIn,  /* type of the input buffer */
				void *outputBuf,    /* output buffer */
				bufferType typeOut, /* type of the output buffer */
				const int *bufferDims,    /* buffers' dimensions */
					     float threshold
				);
  
/* Compute a 5x5x5 chamfer 3D distance
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * idem Compute3DChamfer5x5x5() but the threshold is binarized
 * the input buffer is provided by the user.
 */

extern int Compute3DChamfer5x5x5WithThreshold( void *inputBuf,     /* input buffer */
				  bufferType typeIn,  /* type of the input buffer */
				  void *outputBuf,    /* output buffer */
				  bufferType typeOut, /* type of the output buffer */
				  const int *bufferDims,    /* buffers' dimensions */
					       float threshold
				  );
    

  

/* Initialization of a distance map
 *
 * RETURN:
 *   1 if success
 *  -1 if failure
 *
 * DESCRIPTION:
 * The input buffer is binarized with the threshold.
 * Points below it (I < threshold) are set to 
 * 65535 (infinity for a unsigned two bytes buffer)
 * and others (I >= threshold) to 0.
 */
extern int InitializeChamferMap( void *inputBuf,
				 bufferType typeIn,
				 unsigned short int *outputBuf,
				 const int *theDim,
				 float threshold );



extern void Compute2DChamfer3InUShortImage( unsigned short int *inputBuf,
					    const int *theDim,
					    int *theCoeff );

extern void Compute3DChamfer3InUShortImage( unsigned short int *inputBuf,
					    const int *theDim,
					    int *theCoeff );

extern void Compute2DChamfer5InUShortImage( unsigned short int *inputBuf,
					    const int *theDim,
					    int *theCoeff );

extern void Compute3DChamfer5InUShortImage( unsigned short int *inputBuf,
					    const int *theDim,
					    int *theCoeff );

/* Set verbose mode
   
   In case of failure, some information about the cause
   is written on stderr
*/
extern void Chamfer_verbose();
extern void Chamfer_noverbose();



extern void SetChamferCoefficientsToBorgefors3x3x3();
extern void SetChamferCoefficientsToMalandain3x3x3();
extern void SetChamferCoefficientsToDistance04();
extern void SetChamferCoefficientsToDistance08();
extern void SetChamferCoefficientsToDistance06();
extern void SetChamferCoefficientsToDistance18();
extern void SetChamferCoefficientsToDistance26();
extern void SetChamferCoefficientsToChamfer5x5x5();



#ifdef __cplusplus
}
#endif

#endif
