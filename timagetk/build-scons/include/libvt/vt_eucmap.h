#ifndef _vt_eucmap_h_
#define _vt_eucmap_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

#include <vt_eucmapsc.h>
#include <vt_eucmapss.h>

#include <vt_distance.h>


extern int  VT_EucliDist( vt_image *resIm, vt_image *theIm, vt_distance *par );


#ifdef __cplusplus
}
#endif

#endif
