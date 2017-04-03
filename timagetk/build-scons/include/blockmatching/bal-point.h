/*************************************************************************
 * bal-point.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun 23 sep 2013 16:52:21 CEST
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#ifndef BAL_POINT_H
#define BAL_POINT_H

#ifdef __cplusplus
extern "C" {
#endif





#include <bal-stddef.h>


extern void BAL_SetVerboseInBalPoint( int v );
extern void BAL_IncrementVerboseInBalPoint(  );
extern void BAL_DecrementVerboseInBalPoint(  );



typedef struct {
  bal_typeFieldPoint *data;
  int n_data;
  int n_allocated_data;

  /* is the displacement encoded in voxel or real units ?
   */
  enumUnitTransfo unit;

} bal_typeFieldPointList;


extern void BAL_InitTypeFieldPointList( bal_typeFieldPointList *l );
extern void BAL_FreeTypeFieldPointList( bal_typeFieldPointList *l );
extern void BAL_PrintTypeFieldPointList( FILE *f, bal_typeFieldPointList *l );
extern int BAL_ReadTypeFieldPointList( bal_typeFieldPointList *l, char *filename );



#ifdef __cplusplus
}
#endif

#endif
