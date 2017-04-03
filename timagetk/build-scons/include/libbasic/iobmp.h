/*************************************************************************
 * iobmp.h - I procedures for BMP raw images
 *
 * $Id: iobmp.h,v 1.1 1999/10/06 15:55:46 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Oct  6 17:03:48 MET DST 1999
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#ifndef _iobmp_h_
#define _iobmp_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>

#include <fcntl.h> /* open, close */
#include <sys/stat.h> /* open, close */
#include <sys/types.h> /* open, close */
#include <string.h>


extern void *_readBmpImage( char *name, int *dimx, int *dimy, int *dimz );


extern void IoBmp_verbose ( );
extern void IoBmp_noverbose ( );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _iobmp_h_ */
