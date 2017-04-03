/*************************************************************************
 * vt_copy.h - copie d'images
 *
 * $Id: vt_copy.h,v 1.3 2003/06/20 08:59:53 greg Exp $
 *
 * DESCRIPTION: 
 *
 *
 *
 *
 *
 * AUTHOR:
 * Gregoire Malandain
 *
 * CREATION DATE:
 * 1996
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 * - Thu Jul 22 11:10:07 MET DST 1999 (GM)
 *   ajout de #include <string.h> pour l'utilisation de memcpy() 
 *   dans VT_CopyImage()
 *
 *
 */


#ifndef _vt_copy_h_
#define _vt_copy_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include <vt_typedefs.h>
#include <vt_unix.h>
#include <vt_error.h>
#include <vt_getval.h>
#include <vt_3m.h>

extern int VT_CopyImage( vt_image *im1, vt_image *im2 );
extern int VT_NormaImage( vt_image *im1, vt_image *im2 );
extern int VT_NormaImageWithinInterval( vt_image *theIm, /* image to be copied */
			       vt_image *resIm,  /* image where the copy is to be done */
			       double min,
			       double max );
extern int VT_NormaImageFromInterval( vt_image *theIm, /* image to be copied */
			       vt_image *resIm,  /* image where the copy is to be done */
			       double min,
			       double max );
extern int VT_NormaMultImage( vt_image *im1, vt_image *im2 );
extern int VT_CopyWindow( vt_image *im1, vt_image *im2, vt_ipt *pt1, vt_ipt *pt2, vt_ipt *pt3 );

#ifdef __cplusplus
}
#endif

#endif 
