/*************************************************************************
 * vt_image.c - manipulation d'images
 *
 * $Id: vt_image.c,v 1.8 2006/04/14 08:39:32 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * it was before Wed Oct  6 16:23:43 MET DST 1999
 *
 * ADDITIONS, CHANGES
 *
 * * Thu May  4 10:30:41 MET DST 2000 (Gregoire Malandain)
 *   add DOUBLE type in VT_STest1Image(), VT_SizeImage(), VT_AllocImage()
 *
 */


#include <vt_image.h>

#if (defined(_ALPHA_) || defined(_LINUX_))
CpuType MY_CPU = LITTLEENDIAN;
#else
CpuType MY_CPU = BIGENDIAN;
#endif


/*------- Definition des fonctions statiques ----------*/


/* Initialisation d'une structure image.

   Les dimensions sont mises a 0, le type a
   VT_UNKNOWN, le buffer a (void*)NULL.
*/

void VT_Image( vt_image *image /* image to be initialized */ )
{
	register int i;
	for (i=0; i<STRINGLENGTH; i++) image->name[i] = '\0';
	image->type = TYPE_UNKNOWN;
	image->dim.v = 0;
	image->dim.x = image->dim.y = image->dim.z = 0;
	image->siz.x = image->siz.y = image->siz.z = 1.0;
	image->off.x = image->off.y = image->off.z = 0.0;
	image->rot.x = image->rot.y = image->rot.z = 0.0;
	image->ctr.x = image->ctr.y = image->ctr.z = 0;
	image->cpu = MY_CPU;
	image->array = (void***)NULL;
	image->buf = (void*)NULL;

	image->nuser = 0;
	image->user = (char**)NULL;
}





/* Initialisation du header d'une image a partir d'une autre image.

*/
void VT_InitFromImage( vt_image *image, vt_image *ref, char *name, int type )
{
  *image = *ref;
  if ( name != (char*)NULL && name[0] != '\0' ) {
    if ( VT_CopyName( image->name, name ) == 0 )
      VT_Error("unable to copy name into image header", "VT_InitFromImage");
  }
  image->type = type;
  image->cpu = MY_CPU;
  image->buf = (void*)NULL;
  image->array = (void***)NULL;
  image->nuser = 0;
  image->user = NULL;
}





/* Initialisation du header d'une structure image.

   La dimension vectorielle est 1.
   Pas de verification sur la validite du type.
*/

void VT_InitImage( vt_image *image /* image to be initialized */,
		  char *name /* image name */,
		  int dimx /* X dimension */,
		  int dimy /* Y dimension */,
		  int dimz /* Z dimension */,
		  int type /* image type  */ )
{
    VT_InitVImage( image, name, (int)1, dimx, dimy, dimz, type );
}





/* Initialisation du header d'une structure image.

   Pas de verification sur la validite du type.
*/

void VT_InitVImage( vt_image *image /* image to be initialized */,
		  char *name /* image name */,
		  int dimv /* V dimension */,
		  int dimx /* X dimension */,
		  int dimy /* Y dimension */,
		  int dimz /* Z dimension */,
		  int type /* image type  */ )
{
  VT_Image( image );
  if ( VT_CopyName( image->name, name ) == 0 )
    VT_Error("unable to copy name into image header", "VT_InitVImage");
  image->dim.v = dimv;
  image->dim.x = dimx;
  image->dim.y = dimy;
  image->dim.z = dimz;
  image->type = type;
  image->cpu = MY_CPU;
  image->buf = (void*)NULL;
  image->array = (void***)NULL;
}





void VT_SetImageOffset( vt_image *image,
			double x,
			double y,
			double z )
{
  image->off.x = (float)x;
  image->off.y = (float)y;
  image->off.z = (float)z;
}





void VT_SetImageRotation( vt_image *image,
			double x,
			double y,
			double z )
{
  image->rot.x = (float)x;
  image->rot.y = (float)y;
  image->rot.z = (float)z;
}





