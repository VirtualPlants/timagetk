#ifndef _vt_getval_h_
#define _vt_getval_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_typedefs.h>
#include <vt_error.h>
#include <vt_image.h>

extern double VT_GetXYZvalue( vt_image *image, int x, int y, int z );
extern double VT_GetVXYZvalue( vt_image *image, int v, int x, int y, int z );
extern double VT_GetINDvalue( vt_image *image, int i );

extern void   VT_SetXYZvalue( vt_image *image, int x, int y, int z, double val );
extern void   VT_SetVXYZvalue( vt_image *image, int v, int x, int y, int z, double val );
extern void   VT_SetINDvalue( vt_image *image, int i, double val );

extern double _VT_GetXYZvalue( vt_image *image, int x, int y, int z );
extern double _VT_GetVXYZvalue( vt_image *image, int v, int x, int y, int z );
extern double _VT_GetVDXYZvalue( vt_image *image, int v, double x, double y, double z );
extern double _VT_GetINDvalue( vt_image *image, int i );
extern void   _VT_SetXYZvalue( vt_image *image, int x, int y, int z, double val );
extern void   _VT_SetVXYZvalue( vt_image *image, int v, int x, int y, int z, double val );
extern void   _VT_SetINDvalue( vt_image *image, int i, double val );

#ifdef __cplusplus
}
#endif

#endif 
