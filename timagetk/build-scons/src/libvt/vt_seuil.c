
#include <vt_seuil.h>

#define VT_F2I( F ) ( (F) >= 0.0 ? ((int)((F)+0.5)) : ((int)((F)-0.5)) )

/*------- Definition des fonctions statiques ----------*/
static int _VT_Seuil( vt_image *im1, vt_image *im2, float fs1, float fs2, int flag );
static int _VT_GreySeuil( vt_image *im1, vt_image *im2, float fs1, float fs2, int flag );






/* Procedure de seuillage a un seul seuil.

   Les points dont la valeur est superieure ou egale au
   seuil sont mis a
   * 127   pour les images codees en char
   * 255   pour les images codees en unsigned char
   * 32767 pour les images codees en signed short int
   * 65535 pour les images codees en unsigned short int
   * 2147483647 pour les images codees en int
   * 1.0 pour les images codees en float
   les autres sont mis a 0 (0.0).

   Le passage du seuil float en int se fait en prenant
   l'entier le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)

RETURN
   Retourne -1 si les images sont incompatibles, ou si
   elles ne sont pas allouees, ou si le type est inconnu.
*/

int VT_Threshold( vt_image *im1 /* image to be thresholded */,
		 vt_image *im2 /* image result */, 
		 float thres /* threshold */ )
{
	return( _VT_Seuil( im2, im1, thres, (float)0.0, (int)1 ) );
}





/* Procedure de seuillage a deux seuils.

   Les points dont la valeur est superieure ou egale au
   premier seuil et inferieure ou egale au second seuil 
   sont mis a
   * 255   pour les images codees en unsigned char
   * 32767 pour les images codees en signed short int
   * 65535 pour les images codees en unsigned short int
   * 2147483647 pour les images codees en int
   * 1.0 pour les images codees en float
   les autres sont mis a 0 (0.0).

   Le passage du seuil float en int se fait en prenant
   l'entier le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)

RETURN
   Retourne -1 si les images sont incompatibles, ou si
   elles ne sont pas allouees, ou si le type est inconnu.
*/

int VT_Threshold2( vt_image *im1 /* image to be thresholded */, 
		  vt_image *im2 /* image result */,
		  float thres1 /* first threshold */,
		  float thres2 /* second threshold */ )
{
	return( _VT_Seuil( im2, im1, thres1, thres2, (int)0 ) );
}





