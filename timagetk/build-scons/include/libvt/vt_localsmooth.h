#ifndef _vt_localsmooth_h_
#define _vt_localsmooth_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_neighborhood.h>
#include <vt_common.h>

extern int VT_06LocalSmoothing( vt_image *theIm, vt_image *resIm, int iterations );
extern int VT_26LocalSmoothing( vt_image *theIm, vt_image *resIm, int iterations );
extern int VT_SmoothingWithConnectivity( vt_image *theIm, vt_image *resIm, Neighborhood c, int i );

#ifdef __cplusplus
}
#endif

#endif /* _vt_localsmooth_h_ */
