/*************************************************************************
 * bal-block-tools.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <chunks.h>

#include <bal-behavior.h>
#include <bal-block-tools.h>



#define _CORRELATION_COEFFICIENT_ERROR_VALUE_ -2
#define _MISC_ERROR_VALUE_ -10
#define _SSD_ERROR_VALUE_ 10000


static int _verbose_ = 1;
static int _debug_ = 0;


void BAL_SetVerboseInBalBlockTools( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalBlockTools(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalBlockTools(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}

void BAL_SetDebugInBalBlockTools( int d )
{
  _debug_ = d;
}

void BAL_IncrementDebugInBalBlockTools(  )
{
  _debug_ ++;
}

void BAL_DecrementDebugInBalBlockTools(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}



/*************************************************************
 *
 * Computation of measures between blocks
 *
 *************************************************************/



static double SumOfSquareDifferences3D( BLOC *bloc_flo, 
                                        BLOC *bloc_ref,
                                        bal_image *inrimage_flo,
                                        bal_image *inrimage_ref,
                                        bal_intensitySelection *selection_flo,
                                        bal_intensitySelection *selection_ref,
                                        bal_integerPoint *blockdim )
{
  char *proc = "SumOfSquareDifferences3D";

  int i, j, k;

  size_t fpos, fposz, fposy;
  size_t fdxy = inrimage_flo->ncols * inrimage_flo->nrows;
  size_t fdx  = inrimage_flo->ncols;
  size_t rpos, rposz, rposy;
  size_t rdxy = inrimage_ref->ncols * inrimage_ref->nrows;
  size_t rdx  = inrimage_ref->ncols;

  int a = bloc_flo->origin.x;
  int b = bloc_flo->origin.y;
  int c = bloc_flo->origin.z;
  int u = bloc_ref->origin.x;
  int v = bloc_ref->origin.y;
  int w = bloc_ref->origin.z;

  int npts;
  double sum_diff;



  /* all points of both blocks are used in the computation
     they are above their respective thresholds
  */
#define _MESURE_3D_SSD_FLO_INC_REF_INC {                         \
    npts = 0; \
    sum_diff = 0.0; \
    for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
      for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
           j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
          npts ++;        \
        }                                                       \
    return( sum_diff / npts ); \
  }



  /* check which points of both blocks are used in the computation
     such points are above their respective thresholds
  */
#define _MESURE_3D_SSD_FLO_INC_REF_NON_INC(_RMAX_) {             \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
              sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
              sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    return( sum_diff / npts ); \
  }



#define _MESURE_3D_SSD_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy)  \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
                 (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
              sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
              sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    return( sum_diff / npts ); \
  }



#define _MESURE_3D_SSD_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)    \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
                 (flo_buf[fpos] < selection_flo->high_threshold) &&     \
                 (ref_buf[rpos] > selection_ref->low_threshold) &&      \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
              sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    else {                                                      \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
              sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                            \
    return( sum_diff / npts ); \
  }






  /* 
     Les 2 blocs sont-ils complets dans les images (par rapport aux seuils sur I) ? 
     
     Sinon, il faut calculer le nombre de voxels actifs communs aux 2 blocs et 
     recalculer moyenne, variance pour le coefficient de correlation
  */

#define _MESURE_3D_SSD(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) {                   \
        _MESURE_3D_SSD_FLO_INC_REF_INC                  \
      }                                           \
      _MESURE_3D_SSD_FLO_INC_REF_NON_INC(_RMAX_)   \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _MESURE_3D_SSD_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _MESURE_3D_SSD_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }



  switch ( inrimage_flo->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such floating image type is not handled yet\n", proc );
    return( _SSD_ERROR_VALUE_ );
  case UCHAR :
    {
      unsigned char *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 255, 255 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 32767, 255 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 65535, 255 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case SSHORT :
    {
      short int *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 255, 32767 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 32767, 32767 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 65535, 32767 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case USHORT :
    {
      unsigned short int *flo_buf = inrimage_flo->data;

      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 255, 65535 )
        }
        break;
      case SSHORT :
        {
          signed short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 32767, 65535 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_SSD( 65535, 65535 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  } /* switch ( inrimage_flo->type ) */

  return( _SSD_ERROR_VALUE_ );
}





static double CorrelationCoefficient3D( BLOC *bloc_flo, 
                                        BLOC *bloc_ref,
                                        bal_image *inrimage_flo,
                                        bal_image *inrimage_ref,
                                        bal_intensitySelection *selection_flo,
                                        bal_intensitySelection *selection_ref,
                                        bal_integerPoint *blockdim )
{
  char *proc = "CorrelationCoefficient3D";

  int i, j, k;

  size_t fpos, fposz, fposy;
  size_t fdxy = inrimage_flo->ncols * inrimage_flo->nrows;
  size_t fdx  = inrimage_flo->ncols;
  size_t rpos, rposz, rposy;
  size_t rdxy = inrimage_ref->ncols * inrimage_ref->nrows;
  size_t rdx  = inrimage_ref->ncols;

  int a = bloc_flo->origin.x;
  int b = bloc_flo->origin.y;
  int c = bloc_flo->origin.z;
  int u = bloc_ref->origin.x;
  int v = bloc_ref->origin.y;
  int w = bloc_ref->origin.z;

  int npts;
  double flo_moy, ref_moy;
  double flo_sum, ref_sum;
  double flo_sum_2, ref_sum_2, flo_ref_sum;
  double rho, Ir2xIp2;



  /* all points of both blocks are used in the computation
     they are above their respective thresholds
  */
#define _MESURE_3D_CC_FLO_INC_REF_INC {                         \
    rho = 0.0; flo_moy = bloc_flo->mean; ref_moy = bloc_ref->mean;    \
    for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
      for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
           j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          rho += (flo_buf[ fpos ] - flo_moy) * (ref_buf[ rpos ] - ref_moy); \
        }                                                       \
    Ir2xIp2 = bloc_flo->nxvariance * bloc_ref->nxvariance;                  \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }



  /* check which points of both blocks are used in the computation
     such points are above their respective thresholds
  */
#define _MESURE_3D_CC_FLO_INC_REF_NON_INC(_RMAX_) {             \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
              flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
              ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
              rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
            }                                                     \
          }                                                       \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
              flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
              ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
              rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
            }                                                   \
          }                                                     \
    }                                                           \
    Ir2xIp2 = flo_sum * ref_sum; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }



#define _DIRECTMESURE_3D_CC_FLO_INC_REF_NON_INC(_RMAX_) {             \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    flo_sum_2 = ref_sum_2 = flo_ref_sum = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              flo_sum_2 += flo_buf[ fpos ] * flo_buf[ fpos ];     \
              ref_sum_2 += ref_buf[ rpos ] * ref_buf[ rpos ];     \
              flo_ref_sum += flo_buf[ fpos ] * ref_buf[ rpos ];   \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
      rho = flo_ref_sum - flo_sum * ref_sum / (double)npts;             \
      Ir2xIp2 = ( flo_sum_2 - flo_sum * flo_sum / (double)npts ) *      \
                ( ref_sum_2 - ref_sum * ref_sum / (double)npts );       \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              flo_sum_2 += flo_buf[ fpos ] * flo_buf[ fpos ];     \
              ref_sum_2 += ref_buf[ rpos ] * ref_buf[ rpos ];     \
              flo_ref_sum += flo_buf[ fpos ] * ref_buf[ rpos ];   \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
      rho = flo_ref_sum - flo_sum * ref_sum / (double)npts;             \
      Ir2xIp2 = ( flo_sum_2 - flo_sum * flo_sum / (double)npts ) *      \
                ( ref_sum_2 - ref_sum * ref_sum / (double)npts );       \
    }                                                           \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }



#define _MESURE_3D_CC_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy)  \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
                 (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy)  \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
                 (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
              flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
              ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
              rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
            }                                                     \
          }                                                       \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
              flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
              ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
              rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
            }                                                     \
          }                                                       \
    }                                                           \
    Ir2xIp2 = flo_sum * ref_sum; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }



