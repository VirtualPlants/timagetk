#ifndef _vt_gausssmooth_h_
#define _vt_gausssmooth_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

extern int VT_GaussianSmoothing( vt_image *theIm, 
				 vt_image *resIm, 
				 double sigma, 
				 int length );

#ifdef __cplusplus
}
#endif

#endif /* _vt_gausssmooth_h_ */
