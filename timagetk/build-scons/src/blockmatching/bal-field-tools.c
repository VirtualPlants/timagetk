/*************************************************************************
 * bal-field-tools.c -
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

#include <bal-field-tools.h>
#include <bal-block-tools.h>
#include <bal-behavior.h>





static int _verbose_ = 1;
static int _debug_ = 0;

static FILE *_verbosef_ = NULL;


void BAL_SetVerboseFileInBalFieldTools( FILE *f )
{
  _verbosef_ = f;
}

void BAL_SetVerboseInBalFieldTools( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalFieldTools(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalFieldTools(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}

void BAL_SetDebugInBalFieldTools( int d )
{
  _debug_ = d;
}

void BAL_IncrementDebugInBalFieldTools(  )
{
  _debug_ ++;
}

void BAL_DecrementDebugInBalFieldTools(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}






/* Debug Notes
   -----------
 in CalculChampVecteur3D
 - change output/error writing
 - block index computation have to be coherent with
   Allocate_Blocks() in py_image.c
   -> there was an cut and paste error for NULL displacement
      

 in _ComputePairingField2D
 - change output/error writing
 - block index computation have to be coherent with
   Allocate_Blocks() in py_image.c
   -> there was an cut and paste error for NULL displacement

in CalculAttributsBlocsWithNoBorders_2D
- old formula for the calculation of the number of blocks
-> the original number value is set, but it seems that there is 
   no guarantee that the discarded blocks (the last ones) are the ones
   that should be discarded ...
 
in CalculAttributsBlocsWithBorders_2D
- old formula for the calculation of the number of blocks
-> the original number value is set, but it seems that there is 
   no guarantee that the discarded blocks (the last ones) are the ones
   that should be discarded ...
 
in CalculAttributsBlocsWithNoBorders_3D
- old formula for the calculation of the number of blocks
-> the original number value is set, but it seems that there is 
   no guarantee that the discarded blocks (the last ones) are the ones
   that should be discarded ...
 
in CalculAttributsBlocsWithBorders_3D
- old formula for the calculation of the number of blocks
-> the original number value is set, but it seems that there is 
   no guarantee that the discarded blocks (the last ones) are the ones
   that should be discarded ...
 

 */







/**************************************************
 *
 * Misc
 *
 **************************************************/

int _CompareResiduals ( const void * a, const void * b )
{
  typeScalarWeightedDisplacement *ad = *( (typeScalarWeightedDisplacement **) a);
  typeScalarWeightedDisplacement *bd = *( (typeScalarWeightedDisplacement **) b);

  if ( ad->error  >  bd->error )
    return( 1 );
  if ( ad->error  <  bd->error )
    return( -1 );
  return( 0 );
}



