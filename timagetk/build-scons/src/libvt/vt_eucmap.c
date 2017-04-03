
#include <math.h>

#include <vt_eucmap.h>

#define _LOCAL_NORME(X,Y,Z) ( (X)*(X) + (Y)*(Y) + (Z)*(Z) )





int VT_EucliDist( vt_image *resIm /* result image */, 
		  vt_image *theIm /* input image */, 
		  vt_distance *par /* parameters */ )
{
    vt_image auxX, auxY, auxZ;
    s16 ***ssx, ***ssy, ***ssz;
    s8  ***scx, ***scy, ***scz;
    register int x, y, z, n;

    if ( VT_Test2Image( resIm, theIm, "VT_EucliDist" ) == -1 ) return( -1 );
    if ( ( theIm->dim.x < 3 ) || ( theIm->dim.y < 3 ) ) {
	VT_Error( "images have bad dimensions", "VT_EucliDist" );
	return( -1 );
    }

    ssx = ssy = ssz = (s16***)NULL;
    scx = scy = scz = (s8***)NULL;

    switch ( par->type_image_eucmap ) {
    case SCHAR :
	/*--- allocations ---*/
	VT_InitImage( &auxX, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)SCHAR );
	VT_InitImage( &auxY, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)SCHAR );
	VT_InitImage( &auxZ, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)SCHAR );
	if ( VT_AllocImage( &auxX ) != 1 ) {
	    VT_Error("unable to allocate auxiliary image","VT_EucliDist");
	    return( -1 );
	}
	if ( VT_AllocImage( &auxY ) != 1 ) {
	    VT_Error("unable to allocate auxiliary image","VT_EucliDist");
	    VT_FreeImage( &auxX );
	    return( -1 );
	}
	if ( (resIm->type != UCHAR) && (resIm->type != SCHAR) ) {
	    if ( VT_AllocImage( &auxZ ) != 1 ) {
		VT_Error("unable to allocate auxiliary image","VT_EucliDist");
		VT_FreeImage( &auxX );
		VT_FreeImage( &auxY );
		return( -1 );
	    }
	}
	else {
	    auxZ.array = resIm->array;
	    auxZ.buf = resIm->buf;
	}

	/*--- calcul ---*/
	if ( VT_VecteurPPP_SC( theIm, &auxX, &auxY, &auxZ, par ) != 1 ) {
	    VT_FreeImage( &auxX );
	    VT_FreeImage( &auxY );
	    if ( (resIm->type != UCHAR) && (resIm->type != SCHAR) ) 
		VT_FreeImage( &auxZ );
	    return( -1 );
	}

	/********************************************************************/
	/*   le point le plus proche du point (x,y,z) est le point de       */
	/*   coordonnees                                                    */
	/*   ( x + auxX.buf[z*auxX.dim.y*auxX.dim.x + y*auxX.dim.x + x],    */
	/*     y + auxY.buf[z*auxX.dim.y*auxX.dim.x + y*auxX.dim.x + x],    */
	/*     z + auxZ.buf[z*auxX.dim.y*auxX.dim.x + y*auxX.dim.x + x] )   */
	/********************************************************************/

	/*---  ecriture de l'image de sortie ---*/
	scx = (s8***)(auxX.array);
	scy = (s8***)(auxY.array);
	scz = (s8***)(auxZ.array);

	switch ( resIm->type ) {
	case UCHAR :
	    {
	    u8 ***buf;
	    buf = (u8 ***)(resIm->array);
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    if ( n > 255 ) buf[z][y][x] = (u8)255;
		    else           buf[z][y][x] = (u8)(n);
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    n = (int)(sqrt((double)n) + 0.5);
		    if ( n > 255 ) buf[z][y][x] = (u8)255;
		    else           buf[z][y][x] = (u8)(n);
		}
	    }}
	    break;
	case USHORT :
	    {
	    u16 ***buf;
	    buf = (u16 ***)(resIm->array);
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    if ( n > 65535 ) buf[z][y][x] = (u16)65535;
		    else             buf[z][y][x] = (u16)(n);
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    n = (int)(sqrt((double)n) + 0.5);
		    if ( n > 65535 ) buf[z][y][x] = (u16)65535;
		    else             buf[z][y][x] = (u16)(n);
		}
	    }}
	    break;
	case FLOAT :
	    {
	    r32 ***buf;
	    buf = (r32 ***)(resIm->array);
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    buf[z][y][x] = (r32)(n);
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    buf[z][y][x] = (r32)(sqrt( (double)n ));
		}
	    }}
	    break;
	default :
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    _VT_SetVXYZvalue( resIm, (int)0, x, y, z, (double)n );
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)scx[z][y][x], (int)scy[z][y][x], (int)scz[z][y][x] );
		    _VT_SetVXYZvalue( resIm, (int)0, x, y, z, (double)sqrt((double)n) );
		}
	    }
	}
	/*--- liberation  de la memoire allouee ---*/
	VT_FreeImage( &auxX );
	VT_FreeImage( &auxY );
	if ( (resIm->type != UCHAR) && (resIm->type != SCHAR) )
	    VT_FreeImage( &auxZ );

	break;

    case SSHORT :
    default :
	/*--- allocations ---*/
	VT_InitImage( &auxX, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)SSHORT );
	VT_InitImage( &auxY, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)SSHORT );
	VT_InitImage( &auxZ, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)SSHORT );
	if ( VT_AllocImage( &auxX ) != 1 ) {
	    VT_Error("unable to allocate auxiliary image","VT_EucliDist");
	    return( -1 );
	}
	if ( VT_AllocImage( &auxY ) != 1 ) {
	    VT_Error("unable to allocate auxiliary image","VT_EucliDist");
	    VT_FreeImage( &auxX );
	    return( -1 );
	}
	if ( (resIm->type != USHORT) && (resIm->type != SSHORT) ) {
	    if ( VT_AllocImage( &auxZ ) != 1 ) {
		VT_Error("unable to allocate auxiliary image","VT_EucliDist");
		VT_FreeImage( &auxX );
		VT_FreeImage( &auxY );
		return( -1 );
	    }
	}
	else {
	    auxZ.array = resIm->array;
	    auxZ.buf = resIm->buf;
	}

	/*--- calcul ---*/
	if ( VT_VecteurPPP_SS( theIm, &auxX, &auxY, &auxZ, par ) != 1 ) {
	    VT_FreeImage( &auxX );
	    VT_FreeImage( &auxY );
	    if ( (resIm->type != USHORT) && (resIm->type != SSHORT) ) 
		VT_FreeImage( &auxZ );
	    return( -1 );
	}

	/*---  ecriture de l'image de sortie ---*/
	ssx = (s16***)(auxX.array);
	ssy = (s16***)(auxY.array);
	ssz = (s16***)(auxZ.array);

	switch ( resIm->type ) {
	case UCHAR :
	    {
	    u8 ***buf;
	    buf = (u8 ***)(resIm->array);
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    if ( n > 255 ) buf[z][y][x] = (u8)255;
		    else           buf[z][y][x] = (u8)(n);
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    n = (int)(sqrt((double)n) + 0.5);
		    if ( n > 255 ) buf[z][y][x] = (u8)255;
		    else           buf[z][y][x] = (u8)(n);
		}
	    }}
	    break;
	case USHORT :
	    {
	    u16 ***buf;
	    buf = (u16 ***)(resIm->array);
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    if ( n > 65535 ) buf[z][y][x] = (u16)65535;
		    else             buf[z][y][x] = (u16)(n);
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    n = (int)(sqrt((double)n) + 0.5);
		    if ( n > 65535 ) buf[z][y][x] = (u16)65535;
		    else             buf[z][y][x] = (u16)(n);
		}
	    }}
	    break;
	case FLOAT :
	    {
	    r32 ***buf;
	    buf = (r32 ***)(resIm->array);
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    buf[z][y][x] = (r32)(n);
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    buf[z][y][x] = (r32)(sqrt( (double)n ));
		}
	    }}
	    break;
	default :
	    if ( par->type == VT_DIST_EUCLI_2 ) {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    _VT_SetVXYZvalue( resIm, (int)0, x, y, z, (double)n );
		}
	    } else {
    for ( z = 0; z < (int)resIm->dim.z; z++ )
          for ( y = 0; y < (int)resIm->dim.y; y++ )
          for ( x = 0; x < (int)resIm->dim.x; x++ ) {
		    n = _LOCAL_NORME( (int)ssx[z][y][x], (int)ssy[z][y][x], (int)ssz[z][y][x] );
		    _VT_SetVXYZvalue( resIm, (int)0, x, y, z, (double)sqrt((double)n) );
		}
	    }
	}
	
	/*--- liberation  de la memoire allouee ---*/
	VT_FreeImage( &auxX );
	VT_FreeImage( &auxY );
	if ( (resIm->type != USHORT) && (resIm->type != SSHORT) )
	    VT_FreeImage( &auxZ );

    }
    return( 1 );
}





