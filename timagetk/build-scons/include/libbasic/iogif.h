/*************************************************************************
 * iogif.h - I procedures for GIF
 *
 * $Id: iogif.h,v 1.1 2000/04/26 15:50:25 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Apr 26 17:28:19 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#ifndef _iogif_h_
#define _iogif_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>

#include <fcntl.h> /* open, close */
#include <sys/stat.h> /* open, close */
#include <sys/types.h> /* open, close */
#include <string.h>
#include <stdlib.h>


extern void *_readBmpImage( char *name, int *dimx, int *dimy, int *dimz );


extern void IoGif_verbose ( );
extern void IoGif_noverbose ( );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _iogif_h_ */
