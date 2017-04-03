/*************************************************************************
 * bal-field-tools.h -
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



#ifndef BAL_FIELD_TOOLS_H
#define BAL_FIELD_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include <bal-image.h>
#include <bal-block.h>
#include <bal-field.h>
#include <bal-estimator.h>
#include <bal-point.h>

extern void BAL_SetVerboseFileInBalFieldTools( FILE *f );
extern void BAL_SetVerboseInBalFieldTools( int v );
extern void BAL_IncrementVerboseInBalFieldTools(  );
extern void BAL_DecrementVerboseInBalFieldTools(  );
extern void BAL_SetDebugInBalFieldTools( int d );
extern void BAL_IncrementDebugInBalFieldTools(  );
extern void BAL_DecrementDebugInBalFieldTools(  );


extern int BAL_SelectSmallestResiduals( FIELD *field,
					bal_estimator *estimator );

extern int BAL_ComputePairingFieldFromRefToFlo( FIELD *field,
			      bal_image *inrimage_flo, BLOCS *blocs_flo,
			      bal_image *inrimage_ref, BLOCS *blocs_ref, 
			      bal_integerPoint *half_neighborhood_size,
			      bal_integerPoint *step_neighborhood_search,
			      enumTypeSimilarity measure_type,
				     double measure_threshold );

extern int BAL_ComputePairingFieldFromPointList( FIELD *field,
						 bal_typeFieldPointList *floPoints,
						 bal_typeFieldPointList *refPoints );


#ifdef __cplusplus
}
#endif

#endif