#define _DIRECTMESURE_3D_CC_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    flo_sum_2 = ref_sum_2 = flo_ref_sum = 0.0; \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy)  \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
                 (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              flo_sum_2 += flo_buf[ fpos ] * flo_buf[ fpos ];     \
              ref_sum_2 += ref_buf[ rpos ] * ref_buf[ rpos ];     \
              flo_ref_sum += flo_buf[ fpos ] * ref_buf[ rpos ];   \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
      rho = flo_ref_sum - flo_sum * ref_sum / (double)npts;             \
      Ir2xIp2 = ( flo_sum_2 - flo_sum * flo_sum / (double)npts ) *      \
                ( ref_sum_2 - ref_sum * ref_sum / (double)npts );       \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              flo_sum_2 += flo_buf[ fpos ] * flo_buf[ fpos ];     \
              ref_sum_2 += ref_buf[ rpos ] * ref_buf[ rpos ];     \
              flo_ref_sum += flo_buf[ fpos ] * ref_buf[ rpos ];   \
              npts ++;                                            \
            }                                                     \
          }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
      rho = flo_ref_sum - flo_sum * ref_sum / (double)npts;             \
      Ir2xIp2 = ( flo_sum_2 - flo_sum * flo_sum / (double)npts ) *      \
                ( ref_sum_2 - ref_sum * ref_sum / (double)npts );       \
    }                                                           \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }



#define _MESURE_3D_CC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)    \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
                 (flo_buf[fpos] < selection_flo->high_threshold) &&     \
                 (ref_buf[rpos] > selection_ref->low_threshold) &&      \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)    \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
                 (flo_buf[fpos] < selection_flo->high_threshold) &&     \
                 (ref_buf[rpos] > selection_ref->low_threshold) &&      \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
              flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
              ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
              rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
            }                                                   \
          }                                                     \
    }                                                           \
    else {                                                      \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
              flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
              ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
              rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
            }                                                   \
          }                                                     \
    }                                                           \
    Ir2xIp2 = flo_sum * ref_sum; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }



#define _DIRECTMESURE_3D_CC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    flo_sum_2 = ref_sum_2 = flo_ref_sum = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)    \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
                 (flo_buf[fpos] < selection_flo->high_threshold) &&     \
                 (ref_buf[rpos] > selection_ref->low_threshold) &&      \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              flo_sum_2 += flo_buf[ fpos ] * flo_buf[ fpos ];     \
              ref_sum_2 += ref_buf[ rpos ] * ref_buf[ rpos ];     \
              flo_ref_sum += flo_buf[ fpos ] * ref_buf[ rpos ];   \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      rho = flo_ref_sum - flo_sum * ref_sum / (double)npts;             \
      Ir2xIp2 = ( flo_sum_2 - flo_sum * flo_sum / (double)npts ) *      \
                ( ref_sum_2 - ref_sum * ref_sum / (double)npts );       \
    }                                                           \
    else {                                                      \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
              flo_sum += flo_buf[ fpos ]; \
              ref_sum += ref_buf[ rpos ]; \
              flo_sum_2 += flo_buf[ fpos ] * flo_buf[ fpos ];     \
              ref_sum_2 += ref_buf[ rpos ] * ref_buf[ rpos ];     \
              flo_ref_sum += flo_buf[ fpos ] * ref_buf[ rpos ];   \
              npts ++;                                            \
            }                                                   \
          }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      rho = flo_ref_sum - flo_sum * ref_sum / (double)npts;             \
      Ir2xIp2 = ( flo_sum_2 - flo_sum * flo_sum / (double)npts ) *      \
                ( ref_sum_2 - ref_sum * ref_sum / (double)npts );       \
    }                                                           \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }



  /* 
     Les 2 blocs sont-ils complets dans les images (par rapport aux seuils sur I) ? 
     
     Sinon, il faut calculer le nombre de voxels actifs communs aux 2 blocs et 
     recalculer moyenne, variance pour le coefficient de correlation
  */

#define _MESURE_3D_CC(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) {                   \
        _MESURE_3D_CC_FLO_INC_REF_INC                  \
      }                                           \
      _MESURE_3D_CC_FLO_INC_REF_NON_INC(_RMAX_)   \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _MESURE_3D_CC_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _MESURE_3D_CC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }

#define _DIRECTMESURE_3D_CC(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) {                   \
        _MESURE_3D_CC_FLO_INC_REF_INC                  \
      }                                           \
      _DIRECTMESURE_3D_CC_FLO_INC_REF_NON_INC(_RMAX_)   \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _DIRECTMESURE_3D_CC_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _DIRECTMESURE_3D_CC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }



  switch ( inrimage_flo->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such floating image type is not handled yet\n", proc );
    return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
  case UCHAR :
    {
      unsigned char *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _DIRECTMESURE_3D_CC( 255, 255 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 32767, 255 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 65535, 255 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case SSHORT :
    {
      short int *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 255, 32767 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 32767, 32767 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 65535, 32767 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case USHORT :
    {
      unsigned short int *flo_buf = inrimage_flo->data;

      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 255, 65535 )
        }
        break;
      case SSHORT :
        {
          signed short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 32767, 65535 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_CC( 65535, 65535 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  } /* switch ( inrimage_flo->type ) */

  return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
}





static double ExtendedCorrelationCoefficient3D( BLOC *bloc_flo, 
                                                BLOC *bloc_ref,
                                                bal_image *inrimage_flo,
                                                bal_image *inrimage_ref,
                                                bal_intensitySelection *selection_flo,
                                                bal_intensitySelection *selection_ref,
                                                bal_integerPoint *blockdim )
{
  char *proc = "ExtendedCorrelationCoefficient3D";

  int i, j, k;

  size_t fpos, fposz, fposy;
  size_t fdxy = inrimage_flo->ncols * inrimage_flo->nrows;
  size_t fdx  = inrimage_flo->ncols;
  size_t rpos, rposz, rposy;
  size_t rdxy = inrimage_ref->ncols * inrimage_ref->nrows;
  size_t rdx  = inrimage_ref->ncols;

  int a = bloc_flo->origin.x;
  int b = bloc_flo->origin.y;
  int c = bloc_flo->origin.z;
  int u = bloc_ref->origin.x;
  int v = bloc_ref->origin.y;
  int w = bloc_ref->origin.z;

  int npts = blockdim->x * blockdim->y * blockdim->z;
  double rho, Ir2xIp2, moyab, moyuv;

  /* all points of both blocks are used in the computation
     they are above their respective thresholds
  */
#define _MESURE_3D_ECC_FLO_INC_REF_INC {                         \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;    \
    for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
      for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
           j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
        }                                                       \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance;                \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

  /* check which points of both blocks are used in the computation
     such points are above their respective thresholds
  */
#define _MESURE_3D_ECC_FLO_INC_REF_NON_INC(_RMAX_) {             \
    npts = 0;                                                 \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;  \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
              rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
              npts ++;                                            \
            }                                                     \
          }                                                       \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
              rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
              npts ++;                                            \
            }                                                   \
          }                                                     \
    }                                                           \
    if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

#define _MESURE_3D_ECC_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0;                                                 \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;  \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy)  \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
                 (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
              rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
              npts ++;                                            \
            }                                                     \
          }                                                       \
    }                                                             \
    else {                                                        \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
              rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
              npts ++;                                            \
            }                                                   \
          }                                                     \
    }                                                           \
    if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

#define _MESURE_3D_ECC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0;                                                 \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;  \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)    \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
                 (flo_buf[fpos] < selection_flo->high_threshold) &&     \
                 (ref_buf[rpos] > selection_ref->low_threshold) &&      \
                 (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
              rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
              npts ++;                                            \
            }                                                   \
          }                                                     \
    }                                                           \
    else {                                                      \
      for (k = 0, fposz = c*fdxy, rposz = w*rdxy; k < blockdim->z; k++, fposz += fdxy, rposz += rdxy) \
        for (j = 0, fposy = fposz + b*fdx, rposy = rposz + v*rdx; \
             j < blockdim->y; j++, fposy += fdx, rposy += rdx)   \
          for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
            if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
              rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
              npts ++;                                            \
            }                                                   \
          }                                                     \
    }                                                           \
    if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

  /* 
     Les 2 blocs sont-ils complets dans les images (par rapport aux seuils sur I) ? 
     
     Sinon, il faut calculer le nombre de voxels actifs communs aux 2 blocs et 
     recalculer moyenne, variance pour le coefficient de correlation
  */

