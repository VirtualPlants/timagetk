#include <vt_recfilters.h>







int VT_ComputeGradientWithRecursiveGaussian( vt_image *theIm,
					    vt_image *theGx,
					    vt_image *theGy,
					    vt_image *theGz,
					    float *sigma,
					    int *bordure )
{
  char *proc ="VT_ComputeGradientWithRecursiveGaussian";
  vt_recfilters par;

  if ( VT_Test2Image( theGx, theIm, proc ) == -1 ) return( -1 );
  if ( VT_Test2Image( theGy, theIm, proc ) == -1 ) return( -1 );
  if ( theIm->dim.z > 1 ) {
    if ( VT_Test2Image( theGz, theIm, proc ) == -1 ) return( -1 );
  }
  

  VT_RecFilters( &par );
  par.type_filter = VT_RECGAUSSIAN_MARTA;

  /*
   * cas 3D
   */
  if ( theIm->dim.z > 1 ) {

    if ( bordure != (int*)NULL ) {
      par.length_continue.x = bordure[0];
      par.length_continue.y = bordure[1];
      par.length_continue.z = bordure[2];
    }
    if ( sigma != (float*)NULL ) {
      par.value_coefficient.x = sigma[0];
      par.value_coefficient.y = sigma[1];
      par.value_coefficient.z = sigma[2];
    }


    par.derivative.x = VT_NODERIVATIVE;
    par.derivative.y = VT_NODERIVATIVE;
    par.derivative.z = VT_DERIVATIVE_0;

    if ( VT_RecFilterOnImage( theIm, theGy, &par ) != 1 ) {
      VT_Error( "unable to smooth along Z",proc );
      return( -1 );
    }

    par.derivative.x = VT_DERIVATIVE_1;
    par.derivative.y = VT_DERIVATIVE_0;
    par.derivative.z = VT_NODERIVATIVE;

    if ( VT_RecFilterOnImage( theGy, theGx, &par ) != 1 ) {
      VT_Error( "unable to derive along X",proc );
      return( -1 );
    }

    par.derivative.x = VT_DERIVATIVE_0;
    par.derivative.y = VT_DERIVATIVE_1;
    par.derivative.z = VT_NODERIVATIVE;

    if ( VT_RecFilterOnImage( theGy, theGy, &par ) != 1 ) {
      VT_Error( "unable to derive along Y",proc );
      return( -1 );
    }

    par.derivative.x = VT_DERIVATIVE_0;
    par.derivative.y = VT_DERIVATIVE_0;
    par.derivative.z = VT_DERIVATIVE_1;

    if ( VT_RecFilterOnImage( theIm, theGz, &par ) != 1 ) {
      VT_Error( "unable to derive along Z",proc );
      return( -1 );
    }
    
    return( 1 );
  }


  /*
   * cas 2D
   */
  if ( bordure != (int*)NULL ) {
    par.length_continue.x = bordure[0];
    par.length_continue.y = bordure[1];
  }
  if ( sigma != (float*)NULL ) {
    par.value_coefficient.x = sigma[0];
    par.value_coefficient.y = sigma[1];
  }

  par.derivative.x = VT_DERIVATIVE_1;
  par.derivative.y = VT_DERIVATIVE_0;
  par.derivative.z = VT_NODERIVATIVE;

  if ( VT_RecFilterOnImage( theIm, theGx, &par ) != 1 ) {
    VT_Error( "unable to derive along X",proc );
    return( -1 );
  }

  par.derivative.x = VT_DERIVATIVE_0;
  par.derivative.y = VT_DERIVATIVE_1;
  par.derivative.z = VT_NODERIVATIVE;

  if ( VT_RecFilterOnImage( theIm, theGy, &par ) != 1 ) {
    VT_Error( "unable to derive along Y",proc );
    return( -1 );
  }


  return( 1 );
}



int VT_SmoothWithRecursiveGaussian( vt_image *theIm,
				    vt_image *theRes,
				    float *sigma,
				    int *bordure )
{
  char *proc ="VT_SmoothWithRecursiveGaussian";
  vt_recfilters par;

  if ( VT_Test2Image( theRes, theIm, proc ) == -1 ) return( -1 );
  

  VT_RecFilters( &par );
  par.type_filter = VT_RECGAUSSIAN_MARTA;
  if ( bordure != (int*)NULL ) {
    par.length_continue.x = bordure[0];
    par.length_continue.y = bordure[1];
  }
  if ( sigma != (float*)NULL ) {
    par.value_coefficient.x = sigma[0];
    par.value_coefficient.y = sigma[1];
  }
  par.derivative.x = VT_DERIVATIVE_0;
  par.derivative.y = VT_DERIVATIVE_0;

  /*
   * cas 3D
   */
  if ( theIm->dim.z > 1 ) {

    if ( bordure != (int*)NULL ) {
      par.length_continue.z = bordure[2];
    }
    if ( sigma != (float*)NULL ) {
      par.value_coefficient.z = sigma[2];
    }
    par.derivative.z = VT_DERIVATIVE_0;

  }

  if ( VT_RecFilterOnImage( theIm, theRes, &par ) != 1 ) {
    VT_Error( "unable to smooth image",proc );
    return( -1 );
  }
  
  return( 1 );

}















