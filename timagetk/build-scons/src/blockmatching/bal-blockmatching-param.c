/*************************************************************************
 * bal-param.c -
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

#include <bal-blockmatching-param.h>





void BAL_InitBlockMatchingPyramidalParameters( bal_blockmatching_pyramidal_param *p ) 
{
  
  /* voxel/pixel selection
   */
  p->floating_selection.low_threshold = -100000;
  p->floating_selection.high_threshold = 100000;
  p->floating_selection.max_removed_fraction = 0.5;

  p->reference_selection.low_threshold = -100000;
  p->reference_selection.high_threshold = 100000;
  p->reference_selection.max_removed_fraction = 0.5;

  /* blocks geometry
   */
  p->block_dim.x = 4;
  p->block_dim.y = 4;
  p->block_dim.z = 4;

  p->block_border.x = 0;
  p->block_border.y = 0;
  p->block_border.z = 0;

  p->block_spacing.x = 3;
  p->block_spacing.y = 3;
  p->block_spacing.z = 3;

  /* pairing parameters
   */
  p->half_neighborhood_size.x = 3;
  p->half_neighborhood_size.y = 3;
  p->half_neighborhood_size.z = 3;
  
  p->step_neighborhood_search.x = 1;
  p->step_neighborhood_search.y = 1;
  p->step_neighborhood_search.z = 1;

  p->similarity_measure = _SQUARED_CC_;
  p->similarity_measure_threshold = 0.0;

  /* transformation parameters
   */
  p->transformation_type = RIGID_3D;
  
  /* sigma for elastic regularization (only for vectorfield)
   */
  p->elastic_regularization_sigma.lowest.x = 4.0;
  p->elastic_regularization_sigma.lowest.y = 4.0;
  p->elastic_regularization_sigma.lowest.z = 4.0;

  p->elastic_regularization_sigma.highest.x = 4.0;
  p->elastic_regularization_sigma.highest.y = 4.0;
  p->elastic_regularization_sigma.highest.z = 4.0;

  /* estimator parameters
   */
  BAL_InitEstimator( &(p->estimator.highest) );
  BAL_InitEstimator( &(p->estimator.lowest) );


  /* block matching
   */
  p->max_iterations.lowest = 10;
  p->max_iterations.highest = 10;
  p->rms_ending_condition = 0;

  /* parameter for pyramid building
   */
  p->pyramid_lowest_level = 0;
  p->pyramid_highest_level = 3;
  p->pyramid_gaussian_filtering = 0;
  p->blocks_fraction.lowest = 0.5;
  p->blocks_fraction.highest = 1.0;

  /* general purpose parameters
   */
  p->verbosef = NULL;
  p->verbose = 0;
  p->write_def = 0;
  p->vischeck = 0;

}




void BAL_InitBlockMatchingPyramidalParametersForLinearTransformation( bal_blockmatching_pyramidal_param *p ) 
{
  
  /* blocks geometry
   */
  p->block_dim.x = 4;
  p->block_dim.y = 4;
  p->block_dim.z = 4;

  p->block_border.x = 0;
  p->block_border.y = 0;
  p->block_border.z = 0;

  p->block_spacing.x = 3;
  p->block_spacing.y = 3;
  p->block_spacing.z = 3;

  /* pairing parameters
   */
  p->half_neighborhood_size.x = 3;
  p->half_neighborhood_size.y = 3;
  p->half_neighborhood_size.z = 3;
  
  p->step_neighborhood_search.x = 1;
  p->step_neighborhood_search.y = 1;
  p->step_neighborhood_search.z = 1;

  /* estimator parameters
   */
  BAL_InitEstimator( &(p->estimator.highest) );
  BAL_InitEstimator( &(p->estimator.lowest) );
  p->estimator.highest.max_iterations               = p->estimator.lowest.max_iterations = 100;
  p->estimator.highest.retained_fraction            = p->estimator.lowest.retained_fraction = 0.500000;
  p->estimator.highest.standard_deviation_threshold = p->estimator.lowest.standard_deviation_threshold = -1.0;

  /* block matching
   */
  p->max_iterations.highest = p->max_iterations.lowest = 10;
  p->rms_ending_condition = 1;
}





