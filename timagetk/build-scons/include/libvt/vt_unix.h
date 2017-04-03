/*************************************************************************
 * vt_unix.h -
 *
 * $Id: vt_unix.h,v 1.2 2000/06/23 16:00:35 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 *
 *
 * ADDITIONS, CHANGES
 * 
 *
 */




#ifndef _vt_unix_h_
#define _vt_unix_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>  
#include <unistd.h>
#include <time.h>

#include <vt_typedefs.h>
#include <vt_error.h>

/*--- type for VT_Free ---*/
#if (defined(_dec_) || defined(_alpha_))
typedef void* FreeType;
#else
typedef char* FreeType;
#endif

extern void  *VT_Malloc( unsigned long int size );
extern void   VT_Free( void** pt );
extern int    VT_Strlen( char *s );
extern void   VT_Strncpy( char *s1, char *s2, int n );
extern void   VT_Strcpy( char *s1, char *s2 );
extern int    VT_Strncmp( char *s1, char *s2, int n );
extern int    VT_ROpen( char *s );
extern int    VT_WOpen( char *s );
extern void   VT_Close( int fd );
extern int    VT_Read( int fd, char *buf, int nbytes );
extern int    VT_Write( int fd, char *buf, int nbytes );
extern void   VT_Memcpy( char *s1, char *s2, int n );

#ifdef __cplusplus
}
#endif

#endif /* _vt_unix_h_ */