static int _VT_Seuil( vt_image *im1, 
		     vt_image *im2, 
		     float fs1, 
		     float fs2, 
		     int flag )
{
    int size, SliceIsAllocated = 0;
    u8 *pl = (u8*)NULL;
    register u8 *p;
    register int z, s, iv, is1, is2;
    register float fv;
    
    if ( VT_Test2Image( im1, im2, "_VT_Seuil" ) == -1 ) return( -1 );
    
    /*--- allocation d'un plan intermediaire ---*/
    size = im2->dim.x * im2->dim.y;
    if ( im1->type != UCHAR ) {
      pl = (u8*)VT_Malloc( (unsigned int)(size * sizeof(u8)) );
      if ( pl == (u8*)NULL ) {
	VT_Error( "allocation failed", "_VT_Seuil" );
	return( -1 );
      }
      SliceIsAllocated = 1;
    }

    is1 = VT_F2I( fs1 );
    is2 = VT_F2I( fs2 );

#define _THRESHOLD_INT( TYPE ) {                                       \
      TYPE *in = (TYPE*)(im2->buf);                                    \
      in += z * size;                                                  \
      if ( flag == (int)1 ) {                                    \
	while ( s-- > 0 ) {                                            \
	  if ( (int)(*in++) >= is1 ) *p++ = (u8)255;        \
	  else                       *p++ = (u8)0;          \
	}                                                              \
      } else {                                                         \
	while ( s-- > 0 ) {                                            \
	  iv = (int)(*in++);                                           \
	  if ( (iv >= is1) && (iv <= is2) ) *p++ = (u8)255; \
	  else                              *p++ = (u8)0;   \
	}                                                              \
      }                                                                \
}

    /*--- le seuillage se fait plan par plan ---*/
    for ( z = 0; z < (int)im2->dim.z; z ++ ) {
	/*--- on remplit le plan intermediaire ---*/
        if ( im1->type == UCHAR ) 
	  p = ((u8 *)(im1->buf)) + z * size;
	else 
	  p = pl;

	s = size;
	switch ( im2->type ) {
	case SCHAR :
	  _THRESHOLD_INT( s8 )
	  break;
	case UCHAR :
	  _THRESHOLD_INT( u8 )
	  break;
	case SSHORT :
	  _THRESHOLD_INT( s16 )
	  break;
	case USHORT :
	  _THRESHOLD_INT( u16 )
	  break;
	case SINT :
	  _THRESHOLD_INT( i32 )
	  break;
	case FLOAT :
	    {
	    r32 *in;
	    in = (r32 *)(im2->buf);
	    in += z * size;
	    while ( s-- > 0 ) {
		fv = (float)(*in++);
		if ( (fv >= fs1) && ( (flag == (int)1) || (fv <= fs2) ) )
		    *p++ = (unsigned char)255;
		else
		    *p++ = (unsigned char)0;
	    }}
	    break;
	case DOUBLE :
	    {
	    r64 *in;
	    in = (r64 *)(im2->buf);
	    in += z * size;
	    while ( s-- > 0 ) {
		fv = (float)(*in++);
		if ( (fv >= fs1) && ( (flag == (int)1) || (fv <= fs2) ) )
		    *p++ = (unsigned char)255;
		else
		    *p++ = (unsigned char)0;
	    }}
	    break;
	default :
	    VT_Error( "input image type unknown or not supported", "_VT_Seuil" );
	    if ( SliceIsAllocated == 1 ) VT_Free( (void**)&pl );
	    return( -1 );
	}

	/*--- on vide le plan intermediaire ---*/
	p = pl;
	s = size;
	switch ( im1->type ) {
	case SCHAR :
	    {
	    s8 *out;
	    out = (s8 *)(im1->buf);
	    out += z * size;
	    while ( s-- > 0 ) 
		*out++ = (char)( (*p++ == (unsigned char)0) ? 0 : 127 );
	    }
	    break;
	case UCHAR :
	    break;
	case SSHORT :
	    {
	    s16 *out;
	    out = (s16 *)(im1->buf);
	    out += z * size;
	    while ( s-- > 0 ) 
		*out++ = (short int)( (*p++ == (unsigned char)0) ? 0 : 32767 );
	    }
	    break;
	case USHORT :
	    {
	    u16 *out;
	    out = (u16 *)(im1->buf);
	    out += z * size;
	    while ( s-- > 0 ) 
		*out++ = (unsigned short int)( (*p++ == (unsigned char)0) ? 0 : 65535 );
	    }
	    break;
	case SINT :
	    {
	    i32  *out;
	    out = (i32 *)(im1->buf);
	    out += z * size;
	    while ( s-- > 0 ) 
		*out++ = (int)( (*p++ == (unsigned char)0) ? 0 : 2147483647 );
	    }
	    break;
	case FLOAT :
	    {
	    r32 *out;
	    out = (r32 *)(im1->buf);
	    out += z * size;
	    while ( s-- > 0 ) 
		*out++ = (float)( (*p++ == (unsigned char)0) ? 0.0 : 1.0 );
	    }
	    break;
	case DOUBLE :
	    {
	    r64 *out;
	    out = (r64 *)(im1->buf);
	    out += z * size;
	    while ( s-- > 0 ) 
		*out++ = (float)( (*p++ == (unsigned char)0) ? 0.0 : 1.0 );
	    }
	    break;
	default :
	    VT_Error( "output image type unknown or not supported", "_VT_Seuil" );
	    if ( SliceIsAllocated == 1 ) VT_Free( (void**)&pl );
	    return( -1 );
	}
    }
    if ( SliceIsAllocated == 1 ) VT_Free( (void**)&pl );
    return( 1 );
}





/* Procedure de seuillage a un seul seuil, on conserve les niveaux de gris.

   Les points dont la valeur est strictement inferieure au seuil
   sont mis a 0, les autres conservent leur valeur.
   Les images doivent avoir le meme type.
*/

int VT_GreyThreshold( vt_image *im1 /* image to be thresholded */,
		 vt_image *im2 /* image result */, 
		 float thres /* threshold */ )
{
    return( _VT_GreySeuil( im2, im1, thres, (float)0.0, (int)1 ) );
}





