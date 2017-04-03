/*************************************************************************
 * bal-block.c -
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

#include <bal-behavior.h>
#include <bal-block.h>


static int _verbose_ = 1;
static int _debug_ = 0;

static FILE *_verbosef_ = NULL;


void BAL_SetVerboseFileInBalBlock( FILE *f )
{
  _verbosef_ = f;
}

void BAL_SetVerboseInBalBlock( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalBlock(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalBlock(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





/***
 Allocation de memoire a un niveau donne de la pyramide : champ et blocs Image_flo, Image_ref
 
 Dans Ima_flo, les blocs sont espaces de bl_next -> dim_mem_champ_flo
 
 Dans Ima_ref, les blocs sont espaces de bl_next_neigh qui est un parametre relatif,
 donc on calcule les attributs de tous les blocs de Ima_ref
 -> dim_mem_champ_ref
 
 DEVRAIT dependre de la dimension des images (cf Allocate_Field())
 ***/



void BAL_InitBlocks( BLOCS *blocks ) 
{
        blocks->data            = (BLOC *)NULL;
        blocks->array           = (BLOC ***)NULL;
        blocks->pointer         = (BLOC **)NULL;

        blocks->blocksarraydim.x = 0;
        blocks->blocksarraydim.y = 0;
        blocks->blocksarraydim.z = 0;
        blocks->n_allocated_blocks   = 0;

        blocks->blockdim.x = 0;
        blocks->blockdim.y = 0;
        blocks->blockdim.z = 0;

        blocks->step.x = 0;
        blocks->step.y = 0;
        blocks->step.z = 0;

        blocks->border.x = 0;
        blocks->border.y = 0;
        blocks->border.z = 0;

        blocks->selection.low_threshold = -100000;
        blocks->selection.high_threshold = 100000;
        blocks->selection.max_removed_fraction = 0.5;

        blocks->n_valid_blocks = 0;

}




void BAL_FreeBlocks( BLOCS *blocks ) 
{
        if ( blocks->data != NULL ) free( blocks->data );
        if ( blocks->array != NULL ) free( blocks->array );
        if ( blocks->pointer != NULL ) free( blocks->pointer );
        BAL_InitBlocks( blocks );
}






