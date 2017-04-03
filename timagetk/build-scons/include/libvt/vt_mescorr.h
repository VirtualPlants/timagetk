#ifndef _vt_mescorr_h_
#define _vt_mescorr_h_



#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

typedef struct vt_mescorr{
	vt_ipt min; /* coordinate of the first computation block corner */
	vt_ipt max; /* coordinate of the second computation block corner */
	double a;   /* the computed approximation is : */
	double b;   /* i2 = a * i1 + b                 */
	double si1;   /* mean of i1 */
	double si2;   /* mean of i2 */
	double si12;  /* mean of i1*i1 */
	double si22;  /* mean of i2*i2 */
	double si1i2; /* mean of i1*i2 */
	double mean;  /* mean of distances to the approximation */
	double sigma; /* approximation error  = */
	              /* mean of squared distances to the approximation  */
	char name[STRINGLENGTH];
} vt_mescorr;

extern void VT_Mescorr( vt_mescorr *s );
extern int  VT_MesureCorrelation( vt_image *im1, vt_image *im2, vt_mescorr *par );



#ifdef __cplusplus
}
#endif

#endif /* _vt_mescorr_h_ */

