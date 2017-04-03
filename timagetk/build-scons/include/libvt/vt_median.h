#ifndef _vt_median_h_
#define _vt_median_h_


#ifdef __cplusplus
extern "C" {
#endif

#include <vt_neighborhood.h>
#include <vt_common.h>

extern int VT_MedianFilter( vt_image *theIm, vt_image *resIm, vt_ipt *window );

#ifdef __cplusplus
}
#endif

#endif /* _vt_median_h_ */
