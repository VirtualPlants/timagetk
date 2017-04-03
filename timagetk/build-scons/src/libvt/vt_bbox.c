
#include <vt_bbox.h>

/*------- Definition des fonctions statiques ----------*/


/* Calcul de la boite englobante contenant un objet dans une image.

   Sont consideres comme points de l'objet ceux dont la valeur est
   differente de zero. La fonction retourne (si l'image n'est pas 
   nulle) le coin superieur gauche (min) et le coin inferieur droit
   (max) du cube contenant l'objet dans l'image.
   
RETURN
   -1 en cas d'erreur, 0 si l'image est partout nulle.
*/

int VT_BoundingBox( vt_image *theIm /* input image */, 
	vt_ipt *min /* left upper corner */, 
	vt_ipt *max /* right lower corner */ )
{
	register int v, x, y, z;
	int dv, dx, dy, dz, flag;

	if ( VT_Test1VImage( theIm, "VT_BoundingBox" ) == -1 ) return( -1 );
	min->x = min->y = min->z = 0;
	max->x = theIm->dim.x - 1;
	max->y = theIm->dim.y - 1;
	max->z = theIm->dim.z - 1;
	dx = theIm->dim.x;
	dy = theIm->dim.y;
	dz = theIm->dim.z;
	dv = theIm->dim.v;

	switch ( theIm->type ) {
	case UCHAR :
	    {
	    u8 ***b;
	    b = (u8***)(theIm->array);

	    if ( dz > 1 ) {
		/*--- computation of z min ---*/
		flag = 0;
		for ( z = 0; ( (z < dz) && (flag == 0) ); z++ )
		for ( y = 0; ( (y < dy) && (flag == 0) ); y++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ )
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ )
		    if ( b[z][y][x*dv + v] > 0 ) flag = 1;
		/*--- si l'image est vide ---*/
		if ( flag == 0 ) return( 0 );
		min->z = z - 1;
		/*--- computation of z max ---*/
		flag = 0;
		for ( z = dz-1; ( (z >= 0) && (flag == 0) ); z-- )
		for ( y = 0; ( (y < dy) && (flag == 0) ); y++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( b[z][y][x*dv + v] > 0 ) flag = 1;
		max->z = z + 1;
	    }

	    if ( dy > 1 ) {
		/*--- computation of y min ---*/
		flag = 0;
		for ( y = 0; ( (y < dy) && (flag == 0) ); y++ )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( b[z][y][x*dv + v] > 0 ) flag = 1;
		/*--- si l'image est vide ---*/
		if ( flag == 0 ) return( 0 );
		min->y = y - 1;
		/*--- computation of y max ---*/
		flag = 0;
		for ( y = dy-1; ( (y >= 0) && (flag == 0) ); y-- )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( b[z][y][x*dv + v] > 0 ) flag = 1;
		max->y = y + 1;
	    }

	    if ( dx > 1 ) {
		/*--- computation of x min ---*/
		flag = 0;
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( y = min->y; ( (y <= max->y) && (flag == 0) ); y++ )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( b[z][y][x*dv + v] > 0 ) flag = 1;
		/*--- si l'image est vide ---*/
		if ( flag == 0 ) return( 0 );
		min->x = x - 1;
		/*--- computation of x max ---*/
		flag = 0;
		for ( x = dx-1; ( (x >= 0) && (flag == 0) ); x-- ) 
		for ( y = min->y; ( (y <= max->y) && (flag == 0) ); y++ )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( b[z][y][x*dv + v] > 0 ) flag = 1;
		max->x = x + 1;
	    }}

	    break;

	default :

	    if ( dz > 1 ) {
		/*--- computation of z min ---*/
		flag = 0;
		for ( z = 0; ( (z < dz) && (flag == 0) ); z++ )
		for ( y = 0; ( (y < dy) && (flag == 0) ); y++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ )
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ )
		    if ( _VT_GetVXYZvalue( theIm, v, x, y, z ) != (double)0.0 ) flag = 1;
		/*--- si l'image est vide ---*/
		if ( flag == 0 ) return( 0 );
		min->z = z - 1;
		/*--- computation of z max ---*/
		flag = 0;
		for ( z = dz-1; ( (z >= 0) && (flag == 0) ); z-- )
		for ( y = 0; ( (y < dy) && (flag == 0) ); y++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( _VT_GetVXYZvalue( theIm, v, x, y, z ) != (double)0.0 ) flag = 1;
		max->z = z + 1;
	    }

	    if ( dy > 1 ) {
		/*--- computation of y min ---*/
		flag = 0;
		for ( y = 0; ( (y < dy) && (flag == 0) ); y++ )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( _VT_GetVXYZvalue( theIm, v, x, y, z ) != (double)0.0 ) flag = 1;
		/*--- si l'image est vide ---*/
		if ( flag == 0 ) return( 0 );
		min->y = y - 1;
		/*--- computation of y max ---*/
		flag = 0;
		for ( y = dy-1; ( (y >= 0) && (flag == 0) ); y-- )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( _VT_GetVXYZvalue( theIm, v, x, y, z ) != (double)0.0 ) flag = 1;
		max->y = y + 1;
	    }

	    if ( dx > 1 ) {
		/*--- computation of x min ---*/
		flag = 0;
		for ( x = 0; ( (x < dx) && (flag == 0) ); x++ ) 
		for ( y = min->y; ( (y <= max->y) && (flag == 0) ); y++ )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( _VT_GetVXYZvalue( theIm, v, x, y, z ) != (double)0.0 ) flag = 1;
		/*--- si l'image est vide ---*/
		if ( flag == 0 ) return( 0 );
		min->x = x - 1;
		/*--- computation of x max ---*/
		flag = 0;
		for ( x = dx-1; ( (x >= 0) && (flag == 0) ); x-- ) 
		for ( y = min->y; ( (y <= max->y) && (flag == 0) ); y++ )
		for ( z = min->z; ( (z <= max->z) && (flag == 0) ); z++ )
		for ( v = 0; ( (v < dv) && (flag == 0) ); v++ ) 
		    if ( _VT_GetVXYZvalue( theIm, v, x, y, z ) != (double)0.0 ) flag = 1;
		max->x = x + 1;
	    }

	}

	/*--- ce n'est pas la peine de faire un default               ---*/
        /*--- l'initialisation de min et de max est toujours correcte ---*/
	return( 1 );
}





