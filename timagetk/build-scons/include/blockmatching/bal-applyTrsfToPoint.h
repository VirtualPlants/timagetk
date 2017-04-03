/*************************************************************************
 * applyTrsf.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Etienne Delclaux (etienne.delclaux@inria.fr) 
 * From Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Tue Sep 17 12:39:00 CET 2013
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#ifndef BAL_APPLYTRSFS_TO_POINT_H
#define BAL_APPLYTRSFS_TO_POINT_H

#ifdef __cplusplus
extern "C" {
#endif



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <bal-stddef.h>
#include <bal-image.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>

int applyTrsfToPoint(
       bal_doublePoint thePt,
       bal_doublePoint* resPt,
       char *real_transformation_name,
       int verbose,
       int debug);

#ifdef __cplusplus
}
#endif

#endif //BAL_APPLYTRSFS_TO_POINTS_H
