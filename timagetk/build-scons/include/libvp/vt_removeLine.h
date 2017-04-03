#ifndef _vt_removeLine_h_
#define _vt_removeLine_h_

#ifdef __cplusplus
extern "C" {
#endif



#include <vt_image.h>

extern void VT_SetVerboseInVtRemoveLine( int v );
extern void VT_IncrementVerboseInVtRemoveLine(  );
extern void VT_DecrementVerboseInVtRemoveLine(  );

extern int VP_RemoveLines( vt_image *theIm, vt_image *resIm, double coefficient );


#ifdef __cplusplus
}
#endif

#endif
