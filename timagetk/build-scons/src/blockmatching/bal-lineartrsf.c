/*************************************************************************
 * bal-lineartrsf.c -
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

#include <bal-lineartrsf.h>
#include <bal-field-tools.h>
#include <bal-behavior.h>




/* this allows to tune the behavior w.r.t.
   baladin version transfered at La Pitie
*/






/************************************************************
 *
 * static variables
 *
 ************************************************************/


static int _verbose_ = 1;
/*
static int _debug_ = 0;
*/

void BAL_SetVerboseInBalLinearTrsf( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalLinearTrsf(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalLinearTrsf(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}












/* --------------------------------------------------------------------
   
   Computation procedures

   -------------------------------------------------------------------- */

typedef struct {

  bal_doublePoint orig_center;
  bal_doublePoint dest_center;
  double sumWeights;
  
  double cov[9];
  double orig_cov[9];

  double sum[16];
  
  double orig_modulus;
  double dest_modulus;

  double similitude_criteria;
  double *rotationMatrix;

  FIELD *field;

} _LinearTrsfAuxiliaryParam;





/* --------------------------------------------------------------------- 
 *
 * Least squares estimation of 2D transformations
 *
 * --------------------------------------------------------------------- */



/************************************************************
 *
 * subroutines for openmp parallelism
 *
 ************************************************************/

static void *_LS_Barycenters2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;

  for ( i=first; i<=last; i++ ) {
    orig_center->x += thePairs[i]->origin.x;
    orig_center->y += thePairs[i]->origin.y;
    dest_center->x += thePairs[i]->vector.x;
    dest_center->y += thePairs[i]->vector.y;
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_TranslationScaling2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    
    /* Incrementation des matrices */
    cov[0] += dest_coord.x * orig_coord.x;    
    cov[4] += dest_coord.y * orig_coord.y;    
    
    orig_cov[0] += orig_coord.x * orig_coord.x;    
    orig_cov[4] += orig_coord.y * orig_coord.y;    
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_Rigid2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y;
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];
    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += aTa[n];
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_Similitude2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  double *orig_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_modulus;
  double *dest_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_modulus;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];
  double local_orig_modulus = 0.0;
  double local_dest_modulus = 0.0;

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;

    local_orig_modulus += ( orig_coord.x*orig_coord.x + orig_coord.y*orig_coord.y );
    local_dest_modulus += ( dest_coord.x*dest_coord.x + dest_coord.y*dest_coord.y );

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y;
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];
    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += aTa[n];
  }
  *orig_modulus = local_orig_modulus;
  *dest_modulus = local_dest_modulus;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_SimilitudeScale2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *similitude_criteria  = &((_LinearTrsfAuxiliaryParam*)parameter)->similitude_criteria;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  double *rotationMatrix       = ((_LinearTrsfAuxiliaryParam*)parameter)->rotationMatrix;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  double orig_coord[3] = {0.0, 0.0, 0.0};
  double orig_trans[3] = {0.0, 0.0, 0.0};
  double dest_coord[3] = {0.0, 0.0, 0.0};
  double diff[3] = {0.0, 0.0, 0.0};
  double local_sum = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord[0] = thePairs[i]->origin.x - orig_center->x;
    orig_coord[1] = thePairs[i]->origin.y - orig_center->y;
    dest_coord[0] = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord[1] = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;

    E_DMMatMulVect ( rotationMatrix, orig_coord, orig_trans, 3 );

    diff[0] = dest_coord[0] - orig_trans[0];
    diff[1] = dest_coord[1] - orig_trans[1];

    local_sum += ( diff[0] * diff[0] + diff[1] * diff[1] );
  }
  *similitude_criteria = local_sum;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_Affine2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    
    /* Incrementation des matrices */
    cov[0] += dest_coord.x * orig_coord.x;    
    cov[1] += dest_coord.x * orig_coord.y;    
    cov[3] += dest_coord.y * orig_coord.x;    
    cov[4] += dest_coord.y * orig_coord.y;    
    
    orig_cov[0] += orig_coord.x * orig_coord.x;    
    orig_cov[1] += orig_coord.x * orig_coord.y;    
    orig_cov[4] += orig_coord.y * orig_coord.y;    
  }
  orig_cov[3] = orig_cov[1];
  chunk->ret = 1;
  return( (void*)NULL );
}





/************************************************************
 *
 * transformation computation
 *
 ************************************************************/

static int LS_Estimation_Barycenters_2D( bal_doublePoint *orig_center,
                                         bal_doublePoint *dest_center,
                                         double nbPairs,
                                         typeChunks *chunks )
{
  char *proc = "LS_Estimation_Barycenters_2D";
  int n;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;



  if ( processChunks( &_LS_Barycenters2DSubroutine, chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter (2D LS linear case)\n", proc );
    return( -1 );
  }

  for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
    orig_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.x;
    orig_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.y;
    dest_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.x;
    dest_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.y;
  }

  orig_center->x /= (nbPairs);
  orig_center->y /= (nbPairs);
  dest_center->x = orig_center->x + dest_center->x / (nbPairs);
  dest_center->y = orig_center->y + dest_center->y / (nbPairs);

  return( 1 );
}





static int LS_Estimation_Translation_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Translation_2D";

  int n;
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_2D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                     &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  free( aux );
  freeChunks( &chunks );



  /* Calcul de la matrice de transformation 
   */
  T->m[0] = 1.0; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = 1.0; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;

  return ( 1 );
}