#define _MESURE_3D_ECC(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) _MESURE_3D_ECC_FLO_INC_REF_INC \
        _MESURE_3D_ECC_FLO_INC_REF_NON_INC(_RMAX_) \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _MESURE_3D_ECC_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _MESURE_3D_ECC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }



  switch ( inrimage_flo->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such floating image type is not handled yet\n", proc );
    return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
  case UCHAR :
    {
      unsigned char *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 255, 255 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 32767, 255 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 65535, 255 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case SSHORT :
    {
      short int *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 255, 32767 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 32767, 32767 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 65535, 32767 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case USHORT :
    {
      unsigned short int *flo_buf = inrimage_flo->data;

      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 255, 65535 )
        }
        break;
      case SSHORT :
        {
          signed short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 32767, 65535 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_3D_ECC( 65535, 65535 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  } /* switch ( inrimage_flo->type ) */

  return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
}





/***
    Calcul de la mesure de similarite entre B(a,b,c) et B(u,v,w),
    a, b, c, u, v, w origines des blocs 
***/
double BAL_ComputeBlockSimilarity3D( BLOC *bloc_flo, BLOC *bloc_ref,
                     bal_image *image_flo, bal_image *image_ref,
                     bal_intensitySelection *selection_flo,
                     bal_intensitySelection *selection_ref,
                     bal_integerPoint *blockdim,
                     enumTypeSimilarity mesure )
{
  char *proc = "BAL_ComputeBlockSimilarity3D";
  switch ( mesure ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such measure is not implemented yet\n", proc );
    return( _MISC_ERROR_VALUE_ );
  case _SSD_ :
    return( SumOfSquareDifferences3D( bloc_flo, bloc_ref, image_flo, image_ref,
                                      selection_flo, selection_ref, blockdim ) );
  case _SQUARED_CC_ :
    return( CorrelationCoefficient3D( bloc_flo, bloc_ref, image_flo, image_ref,
                                      selection_flo, selection_ref, blockdim ) );
  case _SQUARED_EXTCC_ :
    return( ExtendedCorrelationCoefficient3D( bloc_flo, bloc_ref, image_flo, image_ref,
                                      selection_flo, selection_ref, blockdim ) );
    
  }
  return( _MISC_ERROR_VALUE_ );
}










/* Beware, this function return the average sum of square differences

*/
static double SumOfSquareDifferences2D( BLOC *bloc_flo, BLOC *bloc_ref,
                                        bal_image *inrimage_flo,
                                        bal_image *inrimage_ref,
                                        bal_intensitySelection *selection_flo,
                                        bal_intensitySelection *selection_ref,
                                        bal_integerPoint *blockdim )
{
  char *proc = "SumOfSquareDifferences2D";

  int i, j;

  size_t fpos, fposy;
  size_t fdx  = inrimage_flo->ncols;
  size_t rpos, rposy;
  size_t rdx  = inrimage_ref->ncols;

  int a = bloc_flo->origin.x;
  int b = bloc_flo->origin.y;
  int u = bloc_ref->origin.x;
  int v = bloc_ref->origin.y;


  int npts;
  double sum_diff;

  /* all points of both blocks are used in the computation
     they are above their respective thresholds
  */
#define _MESURE_2D_SSD_FLO_INC_REF_INC {                       \
    npts = 0; \
    sum_diff = 0.0; \
    for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
      for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
        sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
        npts ++;                                            \
      }                                                       \
    return( sum_diff / npts ); \
  }

  /* check which points of both blocks are used in the computation
     such points are above their respective thresholds
  */
#define _MESURE_2D_SSD_FLO_INC_REF_NON_INC(_RMAX_) {             \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
            sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
            npts ++;                                            \
          }                                                     \
        }                                                       \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
            sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
            npts ++;                                            \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    return( sum_diff / npts ); \
  }

#define _MESURE_2D_SSD_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
               (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
            sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
            npts ++;                                            \
          }                                                     \
        }                                                       \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
            sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                         \
    return(  sum_diff / npts ); \
  }

#define _MESURE_2D_SSD_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
               (flo_buf[fpos] < selection_flo->high_threshold) &&     \
               (ref_buf[rpos] > selection_ref->low_threshold) &&      \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
            sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
     }                                                         \
    else {                                                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
            sum_diff += (flo_buf[ fpos ]-ref_buf[ rpos ])*(flo_buf[ fpos ]-ref_buf[ rpos ]); \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                         \
    return( sum_diff / npts ); \
  }

  /* 
     Les 2 blocs sont-ils complets dans les images (par rapport aux seuils sur I) ? 
     
     Sinon, il faut calculer le nombre de voxels actifs communs aux 2 blocs et 
     recalculer moyenne, variance pour le coefficient de correlation
  */

#define _MESURE_2D_SSD(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) _MESURE_2D_SSD_FLO_INC_REF_INC \
        _MESURE_2D_SSD_FLO_INC_REF_NON_INC(_RMAX_) \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _MESURE_2D_SSD_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _MESURE_2D_SSD_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }



  switch ( inrimage_flo->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such floating image type is not handled yet\n", proc );
    return( _SSD_ERROR_VALUE_ );
  case UCHAR :
    {
      unsigned char *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 255, 255 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 32767, 255 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 65535, 255 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case SSHORT :
    {
      short int *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 255, 32767 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 32767, 32767 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 65535, 32767 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case USHORT :
    {
      unsigned short int *flo_buf = inrimage_flo->data;

      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 255, 65535 )
        }
        break;
      case SSHORT :
        {
          signed short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 32767, 65535 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SSD( 65535, 65535 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  } /* switch ( inrimage_flo->type ) */

  return( _SSD_ERROR_VALUE_ );
}






/* Beware, this function return the average sum of square differences

*/
static double SumOfAbsoluteDifferences2D( BLOC *bloc_flo, BLOC *bloc_ref,
                                        bal_image *inrimage_flo,
                                        bal_image *inrimage_ref,
                                        bal_intensitySelection *selection_flo,
                                        bal_intensitySelection *selection_ref,
                                        bal_integerPoint *blockdim )
{
  char *proc = "SumOfAbsoluteDifferences2D";

  int i, j;

  size_t fpos, fposy;
  size_t fdx  = inrimage_flo->ncols;
  size_t rpos, rposy;
  size_t rdx  = inrimage_ref->ncols;

  int a = bloc_flo->origin.x;
  int b = bloc_flo->origin.y;
  int u = bloc_ref->origin.x;
  int v = bloc_ref->origin.y;


  int npts;
  double sum_diff;

  /* all points of both blocks are used in the computation
     they are above their respective thresholds
  */

#define _INTABS(A,B) ( (A) > (B) ? ((int)(A)-(int)(B)) : ((int)(B)-(int)(A)) )

#define _MESURE_2D_SAD_FLO_INC_REF_INC {                       \
    npts = 0; \
    sum_diff = 0.0; \
    for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
      for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
        sum_diff += _INTABS(flo_buf[ fpos ],ref_buf[ rpos ]); \
        npts ++;                                            \
      }                                                       \
    return( sum_diff / npts ); \
  }

  /* check which points of both blocks are used in the computation
     such points are above their respective thresholds
  */
#define _MESURE_2D_SAD_FLO_INC_REF_NON_INC(_RMAX_) {             \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
            sum_diff += _INTABS(flo_buf[ fpos ],ref_buf[ rpos ]); \
            npts ++;                                            \
          }                                                     \
        }                                                       \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
            sum_diff += _INTABS(flo_buf[ fpos ],ref_buf[ rpos ]); \
            npts ++;                                            \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    return( sum_diff / npts ); \
  }

#define _MESURE_2D_SAD_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
               (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
            sum_diff += _INTABS(flo_buf[ fpos ],ref_buf[ rpos ]); \
            npts ++;                                            \
          }                                                     \
        }                                                       \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
            sum_diff += _INTABS(flo_buf[ fpos ],ref_buf[ rpos ]); \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                         \
    return(  sum_diff / npts ); \
  }

#define _MESURE_2D_SAD_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0; \
    sum_diff = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
               (flo_buf[fpos] < selection_flo->high_threshold) &&     \
               (ref_buf[rpos] > selection_ref->low_threshold) &&      \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
            sum_diff += _INTABS(flo_buf[ fpos ],ref_buf[ rpos ]); \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
     }                                                         \
    else {                                                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
            sum_diff += _INTABS(flo_buf[ fpos ],ref_buf[ rpos ]); \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _SSD_ERROR_VALUE_ );         \
    }                                                         \
    return( sum_diff / npts ); \
  }

  /* 
     Les 2 blocs sont-ils complets dans les images (par rapport aux seuils sur I) ? 
     
     Sinon, il faut calculer le nombre de voxels actifs communs aux 2 blocs et 
     recalculer moyenne, variance pour le coefficient de correlation
  */

