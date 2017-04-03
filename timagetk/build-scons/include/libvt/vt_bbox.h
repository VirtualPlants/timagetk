#ifndef _vt_bbox_h_
#define _vt_bbox_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

extern int VT_BoundingBox( vt_image *theIm, vt_ipt *min, vt_ipt *max );
extern int VT_CopyBBoxWithBorders( vt_image *theIm, vt_image *resIm, vt_ipt *bor, vt_ipt *min );

#ifdef __cplusplus
}
#endif

#endif /* _vt_bbox_h_ */
