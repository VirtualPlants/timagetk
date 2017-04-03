/*************************************************************************
 * vt_bytes.c - 
 *
 * $Id: vt_bytes.c,v 1.3 2000/09/07 07:47:16 greg Exp $
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
 * * Fri Sep  1 11:01:05 MET DST 2000 (Gregoire Malandain)
 *   prise en compte de la dimension vectorielle dans
 *   VT_Swap2Octets() et VT_Swap4Octets().
 *
 */


#include <vt_bytes.h>





/* Procedure de swap d'une image 2 octets avec resultat dans la meme image.

   Ne realise le swap que si l'image est effectivement
   sur deux octets. Teste la validite de l'image.
*/

void VT_SwapImage( vt_image *image /* image to be swapped */ )
{
  switch ( image->type ) {
  case SSHORT :
  case USHORT :
    VT_Swap2Octets( image, image );
    VT_Message("Swap on 2 bytes","VT_SwapImage");
    break;
  case SINT :
  case FLOAT :
    VT_Swap4Octets( image, image ); 
    VT_Message("Swap on 4 bytes","VT_SwapImage");
    break;
  default :
    VT_Warning("Unable to swap such type of image","VT_SwapImage");
  }
}





/* Procedure d'inversion d'une image.

   Teste la validite de l'image.
*/

void VT_InverseImage( vt_image *image /* image to be inversed */ )
{
  int i;
  int v = image->dim.x * image->dim.y * image->dim.z;
  
  switch ( image->type ) {
  case UCHAR :
  case SCHAR :
  case USHORT :
  case SSHORT :
    VT_LogicInverse( image, image );
    break;
  case FLOAT :
    {
      float *tab = (float*)image->buf;
      for (i=0; i<v; i++) tab[i] = (-tab[i]);
    }
    break;
  case DOUBLE :
    {
      double *tab = (double*)image->buf;
      for (i=0; i<v; i++) tab[i] = (-tab[i]);
    }
    break;
  default :
    VT_Warning( "Unable to inverse such type of image", "VT_InverseImage" );
  }
    
}





/* Procedure de mise a jour d'une image.

   Teste la validite de l'image.
*/

void VT_ZeroImage( vt_image *image /* image to be filled with 0 */ )
{
  register int i, v;
  if ( VT_Test1Image( image, "VT_ZeroImage" ) == 0 ) {
    VT_Error("Unable to fill image with 0","VT_ZeroImage");
    return;
  }
  v = image->dim.z * image->dim.y * image->dim.x * image->dim.v;
  switch ( image->type ) {
  case UCHAR :
    {
      unsigned char *buf;
      buf = (unsigned char*)(image->buf);
      for ( i = v; i > 0; i -- )
	*buf++ = (unsigned char)0;
    }
    break;
  case SCHAR :
    {
      s8 *buf = (s8*)(image->buf);
      for ( i = v; i > 0; i -- ) *buf++ = (s8)0;
    }
    break;
  case USHORT :
    {
      u16 *buf = (u16*)(image->buf);
      for ( i = v; i > 0; i -- ) *buf++ = (u16)0;
    }
    break;
  case SSHORT :
    {
      s16 *buf = (s16*)(image->buf);
      for ( i = v; i > 0; i -- ) *buf++ = (s16)0;
    }
    break;
  default :
    for ( i = (v - 1); i >= 0; i -- )
      _VT_SetINDvalue( image, i, (double)0.0 );
  }
  return;
}





