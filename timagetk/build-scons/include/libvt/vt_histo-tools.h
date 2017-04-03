/*************************************************************************
 * vt_histo-tools.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2014, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer  9 jul 2014 23:50:16 CEST
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#ifndef VT_HISTO_TOOLS_H
#define VT_HISTO_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

extern int VT_ReadImageAsHistogram( char *filename, typeHistogram *h );
extern int VT_WriteHistogramAsImage( char *filename, typeHistogram *h );

extern int VT_WriteHistogram( char *filename, typeHistogram *h, char *description );

extern int VT_ImageToHistogram( vt_image *im, typeHistogram *h  );
extern int VT_HistogramToImage( typeHistogram *h, vt_image *im );

#ifdef __cplusplus
}
#endif

#endif
