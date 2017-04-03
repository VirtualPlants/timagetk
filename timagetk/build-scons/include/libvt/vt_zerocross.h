#ifndef _vt_zerocrossings_h_
#define _vt_zerocrossings_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_seuil.h>

extern int  VT_ZeroCrossings( vt_image *r,  vt_image *t, DimType d );
extern int _VT_ZeroCrossings( vt_image *r,  vt_image *t, DimType d, int ze, int zo );

#ifdef __cplusplus
}
#endif

#endif