void VT_FillImage( vt_image *image /* image to be filled with 0 */,
		   double val )
{
  register int i, v;
  if ( VT_Test1Image( image, "VT_ZeroImage" ) == 0 ) {
    VT_Error("Unable to fill image with 0","VT_ZeroImage");
    return;
  }
  v = image->dim.z * image->dim.y * image->dim.x * image->dim.v;
  switch ( image->type ) {
  case UCHAR :
    {
      u8 uval, *buf = (unsigned char*)(image->buf);
      if ( val < 0 ) uval = 0;
      else if ( val > 255 ) uval = 255;
      else uval = (int) (val + 0.5);
      for ( i = v; i > 0; i -- )
	*buf++ = (unsigned char)uval;
    }
    break;
  case SCHAR :
    {
      s8 sval, *buf = (s8*)(image->buf);
      if ( val < -128 ) sval = -128;
      else if ( val > 127 ) sval = -127;
      else if ( val < 0 ) sval = (int) (val - 0.5);
      else sval = (int) (val + 0.5);
      for ( i = v; i > 0; i -- ) *buf++ = (s8)sval;
    }
    break;
  case USHORT :
    {
      u16 uval, *buf = (u16*)(image->buf);
      if ( val < 0 ) uval = 0;
      else if ( val > 65535 ) uval = 65535;
      else uval = (int) (val + 0.5);
      for ( i = v; i > 0; i -- ) *buf++ = (u16)uval;
    }
    break;
  case SSHORT :
    {
      s16 sval, *buf = (s16*)(image->buf);
      if ( val < -32768 ) sval = -32768;
      else if ( val > 32767 ) sval = -32767;
      else if ( val < 0 ) sval = (int) (val - 0.5);
      else sval = (int) (val + 0.5);
      for ( i = v; i > 0; i -- ) *buf++ = (s16)sval;
    }
    break;
  default :
    for ( i = (v - 1); i >= 0; i -- )
      _VT_SetINDvalue( image, i, (double)val );
  }
  return;
}












int _CPUtoCPU( vt_image *image )
{
    switch ( image->type ) {
    case SSHORT :
    case USHORT :
	/*--- swap eventuel ---*/
	switch ( MY_CPU ) {
	case LITTLEENDIAN :
	  switch ( image->cpu ) {
	  case BIGENDIAN :
	    VT_Swap2Octets( image, image ); break;
	  default :
	    break;
	  }
	  break;
	case BIGENDIAN :
	  switch ( image->cpu ) {
	  case LITTLEENDIAN :
	    VT_Swap2Octets( image, image ); break;
	  default :
	    break;
	  }
	  break;
	default :
	  VT_Warning("2 bytes image not swapped because of unknown CPU type","_CPUtoCPU");
	}
	break;
    case SINT :
    case FLOAT :
	/*--- swap eventuel ---*/
	switch ( MY_CPU ) {
	case LITTLEENDIAN :
	  switch ( image->cpu ) {
	  case BIGENDIAN :
	    VT_Swap4Octets( image, image ); break;
	  default :
	    break;
	  }
	  break;
	case BIGENDIAN :
	  switch ( image->cpu ) {
	  case LITTLEENDIAN :
	    VT_Swap4Octets( image, image ); break;
	  default :
	    break;
	  }
	  break;
	default :
	  VT_Warning("4 bytes image not swapped because of unknown CPU type","_CPUtoCPU");
	}
	break;
    case UCHAR :
    case SCHAR :
    default :
      image->cpu = MY_CPU;
      return( 1 );
    }
    image->cpu = MY_CPU;
    return( 1 );
}













/* Procedure de swap (echange de 2 octets).

   Ne realise le swap que si l'image est effectivement
   sur deux octets. Teste la validite des images.
*/

void VT_Swap2Octets( vt_image *im1 /* output image */,
		     vt_image *im2 /* input image */ )
{
    register unsigned short int *theBuf, si, *resBuf;
    register int nb;

    if ( VT_Test2Image( im1, im2, "VT_Swap2Octets" ) == 0 ) return;
    if ( im1->type != im2->type ) {
	VT_Error("images have different types","VT_Swap2Octets");
	return;
    }
    if ( (im1->type != SSHORT) && (im1->type != USHORT) ) {
	VT_Warning("Unable to swap such type of image","VT_Swap2Octets");
	return;
    }

    /*--- on swappe ---*/
    nb = im1->dim.v * im1->dim.x * im1->dim.y * im1->dim.z;

    theBuf = (unsigned short int*)(im2->buf);
    resBuf = (unsigned short int*)(im1->buf);
    while ( nb-- > 0 ) {
	si = *theBuf++;
	*resBuf++ = ((si >> 8) & 0xff) | (si << 8);
    }
}












/* Procedure de swap (echange de 4 octets).

   Ne realise le swap que si l'image est effectivement
   sur quatre octets. Teste la validite des images.
*/