/* Copie de la boite englobante d'une image avec ajout de marges.

   On calcule la boite englobante de l'objet dans la premiere
   image. La seconde image est allouee pour contenir
   cette boite plus des marges (la marge a droite
   est egale a la marge a gauche pour chaque 
   dimension). Les valeurs dans les marges sont mises
   a 0.

   La structure theMin contiendra le coin superieur
   gauche calcule dans l'image input.

   Attention, l'image resultat est allouee dans la
   procedure, il ne faudra pas oublier de la desallouer.

*/

int VT_CopyBBoxWithBorders( vt_image *theIm /* input image */,
			    vt_image *resIm /* output image */,
			    vt_ipt *border /* border dimensions */,
			    vt_ipt *theMin /* upper left corner */ )
{
    char *local_name = "VT_CopyBBoxWithBorders";
    vt_ipt min, max, dim;

    theMin->x = theMin->y = theMin->z = 0;
    if ( VT_Test1Image( theIm, local_name ) == -1 ) return( -1 );
    
    /*--- test des marges ---*/
    if ( (border->x < 0) || (border->y < 0) || (border->z < 0) ) {
	VT_Error( "border dimensions are not valid", local_name );
	return( -1 );
    }

    /*--- calcul de la boite englobante ---*/
    if ( VT_BoundingBox( theIm, &min, &max ) != 1 ) {
	VT_Error( "unable to compute bounding box", local_name );
	return( -1 );
    }

    /*--- dimensions ---*/
    dim.x = max.x - min.x + 1 + 2 * border->x;
    dim.y = max.y - min.y + 1 + 2 * border->y;
    dim.z = max.z - min.z + 1 + 2 * border->z;

    /*--- initialisation de l'image resultat ---*/
    VT_Image( resIm );
    VT_InitVImage( resIm, "", theIm->dim.v, dim.x, dim.y, dim.z, theIm->type );
    if ( VT_AllocImage( resIm ) != 1 ) {
	VT_Error( "unable to allocate result image", local_name );
	return( -1 );
    }

    /*--- dimensions ---*/
    dim.x = max.x - min.x + 1;
    dim.y = max.y - min.y + 1;
    dim.z = max.z - min.z + 1;

    /*--- image resultat ---*/
    VT_ZeroImage( resIm );
    if ( VT_CopyWindow( theIm, resIm, &min, border, &dim ) != 1 ) {
	VT_FreeImage( resIm );
	VT_Error( "unable to compute result image", local_name );
	return( -1 );
    }

    *theMin = min;
    return( 1 );
}