#define _MESURE_2D_SAD(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) _MESURE_2D_SAD_FLO_INC_REF_INC \
        _MESURE_2D_SAD_FLO_INC_REF_NON_INC(_RMAX_) \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _MESURE_2D_SAD_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _MESURE_2D_SAD_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }



  switch ( inrimage_flo->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such floating image type is not handled yet\n", proc );
    return( _SSD_ERROR_VALUE_ );
  case UCHAR :
    {
      unsigned char *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 255, 255 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 32767, 255 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 65535, 255 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case SSHORT :
    {
      short int *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 255, 32767 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 32767, 32767 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 65535, 32767 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case USHORT :
    {
      unsigned short int *flo_buf = inrimage_flo->data;

      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _SSD_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 255, 65535 )
        }
        break;
      case SSHORT :
        {
          signed short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 32767, 65535 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_SAD( 65535, 65535 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  } /* switch ( inrimage_flo->type ) */

  return( _SSD_ERROR_VALUE_ );
}






/* Beware, this function return the square
   correlation coefficient.
   CC = sum ( (i-E[i])*(j-E[j]) / (SD[i]*SD[j]) )
   with SD[i] = standard deviation de i = sqrt( variance[i] )
*/
static double CorrelationCoefficient2D( BLOC *bloc_flo, BLOC *bloc_ref,
                                        bal_image *inrimage_flo,
                                        bal_image *inrimage_ref,
                                        bal_intensitySelection *selection_flo,
                                        bal_intensitySelection *selection_ref,
                                        bal_integerPoint *blockdim )
{
  char *proc = "CorrelationCoefficient2D";

  int i, j;

  size_t fpos, fposy;
  size_t fdx  = inrimage_flo->ncols;
  size_t rpos, rposy;
  size_t rdx  = inrimage_ref->ncols;

  int a = bloc_flo->origin.x;
  int b = bloc_flo->origin.y;
  int u = bloc_ref->origin.x;
  int v = bloc_ref->origin.y;


  int npts;
  double flo_moy, flo_sum, ref_moy, ref_sum;
  double rho, Ir2xIp2;

  /* all points of both blocks are used in the computation
     they are above their respective thresholds
  */
#define _MESURE_2D_CC_FLO_INC_REF_INC {                       \
    rho = 0.0; flo_moy = bloc_flo->mean; ref_moy = bloc_ref->mean;  \
    for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
      for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
        rho += (flo_buf[ fpos ] - flo_moy) * (ref_buf[ rpos ] - ref_moy); \
      }                                                       \
    Ir2xIp2 = bloc_flo->nxvariance * bloc_ref->nxvariance;                \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

  /* check which points of both blocks are used in the computation
     such points are above their respective thresholds
  */
#define _MESURE_2D_CC_FLO_INC_REF_NON_INC(_RMAX_) {             \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
            flo_sum += flo_buf[ fpos ]; \
            ref_sum += ref_buf[ rpos ]; \
            npts ++;                                            \
          }                                                     \
        }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
            flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
            ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
            rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
          }                                                     \
        }                                                       \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
            flo_sum += flo_buf[ fpos ]; \
            ref_sum += ref_buf[ rpos ]; \
            npts ++;                                            \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
            flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
            ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
            rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
          }                                                   \
        }                                                     \
    }                                                           \
    Ir2xIp2 = flo_sum * ref_sum; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

#define _MESURE_2D_CC_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
               (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
            flo_sum += flo_buf[ fpos ]; \
            ref_sum += ref_buf[ rpos ]; \
            npts ++;                                            \
          }                                                     \
        }                                                       \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
               (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
            flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
            ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
            rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
          }                                                     \
        }                                                       \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
            flo_sum += flo_buf[ fpos ]; \
            ref_sum += ref_buf[ rpos ]; \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
            flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
            ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
            rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
          }                                                   \
        }                                                     \
    }                                                         \
    Ir2xIp2 = flo_sum * ref_sum; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

#define _MESURE_2D_CC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0; \
    flo_moy = ref_moy = 0.0; \
    flo_sum = ref_sum = 0.0; \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
               (flo_buf[fpos] < selection_flo->high_threshold) &&     \
               (ref_buf[rpos] > selection_ref->low_threshold) &&      \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
            flo_sum += flo_buf[ fpos ]; \
            ref_sum += ref_buf[ rpos ]; \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
               (flo_buf[fpos] < selection_flo->high_threshold) &&     \
               (ref_buf[rpos] > selection_ref->low_threshold) &&      \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
            flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
            ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
            rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
          }                                                   \
        }                                                     \
     }                                                         \
    else {                                                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
            flo_sum += flo_buf[ fpos ]; \
            ref_sum += ref_buf[ rpos ]; \
            npts ++;                                          \
          }                                                   \
        }                                                     \
      if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
      flo_moy = flo_sum / (double)npts; \
      ref_moy = ref_sum / (double)npts; \
      rho = flo_sum = ref_sum = 0.0; \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
            flo_sum += (flo_buf[ fpos ] - flo_moy)*(flo_buf[ fpos ] - flo_moy); \
            ref_sum += (ref_buf[ rpos ] - ref_moy)*(ref_buf[ rpos ] - ref_moy); \
            rho     += (flo_buf[ fpos ] - flo_moy)*(ref_buf[ rpos ] - ref_moy); \
          }                                                   \
        }                                                     \
    }                                                         \
    Ir2xIp2 = flo_sum * ref_sum; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / Ir2xIp2 ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

  /* 
     Les 2 blocs sont-ils complets dans les images (par rapport aux seuils sur I) ? 
     
     Sinon, il faut calculer le nombre de voxels actifs communs aux 2 blocs et 
     recalculer moyenne, variance pour le coefficient de correlation
  */

#define _MESURE_2D_CC(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) _MESURE_2D_CC_FLO_INC_REF_INC \
        _MESURE_2D_CC_FLO_INC_REF_NON_INC(_RMAX_) \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _MESURE_2D_CC_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _MESURE_2D_CC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }



  switch ( inrimage_flo->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such floating image type is not handled yet\n", proc );
    return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
  case UCHAR :
    {
      unsigned char *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 255, 255 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 32767, 255 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 65535, 255 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case SSHORT :
    {
      short int *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 255, 32767 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 32767, 32767 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 65535, 32767 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case USHORT :
    {
      unsigned short int *flo_buf = inrimage_flo->data;

      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 255, 65535 )
        }
        break;
      case SSHORT :
        {
          signed short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 32767, 65535 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_CC( 65535, 65535 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  } /* switch ( inrimage_flo->type ) */

  return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
}





static double ExtendedCorrelationCoefficient2D( BLOC *bloc_flo, BLOC *bloc_ref,
                                        bal_image *inrimage_flo,
                                        bal_image *inrimage_ref,
                                        bal_intensitySelection *selection_flo,
                                        bal_intensitySelection *selection_ref,
                                        bal_integerPoint *blockdim )
{
  char *proc = "ExtendedCorrelationCoefficient2D";

  int i, j;

  size_t fpos, fposy;
  size_t fdx  = inrimage_flo->ncols;
  size_t rpos, rposy;
  size_t rdx  = inrimage_ref->ncols;

  int a = bloc_flo->origin.x;
  int b = bloc_flo->origin.y;
  int u = bloc_ref->origin.x;
  int v = bloc_ref->origin.y;


  int npts = blockdim->x * blockdim->y;
  double rho, Ir2xIp2, moyab, moyuv;

  /* all points of both blocks are used in the computation
     they are above their respective thresholds
  */
#define _MESURE_2D_ECC_FLO_INC_REF_INC {                      \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;  \
    for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
      for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
        rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
      }                                                     \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance;                \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

  /* check which points of both blocks are used in the computation
     such points are above their respective thresholds
  */
#define _MESURE_2D_ECC_FLO_INC_REF_NON_INC(_RMAX_) {          \
    npts = 0;                                                 \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;  \
    if ( selection_ref->high_threshold <= _RMAX_ ) {                  \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (ref_buf[rpos] > selection_ref->low_threshold) &&        \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {      \
            rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
            npts ++;                                            \
          }                                                     \
        }                                                       \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( ref_buf[rpos] > selection_ref->low_threshold ) {         \
            rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
            npts ++;                                          \
          }                                                   \
        }                                                     \
    }                                                           \
    if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );         \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

#define _MESURE_2D_ECC_FLO_NON_INC_REF_INC(_FMAX_) {             \
    npts = 0;                                                 \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;  \
    if ( selection_flo->high_threshold <= _FMAX_ ) {                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&        \
               (flo_buf[fpos] < selection_flo->high_threshold) ) {      \
            rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
            npts ++;                                            \
          }                                                     \
        }                                                       \
    }                                                           \
    else {                                                      \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( flo_buf[fpos] > selection_flo->low_threshold ) {         \
            rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
            npts ++;                                            \
          }                                                   \
        }                                                     \
    }                                                         \
    if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );       \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

