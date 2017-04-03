/*************************************************************************
 * bal-block-tools.h -
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



#ifndef BAL_BLOCK_TOOLS_H
#define BAL_BLOCK_TOOLS_H


#ifdef __cplusplus
extern "C" {
#endif


#include <bal-image.h>
#include <bal-block.h>




extern void BAL_SetVerboseInBalBlockTools( int v );
extern void BAL_IncrementVerboseInBalBlockTools(  );
extern void BAL_DecrementVerboseInBalBlockTools(  );
extern void BAL_SetDebugInBalBlockTools( int d );
extern void BAL_IncrementDebugInBalBlockTools(  );
extern void BAL_DecrementDebugInBalBlockTools(  );


extern double BAL_ComputeBlockSimilarity2D( BLOC *bloc_flo, BLOC *bloc_ref,
					    bal_image *image_flo, bal_image *image_ref,
					    bal_intensitySelection *selection_flo,
					    bal_intensitySelection *selection_ref,
					    bal_integerPoint *blockdim,
					    enumTypeSimilarity mesure );

extern double BAL_ComputeBlockSimilarity3D( BLOC *bloc_flo, BLOC *bloc_ref,
					    bal_image *image_flo, bal_image *image_ref,
					    bal_intensitySelection *selection_flo,
					    bal_intensitySelection *selection_ref,
					    bal_integerPoint *blockdim,
					    enumTypeSimilarity mesure );

extern int BAL_ComputeBlockAttributes( bal_image *inrimage, BLOCS *blocs );

#ifdef __cplusplus
}
#endif

#endif