int BAL_SelectSmallestResiduals( FIELD *field,
                                 bal_estimator *estimator )
{
  char *proc = "BAL_SelectSmallestResiduals";

  size_t selectedresiduals, h;
  
  size_t last, left, right; 
  size_t i, j, k;

  double m, s, t;

  typeScalarWeightedDisplacement *tmp;

  /* no selection at the beginning
   */
  selectedresiduals = field->n_computed_pairs;

  /* selection on error distribution
   */
  if ( estimator->standard_deviation_threshold > 0.0 ) {

    m = s = 0.0;
    for ( m=0.0, i=0; i<field->n_computed_pairs; i++ )
      m += field->pointer[i]->error;
    m /= field->n_computed_pairs;
    for ( s=0.0, i=0; i<field->n_computed_pairs; i++ )
      s += (field->pointer[i]->error - m) * (field->pointer[i]->error - m);
    s /= field->n_computed_pairs;
    s = sqrt( s );

    /* threshold
     */
    t = m + estimator->standard_deviation_threshold * s;

    if ( _debug_ ) {
      fprintf( stderr, "%s: selection on error distribution\n", proc );
      fprintf( stderr, "m=%f, s=%f, threshold=%f\n", m, s, t );
    }

    /* selection
     */
    for ( i=0, selectedresiduals=field->n_computed_pairs; i<selectedresiduals; i++ ) {
      if ( field->pointer[i]->error > t ) {
        /* swap with right */
        tmp = field->pointer[i];
        field->pointer[i] = field->pointer[selectedresiduals-1];
        field->pointer[selectedresiduals-1] = tmp;
        selectedresiduals --;
        i --;
      }
    }

    
  }



  /* selection on percentage
   */
  if ( estimator->retained_fraction  <= 0.0 ||  1.0 <= estimator->retained_fraction ) 
    return( selectedresiduals );
  
  h = (int) ( estimator->retained_fraction  * (double) field->n_computed_pairs );
  
  if ( h <= 0 || h >= selectedresiduals )
    return( selectedresiduals );
  
  if ( _debug_ ) {
    fprintf( stderr, "%s: selection percentage\n", proc );
    fprintf( stderr, "will retain %lu / %lu points\n", h, field->n_computed_pairs  );
  }

#ifdef _ORIGINAL_BALADIN_QSORT_IN_LTS_ 
  if ( _debug_ ) 
    fprintf( stderr, "%s: selection percentage, use qsort\n", proc );
  qsort ( field->pairs, selectedresiduals, sizeof(typeScalarWeightedDisplacement *), &_CompareResiduals );
#else
  if ( _debug_ ) 
    fprintf( stderr, "%s: selection percentage, use home made procedure\n", proc );
  left = 0; 
  right = selectedresiduals-1;
  do {
    /* swap left et (left+right)/2 */
    j = (left+right)/2;
    if ( _debug_ ) fprintf( stderr, "[%lu - %lu] : swap %lu <-> %lu (test <-> pivot)\n", left, right, left, j );
    tmp = field->pointer[left]; field->pointer[left] = field->pointer[j]; field->pointer[j] = tmp;
    last = left;
    for ( k = left+1; k <= right; k++ ) {
 
      if ( field->pointer[k]->error < field->pointer[left]->error ) {
        last ++;
        /* if ( k > last ) */ {
          if ( _debug_ ) fprintf( stderr, "[%lu - %lu] : swap %lu <-> %lu\n", left, right, k, last );
          if ( _debug_ && ( left == 235 && right == 267 ) )
            fprintf( stderr, "\t e[k] = %20.15f < %20.15f = e[left]\n", field->pointer[k]->error, field->pointer[left]->error );
          tmp = field->pointer[k]; field->pointer[k] = field->pointer[last]; field->pointer[last] = tmp;
        }
      }
    }
    if ( _debug_ ) fprintf( stderr, "[%lu - %lu] : swap %lu <-> %lu (test <-> last)\n", left, right, left, last );
    tmp = field->pointer[left]; field->pointer[left] = field->pointer[last]; field->pointer[last] = tmp;
    if ( last >  h ) right = last - 1;
    if ( last <  h ) left  = last + 1;
  } while ( last != h );                         
#endif
  
  if ( _debug_ ) {
    for ( i=0;i<field->n_computed_pairs;i++ ) 
      fprintf( stderr, "residual[%lu] = %14.9f\n", i, field->pointer[i]->error );
  }

  return( h );

}





/*************************************************************
 *
 * Computation of pairs
 *
 *************************************************************/



typedef struct {
  FIELD *field;
  bal_image *inrimage_flo; 
  BLOCS *blocs_flo;
  bal_image *inrimage_ref; 
  BLOCS *blocs_ref;
  bal_integerPoint *half_neighborhood_size;
  bal_integerPoint *step_neighborhood_search;
  enumTypeSimilarity measure_type;
  double measure_threshold;
  size_t offset;
} _PairingFieldParam;





/* procedure for parallelism
 */