static int LS_Estimation_Translation_Scaling_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Translation_Scaling_2D";
  
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;

  int i, n;
  double cov[9], orig_cov[9];
  bal_doublePoint scale;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_2D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                     &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    cov[i] = orig_cov[i] = 0.0;

  if ( processChunks( &_LS_TranslationScaling2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    cov[0] += aux[n].cov[0];
    cov[4] += aux[n].cov[4];

    orig_cov[0] += aux[n].orig_cov[0];
    orig_cov[4] += aux[n].orig_cov[4];
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  scale.x = cov[0] / orig_cov[0];
  scale.y = cov[4] / orig_cov[4];
  
  

  /* Calcul de la matrice de transformation 
   */
  T->m[0] = scale.x; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - scale.x * orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = scale.y; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - scale.y * orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int LS_Estimation_Rigid_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Rigid_2D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 2 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_2D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                     &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_LS_Rigid2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = 0.0;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = mr[0]; T->m[1] = mr[1]; T->m[2] = mr[2]; 
  T->m[3] = dest_center.x - rog[0];
  
  T->m[4] = mr[3]; T->m[5] = mr[4]; T->m[6] = mr[5]; 
  T->m[7] = dest_center.y - rog[1];
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int LS_Estimation_Similitude_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Similitude_2D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];
  double orig_modulus = 0.0;
  double dest_modulus = 0.0;
  double CritereRigide = 0.0;
  double echelle = 1.0;

  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 2 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_2D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_LS_Similitude2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
    orig_modulus += aux[n].orig_modulus;
    dest_modulus += aux[n].dest_modulus;
  }



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );



  /* calcul du scaling
   */
  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].rotationMatrix = mr;
  }

  if ( processChunks( &_LS_SimilitudeScale2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute scaling (2D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( CritereRigide=0.0, n=0; n<chunks.n_allocated_chunks; n++ ) {
    CritereRigide += aux[n].similitude_criteria;
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* calcul de l'echelle */
  echelle = (orig_modulus + dest_modulus - CritereRigide) / ( 2 * orig_modulus );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = 0.0;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = echelle*mr[0]; T->m[1] = echelle*mr[1]; T->m[2] = echelle*mr[2]; 
  T->m[3] = dest_center.x - echelle*rog[0];
  
  T->m[4] = echelle*mr[3]; T->m[5] = echelle*mr[4]; T->m[6] = echelle*mr[5]; 
  T->m[7] = dest_center.y - echelle*rog[1];
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] =  1.0;
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return( 1 );
}





static int LS_Estimation_Affine_2D ( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Affine_2D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  int i, j, n;
  double A[9], Cyx[9], Vx[9], invVx[9];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 3 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_2D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    Cyx[i] = Vx[i] = 0.0;
  
  if ( processChunks( &_LS_Affine2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    Cyx [0] += aux[n].cov[0];
    Cyx [1] += aux[n].cov[1];
    Cyx [3] += aux[n].cov[3];
    Cyx [4] += aux[n].cov[4];
    
    Vx [0] += aux[n].orig_cov[0];
    Vx [1] += aux[n].orig_cov[1];
    Vx [4] += aux[n].orig_cov[4];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* additionnal terms
   */
  Vx[3] = Vx[1];  
  
  /* Normalisation pour stabilite numerique (peut-etre superflu) 
     (n'est pas faite dans le cas non weighted)
  */
  if ( 0 ) {
    for ( i = 0 ; i < 3 ; i ++ )
    for ( j = 0 ; j < 3 ; j ++ )  {
      Cyx[j + 3*i] = Cyx[j + 3*i] / (double)field->n_selected_pairs;
      Vx[j + 3*i] = Vx[j + 3*i] / (double)field->n_selected_pairs;
    }
  }

  /* Gestion ad hoc du cas 2D */
  Cyx[8] = 1; 
  Vx[8] = 1; 
  
  /* Inversion de la matrice de variance (3x3) 
     invVx = inv_mat (Vx);*/
  /* Pas confiance dans inv_mat -> on utilise InverseMat4x4 */
  if ( InverseMat3x3( Vx, invVx ) != 3 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to invert matrix Vx\n", proc );
    return( -1 );
  }
  
  /* Calcul de la matrice A */
  E_DMMatMul ( Cyx, invVx, A, 3 );
  
  /* Calcul de la transformation affine 
   */
  T->m [0] = A [0];  T->m [1] = A [1];  T->m [2] = A [2];
  T->m [3] = dest_center.x - A[0]*orig_center.x - A[1]*orig_center.y;
  
  T->m [4] = A [3];  T->m [5] = A [4];  T->m [6] = A [5];
  T->m [7] = dest_center.y - A[3]*orig_center.x - A[4]*orig_center.y;
  
  T->m [8]  = 0.0;  T->m [9]  = 0.0;  T->m [10] = 1.0;
  T->m [11] = 0.0;
  
  T->m [12] = 0.0;  T->m [13] = 0.0;  T->m [14] = 0.0;  T->m [15] = 1.0;
  
  return ( 1 );
}





/* --------------------------------------------------------------------- 
 *
 * Least squares estimation of 3D transformations
 *
 * --------------------------------------------------------------------- */



/************************************************************
 *
 * subroutines for openmp parallelism
 *
 ************************************************************/

static void *_LS_Barycenters3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;

  for ( i=first; i<=last; i++ ) {
    orig_center->x += thePairs[i]->origin.x;
    orig_center->y += thePairs[i]->origin.y;
    orig_center->z += thePairs[i]->origin.z;
    dest_center->x += thePairs[i]->vector.x;
    dest_center->y += thePairs[i]->vector.y;
    dest_center->z += thePairs[i]->vector.z;
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_TranslationScaling3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;
    
    /* Incrementation des matrices */
    cov[0] += dest_coord.x * orig_coord.x;    
    cov[4] += dest_coord.y * orig_coord.y;    
    cov[8] += dest_coord.z * orig_coord.z;    
    
    orig_cov[0] += orig_coord.x * orig_coord.x;    
    orig_cov[4] += orig_coord.y * orig_coord.y;    
    orig_cov[8] += orig_coord.z * orig_coord.z;    
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_Rigid3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y; 
    a[3]   = orig_coord.z - dest_coord.z;
    a[6]   = - ( orig_coord.z + dest_coord.z );  
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];    a[9]   = - a[6];
    a[12]  = - a[3];    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += aTa[n];
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_Similitude3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  double *orig_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_modulus;
  double *dest_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_modulus;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];
  double local_orig_modulus = 0.0;
  double local_dest_modulus = 0.0;

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;

    local_orig_modulus += ( orig_coord.x*orig_coord.x + orig_coord.y*orig_coord.y 
                                               + orig_coord.z*orig_coord.z );
    local_dest_modulus += ( dest_coord.x*dest_coord.x + dest_coord.y*dest_coord.y 
                                               + dest_coord.z*dest_coord.z);

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y; 
    a[3]   = orig_coord.z - dest_coord.z;
    a[6]   = - ( orig_coord.z + dest_coord.z );  
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];    a[9]   = - a[6];
    a[12]  = - a[3];    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += aTa[n];
  }
  *orig_modulus = local_orig_modulus;
  *dest_modulus = local_dest_modulus;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_SimilitudeScale3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *similitude_criteria  = &((_LinearTrsfAuxiliaryParam*)parameter)->similitude_criteria;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  double *rotationMatrix       = ((_LinearTrsfAuxiliaryParam*)parameter)->rotationMatrix;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  double orig_coord[3] = {0.0, 0.0, 0.0};
  double orig_trans[3] = {0.0, 0.0, 0.0};
  double dest_coord[3] = {0.0, 0.0, 0.0};
  double diff[3] = {0.0, 0.0, 0.0};
  double local_sum = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord[0] = thePairs[i]->origin.x - orig_center->x;
    orig_coord[1] = thePairs[i]->origin.y - orig_center->y;
    orig_coord[2] = thePairs[i]->origin.z - orig_center->z;
    dest_coord[0] = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord[1] = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord[2] = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;

    E_DMMatMulVect ( rotationMatrix, orig_coord, orig_trans, 3 );

    diff[0] = dest_coord[0] - orig_trans[0];
    diff[1] = dest_coord[1] - orig_trans[1];
    diff[2] = dest_coord[2] - orig_trans[2];

    local_sum += ( diff[0] * diff[0] + diff[1] * diff[1]  + diff[2] * diff[2] );
  }
  *similitude_criteria = local_sum;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_LS_Affine3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;
    
    /* Incrementation des matrices */
    cov[0] += dest_coord.x * orig_coord.x;    
    cov[1] += dest_coord.x * orig_coord.y;    
    cov[2] += dest_coord.x * orig_coord.z;    
    cov[3] += dest_coord.y * orig_coord.x;    
    cov[4] += dest_coord.y * orig_coord.y;    
    cov[5] += dest_coord.y * orig_coord.z;    
    cov[6] += dest_coord.z * orig_coord.x;    
    cov[7] += dest_coord.z * orig_coord.y;    
    cov[8] += dest_coord.z * orig_coord.z;    
    
    orig_cov[0] += orig_coord.x * orig_coord.x;    
    orig_cov[1] += orig_coord.x * orig_coord.y;    
    orig_cov[2] += orig_coord.x * orig_coord.z;    
    orig_cov[4] += orig_coord.y * orig_coord.y;    
    orig_cov[5] += orig_coord.y * orig_coord.z;    
    orig_cov[8] += orig_coord.z * orig_coord.z;    
  }
  orig_cov[3] = orig_cov[1];
  orig_cov[6] = orig_cov[2];
  orig_cov[7] = orig_cov[5];
  chunk->ret = 1;
  return( (void*)NULL );
}





/************************************************************
 *
 * transformation computation
 *
 ************************************************************/

static int LS_Estimation_Barycenters_3D( bal_doublePoint *orig_center,
                                         bal_doublePoint *dest_center,
                                         double nbPairs,
                                         typeChunks *chunks )
{
  char *proc = "LS_Estimation_Barycenters_3D";
  int n;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;



  if ( processChunks( &_LS_Barycenters3DSubroutine, chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter (3D LS linear case)\n", proc );
    return( -1 );
  }

  for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
    orig_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.x;
    orig_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.y;
    orig_center->z += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.z;
    dest_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.x;
    dest_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.y;
    dest_center->z += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.z;
  }

  orig_center->x /= (nbPairs);
  orig_center->y /= (nbPairs);
  orig_center->z /= (nbPairs);
  dest_center->x = orig_center->x + dest_center->x / (nbPairs);
  dest_center->y = orig_center->y + dest_center->y / (nbPairs);
  dest_center->z = orig_center->z + dest_center->z / (nbPairs);

  return( 1 );
}





static int LS_Estimation_Translation_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Translation_3D";

  int n;
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_3D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                     &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  free( aux );
  freeChunks( &chunks );



  /* Calcul de la matrice de transformation 
   */
  T->m[0] = 1.0; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = 1.0; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = dest_center.z - orig_center.z;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;

  return ( 1 );
}





