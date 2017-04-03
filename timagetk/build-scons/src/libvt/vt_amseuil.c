
#include <vt_amseuil.h>

#define VT_F2I( F ) ( (F) >= 0.0 ? ((int)((F)+0.5)) : ((int)((F)-0.5)) )





/* Procedures de seuillage pour l'amincissement :

   - procedure a 1 seuil : tous les points sont
     consideres comme pouvant etre effaces

   - procedure a 2 seuils : tous les points 
     - au-dessus du seuil bas sont consideres 
       comme pouvant etre effaces
     - au-dessus du seuil haut sont consideres 
       comme ne pouvant pas etre effaces
*/

/* Premiere procedure de seuillage pour l'amincissement.

   L'image resultat doit etre de type UCHAR.
   Les points dont la valeur est superieure ou egale au
   seuil sont mis a VT_DELETABLE, les autres a 0.

   Le passage du seuil float en int se fait en prenant
   l'entier le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)
*/

int _VT_ThinThreshold( vt_image *im1 /* image result */, 
		 vt_image *im2 /* image to be thresholded */,
		 float thres /* threshold */ )
{
	register u8 *b;
	register int size, v, is;

	if ( VT_Test2Image( im1, im2, "_VT_ThinThreshold" ) == -1 ) return( -1 );
	if ( im1->type != UCHAR ) {
		VT_Error( "incorrect image type", "_VT_ThinThreshold" );
		return( -1 );
	}
	is = VT_F2I( thres );
	size = im1->dim.z * im1->dim.y * im1->dim.x;
	b = (u8*)(im1->buf);
	switch ( im2->type ) {
	case SCHAR :
		{
		s8 *in;
		in = (s8*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			*b++ = ( v >= is ) ? (u8)VT_DELETABLE : (u8)0;
	        }}
		break;
	case UCHAR :
		{
		u8 *in;
		in = (u8*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			*b++ = ( v >= is ) ? (u8)VT_DELETABLE : (u8)0;
	        }}
		break;
	case SSHORT :
		{
		s16 *in;
		in = (s16*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			*b++ = ( v >= is ) ? (u8)VT_DELETABLE : (u8)0;
	        }}
		break;
	case USHORT :
		{
		u16 *in;
		in = (u16*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			*b++ = ( v >= is ) ? (u8)VT_DELETABLE : (u8)0;
	        }}
		break;
	case SINT :
		{
		i32 *in;
		in = (i32*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			*b++ = ( v >= is ) ? (u8)VT_DELETABLE : (u8)0;
	        }}
		break;
	case FLOAT :
		{
		r32 *in;
		in = (r32*)(im2->buf);
		while ( size-- > 0 ) {
			*b++ = ( *in++ >= thres ) ? (u8)VT_DELETABLE : (u8)0;
	        }}
		break;
	default :
	        for ( is = 0; is < size; is ++ ) {
		    *b++ = ( VT_GetINDvalue( im2, is ) >= thres ) ? (u8)VT_DELETABLE : (u8)0;
		}
	}
	return( 1 );
}





/* Seconde procedure de seuillage pour l'amincissement.

   L'image resultat doit etre de type UCHAR.
   Les points dont la valeur est 
   - strictement inferieure au premier seuil sont mis a 0,
   - superieure ou egale au premier seuil et strictement 
     inferieure au second seuil sont mis a VT_DELETABLE
   - superieure ou egale au second seuil sont mis a VT_UNDELETABLE

   Le passage du seuil float en int se fait en prenant
   l'entier le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)
*/

int _VT_Thin2Thresholds( vt_image *im1 /* image result */, 
			 vt_image *im2 /* image to be thresholded */,
			 float thres1 /* first threshold */,
			 float thres2 /* second threshold */ )
{
	register u8 *b;
	register int size, v, imin, imax;
	register float w, fmin, fmax;

	if ( VT_Test2Image( im1, im2, "_VT_Thin2Thresholds" ) == -1 ) return( -1 );
	if ( im1->type != UCHAR ) {
		VT_Error( "incorrect image type", "_VT_Thin2Thresholds" );
		return( -1 );
	}

	/*--- tests sur les seuils ---*/
	if ( thres1 > thres2 ) {
	    fmax = thres1;
	    fmin = thres2;
	} else if ( thres2 > thres1 ) {
	    fmax = thres2;
	    fmin = thres1;
	} else {
	    /*--- les deux seuils sont egaux ---*/
	    return( _VT_ThinThreshold( im1, im2, thres1 ) );
	}

	imin = VT_F2I( fmin );
	imax = VT_F2I( fmax );

	/*--- initialisations ---*/
	size = im1->dim.z * im1->dim.y * im1->dim.x;
	b = (u8*)(im1->buf);

	switch ( im2->type ) {
	case SCHAR :
		{
		s8 *in;
		in = (s8*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			if ( v < imin )      *b = (u8)0;
			else if ( v < imax ) *b = (u8)VT_DELETABLE;
			else                 *b = (u8)VT_UNDELETABLE;
			b ++;
	        }}
		break;
	case UCHAR :
		{
		u8 *in;
		in = (u8*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			if ( v < imin )      *b = (u8)0;
			else if ( v < imax ) *b = (u8)VT_DELETABLE;
			else                 *b = (u8)VT_UNDELETABLE;
			b ++;
	        }}
		break;
	case SSHORT :
		{
		s16 *in;
		in = (s16*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			if ( v < imin )      *b = (u8)0;
			else if ( v < imax ) *b = (u8)VT_DELETABLE;
			else                 *b = (u8)VT_UNDELETABLE;
			b ++;
	        }}
		break;
	case USHORT :
		{
		u16 *in;
		in = (u16*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			if ( v < imin )      *b = (u8)0;
			else if ( v < imax ) *b = (u8)VT_DELETABLE;
			else                 *b = (u8)VT_UNDELETABLE;
			b ++;
	        }}
		break;
	case SINT :
		{
		i32 *in;
		in = (i32*)(im2->buf);
		while ( size-- > 0 ) {
			v = (int)(*in++);
			if ( v < imin )      *b = (u8)0;
			else if ( v < imax ) *b = (u8)VT_DELETABLE;
			else                 *b = (u8)VT_UNDELETABLE;
			b ++;
	        }}
		break;
	case FLOAT :
		{
		r32 *in;
		in = (r32*)(im2->buf);
		while ( size-- > 0 ) {
		        w = *in++;
			if ( w < fmin )      *b = (u8)0;
			else if ( w < fmax ) *b = (u8)VT_DELETABLE;
			else                 *b = (u8)VT_UNDELETABLE;
			b ++;
	        }}
		break;
	default :
	        for ( imin = 0; imin < size; imin ++ ) {
		    w = (float)VT_GetINDvalue( im2, imin );
		    if ( w < fmin )      *b = (u8)0;
		    else if ( w < fmax ) *b = (u8)VT_DELETABLE;
		    else                 *b = (u8)VT_UNDELETABLE;
		    b ++;
		}
	}
	return( 1 );
}
