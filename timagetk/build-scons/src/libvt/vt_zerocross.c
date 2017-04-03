
#include <vt_zerocross.h>

#define VT_ZERO      0

#define VT_POSITIVE  1
#define VT_NEGATIVE -1

#define _VT_PLUS   (s8)1
#define _VT_MOINS  (s8)-1
#define _VT_PPLUS  (s8)127
#define _VT_MMOINS (s8)-128

#define _VT_NEGMARK( A ) ( (A) >  0 ? _VT_PLUS : _VT_MOINS )
#define _VT_POSMARK( A ) ( (A) >= 0 ? _VT_PLUS : _VT_MOINS )



/* Extraction des passages par zeros (avec valeurs par defaut)

   Les points a 0 sont consideres comme etant positifs.

   Les passages par zeros sont marques dans la zone negative.

 */

int VT_ZeroCrossings( vt_image *resIm /* result image */, 
		      vt_image *theIm /* input image */,
		      DimType dim /* dimension of the computation */ )
{
    if ( VT_Test2Image( resIm, theIm, "VT_ZeroCrossings" ) == -1 ) return( -1 );

    return( _VT_ZeroCrossings( resIm, theIm, dim, (int)VT_POSITIVE, (int)VT_NEGATIVE ) );
}



/* Extraction des passages par zeros (fonction generale)

   Si le champ zero vaut VT_POSITIVE, les points de valeur nulle
   sont consideres comme positifs.

   Si le champ zone vaut VT_POSITIVE, les passages par zeros sont marques 
   dans la zone positive.
 */

