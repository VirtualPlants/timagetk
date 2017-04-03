/*************************************************************************
 * bal-interpolation.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2016, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun  4 avr 2016 22:07:46 CEST
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#ifndef BAL_INTERPOLATION_H
#define BAL_INTERPOLATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include <bal-stddef.h>



extern void BAL_SetVerboseInBalInterpolation( int v );
extern void BAL_IncrementVerboseInBalInterpolation(  );
extern void BAL_DecrementVerboseInBalInterpolation(  );


typedef struct typeLabelList {
  int *data;
  int n_data;
  int n_allocated_data;
} typeLabelList;

typedef struct typeCorrespondence {
  typeLabelList labelA;
  typeLabelList labelB;
} typeCorrespondence;


typedef struct typeCorrespondenceList {
  typeCorrespondence *data;
  int n_data;
  int n_allocated_data;
} typeCorrespondenceList;

extern void BAL_InitCorrespondenceList( typeCorrespondenceList *l );
extern void BAL_FreeCorrespondenceList( typeCorrespondenceList *l );
extern void BAL_PrintCorrespondenceList( FILE *f, typeCorrespondenceList *l );
extern int  BAL_ReadCorrespondenceList( FILE *f, typeCorrespondenceList *l );

#ifdef __cplusplus
}
#endif

#endif