void BAL_InitBlockMatchingPyramidalParametersForVectorfieldTransformation( bal_blockmatching_pyramidal_param *p ) 
{
  
  /* blocks geometry
   */
  p->block_dim.x = 5;
  p->block_dim.y = 5;
  p->block_dim.z = 5;

  p->block_border.x = 0;
  p->block_border.y = 0;
  p->block_border.z = 0;

  p->block_spacing.x = 1;
  p->block_spacing.y = 1;
  p->block_spacing.z = 1;

  /* pairing parameters
   */
  p->half_neighborhood_size.x = 1;
  p->half_neighborhood_size.y = 1;
  p->half_neighborhood_size.z = 1;
  
  p->step_neighborhood_search.x = 1;
  p->step_neighborhood_search.y = 1;
  p->step_neighborhood_search.z = 1;

  /* estimator parameters
   */
  BAL_InitEstimator( &(p->estimator.highest) );
  BAL_InitEstimator( &(p->estimator.lowest) );
  p->estimator.highest.max_iterations               = p->estimator.lowest.max_iterations = 10;
  p->estimator.highest.max_iterations               = p->estimator.lowest.max_iterations = 5;
  p->estimator.highest.retained_fraction            = p->estimator.lowest.retained_fraction = 1.0;
  p->estimator.highest.standard_deviation_threshold = p->estimator.lowest.standard_deviation_threshold = 2.0;

  /* block matching
   */
  p->max_iterations.highest = p->max_iterations.lowest =  10;
  p->rms_ending_condition = 0;

}






void BAL_InitBlockMatchingPyramidalParametersToAuto( bal_blockmatching_pyramidal_param *p ) 
{
  BAL_InitBlockMatchingPyramidalParameters( p );
  
  p->max_iterations.lowest = 10;
  p->max_iterations.highest = 10;
  p->rms_ending_condition = 1;

  p->pyramid_lowest_level = 1;
  p->pyramid_highest_level = 5;

  p->blocks_fraction.highest = 0.8;
  p->blocks_fraction.lowest = 0.3;
}





void BAL_InitBlockMatchingParametersFromPyramidalOnes( bal_blockmatching_pyramidal_param *global, bal_blockmatching_param *onelevel )
{

  /* voxel/pixel selection
   */
  onelevel->floating_selection = global->floating_selection;
  onelevel->reference_selection = global->reference_selection;

  /* blocks geometry
   */
  onelevel->block_dim = global->block_dim;
  onelevel->block_border = global->block_border;
  onelevel->block_spacing = global->block_spacing;

  /* pairing parameters
   */
  onelevel->half_neighborhood_size = global->half_neighborhood_size;
  onelevel->step_neighborhood_search = global->step_neighborhood_search;

  onelevel->similarity_measure = global->similarity_measure;
  onelevel->similarity_measure_threshold = global->similarity_measure_threshold;
  
  /* transformation parameters
  */
  onelevel->transformation_type = global->transformation_type;
  onelevel->estimator = global->estimator.lowest;

  /* parameter for block matching at one level
   */
  onelevel->rms_ending_condition = global->rms_ending_condition;
  
  /* parameter for pyramid building
   */

  /* general purpose parameters
   */
  onelevel->verbosef = global->verbosef;
  onelevel->verbose = global->verbose;
  onelevel->write_def = global->write_def;
  onelevel->vischeck = global->vischeck;
}





void BAL_PrintBlockMatchingPyramidalParameters( FILE* f, bal_blockmatching_pyramidal_param *p )
{

  fprintf( f, "\n" );
  fprintf( f, "GLOBAL PARAMETERS\n" );
  fprintf( f, "--- voxel/pixel selection\n" );

  BAL_PrintIntensitySelection( f, &(p->floating_selection), "p->floating_selection" );
  BAL_PrintIntensitySelection( f, &(p->reference_selection), "p->reference_selection" );

  fprintf( f, "--- blocks geometry\n" );

  BAL_PrintIntegerPoint( f, &(p->block_dim), "p->block_dim" );
  BAL_PrintIntegerPoint( f, &(p->block_border), "p->block_border" );
  BAL_PrintIntegerPoint( f, &(p->block_spacing), "p->block_spacing" );

  fprintf( f, "--- pairing parameters\n" );  

  BAL_PrintIntegerPoint( f, &(p->half_neighborhood_size), "p->half_neighborhood_size" );
  BAL_PrintIntegerPoint( f, &(p->step_neighborhood_search), "p->step_neighborhood_search" );

  BAL_PrintTypeSimilarity( f, p->similarity_measure, "p->similarity_measure = " );
  fprintf( f, "p->similarity_measure_threshold = %f\n", p->similarity_measure_threshold );

  fprintf( f, "--- transformation parameters\n" );  

  BAL_PrintTypeTransformation( f, p->transformation_type, "p->transformation_type = " );

  BAL_PrintDoublePoint( f, &(p->elastic_regularization_sigma.highest), "p->elastic_regularization_sigma (high) = " );
  BAL_PrintDoublePoint( f, &(p->elastic_regularization_sigma.lowest),  "                                (low)  = " );

  fprintf( f, "--- estimator parameters (highest resolution)\n" );
  BAL_PrintEstimator(f, &(p->estimator.highest) );
  fprintf( f, "    estimator parameters (lowest resolution)\n" );
  BAL_PrintEstimator(f, &(p->estimator.lowest) );

  fprintf( f, "--- block matching parameters\n" );

  fprintf( f, "p->max_iterations (high, low) = [%d %d]\n", p->max_iterations.highest, p->max_iterations.lowest );
  fprintf( f, "p->rms_ending_condition = %d\n", p->rms_ending_condition );

  fprintf( f, "--- pyramid building parameters\n" );

  fprintf( f, "p->pyramid_lowest_level = %d\n", p->pyramid_lowest_level );
  fprintf( f, "p->pyramid_highest_level = %d\n", p->pyramid_highest_level );
  fprintf( f, "p->pyramid_gaussian_filtering = %d\n", p->pyramid_gaussian_filtering );
  fprintf( f, "p->blocks_fraction (high, low) = (%f %f)\n", p->blocks_fraction.highest, p->blocks_fraction.lowest );

  fprintf( f, "--- general purpose parameters\n" );

  fprintf( f, "p->verbosef = " );
  if ( p->verbosef == NULL ) fprintf( f, "NULL\n" );
  else fprintf( f, "NOT NULL\n" );
  fprintf( f, "p->verbose = %d\n", p->verbose );
  fprintf( f, "p->write_def = %d\n", p->write_def );
  fprintf( f, "p->vischeck = %d\n", p->vischeck );

  fprintf( f, "\n" );

}





