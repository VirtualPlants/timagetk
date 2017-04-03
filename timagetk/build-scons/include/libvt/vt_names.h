#ifndef _vt_names_h_
#define _vt_names_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_typedefs.h>
#include <vt_unix.h>

typedef struct vt_names{
        char in[STRINGLENGTH];   /* name of the input image  */
        char out[STRINGLENGTH];  /* name of the output image */
        char ext[STRINGLENGTH];  /* name of an extra image   */
        typeBoolean inv;             /* if True, images are to be inversed */
        typeBoolean swap;            /* if True, images are to be swapped */
} vt_names;

extern void VT_PrintNames( FILE *theFile, vt_names *n );
extern void VT_Names( vt_names *n );
extern int  VT_CopyName( char *name1, char *name2 );

#ifdef __cplusplus
}
#endif

#endif /* _vt_names_h_ */
