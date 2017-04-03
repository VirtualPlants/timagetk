/*************************************************************************
 * cropImage.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Thu Nov 22 18:04:47 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */
#include "bal-cropImage.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <extract.h>

#include <bal-image.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>

static char* program = "cropImage";

static int _debug_ = 0;
static int _verbose_ = 0;

int cropImage(
	      char *theim_name,
	      char *resim_name,
	      char *real_transformation_name,
	      char *voxel_transformation_name,
	      char *template_image_name, /* template */
        int analyzeFiji,
	      bal_integerPoint origin,
	      bal_integerPoint dim,
	      bal_integerPoint slice,
	      int isDebug,
	      int isVerbose
	      )
{
  bal_image theim;
  bal_image resim;
  bal_image template;
  bal_transformation theTrsf;
  int theDim[3];
  int resDim[3] = {-1, -1, -1};
  int theLeftCorner[3] = {0, 0, 0};
  int theRightCorner[3] = {0, 0, 0};

  _debug_ = isDebug;
  _verbose_ = isVerbose;

  /***************************************************
   *
   *
   *
   ***************************************************/
  BAL_InitImage( &theim, NULL, 0, 0, 0, 0, UCHAR );
  BAL_InitImage( &resim, NULL, 0, 0, 0, 0, UCHAR );
  BAL_InitImage( &template, NULL, 0, 0, 0, 0, UCHAR );
  BAL_InitTransformation( &theTrsf );



  if ( BAL_AllocTransformation( &theTrsf, AFFINE_3D, (bal_image*)NULL ) != 1 ) {
    fprintf( stderr, "%s: Unable to allocate result transformation \n", program);
    return -1;
  }
  BAL_SetTransformationToIdentity( &theTrsf );
  theTrsf.transformation_unit = VOXEL_UNIT;



  /* reading input image
   */
  if ( BAL_ReadImage( &theim, theim_name, 0 ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      fprintf( stderr, "%s: can not read input image '%s'\n", program, theim_name );
      return -1;
  }
  theDim[0] = theim.ncols;
  theDim[1] = theim.nrows;
  theDim[2] = theim.nplanes;



  /* initializing result image
     - with reference image, if any
     - with parameters, if any
     - with transformation, if vector field
     - with input image
  */

  /* initialisation with a reference image
   */
  if ( template_image_name != NULL ) {
    if ( BAL_ReadImage( &template, template_image_name, 0 ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      if ( _verbose_ )
	fprintf( stderr, "%s: can not read template image '%s'\n", program, template_image_name );
      return -1;
    }

    theLeftCorner[0] = origin.x;
    theLeftCorner[1] = (analyzeFiji == 0) ? origin.y : (int)theim.nrows - (int)template.nrows - origin.y;
    theLeftCorner[2] = origin.z;

    theRightCorner[0] = theLeftCorner[0] + template.ncols;
    theRightCorner[1] = theLeftCorner[1] + template.nrows;
    theRightCorner[2] = theLeftCorner[2] + template.nplanes;

    BAL_FreeImage( &template );

    if ( theLeftCorner[0] < 0 ) theLeftCorner[0] = 0;
    if ( theLeftCorner[1] < 0 ) theLeftCorner[1] = 0;
    if ( theLeftCorner[2] < 0 ) theLeftCorner[2] = 0;

    if ( theRightCorner[0] > (int)theim.ncols ) theRightCorner[0] = theim.ncols;
    if ( theRightCorner[1] > (int)theim.nrows ) theRightCorner[1] = theim.nrows;
    if ( theRightCorner[2] > (int)theim.nplanes ) theRightCorner[2] = theim.nplanes;

    if ( BAL_InitAllocImage( &resim, (char*)NULL,
			     theRightCorner[0] - theLeftCorner[0],
			     theRightCorner[1] - theLeftCorner[1],
			     theRightCorner[2] - theLeftCorner[2],
			     theim.vdim, theim.type ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to initialize result image (from template image)\n", program );
      return -1;
    }

    resim.vx = theim.vx;
    resim.vy = theim.vy;
    resim.vz = theim.vz;

    theTrsf.mat.m[ 3] = theLeftCorner[0];
    theTrsf.mat.m[ 7] = theLeftCorner[1];
    theTrsf.mat.m[11] = theLeftCorner[2];

    resDim[0] = resim.ncols;
    resDim[1] = resim.nrows;
    resDim[2] = resim.nplanes;
  }



  /* initialisation with dimension parameters
   */
  else if ( dim.x > 0 && dim.y > 0 ) {

    theLeftCorner[0] = origin.x;
    theLeftCorner[1] = (analyzeFiji == 0) ? origin.y : (int)theim.nrows - dim.y - origin.y;

    theRightCorner[0] = theLeftCorner[0] + dim.x;
    theRightCorner[1] = theLeftCorner[1] + dim.y;

    if ( theLeftCorner[0] < 0 ) theLeftCorner[0] = 0;
    if ( theLeftCorner[1] < 0 ) theLeftCorner[1] = 0;

    if ( theRightCorner[0] > (int)theim.ncols ) theRightCorner[0] = theim.ncols;
    if ( theRightCorner[1] > (int)theim.nrows ) theRightCorner[1] = theim.nrows;

    if ( dim.z > 0 ) {

      theLeftCorner[2] = origin.z;
      theRightCorner[2] = theLeftCorner[2] + dim.z;

      if ( theLeftCorner[2] < 0 ) theLeftCorner[2] = 0;
      if ( theRightCorner[2] > (int)theim.nplanes ) theRightCorner[2] = theim.nplanes;

    }
    else {

      theLeftCorner[2] = 0;
      theRightCorner[2] = 1;

    }

    if ( BAL_InitAllocImage( &resim, (char*)NULL,
			     theRightCorner[0] - theLeftCorner[0],
			     theRightCorner[1] - theLeftCorner[1],
			     theRightCorner[2] - theLeftCorner[2],
			     theim.vdim, theim.type ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to initialize result image (from parameters)\n", program );
      return -1;
    }

    resim.vx = theim.vx;
    resim.vy = theim.vy;
    resim.vz = theim.vz;

    theTrsf.mat.m[ 3] = theLeftCorner[0];
    theTrsf.mat.m[ 7] = theLeftCorner[1];
    theTrsf.mat.m[11] = theLeftCorner[2];

    resDim[0] = resim.ncols;
    resDim[1] = resim.nrows;
    resDim[2] = resim.nplanes;
  }



  /* initialisation with slice information
   */
  else if ( slice.z >= 0 && slice.z < (int)theim.nplanes ) {
    if ( BAL_InitAllocImage( &resim, (char*)NULL,
			     theim.ncols, theim.nrows, 1,
			     theim.vdim, theim.type ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      fprintf( stderr, "%s: unable to initialize result image (XY slice)\n", program );
      return -1;
    }
    resim.vx = theim.vx;
    resim.vy = theim.vy;
    resim.vz = theim.vz;

    theTrsf.mat.m[11] = slice.z;

    resDim[0] = theim.ncols;
    resDim[1] = theim.nrows;
    resDim[2] = 1;

    theLeftCorner[2] = slice.z;
  }

  else if ( slice.y >= 0 && slice.y < (int)theim.nrows ) {
    if ( BAL_InitAllocImage( &resim, (char*)NULL,
			     theim.ncols, theim.nplanes, 1,
			     theim.vdim, theim.type ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      fprintf( stderr, "%s: unable to initialize result image (XZ slice)\n", program );
      return -1;
    }
    resim.vx = theim.vx;
    resim.vy = theim.vz;
    resim.vz = theim.vy;

    theTrsf.mat.m[0] = theTrsf.mat.m[5] = theTrsf.mat.m[10] = 0.0;
    theTrsf.mat.m[0] = 1.0;
    theTrsf.mat.m[6] = -1.0;
    theTrsf.mat.m[9] = 1.0;
    theTrsf.mat.m[7] = (analyzeFiji == 0) ? origin.y : (int)theim.nrows - origin.y - 1;

    resDim[0] = theim.ncols;
    resDim[1] = 1;
    resDim[2] = theim.nplanes;

    theLeftCorner[1] = (analyzeFiji == 0) ? origin.y : (int)theim.nrows - origin.y - 1;
  }

  else if ( slice.x >= 0 && slice.x < (int)theim.ncols ) {
    if ( BAL_InitAllocImage( &resim, (char*)NULL,
			     theim.nrows, theim.nplanes, 1,
			     theim.vdim, theim.type ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to initialize result image (XZ slice)\n", program );
      return -1;
    }
    resim.vx = theim.vy;
    resim.vy = theim.vz;
    resim.vz = theim.vx;

    theTrsf.mat.m[0] = theTrsf.mat.m[5] = theTrsf.mat.m[10] = 0.0;
    theTrsf.mat.m[4] = 1.0;
    theTrsf.mat.m[9] = 1.0;
    theTrsf.mat.m[2] = 1.0;
    theTrsf.mat.m[3] = slice.x;

    resDim[0] = 1;
    resDim[1] = theim.ncols;
    resDim[2] = theim.nplanes;

    theLeftCorner[0] = slice.x;
  }

  else {
    BAL_FreeTransformation( &theTrsf );
    BAL_FreeImage( &theim );
    if ( _verbose_ )
      fprintf( stderr, "%s: no (valid?) geometrical information?\n", program );
    return -1;
  }

  /***************************************************
   *
   *
   *
   ***************************************************/

  ExtractFromBuffer( theim.data, theDim, resim.data, resDim, theLeftCorner, theim.type );


  /***************************************************
   *
   *
   *
   ***************************************************/




  if ( BAL_WriteImage( &resim, resim_name ) != 1 ) {
    BAL_FreeTransformation( &theTrsf );
    BAL_FreeImage( &theim );
    BAL_FreeImage( &resim );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to write result image '%s'\n", program, resim_name );
    return -1;
  }

  if ( voxel_transformation_name != (char*)NULL ) {
    if ( BAL_WriteTransformation( &theTrsf, voxel_transformation_name ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      BAL_FreeImage( &resim );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to write result voxel transformation '%s'\n", program, voxel_transformation_name  );
      return -1;
    }
  }

  if ( real_transformation_name != (char*)NULL ) {
    if ( BAL_ChangeTransformationToRealUnit( &theim, &resim, &theTrsf, &theTrsf ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      BAL_FreeImage( &resim );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to convert 'voxel' transformation into the 'real' world\n", program );
      return -1;
    }
    if ( BAL_WriteTransformation( &theTrsf, real_transformation_name ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeImage( &theim );
      BAL_FreeImage( &resim );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to write result voxel transformation '%s'\n", program, real_transformation_name  );
      return -1;
    }
  }

  BAL_FreeTransformation( &theTrsf );
  BAL_FreeImage( &theim );
  BAL_FreeImage( &resim );


  return 0;
}