#define _MESURE_2D_ECC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) {  \
    npts = 0;                                                 \
    rho = 0.0; moyab = bloc_flo->mean; moyuv = bloc_ref->mean;  \
    if ( selection_ref->high_threshold <= _RMAX_ && selection_flo->high_threshold <= _FMAX_ ) { \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) &&      \
               (flo_buf[fpos] < selection_flo->high_threshold) &&     \
               (ref_buf[rpos] > selection_ref->low_threshold) &&      \
               (ref_buf[rpos] < selection_ref->high_threshold) ) {    \
            rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
            npts ++;                                            \
          }                                                   \
        }                                                     \
    }                                                         \
    else {                                                    \
      for (j = 0, fposy = b*fdx, rposy = v*rdx; j < blockdim->y; j++, fposy += fdx, rposy += rdx) \
        for (i = 0, fpos = fposy + a, rpos = rposy + u; i < blockdim->x; i++, fpos++, rpos++ ) { \
          if ( (flo_buf[fpos] > selection_flo->low_threshold) && (ref_buf[rpos] > selection_ref->low_threshold) ) { \
            rho += (flo_buf[ fpos ] - moyab) * (ref_buf[ rpos ] - moyuv); \
            npts ++;                                            \
          }                                                   \
        }                                                     \
    }                                                         \
    if ( npts == 0 ) return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );       \
    Ir2xIp2 = bloc_flo->variance * bloc_ref->variance; \
    return( ( Ir2xIp2 > EPSILON ) ? ( (rho*rho) / (npts*npts*Ir2xIp2) ) : _CORRELATION_COEFFICIENT_ERROR_VALUE_ ); \
  }

  /* 
     Les 2 blocs sont-ils complets dans les images (par rapport aux seuils sur I) ? 
     
     Sinon, il faut calculer le nombre de voxels actifs communs aux 2 blocs et 
     recalculer moyenne, variance pour le coefficient de correlation
  */

#define _MESURE_2D_ECC(_RMAX_,_FMAX_) {            \
    if ( bloc_flo->inclus ) {                     \
      if ( bloc_ref->inclus ) _MESURE_2D_ECC_FLO_INC_REF_INC \
        _MESURE_2D_ECC_FLO_INC_REF_NON_INC(_RMAX_) \
    }                                             \
    if ( bloc_ref->inclus ) {                     \
      _MESURE_2D_ECC_FLO_NON_INC_REF_INC(_FMAX_)   \
    }                                             \
    _MESURE_2D_ECC_FLO_NON_INC_REF_NON_INC(_RMAX_,_FMAX_) \
  }



  switch ( inrimage_flo->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such floating image type is not handled yet\n", proc );
    return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
  case UCHAR :
    {
      unsigned char *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 255, 255 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 32767, 255 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 65535, 255 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case SSHORT :
    {
      short int *flo_buf = inrimage_flo->data;
      
      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 255, 32767 )
        }
        break;
      case SSHORT :
        {
          short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 32767, 32767 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 65535, 32767 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  case USHORT :
    {
      unsigned short int *flo_buf = inrimage_flo->data;

      switch ( inrimage_ref->type ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such reference image type is not handled yet\n", proc );
        return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
      case UCHAR :
        {
          unsigned char *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 255, 65535 )
        }
        break;
      case SSHORT :
        {
          signed short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 32767, 65535 )
        }
        break;
      case USHORT :
        {
          unsigned short int *ref_buf = inrimage_ref->data;
          _MESURE_2D_ECC( 65535, 65535 )
        }
        break;
      } /* switch ( inrimage_ref->type ) */
    }
    break;
  } /* switch ( inrimage_flo->type ) */

  return( _CORRELATION_COEFFICIENT_ERROR_VALUE_ );
}





/***
    Calcul de la mesure de similarite entre B(a,b) et B(u,v), a, b, u, v origines des blocs
***/
double BAL_ComputeBlockSimilarity2D( BLOC *bloc_flo, BLOC *bloc_ref,
                                     bal_image *image_flo, bal_image *image_ref,
                                     bal_intensitySelection *selection_flo,
                                     bal_intensitySelection *selection_ref,
                                     bal_integerPoint *blockdim,
                                     enumTypeSimilarity mesure )
{
  char *proc = "BAL_ComputeBlockSimilarity2D";
  switch ( mesure ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such measure is not implemented yet\n", proc );
    return( _MISC_ERROR_VALUE_ );

  case _SSD_ :
    return( SumOfSquareDifferences2D( bloc_flo, bloc_ref, image_flo, image_ref,
                                                          selection_flo, selection_ref, blockdim ) );

  case _SAD_ :
    return( SumOfAbsoluteDifferences2D( bloc_flo, bloc_ref, image_flo, image_ref,
                                        selection_flo, selection_ref, blockdim ) );

  case _SQUARED_CC_ :
    return( CorrelationCoefficient2D( bloc_flo, bloc_ref, image_flo, image_ref,
                                      selection_flo, selection_ref, blockdim ) );

  case _SQUARED_EXTCC_ :
    return( ExtendedCorrelationCoefficient2D( bloc_flo, bloc_ref, image_flo, image_ref,
                                      selection_flo, selection_ref, blockdim ) );
  
    /* A REVOIR PLUS TARD... E.B. 26/04/2002
      case MESURE_CR: 
      for (i=0;i<256;i++) {
      cond.sum[i] = 0;
      cond.square[i] = 0;
      cond.Npoints[i] = 0;
      }
      cond.Ntotal = 0;
      cond.sum_total = 0;
      cond.square_total = 0;
      
      for (i = 0; i < blockdim->x; i++)
      for (j = 0, 
      pos      = u + i + v*rdx,
      pos_ref  = a + i + b*fdx;
      j < blockdim->y; 
      j++, 
      pos     += dx_ref,
      pos_ref += dx_ref) {
      
      i_ref_bas  = max(selection_flo->low_threshold,  flo_buf[pos_ref]);
      i_ref_haut = min(selection_flo->high_threshold, flo_buf[pos_ref]);
      i_flo_bas  = max(selection_ref->low_threshold,  ref_buf[pos]);
      i_flo_haut = min(selection_ref->high_threshold, ref_buf[pos]);
      
      if (i_ref_bas == selection_flo->low_threshold)
      nb_voxels_seuil_flo++;
      if (i_ref_haut == selection_flo->high_threshold) 
      nb_voxels_seuil_flo++;
      if (i_flo_bas == selection_ref->low_threshold)
      nb_voxels_seuil_ref++;
      if (i_flo_haut == selection_ref->high_threshold) 
      nb_voxels_seuil_ref++;
      
      if ( (nb_voxels_seuil_flo < nb_voxels_max_flo) &&
      (nb_voxels_seuil_ref < nb_voxels_max_ref) ) { 
      
      cond.sum[flo_buf[pos_ref]]     += (double)ref_buf[pos];
      cond.square[flo_buf[pos_ref]]  += (double)ref_buf[pos] * (double)ref_buf[pos];
      cond.Npoints[flo_buf[pos_ref]] += 1.0;
      cond.sum_total                 += (double)ref_buf[pos];
      cond.square_total              += (double)ref_buf[pos] * (double)ref_buf[pos];
      cond.Ntotal                    += 1.0;
      } 
      else {
      return( RETURNED_VALUE_ON_ERROR ); 
      }
      }
      
      sigma = cond.square_total - (cond.sum_total*cond.sum_total)/cond.Ntotal;
      
      for(i=0;i<256;i++)
      if(cond.Npoints[i]!=0)
      PiSi += cond.square[i] - (cond.sum[i]*cond.sum[i])/cond.Npoints[i]; 
      
      return (1 - PiSi/sigma);
      
      break;
    */
  } /* switch (param->mesure) */
  
  return( _MISC_ERROR_VALUE_ );
}













/*************************************************************
 *
 * Precomputation of some values for blocks
 * -> done for the 'data' field of BLOCS, ie all the blocks
 *
 *************************************************************/



typedef struct _BlockAttributesParam {
  bal_image *inrimage;
  BLOCS *blocks;
} _BlockAttributesParam;





