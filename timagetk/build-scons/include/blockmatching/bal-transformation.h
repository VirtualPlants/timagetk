/*************************************************************************
 * bal-transformation.h -
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



#ifndef BAL_TRANSFORMATION_H
#define BAL_TRANSFORMATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string-tools.h>

#include <bal-stddef.h>

#include <bal-matrix.h>
#include <bal-image.h>



  

typedef struct {

  enumTypeTransfo type;
  _MATRIX mat;
  enumUnitTransfo transformation_unit;
  bal_image vx;
  bal_image vy;
  bal_image vz;

  /* for transformation averaging
   */
  float weight;
  float error;

} bal_transformation;



/* list of transformation
   - when allocated, data is initialized and
    pointer points towards each data 
   - when reading, only the first data are filled
*/
typedef struct {
  bal_transformation *data;      /* allocated transformations */
  bal_transformation **pointer;  /* allocated pointers */
  int n_selected_trsfs;
  int n_trsfs;           /* read transformations */
  int n_allocated_trsfs; /* allocated transformations */
} bal_transformationList;



/* array of transformation
   - when allocated, data is initialized and the array allows an [j][i]
   access; however, pointers are initialized to NULL
   - when reading, if successful reading, the pointer points toward the corresponding data
 */
typedef struct {
  bal_transformation *data;
  bal_transformation **array;
  bal_transformation **pointer;
  bal_transformation ***pointer_array;
  int n_allocated_trsfs;
  int ncols;
  int nrows;
} bal_transformationArray;



extern int BAL_GetVerboseInBalTransformation( );
extern void BAL_SetVerboseInBalTransformation( int v );
extern void BAL_IncrementVerboseInBalTransformation( );
extern void BAL_DecrementVerboseInBalTransformation( );



/*--------------------------------------------------
 *
 * TRANSFORMATION MANAGEMENT
 *
 --------------------------------------------------*/

extern void BAL_InitTransformation( bal_transformation *t );
extern int BAL_AllocTransformation( bal_transformation *t, enumTypeTransfo type, bal_image *ref );
extern void BAL_FreeTransformation( bal_transformation *t );
extern int BAL_CopyTransformation( bal_transformation *theTrsf,
                                   bal_transformation *resTrsf );
extern void BAL_SetTransformationToIdentity( bal_transformation *t );



/*--------------------------------------------------
 *
 * TRANSFORMATION LIST MANAGEMENT
 *
 --------------------------------------------------*/

extern void BAL_InitTransformationList( bal_transformationList *l );
extern int BAL_AllocTransformationList( bal_transformationList *l, 
                                        int n_transformations );
extern int BAL_FullAllocTransformationList( bal_transformationList *l, 
                                            int n_transformations,
                                            enumTypeTransfo type, bal_image *ref );
extern void BAL_FreeTransformationList( bal_transformationList *l );
extern int BAL_CopyTransformationList( bal_transformationList *theTrsfs,
                                bal_transformationList *resTrsfs );


/*--------------------------------------------------
 *
 * TRANSFORMATION ARRAY MANAGEMENT
 *
 --------------------------------------------------*/

extern void BAL_InitTransformationArray( bal_transformationArray *l );
extern int BAL_AllocTransformationArray( bal_transformationArray *l, 
                                        int ncols, int nrows );
extern void BAL_FreeTransformationArray( bal_transformationArray *l );



/*--------------------------------------------------
 *
 * TRANSFORMATION I/O
 *
 --------------------------------------------------*/

extern void BAL_PrintTransformationType( FILE *f, enumTypeTransfo t );
extern void BAL_PrintTransformation( FILE *f, bal_transformation *t, char *s );
extern int BAL_ReadTransformation( bal_transformation *theTrsf, char *name );
extern int BAL_WriteTransformation( bal_transformation *theTrsf, char *name );



/*--------------------------------------------------
 *
 * TRANSFORMATION LIST I/O
 *
 --------------------------------------------------*/

extern void BAL_PrintTransformationList( FILE *f, bal_transformationList *l, char *s );
extern int BAL_ReadTransformationList( bal_transformationList *theList, 
                                       stringList *trsfFileList );
extern int BAL_WriteTransformationList( bal_transformationList *theList, 
                                 stringList *trsfFileList );

/*--------------------------------------------------
 *
 * TRANSFORMATION ARRAY I/O
 *
 --------------------------------------------------*/

int BAL_ReadTransformationArray( bal_transformationArray *theArray, 
                                 stringArray *trsfFileArray );

/*--------------------------------------------------
 *
 * TRANSFORMATION TESTS
 *
 --------------------------------------------------*/

extern int BAL_DoesTransformationExist( bal_transformation *t );
extern int BAL_IsTransformationLinear( bal_transformation *t );
extern int BAL_IsTransformationVectorField( bal_transformation *t );
extern int BAL_IsTransformationTypeLinear( enumTypeTransfo type );
extern int BAL_IsTransformationTypeVectorField( enumTypeTransfo type );


/*--------------------------------------------------
 *
 * TRANSFORMATION ARRAY TESTS
 *
 --------------------------------------------------*/

extern void BAL_TestTransformationArray( bal_transformationArray *theArray, 
                                         stringArray *trsfFileArray );


/*--------------------------------------------------
 *
 * MISC
 *
 --------------------------------------------------*/

extern int BAL_RotationAngle( bal_transformation *theTrsf, double *radian_angle );

#ifdef __cplusplus
}
#endif

#endif
