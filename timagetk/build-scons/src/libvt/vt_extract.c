/*************************************************************************
 * vt_extract.c -
 *
 * $Id: vt_extract.c,v 1.2 2000/04/11 17:15:24 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * ?
 *
 * ADDITIONS, CHANGES
 *
 * - Tue Apr 11 19:07:13 MET DST 2000, Gregoire Malandain
 *   propagation de la taille du voxel
 */


#include <vt_extract.h>

int VT_Extract( vt_image *im1  /* sub-image to be extracted */, 
		vt_image *im2  /* original image */,
		vt_ipt *corner /* origin of the block */ )
{
    vt_ipt local_corner;
    int dim_plane, dim_line, dim_beg_line, dim_mid_line, dim_end_line, size;
    char *theBuf, *resBuf;
    register int y, z;

    if ( VT_Test1Image( im1, "VT_Extract" ) == -1 ) return( -1 );
    if ( VT_Test1Image( im2, "VT_Extract" ) == -1 ) return( -1 );
    if ( im1->type != im2->type ) {
	VT_Error("Images have different types","VT_Extract");
	return( -1 );
    }

    /*--- origine et dimensions locales ---*/
    local_corner.x = corner->x;
    local_corner.y = corner->y;
    local_corner.z = corner->z;
    if ( local_corner.x < 0 ) local_corner.x = 0;
    if ( local_corner.y < 0 ) local_corner.y = 0;
    if ( local_corner.z < 0 ) local_corner.z = 0;
    if ( ((local_corner.x + im1->dim.x) > im2->dim.x) ||
	 ((local_corner.y + im1->dim.y) > im2->dim.y) ||
	 ((local_corner.z + im1->dim.z) > im2->dim.z) ) {
	VT_Error("Invalid block dimensions or origin","VT_Extract");
	return( -1 );
    }

    /*--- taille d'un point ---*/
    size = 0;
    switch ( im2->type ) {
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
    case SINT :
	size = sizeof(i32);
	break;
    case TYPE_UNKNOWN :
    default :
	VT_Error( "Image type unknown or not supported", "VT_Extract" );
	return( -1 );
    }
    
    /*--- dimensions ---*/
    dim_plane    = im2->dim.x * im2->dim.y * size;                     /* plan           : a enlever */
    dim_line     = im2->dim.x * size;                                  /* ligne entiere  : a enlever */
    dim_beg_line = local_corner.x * size;                              /* debut de ligne : a enlever */
    dim_end_line = (im2->dim.x - local_corner.x - im1->dim.x) * size;  /* fin de ligne   : a enlever */
    dim_mid_line = im1->dim.x * size;                                  /* milieu de ligne : a conserver */
    
    /*--- buffers ---*/
    theBuf = (char*)(im2->buf);
    resBuf = (char*)(im1->buf);

    /*--- lecture de l'image ---*/
    /*--- plans vides ---*/
    for ( z = 0; z < local_corner.z; z ++ ) 
	theBuf += dim_plane;

    /*--- plans pleins ---*/
    for ( z = 0; z < (int)im1->dim.z; z ++ ) {

	/*--- lignes vides ---*/
	theBuf += local_corner.y * dim_line;
	
	/*--- lignes pleines ---*/
  for ( y = 0; y < (int)im1->dim.y; y ++ ) {
	    /*--- debut de ligne ---*/
	    theBuf += dim_beg_line;
	    /*--- milieu de ligne ---*/
	    VT_Memcpy( resBuf, theBuf, dim_mid_line );
	    resBuf += dim_mid_line;
	    theBuf += dim_mid_line;
	    /*--- fin de ligne ---*/
	    theBuf += dim_end_line;
	}

	/*--- lignes vides ---*/
	theBuf += (im2->dim.y - local_corner.y - im1->dim.y) * dim_line;
    }

    return( 1 );
}