/* Taille du buffer d'une image.

RETURN
   Retourne 0 si les dimensions ou le type n'est pas valide.
*/
long int VT_SizeImage( vt_image *image /* image whose size is to be computed */ )
{
	char *local_name = "VT_SizeImage";
	long int volume, size;

	if ( VT_STest1Image( image, local_name ) == 0 )
		return( 0 );
	size = 0;
	switch ( image->type ) {
	case SCHAR :
	case UCHAR :
		size = sizeof(u8);
		break;
	case SSHORT :
	case USHORT :
		size = sizeof(u16);
		break;
	case FLOAT :
		size = sizeof(r32);
		break;
	case DOUBLE :
		size = sizeof(r64);
		break;
	case SINT :
	case UINT :
		size = sizeof(i32);
		break;
	case TYPE_UNKNOWN :
	default :
		VT_Error("image type unknown or not supported",local_name);
		return( 0 );
	}
	volume = image->dim.x * image->dim.y * image->dim.z * image->dim.v * size;
	if (volume <= 0) {
		VT_Error("image dimensions not valid",local_name);
		return( 0 );
	}
	return( volume );
}





/* Allocation d'une structure image.

RETURN 
   Retourne 0 en cas d'erreur.
*/

int VT_AllocImage( vt_image *image /* image to be allocated */ )
{
  char *local_name = "VT_AllocImage";

  if ( VT_STest1Image( image, local_name ) == 0 )
    return( 0 );
  
  if ( VT_AllocBufferImage( image ) != 1 ) {
    VT_Error("image buffer allocation failed",local_name);
    return( 0 );
  }

  if ( VT_AllocArrayImage( image ) != 1 ) {
    VT_Error("image array allocation failed",local_name);
    return( 0 );
  }

  return( 1 );
}