int _VT_ZeroCrossings( vt_image *resIm /* result image */, 
		       vt_image *theIm /* input image */,
		       DimType dim  /* dimension of the computation */,
		       int zero /* sign of zero */,
		       int zone /* where zero-crossings are to be marked */ )
{
    DimType local_dim;
    vt_image auxIm;
    s8 *buf, *b1, *b2;
    register int x, y, z, dx, dxy, dxyz;
    int dx1, dy1, dz1, dy2, dz2;

    if ( VT_Test2Image( resIm, theIm, "_VT_ZeroCrossings" ) == -1 ) return( -1 );

    /*--- parametre ---*/
    local_dim = 0;
    if ( theIm->dim.x > 1 )                           local_dim = VT_1D;
    if ( (local_dim == VT_1D) && (theIm->dim.y > 1) ) local_dim = VT_2D;
    if ( (local_dim == VT_2D) && (theIm->dim.z > 1) ) local_dim = VT_3D;

    if ( local_dim == 0 ) {
	VT_Error("unable to compute zero crossings","_VT_ZeroCrossings");
	return( -1 );
    }

    switch ( local_dim ) {
    default :
      VT_Error( "dimension noy handled in switch", "_VT_ZeroCrossings" );
      return( -1 );
    case VT_3D :
	if ( (dim == VT_3D) || (dim == VT_2D) || (dim == VT_1D) ) local_dim = dim;
	break;
    case VT_2D :
    case VT_1D :
	if ( (dim == VT_2D) || (dim == VT_1D) ) local_dim = dim;
    }

    /*--- image intermediaire ---*/
    VT_InitImage( &auxIm, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)SCHAR );
    if ( VT_AllocImage( &auxIm ) != 1 ) {
	VT_Error("unable to allocate auxiliary image","_VT_ZeroCrossings");
	return( -1 );
    }

    /*--- dimensions ---*/
    dx = theIm->dim.x;
    dxy = theIm->dim.x * theIm->dim.y;
    dxyz = theIm->dim.x * theIm->dim.y * theIm->dim.z;
    dx1 = theIm->dim.x - 1;
    dy1 = theIm->dim.y - 1;
    dz1 = theIm->dim.z - 1;
    dy2 = theIm->dim.y - 2;
    dz2 = theIm->dim.z - 2;

    /*--- construction de l'image intermediaire ---*/
    buf = (s8 *)(auxIm.buf);
    if ( zero >= VT_ZERO ) {
	switch( theIm->type ) {
	case SCHAR : 
	     {
	     s8 *b;
	     b = (s8 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_POSMARK( *b++ );
	     }
	     break;
	case UCHAR : 
	     {
	     u8 *b;
	     b = (u8 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_PLUS;
	     }
	     break;	
	case SSHORT : 
	     {
	     s16 *b;
	     b = (s16 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_POSMARK( *b++ );
	     }
	     break;
	case USHORT : 
	     {
	     u16 *b;
	     b = (u16 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_PLUS;
	     }
	     break;
	case SINT : 
	     {
	     i32 *b;
	     b = (i32 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_POSMARK( *b++ );
	     }
	     break;
	case FLOAT : 
	     {
	     r32 *b;
	     b = (r32 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_POSMARK( *b++ );
	     }
	     break;
	default :
	    VT_FreeImage( &auxIm );
	    VT_Error( "input image type unknown or not supported", "_VT_ZeroCrossings" );
	    return( -1 );
	}
    } else {
	switch( theIm->type ) {
	case SCHAR : 
	     {
	     s8 *b;
	     b = (s8 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_NEGMARK( *b++ );
	     }
	     break;
	case UCHAR : 
	     {
	     u8 *b;
	     b = (u8 *)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_NEGMARK( *b++ );
	     }
	     break;	
	case SSHORT : 
	     {
	     s16 *b;
	     b = (s16*)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_NEGMARK( *b++ );
	     }
	     break;
	case USHORT : 
	     {
	     u16 *b;
	     b = (u16*)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_NEGMARK( *b++ );
	     }
	     break;
	case SINT : 
	     {
	     i32 *b;
	     b = (i32*)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_NEGMARK( *b++ );
	     }
	     break;
	case FLOAT : 
	     {
	     r32 *b;
	     b = (r32*)(theIm->buf);
	     for ( x = 0; x < dxyz; x ++ ) *buf++ = _VT_NEGMARK( *b++ );
	     }
	     break;
	default :
	    VT_FreeImage( &auxIm );
	    VT_Error( "input image type unknown or not supported", "_VT_ZeroCrossings" );
	    return( -1 );
	}
    }

    /*--- determination des passages par zeros ---*/
    buf = (s8 *)(auxIm.buf);
    if ( zone >= VT_ZERO ) {
	/*--- on marque les passages par zeros dans la zone positive ---*/
	switch ( local_dim ) {
	default :
	  VT_Error( "dimension noy handled in switch", "_VT_ZeroCrossings" );
	  return( -1 );
	case VT_3D :
	    /*--- along Z : on traite d'abord les (n-1) premiers elements
	          en avancant le pointeur, puis le dernier sans avancer
		  (sinon INSIGHT trouve une erreur)                       ---*/
	    for ( x = 0; x < dxy; x ++ ) {
		b1 = buf + x;
		b2 = b1 + dxy;		
		for ( z = 0; z < dz2; z ++, b1 += dxy, b2 += dxy ) {
		    if ( *b1 > 0 ) {
			if ( *b2 < 0 ) *b1 = _VT_PPLUS;
		    } else {
			if ( *b2 > 0 ) *b2 = _VT_PPLUS;
		    }
		}
		if ( *b1 > 0 ) {
		    if ( *b2 < 0 ) *b1 = _VT_PPLUS;
		} else {
		    if ( *b2 > 0 ) *b2 = _VT_PPLUS;
		}
	    }
	case VT_2D :
	    /*--- along Y ---*/
	    for ( z = 0; z < dxyz; z += dxy )
	    for ( x = 0; x < dx; x ++ ) {
		b1 = buf + z + x;
		b2 = b1 + dx;
		for ( y = 0; y < dy2; y ++, b1 += dx, b2 += dx ) {
		    if ( *b1 > 0 ) {
			if ( *b2 < 0 ) *b1 = _VT_PPLUS;
		    } else {
			if ( *b2 > 0 ) *b2 = _VT_PPLUS;
		    }
		}
		if ( *b1 > 0 ) {
		    if ( *b2 < 0 ) *b1 = _VT_PPLUS;
		} else {
		    if ( *b2 > 0 ) *b2 = _VT_PPLUS;
		}
	    }
	case VT_1D :
	    /*--- along X ---*/
	    for ( y = 0; y < dxyz; y += dx ) {
		b1 = buf + y;
		b2 = b1 + 1;
		for ( x = 0; x < dx1; x ++, b1 ++, b2 ++ ) {
		    if ( *b1 > 0 ) {
			if ( *b2 < 0 ) *b1 = _VT_PPLUS;
		    } else {
			if ( *b2 > 0 ) *b2 = _VT_PPLUS;
		    }
		}
	    }
	}
    } else {
	/*--- on marque les passages par zeros dans la zone negative ---*/
	switch ( local_dim ) {
	default :
	  VT_Error( "dimension noy handled in switch", "_VT_ZeroCrossings" );
	  return( -1 );
	case VT_3D :
	    /*--- along Z ---*/
	    for ( x = 0; x < dxy; x ++ ) {
		b1 = buf + x;
		b2 = b1 + dxy;		
		for ( z = 0; z < dz2; z ++, b1 += dxy, b2 += dxy ) {
		    if ( *b1 > 0 ) {
			if ( *b2 < 0 ) *b2 = _VT_MMOINS;
		    } else {
			if ( *b2 > 0 ) *b1 = _VT_MMOINS;
		    }
		}
		if ( *b1 > 0 ) {
		    if ( *b2 < 0 ) *b2 = _VT_MMOINS;
		} else {
		    if ( *b2 > 0 ) *b1 = _VT_MMOINS;
		}
	    }
	case VT_2D :
	    /*--- along Y ---*/
	    for ( z = 0; z < dxyz; z += dxy )
	    for ( x = 0; x < dx; x ++ ) {
		b1 = buf + z + x;
		b2 = b1 + dx;
		for ( y = 0; y < dy2; y ++, b1 += dx, b2 += dx ) {
		    if ( *b1 > 0 ) {
			if ( *b2 < 0 ) *b2 = _VT_MMOINS;
		    } else {
			if ( *b2 > 0 ) *b1 = _VT_MMOINS;
		    }
		}
		if ( *b1 > 0 ) {
		    if ( *b2 < 0 ) *b2 = _VT_MMOINS;
		} else {
		    if ( *b2 > 0 ) *b1 = _VT_MMOINS;
		}
	    }
	case VT_1D :
	    /*--- along X ---*/
	    for ( y = 0; y < dxyz; y += dx ) {
		b1 = buf + y;
		b2 = b1 + 1;
		for ( x = 0; x < dx1; x ++, b1 ++, b2 ++ ) {
		    if ( *b1 > 0 ) {
			if ( *b2 < 0 ) *b2 = _VT_MMOINS;
		    } else {
			if ( *b2 > 0 ) *b1 = _VT_MMOINS;
		    }
		}
	    }
	}
    }

    /*--- un dernier passage sur l'image pour ne garder que les passages par zeros ---*/
    buf = (s8 *)(auxIm.buf);
    for ( x = 0; x < dxyz; x++, buf++ )
	*buf = ( (*buf == _VT_PLUS) || (*buf == _VT_MOINS) ) ? 0 : _VT_PPLUS;
    
    /*--- on seuille par procedure :
          ce n'est pas optimal, mais c'est plus propre ---*/
    if ( VT_Threshold( &auxIm, resIm, (float)(50.0) ) != 1 ) {
	VT_Error("unable to threshold auxiliary image","_VT_ZeroCrossings");
	VT_FreeImage( &auxIm );
	return( -1 );
    }

    VT_FreeImage( &auxIm );
    return( 1 );
}


