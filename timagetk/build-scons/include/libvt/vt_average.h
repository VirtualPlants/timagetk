#ifndef _vt_average_h_
#define _vt_average_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

extern int VT_AverageFilter( vt_image *theIm, 
			     vt_image *resIm, 
			     vt_ipt *window );

#ifdef __cplusplus
}
#endif

#endif /* _vt_average_h_ */
