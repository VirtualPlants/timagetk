/*************************************************************************
 * bal-image.h -
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




#ifndef BAL_IMAGE_H
#define BAL_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-stddef.h>
#include <bal-behavior.h>

#include <typedefs.h>
#include <linearFiltering-common.h>


typedef struct {
        size_t ncols;         /* Number of columns (X dimension) */
        size_t nrows;         /* Number of rows (Y dimension) */
        size_t nplanes;       /* Number of planes (Z dimension) */
        size_t vdim;          /* Vector size */
        bufferType type;
        void *data;        /* Generic pointer on image data buffer.
                              This pointer has to be casted in proper data type
                              depending on the type field */
        void ***array;     /* Generic 3D array pointing on each image element.
                              This pointer has to be casted in proper data type
                              depending on the type field */

        typeVoxelSize vx;          /* real voxel size in X dimension */
        typeVoxelSize vy;          /* real voxel size in Y dimension */
        typeVoxelSize vz;          /* real voxel size in Z dimension */

        char *name;
} bal_image;



extern int BAL_GetVerboseInBalImage(  );
extern void BAL_SetVerboseInBalImage( int v );
extern void BAL_IncrementVerboseInBalImage(  );
extern void BAL_DecrementVerboseInBalImage(  );





/* information/manipulation
 */

/* Fill the bal_image structure, but does not allocate the buffers
 * Has to be followed by BAL_AllocImage(), except in the case where
 * a (empty) image is used as a template
 */
extern int  BAL_InitImage    ( bal_image *image, char *name,
                               int dimx, int dimy, int dimz, int dimv, bufferType type );

extern void BAL_FreeImage    ( bal_image *image );

extern int BAL_AllocArrayImage( bal_image *image );

/* Allocate the buffers of the bal_image structure.
 * There are two buffers:
 * - data is a 1D buffer containing the data
 * - array is a multiple pointer, that points to convenient places
 *   of 'data' so ((cast)array)[z][y][x] gives access to the voxel value
 * values are initialized at 0.
 */
extern int  BAL_AllocImage   ( bal_image *image );

/* does nothing but a call to BAL_InitImage()
 * followed by a call to BAL_AllocImage()
 */
extern int BAL_InitAllocImage( bal_image *image, char *name,
                               int dimx, int dimy, int dimz, int dimv, bufferType type );

/* similar to BAL_InitAllocImage(), except that geometry informations
 * (image dimensions) are issued from a template image
 */
extern int BAL_InitAllocImageFromImage( bal_image *image, char *name,
                                 bal_image *from, bufferType type );

extern int BAL_CopyImage( bal_image *theIm, bal_image *resIm );

/* allows to fill the buffer image with one given value
 */
extern int BAL_FillImage( bal_image *theIm, float fval );
extern int BAL_NormaImage( bal_image *theIm, bal_image *resIm );
extern size_t  BAL_ImageDataSize( bal_image *image );



/* Get (interpolate) image value at the given position
   (in voxels not in millimeters)
*/
extern double BAL_GetXYZvalue( bal_image *image, double x, double y, double z );
extern double BAL_GetXYKvalue( bal_image *image, double x, double y, int k );



/* I/O operation
 */
extern void BAL_PrintImage( FILE *f, bal_image *image, char *s );
extern int BAL_ReadImage ( bal_image *image, char *name, int normalisation );
extern int BAL_WriteImage( bal_image *image, char *name );


/* filtering
 */
extern void BAL_SetFilterType( filterType filter );
extern int BAL_SmoothImage( bal_image *theIm,
                            bal_doublePoint *theSigma );
extern int BAL_SmoothImageIntoImage( bal_image *theIm, bal_image *resIm,
                                     bal_doublePoint *theSigma );
extern int BAL_2DDerivativesOfImage( bal_image *theIm, 
                                     bal_image *theDx, bal_image *theDy,
                                     bal_doublePoint *theSigma );
extern int BAL_3DDerivativesOfImage( bal_image *theIm, 
                                     bal_image *theDx, bal_image *theDy, bal_image *theDz,
                                     bal_doublePoint *theSigma );

#ifdef __cplusplus
}
#endif

#endif