/* Procedure de seuillage a deux seuils, on conserve les niveaux de gris.

   Les points dont la valeur est strictement inferieure au 
   premier seuil ou strictement superieure au second seuil
   sont mis a 0, les autres conservent leur valeur.
   Les images doivent avoir le meme type.
*/

int VT_GreyThreshold2( vt_image *im1 /* image to be thresholded */,
		  vt_image *im2 /* image result */, 
		  float thres1 /* first threshold */,
		  float thres2 /* second threshold */ )
{
	return( _VT_GreySeuil( im2, im1, thres1, thres2, (int)0 ) );
}





static int _VT_GreySeuil( vt_image *im1, 
		     vt_image *im2, 
		     float fs1, 
		     float fs2, 
		     int flag )
{
    int size;
    register int i, iv, is1, is2;
    register float fv;

    if ( VT_Test2Image( im1, im2, "_VT_GreySeuil" ) == -1 ) return( -1 );
    if ( im1->type != im2->type ) {
	VT_Error( "images have different types", "_VT_GreySeuil" );
	return( -1 );
    }

    is1 = VT_F2I( fs1 );
    is2 = VT_F2I( fs2 );

    size = im1->dim.x * im1->dim.y * im1->dim.z;

    switch ( im1->type ) {
    case SCHAR :
        {
	s8 *in, *out;
	in = (s8 *)(im2->buf);
	out = (s8 *)(im1->buf);
	for ( i = 0; i < size; i ++ ) {
	    iv = (int)(*in++);
	    if ( (iv >= is1) && ( (flag == (int)1) || (iv <= is2) ) )
		*out++ = iv;
	    else
		*out++ = (s8)0;
	}}
	break;
    case UCHAR :
        {
	u8 *in, *out;
	in = (u8 *)(im2->buf);
	out = (u8 *)(im1->buf);
	for ( i = 0; i < size; i ++ ) {
	    iv = (int)(*in++);
	    if ( (iv >= is1) && ( (flag == (int)1) || (iv <= is2) ) )
		*out++ = iv;
	    else
		*out++ = (u8)0;
	}}
	break;
    case SSHORT :
        {
	s16 *in, *out;
	in = (s16 *)(im2->buf);
	out = (s16 *)(im1->buf);
	for ( i = 0; i < size; i ++ ) {
	    iv = (int)(*in++);
	    if ( (iv >= is1) && ( (flag == (int)1) || (iv <= is2) ) )
		*out++ = iv;
	    else
		*out++ = (s16)0;
	}}
	break;
    case USHORT :
        {
	u16 *in, *out;
	in = (u16 *)(im2->buf);
	out = (u16 *)(im1->buf);
	for ( i = 0; i < size; i ++ ) {
	    iv = (int)(*in++);
	    if ( (iv >= is1) && ( (flag == (int)1) || (iv <= is2) ) )
		*out++ = iv;
	    else
		*out++ = (u16)0;
	}}
	break;
    case SINT :
        {
	i32 *in, *out;
	in = (i32 *)(im2->buf);
	out = (i32 *)(im1->buf);
	for ( i = 0; i < size; i ++ ) {
	    iv = (int)(*in++);
	    if ( (iv >= is1) && ( (flag == (int)1) || (iv <= is2) ) )
		*out++ = iv;
	    else
		*out++ = (i32)0;
	}}
	break;
    case FLOAT :
        {
	r32 *in, *out;
	in = (r32 *)(im2->buf);
	out = (r32 *)(im1->buf);
	for ( i = 0; i < size; i ++ ) {
	    fv = (int)(*in++);
	    if ( (fv >= fs1) && ( (flag == (int)1) || (fv <= fs2) ) )
		*out++ = fv;
	    else
		*out++ = (r32)0.0;
	}}
	break;
    case DOUBLE :
        {
	r64 *in, *out;
	in = (r64 *)(im2->buf);
	out = (r64 *)(im1->buf);
	for ( i = 0; i < size; i ++ ) {
	    fv = (int)(*in++);
	    if ( (fv >= fs1) && ( (flag == (int)1) || (fv <= fs2) ) )
		*out++ = fv;
	    else
		*out++ = (r64)0.0;
	}}
	break;
    default :
	    VT_Error( "input image type unknown or not supported", "_VT_GreySeuil" );
	    return( -1 );
	}

    return( 1 );
}
