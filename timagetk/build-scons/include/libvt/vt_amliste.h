#ifndef _vt_amliste_h_
#define _vt_amliste_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_amincir.h>

extern vt_pt_amincir*  _VT_ThinPtList( vt_image *image, 
				       int dim, 
				       int *nb );
extern vt_vpt_amincir* _VT_ThinVPtList( vt_image *image, 
					vt_image *value, 
					int dim, 
					int *nb );

#ifdef __cplusplus
}
#endif

#endif /* _vt_amliste_h_ */