static int LS_Estimation_Translation_Scaling_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Translation_Scaling_3D";
  
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;

  int i, n;
  double cov[9], orig_cov[9];
  bal_doublePoint scale;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_3D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                     &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    cov[i] = orig_cov[i] = 0.0;

  if ( processChunks( &_LS_TranslationScaling3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    cov[0] += aux[n].cov[0];
    cov[4] += aux[n].cov[4];
    cov[8] += aux[n].cov[8];

    orig_cov[0] += aux[n].orig_cov[0];
    orig_cov[4] += aux[n].orig_cov[4];
    orig_cov[8] += aux[n].orig_cov[8];
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  scale.x = cov[0] / orig_cov[0];
  scale.y = cov[4] / orig_cov[4];
  scale.z = cov[8] / orig_cov[8];
  
  

  /* Calcul de la matrice de transformation 
   */
  T->m[0] = scale.x; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - scale.x * orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = scale.y; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - scale.y * orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = scale.z; 
  T->m[11] = dest_center.z - scale.z * orig_center.z;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int LS_Estimation_Rigid_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Rigid_3D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 3 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_3D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                     &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_LS_Rigid3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = orig_center.z;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = mr[0]; T->m[1] = mr[1]; T->m[2] = mr[2]; 
  T->m[3] = dest_center.x - rog[0];
  
  T->m[4] = mr[3]; T->m[5] = mr[4]; T->m[6] = mr[5]; 
  T->m[7] = dest_center.y - rog[1];
  
  T->m[8] = mr[6]; T->m[9] = mr[7]; T->m[10] =  mr[8]; 
  T->m[11] = dest_center.z - rog[2];
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int LS_Estimation_Similitude_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Similitude_3D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];
  double orig_modulus = 0.0;
  double dest_modulus = 0.0;
  double CritereRigide = 0.0;
  double echelle = 1.0;

  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 3 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_3D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_LS_Similitude3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
    orig_modulus += aux[n].orig_modulus;
    dest_modulus += aux[n].dest_modulus;
  }



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );



  /* calcul du scaling
   */
  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].rotationMatrix = mr;
  }

  if ( processChunks( &_LS_SimilitudeScale3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute scaling (3D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( CritereRigide=0.0, n=0; n<chunks.n_allocated_chunks; n++ ) {
    CritereRigide += aux[n].similitude_criteria;
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* calcul de l'echelle */
  echelle = (orig_modulus + dest_modulus - CritereRigide) / ( 2 * orig_modulus );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = orig_center.z;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = echelle*mr[0]; T->m[1] = echelle*mr[1]; T->m[2] = echelle*mr[2]; 
  T->m[3] = dest_center.x - echelle*rog[0];
  
  T->m[4] = echelle*mr[3]; T->m[5] = echelle*mr[4]; T->m[6] = echelle*mr[5]; 
  T->m[7] = dest_center.y - echelle*rog[1];
  
  T->m[8] = echelle*mr[6]; T->m[9] = echelle*mr[7]; T->m[10] =  echelle*mr[8]; 
  T->m[11] = dest_center.z - echelle*rog[2];
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return( 1 );
}





static int LS_Estimation_Affine_3D ( _MATRIX *T, FIELD *field )
{
  char *proc = "LS_Estimation_Affine_3D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  int i, j, n;
  double A[9], Cyx[9], Vx[9], invVx[9];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( LS_Estimation_Barycenters_3D( &orig_center, &dest_center, (double)field->n_selected_pairs, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    Cyx[i] = Vx[i] = 0.0;
  
  if ( processChunks( &_LS_Affine3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D LS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    Cyx [0] += aux[n].cov[0];
    Cyx [1] += aux[n].cov[1];
    Cyx [2] += aux[n].cov[2];
    Cyx [3] += aux[n].cov[3];
    Cyx [4] += aux[n].cov[4];
    Cyx [5] += aux[n].cov[5];
    Cyx [6] += aux[n].cov[6];
    Cyx [7] += aux[n].cov[7];
    Cyx [8] += aux[n].cov[8];
    
    Vx [0] += aux[n].orig_cov[0];
    Vx [1] += aux[n].orig_cov[1];
    Vx [2] += aux[n].orig_cov[2];
    Vx [4] += aux[n].orig_cov[4];
    Vx [5] += aux[n].orig_cov[5];
    Vx [8] += aux[n].orig_cov[8];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* additionnal terms
   */
  Vx[3] = Vx[1];  
  Vx[6] = Vx[2];  
  Vx[7] = Vx[5]; 
  
  /* Normalisation pour stabilite numerique (peut-etre superflu) 
     (n'est pas faite dans le cas non weighted)
  */
  if ( 0 ) {
    for ( i = 0 ; i < 3 ; i ++ )
    for ( j = 0 ; j < 3 ; j ++ )  {
      Cyx[j + 3*i] = Cyx[j + 3*i] / (double)field->n_selected_pairs;
      Vx[j + 3*i] = Vx[j + 3*i] / (double)field->n_selected_pairs;
    }
  }

  /* Inversion de la matrice de variance (3x3) 
     invVx = inv_mat (Vx);*/
  /* Pas confiance dans inv_mat -> on utilise InverseMat4x4 */
  if ( InverseMat3x3( Vx, invVx ) != 3 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to invert matrix Vx\n", proc );
    return( -1 );
  }
  
  /* Calcul de la matrice A */
  E_DMMatMul ( Cyx, invVx, A, 3 );
  
  /* Calcul de la transformation affine 
   */
  T->m [0] = A [0];  T->m [1] = A [1];  T->m [2] = A [2];
  T->m [3] = dest_center.x - A[0]*orig_center.x - A[1]*orig_center.y - A[2]*orig_center.z;
  
  T->m [4] = A [3];  T->m [5] = A [4];  T->m [6] = A [5];
  T->m [7] = dest_center.y - A[3]*orig_center.x - A[4]*orig_center.y - A[5]*orig_center.z;
  
  T->m [8]  = A [6];  T->m [9]  = A [7];  T->m [10] = A [8];
  T->m [11] = dest_center.z - A[6]*orig_center.x - A[7]*orig_center.y - A[8]*orig_center.z;
  
  T->m [12] = 0.0;  T->m [13] = 0.0;  T->m [14] = 0.0;  T->m [15] = 1.0;
  
  return ( 1 );
}






/* --------------------------------------------------------------------- 
 *
 * Weighted least squares estimation of 2D transformations
 *
 * --------------------------------------------------------------------- */



/************************************************************
 *
 * subroutines for openmp parallelism
 *
 ************************************************************/

static void *_WLS_Barycenters2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  double *sumWeights           = &((_LinearTrsfAuxiliaryParam*)parameter)->sumWeights;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  double local_sumWeights = 0.0;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;

  for ( i=first; i<=last; i++ ) {
    orig_center->x += thePairs[i]->rho * thePairs[i]->origin.x;
    orig_center->y += thePairs[i]->rho * thePairs[i]->origin.y;
    dest_center->x += thePairs[i]->rho * thePairs[i]->vector.x;
    dest_center->y += thePairs[i]->rho * thePairs[i]->vector.y;
    local_sumWeights += thePairs[i]->rho;
  }
  *sumWeights = local_sumWeights;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_TranslationScaling2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    
    /* Incrementation des matrices */
    cov[0] += thePairs[i]->rho * dest_coord.x * orig_coord.x;    
    cov[4] += thePairs[i]->rho * dest_coord.y * orig_coord.y;    
    
    orig_cov[0] += thePairs[i]->rho * orig_coord.x * orig_coord.x;    
    orig_cov[4] += thePairs[i]->rho * orig_coord.y * orig_coord.y;    
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_Rigid2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y;
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];
    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += thePairs[i]->rho * aTa[n];
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_Similitude2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  double *orig_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_modulus;
  double *dest_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_modulus;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];
  double local_orig_modulus = 0.0;
  double local_dest_modulus = 0.0;

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;

    local_orig_modulus += thePairs[i]->rho * ( orig_coord.x*orig_coord.x + orig_coord.y*orig_coord.y );
    local_dest_modulus += thePairs[i]->rho * ( dest_coord.x*dest_coord.x + dest_coord.y*dest_coord.y );

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y;
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];
    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += thePairs[i]->rho * aTa[n];
  }
  *orig_modulus = local_orig_modulus;
  *dest_modulus = local_dest_modulus;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_SimilitudeScale2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *similitude_criteria  = &((_LinearTrsfAuxiliaryParam*)parameter)->similitude_criteria;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  double *rotationMatrix       = ((_LinearTrsfAuxiliaryParam*)parameter)->rotationMatrix;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  double orig_coord[3] = {0.0, 0.0, 0.0};
  double orig_trans[3] = {0.0, 0.0, 0.0};
  double dest_coord[3] = {0.0, 0.0, 0.0};
  double diff[3] = {0.0, 0.0, 0.0};
  double local_sum = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord[0] = thePairs[i]->origin.x - orig_center->x;
    orig_coord[1] = thePairs[i]->origin.y - orig_center->y;
    dest_coord[0] = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord[1] = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;

    E_DMMatMulVect ( rotationMatrix, orig_coord, orig_trans, 3 );

    diff[0] = dest_coord[0] - orig_trans[0];
    diff[1] = dest_coord[1] - orig_trans[1];

    local_sum += thePairs[i]->rho * ( diff[0] * diff[0] + diff[1] * diff[1] );
  }
  *similitude_criteria = local_sum;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_Affine2DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    
    /* Incrementation des matrices */
    cov[0] += thePairs[i]->rho * dest_coord.x * orig_coord.x;    
    cov[1] += thePairs[i]->rho * dest_coord.x * orig_coord.y;    
    cov[3] += thePairs[i]->rho * dest_coord.y * orig_coord.x;    
    cov[4] += thePairs[i]->rho * dest_coord.y * orig_coord.y;    
    
    orig_cov[0] += thePairs[i]->rho * orig_coord.x * orig_coord.x;    
    orig_cov[1] += thePairs[i]->rho * orig_coord.x * orig_coord.y;    
    orig_cov[4] += thePairs[i]->rho * orig_coord.y * orig_coord.y;    
  }
  orig_cov[3] = orig_cov[1];
  chunk->ret = 1;
  return( (void*)NULL );
}





