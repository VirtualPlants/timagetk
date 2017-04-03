
#include <math.h>
#include <vt_filters.h>

typedef struct _vt_RGcoeff {
    double B;
    double Bd;
    double b0;
    double b1;
    double b2;
    double b3;
} _vt_RGcoeff;

/*------ Static Functions ------*/
static void _VT_ComputeRGCoefficients( _vt_RGcoeff *c, double s );
static void _VT_ComputeRGLine( double *in, double *out, _vt_RGcoeff *c, int d, int o );





/* Filtrage : valeurs par defaut pour la structure de parametres.

   Les 3 champs (x, y et z) du champ coeff_filter sont mis a 1.0;
   Les 3 champs (x, y et z) du champ order_filter sont mis a -1
   (pas de filtrage).
   Les 3 champs (x, y et z) du champ type_bord sont mis a 0.
   Les 3 champs (x, y et z) du champ long_bord sont mis a 0.
*/



void VT_Filters( vt_filters *par /* parameters */ )
{
    par->coeff_filter.x = par->coeff_filter.y = par->coeff_filter.z = 1.0;
    par->order_filter.x = par->order_filter.y = par->order_filter.z = -1;
    par->type_bord.x = par->type_bord.y = par->type_bord.z = 0;
    par->long_bord.x = par->long_bord.y = par->long_bord.z = 0;
}





/* Convolution par un filtre gaussien (implementation recursive)

   Tire de l'article :
   "Recursive Gaussian Filtering"
   Ian T. Young and Lucas J. van Vliet
   9th Scandinavian Conference on Image Analysis
   Uppsala, Sweden, June 1995
   pages 333-341
*/

