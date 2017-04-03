/*************************************************************************
 * bal-image-tools.h -
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



#ifndef BAL_IMAGE_TOOLS_H
#define BAL_IMAGE_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-behavior.h>
#include <bal-stddef.h>
#include <bal-image.h>

extern void BAL_SetVerboseInBalImageTools( int v );
extern void BAL_IncrementVerboseInBalImageTools(  );
extern void BAL_DecrementVerboseInBalImageTools(  );

extern int BAL_DrawGrid( bal_image *theIm,
			 bal_integerPoint *offset,
			 bal_integerPoint *spacing );

#ifdef __cplusplus
}
#endif

#endif