static void *_ComputePairingField3D( void *par ) 
{
  char *proc = "_ComputePairingField3D";
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  FIELD *field            = ((_PairingFieldParam*)parameter)->field;
  bal_image *inrimage_flo = ((_PairingFieldParam*)parameter)->inrimage_flo; 
  BLOCS *blocs_flo        = ((_PairingFieldParam*)parameter)->blocs_flo;
  bal_image *inrimage_ref = ((_PairingFieldParam*)parameter)->inrimage_ref; 
  BLOCS *blocs_ref        = ((_PairingFieldParam*)parameter)->blocs_ref;
  bal_integerPoint *half_neighborhood_size   = ((_PairingFieldParam*)parameter)->half_neighborhood_size;
  bal_integerPoint *step_neighborhood_search = ((_PairingFieldParam*)parameter)->step_neighborhood_search;
  enumTypeSimilarity measure_type = ((_PairingFieldParam*)parameter)->measure_type;
  double measure_threshold    = ((_PairingFieldParam*)parameter)->measure_threshold;
  size_t offset               = ((_PairingFieldParam*)parameter)->offset;

  size_t i;
  int a, b, c, u, v, w, ind_Buvw;
  int i_max = 0;
  int j_max = 0; 
  int k_max = 0;

  double measure_max, measure_0;
  double measure_default = -2.0;
  double *themeas = NULL;
  int xmeas, ymeas, zmeas, xymeas, xyzmeas;
  int x, y, z,  minx, miny, minz;
  int n_pairs;

  double demi_bl_dx = (double) (blocs_flo->blockdim.x-1) / 2.0;
  double demi_bl_dy = (double) (blocs_flo->blockdim.y-1) / 2.0;
  double demi_bl_dz = (double) (blocs_flo->blockdim.z-1) / 2.0; 
  
  int minu, maxu;
  int minv, maxv;
  int minw, maxw;

  n_pairs = field->n_computed_pairs = 0;

  xmeas = 1+(2*half_neighborhood_size->x)/step_neighborhood_search->x;
  ymeas = 1+(2*half_neighborhood_size->y)/step_neighborhood_search->y;
  zmeas = 1+(2*half_neighborhood_size->z)/step_neighborhood_search->z;
  xymeas = xmeas * ymeas;
  xyzmeas = xmeas * ymeas * zmeas;
  themeas = (double*)malloc( xyzmeas * sizeof( double ) );
  if ( themeas == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate themeas array\n", proc );
    chunk->ret = -1;
    return( (void*)NULL );
  }
  /***
      Passe sur les blocs retenus de l'image flottante, et exploration
      d'un voisinage de chaque bloc dans l'image de reference 

      Rappel: a, b, c : origine du bloc
  ***/



  for (i=first; i<=last; i++) {

    field->pointer[offset+i]->valid = 0;

    a = blocs_flo->pointer[i]->origin.x;      
    b = blocs_flo->pointer[i]->origin.y;      
    c = blocs_flo->pointer[i]->origin.z;


    
    /***
        Pour un bloc donne dans l'image flottante, on calcule la mesure de
        similarite avec un certain nombre de blocs de l'image de reference

        Pas de progression : bl_next_neigh_?

        Dimensions du voisinage d'exploration : - bl_size_neigh ... bl_size_neigh
    ***/
    for ( x=xyzmeas-1; x>=0; x-- ) themeas[x] = measure_default;

    minu = a-half_neighborhood_size->x;
    maxu = a+half_neighborhood_size->x;
    minv = b-half_neighborhood_size->y;
    maxv = b+half_neighborhood_size->y;
    minw = c-half_neighborhood_size->z;
    maxw = c+half_neighborhood_size->z;

    minx = miny = minz = 0;

    while ( minu < 0 ) { minu += step_neighborhood_search->x; minx ++; }
    while ( minv < 0 ) { minv += step_neighborhood_search->y; miny ++; }
    while ( minw < 0 ) { minw += step_neighborhood_search->z; minz ++ ; }
    if ( maxu >= (int)blocs_ref->blocksarraydim.x ) maxu = blocs_ref->blocksarraydim.x - 1;
    if ( maxv >= (int)blocs_ref->blocksarraydim.y ) maxv = blocs_ref->blocksarraydim.y - 1;
    if ( maxw >= (int)blocs_ref->blocksarraydim.z ) maxw = blocs_ref->blocksarraydim.z - 1;
    
    for ( u = minu, x = minx; u <= maxu; u += step_neighborhood_search->x, x++ )
    for ( v = minv, y = miny; v <= maxv; v += step_neighborhood_search->y, y++ )
    for ( w = minw, z = minz; w <= maxw; w += step_neighborhood_search->z, z++ ) {

      /* index of  B(u,v,w) in blocs_ref->bloc 
         this is linked to the building of blocs_ref
         cf Allocate_Blocks()
      */
#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
      ind_Buvw = w + (v + u * blocs_ref->blocksarraydim.y) * blocs_ref->blocksarraydim.z;
#else
      ind_Buvw = u + (v + w * blocs_ref->blocksarraydim.y) * blocs_ref->blocksarraydim.x;
#endif

      /* bloc B(u,v,w) actif ? */
      if ( blocs_ref->data[ind_Buvw].valid != 1 ) continue;
      
      themeas[z*xymeas+y*xmeas+x] = BAL_ComputeBlockSimilarity3D( blocs_flo->pointer[i], 
                                                                   &(blocs_ref->data[ind_Buvw]),
                                                                   inrimage_flo, inrimage_ref,
                                                                   &(blocs_flo->selection),
                                                                   &(blocs_ref->selection),
                                                                   &(blocs_flo->blockdim),
                                                                   measure_type );
      }
    
    /* it may happens that the NULL displacement has not be considered
     */
    measure_0 = measure_default;
    if ( half_neighborhood_size->x % step_neighborhood_search->x != 0
         || half_neighborhood_size->y % step_neighborhood_search->y != 0
         || half_neighborhood_size->z % step_neighborhood_search->z != 0 ) {
      
#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
      ind_Buvw = c + (b + a * blocs_ref->blocksarraydim.y) * blocs_ref->blocksarraydim.z;
#else
      ind_Buvw = a + (b + c * blocs_ref->blocksarraydim.y) * blocs_ref->blocksarraydim.x;
      /* was
         ind_Buvw = c + (b + a * blocs_ref->blocksarraydim.y) * blocs_ref->blocksarraydim.x;
         this should be false !
         corrected 31 jan 2012, GM
      */
#endif
      if ( blocs_ref->data[ind_Buvw].valid == 1 )
        measure_0 = BAL_ComputeBlockSimilarity3D( blocs_flo->pointer[i],
                                                  &(blocs_ref->data[ind_Buvw]),
                                                  inrimage_flo, inrimage_ref,
                                                  &(blocs_flo->selection),
                                                  &(blocs_ref->selection),
                                                  &(blocs_flo->blockdim),
                                                  measure_type );
    }
    else {
      measure_0 = themeas[ (half_neighborhood_size->z/step_neighborhood_search->z)*xymeas
                          + (half_neighborhood_size->y/step_neighborhood_search->y)*xmeas
                          + (half_neighborhood_size->x/step_neighborhood_search->x) ];
    }

    /* choice of the "best" pairing
     */

    measure_max = measure_0;
    k_max = c;
    j_max = b;
    i_max = a;

    switch ( measure_type ) {
    default :
      free( themeas );
      if ( _verbose_ ) 
        fprintf( stderr, "%s: measure not handled yet\n", proc );
      chunk->ret = -1;
      return( (void*)NULL );
    case _SQUARED_CC_ :
    case _SQUARED_EXTCC_ :
      for ( u = minu, x = minx; u <= maxu; u += step_neighborhood_search->x, x++ )
      for ( v = minv, y = miny; v <= maxv; v += step_neighborhood_search->y, y++ )
      for ( w = minw, z = minz; w <= maxw; w += step_neighborhood_search->z, z++ ) {
        if ( themeas[z*xymeas+y*xmeas+x] > measure_max ) {
          measure_max = themeas[z*xymeas+y*xmeas+x];
          k_max = w;
          j_max = v;
          i_max = u;
        }
      }
      /* is it "best" enough ?
       */
      if ( (measure_max > measure_threshold) ) {
        field->pointer[offset+i]->origin.x = a + demi_bl_dx;
        field->pointer[offset+i]->origin.y = b + demi_bl_dy;
        field->pointer[offset+i]->origin.z = c + demi_bl_dz;
        field->pointer[offset+i]->vector.x = i_max - a;
        field->pointer[offset+i]->vector.y = j_max - b;
        field->pointer[offset+i]->vector.z = k_max - c;
        field->pointer[offset+i]->valid = 1;
        field->pointer[offset+i]->rho = measure_max;
      }
      else {
        field->pointer[offset+i]->valid = 0;
      }
      break;
    case _SAD_ :
    case _SSD_ :
      for ( u = minu, x = minx; u <= maxu; u += step_neighborhood_search->x, x++ )
      for ( v = minv, y = miny; v <= maxv; v += step_neighborhood_search->y, y++ )
      for ( w = minw, z = minz; w <= maxw; w += step_neighborhood_search->z, z++ ) {
        if ( themeas[z*xymeas+y*xmeas+x] < measure_max ) {
          measure_max = themeas[z*xymeas+y*xmeas+x];
          k_max = w;
          j_max = v;
          i_max = u;
        }
      }
      /* is it "best" enough ?
       */
      if ( (measure_max < measure_threshold) ) {
        field->pointer[offset+i]->origin.x = a + demi_bl_dx;
        field->pointer[offset+i]->origin.y = b + demi_bl_dy;
        field->pointer[offset+i]->origin.z = c + demi_bl_dz;
        field->pointer[offset+i]->vector.x = i_max - a;
        field->pointer[offset+i]->vector.y = j_max - b;
        field->pointer[offset+i]->vector.z = k_max - c;
        field->pointer[offset+i]->valid = 1;
        field->pointer[offset+i]->rho = measure_max;
      }
      else {
        field->pointer[offset+i]->valid = 0;
      }
      break;
    }
  
    
  }

  free( themeas );
  field->n_computed_pairs = n_pairs;
  field->n_selected_pairs = n_pairs;
  
  chunk->ret = 1;
  return( (void*)NULL );
}





