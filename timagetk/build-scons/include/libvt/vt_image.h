#ifndef _vt_image_h_
#define _vt_image_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_typedefs.h>

/* Les differentes representations liees a l'architecture.
 */
typedef enum {
  CPU_UNKNOWN = 0,
  LITTLEENDIAN = 1,
  BIGENDIAN = 2
} CpuType;

extern CpuType MY_CPU;

#define TXT_LENGTH 2048



/* Structure image.

   Cette structure contient les champs suivants :

   char name[STRINGLENGTH] : le nom de l'image
        ("<" = standard input, ">" = standard output)
	
   int type : le type de l'image

   vt_4vpt dim : les dimensions de l'image
           v = dimension vectorielle,
           x = dimension selon X,
           y = dimension selon Y,
           z = dimension selon Z.

   vt_4vpt private_dim : dimensions pre-calculees
           v = dim.v,
	   x = dim.v * dim.x,
	   y = dim.v * dim.x * dim.y,
	   z = dim.v * dim.x * dim.y * dim.z.

   int cpu : CPU sur lequel a ete cree l'image

   void ***array : buffer a 3 dimensions
        la valeur d'un point est
        ((cast***)(array))[z][y][x*dimv+v]

   void *buf : buffer a 1 dimension (non-inclus dans le precedent)
        la valeur d'un point est
        ((cast*)(buf))[v + x*dv + y*dv*dx + z*dv*dx*dy]
 */

typedef struct vt_image {
  char name[STRINGLENGTH];
  ImageType type;
  vt_4vsize dim; /* dimension of image */
  vt_fpt siz;  /* voxel size */
  vt_fpt off;  /* translation or offset */
  vt_fpt rot;  /* rotation */
  vt_ipt ctr;
  CpuType cpu;
  void ***array;
  void *buf;
  
  /** User defined strings array. The user can use any internal purpose string.
      Each string is written at then end of header after a '#' character. */
  char **user;
  /** Number of user defined strings */
  unsigned int nuser;


} vt_image;

#include <vt_unix.h>
#include <vt_error.h>
#include <vt_names.h>

extern void   VT_Image( vt_image *image );
extern void   VT_InitFromImage( vt_image *image, vt_image *ref, char *name, int type );
extern void   VT_InitImage( vt_image *i, char *n, int x, int y, int z, int t );

extern int    VT_AllocImage( vt_image *image );
extern int    VT_AllocArrayImage( vt_image *image);
extern int    VT_AllocBufferImage( vt_image *image);

extern void   VT_FreeImage( vt_image *image );
extern void   VT_ResetImage( vt_image *image );

extern void    VT_InitVImage( vt_image *i, char *n, int v, int x, int y, int z, int t );
extern void VT_SetImageOffset( vt_image *image,
			double x,
			double y,
			double z );

extern void VT_SetImageRotation( vt_image *image,
			double x,
			double y,
			double z );


extern long int     VT_SizeImage( vt_image *image );

extern void VT_PrintImage( FILE *f, vt_image *image, char *s );

extern int VT_Test1Image( vt_image *im, char *proc );
extern int VT_Test1VImage( vt_image *im, char *proc );
extern int VT_STest1Image( vt_image *im, char *proc );
extern int VT_Test2Image( vt_image *im1, vt_image *im2, char *proc );

#ifdef __cplusplus
}
#endif

#endif /* _vt_image_h_ */
