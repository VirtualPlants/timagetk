
#ifndef __filters_h_
#define __filters_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void monimage;


extern int _gradient( const monimage *image, 
		      monimage *gradient,
		      float sigma );

extern int _gaussian( const monimage *image, 
		      monimage *smooth,
		      float sigma );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _recline_h_ */