int BAL_AllocateBlocks( BLOCS *blocks, 
                        /* image dimensions */
                        bal_sizePoint *imagedim,
                        /* blocks dimensions */
                        bal_integerPoint *blockdim,
                        /* blocks spacing */
                        bal_integerPoint *blockspacing )
{
  char * proc = "Allocate_Blocks";
  size_t i, x, y, z;
  size_t array_size;
  BLOC ***ppp, **pp, *p;

  BAL_InitBlocks( blocks );

  
  blocks->blockdim.x = blockdim->x;
  blocks->blockdim.y = blockdim->y;
  blocks->blockdim.z = blockdim->z;
  
  blocks->step.x = blockspacing->x;
  blocks->step.y = blockspacing->y;
  blocks->step.z = blockspacing->z;


  /* dimensions of the array of blocks
   */
  blocks->blocksarraydim.x = 0;
  if ( (int)(imagedim->x) - (int)(blocks->blockdim.x) >= 0 ) {
    blocks->blocksarraydim.x = (imagedim->x - blocks->blockdim.x) / blocks->step.x + 1;
  }
  else {
    if ( _verbose_ )
      fprintf( stderr, "%s: X block dimension (%d) larger than image dimension (%lu)\n",
               proc, blocks->blockdim.x, imagedim->x );
    return( -1 );
  }

  blocks->blocksarraydim.y = 0;
  if ( (int)(imagedim->y) - (int)(blocks->blockdim.y) >= 0 ) {
    blocks->blocksarraydim.y = (imagedim->y - blocks->blockdim.y) / blocks->step.y + 1;
  }
  else {
    if ( _verbose_ )
      fprintf( stderr, "%s: Y block dimension (%d) larger than image dimension (%lu)\n",
               proc, blocks->blockdim.y, imagedim->y );
    return( -1 );
  }
  
  if ( imagedim->z == 1 ) {
    blocks->blocksarraydim.z = 1;
    blocks->blockdim.z = 1;
    blocks->step.z = 1;
  }
  else {
    blocks->blocksarraydim.z = 0;
    if ( (int)(imagedim->z) - (int)(imagedim->z-blockdim->z) >= 0 ) {
      blocks->blocksarraydim.z = (imagedim->z - blocks->blockdim.z) / blocks->step.z + 1;
    }
    else {
      if ( _verbose_ )
        fprintf( stderr, "%s: Z block dimension (%d) larger than image dimension (%lu)\n",
                 proc, blocks->blockdim.z, imagedim->z );
      return( -1 );
    }
  }



  /* sizes of allocated spaces:
     - number of blocks
     - pointers
       * _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_ -> address [x][y][z]
       * else [z][y][x]
   */
  blocks->n_allocated_blocks = blocks->blocksarraydim.x * blocks->blocksarraydim.y * blocks->blocksarraydim.z;
  
  if ( _debug_ ) 
    fprintf( stderr, "%s: allocate %lu blocks\n", proc, blocks->n_allocated_blocks );

#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
  array_size  = blocks->blocksarraydim.x * sizeof( BLOC** );
  array_size += blocks->blocksarraydim.x * blocks->blocksarraydim.y * sizeof( BLOC* );
#else
  array_size  = blocks->blocksarraydim.z * sizeof( BLOC** );
  array_size += blocks->blocksarraydim.z * blocks->blocksarraydim.y * sizeof( BLOC* );
#endif



  /* allocations
   */
  blocks->data = ( BLOC * ) calloc ( blocks->n_allocated_blocks, sizeof ( BLOC ) );
  if ( blocks->data == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate blocks\n", proc );
    BAL_InitBlocks( blocks );
    return( -1 );
  }
  
  blocks->pointer = ( BLOC ** ) calloc ( blocks->n_allocated_blocks, sizeof ( BLOC* ) );
  if ( blocks->pointer == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate blocks pointers\n", proc );
    free( blocks->data );
    BAL_InitBlocks( blocks );
    return( -1 );
  }
  
  blocks->array = ( BLOC *** ) malloc ( array_size );
  if ( blocks->array == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate blocks array\n", proc );
    free( blocks->data );
    free( blocks->pointer );
    BAL_InitBlocks( blocks );
    return( -1 );
  }



  /* build array
   */
#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
  ppp = (BLOC***)blocks->array;
  pp = (BLOC**)(ppp + blocks->blocksarraydim.x);
  p = (BLOC*)(blocks->data);
  for ( x=0; x<blocks->blocksarraydim.x; x++, ppp++ ) {
    *ppp = pp;
    for ( y=0; y<blocks->blocksarraydim.y; y++, pp++, p+=blocks->blocksarraydim.z )
      *pp = p;
  }
#else
  ppp = (BLOC***)blocks->array;
  pp = (BLOC**)(ppp + blocks->blocksarraydim.z);
  p = (BLOC*)(blocks->data);
  for ( z=0; z<blocks->blocksarraydim.z; z++, ppp++ ) {
    *ppp = pp;
    for ( y=0; y<blocks->blocksarraydim.y; y++, pp++, p+=blocks->blocksarraydim.x )
      *pp = p;
  }
#endif



  /* fill data
   */
#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
  for ( i = 0, x = 0; x < imagedim->x+1-blocks->blockdim.x; x += blocks->step.x )
  for ( y = 0; y < imagedim->y+1-blocks->blockdim.y; y += blocks->step.y )
  for ( z = 0; z < imagedim->z+1-blocks->blockdim.z; z += blocks->step.z, i++ ) {
    blocks->pointer[i] = &(blocks->data[i]);
    blocks->data[i].origin.x = x;
    blocks->data[i].origin.y = y;
    blocks->data[i].origin.z = z;
  }
#else
  for ( i = 0, z = 0; z < imagedim->z+1-blocks->blockdim.z; z += blocks->step.z )
  for ( y = 0; y < imagedim->y+1-blocks->blockdim.y; y += blocks->step.y )
  for ( x = 0; x < imagedim->x+1-blocks->blockdim.x; x += blocks->step.x, i++ ) {
    blocks->pointer[i] = &(blocks->data[i]);
    blocks->data[i].origin.x = x;
    blocks->data[i].origin.y = y;
    blocks->data[i].origin.z = z;
  }
#endif

  blocks->border.x = 0;
  blocks->border.y = 0;
  blocks->border.z = 0;
  
  return( 1 );
}





/* comparaison de deux variables de type BLOCKPOINTER avec la variance 
 */
int Compare_BLOCKPOINTER ( const void * a, const void * b )
{
        BLOC **aov = (BLOC **) a;
        BLOC **bov = (BLOC **) b;

        if ( (*aov)->variance  >  (*bov)->variance )
                return (-1);
        if ( (*aov)->variance  <  (*bov)->variance )
                return (1);
        return (0);
}




/************************************************************
 * Sort blocks
 * 1. put valid blocks at the beginning of the list
 * 2. sort the valid blocks according to the variance
 ************************************************************/

void BAL_SortBlocks( BLOCS * blocks )
{
  size_t i;
  size_t last = blocks->n_allocated_blocks;
  BLOC *tmp;
  
  /* active blocks are in [0, last-1]
     a block is active if 
     - it is enough points (active == 1)
     - it is not constant  (var > 0.0)
  */
  
  for ( i = 0; 
        i < blocks->n_allocated_blocks
          && blocks->pointer[i]->valid == 1
          && blocks->pointer[i]->variance > 0.0;
        i ++ )
    ;
  
  if ( i < blocks->n_allocated_blocks ) {
    while ( i < last ) {
      if ( blocks->pointer[i]->valid == 1 &&
           blocks->pointer[i]->variance > 0.0 ) {
        i ++;
      }
      else {
        /* switch i and last-1 */
        tmp                     = blocks->pointer[i];
        blocks->pointer[i]      = blocks->pointer[last-1];
        blocks->pointer[last-1] = tmp;
        last --;
      }
    }
  }
  
  blocks->n_valid_blocks = last;
  
  if ( _verbosef_ != NULL) {
    fprintf( _verbosef_, "Nombre de blocs_flo avec variances non nulles = %lu\n",
             blocks->n_valid_blocks);
  }
  
  /* on ne trie donc que les blocs de variance non nulle */
  qsort ( blocks->pointer, blocks->n_valid_blocks, sizeof (BLOC *), &Compare_BLOCKPOINTER);
  
  
  return;
}