int VT_RecursiveGaussian( vt_image *resIm /* result image */, 
			  vt_image *theIm /* input image */, 
			  vt_filters *par /* parameters */ )
{
    vt_image auxIm;
    vt_filters local_par;
    vt_ipt local_dim;
    register float *bufIN, *bufOUT;
    double *aux1, *aux2, *first, *last;
    register double *db1, *db2;
    int bool_needaux, maxdim, dxy, dxyz, dy1, dz1;
    _vt_RGcoeff local_coeff;
    register int x, y, z;

    if ( VT_Test2Image( resIm, theIm, "VT_RecursiveGaussian" ) == -1 ) return( -1 );

    /*--- parametres : copie et verification ---*/
    local_par = *par;
    if ( local_par.long_bord.x < 0 ) local_par.long_bord.x = 0;
    if ( local_par.long_bord.y < 0 ) local_par.long_bord.y = 0;
    if ( local_par.long_bord.z < 0 ) local_par.long_bord.z = 0;
    local_dim.x = theIm->dim.x + 2 * local_par.long_bord.x;
    local_dim.y = theIm->dim.y + 2 * local_par.long_bord.y;
    local_dim.z = theIm->dim.z + 2 * local_par.long_bord.z;

    if ( (local_par.order_filter.x >= 0) && (local_par.order_filter.x <= 3) ) {
	if ( local_dim.x <= 4 ) {
	    VT_Error("too small X dimension for recursive filtering","VT_RecursiveGaussian");
	    return( -1 );
	}
	if ( local_par.coeff_filter.x < 0.5 ) {
	    VT_Error("too small sigma along X dimension","VT_RecursiveGaussian");
	    return( -1 );
	}
    }
    if ( (local_par.order_filter.y >= 0) && (local_par.order_filter.y <= 3) ) {
	if ( local_dim.y <= 4 ) {
	    VT_Error("too small Y dimension for recursive filtering","VT_RecursiveGaussian");
	    return( -1 );
	}
	if ( local_par.coeff_filter.y < 0.5 ) {
	    VT_Error("too small sigma along Y dimension","VT_RecursiveGaussian");
	    return( -1 );
	}
    }
    if ( (local_par.order_filter.z >= 0) && (local_par.order_filter.z <= 3) ) {
	if ( local_dim.z <= 4 ) {
	    VT_Error("too small Z dimension for recursive filtering","VT_RecursiveGaussian");
	    return( -1 );
	}
	if ( local_par.coeff_filter.z < 0.5 ) {
	    VT_Error("too small sigma along Z dimension","VT_RecursiveGaussian");
	    return( -1 );
	}
    }
	 
    /*--- coefficients pour le filtrage ---*/
    local_coeff.B = local_coeff.Bd = 0.0;
    local_coeff.b0 = local_coeff.b1 = local_coeff.b2 = local_coeff.b3 = 0.0;

    /*--- image intermediaire ---*/
    VT_InitImage( &auxIm, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)FLOAT );
    bool_needaux = 1;
    if ( ( resIm->type == FLOAT ) && ( resIm->buf != theIm->buf ) ) {
	bool_needaux = 0;
	auxIm.buf = resIm->buf;
    } else {
	if ( VT_AllocImage( &auxIm ) != 1 ) {
	    VT_Error("unable to allocate auxiliary image","VT_RecursiveGaussian");
	    return( -1 );
	}
    }
    if ( VT_CopyImage( theIm, &auxIm ) != 1 ) {
	VT_Error("unable to copy input image","VT_RecursiveGaussian");
	if ( bool_needaux == 1 ) VT_FreeImage( &auxIm );
	return( -1 );
    }

    /*--- tableaux de travail ---*/
    bufIN = bufOUT = (float*)NULL;

    dxy = theIm->dim.x * theIm->dim.y;
    dxyz = theIm->dim.x * theIm->dim.y * theIm->dim.z;
    dy1 = theIm->dim.y - 1;
    dz1 = theIm->dim.z - 1;

    maxdim = local_dim.x;
    if ( local_dim.y > maxdim ) maxdim = local_dim.y;
    if ( local_dim.z > maxdim ) maxdim = local_dim.z;
    aux1 = (double*)VT_Malloc( (unsigned int)(maxdim * sizeof(double)) );
    if ( aux1 == (double*)NULL ) {
	VT_Error("unable to allocate first auxiliary array","VT_RecursiveGaussian");
	if ( bool_needaux == 1 ) VT_FreeImage( &auxIm );
	return( -1 );
    }
    aux2 = (double*)VT_Malloc( (unsigned int)(maxdim * sizeof(double)) );
    if ( aux2 == (double*)NULL ) {
	VT_Error("unable to allocate first auxiliary array","VT_RecursiveGaussian");
	VT_Free( (void**)(&aux1) );
	if ( bool_needaux == 1 ) VT_FreeImage( &auxIm );
	return( -1 );
    }

    /*--- filtrage selon X ---*/
    if ( (local_par.order_filter.x >= 0) && (local_par.order_filter.x <= 3) ) {

	/*--- coefficients ---*/
	_VT_ComputeRGCoefficients( &local_coeff, (double)local_par.coeff_filter.x );
	
	/*--- tableaux ---*/
	first = aux1 + local_par.long_bord.x;
	last  = aux1 + (local_par.long_bord.x + theIm->dim.x - 1);
	bufIN = bufOUT = (float*)(auxIm.buf);

	/*--- traitement ----*/
  for ( z = 0; z < (int)theIm->dim.z ; z++ )
        for ( y = 0; y < (int)theIm->dim.y ; y++ ) {

	    /*--- copie du tableau ---*/
	    db1 = first;
      for ( x = 0; x < (int)theIm->dim.x ; x++ ) *db1++ = (double)*bufIN++;

	    /*--- les bords ? ---*/
	    if ( local_par.long_bord.x > 0 ) {
		db1 = first;   db2 = last;
		switch( local_par.type_bord.x ) {
		case _VT_BORD_CONT :
		default :
		    for ( x = 0; x < local_par.long_bord.x ; x++ ) {
			*(--db1) = *first;
			*(++db2) = *last;
		    }
		}
	    }

	    /*--- calcul de la ligne ---*/
	    _VT_ComputeRGLine( aux1, aux2, &local_coeff, local_dim.x, local_par.order_filter.x );
	    
	    /*--- copie de la ligne ---*/
	    db1 = first;
      for ( x = 0; x < (int)theIm->dim.x ; x++ ) *bufOUT++ = (float)*db1++;
	}	
    }

    /*--- filtrage selon Y ---*/
    if ( (local_par.order_filter.y >= 0) && (local_par.order_filter.y <= 3) ) {

	/*--- coefficients ---*/
	_VT_ComputeRGCoefficients( &local_coeff, (double)local_par.coeff_filter.y );
	
	/*--- tableaux ---*/
	first = aux1 + local_par.long_bord.y;
	last  = aux1 + (local_par.long_bord.y + theIm->dim.y - 1);

	/*--- traitement ----*/
	for ( z = 0; z < dxyz ; z += dxy )
        for ( x = 0; x < (int)theIm->dim.x ; x++ ) {
	    bufIN = bufOUT = (float*)(auxIm.buf) + z + x;

	    /*--- copie du tableau :
	          on recopie d'abord les (n-1) premiers elements du tableau
		  avec un deplacement, puis on copie le dernier sans faire
		  le deplacement (sinon cela donne une erreur sous INSIGHT) ---*/
	    db1 = first;
	    for ( y = 0; y < dy1 ; y++ ) {
		*db1++ = (double)*bufIN;
		bufIN += theIm->dim.x;
	    }
	    *db1 = (double)*bufIN;

	    /*--- les bords ? ---*/
	    if ( local_par.long_bord.y > 0 ) {
		db1 = first;   db2 = last;
		switch( local_par.type_bord.y ) {
		case _VT_BORD_CONT :
		default :
		    for ( y = 0; y < local_par.long_bord.y ; y++ ) {
			*(--db1) = *first;
			*(++db2) = *last;
		    }
		}
	    }

	    /*--- calcul de la ligne ---*/
	    _VT_ComputeRGLine( aux1, aux2, &local_coeff, local_dim.y, local_par.order_filter.y );
	    
	    /*--- copie de la ligne ---*/
	    db1 = first;
	    for ( y = 0; y < dy1 ; y++ ) {
		*bufOUT = (float)*db1++;
		bufOUT += theIm->dim.x;
	    }
	    *bufOUT = (float)*db1;
	}	
    }

    /*--- filtrage selon Z ---*/
    if ( (local_par.order_filter.z >= 0) && (local_par.order_filter.z <= 3) ) {

	/*--- coefficients ---*/
	_VT_ComputeRGCoefficients( &local_coeff, (double)local_par.coeff_filter.z );
	
	/*--- tableaux ---*/
	first = aux1 + local_par.long_bord.z;
	last  = aux1 + (local_par.long_bord.z + theIm->dim.z - 1);

	/*--- traitement ----*/
        for ( x = 0; x < dxy ; x++ ) {
	    bufIN = bufOUT = (float*)(auxIm.buf) + x;

	    /*--- copie du tableau :
	          on recopie d'abord les (n-1) premiers elements du tableau
		  avec un deplacement, puis on copie le dernier sans faire
		  le deplacement (sinon cela donne une erreur sous INSIGHT) ---*/
	    db1 = first;
	    for ( z = 0; z < dz1 ; z++ ) {
		*db1++ = (double)*bufIN;
		bufIN += dxy;
	    }
	    *db1 = (double)*bufIN;

	    /*--- les bords ? ---*/
	    if ( local_par.long_bord.z > 0 ) {
		db1 = first;   db2 = last;
		switch( local_par.type_bord.z ) {
		case _VT_BORD_CONT :
		default :
		    for ( z = 0; z < local_par.long_bord.z ; z++ ) {
			*(--db1) = *first;
			*(++db2) = *last;
		    }
		}
	    }

	    /*--- calcul de la ligne ---*/
	    _VT_ComputeRGLine( aux1, aux2, &local_coeff, local_dim.z, local_par.order_filter.z );
	    
	    /*--- copie de la ligne ---*/
	    db1 = first;
	    for ( z = 0; z < dz1 ; z++ ) {
		*bufOUT = (float)*db1++;
		bufOUT += dxy;
	    }
	    *bufOUT = (float)*db1;
	}	
    }

    /*--- retour ---*/
    if ( VT_CopyImage( &auxIm, resIm ) != 1 ) {
	VT_Error("unable to copy output image","VT_RecursiveGaussian");	
	VT_Free( (void**)(&aux1) );
	VT_Free( (void**)(&aux2) );
	if ( bool_needaux == 1 ) VT_FreeImage( &auxIm );
	return( -1 );
    }

    VT_Free( (void**)(&aux1) );
    VT_Free( (void**)(&aux2) );
    if ( bool_needaux == 1 ) VT_FreeImage( &auxIm );
    return( 1 );
}





