#include <math.h>
#include <vt_contours.h>

/* Initialization of the structure vt_contours.
 */

void VT_Contours( vt_contours *par )
{
    par->dim = VT_3D;
    par->type_contours = VT_MAXIMA_GRADIENT;
    par->type_filter = 0;
    par->length_continue.x = par->length_continue.y = par->length_continue.z = 0;
    par->value_coefficient.x = par->value_coefficient.y = par->value_coefficient.z = 1.0;
}





/* Computation of the edge map.

   Coming soon: edges from laplacian.
   */

int VT_ExtractEdges( vt_image *theIm, vt_image *resIm, vt_contours *par )
{
    switch ( par->type_contours ) {
    case VT_MAXIMA_GRADIENT :
    default :
      return( VT_MaximaGradient( theIm, resIm, par ) );
    }
    return( 1 );
}





int VT_NormeGradient( vt_image *theIm, vt_image *resIm, vt_contours *par, int derivative )
{
  char *proc="VT_NormeGradient";
  vt_image tmpIm, sumIm;
  vt_recfilters x_par, y_par, z_par;
  int i, v, local_derivative;
  r32 *tmp, *sum;

  if ( VT_Test2Image( resIm, theIm, proc ) == -1 ) return( -1 );


  VT_InitFromImage( &tmpIm, theIm, "", FLOAT );
  VT_InitFromImage( &sumIm, theIm, "", FLOAT );
  if ( VT_AllocImage( &tmpIm ) != 1 ) {
    VT_Error( "unable to allocate tmp image", proc );
    return( -1 );
  }
  if ( VT_AllocImage( &sumIm ) != 1 ) {
    VT_Error( "unable to allocate sum image", proc );
    VT_FreeImage( &tmpIm );
    return( -1 );
  }
  tmp = (r32*)tmpIm.buf;
  sum = (r32*)sumIm.buf;
  v = theIm->dim.x * theIm->dim.y * theIm->dim.z;



  switch ( par->type_filter ) {
  case VT_RECFILTERS_DERICHE :
  case VT_RECGAUSSIAN_DERICHE :
    x_par.type_filter = y_par.type_filter = z_par.type_filter = par->type_filter;
    break;
  default :
    x_par.type_filter = y_par.type_filter = z_par.type_filter = VT_RECFILTERS_DERICHE;
  }
 
    
  x_par.length_continue   = y_par.length_continue   = z_par.length_continue   = par->length_continue;
  x_par.value_coefficient = y_par.value_coefficient = z_par.value_coefficient = par->value_coefficient;


  switch ( derivative ) {
  case VT_DERIVATIVE_1_CONTOURS :
    local_derivative = VT_DERIVATIVE_1_CONTOURS;
    break;
  default :
    local_derivative = VT_DERIVATIVE_1;
  }



  if ( (theIm->dim.z < 4) || (par->dim == VT_2D) ) {

    x_par.derivative.x = local_derivative;
    x_par.derivative.y = VT_DERIVATIVE_0;
    x_par.derivative.z = VT_NODERIVATIVE;

    y_par.derivative.x = VT_DERIVATIVE_0;
    y_par.derivative.y = local_derivative;
    y_par.derivative.z = VT_NODERIVATIVE;

    if ( VT_RecFilterOnImage( theIm, &tmpIm, &x_par ) != 1 ) {
      VT_Error( "unable to compute X derivative", proc );
      VT_FreeImage( &tmpIm );
      VT_FreeImage( &sumIm );
      return( -1 );
    }
    for ( i=0; i<v; i++ ) sum[i] = tmp[i] * tmp[i];

    if ( VT_RecFilterOnImage( theIm, &tmpIm, &y_par ) != 1 ) {
      VT_Error( "unable to compute Y derivative", proc );
      VT_FreeImage( &tmpIm );
      VT_FreeImage( &sumIm );
      return( -1 );
    }
    for ( i=0; i<v; i++ ) sum[i] = (r32)sqrt( (double)(sum[i] + tmp[i] * tmp[i]) );
  


  } else {



    x_par.derivative.x = VT_NODERIVATIVE;
    x_par.derivative.y = VT_NODERIVATIVE;
    x_par.derivative.z = VT_DERIVATIVE_0;

    if ( VT_RecFilterOnImage( theIm, &tmpIm, &x_par ) != 1 ) {
      VT_Error( "unable to smooth along Z", proc );
      VT_FreeImage( &tmpIm );
      VT_FreeImage( &sumIm );
      return( -1 );
    }

    x_par.derivative.x = local_derivative;
    x_par.derivative.y = VT_DERIVATIVE_0;
    x_par.derivative.z = VT_NODERIVATIVE;

    y_par.derivative.x = VT_DERIVATIVE_0;
    y_par.derivative.y = local_derivative;
    y_par.derivative.z = VT_NODERIVATIVE;

    if ( VT_RecFilterOnImage( &tmpIm, &sumIm, &x_par ) != 1 ) {
      VT_Error( "unable to compute X derivative", proc );
      VT_FreeImage( &tmpIm );
      VT_FreeImage( &sumIm );
      return( -1 );
    }

    if ( VT_RecFilterOnImage( &tmpIm, &tmpIm, &y_par ) != 1 ) {
      VT_Error( "unable to compute Y derivative", proc );
      VT_FreeImage( &tmpIm );
      VT_FreeImage( &sumIm );
      return( -1 );
    }

    for ( i=0; i<v; i++ ) sum[i] = sum[i] * sum[i] + tmp[i] * tmp[i];

    z_par.derivative.x = VT_DERIVATIVE_0;
    z_par.derivative.y = VT_DERIVATIVE_0;
    z_par.derivative.z = local_derivative;

    if ( VT_RecFilterOnImage( theIm, &tmpIm, &z_par ) != 1 ) {
      VT_Error( "unable to compute Z derivative", proc );
      VT_FreeImage( &tmpIm );
      VT_FreeImage( &sumIm );
      return( -1 );
    }
    for ( i=0; i<v; i++ ) sum[i] = (r32)sqrt( (double)(sum[i] + tmp[i] * tmp[i]) );

  }
  
  VT_FreeImage( &tmpIm );

  
  if ( VT_CopyImage( &sumIm, resIm ) != 1 ) {
    VT_Error( "unable to copy into output image", proc );
    VT_FreeImage( &sumIm );
    return( -1 );
  }


  VT_FreeImage( &sumIm );
  return( 1 );

}










