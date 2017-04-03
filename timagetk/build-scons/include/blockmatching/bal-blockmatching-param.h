/*************************************************************************
 * bal-param.h -
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



#ifndef BAL_BLOCKMATCHING_PARAM_H
#define BAL_BLOCKMATCHING_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include <bal-stddef.h>
#include <bal-estimator.h>





typedef struct bal_blockmatching_pyramidal_param {

  /* voxel/pixel selection

   Thresholds are considered if they are in the range of image encoding.
   Eg: for images encoded on unsigned char, a low threshold strictly below 0
       is not considered, and a high threshold strictly over 255 is not 
       considered.

   If thresholds are to be considered, only points strictly between the two 
   thresholds are taken into account: the block is valid if the fraction
   of removed voxels (wrt to the number of voxels in the block) is lower than 
   'max_removed_fraction'
  */
  
  bal_intensitySelection floating_selection;
  bal_intensitySelection reference_selection;



  /* blocks geometry

     - size of a block
     - border to be added for statistics computation :
       dimensions for computation will be 'blockdim + 2*border'
     - spacing between two consecutive blocks in the floating
       image
     
   */
  bal_integerPoint block_dim;
  bal_integerPoint block_border;
  bal_integerPoint block_spacing;



  /* pairing parameters
     - half size of the neighboorhood search for similar blocks
     - spacing between blocks in the the neighboorhood search

     - similarity measure
     - threshold on this measure
   */
  bal_integerPoint half_neighborhood_size;
  bal_integerPoint step_neighborhood_search;

  enumTypeSimilarity similarity_measure;
  double similarity_measure_threshold;


  
  /* transformation parameters
     - transformation type
     - sigma for elastic regularization (only for vectorfield)
     - fraction of keeped points for least trimmed squares
  */

  enumTypeTransfo transformation_type;
  bal_pyramidDoublePoint elastic_regularization_sigma;
  bal_pyramidEstimator estimator;

  

  /* parameter for block matching at one level
     - maximal number of iteration
     - Root Mean Square ending condition
   */
  bal_pyramidInt max_iterations;
  int rms_ending_condition;
  
  
  /* parameter for pyramid building
     - lowest (ie high resolution) level of the pyramid
       0 : original geometry
       1 : first level, etc
     - highest level of the pyramid
     - gaussian filtering to build the pyramid

     - fraction of (floating) blocks to be considered at the lowest level
     - fraction of (floating) blocks to be considered at the highest level
   */
  int pyramid_lowest_level;
  int pyramid_highest_level;
  int pyramid_gaussian_filtering;
  bal_pyramidDouble blocks_fraction;
  

  /* general purpose parameters
   */

  /* informations lors de l'execution ? affichees, sauvees ou RAS */
  FILE *verbosef;

  /* informations minimales lors de l'execution */
  int verbose;

  /* save on disk the pairing fields */
  int write_def;

  /* visually check the process with intermediate images saved on disk */
  int vischeck;


} bal_blockmatching_pyramidal_param;





typedef struct bal_blockmatching_param {

  /* voxel/pixel selection

   Thresholds are considered if they are in the range of image encoding.
   Eg: for images encoded on unsigned char, a low threshold strictly below 0
       is not considered, and a high threshold strictly over 255 is not 
       considered.

   If thresholds are to be considered, only points strictly between the two 
   thresholds are taken into account: the block is valid if the fraction
   of removed voxels (wrt to the number of voxels in the block) is lower than 
   'max_removed_fraction'
  */
  
  bal_intensitySelection floating_selection;
  bal_intensitySelection reference_selection;



  /* blocks geometry

     - size of a block
     - border to be added for statistics computation :
       dimensions for computation will be 'blockdim + 2*border'
     - spacing between two consecutive blocks in the floating
       image
     
   */
  bal_integerPoint block_dim;
  bal_integerPoint block_border;
  bal_integerPoint block_spacing;



  /* pairing parameters
     - half size of the neighboorhood search for similar blocks
     - spacing between blocks in the the neighboorhood search

     - similarity measure
     - threshold on this measure
   */
  bal_integerPoint half_neighborhood_size;
  bal_integerPoint step_neighborhood_search;

  enumTypeSimilarity similarity_measure;
  double similarity_measure_threshold;


  
  /* transformation parameters
     - transformation type
     - sigma for elastic regularization (only for vectorfield)
     - estimator type
  */

  enumTypeTransfo transformation_type;
  bal_doublePoint elastic_regularization_sigma;
  bal_estimator estimator;

  

  /* parameter for block matching at one level
     - maximal number of iteration
     - Root Mean Square ending condition
   */
  int max_iterations;
  int rms_ending_condition;
  
  
  /* parameter for pyramid building
     - level of the pyramid
     - fraction of (floating) blocks to be considered 
   */
  int pyramid_level;
  double blocks_fraction;
  

  /* general purpose parameters
   */

  /* informations lors de l'execution ? affichees, sauvees ou RAS */
  FILE *verbosef;

  /* informations minimales lors de l'execution */
  int verbose;

  /* save on disk the pairing fields */
  int write_def;

  /* visually check the process with intermediate images saved on disk */
  int vischeck;


} bal_blockmatching_param;


extern void BAL_InitBlockMatchingPyramidalParameters( bal_blockmatching_pyramidal_param *p );
extern void BAL_InitBlockMatchingPyramidalParametersForLinearTransformation( bal_blockmatching_pyramidal_param *p ); 
extern void BAL_InitBlockMatchingPyramidalParametersForVectorfieldTransformation( bal_blockmatching_pyramidal_param *p );
extern void BAL_InitBlockMatchingPyramidalParametersToAuto( bal_blockmatching_pyramidal_param *p );
extern void BAL_InitBlockMatchingParametersFromPyramidalOnes( bal_blockmatching_pyramidal_param *global, 
							      bal_blockmatching_param *onelevel );

extern void BAL_PrintBlockMatchingPyramidalParameters( FILE* f, bal_blockmatching_pyramidal_param *p );
extern void BAL_PrintBlockMatchingParameters( FILE* f, bal_blockmatching_param *p );
extern void BAL_PrintBlockMatchingPyramidalParametersForBlocks( FILE *f, bal_blockmatching_pyramidal_param *p );

#ifdef __cplusplus
}
#endif

#endif
