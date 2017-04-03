#ifndef _vt_amseuil_h_
#define _vt_amseuil_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_amincir.h>


extern int _VT_ThinThreshold( vt_image *im1, vt_image *im2, float thres );
extern int _VT_Thin2Thresholds( vt_image *im1, vt_image *im2, float thres1, float thres2 );


#ifdef __cplusplus
}
#endif

#endif /* _vt_amseuil_h_ */
