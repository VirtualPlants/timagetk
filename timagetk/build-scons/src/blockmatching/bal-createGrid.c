/*************************************************************************
 * bal-createGrid.c -
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include <bal-stddef.h>
#include <bal-image.h>
#include <bal-image-tools.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>



#include <bal-createGrid.h>



static char *program = "createGrid";

int createGrid(
	       char *resim_name,
	       char *template_image_name,
	       bal_integerPoint dim,
	       bal_doublePoint voxel,
	       bal_integerPoint offset,
	       bal_integerPoint spacing
	       )
{
  /* Initialize parameters
   */
  bal_image resim;
  char auxstr[256], *auxptr;

  /***************************************************
   *
   * 
   *
   ***************************************************/

  if ( template_image_name != NULL ) {
    if ( BAL_ReadImage( &resim, template_image_name, 0 ) != 1 ) {    
      fprintf( stderr, "%s: can not read '%s'\n", program, template_image_name );
      return -1;
    }
    dim.x = resim.ncols;
    dim.y = resim.nrows;
    dim.z = resim.nplanes;
    voxel.x = resim.vx;
    voxel.y = resim.vy;
    voxel.z = resim.vz;
    BAL_FreeImage( &resim );
  }

  if ( BAL_InitAllocImage( &resim, NULL, dim.x, dim.y, dim.z, 1, UCHAR ) == -1 ) {
    fprintf( stderr, "%s: can not allocate result image \n", program );
    return -1;
  }
  resim.vx = voxel.x;
  resim.vy = voxel.y;
  resim.vz = voxel.z;
			   
  if ( BAL_DrawGrid( &resim, &(offset), &(spacing) ) != 1 ) {
    fprintf( stderr, "%s: can not draw grid \n", program );
    BAL_FreeImage( &resim );
    return -1;
  }

#ifndef WIN32
  sprintf( auxstr, "%s-%d-result-image.hdr", program, getpid() );
#endif

  auxptr =  ( resim_name != NULL ) ? resim_name : auxstr;
  
  if ( BAL_WriteImage( &resim, auxptr ) != 1 ) {
    fprintf( stderr, "%s: unable to write result image\n", program );
    BAL_FreeImage( &resim );
    return -1;
  }

  BAL_FreeImage( &resim );


  return 0;
}
