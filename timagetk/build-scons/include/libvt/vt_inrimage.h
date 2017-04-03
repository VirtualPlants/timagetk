/*************************************************************************
 * vt_inrimage.h -
 *
 * $Id: vt_inrimage.h,v 1.2 2000/09/07 07:42:33 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 */


#ifndef _vt_inrimage_h_
#define _vt_inrimage_h_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vt_typedefs.h>
#include <vt_unix.h>
#include <vt_bytes.h>
#include <vt_error.h>
#include <vt_names.h>
#include <vt_image.h>



extern vt_image *_VT_Inrimage( char *name );
extern int VT_ReadInrimage( vt_image *image, /* structure deja allouee */
		     char *name /* file name containing the inrimage image */ );

extern int        VT_WriteInrimage( vt_image *image );

extern int VT_WriteInrimageWithName( vt_image *image, char *name );

extern int  VT_ReadInrimHeader( vt_image *image, /* structure deja allouee */
		     char *name /* file name containing the inrimage image */ );
extern void VT_FillInrimHeader( char *header, vt_image *image );


#ifdef __cplusplus
}
#endif

#endif /* _vt_inrimage_h_ */
