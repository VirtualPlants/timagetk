#ifndef _vt_recfilters_h_
#define _vt_recfilters_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

#define VT_RECFILTERS_DERICHE  1
#define VT_RECGAUSSIAN_DERICHE 2
#define VT_RECGAUSSIAN_MARTA   3

#define VT_NODERIVATIVE -1
#define VT_DERIVATIVE_0  0
#define VT_SMOOTHING     0
#define VT_DERIVATIVE_1  1
#define VT_DERIVATIVE_2  2
#define VT_DERIVATIVE_3  3
#define VT_DERIVATIVE_1_CONTOURS 11

typedef struct vt_recfilters {
    int type_filter;
    vt_ipt derivative;
    vt_ipt length_continue;
    vt_fpt value_coefficient;
} vt_recfilters;

#include <vt_recline.h>

extern int VT_ComputeGradientWithRecursiveGaussian( vt_image *theIm,
					    vt_image *theGx,
					    vt_image *theGy,
					    vt_image *theGz,
					    float *sigma,
					    int *bordure );

extern int VT_SmoothWithRecursiveGaussian( vt_image *theIm,
				    vt_image *theRes,
				    float *sigma,
				    int *bordure );
extern void VT_RecFilters( vt_recfilters *par );
extern int  VT_RecFilterOnImage( vt_image *im1, vt_image *im2, vt_recfilters *par );


#ifdef __cplusplus
}
#endif

#endif /* _vt_recfilters_h_ */

