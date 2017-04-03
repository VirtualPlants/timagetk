/*************************************************************************
 * bal-matching.h -
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



#ifndef BAL_BLOCKMATCHING_H
#define BAL_BLOCKMATCHING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-blockmatching-param.h> 
#include <bal-image.h>
#include <bal-transformation.h>




extern void BAL_SetVerboseInBalBlockMatching( int v );
extern void BAL_IncrementVerboseInBalBlockMatching(  );
extern void BAL_DecrementVerboseInBalBlockMatching(  );
extern void BAL_SetDebugInBalBlockMatching( int v );
extern void BAL_IncrementDebugInBalBlockMatching(  );
extern void BAL_DecrementDebugInBalBlockMatching(  );
extern void BAL_SetTraceInBalBlockMatching( int v );
extern void BAL_IncrementTraceInBalBlockMatching(  );
extern void BAL_DecrementTraceInBalBlockMatching(  );
extern void BAL_SetTimeInBalBlockMatching( int v );
extern void BAL_IncrementTimeInBalBlockMatching(  );
extern void BAL_DecrementTimeInBalBlockMatching(  );

extern
bal_transformation *BAL_PyramidalBlockMatching( bal_image *theInrimage_ref,
                                                bal_image *theInrimage_flo,
                                                bal_transformation *theLeftTransformation,
                                                bal_transformation *theInitResult,
                                                bal_blockmatching_pyramidal_param *theParam );




extern int BAL_BlockMatching( bal_image *theInrimage_ref, /* reference image 
                                                 (may be subsampled) */
                              bal_image *theInrimage_flo, /* floating image
                                                 (in its original geometry,
                                                 may be filtered) */
                              bal_transformation *theLeft, /* initial transformation */
                              bal_transformation *theTr, /* allows to goes from Iref to Iflo
                                                (after initial transformation, if any)
                                                ie to resample Iflo into Iref
                                                that is T_{floating <- reference} */
                              bal_blockmatching_param *param );

#ifdef __cplusplus
}
#endif

#endif