static void _VT_ComputeRGCoefficients( _vt_RGcoeff *c, double s )
{
    double q;

    /*--- coefficients ---*/
    if ( s <= 2.5 ) 
	q = 3.97156 - 4.14554 * sqrt( (double)( 1.0 - 0.26891 * s ) );
    else 
	q = 0.98711 * s - 0.96330;
    
    c->b0 = 1.57825 + 2.44413 * q + 1.4281 * q * q + 0.422205 * q * q * q;
    c->b1 = 2.44413 * q + 2.85619 * q * q + 1.26661 * q * q * q;
    c->b2 = -( 1.4281 * q * q + 1.26661 * q * q * q );
    c->b3 = 0.422205 * q * q * q;
    c->B = 1.0 - ( c->b1 + c->b2 + c->b3 ) / c->b0;
    
    c->b1 /= c->b0;
    c->b2 /= c->b0;
    c->b3 /= c->b0;
    c->Bd = c->B / 2.0;
}
    




static void _VT_ComputeRGLine( double *in, double *out, _vt_RGcoeff *c, int dim, int order )
{
    register int i;
    
    switch ( order ) {
    case 0 :
	/*--- forward ---*/
	out[0] = c->B * in[0];
	out[1] = c->B * in[1] + c->b1 * out[0];
	out[2] = c->B * in[2] + c->b1 * out[1] + c->b2 * out[0];
	for ( i = 3; i < dim; i++ )
	    out[i] = c->B * in[i] + c->b1 * out[i-1] + c->b2 * out[i-2] + c->b3 * out[i-3];
	/*--- backward ---*/
	in[dim-1] = c->B * out[dim-1];
	in[dim-2] = c->B * out[dim-2] + c->b1 * in[dim-1];
	in[dim-3] = c->B * out[dim-3] + c->b1 * in[dim-2] + c->b2 * in[dim-1];
	for ( i = dim-4; i >= 0; i-- )
	    in[i] = c->B * out[i] + c->b1 * in[i+1] + c->b2 * in[i+2] + c->b3 * in[i+3];
	break;
    case 1 :
	/*--- forward ---*/
	out[0] = c->Bd * in[1];
	out[1] = c->Bd * (in[2] - in[0]) + c->b1 * out[0];
	out[2] = c->Bd * (in[3] - in[1]) + c->b1 * out[1] + c->b2 * out[0];
	for ( i = 3; i < dim-1; i++ )
	    out[i] = c->Bd * (in[i+1] - in[i-1]) + c->b1 * out[i-1] + c->b2 * out[i-2] + c->b3 * out[i-3];
	out[dim-1] = - c->Bd * in[dim-2] + c->b1 * out[dim-2] + c->b2 * out[dim-3] + c->b3 * out[dim-4];
	/*--- backward ---*/
	in[dim-1] = c->B * out[dim-1];
	in[dim-2] = c->B * out[dim-2] + c->b1 * in[dim-1];
	in[dim-3] = c->B * out[dim-3] + c->b1 * in[dim-2] + c->b2 * in[dim-1];
	for ( i = dim-4; i >= 0; i-- )
	    in[i] = c->B * out[i] + c->b1 * in[i+1] + c->b2 * in[i+2] + c->b3 * in[i+3];
	break;
    case 2 :
	/*--- forward ---*/
	out[0] = c->B * in[0];
	out[1] = c->B * (in[1] - in[0]) + c->b1 * out[0];
	out[2] = c->B * (in[2] - in[1]) + c->b1 * out[1] + c->b2 * out[0];
	for ( i = 3; i < dim; i++ )
	    out[i] = c->B * (in[i] - in[i-1]) + c->b1 * out[i-1] + c->b2 * out[i-2] + c->b3 * out[i-3];
	/*--- backward ---*/
	in[dim-1] = - c->B * out[dim-1];
	in[dim-2] = c->B * (out[dim-1] - out[dim-2]) + c->b1 * in[dim-1];
	in[dim-3] = c->B * (out[dim-2] - out[dim-3]) + c->b1 * in[dim-2] + c->b2 * in[dim-1];
	for ( i = dim-4; i >= 0; i-- )
	    in[i] = c->B * (out[i+1] - out[i]) + c->b1 * in[i+1] + c->b2 * in[i+2] + c->b3 * in[i+3];
	break;
    case 3 :
	/*--- forward ---*/
	out[0] = c->Bd * (in[1] - in[0] - in[0]);
	out[1] = c->Bd * (in[2] - in[1] - in[1] + in[0]) + c->b1 * out[0];
	out[2] = c->Bd * (in[3] - in[2] - in[2] + in[1]) + c->b1 * out[1] + c->b2 * out[0];
	for ( i = 3; i < dim-1; i++ )
	    out[i] = c->Bd * (in[i+1] - in[i] - in[i] + in[i-1]) + c->b1 * out[i-1] + c->b2 * out[i-2] + c->b3 * out[i-3];
	out[dim-1] = c->Bd * (- in[dim-1] - in[dim-1] + in[dim-2]) + c->b1 * out[dim-2] + c->b2 * out[dim-3] + c->b3 * out[dim-4];
	/*--- backward ---*/
	in[dim-1] = - c->B * out[dim-2];
	in[dim-2] = c->B * (out[dim-1] - out[dim-3]) + c->b1 * in[dim-1];
	in[dim-3] = c->B * (out[dim-2] - out[dim-4]) + c->b1 * in[dim-2] + c->b2 * in[dim-1];
	for ( i = dim-4; i >= 1; i-- )
	    in[i] = c->B * (out[i+1] - out[i-1]) + c->b1 * in[i+1] + c->b2 * in[i+2] + c->b3 * in[i+3];
	in[0] = c->B * out[1] + c->b1 * in[1] + c->b2 * in[2] + c->b3 * in[3];
	break;
    }
}





