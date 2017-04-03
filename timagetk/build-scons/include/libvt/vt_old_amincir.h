#ifndef _vt_old_amincir_h_
#define _vt_old_amincir_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_amincir.h>
#include <vt_amliste.h>
#include <vt_cc.h>

extern int _VT_OLD_GONGBERTRAND( vt_image *im, vt_amincir *par );
extern int _VT_OLD_GREG_PLANES( vt_image *im, vt_amincir *par );
extern int _VT_OLD_GREG_CURVES( vt_image *im, vt_amincir *par );
extern int _VT_OLD_GREG_2D( vt_image *im, vt_amincir *par );


#ifdef __cplusplus
}
#endif

#endif /* _vt_old_amincir_eh_ */
