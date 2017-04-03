#ifndef _vt_error_h_
#define _vt_error_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <vt_typedefs.h>

extern typeBoolean _VT_DEBUG_;
extern typeBoolean _VT_VERBOSE_;

extern void VT_Error( char *mess, char *proc );
extern void VT_Warning( char *mess, char *proc );
extern void VT_Message( char *mess, char *proc );

#ifdef __cplusplus
}
#endif

#endif /* _vt_error_h_ */
