/*************************************************************************
 * file-tools.h -
 *
 * $Id: file-tools.h,v 1.2 2000/04/26 15:50:25 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Apr 26 13:24:41 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#ifndef _file_tools_h_
#define _file_tools_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

/* magic strings
   pour les fichiers compresses
*/
#define FUTIL_MAGIC_PACK      "\037\036" /* Magic header for packed files */
#define FUTIL_MAGIC_GZIP      "\037\213" /* Magic header for gzip files, 1F 8B */
#define FUTIL_MAGIC_OLD_GZIP  "\037\236" /* Magic header for gzip 0.5 = freeze 1.x */
#define FUTIL_MAGIC_LZH       "\037\240" /* Magic header for SCO LZH Compress files*/
#define FUTIL_MAGIC_PKZIP     "\120\113\003\004" /* Magic header for pkzip files */
#define FUTIL_MAGIC_LZW       "\037\235"   /* Magic header for lzw files, 1F 9D */



#define FUTIL_MAGIC_INR       "#INRIMAGE-4#"

#define FUTIL_MAGIC_GIF       "GIF8"

#define FUTIL_MAGIC_PGM       "P5" /* Portable graymap format (gray scale) */
#define FUTIL_MAGIC_PNM       "P5" /* Portable anymap */
#define FUTIL_MAGIC_PPM       "P6" /* Portable pixmap format (color) */


#define FUTIL_MAGIC_ICO       "IC"
#define FUTIL_MAGIC_BMP       "BM"
#define FUTIL_MAGIC_PTR       "PT"
#define FUTIL_MAGIC_ICO_COLOR "CI"
#define FUTIL_MAGIC_PTR_COLOR "CT"
#define FUTIL_MAGIC_ARRAY     "BA"

typedef enum {
  _TYPEFILE_UNKNOWN_,
  _TYPEFILE_COMPRESSED_,
  _TYPEFILE_INR_,
  _TYPEFILE_GIF_,
  _TYPEFILE_PGM_,
  _TYPEFILE_PNM_,
  _TYPEFILE_PPM_,
  _TYPEFILE_BMP_
} enumTypeFile;





extern enumTypeFile getImageFileType( char *name );

extern int getFileSize( char *name );






#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _file_tools_h_ */