/* Calcul du laplacien par differences finies.

*/
int VT_LaplacianWithFD( vt_image *resIm /* result image */, 
			vt_image *theIm /* input image */, 
			int dim /* dimension od the computation */ )
{
    vt_image auxIn, auxOut;
    int bool_needAuxIn, bool_needAuxOut;
    register float *in, *out;
    register int x, y, z, dx, dxy;
    int local_dim, dx1, dy1, dz1;

    if ( VT_Test2Image( resIm, theIm, "VT_LaplacianWithFD" ) == -1 ) return( -1 );

    /*--- parametre ---*/
    local_dim = 0;
    if ( theIm->dim.x > 1 )                           local_dim = VT_1D;
    if ( (local_dim == VT_1D) && (theIm->dim.y > 1) ) local_dim = VT_2D;
    if ( (local_dim == VT_2D) && (theIm->dim.z > 1) ) local_dim = VT_3D;

    if ( local_dim == 0) {
	VT_Error("unable to compute laplacian","VT_LaplacianWithFD");
	return( -1 );
    }

    switch ( local_dim ) {
    case VT_3D :
	if ( (dim == VT_3D) || (dim == VT_2D) || (dim == VT_1D) ) local_dim = dim;
	break;
    case VT_2D :
	if ( (dim == VT_2D) || (dim == VT_1D) ) local_dim = dim;
    }

    /*--- images pour le calcul ---*/
    bool_needAuxIn = bool_needAuxOut = 1;
    VT_InitImage( &auxIn, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)FLOAT );
    VT_InitImage( &auxOut, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)FLOAT );

    if ( theIm->type == FLOAT ) {
	bool_needAuxIn = 0;
	auxIn.buf = theIm->buf;
    } else {
	if ( VT_AllocImage( &auxIn ) != 1 ) {
	    VT_Error("unable to allocate first auxiliary image","VT_LaplacianWithFD");
	    return( -1 );
	}
    }

    if ( (resIm->type == FLOAT) && (resIm->buf != theIm->buf) ) {
	bool_needAuxOut = 0;
	auxOut.buf = resIm->buf;
    } else {
	if ( VT_AllocImage( &auxOut ) != 1 ) {
	    VT_Error("unable to allocate second auxiliary image","VT_LaplacianWithFD");
	    if ( bool_needAuxIn == 1 ) VT_FreeImage( &auxIn );
	    return( -1 );
	}
    }
    
    /*--- copie de l'image d'entree ---*/
    if ( bool_needAuxIn != 1 ) {
	if ( VT_CopyImage( theIm, &auxIn ) != 1 ) {
	    VT_Error("unable to copy input image","VT_LaplacianWithFD");
	    if ( bool_needAuxIn == 1 ) VT_FreeImage( &auxIn );
	    if ( bool_needAuxOut == 1 ) VT_FreeImage( &auxOut );
	    return( -1 );
	}
    }

    /*--- calcul ---*/
    dx1 = theIm->dim.x - 1;
    dy1 = theIm->dim.y - 1;
    dz1 = theIm->dim.z - 1;
    dx = theIm->dim.x;
    dxy = theIm->dim.x * theIm->dim.y;
    in = (float*)(auxIn.buf);
    out = (float*)(auxOut.buf);

    for ( z = 0; z < (int)theIm->dim.z; z++ )
    for ( y = 0; y < (int)theIm->dim.y; y++ )
    for ( x = 0; x < (int)theIm->dim.x; x++ ) {
	*out = 0.0;    
	switch ( local_dim ) {
	case VT_3D :
	    if ( z > 0 )   *out += *(in - dxy) - *in;
	    if ( z < dz1 ) *out += *(in + dxy) - *in;
	case VT_2D :
	    if ( y > 0 )   *out += *(in - dx) - *in;
	    if ( y < dy1 ) *out += *(in + dx) - *in;
	case VT_1D :
	    if ( x > 0 )   *out += *(in - 1) - *in;
	    if ( x < dx1 ) *out += *(in + 1) - *in;
	}
	out ++;
	in ++;
    }

    /*--- copie de l'image resultat ---*/
    if ( bool_needAuxOut != 1 ) {
	if ( VT_CopyImage( &auxOut, resIm ) != 1 ) {
	    VT_Error("unable to copy result image","VT_LaplacianWithFD");
	    if ( bool_needAuxIn == 1 ) VT_FreeImage( &auxIn );
	    if ( bool_needAuxOut == 1 ) VT_FreeImage( &auxOut );
	    return( -1 );
	}
    }

    /*--- fin ---*/
    if ( bool_needAuxIn == 1 ) VT_FreeImage( &auxIn );
    if ( bool_needAuxOut == 1 ) VT_FreeImage( &auxOut );
    return( 1 );
}
