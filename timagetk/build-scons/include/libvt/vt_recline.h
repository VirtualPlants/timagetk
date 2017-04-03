#ifndef _vt_recline_h_
#define _vt_recline_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_recfilters.h>

extern int VT_InitRecursiveCoefficients( double x, int type_filter, int derivative );
extern int VT_RecFilterOnLine( double *t1, double *t2, double *work1, double *work2, int dim );

#ifdef __cplusplus
}
#endif


#endif /* _vt_recline_h_ */
