/*************************************************************************
 * copyTrsf.c -
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
#include "bal-copyTrsf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <bal-stddef.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>

static char *program = "copyTrsf";

int copyTrsf(
	     char* thetrsf_name,
	     char* restrsf_name,
	     enumUnitTransfo thetrsf_unit,
	     enumUnitTransfo restrsf_unit,
	     char *template_image_name,
	     char *floating_image_name,
	     bal_integerPoint dim,
	     bal_doublePoint voxel,
	     enumTypeTransfo transformation_type,
	     int isDebug,
	     int isVerbose
	     )
{
  bal_transformation theTransformation;
  bal_transformation resTransformation;
  bal_transformation *resTrsf = (bal_transformation *)NULL;
  bal_image theTemplate;
  bal_image theFloating;
  bal_image *template = (bal_image *)NULL;
  bal_image *floating = (bal_image *)NULL;

  /* initialisation
   */
  BAL_InitTransformation( &theTransformation );
  BAL_InitTransformation( &resTransformation );
  BAL_InitImage( &theTemplate, NULL, 0, 0, 0, 0, UCHAR );
  BAL_InitImage( &theFloating, NULL, 0, 0, 0, 0, UCHAR );
 




  /***************************************************
   *
   * reading input transformation
   *
   ***************************************************/

  if ( thetrsf_name == NULL ) {
    if ( isVerbose )
      fprintf( stderr, "%s: no input transformation\n", program );
    return -1;
  }

  if ( BAL_ReadTransformation( &theTransformation, thetrsf_name ) != 1 ) {
    if ( isVerbose )
      fprintf( stderr, "%s: unable to read '%s'\n", program, thetrsf_name );
    return -1;
  }
  theTransformation.transformation_unit = thetrsf_unit;

  if ( isDebug )
    BAL_PrintTransformation( stderr, &theTransformation, "read transformation" );


 


  /***************************************************
   *
   * reading template (if required)
   * - the reference/template is required to define the geometry
   *   of a vector field and/or to convert from voxel to real
   *   (or the inverse)
   * - the floating is required to convert from voxel to real
   *   (or the inverse)
   *
   ***************************************************/

  if ( ( BAL_IsTransformationTypeLinear( theTransformation.type ) 
	 && BAL_IsTransformationTypeVectorField( transformation_type ) )
       || ( thetrsf_unit != restrsf_unit ) ) {

    /* initializing reference image, if any
     */
    if ( template_image_name != NULL ) {
      if ( BAL_ReadImage( &theTemplate, template_image_name, 1 ) != 1 ) {
	if ( isVerbose )
	  fprintf( stderr, "%s: unable to read template/reference '%s'\n", program, template_image_name );
	BAL_FreeTransformation( &theTransformation );
	return -1;
      }
      template = &theTemplate;
    }
    
    /* initializing result image
       - with parameters, if any
    */
    else if ( dim.x > 0 && dim.y > 0 ) {
      if ( dim.z > 0 ) {
	if ( BAL_InitImage( &theTemplate, (char*)NULL, dim.x, dim.y, dim.z, 1, UCHAR ) != 1 ) {
	  if ( isVerbose )
	    fprintf( stderr, "%s: unable to initialize template/reference image\n", program );
	  BAL_FreeTransformation( &theTransformation );
	  return -1;
	}
      }
      else {
	if ( BAL_InitImage( &theTemplate, (char*)NULL, dim.x, dim.y, 1, 1, UCHAR ) != 1 ) {
	  if ( isVerbose )
	    fprintf( stderr, "%s: unable to initialize template/reference image (dimz=1) \n", program );
	  BAL_FreeTransformation( &theTransformation );
	  return -1;
	}
      }
      if ( voxel.x > 0.0 ) theTemplate.vx = voxel.x;
      if ( voxel.y > 0.0 ) theTemplate.vy = voxel.y;
      if ( voxel.z > 0.0 ) theTemplate.vz = voxel.z;
      template = &theTemplate;
    }
    else {
      if ( isVerbose )
	fprintf( stderr, "%s: unable to initialize template/reference image\n", program );
      BAL_FreeTransformation( &theTransformation );
      return -1;
    }

  }
  
  if ( thetrsf_unit != restrsf_unit ) {

    /* initializing floating image, if any
     */
    if ( floating_image_name != NULL ) {
      if ( BAL_ReadImage( &theFloating, floating_image_name, 1 ) != 1 ) {
	if ( isVerbose )
	  fprintf( stderr, "%s: unable to read floating '%s'\n", program, floating_image_name );
	BAL_FreeImage( &theTemplate );
	BAL_FreeTransformation( &theTransformation );
	return -1;
      }
      floating = &theFloating;
    }
    else {

      /* choice:
	 1. if no floating image, use the reference one
	 2. force to give a floating image
      */
      if ( isVerbose ) {
	fprintf( stderr, "WARNING, use the template/reference image as floating image for unit conversion\n" );
      }
      floating = &theTemplate;

      if ( 0 ) {
	if ( isVerbose )
	  fprintf( stderr, "%s: unable to initialize floating image\n", program );
	BAL_FreeImage( &theTemplate );
	BAL_FreeTransformation( &theTransformation );
	return -1;
      }

    }

  }
	 

  


  /***************************************************
   *
   * copy transformation
   *
   ***************************************************/

  switch ( theTransformation.type ) {
  default :
    BAL_FreeImage( &theFloating );
    BAL_FreeImage( &theTemplate );
    BAL_FreeTransformation( &theTransformation );  
    if ( isVerbose ) {
      fprintf( stderr, "%s: unknow type'", program );
      BAL_PrintTransformationType( stderr, theTransformation.type );
      fprintf( stderr, "' for transformation '%s'\n", thetrsf_name );
    }
    return -1;

  case TRANSLATION_2D :
  case TRANSLATION_3D :
  case TRANSLATION_SCALING_2D :
  case TRANSLATION_SCALING_3D :
  case RIGID_2D :
  case RIGID_3D :
  case SIMILITUDE_2D :
  case SIMILITUDE_3D :
  case AFFINE_2D :
  case AFFINE_3D :

    switch( transformation_type ) {
    default :
      BAL_FreeImage( &theFloating );
      BAL_FreeImage( &theTemplate );
      BAL_FreeTransformation( &theTransformation );  
      if ( isVerbose ) {
	fprintf( stderr, "%s: unknow type'", program );
	BAL_PrintTransformationType( stderr, transformation_type );
	fprintf( stderr, "' for result transformation\n" );
      }
      return -1;

    case TRANSLATION_2D :
    case TRANSLATION_3D :
    case TRANSLATION_SCALING_2D :
    case TRANSLATION_SCALING_3D :
    case RIGID_2D :
    case RIGID_3D :
    case SIMILITUDE_2D :
    case SIMILITUDE_3D :
    case AFFINE_2D :
    case AFFINE_3D :
      resTrsf = &theTransformation;
      break;
      
    case VECTORFIELD_2D :
    case VECTORFIELD_3D :
      if ( BAL_AllocTransformation( &resTransformation, transformation_type, template ) != 1 ) {
	if ( isVerbose )
	  fprintf( stderr, "%s: unable to allocate result transformation\n", program );
	BAL_FreeImage( &theFloating );
	BAL_FreeImage( &theTemplate );
	BAL_FreeTransformation( &theTransformation );
	return -1;
      }

      if ( BAL_CopyTransformation( &theTransformation, &resTransformation ) != 1 ) {
	if ( isVerbose )
	  fprintf( stderr, "%s: unable to copy transformation\n", program );
	BAL_FreeImage( &theFloating );
	BAL_FreeImage( &theTemplate );
	BAL_FreeTransformation( &theTransformation );
	return -1;
      }
      if ( isDebug ) BAL_PrintTransformation( stderr, &resTransformation, "after copy" );
      resTrsf = &resTransformation;
      break;

    }
    break;
    
  case VECTORFIELD_2D :
  case VECTORFIELD_3D :

    switch( transformation_type ) {
    default :
      BAL_FreeImage( &theFloating );
      BAL_FreeImage( &theTemplate );
      BAL_FreeTransformation( &theTransformation );  
      if ( isVerbose ) {
	fprintf( stderr, "%s: unknow type'", program );
	BAL_PrintTransformationType( stderr, transformation_type );
	fprintf( stderr, "' for result transformation\n" );
      }
      return -1;

    case TRANSLATION_2D :
    case TRANSLATION_3D :
    case TRANSLATION_SCALING_2D :
    case TRANSLATION_SCALING_3D :
    case RIGID_2D :
    case RIGID_3D :
    case SIMILITUDE_2D :
    case SIMILITUDE_3D :
    case AFFINE_2D :
    case AFFINE_3D :
      BAL_FreeImage( &theFloating );
      BAL_FreeImage( &theTemplate );
      BAL_FreeTransformation( &theTransformation );  
      if ( isVerbose ) 
	fprintf( stderr, "%s: unable to convert non-linear transformation into linear\n", program );
      return -1;
      
    case VECTORFIELD_2D :
    case VECTORFIELD_3D :
      resTrsf = &theTransformation;
      break;
    }

  }



  /***************************************************
   *
   * conversion
   *
   ***************************************************/
  
  if ( resTrsf->transformation_unit != thetrsf_unit ) {
    BAL_FreeTransformation( &resTransformation );
    BAL_FreeImage( &theFloating );
    BAL_FreeImage( &theTemplate );
    BAL_FreeTransformation( &theTransformation );  
    if ( isVerbose ) 
      fprintf( stderr, "%s: weird unwanted conversion\n", program );
    return -1;
  }


  switch( resTrsf->transformation_unit ) {
  default :
    BAL_FreeTransformation( &resTransformation );
    BAL_FreeImage( &theFloating );
    BAL_FreeImage( &theTemplate );
    BAL_FreeTransformation( &theTransformation );  
    if ( isVerbose ) 
      fprintf( stderr, "%s: unknown unit for input transformation\n", program );
    return -1;

  case VOXEL_UNIT :

    switch( restrsf_unit ) {
    default :
      BAL_FreeTransformation( &resTransformation );
      BAL_FreeImage( &theFloating );
      BAL_FreeImage( &theTemplate );
      BAL_FreeTransformation( &theTransformation );  
      if ( isVerbose ) 
	fprintf( stderr, "%s: unknown unit for output transformation\n", program );
      return -1;

    case VOXEL_UNIT :
      break;

    case REAL_UNIT :
      if ( isDebug )
	fprintf( stderr, "%s: converting from VOXEL to REAL\n", program );
      if ( BAL_ChangeTransformationToRealUnit( floating, template, resTrsf, resTrsf ) != 1 ) {
	BAL_FreeTransformation( &resTransformation );
	BAL_FreeImage( &theFloating );
	BAL_FreeImage( &theTemplate );
	BAL_FreeTransformation( &theTransformation );  
	if ( isVerbose ) 
	  fprintf( stderr, "%s: unable to convert from VOXEL to REAL\n", program );
	return -1;
      }
      break;
    }
    
    break;

  case REAL_UNIT :

    switch( restrsf_unit ) {
    default :
      BAL_FreeTransformation( &resTransformation );
      BAL_FreeImage( &theFloating );
      BAL_FreeImage( &theTemplate );
      BAL_FreeTransformation( &theTransformation );  
      if ( isVerbose ) 
	fprintf( stderr, "%s: unknown unit for output transformation\n", program );
      return -1;

    case VOXEL_UNIT :
      if ( isDebug )
	fprintf( stderr, "%s: converting from REAL to VOXEL\n", program );
      if ( BAL_ChangeTransformationToVoxelUnit( floating, template, resTrsf, resTrsf ) != 1 ) {
	BAL_FreeTransformation( &resTransformation );
	BAL_FreeImage( &theFloating );
	BAL_FreeImage( &theTemplate );
	BAL_FreeTransformation( &theTransformation );  
	if ( isVerbose ) 
	  fprintf( stderr, "%s: unable to convert from REAL to VOXEL\n", program );
	return -1;
      }
      break;
    case REAL_UNIT :
      break;
    }
    
    break;

  }

  if ( isDebug ) BAL_PrintTransformation( stderr, resTrsf, "after conversion" );

  BAL_FreeImage( &theFloating );
  BAL_FreeImage( &theTemplate );





  /***************************************************
   *
   * writing transformation
   *
   ***************************************************/
  
  if ( restrsf_name != NULL ) {
    if ( BAL_WriteTransformation( resTrsf, restrsf_name ) != 1 ) {
      if ( isVerbose )
	fprintf( stderr, "%s: unable to write '%s'\n", program, restrsf_name );
      BAL_FreeTransformation( &resTransformation );
      BAL_FreeTransformation( &theTransformation );
      return -1;
    }
  }
 
  BAL_FreeTransformation( &resTransformation );
  BAL_FreeTransformation( &theTransformation );



  return 0;
}