int _ComputeBlockAttributesWithNoBorders2D ( bal_image *inrimage, 
                                             BLOCS *blocs,
                                             size_t first,
                                             size_t last )
{
  char *proc = "_ComputeBlockAttributesWithNoBorders2D";

  size_t n;
  int n_passive_voxels;  

  int n_max_passive_voxels = (int)
    ( (blocs->selection.max_removed_fraction) * (double)blocs->blockdim.x * (double)blocs->blockdim.y + 0.5);
  int seuil_bas = blocs->selection.low_threshold;
  int seuil_haut = blocs->selection.high_threshold;

  int n_pts = blocs->blockdim.x * blocs->blockdim.y;

  size_t dimx = inrimage->ncols;
  int a, b, i, j;
  int posx, posy;

  double sum, moy;
  
  /* it is implicitly assumed that blocks are included
     in the image.
     Do tests on the intensity value only when needed.
  */

#define _ATTRIBUTES_2D_NO_TEST_ { \
    for ( n=first; n<=last; n++ ) { \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      blocs->data[n].inclus = 1; \
      blocs->data[n].valid = 1; \
      moy = sum = 0.0; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) \
        sum += buf[posx]; \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) \
        sum += (buf[posx] - moy)*(buf[posx] - moy); \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define _ATTRIBUTES_2D_TEST_MIN_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      n_pts = 0;   moy = sum = 0.0; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas ) n_passive_voxels ++; \
        else { sum += buf[posx];   n_pts ++; } \
      } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas ) continue; \
        sum += (buf[posx] - moy)*(buf[posx] - moy); \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define _ATTRIBUTES_2D_TEST_MIN_MAX_ {  \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      n_pts = 0;   moy = sum = 0.0; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) n_passive_voxels ++; \
        else { sum += buf[posx];   n_pts ++; } \
      } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) continue; \
        sum += (buf[posx] - moy)*(buf[posx] - moy); \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define  _ATTRIBUTES_2D_(_MIN_,_MAX_) { \
    if ( seuil_bas < _MIN_ && seuil_haut > _MAX_ ) { \
      _ATTRIBUTES_2D_NO_TEST_ \
    } \
    else if ( seuil_haut > _MAX_ ) { \
      _ATTRIBUTES_2D_TEST_MIN_ \
    } \
    else { \
      _ATTRIBUTES_2D_TEST_MIN_MAX_ \
    } \
  }

  switch ( inrimage->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  case UCHAR : 
    {
      unsigned char *buf = inrimage->data;
      _ATTRIBUTES_2D_( 0, 255 )
    }
    break;
  case USHORT : 
    {
      unsigned short int *buf = inrimage->data;
      _ATTRIBUTES_2D_( 0, 65535 )
    }
    break;
  case SSHORT : 
    {
      short int *buf = inrimage->data;
      _ATTRIBUTES_2D_( -32768, 32767 )
    }
    break;
  }

  return( 1 );
}





int _ComputeBlockAttributesWithBorders2D (bal_image *inrimage, 
                                             BLOCS *blocs,
                                             size_t first,
                                             size_t last )
{
  char *proc = "_ComputeBlockAttributesWithBorders2D";

  size_t n;
  int n_passive_voxels;
  
  int n_max_passive_voxels = (int)
    ( (blocs->selection.max_removed_fraction) * (double)blocs->blockdim.x * (double)blocs->blockdim.y + 0.5);
  int seuil_bas = blocs->selection.low_threshold;
  int seuil_haut = blocs->selection.high_threshold;

  int n_pts;

  int dimx = inrimage->ncols;
  int dimxy = dimx * inrimage->nrows;
  int a, b, i, j;
  int posx, posy;

  double sum, moy;
  
  /* it is implicitly assumed that blocks are included
     in the image.
     Do tests on the intensity value only when needed.
  */

#define _ATTRIBUTES_2D_WB_NO_TEST_ { \
    for ( n=first; n<=last; n++ ) { \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      blocs->data[n].inclus = 1; \
      blocs->data[n].valid = 1; \
      n_pts = 0;   moy = sum = 0.0; \
      for ( j = 0, posy = (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
        if ( posy < 0 || posy >= dimxy ) continue; \
        for ( i = 0, posx = posy + a - blocs->border.x; \
              i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
          if ( posx < 0 || posx >= dimxy ) continue; \
          sum += buf[posx]; \
          n_pts ++; \
        } \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for ( j = 0, posy = (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
        if ( posy < 0 || posy >= dimxy ) continue; \
        for ( i = 0, posx = posy + a - blocs->border.x; \
              i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
          if ( posx < 0 || posx >= dimxy ) continue; \
          sum += (buf[posx] - moy)*(buf[posx] - moy); \
        } \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define _ATTRIBUTES_2D_WB_TEST_MIN_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
        for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
          if ( buf[posx] <= seuil_bas ) n_passive_voxels ++; \
        } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      n_pts = 0;   moy = sum = 0.0; \
      for ( j = 0, posy = (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
        if ( posy < 0 || posy >= dimxy ) continue; \
        for ( i = 0, posx = posy + a - blocs->border.x; \
              i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
          if ( posx < 0 || posx >= dimxy ) continue; \
          if ( buf[posx] <= seuil_bas ) continue; \
          sum += buf[posx]; \
          n_pts ++; \
        } \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for ( j = 0, posy = (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
        if ( posy < 0 || posy >= dimxy ) continue; \
        for ( i = 0, posx = posy + a - blocs->border.x; \
              i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
          if ( posx < 0 || posx >= dimxy ) continue; \
          if ( buf[posx] <= seuil_bas ) continue; \
          sum += (buf[posx] - moy)*(buf[posx] - moy); \
        } \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define _ATTRIBUTES_2D_WB_TEST_MIN_MAX_ {  \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      for (j = 0, posy = b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
        for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
          if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut )  \
            n_passive_voxels ++; \
        } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      n_pts = 0;   moy = sum = 0.0; \
      for ( j = 0, posy = (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
        if ( posy < 0 || posy >= dimxy ) continue; \
        for ( i = 0, posx = posy + a - blocs->border.x; \
              i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
          if ( posx < 0 || posx >= dimxy ) continue; \
          if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut )  \
          sum += buf[posx]; \
          n_pts ++; \
        } \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for ( j = 0, posy = (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
        if ( posy < 0 || posy >= dimxy ) continue; \
        for ( i = 0, posx = posy + a - blocs->border.x; \
              i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
          if ( posx < 0 || posx >= dimxy ) continue; \
          if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut )  \
          sum += (buf[posx] - moy)*(buf[posx] - moy); \
        } \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define  _ATTRIBUTES_2D_WB_(_MIN_,_MAX_) { \
    if ( seuil_bas < _MIN_ && seuil_haut > _MAX_ ) { \
      _ATTRIBUTES_2D_WB_NO_TEST_ \
    } \
    else if ( seuil_haut > _MAX_ ) { \
      _ATTRIBUTES_2D_WB_TEST_MIN_ \
    } \
    else { \
      _ATTRIBUTES_2D_WB_TEST_MIN_MAX_ \
    } \
  }

  switch ( inrimage->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  case UCHAR : 
    {
      unsigned char *buf = inrimage->data;
      _ATTRIBUTES_2D_WB_( 0, 255 )
    }
    break;
  case USHORT : 
    {
      unsigned short int *buf = inrimage->data;
      _ATTRIBUTES_2D_WB_( 0, 65535 )
    }
    break;
  case SSHORT : 
    {
      short int *buf = inrimage->data;
      _ATTRIBUTES_2D_WB_( -32768, 32767 )
    }
    break;
  }

  return( 1 );
}





/* procedure for parallelism
 */

void *_ComputeBlockAttributes2D( void *par ) 
{
  char *proc = "_ComputeBlockAttributes2D";
  typeChunk *c = (typeChunk *)par;
  _BlockAttributesParam *p = ( _BlockAttributesParam*)(c->parameters);

  if ( p->blocks->border.x < 0 || p->blocks->border.y < 0 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: negative values of blocks borders\n", proc );
    c->ret = 0;
    return( (void*)NULL );
  }

  if ( p->blocks->border.x == 0 && p->blocks->border.y == 0 ) {
    c->ret = _ComputeBlockAttributesWithNoBorders2D( p->inrimage, p->blocks, c->first, c->last );
    return( (void*)NULL );
  }
  
  c->ret = _ComputeBlockAttributesWithBorders2D( p->inrimage, p->blocks, c->first, c->last );
  return( (void*)NULL );
  
}





int _ComputeBlockAttributesWithNoBorders3D ( bal_image *inrimage, 
                                                BLOCS *blocs,
                                                size_t first,
                                                size_t last )
{
  char *proc = "_ComputeBlockAttributesWithNoBorders3D";

  size_t n;
  int n_passive_voxels = 0;  

  int n_max_passive_voxels = (int)
    ( (blocs->selection.max_removed_fraction) * (double)blocs->blockdim.x 
      * (double)blocs->blockdim.y * (double)blocs->blockdim.z + 0.5);
  int seuil_bas = blocs->selection.low_threshold;
  int seuil_haut = blocs->selection.high_threshold;

  int n_pts = blocs->blockdim.x * blocs->blockdim.y * blocs->blockdim.z;

  size_t dimx = inrimage->ncols;
  size_t dimxy = dimx * inrimage->nrows;
  int a, b, c, i, j, k;
  int posx, posy, posz;

  double sum, sum_2, moy;



  /* it is implicitly assumed that blocks are included
     in the image.
     Do tests on the intensity value only when needed.
  */

#define _ATTRIBUTES_3D_NO_TEST_ { \
    for ( n=first; n<=last; n++ ) { \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      blocs->data[n].inclus = 1; \
      blocs->data[n].valid = 1; \
      moy = sum = 0.0; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
      for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) \
        sum += buf[posx]; \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
      for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) \
        sum += (buf[posx] - moy)*(buf[posx] - moy); \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }



#define _DIRECTATTRIBUTES_3D_NO_TEST_ { \
    for ( n=first; n<=last; n++ ) { \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      blocs->data[n].inclus = 1; \
      blocs->data[n].valid = 1; \
      sum = sum_2 = 0.0; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
      for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) {  \
        sum += buf[posx];               \
        sum_2 += buf[posx] * buf[posx]; \
      } \
      blocs->data[n].mean = sum / (double)n_pts; \
      blocs->data[n].nxvariance = sum_2 - (sum * sum) / (double)n_pts; \
      blocs->data[n].variance  = blocs->data[n].nxvariance / (double)n_pts; \
    } \
  }



#define _ATTRIBUTES_3D_TEST_MIN_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      n_pts = 0;   moy = sum = 0.0; \
      for (i = 0; i<blocs->blockdim.x; i++ ) \
      for (j = 0, posy = a + i + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (k = 0, posx = posy + c*dimxy; k<blocs->blockdim.z; k++, posx+=dimxy ) {  \
        if ( buf[posx] <= seuil_bas ) n_passive_voxels ++; \
        else { sum += buf[posx];   n_pts ++; } \
      } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
      for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas ) continue; \
        sum += (buf[posx] - moy)*(buf[posx] - moy); \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }



#define _DIRECTATTRIBUTES_3D_TEST_MIN_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      n_pts = 0;   sum = sum_2 = 0.0; \
      for (i = 0; i<blocs->blockdim.x; i++ ) \
      for (j = 0, posy = a + i + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (k = 0, posx = posy + c*dimxy; k<blocs->blockdim.z; k++, posx+=dimxy ) {  \
        if ( buf[posx] <= seuil_bas ) n_passive_voxels ++; \
        else { \
          sum += buf[posx];               \
          sum_2 += buf[posx] * buf[posx]; \
          n_pts ++; }                          \
      } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = sum / (double)n_pts; \
      blocs->data[n].nxvariance = sum_2 - (sum * sum) / (double)n_pts; \
      blocs->data[n].variance  = blocs->data[n].nxvariance / (double)n_pts; \
    } \
  }



#define _ATTRIBUTES_3D_TEST_MIN_MAX_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      n_pts = 0;   moy = sum = 0.0; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
      for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) n_passive_voxels ++; \
        else { sum += buf[posx];   n_pts ++; } \
      } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
      for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) continue; \
        sum += (buf[posx] - moy)*(buf[posx] - moy); \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }



#define _DIRECTATTRIBUTES_3D_TEST_MIN_MAX_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      n_pts = 0;   sum = sum_2 = 0.0; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
      for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
      for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
        if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) n_passive_voxels ++; \
        else { \
          sum += buf[posx];               \
          sum_2 += buf[posx] * buf[posx]; \
          n_pts ++; }                          \
      } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = sum / (double)n_pts; \
      blocs->data[n].nxvariance = sum_2 - (sum * sum) / (double)n_pts; \
      blocs->data[n].variance  = blocs->data[n].nxvariance / (double)n_pts; \
    } \
  }