/************************************************************
 *
 * transformation computation
 *
 ************************************************************/

static int WLS_Estimation_Barycenters_2D( bal_doublePoint *orig_center,
                                          bal_doublePoint *dest_center,
                                          double *sumWeights,
                                          typeChunks *chunks )
{
  char *proc = "WLS_Estimation_Barycenters_2D";
  int n;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;
  *sumWeights = 0.0;



  if ( processChunks( &_WLS_Barycenters2DSubroutine, chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter (2D WLS linear case)\n", proc );
    return( -1 );
  }

  for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
    orig_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.x;
    orig_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.y;
    dest_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.x;
    dest_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.y;
    *sumWeights    += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->sumWeights;
  }

  orig_center->x /= (*sumWeights);
  orig_center->y /= (*sumWeights);
  dest_center->x = orig_center->x + dest_center->x / (*sumWeights);
  dest_center->y = orig_center->y + dest_center->y / (*sumWeights);

  return( 1 );
}





static int WLS_Estimation_Translation_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Translation_2D";

  int n;
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_2D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  free( aux );
  freeChunks( &chunks );



  /* Calcul de la matrice de transformation 
   */
  T->m[0] = 1.0; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = 1.0; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;

  return ( 1 );
}





static int WLS_Estimation_Translation_Scaling_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Translation_Scaling_2D";
  
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;

  int i, n;
  double cov[9], orig_cov[9];
  bal_doublePoint scale;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_2D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    cov[i] = orig_cov[i] = 0.0;

  if ( processChunks( &_WLS_TranslationScaling2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    cov[0] += aux[n].cov[0];
    cov[4] += aux[n].cov[4];

    orig_cov[0] += aux[n].orig_cov[0];
    orig_cov[4] += aux[n].orig_cov[4];
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  scale.x = cov[0] / orig_cov[0];
  scale.y = cov[4] / orig_cov[4];
  
  

  /* Calcul de la matrice de transformation 
   */
  T->m[0] = scale.x; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - scale.x * orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = scale.y; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - scale.y * orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int WLS_Estimation_Rigid_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Rigid_2D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;  
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_2D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_WLS_Rigid2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = 0.0;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = mr[0]; T->m[1] = mr[1]; T->m[2] = mr[2]; 
  T->m[3] = dest_center.x - rog[0];
  
  T->m[4] = mr[3]; T->m[5] = mr[4]; T->m[6] = mr[5]; 
  T->m[7] = dest_center.y - rog[1];
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int WLS_Estimation_Similitude_2D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Similitude_2D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;  
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];
  double orig_modulus = 0.0;
  double dest_modulus = 0.0;
  double CritereRigide = 0.0;
  double echelle = 1.0;

  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_2D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_WLS_Similitude2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
    orig_modulus += aux[n].orig_modulus;
    dest_modulus += aux[n].dest_modulus;
  }



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );



  /* calcul du scaling
   */
  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].rotationMatrix = mr;
  }

  if ( processChunks( &_WLS_SimilitudeScale2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute scaling (2D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( CritereRigide=0.0, n=0; n<chunks.n_allocated_chunks; n++ ) {
    CritereRigide += aux[n].similitude_criteria;
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* calcul de l'echelle */
  echelle = (orig_modulus + dest_modulus - CritereRigide) / ( 2 * orig_modulus );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = 0.0;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = echelle*mr[0]; T->m[1] = echelle*mr[1]; T->m[2] = echelle*mr[2]; 
  T->m[3] = dest_center.x - echelle*rog[0];
  
  T->m[4] = echelle*mr[3]; T->m[5] = echelle*mr[4]; T->m[6] = echelle*mr[5]; 
  T->m[7] = dest_center.y - echelle*rog[1];
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] =  1.0;
  T->m[11] = 0.0;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return( 1 );
}





static int WLS_Estimation_Affine_2D ( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Affine_2D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;
  int i, j, n;
  double A[9], Cyx[9], Vx[9], invVx[9];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_2D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    Cyx[i] = Vx[i] = 0.0;
  
  if ( processChunks( &_WLS_Affine2DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (2D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    Cyx [0] += aux[n].cov[0];
    Cyx [1] += aux[n].cov[1];
    Cyx [3] += aux[n].cov[3];
    Cyx [4] += aux[n].cov[4];
    
    Vx [0] += aux[n].orig_cov[0];
    Vx [1] += aux[n].orig_cov[1];
    Vx [4] += aux[n].orig_cov[4];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* additionnal terms
   */
  Vx[3] = Vx[1];  
  
  /* Normalisation pour stabilite numerique (peut-etre superflu) 
     (n'est pas faite dans le cas non weighted)
  */
  for ( i = 0 ; i < 3 ; i ++ )
  for ( j = 0 ; j < 3 ; j ++ )  {
    Cyx[j + 3*i] = Cyx[j + 3*i] / sumWeights;
    Vx[j + 3*i] = Vx[j + 3*i] / sumWeights;
  }
  
  /* Gestion ad hoc du cas 2D */
  Cyx[8] = 1; 
  Vx[8] = 1; 
  
  /* Inversion de la matrice de variance (3x3) 
     invVx = inv_mat (Vx);*/
  /* Pas confiance dans inv_mat -> on utilise InverseMat4x4 */
  if ( InverseMat3x3( Vx, invVx ) != 3 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to invert matrix Vx\n", proc );
    return( -1 );
  }
  
  /* Calcul de la matrice A */
  E_DMMatMul ( Cyx, invVx, A, 3 );
  
  /* Calcul de la transformation affine 
   */
  T->m [0] = A [0];  T->m [1] = A [1];  T->m [2] = A [2];
  T->m [3] = dest_center.x - A[0]*orig_center.x - A[1]*orig_center.y;
  
  T->m [4] = A [3];  T->m [5] = A [4];  T->m [6] = A [5];
  T->m [7] = dest_center.y - A[3]*orig_center.x - A[4]*orig_center.y;
  
  T->m [8]  = 0.0;  T->m [9]  = 0.0;  T->m [10] = 1.0;
  T->m [11] = 0.0;
  
  T->m [12] = 0.0;  T->m [13] = 0.0;  T->m [14] = 0.0;  T->m [15] = 1.0;
  
  return ( 1 );
}





/* --------------------------------------------------------------------- 
 *
 * Weighted least squares estimation of 3D transformations
 *
 * --------------------------------------------------------------------- */



/************************************************************
 *
 * subroutines for openmp parallelism
 *
 ************************************************************/

static void *_WLS_Barycenters3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  double *sumWeights           = &((_LinearTrsfAuxiliaryParam*)parameter)->sumWeights;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  double local_sumWeights = 0.0;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;

  for ( i=first; i<=last; i++ ) {
    orig_center->x += thePairs[i]->rho * thePairs[i]->origin.x;
    orig_center->y += thePairs[i]->rho * thePairs[i]->origin.y;
    orig_center->z += thePairs[i]->rho * thePairs[i]->origin.z;
    dest_center->x += thePairs[i]->rho * thePairs[i]->vector.x;
    dest_center->y += thePairs[i]->rho * thePairs[i]->vector.y;
    dest_center->z += thePairs[i]->rho * thePairs[i]->vector.z;
    local_sumWeights += thePairs[i]->rho;
  }
  *sumWeights = local_sumWeights;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_TranslationScaling3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;
    
    /* Incrementation des matrices */
    cov[0] += thePairs[i]->rho * dest_coord.x * orig_coord.x;    
    cov[4] += thePairs[i]->rho * dest_coord.y * orig_coord.y;    
    cov[8] += thePairs[i]->rho * dest_coord.z * orig_coord.z;    
    
    orig_cov[0] += thePairs[i]->rho * orig_coord.x * orig_coord.x;    
    orig_cov[4] += thePairs[i]->rho * orig_coord.y * orig_coord.y;    
    orig_cov[8] += thePairs[i]->rho * orig_coord.z * orig_coord.z;    
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_Rigid3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y;
    a[3]   = orig_coord.z - dest_coord.z;
    a[6]   = - ( orig_coord.z + dest_coord.z );  
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];    a[9]   = - a[6];
    a[12]  = - a[3];    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += thePairs[i]->rho * aTa[n];
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_Similitude3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *sum                  = ((_LinearTrsfAuxiliaryParam*)parameter)->sum;
  double *orig_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_modulus;
  double *dest_modulus         = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_modulus;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;
  double a[16], aT[16], aTa[16];
  double local_orig_modulus = 0.0;
  double local_dest_modulus = 0.0;

  for ( n=0; n<16; n++ )
    sum[n] = a[n] = aT[n] = aTa[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;

    local_orig_modulus += thePairs[i]->rho * ( orig_coord.x*orig_coord.x + orig_coord.y*orig_coord.y 
                                               + orig_coord.z*orig_coord.z );
    local_dest_modulus += thePairs[i]->rho * ( dest_coord.x*dest_coord.x + dest_coord.y*dest_coord.y 
                                               + dest_coord.z*dest_coord.z);

    /* Calcul de la matrice somme telle que
       SSD = q^T * A * q ,
       q etant le quaternion recherche
    */
    a[1]   = orig_coord.x - dest_coord.x;  
    a[2]   = orig_coord.y - dest_coord.y;
    a[3]   = orig_coord.z - dest_coord.z;
    a[6]   = - ( orig_coord.z + dest_coord.z );  
    a[7]   = orig_coord.y + dest_coord.y;  
    a[11]  = - ( orig_coord.x + dest_coord.x );

    a[4]   = - a[1];    a[8]   = - a[2];    a[9]   = - a[6];
    a[12]  = - a[3];    a[13]  = - a[7];    a[14]  = - a[11];

    E_DMMatTrans ( a, aT, 4 );
    E_DMMatMul ( aT, a, aTa, 4 );
    for ( n=0; n < 16; n++ ) sum[n] += thePairs[i]->rho * aTa[n];
  }
  *orig_modulus = local_orig_modulus;
  *dest_modulus = local_dest_modulus;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_SimilitudeScale3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *similitude_criteria  = &((_LinearTrsfAuxiliaryParam*)parameter)->similitude_criteria;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  double *rotationMatrix       = ((_LinearTrsfAuxiliaryParam*)parameter)->rotationMatrix;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  double orig_coord[3] = {0.0, 0.0, 0.0};
  double orig_trans[3] = {0.0, 0.0, 0.0};
  double dest_coord[3] = {0.0, 0.0, 0.0};
  double diff[3] = {0.0, 0.0, 0.0};
  double local_sum = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord[0] = thePairs[i]->origin.x - orig_center->x;
    orig_coord[1] = thePairs[i]->origin.y - orig_center->y;
    orig_coord[2] = thePairs[i]->origin.z - orig_center->z;
    dest_coord[0] = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord[1] = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord[2] = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;

    E_DMMatMulVect ( rotationMatrix, orig_coord, orig_trans, 3 );

    diff[0] = dest_coord[0] - orig_trans[0];
    diff[1] = dest_coord[1] - orig_trans[1];
    diff[2] = dest_coord[2] - orig_trans[2];

    local_sum += thePairs[i]->rho * ( diff[0] * diff[0] + diff[1] * diff[1]  + diff[2] * diff[2] );
  }
  *similitude_criteria = local_sum;
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_WLS_Affine3DSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  double *cov                  = ((_LinearTrsfAuxiliaryParam*)parameter)->cov;
  double *orig_cov             = ((_LinearTrsfAuxiliaryParam*)parameter)->orig_cov;
  bal_doublePoint *orig_center = &((_LinearTrsfAuxiliaryParam*)parameter)->orig_center;
  bal_doublePoint *dest_center = &((_LinearTrsfAuxiliaryParam*)parameter)->dest_center;
  FIELD *field                 = ((_LinearTrsfAuxiliaryParam*)parameter)->field;

  typeScalarWeightedDisplacement **thePairs = field->pointer;
  size_t i;
  int n;
  bal_doublePoint orig_coord;
  bal_doublePoint dest_coord;

  for ( n=0; n<9; n++ )
    cov[n] = orig_cov[n] = 0.0;

  for ( i=first; i<=last; i++ ) {
    /* barycentric coordinates
     */
    orig_coord.x = thePairs[i]->origin.x - orig_center->x;
    orig_coord.y = thePairs[i]->origin.y - orig_center->y;
    orig_coord.z = thePairs[i]->origin.z - orig_center->z;
    dest_coord.x = thePairs[i]->origin.x + thePairs[i]->vector.x - dest_center->x;
    dest_coord.y = thePairs[i]->origin.y + thePairs[i]->vector.y - dest_center->y;
    dest_coord.z = thePairs[i]->origin.z + thePairs[i]->vector.z - dest_center->z;
    
    /* Incrementation des matrices */
    cov[0] += thePairs[i]->rho * dest_coord.x * orig_coord.x;    
    cov[1] += thePairs[i]->rho * dest_coord.x * orig_coord.y;    
    cov[2] += thePairs[i]->rho * dest_coord.x * orig_coord.z;    
    cov[3] += thePairs[i]->rho * dest_coord.y * orig_coord.x;    
    cov[4] += thePairs[i]->rho * dest_coord.y * orig_coord.y;    
    cov[5] += thePairs[i]->rho * dest_coord.y * orig_coord.z;    
    cov[6] += thePairs[i]->rho * dest_coord.z * orig_coord.x;    
    cov[7] += thePairs[i]->rho * dest_coord.z * orig_coord.y;    
    cov[8] += thePairs[i]->rho * dest_coord.z * orig_coord.z;    
    
    orig_cov[0] += thePairs[i]->rho * orig_coord.x * orig_coord.x;    
    orig_cov[1] += thePairs[i]->rho * orig_coord.x * orig_coord.y;    
    orig_cov[2] += thePairs[i]->rho * orig_coord.x * orig_coord.z;    
    orig_cov[4] += thePairs[i]->rho * orig_coord.y * orig_coord.y;    
    orig_cov[5] += thePairs[i]->rho * orig_coord.y * orig_coord.z;    
    orig_cov[8] += thePairs[i]->rho * orig_coord.z * orig_coord.z;    
  }
  orig_cov[3] = orig_cov[1];
  orig_cov[6] = orig_cov[2];
  orig_cov[7] = orig_cov[5];
  chunk->ret = 1;
  return( (void*)NULL );
}





/************************************************************
 *
 * transformation computation
 *
 ************************************************************/

static int WLS_Estimation_Barycenters_3D( bal_doublePoint *orig_center,
                                          bal_doublePoint *dest_center,
                                          double *sumWeights,
                                          typeChunks *chunks )
{
  char *proc = "WLS_Estimation_Barycenters_3D";
  int n;

  orig_center->x = orig_center->y = orig_center->z = 0;
  dest_center->x = dest_center->y = dest_center->z = 0;
  *sumWeights = 0.0;



  if ( processChunks( &_WLS_Barycenters3DSubroutine, chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter (3D WLS linear case)\n", proc );
    return( -1 );
  }

  for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
    orig_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.x;
    orig_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.y;
    orig_center->z += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->orig_center.z;
    dest_center->x += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.x;
    dest_center->y += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.y;
    dest_center->z += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->dest_center.z;
    *sumWeights    += ((_LinearTrsfAuxiliaryParam*)chunks->data[n].parameters)->sumWeights;
  }

  orig_center->x /= (*sumWeights);
  orig_center->y /= (*sumWeights);
  orig_center->z /= (*sumWeights);
  dest_center->x = orig_center->x + dest_center->x / (*sumWeights);
  dest_center->y = orig_center->y + dest_center->y / (*sumWeights);
  dest_center->z = orig_center->z + dest_center->z / (*sumWeights);

  return( 1 );
}





static int WLS_Estimation_Translation_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Translation_3D";

  int n;
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_3D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  free( aux );
  freeChunks( &chunks );



  /* Calcul de la matrice de transformation 
   */
  T->m[0] = 1.0; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = 1.0; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = 1.0; 
  T->m[11] = dest_center.z - orig_center.z;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;

  return ( 1 );
}





static int WLS_Estimation_Translation_Scaling_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Translation_Scaling_3D";
  
  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;

  int i, n;
  double cov[9], orig_cov[9];
  bal_doublePoint scale;



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_3D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    cov[i] = orig_cov[i] = 0.0;

  if ( processChunks( &_WLS_TranslationScaling3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    cov[0] += aux[n].cov[0];
    cov[4] += aux[n].cov[4];
    cov[8] += aux[n].cov[8];

    orig_cov[0] += aux[n].orig_cov[0];
    orig_cov[4] += aux[n].orig_cov[4];
    orig_cov[8] += aux[n].orig_cov[8];
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  scale.x = cov[0] / orig_cov[0];
  scale.y = cov[4] / orig_cov[4];
  scale.z = cov[8] / orig_cov[8];
  
  

  /* Calcul de la matrice de transformation 
   */
  T->m[0] = scale.x; T->m[1] = 0.0; T->m[2] = 0.0; 
  T->m[3] = dest_center.x - scale.x * orig_center.x;
  
  T->m[4] = 0.0; T->m[5] = scale.y; T->m[6] = 0.0; 
  T->m[7] = dest_center.y - scale.y * orig_center.y;
  
  T->m[8] = 0.0; T->m[9] = 0.0; T->m[10] = scale.z; 
  T->m[11] = dest_center.z - scale.z * orig_center.z;
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int WLS_Estimation_Rigid_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Rigid_3D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;  
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_3D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_WLS_Rigid3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = orig_center.z;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = mr[0]; T->m[1] = mr[1]; T->m[2] = mr[2]; 
  T->m[3] = dest_center.x - rog[0];
  
  T->m[4] = mr[3]; T->m[5] = mr[4]; T->m[6] = mr[5]; 
  T->m[7] = dest_center.y - rog[1];
  
  T->m[8] = mr[6]; T->m[9] = mr[7]; T->m[10] =  mr[8]; 
  T->m[11] = dest_center.z - rog[2];
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return ( 1 );
}





static int WLS_Estimation_Similitude_3D( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Similitude_3D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;  
  int i, n;
  double a[16], som[16], r[4], q[4];
  double rog[3], mr[9];
  double xb[3];
  double orig_modulus = 0.0;
  double dest_modulus = 0.0;
  double CritereRigide = 0.0;
  double echelle = 1.0;

  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }
  
  

  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_3D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 16 ; i ++ )
    som[i] = 0.0;
  
  if ( processChunks( &_WLS_Similitude3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    for ( i = 0 ; i < 16 ; i ++ )
      som[i] += aux[n].sum[i];
    orig_modulus += aux[n].orig_modulus;
    dest_modulus += aux[n].dest_modulus;
  }



  /* Calcul du vecteur propre associe a la plus petite valeur propre 
   */
  if ( !E_DMVVpropresMatSym( som, r, a, 4 ) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute rotation\n", proc );
    return( -1 );
  }
  
  
  
  if ( fabs( (r[1]-r[0])/r[1] ) < 1e-4 ) {
    fprintf( stderr, "WARNING: %s, the smallest eigenvalues are close\n", proc );
    fprintf( stderr, "    the computed rotation is not robust\n" );
  }
  
  
  
  /* L'erreur sur la rotation est dans r [0] (valeur propre)
   */
  for ( i = 0 ; i < 4; i ++ )
    q [i] = a [ i*4 ];
  
  /* Calcul de la matrice de rotation 
   */
  E_MatRotFromQuat ( q, mr );



  /* calcul du scaling
   */
  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].rotationMatrix = mr;
  }

  if ( processChunks( &_WLS_SimilitudeScale3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute scaling (3D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( CritereRigide=0.0, n=0; n<chunks.n_allocated_chunks; n++ ) {
    CritereRigide += aux[n].similitude_criteria;
  }



  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* calcul de l'echelle */
  echelle = (orig_modulus + dest_modulus - CritereRigide) / ( 2 * orig_modulus );
  
  
  
  xb[0] = orig_center.x;    xb[1] = orig_center.y;    xb[2] = orig_center.z;
  E_DMMatMulVect ( mr, xb, rog, 3 );
  
  /* Calcul de la matrice de transformation */
  T->m[0] = echelle*mr[0]; T->m[1] = echelle*mr[1]; T->m[2] = echelle*mr[2]; 
  T->m[3] = dest_center.x - echelle*rog[0];
  
  T->m[4] = echelle*mr[3]; T->m[5] = echelle*mr[4]; T->m[6] = echelle*mr[5]; 
  T->m[7] = dest_center.y - echelle*rog[1];
  
  T->m[8] = echelle*mr[6]; T->m[9] = echelle*mr[7]; T->m[10] =  echelle*mr[8]; 
  T->m[11] = dest_center.z - echelle*rog[2];
  
  T->m[12] = 0.0; T->m[13] = 0.0; T->m[14] = 0.0; T->m[15] = 1.0;
  
  return( 1 );
}





static int WLS_Estimation_Affine_3D ( _MATRIX *T, FIELD *field )
{
  char *proc = "WLS_Estimation_Affine_3D";

  typeChunks chunks;
  size_t first, last;
  _LinearTrsfAuxiliaryParam *aux;

  bal_doublePoint orig_center, dest_center;
  double sumWeights = 0.0;
  int i, j, n;
  double A[9], Cyx[9], Vx[9], invVx[9];



  /* set return matrix to identity
   */
  _identity_mat( T );


  
  /* some tests
   */
  if ( field->n_selected_pairs < 1 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: empty field\n", proc );
    return( -1 );
  }
  else if ( field->n_selected_pairs < 4 ) {
    if ( _verbose_ )
      fprintf (stderr, "%s: not enough pairs for the estimation\n", proc );
    return( -1 );
  }



  /* allocations
   */

  first = 0;
  last = field->n_selected_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( RETURNED_VALUE_ON_ERROR );
  }
  
  aux = (_LinearTrsfAuxiliaryParam*)malloc( chunks.n_allocated_chunks * sizeof(_LinearTrsfAuxiliaryParam) );
  if ( aux == (_LinearTrsfAuxiliaryParam*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].field = field;
    chunks.data[n].parameters = (void*)(&aux[n]);
  }



  /* barycenters computation
   */

  if ( WLS_Estimation_Barycenters_3D( &orig_center, &dest_center, &sumWeights, 
                                      &chunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute barycenter\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    aux[n].orig_center = orig_center;
    aux[n].dest_center = dest_center;
  }



  /* transformation computation
   */
  for ( i = 0 ; i < 9 ; i ++ )
    Cyx[i] = Vx[i] = 0.0;
  
  if ( processChunks( &_WLS_Affine3DSubroutine, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute transformation (3D WLS linear case)\n", proc );
    free( aux );
    freeChunks( &chunks );
    return( RETURNED_VALUE_ON_ERROR );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    Cyx [0] += aux[n].cov[0];
    Cyx [1] += aux[n].cov[1];
    Cyx [2] += aux[n].cov[2];
    Cyx [3] += aux[n].cov[3];
    Cyx [4] += aux[n].cov[4];
    Cyx [5] += aux[n].cov[5];
    Cyx [6] += aux[n].cov[6];
    Cyx [7] += aux[n].cov[7];
    Cyx [8] += aux[n].cov[8];
    
    Vx [0] += aux[n].orig_cov[0];
    Vx [1] += aux[n].orig_cov[1];
    Vx [2] += aux[n].orig_cov[2];
    Vx [4] += aux[n].orig_cov[4];
    Vx [5] += aux[n].orig_cov[5];
    Vx [8] += aux[n].orig_cov[8];
  }


  
  /* freeing stuff
   */
  freeChunks( &chunks );
  free( aux );



  /* additionnal terms
   */
  Vx[3] = Vx[1];  
  Vx[6] = Vx[2];  
  Vx[7] = Vx[5]; 
  
  /* Normalisation pour stabilite numerique (peut-etre superflu) 
     (n'est pas faite dans le cas non weighted)
  */
  for ( i = 0 ; i < 3 ; i ++ )
  for ( j = 0 ; j < 3 ; j ++ )  {
    Cyx[j + 3*i] = Cyx[j + 3*i] / sumWeights;
    Vx[j + 3*i] = Vx[j + 3*i] / sumWeights;
  }
  
  /* Inversion de la matrice de variance (3x3) 
     invVx = inv_mat (Vx);*/
  /* Pas confiance dans inv_mat -> on utilise InverseMat4x4 */
  if ( InverseMat3x3( Vx, invVx ) != 3 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to invert matrix Vx\n", proc );
    return( -1 );
  }
  
  /* Calcul de la matrice A */
  E_DMMatMul ( Cyx, invVx, A, 3 );
  
  /* Calcul de la transformation affine 
   */
  T->m [0] = A [0];  T->m [1] = A [1];  T->m [2] = A [2];
  T->m [3] = dest_center.x - A[0]*orig_center.x - A[1]*orig_center.y - A[2]*orig_center.z;
  
  T->m [4] = A [3];  T->m [5] = A [4];  T->m [6] = A [5];
  T->m [7] = dest_center.y - A[3]*orig_center.x - A[4]*orig_center.y - A[5]*orig_center.z;
  
  T->m [8]  = A [6];  T->m [9]  = A [7];  T->m [10] = A [8];
  T->m [11] = dest_center.z - A[6]*orig_center.x - A[7]*orig_center.y - A[8]*orig_center.z;
  
  T->m [12] = 0.0;  T->m [13] = 0.0;  T->m [14] = 0.0;  T->m [15] = 1.0;
  
  return ( 1 );
}










/*********************************************************************
 *********************************************************************
 *
 *
 * General procedures
 * 
 *
 **********************************************************************
 **********************************************************************/



/* --------------------------------------------------------------------- */
/* Calcul d'une transformation rigide/similitude/affine aux moindres     */
/*   carres par la  formule explicite utilisant les quaternions.         */
/*   Valable uniquement pour des champs 3D.                              */
/*   Adapte de Gregoire Malandain et Xavier Pennec (these p. 170)        */
/* --------------------------------------------------------------------- */

static int LS_Trsf_Estimation ( _MATRIX *T, FIELD *field, enumTypeTransfo transfo )
{
  char *proc = "LS_Trsf_Estimation";

  switch( transfo ) {
  default :
  case SPLINE :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation not handled yet\n", proc );
    return( -1 );
  case TRANSLATION_2D :
    return( LS_Estimation_Translation_2D( T, field ) );    
  case TRANSLATION_3D :
    return( LS_Estimation_Translation_3D( T, field ) );    
  case TRANSLATION_SCALING_2D :
    return( LS_Estimation_Translation_Scaling_2D( T, field ) );
  case TRANSLATION_SCALING_3D :
    return( LS_Estimation_Translation_Scaling_3D( T, field ) );
  case RIGID_2D :
    return( LS_Estimation_Rigid_2D( T, field ) );
  case RIGID_3D :
    return( LS_Estimation_Rigid_3D( T, field ) );
  case SIMILITUDE_2D :
    return( LS_Estimation_Similitude_2D( T, field ) );
  case SIMILITUDE_3D :
    return( LS_Estimation_Similitude_3D( T, field ) );
  case AFFINE_2D :
    return( LS_Estimation_Affine_2D( T, field ) );
  case AFFINE_3D :
    return( LS_Estimation_Affine_3D( T, field ) );
  }
  return( -1 );
}










/* --------------------------------------------------------------------- */
/* Calcul d'une transformation rigide/similitude/affine aux moindres     */
/* carres par la  formule explicite utilisant les quaternions.           */
/* avec ponderation par la  mesure locale de correlation.                */
/*   Valable uniquement pour des champs 3D.                              */
/*   Adapte de Gregoire Malandain et Xavier Pennec (these p. 170)        */
/* --------------------------------------------------------------------- */

static int WLS_Trsf_Estimation ( _MATRIX *T, FIELD *field, 
                              enumTypeTransfo transfo )
{
  char *proc = "WLS_Trsf_Estimation";

  switch( transfo ) {
  default :
  case SPLINE :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation not handled yet\n", proc );
    return( -1 );
  case TRANSLATION_2D :
    return( WLS_Estimation_Translation_2D( T, field ) );    
  case TRANSLATION_3D :
    return( WLS_Estimation_Translation_3D( T, field ) );    
  case TRANSLATION_SCALING_2D :
    return( WLS_Estimation_Translation_Scaling_2D( T, field ) );
  case TRANSLATION_SCALING_3D :
    return( WLS_Estimation_Translation_Scaling_3D( T, field ) );
  case RIGID_2D :
    return( WLS_Estimation_Rigid_2D( T, field ) );
  case RIGID_3D :
    return( WLS_Estimation_Rigid_3D( T, field ) );
  case SIMILITUDE_2D :
    return( WLS_Estimation_Similitude_2D( T, field ) );
  case SIMILITUDE_3D :
    return( WLS_Estimation_Similitude_3D( T, field ) );
  case AFFINE_2D :
    return( WLS_Estimation_Affine_2D( T, field ) );
  case AFFINE_3D :
    return( WLS_Estimation_Affine_3D( T, field ) );
  }
  return( -1 );
}





/************************************************************
 *
 *
 *
 ************************************************************/



typedef struct {
  bal_transformation *theTrsf;
  FIELD *field;
} _LinearTrsfResidualsParam;





static void *_LinearTrsf_Residuals_2DSubroutine( void *par )
{
  typeChunk *c = (typeChunk *)par;
  _LinearTrsfResidualsParam *parameter = (_LinearTrsfResidualsParam*)c->parameters;
  double *mat  = parameter->theTrsf->mat.m;
  FIELD *field = parameter->field;

  size_t n;
  double x, y, dx, dy;

  for ( n = c->first; n <= c->last; n ++ ) {
    x = field->pointer[n]->origin.x;
    y = field->pointer[n]->origin.y;
    dx = ( x + field->pointer[n]->vector.x - (mat[0]*x + mat[1]*y + mat[3]) );   
    dy = ( y + field->pointer[n]->vector.y - (mat[4]*x + mat[5]*y + mat[7]) );
    field->pointer[n]->error = (float)(dx*dx + dy*dy);
  }
  c->ret = 1;
  return( (void*)NULL );
}



static void *_LinearTrsf_Residuals_3DSubroutine( void *par )
{
    typeChunk *c = (typeChunk *)par;
    _LinearTrsfResidualsParam *parameter = (_LinearTrsfResidualsParam*)c->parameters;
    double *mat  = parameter->theTrsf->mat.m;
    FIELD *field = parameter->field;

  size_t n;
  double x, y, z, dx, dy, dz;

  for ( n = c->first; n <= c->last; n ++ ) {
      x = field->pointer[n]->origin.x;
      y = field->pointer[n]->origin.y;
      z = field->pointer[n]->origin.z;
      dx = ( x + field->pointer[n]->vector.x - (mat[0]*x + mat[1]*y + mat[ 2]*z  + mat[3]) );   
      dy = ( y + field->pointer[n]->vector.y - (mat[4]*x + mat[5]*y + mat[ 6]*z  + mat[7]) );   
      dz = ( z + field->pointer[n]->vector.z - (mat[8]*x + mat[9]*y + mat[10]*z + mat[11]) );
      field->pointer[n]->error = (float)(dx*dx + dy*dy + dz*dz);
  }
  c->ret = 1;
  return( (void*)NULL );
}



int BAL_LinearTrsf_Residuals( bal_transformation* theTrsf, 
                              FIELD * field )
{
  char *proc = "BAL_LinearTrsf_Residuals";
  int n;

  _LinearTrsfResidualsParam p;
  typeChunks chunks;
  size_t first, last;

  first = 0;
  last = field->n_computed_pairs-1;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( -1 );
  }
  
  p.theTrsf = theTrsf;
  p.field = field;

  for ( n=0; n<chunks.n_allocated_chunks; n++ )
    chunks.data[n].parameters = (void*)(&p);





  switch( theTrsf->type ) {
  default :
   if ( _verbose_ )
     fprintf (stderr, "%s: such transformation not handled in switch\n", proc );
   freeChunks( &chunks );
   return( -1 );
    
  case TRANSLATION_2D :
  case TRANSLATION_SCALING_2D :
  case RIGID_2D :
  case SIMILITUDE_2D :
  case AFFINE_2D :

    if ( processChunks( &_LinearTrsf_Residuals_2DSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute residuals (2D linear case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;

  case TRANSLATION_3D :
  case TRANSLATION_SCALING_3D :
  case RIGID_3D :
  case SIMILITUDE_3D :
  case AFFINE_3D :

    if ( processChunks( &_LinearTrsf_Residuals_3DSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute residuals (3D linear case)\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;
    
  }

  freeChunks( &chunks );
  return( 1 );

}





static int _LinearTrsf_Estimation ( bal_transformation* theTrsf, 
                                    FIELD * field,
                                    bal_estimator *estimator )
{
  char *proc="_LinearTrsf_Estimation";
  switch( estimator->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: estimator type not handled in switch", proc );
    return( -1 );
  case TYPE_LS :
  case TYPE_LTS :
    return( LS_Trsf_Estimation( &(theTrsf->mat), field, theTrsf->type ) );
  case TYPE_WLS :
  case TYPE_WLTS :
    return( WLS_Trsf_Estimation( &(theTrsf->mat), field, theTrsf->type ) );
  }
  return( -1 );
}






  
/* ---------------------------------------------------------------------
   Calcul d'une transformation rigide/similitude/affine par LTS. 
   Interet: robustesse. 
   Le critere du LTS est : sum_{i<h} r_{o(i)}^2
   Principe de l'algorithme:
      1. Initialiser (A, t) 
      2. Calculer les residus r_i = || y_i - A x_i - t ||
      3. Considerer une liste d'appariemments correspondant 
         aux h plus faibles r_i 
      4. Sur cette liste, calculer la transformation (A, t)
         aux moindres carres
      5. Reprendre en 2 si la transformation a peu evolue.
--------------------------------------------------------------------- */

static int _LinearTrsf_Trimmed_Estimation ( bal_transformation* theTrsf, 
                                            FIELD * field,
                                            bal_estimator *estimator )
{
  char * proc = "_LinearTrsf_Trimmed_Estimation";

  int nretainedpairs;

  int iter;
  double eps_r;
  double tol_r = 1e-4;
  double eps_t;
  double tol_t = 1e-4;

  _MATRIX T0, dT, Taux;

  int i, j;

#ifdef _ORIGINAL_BALADIN_TOLERANCE_IN_LTS_
  switch( estimator->type ) {
  default :
    break;
  case TYPE_LS :
  case TYPE_LTS :
    tol_r=1e-4;
    tol_t=1e-2;
    break;
  case TYPE_WLS :
  case TYPE_WLTS :
    tol_r=1e-1;
    tol_t=1e-1;
  }
#endif


  /* set return matrix to identity
   */
  BAL_SetTransformationToIdentity( theTrsf );



  /* initial transformation estimation
   */
  field->n_selected_pairs = field->n_computed_pairs;
  if ( _LinearTrsf_Estimation( theTrsf, field, estimator ) <= 0 ) {
    if ( _verbose_ )
      fprintf ( stderr, "%s: error when estimating initial transformation\n", proc );
    return( -1 );
  }

 

  /* --- Allocation memoire pour le sous-champ --- */
  if ( _alloc_mat( &T0, 4, 4 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate matrix #1\n", proc );
    return( -1 );
  }
  if ( _alloc_mat( &Taux, 4, 4 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate matrix #2\n", proc );
    _free_mat( &T0 );
    return( -1 );
  }
  if ( _alloc_mat( &dT, 4, 4 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate matrix #3\n", proc );
    _free_mat( &Taux );
    _free_mat( &T0 );
    return( -1 );
  }



  /* loop initialisation
   */
  for ( iter = 0, eps_r = 2.0 * tol_r, eps_t = 2.0 * tol_t;
        ((eps_r > tol_r) || (eps_t > tol_t)) && (iter < estimator->max_iterations);
        iter ++ ) {
    
    _copy_mat( &(theTrsf->mat), &T0 );

    /* calcul des residus
     */
    if ( BAL_LinearTrsf_Residuals( theTrsf, field ) != 1 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: something goes wrong in the transformation estimation\n", proc );
        fprintf( stderr, "\t iteration #%d of the iterated (trimmed) estimation\n", iter );
      }
      _free_mat( &dT );
      _free_mat( &Taux );
      _free_mat( &T0 );
      return( -1 );
    }

    /* sort residuals
     */
    nretainedpairs = BAL_SelectSmallestResiduals( field, estimator );
    if ( nretainedpairs <= 0 ) {
      if ( _verbose_ )
        fprintf (stderr, "%s: no retained residuals? Returned value was %d\n", proc, nretainedpairs );
      _free_mat( &dT );
      _free_mat( &Taux );
      _free_mat( &T0 );
      
      return( -1 );
    }
    field->n_selected_pairs = nretainedpairs;
    
    /* transformation estimation
     */
    if ( _LinearTrsf_Estimation( theTrsf, field, estimator ) <= 0 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: something goes wrong in the transformation estimation\n", proc );
        fprintf( stderr, "\t iteration #%d of the iterated (trimmed) estimation\n", iter );
      }
      
#ifdef _ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_
      /* OLD BEHAVIOR :
         en cas de probleme, on renvoie l'identite
         et non le resultat courant comme je l'avais fait.
         GM, Sun Feb 19 21:59:57 CET 2012
      */
      /* set return matrix to identity
       */
      theTrsf->mat.m [0] = 1.0;    theTrsf->mat.m [1] = 0.0;    theTrsf->mat.m [2] = 0.0;    theTrsf->mat.m [3] = 0.0;
      theTrsf->mat.m [4] = 0.0;    theTrsf->mat.m [5] = 1.0;    theTrsf->mat.m [6] = 0.0;    theTrsf->mat.m [7] = 0.0;
      theTrsf->mat.m [8] = 0.0;    theTrsf->mat.m [9] = 0.0;    theTrsf->mat.m [10] = 1.0;   theTrsf->mat.m [11] = 0.0;
      theTrsf->mat.m [12] = 0.0;   theTrsf->mat.m [13] = 0.0;   theTrsf->mat.m [14] = 0.0;   theTrsf->mat.m [15] = 1.0;
#else
      _copy_mat( &T0, T );
#endif
      _free_mat( &dT );
      _free_mat( &Taux );
      _free_mat( &T0 );
#ifdef _ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_
      return( -1 );
#else
      return( 1 );
#endif
    }

    
    /* --- Comparaison de T et T0 --- */
    switch ( theTrsf->type ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: transformation type not handled yet\n", proc );
      _copy_mat( &T0, &(theTrsf->mat) );      
       _free_mat( &dT );
      _free_mat( &Taux );
      _free_mat( &T0 );
      return( -1 );
    case TRANSLATION_2D :
    case TRANSLATION_3D :
    case RIGID_2D :
    case RIGID_3D :
      Inverse_RigidTmatrix ( T0, Taux ); 
      Compose_Tmatrix ( Taux, theTrsf->mat, dT ); 
      Norms_Tmatrix ( dT, & eps_r, & eps_t ); 
      break;
    case TRANSLATION_SCALING_2D :
    case TRANSLATION_SCALING_3D :
    case SIMILITUDE_2D :
    case SIMILITUDE_3D :
    case AFFINE_2D :
    case AFFINE_3D :
      eps_r = 0;
      for ( i = 0; i < 3; i ++ )
        for ( j = 0; j < 3; j ++ )
          eps_r += ( T0.m[j+i*T0.c] - theTrsf->mat.m[j+i*theTrsf->mat.c] )
            * ( T0.m[j+i*T0.c] - theTrsf->mat.m[j+i*theTrsf->mat.c] );
      eps_r = sqrt (eps_r);
      eps_t = (T0.m[3] - theTrsf->mat.m[3])*(T0.m[3] - theTrsf->mat.m[3]) 
        + (T0.m[7] - theTrsf->mat.m[7])*(T0.m[7] - theTrsf->mat.m[7])
        + (T0.m[11] - theTrsf->mat.m[11])*(T0.m[11] - theTrsf->mat.m[11]);
      eps_t = sqrt (eps_t);
    }


    
    if ( _verbose_ >= 3 ) {
#ifdef _ORIGINAL_BALADIN_PRINTS_
      switch ( estimator->type ) {
      default: break;
      case TYPE_LS :
      case TYPE_LTS :
        fprintf( stderr, "      LTS Iteration n. %d \r", iter);
        break;
      case TYPE_WLS :
      case TYPE_WLTS :
        fprintf( stderr, "      WLTS Iteration n. %d \r", iter);
        break;
      }
#else
      switch ( estimator->type ) {
      default: break;
      case TYPE_LS :
      case TYPE_LTS :
        fprintf( stderr, "      LTS: iteration #%2d ... \r", iter );
        break;
      case TYPE_WLS :
      case TYPE_WLTS :
        fprintf( stderr, "      WLTS: iteration #%2d ... \r", iter );
        break;
      }
#endif
    }

  }

  _free_mat( &dT );
  _free_mat( &Taux );
  _free_mat( &T0 );
    
  return( 1 );
}

















/* Compute the transformation from the reference image 
   towards the floating one, thus allows to resample the floating
   in the reference frame.
   
   Pairings are in real units.
*/
int BAL_ComputeLinearTransformation( bal_transformation* theTrsf, 
                                     FIELD * field,
                                     bal_estimator *estimator )
{
  char *proc= "BAL_ComputeLinearTransformation";
  switch ( estimator->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation estimation not handled yet\n", proc );
    return( -1 );
  case TYPE_LS :
  case TYPE_WLS :
    field->n_selected_pairs = field->n_computed_pairs;
    return( _LinearTrsf_Estimation( theTrsf, field, estimator ) );
  case TYPE_LTS :
  case TYPE_WLTS :
    return( _LinearTrsf_Trimmed_Estimation( theTrsf, field, estimator ) );
  }
}


