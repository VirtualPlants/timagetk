/*************************************************************************
 * bal-param-tools.c -
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

#include <bal-blockmatching-param-tools.h>

void BAL_AdjustBlockMatchingPyramidalParameters( bal_image *reference, 
                           bal_blockmatching_pyramidal_param *p )
{
  char *proc = "BAL_AdjustBlockMatchingPyramidalParameters";

  /* voxel/pixel selection
   */

  /* blocks geometry
   */
  if ( p->block_dim.x > (int)reference->ncols )
    p->block_dim.x = reference->ncols;
  if ( p->block_dim.y > (int)reference->nrows )
    p->block_dim.y = reference->nrows;
  if ( p->block_dim.z > (int)reference->nplanes )
    p->block_dim.z = reference->nplanes;

  if ( p->block_dim.x + 2*p->block_border.x > (int)reference->ncols )
    p->block_border.x = (reference->ncols - p->block_dim.x) / 2;
  if ( p->block_dim.y + 2*p->block_border.y > (int)reference->nrows )
    p->block_border.y = (reference->nrows - p->block_dim.y) / 2;
  if ( p->block_dim.z + 2*p->block_border.z > (int)reference->nplanes )
    p->block_border.z = (reference->nplanes - p->block_dim.z) / 2;

  /* pairing parameters
   */
  if ( 2*p->half_neighborhood_size.x + 1 > (int)reference->ncols )
    p->half_neighborhood_size.x = (reference->ncols - 1) / 2;
  if ( 2*p->half_neighborhood_size.y + 1 > (int)reference->nrows )
    p->half_neighborhood_size.y = (reference->nrows - 1) / 2;
  if ( 2*p->half_neighborhood_size.z + 1 > (int)reference->nplanes )
    p->half_neighborhood_size.z = (reference->nplanes - 1) / 2;

  /* transformation parameters
   */
  if ( reference->nplanes == 1 ) {
    switch ( p->transformation_type ) {
    default :
      break;
    case TRANSLATION_3D :
      p->transformation_type = TRANSLATION_2D;
      break;
    case TRANSLATION_SCALING_3D :
      p->transformation_type = TRANSLATION_SCALING_2D;
      break;
    case RIGID_3D :
      p->transformation_type = RIGID_2D;
      break;
    case SIMILITUDE_3D :
      p->transformation_type = SIMILITUDE_2D;
      break;
    case AFFINE_3D :
      p->transformation_type = AFFINE_2D;
      break;
    case VECTORFIELD_3D :
      p->transformation_type = VECTORFIELD_2D;
      break;
    }
  }
  
  switch ( p->transformation_type ) {
  default :
    break;
  case VECTORFIELD_3D :
    if ( p->block_dim.z % 2 != 1 ) {
      p->block_dim.z ++;
      if ( p->block_dim.z > (int)reference->nplanes ) {
        fprintf( stderr, "%s: can not change 'p->block_dim.z'\n", proc );
        exit( -1 );
      }
    }
  case VECTORFIELD_2D :
    if ( p->block_dim.x % 2 != 1 ) {
      p->block_dim.x ++;
      if ( p->block_dim.x > (int)reference->ncols ) {
        fprintf( stderr, "%s: can not change 'p->block_dim.x'\n", proc );
        exit( -1 );
      }
    }
    if ( p->block_dim.y % 2 != 1 ) {
      p->block_dim.y ++;
      if ( p->block_dim.y > (int)reference->nrows ) {
        fprintf( stderr, "%s: can not change 'p->block_dim.y'\n", proc );
        exit( -1 );
      }
    }
  }

  /* parameter for block matching at one level
    */
  
  /* parameter for pyramid building
   */
  
  /* general purpose parameters
   */

}
