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

#ifndef BAL_COPYTRSF_H
#define BAL_COPYTRSF_H

#include <bal-transformation-tools.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	       );

#ifdef __cplusplus
}
#endif

#endif //BAL_COPYTRSF_H
