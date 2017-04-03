#ifndef _vt_maxgrad_h_
#define _vt_maxgrad_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_contours.h>
#include <vt_recfilters.h>

extern int VT_MaximaGradient( vt_image *im1, 
			      vt_image *im2, 
			      vt_contours *par );
#ifdef __cplusplus
}
#endif

#endif /* _vt_maxgrad_h_ */