#define  _ATTRIBUTES_3D_(_MIN_,_MAX_) { \
    if ( seuil_bas < _MIN_ && seuil_haut > _MAX_ ) { \
       _ATTRIBUTES_3D_NO_TEST_ \
    } \
    else if ( seuil_haut > _MAX_ ) { \
       _ATTRIBUTES_3D_TEST_MIN_ \
    } \
    else { \
       _ATTRIBUTES_3D_TEST_MIN_MAX_ \
    } \
  }



#define  _DIRECTATTRIBUTES_3D_(_MIN_,_MAX_) { \
    if ( seuil_bas < _MIN_ && seuil_haut > _MAX_ ) { \
       _DIRECTATTRIBUTES_3D_NO_TEST_ \
    } \
    else if ( seuil_haut > _MAX_ ) { \
       _DIRECTATTRIBUTES_3D_TEST_MIN_ \
    } \
    else { \
       _DIRECTATTRIBUTES_3D_TEST_MIN_MAX_ \
    } \
  }



  switch ( inrimage->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  case UCHAR : 
    {
      unsigned char *buf = (unsigned char*)inrimage->data;
      _DIRECTATTRIBUTES_3D_( 0, 255 )
    }
    break;
  case USHORT : 
    {
      unsigned short int *buf = (unsigned short int*)inrimage->data;
      _ATTRIBUTES_3D_( 0, 65535 )
    }
    break;
  case SSHORT : 
    {
      short int *buf = (short int*)inrimage->data;
      _ATTRIBUTES_3D_( -32768, 32767 )
    }
    break;
  }
  
  return( 1 );
}





