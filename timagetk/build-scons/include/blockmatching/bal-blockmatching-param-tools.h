/*************************************************************************
 * bal-param-tools.h -
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



#ifndef BAL_BLOCKMATCHING_PARAM_TOOLS_H
#define BAL_BLOCKMATCHING_PARAM_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-blockmatching-param.h>
#include <bal-image.h>

extern void BAL_AdjustBlockMatchingPyramidalParameters( bal_image *reference, 
                                                        bal_blockmatching_pyramidal_param *param );

#ifdef __cplusplus
}
#endif

#endif