void VT_RecFilters( vt_recfilters *par )
{
  par->type_filter = 0;
    par->derivative.x = par->derivative.y = par->derivative.z = VT_NODERIVATIVE;
    par->length_continue.x = par->length_continue.y = par->length_continue.z = 0;
    par->value_coefficient.x = par->value_coefficient.y = par->value_coefficient.z = 1.0;
}





/* Recursive filtering on one image.

 */

int VT_RecFilterOnImage( vt_image *theIm, vt_image *resIm, vt_recfilters *par )
{
    vt_image auxIm;
    int auxIsAllocated = 1;
    register float *auxBuf;
    vt_ipt offset, local_dim;
    double *in, *out, *work;
    int initIsOk;
    int dx, dxy;
    register int i, ind, x, y, z;
    char *proc="VT_RecFilterOnImage";

    if ( VT_Test2Image( resIm, theIm, proc ) == -1 ) return( -1 );
    if ( par->type_filter == 0 ) {
	VT_Error("unknown type of recursive filter",proc);
	return( 0 );
    }
    
    /*--- initialisation de l'image intermediaire ---*/
    VT_Image( &auxIm );
    VT_InitImage( &auxIm, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)FLOAT );
    if ( (resIm->type == FLOAT) && (resIm->buf != theIm->buf) ) {
	auxIm.buf = resIm->buf;
	auxIsAllocated = 0;
    }
    else {
	if ( VT_AllocImage( &auxIm ) != 1 ) {
	    VT_Error( "unable to allocate auxiliary image", proc );
	    return( -1 );
	}
    }
    if ( VT_CopyImage( theIm, &auxIm ) != 1 ) {
	VT_Error( "unable to copy from input image", proc );
	if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	return( -1 );
    }
    auxBuf = (float*)(auxIm.buf);

    /*--- dimensions ---*/
    offset.x = offset.y = offset.z = 0;
    if ( par->length_continue.x > 0 ) offset.x = par->length_continue.x;
    if ( par->length_continue.y > 0 ) offset.y = par->length_continue.y;
    if ( par->length_continue.z > 0 ) offset.z = par->length_continue.z;
    local_dim.x = theIm->dim.x + 2 * offset.x;
    local_dim.y = theIm->dim.y + 2 * offset.y;
    local_dim.z = theIm->dim.z + 2 * offset.z;
    dx  = theIm->dim.x;
    dxy = theIm->dim.x * theIm->dim.y;

    /*--- allocations des tableaux de travail ---*/
    i = local_dim.x;
    if ( local_dim.y > i ) i = local_dim.y;
    if ( local_dim.z > i ) i = local_dim.z;
    in = out = work = (double*)NULL;
    in   = (double*)VT_Malloc( (unsigned int)(i * sizeof( double ) ) );
    out  = (double*)VT_Malloc( (unsigned int)(i * sizeof( double ) ) );
    work = (double*)VT_Malloc( (unsigned int)(i * sizeof( double ) ) );
    if ( (in == (double*)NULL) || (out == (double*)NULL) || (work == (double*)NULL) ) {
	VT_Error( "unable to allocate auxiliary arrays", proc );
	VT_Free( (void**)&in );
	VT_Free( (void**)&out );
	VT_Free( (void**)&work );
	if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
	return( -1 );
    }

    /*--- filtrage recursif selon X ---*/
    if ( par->derivative.x != VT_NODERIVATIVE ) {
	initIsOk = VT_InitRecursiveCoefficients( (double)(par->value_coefficient.x), par->type_filter, par->derivative.x );
	if ( (initIsOk != 1) || (local_dim.x < 4) ) {
	    if ( initIsOk != 1 )
		VT_Error( "unable to initialize coefficients to filter along X", proc );
	    if ( local_dim.x < 4 )
		VT_Error( "too little dimension along X", proc );
	}
	else {
      for ( z = 0; z < (int)theIm->dim.z; z ++ )
      for ( y = 0; y < (int)theIm->dim.y; y ++ ) {
		/*--- saisie d'une ligne ---*/
		ind = z * dxy + y * dx;
    for ( x = 0, i = offset.x; x < (int)theIm->dim.x; x++, i++, ind++ )
		    in[i] = (double)auxBuf[ind];
		/*--- ajout eventuel de points ---*/
		x = theIm->dim.x + offset.x;
		for ( i = 0; i < offset.x; i++, x++ ) {
		    in[i] = in[offset.x];
		    in[x] = in[theIm->dim.x + offset.x - 1];
		}
		/*--- traitement de la ligne ---*/
		if ( VT_RecFilterOnLine( in, out, work, out, local_dim.x ) != 1 ) {
		    VT_Error( "unable to filter along X", proc );
		    VT_Free( (void**)&in );
		    VT_Free( (void**)&out );
		    VT_Free( (void**)&work );
		    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
		    return( -1 );
		}
		/*--- copie de la ligne ---*/
		ind = z * dxy + y * dx;
    for ( x = 0, i = offset.x; x < (int)theIm->dim.x; x++, i++, ind++ )
		    auxBuf[ind] = (float)out[i];
	    }
	}
    }

    /*--- filtrage recursif selon Y ---*/
    if ( par->derivative.y != VT_NODERIVATIVE ) {
	initIsOk = VT_InitRecursiveCoefficients( (double)(par->value_coefficient.y), par->type_filter, par->derivative.y );
	if ( (initIsOk != 1) || (local_dim.y < 4) ) {
	    if ( initIsOk != 1 )
		VT_Error( "unable to initialize coefficients to filter along Y", proc );
	    if ( local_dim.y < 4 )
		VT_Error( "too little dimension along Y", proc );
	}
	else {
      for ( z = 0; z < (int)theIm->dim.z; z ++ )
      for ( x = 0; x < (int)theIm->dim.x; x ++ ) {
		/*--- saisie d'une ligne ---*/
		ind = z * dxy + x;
    for ( y = 0, i = offset.y; y < (int)theIm->dim.y; y++, i++, ind += dx )
		    in[i] = (double)auxBuf[ind];
		/*--- ajout eventuel de points ---*/
		y = theIm->dim.y + offset.y;
		for ( i = 0; i < offset.y; i++, y++ ) {
		    in[i] = in[offset.y];
		    in[y] = in[theIm->dim.y + offset.y - 1];
		}
		/*--- traitement de la ligne ---*/
		if ( VT_RecFilterOnLine( in, out, work, out, local_dim.y ) != 1 ) {
		    VT_Error( "unable to filter along Y", proc );
		    VT_Free( (void**)&in );
		    VT_Free( (void**)&out );
		    VT_Free( (void**)&work );
		    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
		    return( -1 );
		}
		/*--- copie de la ligne ---*/
		ind = z * dxy + x;
    for ( y = 0, i = offset.y; y < (int)theIm->dim.y; y++, i++, ind += dx )
		    auxBuf[ind] = (float)out[i];
	    }
	}
    }

    /*--- filtrage recursif selon Z ---*/
    if ( par->derivative.z != VT_NODERIVATIVE ) {
	initIsOk = VT_InitRecursiveCoefficients( (double)(par->value_coefficient.z), par->type_filter, par->derivative.z );
	if ( (initIsOk != 1) || (local_dim.z < 4) ) {
	    if ( initIsOk != 1 )
		VT_Error( "unable to initialize coefficients to filter along Z", proc );
	    if ( local_dim.z < 4 )
		VT_Error( "too little dimension along Z", proc );
	}
	else {
      for ( y = 0; y < (int)theIm->dim.y; y ++ )
            for ( x = 0; x < (int)theIm->dim.x; x ++ ) {
		/*--- saisie d'une ligne ---*/
		ind = y * dx + x;
    for ( z = 0, i = offset.z; z < (int)theIm->dim.z; z++, i++, ind += dxy )
		    in[i] = (double)auxBuf[ind];
		/*--- ajout eventuel de points ---*/
		z = theIm->dim.z + offset.z;
		for ( i = 0; i < offset.z; i++, z++ ) {
		    in[i] = in[offset.z];
		    in[z] = in[theIm->dim.z + offset.z - 1];
		}
		/*--- traitement de la ligne ---*/
		if ( VT_RecFilterOnLine( in, out, work, out, local_dim.z ) != 1 ) {
		    VT_Error( "unable to filter along Z", proc );
		    VT_Free( (void**)&in );
		    VT_Free( (void**)&out );
		    VT_Free( (void**)&work );
		    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
		    return( -1 );
		}
		/*--- copie de la ligne ---*/
		ind = y * dx + x;
    for ( z = 0, i = offset.z; z < (int)theIm->dim.z; z++, i++, ind += dxy )
		    auxBuf[ind] = (float)out[i];
	    }
	}
    }
    
    /*--- liberations memoire ---*/
    VT_Free( (void**)&in );
    VT_Free( (void**)&out );
    VT_Free( (void**)&work );

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










