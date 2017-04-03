#ifndef _vt_cellfilter_h_
#define _vt_cellfilter_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <vt_image.h>

extern int removeExternalExtension( vt_image *image,
			     vt_image *opening,
			     int low_threshold,
			     int high_threshold,
			     int connectivity,
			     int cc_size );


#ifdef __cplusplus
}
#endif

#endif