/* procedure for parallelism
 */

static void *_ComputePairingField2D( void *par ) 
{
  char *proc = "_ComputePairingField2D";
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  FIELD *field            = ((_PairingFieldParam*)parameter)->field;
  bal_image *inrimage_flo = ((_PairingFieldParam*)parameter)->inrimage_flo; 
  BLOCS *blocs_flo        = ((_PairingFieldParam*)parameter)->blocs_flo;
  bal_image *inrimage_ref = ((_PairingFieldParam*)parameter)->inrimage_ref; 
  BLOCS *blocs_ref        = ((_PairingFieldParam*)parameter)->blocs_ref;
  bal_integerPoint *half_neighborhood_size   = ((_PairingFieldParam*)parameter)->half_neighborhood_size;
  bal_integerPoint *step_neighborhood_search = ((_PairingFieldParam*)parameter)->step_neighborhood_search;
  enumTypeSimilarity measure_type = ((_PairingFieldParam*)parameter)->measure_type;
  double measure_threshold    = ((_PairingFieldParam*)parameter)->measure_threshold;
  size_t offset               = ((_PairingFieldParam*)parameter)->offset;

  size_t i;
  int a, b, u, v, ind_Buv;
  int i_max = 0;
  int j_max = 0;

  double measure_max, measure_0;
  double measure_default = -2.0;
  double *themeas = NULL;
  int xmeas, ymeas;
  int x, y, minx, miny;
  int n_pairs;

  double demi_bl_dx = (double) (blocs_flo->blockdim.x-1) / 2.0;
  double demi_bl_dy = (double) (blocs_flo->blockdim.y-1) / 2.0;

  int minu, maxu;
  int minv, maxv;



  n_pairs = field->n_computed_pairs = 0;

  xmeas = 1+(2*half_neighborhood_size->x)/step_neighborhood_search->x;
  ymeas = 1+(2*half_neighborhood_size->y)/step_neighborhood_search->y;
  themeas = (double*)malloc( xmeas * ymeas * sizeof( double ) );
  if ( themeas == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate themeas array\n", proc );
    chunk->ret = -1;
    return( (void*)NULL );
  }
  /***
      Passe sur les blocs retenus de l'image flottante, et exploration
      d'un voisinage de chaque bloc dans l'image de reference 

      Rappel: a, b : origine du bloc
  ***/



  for (i=first; i<=last; i++) {

    field->pointer[offset+i]->valid = 0;

    a = blocs_flo->pointer[i]->origin.x;      
    b = blocs_flo->pointer[i]->origin.y;



    /***
        Pour un bloc donne dans l'image flottante, on calcule la mesure de
        similarite avec un certain nombre de blocs de l'image de reference

        Pas de progression : bl_next_neigh_?

        Dimensions du voisinage d'exploration : - bl_size_neigh ... bl_size_neigh
    ***/
    for ( x=xmeas*ymeas-1; x>=0; x-- ) themeas[x] = measure_default;

    minu = a-half_neighborhood_size->x;
    maxu = a+half_neighborhood_size->x;
    minv = b-half_neighborhood_size->y;
    maxv = b+half_neighborhood_size->y;

    minx = miny = 0;

    while ( minu < 0 ) { minu += step_neighborhood_search->x; minx ++; }
    while ( minv < 0 ) { minv += step_neighborhood_search->y; miny ++; }
    if ( maxu >= (int)blocs_ref->blocksarraydim.x ) maxu = blocs_ref->blocksarraydim.x - 1;
    if ( maxv >= (int)blocs_ref->blocksarraydim.y ) maxv = blocs_ref->blocksarraydim.y - 1;
    
    for ( u = minu, x = minx; u <= maxu; u += step_neighborhood_search->x, x++ )
    for ( v = minv, y = miny; v <= maxv; v += step_neighborhood_search->y, y++ ) {

      /* index of  B(u,v) in blocs_ref->bloc 
         this is linked to the building of blocs_ref
         cf Allocate_Blocks()
      */
#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
      ind_Buv = v + u * blocs_ref->blocksarraydim.y;
#else
      ind_Buv = u + v * blocs_ref->blocksarraydim.x;
#endif

      /* bloc B(u,v) actif ? */
      if ( blocs_ref->data[ind_Buv].valid != 1 ) continue;

      themeas[y*xmeas+x] = BAL_ComputeBlockSimilarity2D( blocs_flo->pointer[i], 
                                                          &(blocs_ref->data[ind_Buv]),
                                                          inrimage_flo, inrimage_ref,
                                                          &(blocs_flo->selection),
                                                          &(blocs_ref->selection),
                                                          &(blocs_flo->blockdim),
                                                          measure_type );
    }

    /* it may happens that the NULL displacement has not be considered
     */
    measure_0 = measure_default;
    if ( half_neighborhood_size->x % step_neighborhood_search->x != 0
         || half_neighborhood_size->y % step_neighborhood_search->y != 0 ) {
      
#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
      ind_Buv = b + a * blocs_ref->blocksarraydim.y;
#else
      ind_Buv = a + b * blocs_ref->blocksarraydim.y;
#endif
      if ( blocs_ref->data[ind_Buv].valid == 1 )
        measure_0 = BAL_ComputeBlockSimilarity2D( blocs_flo->pointer[i],
                                                  &(blocs_ref->data[ind_Buv]),
                                                  inrimage_flo, inrimage_ref,
                                                  &(blocs_flo->selection),
                                                  &(blocs_ref->selection),
                                                  &(blocs_flo->blockdim),
                                                  measure_type );
    }
    else {
      measure_0 = themeas[ (half_neighborhood_size->y/step_neighborhood_search->y)*xmeas
                          + (half_neighborhood_size->x/step_neighborhood_search->x) ];
    }
    
    /* choice of the "best" pairing
     */
    measure_max = measure_0;
    j_max = b;
    i_max = a;

    switch ( measure_type ) {
    default :
      free( themeas );
      if ( _verbose_ ) 
        fprintf( stderr, "%s: measure not handled yet\n", proc );
      chunk->ret = -1;
      return( (void*)NULL );
    case   _SQUARED_CC_ :
    case _SQUARED_EXTCC_ :
      for ( u = minu, x = minx; u <= maxu; u += step_neighborhood_search->x, x++ )
      for ( v = minv, y = miny; v <= maxv; v += step_neighborhood_search->y, y++ ) {
        if ( themeas[y*xmeas+x] > measure_max ) {
          measure_max = themeas[y*xmeas+x];
          j_max = v;
          i_max = u;
        }
      }
      /* is it "best" enough ?
       */
      if ( (measure_max > measure_threshold) ) {
        field->pointer[offset+i]->origin.x = a + demi_bl_dx;
        field->pointer[offset+i]->origin.y = b + demi_bl_dy;
        field->pointer[offset+i]->origin.z = 0.0;
        field->pointer[offset+i]->vector.x = i_max - a;
        field->pointer[offset+i]->vector.y = j_max - b;
        field->pointer[offset+i]->vector.z = 0.0;
        field->pointer[offset+i]->valid = 1;
        field->pointer[offset+i]->rho = measure_max;
      }
      else {
        field->pointer[offset+i]->valid = 0;
      }
      break;
    case _SAD_ :
    case _SSD_ :
      for ( u = minu, x = minx; u <= maxu; u += step_neighborhood_search->x, x++ )
      for ( v = minv, y = miny; v <= maxv; v += step_neighborhood_search->y, y++ ) {
        if ( themeas[y*xmeas+x] > measure_max ) {
          measure_max = themeas[y*xmeas+x];
          j_max = v;
          i_max = u;
        }
      }
      /* is it "best" enough ?
       */
      if ( (measure_max > measure_threshold) ) {
        field->pointer[offset+i]->origin.x = a + demi_bl_dx;
        field->pointer[offset+i]->origin.y = b + demi_bl_dy;
        field->pointer[offset+i]->origin.z = 0.0;
        field->pointer[offset+i]->vector.x = i_max - a;
        field->pointer[offset+i]->vector.y = j_max - b;
        field->pointer[offset+i]->vector.z = 0.0;
        field->pointer[offset+i]->valid = 1;
        field->pointer[offset+i]->rho = measure_max;
      }
      else {
        field->pointer[offset+i]->valid = 0;
      }
      break;
    }

  }

  free( themeas );
  field->n_computed_pairs = n_pairs;
  field->n_selected_pairs = n_pairs;
  
  chunk->ret = 1;
  return( (void*)NULL );
}





