/*************************************************************************
 * vt_mip.h -
 *
 * $Id: vt_mip.h,v 1.5 2002/12/11 12:05:39 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _vt_mip_h_
#define _vt_mip_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_bytes.h>
#include <transfo.h>
#include <reech4x4.h>


typedef enum {
  _LINEAR_,
  _NEAREST_
} enumInterpolation;

typedef enum {
  _X_,
  _Y_,
  _Z_,
  _ALL_
} enumDirection;



extern int VT_FIP( vt_image *im, vt_image *xy, vt_image *xz, vt_image *zy );

extern int VT_MIP( vt_image *im, vt_image *xy, vt_image *xz, vt_image *zy );


extern int VT_MinMIP( vt_image *im, vt_image *xy, vt_image *xz, vt_image *zy );


extern int VT_AnimMIP( vt_image *theIm, 
		vt_image *resIm, 
		float *rotationDir,
		enumDirection projectionDirection,
		enumInterpolation typeInterpolation );

extern int VT_OneViewMIP( void *theBuf,       /* buffer 3D d'entree */
		   int *theDim,        
		   void *resSlice,     /* buffer 2D resultat */
		   int *resDim,        
		   void *auxBuf,       /* buffer 3D auxiliaire, peut etre NULL */
		   int *auxDim,
		   bufferType theType,    /* type des 3 buffers */
		   double rotationAngle,
		   float *rotationDir,
		   enumDirection projectionDirection,
		   enumInterpolation typeInterpolation );








#ifdef __cplusplus
}
#endif

#endif /* _vt_mip_h_ */
