/*************************************************************************
 * bal-field.c -
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

#include <bal-field.h>



static int _verbose_ = 1;
static int _debug_ = 0;


/*--------------------------------------------------*
 *
 * FIELD MANAGEMENT
 *
 *--------------------------------------------------*/

void Init_Field( FIELD *field )
{
  field->data = (typeScalarWeightedDisplacement*)NULL;
  field->pointer = (typeScalarWeightedDisplacement**)NULL;

  field->n_allocated_pairs = 0;

  field->unit =  VOXEL_UNIT;

  field->vx = 1.0;
  field->vy = 1.0;
  field->vz = 1.0;

  field->n_computed_pairs = 0;

  field->n_selected_pairs = 0;
}



static void Init_ScalarWeightedDisplacement( typeScalarWeightedDisplacement *d )
{
  d->origin.x = 0.0;
  d->origin.y = 0.0;
  d->origin.z = 0.0;
  d->vector.x = 0.0;
  d->vector.y = 0.0;
  d->vector.z = 0.0;
  d->valid = 0;
  d->rho = 0.0;
  d->error = 0.0;
}



/*---------------------------------------------------------
          Allocation d'un champ 3D de type double
----------------------------------------------------------*/

int BAL_AllocateField ( FIELD *field, size_t npairs )
{
  char *proc = "BAL_AllocateField";
  size_t i;

  Init_Field( field );

  if ( npairs <= 0 ) return( -1 );
  
  if ( _debug_ ) 
    fprintf( stderr, "%s: allocate %lu pairs\n", proc, npairs );

  field->data = (typeScalarWeightedDisplacement*)malloc( npairs * sizeof(typeScalarWeightedDisplacement) );
  if ( field->data == (typeScalarWeightedDisplacement*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation failed\n", proc );
    return( -1 );
  }

  field->pointer = (typeScalarWeightedDisplacement**)malloc( npairs * sizeof(typeScalarWeightedDisplacement*) );
  if ( field->pointer == (typeScalarWeightedDisplacement**)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation failed\n", proc );
    free( field->data );
    return( -1 );
  }

  for ( i=0; i<npairs; i++ ) {
    field->pointer[i] = &( field->data[i] );
    Init_ScalarWeightedDisplacement( field->pointer[i] );
  }
  
  field->n_allocated_pairs = npairs;

  return( 1 );
}





/*----------------------------------------------------
         Desallocation d'un champ 3D de type double
-----------------------------------------------------*/

void BAL_FreeField ( FIELD * field )
{
  if ( field->data != NULL ) free( field->data );
  if ( field->pointer != NULL ) free( field->pointer );
  Init_Field( field );
}





/*----------------------------------------------------
         output procedures
-----------------------------------------------------*/

void BAL_PrintField( FILE *f, FIELD *field )
{
  size_t i;
  
  fprintf( f, "Pairings: allocated = %lu, computed = %lu, selected = %lu\n",
           field->n_allocated_pairs, field->n_computed_pairs, field->n_selected_pairs );
  for ( i=0; i<field->n_computed_pairs; i++ ) {
    fprintf( f, "#%2lu: (%f %f %f) <-> (%f %f %f) weight = %f",
             i,
             field->pointer[i]->origin.x, field->pointer[i]->origin.y, field->pointer[i]->origin.z,
             field->pointer[i]->vector.x, field->pointer[i]->vector.y, field->pointer[i]->vector.z,
             field->pointer[i]->rho );
    if ( 1 ) fprintf( f, " error = %20.15f", field->pointer[i]->error );
    fprintf( f, "\n" );
    
  }

}



void BAL_PrintSelectedPairsOfField( FILE *f, FIELD *field )
{
  size_t i;
  
  fprintf( f, "Pairings: allocated = %lu, computed = %lu, selected = %lu\n",
           field->n_allocated_pairs, field->n_computed_pairs, field->n_selected_pairs );
  for ( i=0; i<field->n_selected_pairs; i++ ) {
    fprintf( f, "#%2lu: (%f %f %f) <-> (%f %f %f) error = %f\n",
             i,
             field->pointer[i]->origin.x, field->pointer[i]->origin.y, field->pointer[i]->origin.z,
             field->pointer[i]->vector.x, field->pointer[i]->vector.y, field->pointer[i]->vector.z,
             field->pointer[i]->error );
  }

}



void BAL_PrintValidPairsOfField( FILE *f, FIELD *field )
{
  size_t i, j;
  
  fprintf( f, "Pairings: allocated = %lu, computed = %lu, selected = %lu\n",
           field->n_allocated_pairs, field->n_computed_pairs, field->n_selected_pairs );
  for ( i=0, j=0; i<field->n_computed_pairs; i++ ) {
    if ( field->pointer[i]->valid ) {
      fprintf( f, "#%2lu: (%f %f %f) <-> (%f %f %f) error = %f\n",
               j,
               field->pointer[i]->origin.x, field->pointer[i]->origin.y, field->pointer[i]->origin.z,
               field->pointer[i]->vector.x, field->pointer[i]->vector.y, field->pointer[i]->vector.z,
               field->pointer[i]->error );
      j++;
    }
  }

}





/*--------------------------------------------------*
 *
 * MISC
 *
 *--------------------------------------------------*/

void BAL_ChangeFieldToVoxelUnit(  FIELD *field )
{
  size_t i;
  
  /* nothing to do
   */
  if ( field->unit == VOXEL_UNIT ) return;

  for (i = 0; i < field->n_computed_pairs; i++) {
    field->pointer[i]->origin.x /= field->vx;
    field->pointer[i]->origin.y /= field->vy;
    field->pointer[i]->origin.z /= field->vz;
    field->pointer[i]->vector.x /= field->vx;
    field->pointer[i]->vector.y /= field->vy;
    field->pointer[i]->vector.z /= field->vz;
  }
  
  field->unit = VOXEL_UNIT;
}





void BAL_ChangeFieldToRealUnit(  FIELD *field )
{
  size_t i;
  
  /* nothing to do
   */
  if ( field->unit == REAL_UNIT ) return;

  for (i = 0; i < field->n_computed_pairs; i++) {
    field->pointer[i]->origin.x *= field->vx;
    field->pointer[i]->origin.y *= field->vy;
    field->pointer[i]->origin.z *= field->vz;
    field->pointer[i]->vector.x *= field->vx;
    field->pointer[i]->vector.y *= field->vy;
    field->pointer[i]->vector.z *= field->vz;
  }
  
  field->unit = REAL_UNIT;
}





/*--------------------------------------------------*
 *
 * MISC
 *
 *--------------------------------------------------*/

void CreateFileDef( FIELD *field,
                    char *nom_image, char *nom_champ )
{
  size_t i;
  FILE *def;
  
  def = fopen( nom_champ, "w");
  if ( def == NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "CreateFileDef: unable to open '%s' for writing\n", nom_champ );
    return;
  }
  
  fprintf(def, "DEF=[\n");
  for(i = 0; i < field->n_computed_pairs; i++)
    fprintf(def, "%f %f %f %f %f %f %f\n", 
            field->pointer[i]->origin.x, field->pointer[i]->origin.y, field->pointer[i]->origin.z,
            field->pointer[i]->vector.x, field->pointer[i]->vector.y, field->pointer[i]->vector.z,
            field->pointer[i]->rho );
  fprintf(def, "];\n");
  fprintf(def, "X=DEF(:,1);\n");
  fprintf(def, "Y=DEF(:,2);\n");
  fprintf(def, "Z=DEF(:,3);\n");
  fprintf(def, "U=DEF(:,4);\n");
  fprintf(def, "V=DEF(:,5);\n");
  fprintf(def, "W=DEF(:,6);\n");
  fprintf(def, "RHO=DEF(:,7);\n");
  fprintf(def, "image_name = '%s';\n", nom_image );

  fclose(def);
}







