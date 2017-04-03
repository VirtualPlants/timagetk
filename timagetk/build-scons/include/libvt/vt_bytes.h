#ifndef _vt_bytes_h_
#define _vt_bytes_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_typedefs.h>
#include <vt_error.h>
#include <vt_image.h>
#include <vt_getval.h>

extern void VT_SwapImage( vt_image *image );
extern void VT_InverseImage( vt_image *image );
extern void VT_ZeroImage( vt_image *image );
extern void VT_FillImage( vt_image *image, double val );
extern void VT_Swap2Octets( vt_image *out, vt_image *in );
extern void VT_Swap4Octets( vt_image *out, vt_image *in );
extern void VT_LogicInverse( vt_image *out, vt_image *in );
extern void VT_LogicOu( vt_image *in1, vt_image *in2, vt_image *out );
extern void VT_LogicEt( vt_image *in1, vt_image *in2, vt_image *out );
extern void VT_LogicXou( vt_image *in1, vt_image *in2, vt_image *out );

extern int _CPUtoCPU( vt_image *image );

#ifdef __cplusplus
}
#endif

#endif 
