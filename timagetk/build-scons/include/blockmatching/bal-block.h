/*************************************************************************
 * bal-block.h -
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



#ifndef BAL_BLOCK_H
#define BAL_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include <bal-stddef.h>

/* block definition
 */
typedef struct{
  /* block origin wrt x, y, z */
  bal_integerPoint origin;

  bal_flag inclus;  /* entierement inclus dans l'image (apres seuils) */
  bal_flag valid;   /* Oui/Non : ce bloc joue-t-il un role, i.e. apres 
		  les tests sur les intensites et les variances ? */

  double mean;  /* moyenne des intensites dans le bloc */
  double variance;  /* variance des intensites dans le bloc */
  double nxvariance; /* sum( I - moyI )^2, c'est n X variance */
} BLOC;




typedef struct{

  /* block array related members
   */

  BLOC *data;            /* tableau contenant tous les blocs */
  BLOC ***array;         /* allows to index the blocks with [z][y][x] */
  BLOC **pointer;        /* pointeurs sur les blocs : 
			    permet de retrouver les blocs 
			    apres le tri sur la variance */

  /* number of blocks along each dimension */
  bal_sizePoint blocksarraydim;

  size_t n_allocated_blocks;     /* number of allocated blocks 
			= product of dimensions*/

  /* block related members
   */

  /* size of blocks along each dimension */
  bal_integerPoint blockdim;

  /* step to reach next block along each dimension */
  bal_integerPoint step;

  /* border to add in each direction for */
  /* statistics computation */
  bal_integerPoint border;

  /* intensity criteria for point selection
   */
  bal_intensitySelection selection;


  /* calculation related members
   */
  size_t n_valid_blocks;    /* nombre de blocs actifs */

} BLOCS;



extern void BAL_SetVerboseFileInBalBlock( FILE *f );
extern void BAL_SetVerboseInBalBlock( int v );
extern void BAL_IncrementVerboseInBalBlock(  );
extern void BAL_DecrementVerboseInBalBlock(  );



extern void BAL_InitBlocks( BLOCS *blocks ); 

extern void BAL_FreeBlocks( BLOCS *blocks ); 

extern int BAL_AllocateBlocks( BLOCS *blocks, 
			/* image dimensions */
			bal_sizePoint *imagedim, 
			/* blocks dimensions */
			bal_integerPoint *blockdim, 
			/* blocks spacing */
			bal_integerPoint *blockspacing );

extern void BAL_SortBlocks( BLOCS * blocks );


void BAL_PrintListOfBlocks( FILE *f, BLOCS* b );
void BAL_CompareDataAndArrayOfBlocks( FILE *f, BLOCS* b );
void BAL_PrintListOfValidBlocks( FILE *f, BLOCS* b );
void BAL_PrintPointersOfBlocks( FILE *f, BLOCS* b );
void BAL_PrintPointersOfValidBlocks( FILE *f, BLOCS* b );
void BAL_PrintBlockMatchingPyramidalParametersOfBlocks( FILE *f, BLOCS* b );

#ifdef __cplusplus
}
#endif

#endif
