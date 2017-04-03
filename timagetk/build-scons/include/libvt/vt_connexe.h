/*
 * vt_connexe.c - connected components computation.
 *
 * $Id: vt_connexe.h,v 1.2 1999/10/13 09:05:45 greg Exp $
 *
 * DESCRIPTION:
 *
 * AUTHOR:        Gregoire Malandain
 * CREATION DATE: April 1997
 * COPYRIGHT:     Gregoire Malandain
 * RELEASE:       1.0
 *
 * ADDITIONS, CHANGES:
 *
 *
 *
 */


#ifndef _vt_connexe_h_
#define _vt_connexe_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_neighborhood.h>
#include <vt_copy.h>
#include <vt_seuil.h>

#define VT_F2I( F ) ( (F) >= 0.0 ? ((int)((F)+0.5)) : ((int)((F)-0.5)) )

#define VT_BINAIRE 1
#define VT_GREY    2
#define VT_SIZE    3


#define _EQUIVALENCE_ARRAY_SIZE_ 65536


typedef struct vt_connexe{
  int min_size;      /* minimum size of connected components */
  int max_nbcc;      /* maximum number of connected components */
  Neighborhood type_connexite; /* connectivity */
  int type_output;   /* output: binary image or label image */
  int dim;           /* if 2D, slice by slice computation */
  int verbose;
  /* for compatibility with vt_slconnexe.c */
  int threshold;
} vt_connexe;



extern int  VT_ConnectedComponents( vt_image *theIm, vt_image *res, float t, vt_connexe *par );
extern int  VT_Hysteresis( vt_image *theIm, vt_image *res, float l, float h, vt_connexe *par );
extern int  VT_ConnectedComponentWithOneSeed( vt_image *theIm, vt_image *res, float t, vt_ipt *seed, vt_connexe *par );
extern int  VT_ConnectedComponentsWithSeeds( vt_image *theIm, vt_image *res, float t, vt_image *seeds, vt_connexe *par );
extern int  VT_CheckConnectivity( Neighborhood connexite, int dimz );
extern void VT_Connexe( vt_connexe *par );



#ifdef __cplusplus
}
#endif

#endif