/*************************************************************
 *
 *
 *
 *************************************************************/

void BAL_PrintBlock( FILE *f, BLOC* b )
{
  fprintf( f, "(%d,%d,%d), inc=%d, val=%d moy=%f, var=%f diff=%f\n",
           b->origin.x, b->origin.y, b->origin.z, b->inclus, b->valid, b->mean, b->variance, b->nxvariance );
}



void BAL_PrintListOfBlocks( FILE *f, BLOCS* b )
{
  size_t i;
  fprintf( f, "List of blocks: %lu valid among %lu blocks\n", 
           b->n_valid_blocks, b->n_allocated_blocks );
  for ( i=0; i<b->n_allocated_blocks; i++ ) {
    fprintf( f, "#%6lu: ", i );
    BAL_PrintBlock( f, &(b->data[i]) );
  }
}


void BAL_CompareDataAndArrayOfBlocks( FILE *f, BLOCS* b )
{
  size_t i, x, y, z;
  fprintf( f, "List of blocks: %lu valid among %lu blocks\n", 
           b->n_valid_blocks, b->n_allocated_blocks );
#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
  for ( i=0, x=0; x<b->blocksarraydim.x; x++ )
  for ( y=0; y<b->blocksarraydim.y; y++ ) 
  for ( z=0; z<b->blocksarraydim.z; z++, i++ ) {
    fprintf( f, "#%6lu: ", i );
    BAL_PrintBlock( f, &(b->data[i]) );
    fprintf( f, "#(%2lu,%2lu,%2lu): ", z,y,x );
    BAL_PrintBlock( f, &(b->array[x][y][z]) );
  }
#else
  for ( i=0, z=0; z<b->blocksarraydim.z; z++ )
  for ( y=0; y<b->blocksarraydim.y; y++ ) 
  for ( x=0; x<b->blocksarraydim.x; x++, i++ ) {
    fprintf( f, "#%6lu: ", i );
    BAL_PrintBlock( f, &(b->data[i]) );
    fprintf( f, "#(%2lu,%2lu,%2lu): ", z,y,x );
    BAL_PrintBlock( f, &(b->array[z][y][x]) );
  }
#endif
}


void BAL_PrintListOfValidBlocks( FILE *f, BLOCS* b )
{
  size_t i, j;
  fprintf( f, "List of blocks: %lu valid among %lu blocks\n", 
           b->n_valid_blocks, b->n_allocated_blocks );
  for ( i=0, j=0; i<b->n_allocated_blocks; i++ ) {
    if ( b->data[i].valid == 1 ) {
      fprintf( f, "#%4lu(%6lu): ", j, i );
      BAL_PrintBlock( f, &(b->data[i]) );
      j++;
    }
  }
}


void BAL_PrintPointersOfBlocks( FILE *f, BLOCS* b )
{
  size_t i;
  fprintf( f, "Pointers of blocks: %lu valid among %lu blocks\n", 
           b->n_valid_blocks, b->n_allocated_blocks );
  for ( i=0; i<b->n_allocated_blocks; i++ ) {
    fprintf( f, "#%6lu: ", i );
    BAL_PrintBlock( f, b->pointer[i] );
  }
}


void BAL_PrintPointersOfValidBlocks( FILE *f, BLOCS* b )
{
  size_t i, j;
  fprintf( f, "Pointers of blocks: %lu valid among %lu blocks\n", 
           b->n_valid_blocks, b->n_allocated_blocks );
  for ( i=0, j=0; i<b->n_allocated_blocks; i++ ) {
    if ( b->pointer[i]->valid == 1 ) {
      fprintf( f, "#%4lu(%6lu): ", j, i );
      BAL_PrintBlock( f, b->pointer[i] );
      j++;
    }
  }
}



void BAL_PrintBlockMatchingPyramidalParametersOfBlocks( FILE *f, BLOCS* b )
{

  fprintf( f, " valid blocks / blocks = %lu\n", b->n_valid_blocks );
  fprintf( f, " allocated blocks = %lu\n", b->n_allocated_blocks );
  if ( 0 ) {
    fprintf( f, " number of blocks = %lu x %lu x %lu\n", 
             b->blocksarraydim.x, b->blocksarraydim.y, b->blocksarraydim.z );
    fprintf (f, " block dimensions = %d x %d x %d\n", 
             b->blockdim.x, b->blockdim.y, b->blockdim.z );
    fprintf (f, " block step = %d x %d x %d\n", 
             b->step.x, b->step.y, b->step.z );
    fprintf (f, " block border = %d x %d x %d\n", 
             b->border.x, b->border.y, b->border.z );
  }
}