/***
    Calcul du champ de vecteurs entre Image_ref et Image_flo
    - chapeau du calcul 2D ou 3D

    - donner le debut et la fin des blocs, et le debut et la fin des paires *dans data*
    ou mettre le resultat, (a partir de n_computed_pairs) et trier les paires ensuites 

    Le champ de vecteur est d'abord construit (_ComputePairingField2D et _ComputePairingField3D)
    tel que
      origin = centre du bloc flottant + demi-taille de bloc
      vector = centre du bloc reference - centre du bloc flottant
    puis il est "inverse" (a la fin de BAL_ComputePairingFieldFromFloToRef)
    et il vaut finalement
      origin = centre du bloc reference + demi-taille de bloc
      vector = centre du bloc flottant - centre du bloc reference


***/
int BAL_ComputePairingFieldFromRefToFlo( FIELD *field,
                              bal_image *inrimage_flo, BLOCS *blocs_flo,
                              bal_image *inrimage_ref, BLOCS *blocs_ref,
                              bal_integerPoint *half_neighborhood_size,
                              bal_integerPoint *step_neighborhood_search,
                              enumTypeSimilarity measure_type,
                              double measure_threshold )
{
  char *proc = "BAL_ComputePairingFieldFromFloToRef";
  int n;
  size_t i, l;
  typeScalarWeightedDisplacement *tmp;

  _PairingFieldParam p;
  typeChunks chunks;
  size_t first, last;

  
  if ( blocs_flo->n_valid_blocks <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no valid blocks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }


  /* preparation for parallelism computing with openmp
     1. chunks calculation
     2. parameter preparation
   */

  first = 0;
  last = blocs_flo->n_valid_blocks-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }

  p.field = field;
  p.inrimage_flo = inrimage_flo; 
  p.blocs_flo = blocs_flo;
  p.inrimage_ref = inrimage_ref; 
  p.blocs_ref = blocs_ref;
  p.half_neighborhood_size = half_neighborhood_size;
  p.step_neighborhood_search = step_neighborhood_search;
  p.measure_type = measure_type;
  p.measure_threshold = measure_threshold;
  p.offset = field->n_computed_pairs;

  for ( n=0; n<chunks.n_allocated_chunks; n++ )
    chunks.data[n].parameters = (void*)(&p);



  /* pairing computation
     - pairs will be in [offset+first, offset+last]
       this allows multiple calls
   */
  if (inrimage_ref->nplanes  == 1) {

    if ( processChunks( &_ComputePairingField2D, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute pairings (2D case)\n", proc );
      freeChunks( &chunks );
      return( RETURNED_VALUE_ON_ERROR );
    }
 
  } /* end of 2D case */
  else {

    if ( processChunks( &_ComputePairingField3D, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute pairings (3D case)\n", proc );
      freeChunks( &chunks );
      return( RETURNED_VALUE_ON_ERROR );
    }
    
  } /* end of 3D case */

  freeChunks( &chunks );



  /* the pairs are re-ordered so that the valid pairs come first

     please note that this re-ordering is different from the original 
     implementation where the valid pairs are put in the beginning of the list
     as soon they appear. However, this is not compatible with parallel 
     computing
  */

  for ( i = field->n_computed_pairs+first, l=field->n_computed_pairs+last+1; 
        i <= field->n_computed_pairs+last && field->pointer[i]->valid == 1;
        i ++ )
    ;
    
  if ( i <= field->n_computed_pairs+last ) {
    while ( i < l ) {
      if ( field->pointer[i]->valid == 1 ) {
        i ++;
      }
      else {
        /* switch i and l-1 */
        tmp                     = field->pointer[i];
        field->pointer[i]      = field->pointer[l-1];
        field->pointer[l-1] = tmp;
        l --;
      }
    }
  }

  field->n_computed_pairs = l;
  field->n_selected_pairs = l;

  /* dans le champ, les points et vecteurs sont en coordonnees voxel,
     le point de depart (x,y,z) est dans l'image flottante,
     et le vecteur (u,v,w) = (x',y',z')-(x,y,z) avec (x',y',z') le point 
     correspondant dans l'image de reference.
     
     Pour calculer la transformation voxel de l'image de reference vers l'image
     flottante, on transforme :
     x -> (x+u)
     u -> (-u)
  */
  for (i = 0; i < field->n_computed_pairs; i++) {
    field->pointer[i]->origin.x = (field->pointer[i]->origin.x + field->pointer[i]->vector.x);
    field->pointer[i]->origin.y = (field->pointer[i]->origin.y + field->pointer[i]->vector.y);
    field->pointer[i]->origin.z = (field->pointer[i]->origin.z + field->pointer[i]->vector.z);
    field->pointer[i]->vector.x = (- field->pointer[i]->vector.x);
    field->pointer[i]->vector.y = (- field->pointer[i]->vector.y);
    field->pointer[i]->vector.z = (- field->pointer[i]->vector.z);
  }
  
  field->unit = VOXEL_UNIT;

  return( 1 );
}





/***
    Displacement from two lists of paired points
    Origin = reference point
    Displacement = floating point - reference point
***/
int BAL_ComputePairingFieldFromPointList( FIELD *field,
                                         bal_typeFieldPointList *floPoints,
                                         bal_typeFieldPointList *refPoints )
{
  char *proc = "BAL_ComputePairingFieldFromPointList";
  int n_data, i;

  if ( floPoints->n_data <= 0 || refPoints->n_data <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no points in the list\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }

  /* pair as many points as possible
   */
  n_data = floPoints->n_data;
  if ( n_data > refPoints->n_data )
    n_data = refPoints->n_data;

  if ( floPoints->unit != refPoints->unit ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: points have different units\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( i=0; i<n_data; i++ ) {
    field->pointer[i]->origin.x = refPoints->data[i].x;
    field->pointer[i]->origin.y = refPoints->data[i].y;
    field->pointer[i]->origin.z = refPoints->data[i].z;

    field->pointer[i]->vector.x = floPoints->data[i].x - refPoints->data[i].x;
    field->pointer[i]->vector.y = floPoints->data[i].y - refPoints->data[i].y;
    field->pointer[i]->vector.z = floPoints->data[i].z - refPoints->data[i].z;
  }

  field->n_computed_pairs = n_data;
  field->n_selected_pairs = n_data;
  field->unit = refPoints->unit;

  return( 1 );
}


