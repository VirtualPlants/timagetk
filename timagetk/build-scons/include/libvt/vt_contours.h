#ifndef _vt_contours_h_
#define _vt_contours_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

#define VT_MAXIMA_GRADIENT 1
#define VT_ZERO_LAPLACIEN  2

typedef struct vt_contours {
    DimType dim;
    int type_contours;
    int type_filter;
    vt_ipt length_continue;
    vt_fpt value_coefficient;
} vt_contours;

#include <vt_recfilters.h>
#include <vt_maxgrad.h>

extern void VT_Contours( vt_contours *par );
extern int  VT_ExtractEdges( vt_image *im1, 
			     vt_image *im2, 
			     vt_contours *par );



extern int VT_NormeGradient( vt_image *theIm, vt_image *resIm, 
			     vt_contours *par, int derivative );

extern int VT_NormeGradientWithDerivatives( vt_image *theX, 
			     vt_image *theY, 
			     vt_image *theZ, 
			     vt_image *theNorme );

#ifdef __cplusplus
}
#endif

#endif /* _vt_contours_h_ */