void BAL_PrintBlockMatchingParameters( FILE* f, bal_blockmatching_param *p )
{
  
  fprintf( f, "--- voxel/pixel selection\n" );

  BAL_PrintIntensitySelection( f, &(p->floating_selection), "p->floating_selection" );
  BAL_PrintIntensitySelection( f, &(p->reference_selection), "p->reference_selection" );

  fprintf( f, "--- blocks geometry\n" );

  BAL_PrintIntegerPoint( f, &(p->block_dim), "p->block_dim" );
  BAL_PrintIntegerPoint( f, &(p->block_border), "p->block_border" );
  BAL_PrintIntegerPoint( f, &(p->block_spacing), "p->block_spacing" );

  fprintf( f, "--- pairing parameters\n" );  

  BAL_PrintIntegerPoint( f, &(p->half_neighborhood_size), "p->half_neighborhood_size" );
  BAL_PrintIntegerPoint( f, &(p->step_neighborhood_search), "p->step_neighborhood_search" );

  BAL_PrintTypeSimilarity( f, p->similarity_measure, "p->similarity_measure = " );
  fprintf( f, "p->similarity_measure_threshold = %f\n", p->similarity_measure_threshold );

  /* transformation parameters
   */
  BAL_PrintTypeTransformation( f, p->transformation_type, "p->transformation_type = " );
  BAL_PrintDoublePoint( f, &(p->elastic_regularization_sigma), "p->elastic_regularization_sigma = " );

  fprintf( f, "--- estimator parameters\n" );
  BAL_PrintEstimator(f, &(p->estimator) );

  /* block matching
   */
  fprintf( f, "p->max_iterations = %d\n", p->max_iterations );
  fprintf( f, "p->rms_ending_condition = %d\n", p->rms_ending_condition );

  /* parameter for pyramid building
   */
  fprintf( f, "p->pyramid_level = %d\n", p->pyramid_level );
  fprintf( f, "p->blocks_fraction = %f\n", p->blocks_fraction );

  /* general purpose parameters
   */
  fprintf( f, "p->verbosef = " );
  if ( p->verbosef == NULL ) fprintf( f, "NULL\n" );
  else fprintf( f, "NOT NULL\n" );
  fprintf( f, "p->verbose = %d\n", p->verbose );
  fprintf( f, "p->write_def = %d\n", p->write_def );
  fprintf( f, "p->vischeck = %d\n", p->vischeck );

}





void BAL_PrintBlockMatchingPyramidalParametersForBlocks( FILE *f, bal_blockmatching_pyramidal_param *p )
{
  fprintf ( f, " block dimensions =  %d x %d x %d\n", 
	    p->block_dim.x,  p->block_dim.y,  p->block_dim.z );
  if ( 0 )
    fprintf ( f, " block borders =  %d x %d x %d\n", 
	      p->block_border.x, p->block_border.y, p->block_border.z ); 
  fprintf ( f, " block step  =  %d x %d x %d\n", 
	    p->block_spacing.x, p->block_spacing.y, p->block_spacing.z );

  fprintf ( f, " neighborhood block step =  %d x %d x %d\n", 
	    p->step_neighborhood_search.x, p->step_neighborhood_search.y, p->step_neighborhood_search.z );
  fprintf ( f, " neighborhood dimension =  %d x %d x %d\n", 
	    p->half_neighborhood_size.x, p->half_neighborhood_size.y, p->half_neighborhood_size.z );

  fprintf ( f, "percentage variance : low level = %f , high level = %f )\n",
	    p->blocks_fraction.lowest, p->blocks_fraction.highest );
}