int VT_NormeGradientWithDerivatives( vt_image *theX, 
                             vt_image *theY,
                             vt_image *theZ,
                             vt_image *theNorme )
{
  char *proc = "VT_NormeGradient";
  double *tmp = (double*)NULL;
  register double *n;
  int x, y, z;
  

  if ( VT_Test2Image( theX, theY, proc ) == -1 ) return( -1 );
  if ( VT_Test2Image( theX, theNorme, proc ) == -1 ) return( -1 );
  if ( (theX->type != theY->type) ) {
    VT_Error("input images have different types", proc );
    return( 0 );
  }

  if ( theZ != (vt_image*)NULL ) {
    if ( VT_Test2Image( theX, theZ, proc ) == -1 ) return( -1 );
    if ( (theX->type != theZ->type) ) {
      VT_Error("input images have different types", proc );
      return( 0 );
    }  
  }

  tmp = (double*)VT_Malloc( (unsigned int)(theX->dim.x * theX->dim.y * sizeof( double )) );
  if ( tmp == (double*)NULL ) {
    VT_Error("unable to allocate auxiliary slice", proc );
    return( 0 );
  }
  
  for ( z=0; z < (int)theX->dim.z; z ++ ) {

    n = tmp;
    switch( theX->type ) {
    case FLOAT :
      {
      r32 ***bx = (r32***)(theX->array);
      r32 ***by = (r32***)(theY->array);
      r32 ***bz = (r32***)NULL;

      if ( theZ != (vt_image*)NULL ) {
        bz = (r32***)(theZ->array);
        for ( y=0; y<(int)theX->dim.y; y++ )
        for ( x=0; x<(int)theX->dim.x; x++ ) {
            *n = (double)bx[z][y][x] * (double)bx[z][y][x] +
            (double)by[z][y][x] * (double)by[z][y][x] +
            (double)bz[z][y][x] * (double)bz[z][y][x];
            n++;
        }
      } else {
        for ( y=0; y<(int)theX->dim.y; y++ )
        for ( x=0; x<(int)theX->dim.x; x++ ) {
            *n = (double)bx[z][y][x] * (double)bx[z][y][x] +
            (double)by[z][y][x] * (double)by[z][y][x];
            n++;
        }
      }

      }
      break;


    default :
      VT_Free( (void**)&tmp );
      VT_Error("such input type not handled yet", proc );
      return( 0 );
    }


    n = tmp;
    switch( theNorme->type ) {
    case FLOAT :
      {
      r32 ***bn = (r32***)(theNorme->array);
      for ( y=0; y<(int)theX->dim.y; y++ )
      for ( x=0; x<(int)theX->dim.x; x++ ) {
        bn[z][y][x] = sqrt( *n );
        n++;
      }
      }
      break;
      

    default :
      VT_Free( (void**)&tmp );
      VT_Error("such output type not handled yet", proc );
      return( 0 );
      
    }

  }

  VT_Free( (void**)&tmp );
  return( 1 );
}
