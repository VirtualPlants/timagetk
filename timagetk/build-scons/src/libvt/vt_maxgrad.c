#include <math.h>
#include <vt_maxgrad.h>


static void VT_Extrema2D( vt_image *resIm, 
			  float *gx, 
			  float *gy, 
			  float *norme, 
			  int z );
static void VT_Extrema3D( vt_image *resIm, 
			  float *gx, 
			  float *gy, 
			  float *gz, 
			  float *norme[3], 
			  int z );

/* Edge extraction by suppression of the non-maxima of the gradient.

   Filtering is achieved by using Deriche's filters or recursive
   approximations of the gaussian (and its derivative).

   if par->dim == VT_2D (2D mode), each slice is processed independantly.
   a 2D suppression of the non-maxima of the 2D gradient is performed.

   if par->dim == VT_3D (3D mode), a 3D suppression of the non-maxima of 
   the 3D gradient is performed, except for the first and last slices
   where a 2D suppression of the non-maxima of the 3D gradient is done.

   */

int VT_MaximaGradient( vt_image *theIm, vt_image *resIm, vt_contours *par )
{
    vt_recfilters rpar;
    int local_dim;
    vt_image theGx, theGy, theGz, theLz, theNo, auxIm;
    float *gx[2], *gy[2], *norme[3], *aux;
    float *lgx, *lgy, *lgz, *lno;
    float *theLzBuf, *theGzBuf;
    register int i, z;
    int dxy, auxIsAllocated = 1;
    char *proc="VT_MaximaGradient";

    if ( VT_Test2Image( resIm, theIm, proc ) == -1 ) return( -1 );

    /*--- preparation des parametres de filtrage ---*/
    VT_RecFilters( &rpar );
    if ( par->value_coefficient.x > 0.0 ) rpar.value_coefficient.x = par->value_coefficient.x;
    if ( par->value_coefficient.y > 0.0 ) rpar.value_coefficient.y = par->value_coefficient.y;
    if ( par->value_coefficient.z > 0.0 ) rpar.value_coefficient.z = par->value_coefficient.z;
    if ( par->length_continue.x > 0 ) rpar.length_continue.x = par->length_continue.x;
    if ( par->length_continue.y > 0 ) rpar.length_continue.y = par->length_continue.y;
    if ( par->length_continue.z > 0 ) rpar.length_continue.z = par->length_continue.z;
    switch ( par->type_filter ) {
    case VT_RECFILTERS_DERICHE :
    case VT_RECGAUSSIAN_DERICHE :
	rpar.type_filter = par->type_filter;
	break;
    default :
	rpar.type_filter = VT_RECFILTERS_DERICHE;
    }
    local_dim = par->dim;
    switch ( par->dim ) {
    case VT_2D :
	break;
    case VT_3D :
    default :
	if ( theIm->dim.z == 1 ) local_dim = VT_2D;
	else {
	    if ( (theIm->dim.z + 2 * rpar.length_continue.z) > 5 ) local_dim = VT_3D;
	    else                                                   local_dim = VT_2D;
	}
    }

    /*--- initialisation ---*/
    dxy = theIm->dim.x * theIm->dim.y;
    gx[0] = gx[1] = (float*)NULL;
    gy[0] = gy[1] = (float*)NULL;
    norme[0] = norme[1] = norme[2] = (float*)NULL;
    VT_Image( &theGx );
    VT_Image( &theGy );
    VT_Image( &theGz );
    VT_Image( &theLz );
    VT_Image( &theNo );
    VT_Image( &auxIm );
    VT_InitImage( &auxIm, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)FLOAT );
    if ( (resIm->type == FLOAT) && (resIm->buf != theIm->buf) ) {
	auxIm.array = resIm->array;
	auxIm.buf = resIm->buf;
	auxIsAllocated = 0;
    }
    else {
	if ( VT_AllocImage( &auxIm ) != 1 ) {
	    VT_Error( "unable to allocate auxiliary image", proc );
	    return( -1 );
	}
    }
    lgx = lgy = lgz = lno = (float*)NULL;
    theLzBuf = theGzBuf = (float*)NULL;

    /*--- extraction des maxima du gradient ---*/
    switch ( local_dim ) {
    case VT_2D :

	/*--- initialisation des images ---*/
	VT_InitImage( &theGx, "", theIm->dim.x, theIm->dim.y, (int)1, (int)FLOAT );
	VT_InitImage( &theGy, "", theIm->dim.x, theIm->dim.y, (int)1, (int)FLOAT );
	VT_InitImage( &theNo, "", theIm->dim.x, theIm->dim.y, (int)1, (int)FLOAT );
	if ( VT_AllocImage( &theGx ) != 1 ) {
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to allocate X gradient image", proc );
	    return( -1 );
	}
	if ( VT_AllocImage( &theGy ) != 1 ) {
	    VT_FreeImage( &theGx );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to allocate Y gradient image", proc );
	    return( -1 );
	}
	if ( VT_AllocImage( &theNo ) != 1 ) {
	    VT_FreeImage( &theGx );
	    VT_FreeImage( &theGy );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to allocate norm image", proc );
	    return( -1 );
	}
	VT_InitImage( &theLz, "", theIm->dim.x, theIm->dim.y, (int)1, theIm->type );
	
	/*--- parcours des plans ---*/
  for ( z = 0; z < (int)theIm->dim.z; z ++ ) {
	    /*--- positionnement du plan a traiter ---*/
	    switch ( theIm->type ) {
	    default :
	      VT_Error( "unable to deal with such image type", proc );
	      VT_FreeImage( &theGx );
	      VT_FreeImage( &theGy );
	      VT_FreeImage( &theNo );
	      if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	      return( -1 );
	    case UCHAR :
		theLz.buf = (void*)( ((u8*)(theIm->buf)) + z * dxy );
		break;
	    case SCHAR :
		theLz.buf = (void*)( ((s8*)(theIm->buf)) + z * dxy );
		break;
	    case USHORT :
		theLz.buf = (void*)( ((u16*)(theIm->buf)) + z * dxy );
		break;
	    case SSHORT :
		theLz.buf = (void*)( ((s16*)(theIm->buf)) + z * dxy );
		break;
	    case FLOAT :
		theLz.buf = (void*)( ((r32*)(theIm->buf)) + z * dxy );
		break;
	    case SINT :
		theLz.buf = (void*)( ((i32*)(theIm->buf)) + z * dxy );
		break;
	    }
	    /*--- calcul des derivees ---*/
	    rpar.derivative.z = VT_NODERIVATIVE;
	    rpar.derivative.x = VT_DERIVATIVE_1_CONTOURS;   rpar.derivative.y = VT_DERIVATIVE_0;
	    if ( VT_RecFilterOnImage( &theLz, &theGx, &rpar ) != 1 ) {
		VT_FreeImage( &theGx );
		VT_FreeImage( &theGy );
		VT_FreeImage( &theNo );
		if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
		VT_Error( "unable to compute X gradient image", proc );
		return( -1 );
	    }
	    rpar.derivative.x = VT_DERIVATIVE_0;   rpar.derivative.y = VT_DERIVATIVE_1_CONTOURS;
	    if ( VT_RecFilterOnImage( &theLz, &theGy, &rpar ) != 1 ) {
		VT_FreeImage( &theGx );
		VT_FreeImage( &theGy );
		VT_FreeImage( &theNo );
		if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
		VT_Error( "unable to compute Y gradient image", proc );
		return( -1 );
	    }
	    /*--- calcul de la norme ---*/
	    /*--- avec la norme au carre, l'interpolation fonctionne moins bien ---*/
	    lgx = (float*)theGx.buf;
	    lgy = (float*)theGy.buf;
	    lno = (float*)theNo.buf;
	    for ( i = 0; i < dxy; i++, lno++, lgx++, lgy++ )
		*lno = (float)sqrt( (double)((*lgx) * (*lgx) + (*lgy) * (*lgy)) );
	    /*--- calcul des extrema ---*/
	    VT_Extrema2D( &auxIm, (float*)theGx.buf, (float*)theGy.buf, (float*)theNo.buf, z );
	}

	/*--- liberations memoire ---*/
	VT_FreeImage( &theGx );
	VT_FreeImage( &theGy );
	VT_FreeImage( &theNo );
	
	break;

    case VT_3D :
	
	/*--- initialisation des images ---*/
	VT_InitImage( &theLz, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)FLOAT );
	VT_InitImage( &theGz, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)FLOAT );
	VT_InitImage( &theGx, "", theIm->dim.x, theIm->dim.y, (int)1, (int)FLOAT );
	VT_InitImage( &theGy, "", theIm->dim.x, theIm->dim.y, (int)1, (int)FLOAT );
	VT_InitImage( &theNo, "", theIm->dim.x, theIm->dim.y, (int)1, (int)FLOAT );
	if ( VT_AllocImage( &theLz ) != 1 ) {
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to allocate first auxiliary image", proc );
	    return( -1 );
	}
	if ( VT_AllocImage( &theGz ) != 1 ) {
	    VT_FreeImage( &theLz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to allocate second auxiliary image", proc );
	    return( -1 );
	}
	theLzBuf = (float*)(theLz.buf);
	theGzBuf = (float*)(theGz.buf);

	/*--- calcul des convolutions selon Z ---*/
	rpar.derivative.x = VT_NODERIVATIVE;   rpar.derivative.y = VT_NODERIVATIVE;   rpar.derivative.z = VT_DERIVATIVE_0;
	if ( VT_RecFilterOnImage( theIm, &theLz, &rpar ) != 1 ) {
	    VT_FreeImage( &theLz );
	    VT_FreeImage( &theGz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to compute Z-smoothed image", proc );
	    return( -1 );
	}
	rpar.derivative.x = VT_DERIVATIVE_0;   rpar.derivative.y = VT_DERIVATIVE_0;   rpar.derivative.z = VT_DERIVATIVE_1_CONTOURS;
	if ( VT_RecFilterOnImage( theIm, &theGz, &rpar ) != 1 ) {
	    VT_FreeImage( &theLz );
	    VT_FreeImage( &theGz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to compute Z gradient image", proc );
	    return( -1 );
	}
	
	/*--- petites allocations supplementaires ---*/
	gx[0] = (float*)VT_Malloc( (unsigned int)(dxy*sizeof(float)) );
	gx[1] = (float*)VT_Malloc( (unsigned int)(dxy*sizeof(float)) );
	gy[0] = (float*)VT_Malloc( (unsigned int)(dxy*sizeof(float)) );
	gy[1] = (float*)VT_Malloc( (unsigned int)(dxy*sizeof(float)) );
	norme[0] = (float*)VT_Malloc( (unsigned int)(dxy*sizeof(float)) );
	norme[1] = (float*)VT_Malloc( (unsigned int)(dxy*sizeof(float)) );
	norme[2] = (float*)VT_Malloc( (unsigned int)(dxy*sizeof(float)) );
	if ( (gx[0] == (float*)NULL) || (gx[1] == (float*)NULL) ||
	     (gy[0] == (float*)NULL) || (gy[1] == (float*)NULL) ||
	     (norme[0] == (float*)NULL) || (norme[1] == (float*)NULL) || (norme[2] == (float*)NULL) ) {
	    VT_Free( (void**)&(gx[0]) );
	    VT_Free( (void**)&(gx[1]) );
	    VT_Free( (void**)&(gy[0]) );
	    VT_Free( (void**)&(gy[1]) );
	    VT_Free( (void**)&(norme[0]) );
	    VT_Free( (void**)&(norme[1]) );
	    VT_Free( (void**)&(norme[2]) );
	    VT_FreeImage( &theLz );
	    VT_FreeImage( &theGz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to allocate auxiliary buffers", proc );
	    return( -1 );
	}

	/*--- premier plan ---*/
	theGx.buf = (void*)gx[0];
	theGy.buf = (void*)gy[0];
	theNo.buf = theLz.buf;

	/*--- premier plan : calcul des derivees ---*/
	rpar.derivative.z = VT_NODERIVATIVE;
	rpar.derivative.x = VT_DERIVATIVE_1_CONTOURS;   rpar.derivative.y = VT_DERIVATIVE_0;
	if ( VT_RecFilterOnImage( &theNo, &theGx, &rpar ) != 1 ) {
	    VT_Free( (void**)&(gx[0]) );
	    VT_Free( (void**)&(gx[1]) );
	    VT_Free( (void**)&(gy[0]) );
	    VT_Free( (void**)&(gy[1]) );
	    VT_Free( (void**)&(norme[0]) );
	    VT_Free( (void**)&(norme[1]) );
	    VT_Free( (void**)&(norme[2]) );
	    VT_FreeImage( &theLz );
	    VT_FreeImage( &theGz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to compute X gradient image", proc );
	    return( -1 );
	}
	rpar.derivative.x = VT_DERIVATIVE_0;   rpar.derivative.y = VT_DERIVATIVE_1_CONTOURS;
	if ( VT_RecFilterOnImage( &theNo, &theGy, &rpar ) != 1 ) {
	    VT_Free( (void**)&(gx[0]) );
	    VT_Free( (void**)&(gx[1]) );
	    VT_Free( (void**)&(gy[0]) );
	    VT_Free( (void**)&(gy[1]) );
	    VT_Free( (void**)&(norme[0]) );
	    VT_Free( (void**)&(norme[1]) );
	    VT_Free( (void**)&(norme[2]) );
	    VT_FreeImage( &theLz );
	    VT_FreeImage( &theGz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to compute Y gradient image", proc );
	    return( -1 );
	}
	/*--- premier plan : calcul de la norme ---*/
	lgx = gx[0];
	lgy = gy[0];
	lgz = theGzBuf;
	lno = norme[0];
	for ( i = 0; i < dxy; i++, lno++, lgx++, lgy++, lgz++ )
	    *lno = (float)sqrt( (double)((*lgx) * (*lgx) + (*lgy) * (*lgy) + (*lgz) * (*lgz)) );
	/*--- premier plan : calcul des extrema ---*/
	VT_Extrema2D( &auxIm, gx[0], gy[0], norme[0], (int)0 );

	/*--- premier plan : preparation du plan suivant : calcul des derivees ---*/
	theNo.buf = (void*)( theLzBuf + dxy );
	rpar.derivative.x = VT_DERIVATIVE_1_CONTOURS;   rpar.derivative.y = VT_DERIVATIVE_0;
	if ( VT_RecFilterOnImage( &theNo, &theGx, &rpar ) != 1 ) {
	    VT_Free( (void**)&(gx[0]) );
	    VT_Free( (void**)&(gx[1]) );
	    VT_Free( (void**)&(gy[0]) );
	    VT_Free( (void**)&(gy[1]) );
	    VT_Free( (void**)&(norme[0]) );
	    VT_Free( (void**)&(norme[1]) );
	    VT_Free( (void**)&(norme[2]) );
	    VT_FreeImage( &theLz );
	    VT_FreeImage( &theGz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to compute X gradient image", proc );
	    return( -1 );
	}
	rpar.derivative.x = VT_DERIVATIVE_0;   rpar.derivative.y = VT_DERIVATIVE_1_CONTOURS;
	if ( VT_RecFilterOnImage( &theNo, &theGy, &rpar ) != 1 ) {
	    VT_Free( (void**)&(gx[0]) );
	    VT_Free( (void**)&(gx[1]) );
	    VT_Free( (void**)&(gy[0]) );
	    VT_Free( (void**)&(gy[1]) );
	    VT_Free( (void**)&(norme[0]) );
	    VT_Free( (void**)&(norme[1]) );
	    VT_Free( (void**)&(norme[2]) );
	    VT_FreeImage( &theLz );
	    VT_FreeImage( &theGz );
	    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	    VT_Error( "unable to compute Y gradient image", proc );
	    return( -1 );
	}
	/*--- premier plan : preparation du plan suivant : calcul de la norme du gradient ---*/
	lgx = gx[0];
	lgy = gy[0];
	lgz = theGzBuf + dxy;
	lno = norme[1];
	for ( i = 0; i < dxy; i++, lno++, lgx++, lgy++, lgz++ )
	    *lno = (float)sqrt( (double)((*lgx) * (*lgx) + (*lgy) * (*lgy) + (*lgz) * (*lgz)) );

        /*--- autres plans ---*/
	/*--------------------*/
  for ( z = 1; z < (int)theIm->dim.z-1; z++ ) {
	    /*--- calcul des derivees du plan suivant ---*/
	    theGx.buf = (void*)gx[1];
	    theGy.buf = (void*)gy[1];
	    theNo.buf = (void*)( theLzBuf + (z+1)*dxy );
	    rpar.derivative.x = VT_DERIVATIVE_1_CONTOURS;   rpar.derivative.y = VT_DERIVATIVE_0;
	    if ( VT_RecFilterOnImage( &theNo, &theGx, &rpar ) != 1 ) {
		VT_Free( (void**)&(gx[0]) );
		VT_Free( (void**)&(gx[1]) );
		VT_Free( (void**)&(gy[0]) );
		VT_Free( (void**)&(gy[1]) );
		VT_Free( (void**)&(norme[0]) );
		VT_Free( (void**)&(norme[1]) );
		VT_Free( (void**)&(norme[2]) );
		VT_FreeImage( &theLz );
		VT_FreeImage( &theGz );
		if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
		VT_Error( "unable to compute X gradient image", proc );
		return( -1 );
	    }
	    rpar.derivative.x = VT_DERIVATIVE_0;   rpar.derivative.y = VT_DERIVATIVE_1_CONTOURS;
	    if ( VT_RecFilterOnImage( &theNo, &theGy, &rpar ) != 1 ) {
		VT_Free( (void**)&(gx[0]) );
		VT_Free( (void**)&(gx[1]) );
		VT_Free( (void**)&(gy[0]) );
		VT_Free( (void**)&(gy[1]) );
		VT_Free( (void**)&(norme[0]) );
		VT_Free( (void**)&(norme[1]) );
		VT_Free( (void**)&(norme[2]) );
		VT_FreeImage( &theLz );
		VT_FreeImage( &theGz );
		if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
		VT_Error( "unable to compute Y gradient image", proc );
		return( -1 );
	    }
	    /*--- calcul de la norme au carre du plan suivant---*/
	    lgx = gx[1];
	    lgy = gy[1];
	    lgz = theGzBuf + (z+1)*dxy;
	    lno = norme[2];
	    for ( i = 0; i < dxy; i++, lno++, lgx++, lgy++, lgz++ )
		*lno = (float)sqrt( (double)((*lgx) * (*lgx) + (*lgy) * (*lgy) + (*lgz) * (*lgz)) );

	    /*--- calcul des extrema du gradient du plan courant ---*/
	    lgz = theGzBuf + z*dxy;
	    VT_Extrema3D( &auxIm, gx[0], gy[0], lgz, norme, z );

	    /*--- permutations ---*/
	    aux = gx[0];   gx[0] = gx[1];   gx[1] = aux;
	    aux = gy[0];   gy[0] = gy[1];   gy[1] = aux;
	    aux = norme[0]; norme[0] = norme[1];  norme[1] = norme[2];  norme[2] = aux;

	    
	}

	/*--- dernier plan : calcul des extrema ---*/
	VT_Extrema2D( &auxIm, gx[0], gy[0], norme[1], (theIm->dim.z - 1) );

        /*--- liberations memoire ---*/
	VT_Free( (void**)&(gx[0]) );
	VT_Free( (void**)&(gx[1]) );
	VT_Free( (void**)&(gy[0]) );
	VT_Free( (void**)&(gy[1]) );
	VT_Free( (void**)&(norme[0]) );
	VT_Free( (void**)&(norme[1]) );
	VT_Free( (void**)&(norme[2]) );
	VT_FreeImage( &theLz );
	VT_FreeImage( &theGz );
	
    }

    /*--- liberations memoire et copie du resultat---*/
    if ( auxIsAllocated == 1 ) {
	if ( VT_CopyImage( &auxIm, resIm ) != 1 ) {
	    VT_Error( "unable to copy into output image", proc );
	    VT_FreeImage( &auxIm );
	    return( -1 );
	}
    }

    return( 1 );
}






static void VT_Extrema2D( vt_image *resIm, float *gx, float *gy, float *norme, int z )
{
    register double ngx, ngy, norm;
    register double xr, yr, dx, dy;
    register int x, y, xi, yi, i, j, pos;
    int dimx, dimy, dimx1, dimy1, off;
    r32 *vl;
    double coef[2][2];
    double EPSILON = 0.00001; /* 1/2^16 = .00001525878906250000 */


    /*--- initialisation ---*/
    dimx = resIm->dim.x;
    dimy = resIm->dim.y;
    dimx1 = dimx - 1;
    dimy1 = dimy - 1;
    off = z * dimx * dimy;
    vl = (r32*)resIm->buf;

    /*--- on met les bords a zero ---*/
    for ( x=0, i=off, j=off+dimx*dimy1; x < dimx; x++, i++, j++ )
	vl[i] = vl[j] = (float)0.0;
    for ( y=1, i=off+dimx, j=off+2*dimx-1; y < dimy1; y++, i+=dimx, j+=dimx )
	vl[i] = vl[j] = (float)0.0;
    
    /*--- on parcourt le centre de l'image ---*/
    for ( y=1; y < dimy1; y++ )
    for ( x=1, i=off+y*dimx+1, j=y*dimx+1; x < dimx1; x++, i++, j++ ) {
	/*--- si la norme est trop petite, on ne considere pas le point ---*/
	if ( *(norme+j) <= EPSILON ) {
	    *(vl+i) = 0.0;
	    continue;
	}
	/*--- on normalise le gradient ---*/
	ngx = *(gx+j) / *(norme+j);
	ngy = *(gy+j) / *(norme+j);

	/*--- ou trouve-t-on le premier point ? ---*/
	xr = (double)x + ngx;   yr = (double)y + ngy;
	if ( (xr < 0.0) || (xr >= dimx1) || (yr < 0.0) || (yr >= dimy1) ) {
	  *(vl+i) = 0.0;
	  continue;
	}
	xi = (int)xr;           yi = (int)yr;
	dx = xr - (double)xi;   dy = yr - (double)yi;
	/*--- interpolation lineaire : calcul des coefficients ---*/
	coef[0][0] = (1.0 - dx) * (1.0 - dy); /* xi,   yi   */
	coef[0][1] = dx         * (1.0 - dy); /* xi+1, yi   */
	coef[1][0] = (1.0 - dx) * dy;         /* xi,   yi+1 */
	coef[1][1] = dx         * dy;         /* xi+1, yi+1 */

	pos = xi+yi*dimx;
	norm  = *(norme+pos)      * coef[0][0] + *(norme+pos+1)      * coef[0][1];
	norm += *(norme+pos+dimx) * coef[1][0] + *(norme+pos+1+dimx) * coef[1][1];

	/*--- comparaison par rapport au premier point : extrema ? ---*/
	if ( *(norme+j) <= norm ) {
	    *(vl+i) = 0.0;
	    continue;
	}
	
	/*--- second point ---*/
	xr = (double)x - ngx;   yr = (double)y - ngy;
	if ( (xr < 0.0) || (xr >= dimx1) || (yr < 0.0) || (yr >= dimy1) ) {
	  *(vl+i) = 0.0;
	  continue;
	}
	xi = (int)xr;           yi = (int)yr;
	dx = xr - (double)xi;   dy = yr - (double)yi;
	/*--- interpolation lineaire : calcul des coefficients ---*/
	coef[0][0] = (1.0 - dx) * (1.0 - dy); /* xi,   yi   */
	coef[0][1] = dx         * (1.0 - dy); /* xi+1, yi   */
	coef[1][0] = (1.0 - dx) * dy;         /* xi,   yi+1 */
	coef[1][1] = dx         * dy;         /* xi+1, yi+1 */

	pos = xi+yi*dimx;
	norm  = *(norme+pos)      * coef[0][0] + *(norme+pos+1)      * coef[0][1];
	norm += *(norme+pos+dimx) * coef[1][0] + *(norme+pos+1+dimx) * coef[1][1];

	/*--- extrema ? ---*/
	if ( *(norme+j) < norm ) {
	    *(vl+i) = 0.0;
	    continue;
	}

	/*--- c'est un extrema ---*/
	*(vl+i) = *(norme+j);
    }
}





static void VT_Extrema3D( vt_image *resIm, float *gx, float *gy, float *gz, float *norme[3], int z_offset )
{
    register double ngx, ngy, ngz, norm, norme_originale;
    register double xr, yr, zr, dx, dy, dz;
    register int x, y, z, xi, yi, zi, i, j, pos;
    int dimx, dimy, dimx1, dimy1, off;
    float *vl;
    double coef[2][2][2];
    double EPSILON = 0.00001; /* 1/2^16 = .00001525878906250000 */

    /*--- initialisation ---*/
    dimx = resIm->dim.x;
    dimy = resIm->dim.y;
    dimx1 = dimx - 1;
    dimy1 = dimy - 1;
    off = z_offset*dimx*dimy;
    vl = (float*)resIm->buf;

    /*--- on met les bords a zero ---*/
    for ( x=0, i=off, j=off+dimx*dimy1; x < dimx; x++, i++, j++ )
	*(vl+i) = *(vl+j) = 0.0;
    for ( y=1, i=off+dimx, j=off+2*dimx-1; y < dimy1; y++, i+=dimx, j+=dimx )
	*(vl+i) = *(vl+j) = 0.0;

    /*--- on se place correctement ---*/
    vl += off;
    z = 1;

    /*--- on parcourt le centre de l'image ---*/
    for ( y = 1; y < dimy1; y++ )
    for ( x=1, j=y*dimx+1; x < dimx1; x++, j++ ) {

	/*--- si la norme est trop petite, on ne considere pas le point ---*/
	norme_originale = *(norme[1]+j);
	if ( norme_originale <= EPSILON ) {
	    *(vl+j) = 0.0;
	    continue;
	}

	/*--- on normalise la norme du gradient ---*/
	ngx = *(gx+j) / norme_originale;
	ngy = *(gy+j) / norme_originale;
	ngz = *(gz+j) / norme_originale;

	/*--- ou trouve-t-on le premier point ? ---*/
	xr = (double)x + ngx;   yr = (double)y + ngy;   zr = (double)z + ngz;
	if ( (xr < 0.0) || (xr >= dimx1) || (yr < 0.0) || (yr >= dimy1) || (zr < 0.0) || (zr >= 2) ) {
	   *(vl+j) = 0.0;
	  continue;
	}
	xi = (int)xr;           yi = (int)yr;           zi = (int)zr;
	dx = xr - (double)xi;   dy = yr - (double)yi;   dz = zr - (double)zi;
	/*--- interpolation lineaire : calcul des coefficients ---*/
	coef[0][0][0] = (1.0 - dx) * (1.0 - dy) * (1.0 - dz); /* xi,   yi,   zi   */
	coef[1][0][0] = dx         * (1.0 - dy) * (1.0 - dz); /* xi+1, yi,   zi   */
	coef[0][1][0] = (1.0 - dx) * dy         * (1.0 - dz); /* xi,   yi+1, zi   */
	coef[1][1][0] = dx         * dy         * (1.0 - dz); /* xi+1, yi+1, zi   */
	coef[0][0][1] = (1.0 - dx) * (1.0 - dy) * dz;         /* xi,   yi,   zi+1 */
	coef[1][0][1] = dx         * (1.0 - dy) * dz;         /* xi+1, yi,   zi+1 */
	coef[0][1][1] = (1.0 - dx) * dy         * dz;         /* xi,   yi+1, zi+1 */
	coef[1][1][1] = dx         * dy         * dz;         /* xi+1, yi+1, zi+1 */

	pos = xi + yi * dimx;
	norm  = *(norme[zi]+pos)        * coef[0][0][0] + *(norme[zi]+pos+1)        * coef[1][0][0];
	norm += *(norme[zi]+pos+dimx)   * coef[0][1][0] + *(norme[zi]+pos+1+dimx)   * coef[1][1][0];
	norm += *(norme[zi+1]+pos)      * coef[0][0][1] + *(norme[zi+1]+pos+1)      * coef[1][0][1];
	norm += *(norme[zi+1]+pos+dimx) * coef[0][1][1] + *(norme[zi+1]+pos+1+dimx) * coef[1][1][1];

	/*--- comparaison par rapport au premier point : extrema ? ---*/
	if ( norme_originale <= norm ) {
	    *(vl+j) = 0.0;
	    continue;
	}

	/*--- second point ---*/
	xr = (double)x - ngx;   yr = (double)y - ngy;   zr = (double)z - ngz;
	if ( (xr < 0.0) || (xr >= dimx1) || (yr < 0.0) || (yr >= dimy1) || (zr < 0.0) || (zr >= 2) ) {
	   *(vl+j) = 0.0;
	  continue;
	}
	xi = (int)xr;           yi = (int)yr;           zi = (int)zr;
	dx = xr - (double)xi;   dy = yr - (double)yi;   dz = zr - (double)zi;
	/*--- interpolation lineaire : calcul des coefficients ---*/
	coef[0][0][0] = (1.0 - dx) * (1.0 - dy) * (1.0 - dz); /* xi,   yi,   zi   */
	coef[1][0][0] = dx         * (1.0 - dy) * (1.0 - dz); /* xi+1, yi,   zi   */
	coef[0][1][0] = (1.0 - dx) * dy         * (1.0 - dz); /* xi,   yi+1, zi   */
	coef[1][1][0] = dx         * dy         * (1.0 - dz); /* xi+1, yi+1, zi   */
	coef[0][0][1] = (1.0 - dx) * (1.0 - dy) * dz;         /* xi,   yi,   zi+1 */
	coef[1][0][1] = dx         * (1.0 - dy) * dz;         /* xi+1, yi,   zi+1 */
	coef[0][1][1] = (1.0 - dx) * dy         * dz;         /* xi,   yi+1, zi+1 */
	coef[1][1][1] = dx         * dy         * dz;         /* xi+1, yi+1, zi+1 */

	pos = xi + yi * dimx;
	norm  = *(norme[zi]+pos)        * coef[0][0][0] + *(norme[zi]+pos+1)        * coef[1][0][0];
	norm += *(norme[zi]+pos+dimx)   * coef[0][1][0] + *(norme[zi]+pos+1+dimx)   * coef[1][1][0];
	norm += *(norme[zi+1]+pos)      * coef[0][0][1] + *(norme[zi+1]+pos+1)      * coef[1][0][1];
	norm += *(norme[zi+1]+pos+dimx) * coef[0][1][1] + *(norme[zi+1]+pos+1+dimx) * coef[1][1][1];

	/*--- extrema ? ---*/
	if ( norme_originale < norm ) {
	    *(vl+j) = 0.0;
	    continue;
	}

	/*--- c'est un extrema ---*/
	*(vl+j) = (float)norme_originale;
    }
}







