/*************************************************************************
 * applyTrsf.c -
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

#ifndef BAL_CREATEGRID_H
#define BAL_CREATEGRID_H

#include <bal-transformation-tools.h>

#ifdef __cplusplus
extern "C" {
#endif

  int createGrid(
		 char *resim_name,
		 char *template_image_name,
		 bal_integerPoint dim,
		 bal_doublePoint voxel,
		 bal_integerPoint offset,
		 bal_integerPoint spacing
                );

#ifdef __cplusplus
}
#endif

#endif //BAL_CREATEGRID_H
