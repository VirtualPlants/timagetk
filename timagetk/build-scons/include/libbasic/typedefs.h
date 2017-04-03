/*************************************************************************
 * typedefs.h - 
 *
 * $Id: typedefs.h,v 1.3 2004/06/04 16:47:31 greg Exp $
 *
 * LICENSE:
 * GPL v3.0 (see gpl-3.0.txt for details)
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * June, 9 1998
 *
 *
 *
 */

#ifndef _typedefs_h_
#define _typedefs_h_

#ifdef __cplusplus
extern "C" {
#endif




/* Differents type coding for images and buffers.
 */
typedef enum {
  TYPE_UNKNOWN /* unknown type */,
  UCHAR  /* unsigned char */,
  SCHAR  /* signed char */,
  USHORT /* unsigned short int */,
  SSHORT /* signed short int */,
  UINT   /* unsigned int */,
  SINT    /* signed int */,
  ULINT  /* unsigned long int */,
  SLINT  /* signed long int */,
  FLOAT  /* float */,
  DOUBLE  /* double */
} ImageType, bufferType;

typedef char               s8;
typedef unsigned char      u8;
typedef short int          s16;
typedef unsigned short int u16;
typedef int                i32;
typedef int                s32;
typedef unsigned int       u32;
typedef long int           s64;
typedef unsigned long int  u64;
typedef float              r32;
typedef double             r64;

typedef union {
  s8 val_s8;
  u8 val_u8;
  s16 val_s16;
  u16 val_u16;
  s32 val_s32;
  u32 val_u32;
  s64 val_s64;
  u64 val_u64;
  r32 val_r32;
  r64 val_r64;
} unionValues;





/* Typedef Booleen
 */
typedef enum {
  False = 0,
  True = 1
} typeBoolean;





/* string length
 */
#define STRINGLENGTH 256



#ifdef __cplusplus
}
#endif

#endif