int _ComputeBlockAttributesWithBorders3D ( bal_image *inrimage, 
                                           BLOCS *blocs,
                                           size_t first,
                                           size_t last )
{
  char *proc = "_ComputeBlockAttributesWithBorders3D";

  size_t n;
  int n_passive_voxels = 0;  

  int n_max_passive_voxels = (int)
    ( (blocs->selection.max_removed_fraction) * (double)blocs->blockdim.x 
      * (double)blocs->blockdim.y * (double)blocs->blockdim.z + 0.5);
  int seuil_bas = blocs->selection.low_threshold;
  int seuil_haut = blocs->selection.high_threshold;

  int n_pts;

  size_t dimx = inrimage->ncols;
  size_t dimxy = dimx * inrimage->nrows;
  size_t dimxyz = dimxy * inrimage->nplanes;
  int a, b, c, i, j, k;
  long int posx, posy, posz;

  double sum, moy;
  
  /* it is implicitly assumed that blocks are included
     in the image.
     Do tests on the intensity value only when needed.
  */

#define _ATTRIBUTES_3D_WB_NO_TEST_ { \
    for ( n=first; n<=last; n++ ) { \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      blocs->data[n].inclus = 1; \
      blocs->data[n].valid = 1; \
      n_pts = 0;   moy = sum = 0.0; \
      for ( k = 0, posz = (c-blocs->border.z)*dimxy; \
            k<blocs->blockdim.z+2*blocs->border.z; k++, posz+=dimxy ) { \
        if ( posz < 0 || posz >= (long int)dimxyz ) continue; \
        for ( j = 0, posy = posz + (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
          if ( posy < 0 || posy >= (long int)dimxyz ) continue; \
          for ( i = 0, posx = posy + a - blocs->border.x; \
                i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
            if ( posx < 0 || posx >= (long int)dimxyz ) continue; \
            sum += buf[posx]; \
            n_pts ++; \
          } \
        } \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for ( k = 0, posz = (c-blocs->border.z)*dimxy; \
            k<blocs->blockdim.z+2*blocs->border.z; k++, posz+=dimxy ) { \
        if ( posz < 0 || posz >= (long int)dimxyz ) continue; \
        for ( j = 0, posy = posz + (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
          if ( posy < 0 || posy >= (long int)dimxyz ) continue; \
          for ( i = 0, posx = posy + a - blocs->border.x; \
                i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
            if ( posx < 0 || posx >= (long int)dimxyz ) continue; \
            sum += (buf[posx] - moy)*(buf[posx] - moy); \
          } \
        } \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define _ATTRIBUTES_3D_WB_TEST_MIN_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
        for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
          for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
            if ( buf[posx] <= seuil_bas ) n_passive_voxels ++; \
          } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      n_pts = 0;   moy = sum = 0.0; \
      for ( k = 0, posz = (c-blocs->border.z)*dimxy; \
            k<blocs->blockdim.z+2*blocs->border.z; k++, posz+=dimxy ) { \
        if ( posz < 0 || posz >= (long int)dimxyz ) continue; \
        for ( j = 0, posy = posz + (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
          if ( posy < 0 || posy >= (long int)dimxyz ) continue; \
          for ( i = 0, posx = posy + a - blocs->border.x; \
                i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
            if ( posx < 0 || posx >= (long int)dimxyz ) continue; \
            if ( buf[posx] <= seuil_bas ) continue; \
            sum += buf[posx]; \
            n_pts ++; \
          } \
        } \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for ( k = 0, posz = (c-blocs->border.z)*dimxy; \
            k<blocs->blockdim.z+2*blocs->border.z; k++, posz+=dimxy ) { \
        if ( posz < 0 || posz >= (long int)dimxyz ) continue; \
        for ( j = 0, posy = posz + (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
          if ( posy < 0 || posy >= (long int)dimxyz ) continue; \
          for ( i = 0, posx = posy + a - blocs->border.x; \
                i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
            if ( posx < 0 || posx >= (long int)dimxyz ) continue; \
            if ( buf[posx] <= seuil_bas ) continue; \
            sum += (buf[posx] - moy)*(buf[posx] - moy); \
          } \
        } \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define _ATTRIBUTES_3D_WB_TEST_MIN_MAX_ { \
    for ( n=first; n<=last; n++ ) { \
      n_passive_voxels = 0; \
      a = blocs->data[n].origin.x; \
      b = blocs->data[n].origin.y; \
      c = blocs->data[n].origin.z; \
      for (k = 0, posz = c*dimxy; k<blocs->blockdim.z; k++, posz+=dimxy ) \
        for (j = 0, posy = posz + b*dimx; j<blocs->blockdim.y; j++, posy+=dimx ) \
          for (i = 0, posx = posy + a; i<blocs->blockdim.x; i++, posx ++ ) { \
            if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) n_passive_voxels ++; \
          } \
      blocs->data[n].inclus = ( n_passive_voxels == 0 ) ? 1 : 0; \
      blocs->data[n].valid = ( n_passive_voxels < n_max_passive_voxels ) ? 1 : 0; \
      if ( blocs->data[n].valid == 0 ) { \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      n_pts = 0;   moy = sum = 0.0; \
      for ( k = 0, posz = (c-blocs->border.z)*dimxy; \
            k<blocs->blockdim.z+2*blocs->border.z; k++, posz+=dimxy ) { \
        if ( posz < 0 || posz >= (long int)dimxyz ) continue; \
        for ( j = 0, posy = posz + (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
          if ( posy < 0 || posy >= (long int)dimxyz ) continue; \
          for ( i = 0, posx = posy + a - blocs->border.x; \
                i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
            if ( posx < 0 || posx >= (long int)dimxyz ) continue; \
            if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) continue; \
            sum += buf[posx]; \
            n_pts ++; \
          } \
        } \
      } \
      if ( n_pts == 0 ) { \
        blocs->data[n].valid = 0; \
        blocs->data[n].variance = 0.0; \
        continue; \
      } \
      blocs->data[n].mean  = moy = sum / (double)n_pts; \
      sum = 0.0; \
      for ( k = 0, posz = (c-blocs->border.z)*dimxy; \
            k<blocs->blockdim.z+2*blocs->border.z; k++, posz+=dimxy ) { \
        if ( posz < 0 || posz >= (long int)dimxyz ) continue; \
        for ( j = 0, posy = posz + (b-blocs->border.y)*dimx; \
              j<blocs->blockdim.y+2*blocs->border.y; j++, posy+=dimx ) { \
          if ( posy < 0 || posy >= (long int)dimxyz ) continue; \
          for ( i = 0, posx = posy + a - blocs->border.x; \
                i<blocs->blockdim.x+2*blocs->border.x; i++, posx ++ ) { \
            if ( posx < 0 || posx >= (long int)dimxyz ) continue; \
            if ( buf[posx] <= seuil_bas || buf[posx] >= seuil_haut ) continue; \
            sum += (buf[posx] - moy)*(buf[posx] - moy); \
          } \
        } \
      } \
      blocs->data[n].nxvariance = sum; \
      blocs->data[n].variance  = sum / (double)n_pts; \
    } \
  }

#define  _ATTRIBUTES_3D_WB_(_MIN_,_MAX_) { \
    if ( seuil_bas < _MIN_ && seuil_haut > _MAX_ ) { \
      _ATTRIBUTES_3D_WB_NO_TEST_ \
    } \
    else if ( seuil_haut > _MAX_ ) { \
      _ATTRIBUTES_3D_WB_TEST_MIN_ \
    } \
    else { \
      _ATTRIBUTES_3D_WB_TEST_MIN_MAX_ \
    } \
  }

  switch ( inrimage->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  case UCHAR : 
    {
      unsigned char *buf = (unsigned char*)inrimage->data;
      _ATTRIBUTES_3D_WB_( 0, 255 )
    }
    break;
  case USHORT : 
    {
      unsigned short int *buf = (unsigned short int*)inrimage->data;
      _ATTRIBUTES_3D_WB_( 0, 65535 )
    }
    break;
  case SSHORT : 
    {
      short int *buf = (short int*)inrimage->data;
      _ATTRIBUTES_3D_WB_( -32768, 32767 )
    }
    break;
  }

  return( 1 );
}





/* procedure for parallelism
 */

void *_ComputeBlockAttributes3D( void *par ) 
{
  char *proc = "_ComputeBlockAttributes3D";
  typeChunk *c = (typeChunk *)par;
  _BlockAttributesParam *p = ( _BlockAttributesParam*)(c->parameters);

  if ( p->blocks->border.x < 0 || p->blocks->border.y < 0 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: negative values of blocks borders\n", proc );
    c->ret = 0;
    return( (void*)NULL );
  }

  if ( p->blocks->border.x == 0 && p->blocks->border.y == 0 ) {
    c->ret = _ComputeBlockAttributesWithNoBorders3D( p->inrimage, p->blocks, c->first, c->last );
    return( (void*)NULL );
  }
  
  c->ret = _ComputeBlockAttributesWithBorders3D( p->inrimage, p->blocks, c->first, c->last );
  return( (void*)NULL );
  
}





/***
    Calcul des attributs des blocs associes a une image a un niveau donne de la pyramide 
    - chapeau du calcul 2D ou 3D
***/

int BAL_ComputeBlockAttributes(bal_image *inrimage, BLOCS *blocks )
{
  char *proc = "BAL_ComputeBlockAttributes";
  size_t n;
  int i;

  _BlockAttributesParam p;
  typeChunks chunks;
  size_t first, last;



  /* preparation for parallelism computing with openmp
     1. chunks calculation
     2. parameter preparation
   */

  first = 0;
  last = blocks->n_allocated_blocks-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
 
  p.inrimage = inrimage;
  p.blocks = blocks;

  for ( i=0; i<chunks.n_allocated_chunks; i++ ) 
    chunks.data[i].parameters = (void*)(&p);

  

  /* computation
   */

  if ( inrimage->nplanes == 1 ) {

    if ( processChunks( &_ComputeBlockAttributes2D, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute block attributes (2D case)\n", proc );
      freeChunks( &chunks );
      return( RETURNED_VALUE_ON_ERROR );
    }
    
  } /* end of 2D case */
  else {

    if ( processChunks( &_ComputeBlockAttributes3D, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute block attributes (3D case)\n", proc );
      freeChunks( &chunks );
      return( RETURNED_VALUE_ON_ERROR );
    }

  }  /* end of 3D case */


  
  freeChunks( &chunks );



#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
  /* the formulas for the number of blocks was wrong
     some of the new blocks have to to discarded
     to get exactly the same behavior
     WARNING: this behavior discards all blocks if the block
     size (along Z) is equal to the Z dimension of image
  */
  if ( inrimage->nplanes == 1 ) {
    
    for ( n=0; n<blocks->n_allocated_blocks; n++ ) {
      if ( blocks->data[n].origin.x >= inrimage->ncols - blocks->blockdim.x 
           || blocks->data[n].origin.y >= inrimage->nrows - blocks->blockdim.y ) {
        if ( blocks->data[n].valid ) {
          blocks->data[n].valid = 0;
          blocks->data[n].variance  = 0;
        }
      }
    }
  
  } 
  else {
    
    for ( n=0; n<blocks->n_allocated_blocks; n++ ) {
      if ( blocks->data[n].origin.x >= inrimage->ncols - blocks->blockdim.x 
           || blocks->data[n].origin.y >= inrimage->nrows - blocks->blockdim.y
           || blocks->data[n].origin.z >= inrimage->nplanes - blocks->blockdim.z) {
        if ( blocks->data[n].valid ) {
          blocks->data[n].valid = 0;
          blocks->data[n].variance  = 0;
        }
      }
    }

  }
#endif

  for (  blocks->n_valid_blocks=0, n=0; n<blocks->n_allocated_blocks; n++ )
    if ( blocks->data[n].valid ) blocks->n_valid_blocks ++;

  return( 1 );
}
