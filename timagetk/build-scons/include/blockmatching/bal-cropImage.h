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

#ifndef BAL_CROPIMAGE_H
#define BAL_CROPIMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-transformation-tools.h>

int cropImage(
        char *theim_name,
        char *resim_name,
        char *real_transformation_name,
        char *voxel_transformation_name,
        char *template_image_name,
        int analyzeFiji,
        bal_integerPoint origin,
        bal_integerPoint dim,
        bal_integerPoint slice,
        int isDebug,
        int isVerbose
        );

#ifdef __cplusplus
}
#endif

#endif //BAL_CROPIMAGE_H
