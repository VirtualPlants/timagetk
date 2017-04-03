#ifndef _vt_histo_h_
#define _vt_histo_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

typedef struct vt_histo {
    char name[STRINGLENGTH];
    ImageType type;
    int size;
    int offset;
    u64 *buf;
    int min;
    int max;
    double moy;
} vt_histo;

extern void VT_PrintHisto( vt_histo *histo );


extern void VT_Histo( vt_histo *histo );
extern int  VT_AllocHisto( vt_histo *histo );
extern void VT_FreeHisto( vt_histo *histo );
extern int  VT_ComputeHisto( vt_histo *histo, vt_image *image );
int VT_ComputeMaskedHisto( vt_histo *histo /* histogram to be filled */,
			   vt_image *image /* input image */,
			   vt_image *mask );
extern int  VT_MMMHisto( vt_histo *histo );
extern int  VT_CumulativeHisto( vt_histo *cum, vt_histo *histo );


#ifdef __cplusplus
}
#endif

#endif /* _vt_histo_h_  */