void VT_Swap4Octets( vt_image *im1 /* output image */,
		     vt_image *im2 /* input image */ )
{
    register unsigned int *theBuf, i, *resBuf;
    register int nb;

    if ( VT_Test2Image( im1, im2, "VT_Swap4Octets" ) == 0 ) return;
    if ( im1->type != im2->type ) {
	VT_Error("images have different types","VT_Swap4Octets");
	return;
    }
    if ( (im1->type != SINT) && (im1->type != FLOAT) ) {
	VT_Warning("Unable to swap such type of image","VT_Swap4Octets");
	return;
    }

    /*--- on swappe ---*/
    nb = im1->dim.v * im1->dim.x * im1->dim.y * im1->dim.z;

    theBuf = (unsigned int*)(im2->buf);
    resBuf = (unsigned int*)(im1->buf);
    while ( nb-- > 0 ) {
	i = *theBuf++;
	*resBuf++ = (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | ((i >> 24) & 0xff);
    }
}





/* Inversion logique d'une image.

   Teste la validite des images.
*/

void VT_LogicInverse( vt_image *im1 /* output image */,
		      vt_image *im2 /* input image */ )
{
    register int i;
    register char *in, *out;

    if ( VT_Test2Image( im1, im2, "VT_LogicInverse" ) == 0 ) return;
    if ( im1->type != im2->type ) {
	VT_Error("images have different types","VT_LogicInverse");
	return;
    }

    i = VT_SizeImage( im2 );
    in = (char*)(im2->buf);
    out = (char*)(im1->buf);

    /*--- on inverse ---*/
    while ( i-- > 0 ) {
	*out++ = *in++ ^ 0xff;
    }
}





/* "OU" logique entre deux images.

   Teste la validite des images.
*/

void VT_LogicOu( vt_image *im1 /* input image */,
		 vt_image *im2 /* input image */,
		 vt_image *res /* output image */ )
{
    register int i;
    register char *out, *in1, *in2, a, b;

    if ( VT_Test2Image( im1, im2, "VT_LogicOu" ) == 0 ) return;
    if ( VT_Test2Image( im1, res, "VT_LogicOu" ) == 0 ) return;
    if ( (im1->type != im2->type) || (im1->type != res->type) ) {
	VT_Error("images have different types","VT_LogicOu");
	return;
    }

    i = VT_SizeImage( im1 );
    out = (char*)(res->buf);
    in1 = (char*)(im1->buf);
    in2 = (char*)(im2->buf);

    /*--- on  fait le OU logique ---*/
    while ( i-- > 0 ) {
	a = *in1++;
	b = *in2++;
	*out++ = a | b;
    }
}





/* "ET" logique entre deux images.

   Teste la validite des images.
*/

void VT_LogicEt( vt_image *im1 /* input image */,
		 vt_image *im2 /* input image */,
		 vt_image *res /* output image */ )
{
    register int i;
    register char *out, *in1, *in2, a, b;

    if ( VT_Test2Image( im1, im2, "VT_LogicEt" ) == 0 ) return;
    if ( VT_Test2Image( im1, res, "VT_LogicEt" ) == 0 ) return;
    if ( (im1->type != im2->type) || (im1->type != res->type) ) {
	VT_Error("images have different types","VT_LogicEt");
	return;
    }

    i = VT_SizeImage( im1 );
    out = (char*)(res->buf);
    in1 = (char*)(im1->buf);
    in2 = (char*)(im2->buf);

    /*--- on  fait le ET logique ---*/
    while ( i-- > 0 ) {
	a = *in1++;
	b = *in2++;
	*out++ = a & b;
    }
}





/* "XOU" logique entre deux images.

   Teste la validite des images.
*/

void VT_LogicXou( vt_image *im1 /* input image */,
		 vt_image *im2 /* input image */,
		 vt_image *res /* output image */ )
{
    register int i;
    register char *out, *in1, *in2, a, b;

    if ( VT_Test2Image( im1, im2, "VT_LogicXou" ) == 0 ) return;
    if ( VT_Test2Image( im1, res, "VT_LogicXou" ) == 0 ) return;
    if ( (im1->type != im2->type) || (im1->type != res->type) ) {
	VT_Error("images have different types","VT_LogicXou");
	return;
    }

    i = VT_SizeImage( im1 );
    out = (char*)(res->buf);
    in1 = (char*)(im1->buf);
    in2 = (char*)(im2->buf);

    /*--- on  fait le XOU logique ---*/
    while ( i-- > 0 ) {
	a = *in1++;
	b = *in2++;
	*out++ = a ^ b;
    }
}