int VT_AllocArrayImage( vt_image *image /* image to be allocated */ )
{
  char *local_name = "VT_AllocArrayImage";
  unsigned long int vol_ptr;
  int i, j;

  if ( VT_STest1Image( image, local_name ) == 0 )
    return( 0 );
  
  vol_ptr = 0;

  /*--- calcul de la taille de l'allocation ---*/
  switch ( image->type ) {
  case SCHAR :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( s8* );
    vol_ptr += image->dim.z                * sizeof( s8** );
    break;
  case UCHAR :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( u8* );
    vol_ptr += image->dim.z                * sizeof( u8** );
    break;
  case SSHORT :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( s16* );
    vol_ptr += image->dim.z                * sizeof( s16** );
    break;
  case USHORT :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( u16* );
    vol_ptr += image->dim.z                * sizeof( u16** );
    break;
  case FLOAT :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( r32* );
    vol_ptr += image->dim.z                * sizeof( r32** );
    break;
  case DOUBLE :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( r64* );
    vol_ptr += image->dim.z                * sizeof( r64** );
    break;
  case SINT :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( i32* );
    vol_ptr += image->dim.z                * sizeof( i32** );
    break;
  case UINT :
    vol_ptr  = image->dim.z * image->dim.y * sizeof( u32* );
    vol_ptr += image->dim.z                * sizeof( u32** );
    break;
  default :
    VT_Error("unknown image type",local_name);
    return( 0 );
  }

  /*--- allocation ---*/
  image->array = VT_Malloc( (unsigned long int)(vol_ptr) );
  if (image->array == (void***)NULL) {
    VT_Error("private buffer allocation failed",local_name);
    return( 0 );
  }
  
  /*--- calcul des adresses ---*/
  switch ( image->type ) {
  case SCHAR :
    {
      s8 ***z;
      s8 **zy;
      s8 *zyx;
      
      z = (s8 ***)(image->array);
      zy = (s8 **)(z + image->dim.z);
      zyx = (s8 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  case UCHAR :
    {
      u8 ***z;
      u8 **zy;
      u8 *zyx;
      
      z = (u8 ***)(image->array);
      zy = (u8 **)(z + image->dim.z);
      zyx = (u8 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  case SSHORT :
    {
      s16 ***z;
      s16 **zy;
      s16 *zyx;
      
      z = (s16 ***)(image->array);
      zy = (s16 **)(z + image->dim.z);
      zyx = (s16 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  case USHORT :
    {
      u16 ***z;
      u16 **zy;
      u16 *zyx;
      
      z = (u16 ***)(image->array);
      zy = (u16 **)(z + image->dim.z);
      zyx = (u16 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  case FLOAT :
    {
      r32 ***z;
      r32 **zy;
      r32 *zyx;
      
      z = (r32 ***)(image->array);
      zy = (r32 **)(z + image->dim.z);
      zyx = (r32 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  case DOUBLE :
    {
      r64 ***z;
      r64 **zy;
      r64 *zyx;
      
      z = (r64 ***)(image->array);
      zy = (r64 **)(z + image->dim.z);
      zyx = (r64 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  case SINT :
    {
      i32 ***z;
      i32 **zy;
      i32 *zyx;
      
      z = (i32 ***)(image->array);
      zy = (i32 **)(z + image->dim.z);
      zyx = (i32 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  case UINT :
    {
      u32 ***z;
      u32 **zy;
      u32 *zyx;
      
      z = (u32 ***)(image->array);
      zy = (u32 **)(z + image->dim.z);
      zyx = (u32 *)(image->buf);
      
      /*--- calcul pour le premier indice Z ---*/
      for ( i = 0; i < (int)image->dim.z; i ++ ) {
	*z = zy;
	z ++;
	/*--- calcul pour le deuxieme indice Y ---*/
  for ( j = 0; j < (int)image->dim.y; j ++ ) {
	  *zy = zyx;
	  zy ++;
	  zyx += image->dim.x * image->dim.v;
	}
      }}
    break;
  default :
    VT_Error("unknown image type",local_name);
    return( 0 );
  }
  
  return( 1 );
}





int VT_AllocBufferImage( vt_image *image /* image to be allocated */ )
{
  char *local_name = "VT_AllocBufferImage";
  unsigned long int vol_ima;
  
  if ( VT_STest1Image( image, local_name ) == 0 )
    return( 0 );
  
  vol_ima = 0;
  
  /*--- calcul de la taille de l'image ---*/
  vol_ima = image->dim.z * image->dim.y * image->dim.x * image->dim.v;
  if ( vol_ima <= 0 ) {
    VT_Error("image dimensions not valid",local_name);
    return( 0 );
  }

  /*--- calcul de la taille de l'allocation ---*/
  switch ( image->type ) {
  case SCHAR :
    vol_ima *=  sizeof( s8 );
    break;
  case UCHAR :
    vol_ima *=  sizeof( u8 );
    break;
  case SSHORT :
    vol_ima *=  sizeof( s16 );
    break;
  case USHORT :
    vol_ima *=  sizeof( u16 );
    break;
  case FLOAT :
    vol_ima *=  sizeof( r32 );
    break;
  case DOUBLE :
    vol_ima *=  sizeof( r64 );
    break;
  case SINT :
    vol_ima *=  sizeof( i32 );
    break;
  case UINT :
    vol_ima *=  sizeof( u32 );
    break;
  default :
    VT_Error("unknown image type",local_name);
    return( 0 );
  }
  
  /*--- allocation ---*/
  image->buf = VT_Malloc( (unsigned long int)(vol_ima) );
  if (image->buf == (void*)NULL) {
    VT_Error("image buffer allocation failed",local_name);
    return( 0 );
  }

  memset( image->buf, 0, vol_ima );

  return( 1 );
}





/* Releasing the buffer of an image structure.

   Dimensions are set to 0, type to TYPE_UNKNOWN.
*/

void VT_FreeImage( vt_image *image )
{
  int i;

  if ( image == (vt_image*)NULL ) return;
  if ( image->array != (void***)NULL )
    VT_Free( (void**)&(image->array) );
  if ( image->buf != (void*)NULL )
    VT_Free( (void**)&(image->buf) );

  if ( image->nuser > 0 && image->user != NULL ) {
    for ( i=0; i<(int)image->nuser; i++ ) {
      if ( image->user[i] != NULL ) free( image->user[i] );
    }
    free( image->user );
  }

  VT_Image( image );
}




/* fait tout comme VT_FreeImage, mais ne libere pas la zone memoire
   des raw data
*/
void VT_ResetImage( vt_image *image )
{
  int i;

  if ( image == (vt_image*)NULL ) return;
  if ( image->array != (void***)NULL )
    VT_Free( (void**)&(image->array) );
  /*
  if ( image->buf != (void*)NULL )
    VT_Free( (void**)&(image->buf) );
  */
  if ( image->nuser > 0 && image->user != NULL ) {
    for ( i=0; i<(int)image->nuser; i++ ) {
      if ( image->user[i] != NULL ) free( image->user[i] );
    }
    free( image->user );
  }

  VT_Image( image );
}










/**************************************************
 *
 * 
 *
 **************************************************/

static void _PrintImageStatistics( FILE *f, vt_image *image )
{
  int i;
  int v = image->dim.x * image->dim.y * image->dim.z * image->dim.v;
  double m;

  if ( image->buf == NULL || v <= 0 ) return;

  switch ( image->type ) {
  default :                
    fprintf( f, "TYPE_UNKNOWN\n" );
    break;
  case UCHAR :  
    {
      unsigned char *buf = (unsigned char *)image->buf;
      unsigned char min, max;
      min = max = buf[0];
      m = buf[0];
      for ( i=1; i<v; i++ ) {
	if ( min > buf[i] ) min = buf[i];
	if ( max < buf[i] ) max = buf[i];
	m += buf[i];
      }
      fprintf( f, "  - min = %d , max = %d , mean = %f\n", min, max, m/(double)v );
    }
    break;
  case USHORT : 
    {
      unsigned short int *buf = (unsigned short int *)image->buf;
      unsigned short int min, max;
      min = max = buf[0];
      m = buf[0];
      for ( i=1; i<v; i++ ) {
	if ( min > buf[i] ) min = buf[i];
	if ( max < buf[i] ) max = buf[i];
	m += buf[i];
      }
      fprintf( f, "  - min = %d , max = %d , mean = %f\n", min, max, m/(double)v );
    }
    break;
  case SSHORT :   
    {
      short int *buf = (short int *)image->buf;
      short int min, max;
      min = max = buf[0];
      m = buf[0];
      for ( i=1; i<v; i++ ) {
	if ( min > buf[i] ) min = buf[i];
	if ( max < buf[i] ) max = buf[i];
	m += buf[i];
      }
      fprintf( f, "  - min = %d , max = %d , mean = %f\n", min, max, m/(double)v );
    }
    break;
  case FLOAT :
    {
      float *buf = (float *)image->buf;
      float min, max;
      min = max = buf[0];
      m = buf[0];
      for ( i=1; i<v; i++ ) {
	if ( min > buf[i] ) min = buf[i];
	if ( max < buf[i] ) max = buf[i];
	m += buf[i];
      }
      fprintf( f, "  - min = %f , max = %f , mean = %f\n", min, max, m/(double)v );
    }
    break;
  }  
}





void VT_PrintImage( FILE *f, vt_image *image, char *s )
{
  if ( s != (char *)NULL )
    fprintf( f, "'%s' information:\n", s );
  else {
    if ( image->name != (char *)NULL )
      fprintf( f, "'%s' information:\n", image->name );
    else
      fprintf( f, "image information:\n" );
  }
  
  fprintf( f, "  - image buffer: %p", image->buf );
  if ( image->buf == NULL ) fprintf( f, " (NULL)" );
  fprintf( f, "\n" );
  fprintf( f, "  - image array: %p", image->array );
  if ( image->array == NULL ) fprintf( f, " (NULL)" );
  fprintf( f, "\n" );


  fprintf( f, "  - dimensions [x y z] = %lu %lu %lu\n", 
	   image->dim.x, image->dim.y, image->dim.z ); 
  if ( image->dim.v > 1 )
    fprintf( f, "  - vectorial dimension [v] = %lu\n", image->dim.v );
  fprintf( f, "  - voxel size [x y z] = %f %f %f\n",
	   image->siz.x, image->siz.y, image->siz.z );
  fprintf( f, "  - image type is: " );
  switch ( image->type ) {
  default :     fprintf( f, "TYPE_UNKNOWN\n" ); break;
  case SCHAR :  fprintf( f, "SCHAR\n" ); break;
  case UCHAR :  fprintf( f, "UCHAR\n" ); break;
  case SSHORT : fprintf( f, "SSHORT\n" ); break;
  case USHORT : fprintf( f, "USHORT\n" ); break; 
  case UINT :   fprintf( f, "UINT\n" ); break; 
  case SINT :    fprintf( f, "INT\n" ); break;
  case ULINT :  fprintf( f, "ULINT\n" ); break;
  case FLOAT :  fprintf( f, "FLOAT\n" ); break;
  case DOUBLE : fprintf( f, "DOUBLE\n" ); break;
  }  

  _PrintImageStatistics( f, image );
}







/**************************************************
 *
 * TESTS
 *
 **************************************************/

/* Test d'une image.

   Cette fonction teste les dimensions et le type
   de l'image. Elle teste en outre si l'image est
   vectorielle (en cas, il y a erreur).
   Si il y a une erreur (dimensions nulles ou negatives, 
   type inconnu, buffer non alloue) et si _VT_DEBUG_ est 
   a 1, un message d'erreur est imprime sur stderr.

RETURN
   Retourne 0 en cas de probleme.
*/

int VT_Test1Image( vt_image *im /* image to be tested */, 
		  char *proc /* name of the calling procedure */ )
{
    if ( VT_STest1Image( im, proc ) == 0 ) return( 0 );
    
    if ( 0 && im->dim.v != 1 ) {
	VT_Error("unable to deal with vectorial image",proc);
	return( 0 );
    }
    if ( im->buf == (void*)NULL ) {
	VT_Error("image not allocated",proc);
	return( 0 );
    }
    return( 1 );
}





/* Test d'une image vectorielle.

   Cette fonction teste les dimensions et le type
   de l'image. Pas de test sur la dimension vectorielle.
   Si il y a une erreur (dimensions nulles ou negatives, 
   type inconnu, buffer non alloue) et si _VT_DEBUG_ est 
   a 1, un message d'erreur est imprime sur stderr.

RETURN
   Retourne 0 en cas de probleme.
*/

int VT_Test1VImage( vt_image *im /* image to be tested */, 
		  char *proc /* name of the calling procedure */ )
{
    if ( VT_STest1Image( im, proc ) == 0 ) return( 0 );
    if ( im->buf == (void*)NULL ) {
	VT_Error("image not allocated",proc);
	return( 0 );
    }
    return( 1 );
}





/* Test d'une image.

   Cette fonction teste les dimensions et le type
   de l'image. Si il y a une erreur (dimensions
   nulles ou negatives, type inconnu) et si
   _VT_DEBUG_ est a 1, un message d'erreur est imprime 
   sur stderr.

RETURN
   Retourne 0 en cas de probleme.
*/

int VT_STest1Image( vt_image *im /* image to be tested */, 
		  char *proc /* name of the calling procedure */ )
{
    if ( (im->dim.v < 1) ||
	 (im->dim.x < 1) ||
	 (im->dim.y < 1) ||
	 (im->dim.z < 1) ) {
	VT_Error("negative or null image dimension(s)", proc);
	return( 0 );
    }
    if ( (im->type != UCHAR) &&
	 (im->type != SCHAR) &&
	 (im->type != SSHORT) &&
	 (im->type != USHORT) &&
	 (im->type != SINT) &&
	 (im->type != UINT) &&
	 (im->type != FLOAT) &&
	 (im->type != DOUBLE) ) {
	VT_Error("image type unknown or not supported", proc);
	return( 0 );
    }
    return( 1 );
}





/* Test de 2 images.

   On teste si les dimensions sont valides et identiques,
   si les types sont valides et si les buffers sont
   alloues.

RETURN
   Retourne 0 en cas de probleme.
*/

int VT_Test2Image( vt_image *im1 /* first image to be tested */, 
		  vt_image *im2 /* second image to be tested */, 
		  char *proc /* name of the calling procedure */ )
{
    if ( VT_Test1Image( im1, proc ) == 0 ) return( 0 );
    if ( VT_Test1Image( im2, proc ) == 0 ) return( 0 );
    if ( (im1->dim.x != im2->dim.x) ||
	 (im1->dim.y != im2->dim.y) ||
	 (im1->dim.z != im2->dim.z) ) {
	VT_Error("incompatible dimensions of images", proc);
	return( 0 );
    }
    return( 1 );
}
