#ifndef _vt_histo_seuil_h_
#define _vt_histo_seuil_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_histo.h>

#include <seg_ment.h>
#include <seg_ment.ei>
#include <seg_ment.ee>


extern int VT_SeuilHisto( vt_histo *histo, int choix, int *seuil );

#ifdef __cplusplus
}
#endif

#endif /* _vt_histo_seuil_h_  */
