/*************************************************************************
 * bal-matching.c -
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
#include <time.h>

#ifndef WIN32
#include <sys/time.h>
#endif


#include <bal-blockmatching.h>

#include <bal-block-tools.h>
#include <bal-field-tools.h>
#include <bal-transformation-tools.h>
#include <bal-blockmatching-param-tools.h>
#include <bal-pyramid.h>
#include <bal-vectorfield.h>

#include <bal-behavior.h>





typedef struct {
  bal_doublePoint corner[2][2][2];
} imageCorners;







#define NAMESIZE 256
#define _BALADIN_WRITE_FLOATING_IMAGE_EVOLUTION_ 0

static int _verbose_ = 1;
static int _debug_ = 0;
static int _trace_ = 0;
static int _time_ = 0;

void BAL_SetVerboseInBalBlockMatching( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalBlockMatching(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalBlockMatching(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}

void BAL_SetDebugInBalBlockMatching( int v )
{
  _debug_ = v;
}

void BAL_IncrementDebugInBalBlockMatching(  )
{
  _debug_ ++;
}

void BAL_DecrementDebugInBalBlockMatching(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}

void BAL_SetTraceInBalBlockMatching( int v )
{
  _trace_ = v;
}

void BAL_IncrementTraceInBalBlockMatching(  )
{
  _trace_ ++;
}

void BAL_DecrementTraceInBalBlockMatching(  )
{
  _trace_ --;
  if ( _trace_ < 0 ) _trace_ = 0;
}

void BAL_SetTimeInBalBlockMatching( int v )
{
  _time_ = v;
}

void BAL_IncrementTimeInBalBlockMatching(  )
{
  _time_ ++;
}

void BAL_DecrementTimeInBalBlockMatching(  )
{
  _time_ --;
  if ( _time_ < 0 ) _time_ = 0;
}






/*******************************************************
 *
 * static procedures
 *
 *******************************************************/


#ifndef WIN32
/* time measurements
 */
static double _GetTime() 
{
  struct timeval tv;
  gettimeofday(&tv, (void *)0);
  return ( (double) tv.tv_sec + tv.tv_usec*1e-6 );
}

static double _GetClock() 
{
  return ( (double) clock() / (double)CLOCKS_PER_SEC );
}

static void _PrintTime( FILE *f, char *s,
                        double time_init, double clock_init,
                        double time_exit, double clock_exit )
{
  if ( s != (char*) NULL ) 
    fprintf( f, "%s:", s );
  else 
    fprintf( f, "\t" );
      
  fprintf( f, " elapsed (real) time = %f\n", time_exit - time_init );
  fprintf( f, "\t elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
  fprintf( f, "\t ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
}
#endif

static int BAL_ChangeTransformationType( bal_transformation *t,
                                         enumTypeTransfo type,
                                         bal_image *ref );


/* rms computation
 */
static int _TransformCorners( imageCorners *f, imageCorners *t,
                              bal_transformation *theTr );
static double _computeRMS( imageCorners *c1, imageCorners *c2 );


/* blocks and field management
 */
static int BAL_AllocateBlocksAndField( FIELD *field, 
                                         BLOCS *blocs_flo,
                                         BLOCS *blocs_ref,
                                         bal_image *image_flo,
                                         bal_image *image_ref,
                                         bal_blockmatching_param *param );
static void BAL_FreeBlocksAndField( FIELD *field, BLOCS *blocs_flo, BLOCS *blocs_ref );

/* misc
 */
static void  WriteImageVoxelsActifs( size_t n_allocated_blocks, BLOCS *blocs, 
                                     bal_image *Inrimage_sub, char *nom_image );



/*******************************************************
 *
 * end of static procedures
 *
 *******************************************************/









/* main procedure
 *
 * compute the transformation T that allows to resample
 * I_flo onto I_ref, by I_flo (o T_init) o T
 *
 *
 *
 */

bal_transformation *BAL_PyramidalBlockMatching( bal_image *theInrimage_ref,
                                                bal_image *theInrimage_flo,
                                                bal_transformation *theLeftTransformation,
                                                bal_transformation *theInitResult,
                                                bal_blockmatching_pyramidal_param *theParam )
{
  char *proc = "BAL_PyramidalBlockMatching";
  
  int pyramid_highest_level;
  bal_pyramid_level *pyramid_level = NULL;
  int l;

  bal_image *Inrimage_flo = (bal_image *)NULL, smoothed_flo;
  bal_image *Inrimage_ref = (bal_image *)NULL, subsampled_ref;
  bal_transformation subsampling_trsf;
  bal_transformation *theResultTransformation, *resultTransformation;

  bal_transformation *transformation = (bal_transformation *)NULL;
  bal_transformation level_trsf;

  bal_blockmatching_pyramidal_param param = *theParam;

  BAL_InitImage( &smoothed_flo, NULL, 0, 0, 0, 0, theInrimage_flo->type );
  BAL_InitImage( &subsampled_ref, NULL, 0, 0, 0, 0, theInrimage_ref->type );
  BAL_InitTransformation( &subsampling_trsf );
  BAL_InitTransformation( &level_trsf );



  /**************************************************
   * adjust parameters
   **************************************************/

  BAL_AdjustBlockMatchingPyramidalParameters( theInrimage_ref, &param );
  if ( _debug_ ) BAL_PrintBlockMatchingPyramidalParameters( stderr, &param );
  if ( param.verbosef != NULL ) {
    fprintf( param.verbosef, "-------------------------------------------------------\n" );
    fprintf( param.verbosef, "---------- adjusted registration parameters -----------\n" );
    BAL_PrintBlockMatchingPyramidalParameters( param.verbosef, &param );
    fprintf( param.verbosef, "-------------------------------------------------------\n" );
  }


  /**************************************************
   * adjust transformation
   *
   * theLeftTransformation == NULL && theInitResult == NULL
   *   allocate theResultTransformation
   *   set it with BAL_ComputeImageToImageTransformation()
   *   change it to the right type
   * theLeftTransformation == NULL && theInitResult != NULL
   *   set theInitResult to the right type
   * theLeftTransformation != NULL && theInitResult == NULL
   *   allocate theResultTransformation to identity
   *   change it to the right type
   * theLeftTransformation != NULL && theInitResult != NULL
   *   set theInitResult to the right type
   **************************************************/

  resultTransformation = (bal_transformation*)NULL;
  theResultTransformation = (bal_transformation*)NULL;

  if ( theInitResult == (bal_transformation*)NULL ) {
    /* no result transformation
     * allocate one with the good type
     * it is set as the identity
     */
    theResultTransformation = (bal_transformation*)malloc( sizeof(bal_transformation) );
    if ( theResultTransformation == (bal_transformation*)NULL ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: can not allocate initial result transformation container\n", proc );
      return( (bal_transformation*)NULL );
    }

    BAL_InitTransformation( theResultTransformation );

    if ( BAL_AllocTransformation( theResultTransformation,
                                  param.transformation_type,
                                  theInrimage_ref ) != 1 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: can not allocate initial result transformation\n", proc );
      return( (bal_transformation*)NULL );
    }
    if ( theLeftTransformation == (bal_transformation*)NULL ) {
      /* no init transformation, compute the transformation that
       * superimposes the FOV centers if the images have different geometries
       */
      if ( theInrimage_ref->ncols != theInrimage_flo->ncols
                    || theInrimage_ref->nrows != theInrimage_flo->nrows
                    || theInrimage_ref->nplanes != theInrimage_flo->nplanes
                    || fabs( theInrimage_ref->vx - theInrimage_flo->vx )/theInrimage_ref->vx > 0.001
                    || fabs( theInrimage_ref->vy - theInrimage_flo->vy )/theInrimage_ref->vy > 0.001
                    || fabs( theInrimage_ref->vz - theInrimage_flo->vz )/theInrimage_ref->vz > 0.001
                    ) {
        if ( BAL_ComputeImageToImageTransformation( theInrimage_ref,
                                                    theInrimage_flo,
                                                    theResultTransformation ) != 1 ) {
          if ( _verbose_ )
              fprintf( stderr, "%s: can not compute initial result transformation\n", proc );
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
          return( (bal_transformation*)NULL );
        }
      }
    }
    resultTransformation = theResultTransformation;
  }
  else {
    resultTransformation = theInitResult;
  }

  if ( resultTransformation->type != param.transformation_type ) {
    /* this case may occur when the initial result transformation
     * has been read from a file
     */
    if ( BAL_ChangeTransformationType( resultTransformation,
                                       param.transformation_type,
                                       theInrimage_ref ) != 1 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: can not change initial result transformation type (NULL init trsf case)\n", proc );
      if ( theInitResult == (bal_transformation*)NULL ) {
        BAL_FreeTransformation( theResultTransformation );
        free( theResultTransformation );
      }
      return( (bal_transformation*)NULL );
    }
  }



  /**************************************************
   * pyramid level construction
     from #0 to #pyramid_highest_level
   **************************************************/
  pyramid_level = (bal_pyramid_level *)malloc( (param.pyramid_highest_level+1) * sizeof(bal_pyramid_level) ); 
  if ( pyramid_level == (bal_pyramid_level *)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate pyramid levels\n", proc );
    if ( theInitResult == (bal_transformation*)NULL ) {
      BAL_FreeTransformation( theResultTransformation );
      free( theResultTransformation );
    }
    return( (bal_transformation*)NULL );
  }
  
  pyramid_highest_level = _ComputePyramidLevel( pyramid_level, 
                                                param.pyramid_highest_level,
                                                theInrimage_ref,
                                                &param );
  if ( pyramid_highest_level < 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute pyramid levels parameters\n", proc );
    free( pyramid_level );
    if ( theInitResult == (bal_transformation*)NULL ) {
      BAL_FreeTransformation( theResultTransformation );
      free( theResultTransformation );
    }
    return( (bal_transformation*)NULL );
  }


  /* allocation of the auxiliary floating image (if necessary)
     (to be smoothed in case of gaussian filtering)
   */
 
  if ( param.pyramid_gaussian_filtering ) {
    if ( BAL_InitAllocImage( &smoothed_flo, "auxiliary_floating_image.inr", 
                             theInrimage_flo->ncols, theInrimage_flo->nrows,
                             theInrimage_flo->nplanes, theInrimage_flo->vdim,
                             theInrimage_flo->type ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to allocate auxiliary floating image\n", proc );
      free( pyramid_level );
      if ( theInitResult == (bal_transformation*)NULL ) {
        BAL_FreeTransformation( theResultTransformation );
        free( theResultTransformation );
      }
      return( (bal_transformation*)NULL );
    }
    smoothed_flo.vx = theInrimage_flo->vx;
    smoothed_flo.vy = theInrimage_flo->vy;
    smoothed_flo.vz = theInrimage_flo->vz;
    Inrimage_flo =  &smoothed_flo;
  }
  else {
    Inrimage_flo = theInrimage_flo;
  }

  
  
  /* allocation of the subsampling matrix
     that is T_{original-reference <- subsampled-reference}
   */

  if ( BAL_AllocTransformation( &subsampling_trsf, AFFINE_3D, (bal_image *)NULL ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate subsampling transformation\n", proc );
    BAL_FreeImage( &smoothed_flo );
    free( pyramid_level );
    if ( theInitResult == (bal_transformation*)NULL ) {
      BAL_FreeTransformation( theResultTransformation );
      free( theResultTransformation );
    }
    return( (bal_transformation*)NULL );
  }


  /* be verbose
   */
  if ( param.verbosef != NULL ) {
    fprintf(param.verbosef, "\tBlock dimensions: %d %d %d\n", 
            param.block_dim.x, param.block_dim.y, param.block_dim.z);
    fprintf(param.verbosef, "\tBlock spacing: %d %d %d\n", 
            param.block_spacing.x, param.block_spacing.y, param.block_spacing.z);
    fprintf(param.verbosef, "\tHalf-size of exploration neighborhood: %d %d %d\n",
            param.half_neighborhood_size.x, param.half_neighborhood_size.y,
            param.half_neighborhood_size.z );
    
    fprintf(param.verbosef, "\n********** PYRAMID BEGINS **********\n");
  }





  /*****************************************
   *
   * pyramidal matching: loop over scales
   *
   *****************************************/
  
  for ( l=pyramid_highest_level; l>=param.pyramid_lowest_level; l-- ) {

    /* no iterations at this level
     */
    if ( pyramid_level[l].param.max_iterations <= 0 )
      continue;

    if ( _verbose_ ) {
      fprintf(stderr," - processing level #%2d          \n", l );
      if ( _debug_ )
        fprintf(stderr," - retained block fraction %f  \n", pyramid_level[l].param.blocks_fraction );
    }
    
    /* be verbose
     */
    if ( param.verbosef != NULL ) {
      _PrintPyramidLevel( param.verbosef, &(pyramid_level[l]), 1 );
    }

    /* allocation of subsampled reference image
     */
    if ( l > 0 ) {
      if ( BAL_InitAllocImage( &subsampled_ref, "subsampled_reference_image.inr", 
                               pyramid_level[l].ncols, pyramid_level[l].nrows,
                               pyramid_level[l].nplanes, theInrimage_ref->vdim,
                               theInrimage_ref->type ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate subsampled reference image at level %d\n", proc, l );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }
      subsampled_ref.vx = (double)theInrimage_ref->ncols * theInrimage_ref->vx / (double)subsampled_ref.ncols;
      subsampled_ref.vy = (double)theInrimage_ref->nrows * theInrimage_ref->vy / (double)subsampled_ref.nrows;
      subsampled_ref.vz = (double)theInrimage_ref->nplanes * theInrimage_ref->vz / (double)subsampled_ref.nplanes;
      Inrimage_ref =  &subsampled_ref;
    }
    else {
      Inrimage_ref = theInrimage_ref;
    }





    /* this is not the lowest level
       - compute a subsampling transformation
       - compute the subsampled reference image
       - allocate a transformation for computation = 'theTr o subsampling'
         
    */

    if ( l > 0 ) {
      /* computation of subsampling matrix 
       * ie T_{original-reference <- subsampled-reference}
       */
      if ( BAL_ComputeImageToImageTransformation( &subsampled_ref, theInrimage_ref, &subsampling_trsf ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to compute subsampling transformation at level %d (1)\n", proc, l );
        if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }

      if ( _debug_ >= 2 )     
        BAL_PrintTransformation( stderr, &subsampling_trsf, " BAL_PyramidalBlockMatching: subsampling matrix (real)" );

      /* computation of subsampled reference image
       */
      if ( _ComputeSubsampledImage( &subsampled_ref, theInrimage_ref, &subsampling_trsf,
                                    &(pyramid_level[l]), param.pyramid_gaussian_filtering ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute subsampled reference image at level %d\n", proc, l );
        if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }

      /* allocation of transformation for level l 
         it is the transformation that allows to goes from subsampled reference
         to floating image, hence it is equal to
         T_{floating <- reference} o T_{original-reference <- subsampled-reference}
         ie
         theTr o subsampling_trsf
      */

      if ( BAL_AllocTransformation( &level_trsf, resultTransformation->type, &subsampled_ref ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate transformation at level %d\n", proc, l );
        if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }
      
      /* computation of input transformation for this pyramid level
       * ie composition of the transformation with the resampling matrix
       * level_trsf = theTr o subsampling_trsf
       * we have level_trsf = T_{floating <- subsampled-reference}
      */
      if ( BAL_TransformationComposition( &level_trsf, resultTransformation, &subsampling_trsf ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute transformation at level %d (1)\n", proc, l );
        if ( l > 0 ) BAL_FreeTransformation( &level_trsf );
        if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }

      if ( _debug_ >= 2 ) {
        fprintf(stderr,"  - Level #%2d\n", l );
        BAL_PrintTransformation( stderr, resultTransformation, "resultTransformation" );
        BAL_PrintTransformation( stderr, &subsampling_trsf, "&subsampling_trsf" );
        BAL_PrintTransformation( stderr, &level_trsf, "&level_trsf" );
      }
      
      if ( _debug_ >= 2 ) {
        BAL_PrintTransformation( stderr, &subsampling_trsf, "=== sous-echantillonnage" );
        BAL_PrintTransformation( stderr, &level_trsf, "    level_trsf apres sous-echantillonnage" );
      }

      transformation = &level_trsf;
    }
    else {
      /* case l=0, ie lowest level of the pyramid
       * where original images are used
       */
      transformation = resultTransformation;
    }


    /* smoothing (if required) of floating image
     * recall that (see above) we have
     * if ( param.pyramid_gaussian_filtering )
     *   smoothed_flo is allocated
     *   Inrimage_flo =  &smoothed_flo;
     * else
     *   Inrimage_flo = theInrimage_flo;
     *
     * if is weird that the same parameter are used than for the reference image ...
     */
    if ( param.pyramid_gaussian_filtering ) {
      if ( BAL_SmoothImageIntoImage(  theInrimage_flo, &smoothed_flo,
                                      &(pyramid_level[l].sigma) ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to filter floating image at level %d\n", proc, l );
        if ( l > 0 ) BAL_FreeTransformation( &level_trsf );
        if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }
    }



    /* matching
     * recall that we have
     *
     * if ( param.pyramid_gaussian_filtering )
     *   smoothed_flo is allocated
     *   Inrimage_flo =  &smoothed_flo;
     * else
     *   Inrimage_flo = theInrimage_flo;
     *
     * if ( l > 0 )
     *   allocation of subsampled_ref
     *   subsampled_ref = theInrimage_ref o &subsampling_trsf,
     *   Inrimage_ref =  &subsampled_ref
     * else
     *   Inrimage_ref = theInrimage_ref
     *
     * if ( l > 0 )
     *   &level_trsf = resultTransformation o &subsampling_trsf
     *   transformation = &level_trsf;
     * else
     *   transformation = resultTransformation;
     *
     */
    
    if ( _debug_ >= 2 ) {
      fprintf( stderr, " === before matching ===\n" );
      if ( theLeftTransformation != (bal_transformation*)NULL )
        BAL_PrintTransformation( stderr, theLeftTransformation, "    left transformation" );
      else 
        fprintf( stderr, "    no initial transformation\n" );
      BAL_PrintTransformation( stderr, transformation, "    transformation avant recalage" );
      fprintf( stderr, " ======================\n" );
      fprintf( stderr, "\n" );
    }

    if ( BAL_BlockMatching( Inrimage_ref, Inrimage_flo, theLeftTransformation, transformation,
                            &(pyramid_level[l].param) ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: matching failed at level %d\n", proc, l );
      if ( l > 0 ) BAL_FreeTransformation( &level_trsf );
      if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
      BAL_FreeTransformation( &subsampling_trsf );
      BAL_FreeImage( &smoothed_flo );
      free( pyramid_level );
      if ( theInitResult == (bal_transformation*)NULL ) {
        BAL_FreeTransformation( theResultTransformation );
        free( theResultTransformation );
      }
      return( (bal_transformation*)NULL );
    }



    /* resampling of returned transformation at the original geometry
     * i.e. T_{subsampled-reference <- original-reference}
     */
    if ( l > 0 ) {
      if ( BAL_ComputeImageToImageTransformation( theInrimage_ref, &subsampled_ref, &subsampling_trsf ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute subsampling transformation at level %d (2)\n", proc, l );
        if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }

      if ( _debug_ >= 2 ) {
        BAL_PrintTransformation( stderr, &subsampling_trsf, "=== sur-echantillonage" );
      }

      /* theTr = level_trsf o subsampling_trsf
       */
      if ( BAL_TransformationComposition( resultTransformation, &level_trsf, &subsampling_trsf ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute transformation at level %d (2)\n", proc, l );
        if ( l > 0 ) BAL_FreeTransformation( &level_trsf );
        if ( l > 0 ) BAL_FreeImage( &subsampled_ref );
        BAL_FreeTransformation( &subsampling_trsf );
        BAL_FreeImage( &smoothed_flo );
        free( pyramid_level );
        if ( theInitResult == (bal_transformation*)NULL ) {
          BAL_FreeTransformation( theResultTransformation );
          free( theResultTransformation );
        }
        return( (bal_transformation*)NULL );
      }

      /* freeing some stuff
       */
      BAL_FreeTransformation( &level_trsf );
      BAL_FreeImage( &subsampled_ref );

    }
    
    if ( _debug_ >= 2 ) {
      fprintf( stderr, " === after matching ====\n" );
      BAL_PrintTransformation( stderr, resultTransformation, "    transformation apres recalage" );
      fprintf( stderr, " ======================\n" );
      fprintf( stderr, "\n" );
    }


  }


  BAL_FreeTransformation( &subsampling_trsf );
  BAL_FreeImage( &smoothed_flo );
  free( pyramid_level );
  return( resultTransformation );

}









/* transformation lineaire 
   - en coordonnees reelles
   
   transformation non-lineaire 
   - en coordonnees reelles, 
     le deplacement d'un point M(x,y,z) est donc
     M + V(x/vx, y/vy, z/vz)

   pour le calcul, on peut le passer en coordonnees voxel,
   en multipliant donc les vecteurs par H^(-1), mais il faut 
   penser :
   - a adapter le sigma de la gaussienne pour l'estimation de la transfo
   - a repasser en coordonnees reelles avant le retour
*/

int BAL_BlockMatching( bal_image *theInrimage_ref, /* reference image 
                                                      (may be subsampled) */
                       bal_image *theInrimage_flo, /* floating image
                                                      (in its original geometry,
                                                      may be filtered) */
                       bal_transformation *theLeft, /* initial transformation
                                                     */
                       bal_transformation *theTr, /* allows to goes from Iref to Iflo
                                                     (after initial transformation, if any)
                                                     ie to resample Iflo into Iref
                             that is T_{floating <- (subsampled) reference}
                                                  */

                       bal_blockmatching_param *param )
{
  char *proc="BAL_BlockMatching";
  
  bal_image Inrimage_flo_sub;
  FIELD field;
  BLOCS blocs_ref, blocs_flo;
  bal_transformation incTrsf;
  bal_transformation resamplingTrsf;
  bal_transformation *resTrsf = (bal_transformation*)NULL;
  

  int n_iteration;

  char image_name[NAMESIZE];
  
  /* double rms = RMS_ERROR; */
  int rms_stop = 0;
  double rms_threshold;
  imageCorners referenceCorners, previousCorners, currentCorners;
  int x, y, z;

  bal_doublePoint sigma =  param->elastic_regularization_sigma;

#ifndef WIN32
  double time_init;
  double time_exit;
  double clock_init;
  double clock_exit;
#endif

  /* some tests
   */

  switch ( param->estimator.type ) {
  default :
    if ( _verbose_ ) 
      fprintf( stderr, "%s: such estimator type not handled yet\n", proc );
    return( -1 );
  case TYPE_LS :
  case TYPE_LTS :
    break;
  case TYPE_WLS :
  case TYPE_WLTS :
    switch ( param->similarity_measure ) {
    default :
      break;
    case _SAD_ :
    case _SSD_ :
      if ( _verbose_ ) 
        fprintf( stderr, "%s: such measure is not compatible with weighted estimation\n", proc );
      return( -1 );
    }
  }



  /* propagate log file
   */
  BAL_SetVerboseFileInBalBlock( param->verbosef );
  BAL_SetVerboseFileInBalFieldTools( param->verbosef );


  /* Allocation: subsampled floating image
     this image is in the same geometry than theInrimage_ref
  */
  BAL_InitImage( &Inrimage_flo_sub, NULL, 0, 0, 0, 0, theInrimage_flo->type );
  if ( BAL_InitAllocImage( &Inrimage_flo_sub, "subsampled_floating_image.inr.gz", 
                           theInrimage_ref->ncols, theInrimage_ref->nrows,
                           theInrimage_ref->nplanes, theInrimage_flo->vdim,
                           theInrimage_flo->type ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate subsampled floating image\n", proc );
    return( -1 );
  }
  Inrimage_flo_sub.vx = theInrimage_ref->vx;
  Inrimage_flo_sub.vy = theInrimage_ref->vy;
  Inrimage_flo_sub.vz = theInrimage_ref->vz;


  
  /* Allocation: blocks and pairing field
     - the block sizes are the same for both images  -> param->bl_d[x,y,z]
     - the spacing between blocks in the *floating* image is given by 
     param->bl_next_[x,y,z]
     - the spacing between blocks in the *reference* image is 1

     For non-linear transformation, the block dimensions have to be odd
     so that the center of a block is a voxel.
     
  */
  switch ( theTr->type ) {
  default :
    break;
  case VECTORFIELD_2D :
  case VECTORFIELD_3D :
    if ( param->block_dim.x % 2 != 1 ||  param->block_dim.y % 2 != 1  || param->block_dim.y % 2 != 1 ) {
      fprintf( stderr, "%s: block dimensions have to be odd\n", proc );
      fprintf( stderr, "\t (dimensions were [%d,%d,%d])\n", param->block_dim.x, param->block_dim.y, param->block_dim.z );
        BAL_FreeImage( &Inrimage_flo_sub );
      return( -1 );    
    }
  }
  

  
  /* Allocation of blocks and field
   */
  if ( BAL_AllocateBlocksAndField( &field, &blocs_flo, &blocs_ref, 
                                   &Inrimage_flo_sub, theInrimage_ref,
                                   param ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate field and blocks\n", proc );
    BAL_FreeImage( & Inrimage_flo_sub );
    return( -1 );
  } 
  field.vx = theInrimage_ref->vx;
  field.vy = theInrimage_ref->vy;
  field.vz = theInrimage_ref->vz;
  


  /* allocation :
   * incremental transformation
   * its type is the one of the searched transformation
  */
  BAL_InitTransformation( &incTrsf );
  if ( BAL_AllocTransformation( &incTrsf, theTr->type, theInrimage_ref ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate incremental transformation\n", proc );
    BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
    BAL_FreeImage( & Inrimage_flo_sub );
    return( -1 );
  }
  incTrsf.transformation_unit = theTr->transformation_unit;



  /* pre-computation:
     Compute attributes of the reference blocks
  */
  if ( _trace_ )
    fprintf( stderr, "%s: computation of reference image block attributes\n", proc );
#ifndef WIN32
  time_init = _GetTime();
  clock_init = _GetClock();
#endif
  if ( BAL_ComputeBlockAttributes( theInrimage_ref, &blocs_ref ) == RETURNED_VALUE_ON_ERROR ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to pre-compute reference blocks attributes\n", proc );
    BAL_FreeTransformation( &incTrsf );
    BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
    BAL_FreeImage( & Inrimage_flo_sub );
    return( -1 );
  }
  if (param->verbosef != NULL) {
    fprintf( param->verbosef, "Nombre de blocs_ref actifs\t=\t%lu\n", blocs_ref.n_valid_blocks );
  }
  if ( _debug_ ) {
    fprintf( stderr, "%s: %lu valid reference blocks out of %lu\n",
             proc, blocs_ref.n_valid_blocks, blocs_ref.n_allocated_blocks );
    fprintf( stderr, "\t block dimensions are %lu x %lu x %lu\n",
             blocs_ref.blocksarraydim.x,
             blocs_ref.blocksarraydim.y,
             blocs_ref.blocksarraydim.z );
    fprintf( stderr, "\t block sizes are %d x %d x %d\n",
             blocs_ref.blockdim.x,
             blocs_ref.blockdim.y,
             blocs_ref.blockdim.z );
  }
#ifndef WIN32
  time_exit = _GetTime();
  clock_exit = _GetClock();
  if ( _time_ )
    _PrintTime( stderr, "reference block attributes", time_init, clock_init, time_exit, clock_exit );
#endif

  /* obsolete (?) :
     Write on disk the reference image with the active voxels only
     they are defined as the voxels included in the active blocks
     ie the blocks with the 'active' flag set to 1
     Thus, all blocks are considered
  */
  if ( param->write_def == 1 || param->vischeck == 1 ) {
    sprintf( image_name, "blocs_actifs_ref_py_%d.inr.gz", param->pyramid_level );
    WriteImageVoxelsActifs( blocs_ref.n_allocated_blocks, &blocs_ref, theInrimage_ref, image_name );
  }




  /* end condition :
     computation of a threshold for RMS, ie end condition
     (stand for linear transformations, not for non-linear)
  */
  if ( param->rms_ending_condition ) {
    for ( z=0; z<2; z++ )
    for ( y=0; y<2; y++ )
    for ( x=0; x<2; x++ ) {
      referenceCorners.corner[z][y][x].x = ( x == 0 ) ? 0 :  (theInrimage_ref->ncols-1)*theInrimage_ref->vx;
      referenceCorners.corner[z][y][x].y = ( y == 0 ) ? 0 :  (theInrimage_ref->nrows-1)*theInrimage_ref->vy;
      referenceCorners.corner[z][y][x].z = ( z == 0 ) ? 0 :  (theInrimage_ref->nplanes-1)*theInrimage_ref->vz;
    }
    if ( _TransformCorners( &referenceCorners, &previousCorners, theTr ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to transform reference corners (#1)\n", proc );
      BAL_FreeTransformation( &incTrsf );
      BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
      BAL_FreeImage( & Inrimage_flo_sub );
      return( -1 );
    }
  }
  rms_threshold = theInrimage_ref->vx / 3.0;
  if ( rms_threshold > theInrimage_ref->vy / 3.0 ) 
    rms_threshold = theInrimage_ref->vy / 3.0;
#ifdef _ORIGINAL_BALADIN_TWO_DIMENSIONS_
  if ( rms_threshold > theInrimage_ref->vz / 3.0 ) 
    rms_threshold = theInrimage_ref->vz / 3.0;
#else
  if ( theInrimage_ref->nplanes > 1 && rms_threshold > theInrimage_ref->vz / 3.0 ) 
    rms_threshold = theInrimage_ref->vz / 3.0;
#endif







  /************************************************************
   registration loop
  ************************************************************/
  for ( n_iteration = 0, rms_stop = 0;
        n_iteration < param->max_iterations && rms_stop != 1;
        n_iteration ++ ) {
    
    if ( param->verbosef != NULL ) {
      fprintf(param->verbosef,"\n" );
      fprintf(param->verbosef,"*********************************************************************\n"); 
      fprintf(param->verbosef,"                    Iteration #%3d       Level #%2d      Size %4lux%4lux%4lu\n", 
              n_iteration+1, param->pyramid_level,
              theInrimage_ref->ncols, theInrimage_ref->nrows, theInrimage_ref->nplanes );
      if ( theLeft != (bal_transformation*)NULL ) {
        BAL_PrintTransformation( param->verbosef, theLeft, "    left-handed transformation" );
      }
      BAL_PrintTransformation( param->verbosef, theTr, "    transformation at beg. of iteration" );
    }
    
    if ( _verbose_ ) {
      fprintf(stderr,"    - Iteration #%3d     Level #%2d     Size %4lux%4lux%4lu",
              n_iteration+1, param->pyramid_level,
              theInrimage_ref->ncols, theInrimage_ref->nrows, theInrimage_ref->nplanes );
      if ( _verbose_ == 1 ) {
    if ( ! _time_ )
          fprintf(stderr, "\r" );
        else
          fprintf(stderr, "\n" );
      }
      else fprintf(stderr, "\n" );
    }


    if ( _debug_ >= 2 ) {
       BAL_PrintTransformation( stderr, theTr, "Transformation at the beginning of the loop" );
    }

    
    
    /*****************************************
       Resampling of the floating image
       - if there is a initialization transformation
         one has to resample with 'init o theTr'
     */

    BAL_InitTransformation( &resamplingTrsf );
    if ( theLeft != (bal_transformation*)NULL ) {

      if ( _time_ )
        fprintf( stderr, "%s: transformation composition with the initial transformation\n", proc );
      
#ifndef WIN32
      time_init = _GetTime();
      clock_init = _GetClock();
#endif

      if ( BAL_AllocTransformationComposition( &resamplingTrsf, theLeft, theTr, theInrimage_ref ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: can not allocate composition transformation (resampling step)\n", proc );
        BAL_FreeTransformation( &incTrsf );
        BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
        BAL_FreeImage( & Inrimage_flo_sub );
        return( -1 );
      }
      if ( BAL_TransformationComposition( &resamplingTrsf, theLeft, theTr ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: can not compute composition transformation (resampling step)\n", proc );
        BAL_FreeTransformation( &resamplingTrsf );
        BAL_FreeTransformation( &incTrsf );
        BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
        BAL_FreeImage( & Inrimage_flo_sub );
        return( -1 );
      }
      resTrsf = &resamplingTrsf;

#ifndef WIN32      
      time_exit = _GetTime();
      clock_exit = _GetClock();
      if ( _time_ )
        _PrintTime( stderr, "initial transformation composition", time_init, clock_init, time_exit, clock_exit );
#endif
    }
    else {
      resTrsf = theTr;
    }
    
    

    if ( _time_ )
      fprintf( stderr, "%s: image resampling\n", proc );

#ifndef WIN32
    time_init = _GetTime();
    clock_init = _GetClock();
#endif
    
    if ( BAL_ResampleImage( theInrimage_flo, &Inrimage_flo_sub, resTrsf, LINEAR ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to resample floating image at iteration #%d\n", proc, n_iteration+1 );
      if ( theLeft != (bal_transformation*)NULL ) BAL_FreeTransformation( &resamplingTrsf );
      BAL_FreeTransformation( &incTrsf );
      BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
      BAL_FreeImage( & Inrimage_flo_sub );
      return( -1 );
    }

#ifndef WIN32
    time_exit = _GetTime();
    clock_exit = _GetClock();
    if ( _time_ )
      _PrintTime( stderr, "image resampling", time_init, clock_init, time_exit, clock_exit );
#endif

    /* desallocation of the resanpling transformation, if any
     */
    if ( theLeft != (bal_transformation*)NULL ) BAL_FreeTransformation( &resamplingTrsf );
    resTrsf = (bal_transformation*)NULL;

    if ( _BALADIN_WRITE_FLOATING_IMAGE_EVOLUTION_ ) {
      sprintf( image_name, "floating_image_level%02d_iteration%02d.hdr", param->pyramid_level, n_iteration );
      if ( BAL_WriteImage( &Inrimage_flo_sub, image_name ) != 1 ) {
        fprintf( stderr, "%s: unable to write floating image at level %d and iteration %d\n",
                 proc, param->pyramid_level, n_iteration );
      }
    }



    /*****************************************
       Compute attributes of the floating blocks
    */
    
    if ( _time_ )
      fprintf( stderr, "%s: computation of floating image block attributes\n", proc );
#ifndef WIN32
    time_init = _GetTime();
    clock_init = _GetClock();
#endif

    if ( BAL_ComputeBlockAttributes( &Inrimage_flo_sub, &blocs_flo ) == RETURNED_VALUE_ON_ERROR ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to pre-compute floating blocks attributes\n", proc );
      BAL_FreeTransformation( &incTrsf );
      BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
      BAL_FreeImage( & Inrimage_flo_sub );
    }
    if (param->verbosef != NULL) {
      fprintf(param->verbosef, "\nNombre de blocs_flo\t=\t%lu\t, actifs apres seuils\t=\t%lu\n",
              blocs_flo.n_allocated_blocks, blocs_flo.n_valid_blocks);
    }
#ifndef WIN32
    time_exit = _GetTime();
    clock_exit = _GetClock();
    if ( _time_ )
      _PrintTime( stderr, "floating block attributes", time_init, clock_init, time_exit, clock_exit );
#endif

   

   /*****************************************
     Tri des blocs_flo selon la variance blocs_flo.pointer[i]->variance 
      ce serait la qu'il faudrait recuperer les "bons" blocks,
      ie echantillonner d'apres les parametres.
   */ 

    if ( _time_ )
      fprintf( stderr, "%s: block sorting\n", proc );

    BAL_SortBlocks( &blocs_flo );
    
    /* pourcent = bl_pourcent_var * blocs_flo.n_valid_blocks 
       par exemple 40 % des blocs actifs, i.e. avec une variance non nulle */
    blocs_flo.n_valid_blocks = (int) (blocs_flo.n_valid_blocks * param->blocks_fraction);
    if ( param->verbosef != NULL) {
      fprintf( param->verbosef, "Nombre de blocs_flo avec les plus fortes variances : %lu\n", 
               blocs_flo.n_valid_blocks);
    }

    

   /*****************************************
     Write on disk the reference image with the active voxels only
       they are defined as the voxels included in the active blocks
       Here (please note that it is different from the reference image)
       the active blocks are the "most significant" blocks, ie the
       blocks with the largest variance and are a subset of the blocks
       with the 'active' flag set to 1
    */
    if ( param->write_def == 1 || param->vischeck == 1 ) {
      sprintf( image_name, "floating_blocks_level%02d_iteration%02d.hdr", param->pyramid_level, n_iteration );
      WriteImageVoxelsActifs( blocs_flo.n_valid_blocks, &blocs_flo, &Inrimage_flo_sub, image_name );
    }



    /*****************************************
     * Compute the displacement field = list of pairs (origin, displacement)
     *
     * origin = center of reference block
     * displacement = floating point - reference point
     */

    if ( _time_ )
      fprintf( stderr, "%s: computation of pairing field\n", proc );
#ifndef WIN32
        time_init = _GetTime();
    clock_init = _GetClock();
#endif
    field.n_computed_pairs = field.n_selected_pairs = 0;
    if ( BAL_ComputePairingFieldFromRefToFlo( &field,
                                         &Inrimage_flo_sub, &blocs_flo,
                                         theInrimage_ref, &blocs_ref,
                                         &(param->half_neighborhood_size),
                                         &(param->step_neighborhood_search),
                                         param->similarity_measure,
                                         param->similarity_measure_threshold) == RETURNED_VALUE_ON_ERROR ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: error when computing displacement field from flo to ref\n", proc );
      BAL_FreeTransformation( &incTrsf );
      BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
      BAL_FreeImage( & Inrimage_flo_sub );
      return( -1 );      
    }
#ifndef WIN32    
    time_exit = _GetTime();
    clock_exit = _GetClock();
    if ( _time_ )
      _PrintTime( stderr, "pairing field", time_init, clock_init, time_exit, clock_exit );
#endif

    if (param->verbosef != NULL) {
      fprintf(param->verbosef, "Nombre de voxels apparies : %lu\n", field.n_computed_pairs ); 
    }
    if ( field.n_computed_pairs <= 0 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: empty displacement field\n", proc );
      BAL_FreeTransformation( &incTrsf );
      BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
      BAL_FreeImage( & Inrimage_flo_sub );
      return( -1 );      
    }



    /*****************************************
      Compute the incremental transformation
       Linear case: it is in the real framework
    */

    if ( _time_ )
      fprintf( stderr, "%s: computation of incremental transformation\n", proc );

#ifndef WIN32
        time_init = _GetTime();
    clock_init = _GetClock();
#endif

    if ( BAL_ComputeIncrementalTransformation( &incTrsf, &field, &(param->estimator) ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to compute incremental transformation\n", proc );
      BAL_FreeTransformation( &incTrsf );
      BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
      BAL_FreeImage( & Inrimage_flo_sub );
      return( -1 ); 
    }

#ifndef WIN32    
    time_exit = _GetTime();
    clock_exit = _GetClock();
    if ( _time_ )
      _PrintTime( stderr, "pairing field", time_init, clock_init, time_exit, clock_exit );
#endif

    if ( _debug_ >= 2 ) {
      fprintf( stderr, "\n" );
      BAL_PrintTransformation( stderr, &incTrsf, "Computed incremental transformation" );
      fprintf( stderr, "\n" );
    }

    

    /*****************************************
       Update the  transformation
       Linear case: it is in the real framework
       theTr = incTrsf o theTr
    */
    
    if ( _time_ )
      fprintf( stderr, "%s: transformation composition with the incremental transformation\n", proc );
#ifndef WIN32
        time_init = _GetTime();
    clock_init = _GetClock();
#endif

    /* we have
     * BAL_TransformationComposition( bal_transformation *t_res,
     *                                bal_transformation *t1,
     *                                bal_transformation *t2 )
     * ie t_res = t1 o t2
     *
     * historical version:
     * if ( BAL_TransformationComposition( theTr, &incTrsf, theTr ) != 1 ) {
     *   ...
     * }
     * seems weird, since
     * - theTr = T^(i)_{floating^(0) <- reference}
     * - floating^(i) = floating^(0) o theTr
     *                = floating^(0) o T^(i)_{floating^(0) <- reference}
     * - incTrsf = T^(i)_{floating^(i) <- reference}
     * we should then have
     * floating^(i+1) = floating^(i) o T^(i)_{floating^(i) <- reference}
     *                = floating^(0) o T^(i)_{floating^(0) <- reference} o T^(i)_{floating^(i) <- reference}
     * ie. T^(i)_{floating^(i+1) <- reference} = T^(i)_{floating^(0) <- reference} o T^(i)_{floating^(i) <- reference}
     * hence the update rule should be
     *     theTr = theTr o &incTrsf
     * hence
     * if ( BAL_TransformationComposition( theTr, theTr, &incTrsf ) != 1 ) {
     *   ...
     * }
     */

    if ( BAL_TransformationComposition( theTr, theTr, &incTrsf ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute updated transformation\n", proc );
      BAL_FreeTransformation( &incTrsf );
      BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
      BAL_FreeImage( & Inrimage_flo_sub );
      return( -1 );
    }


#ifndef WIN32    
    time_exit = _GetTime();
    clock_exit = _GetClock();
    if ( _time_ )
      _PrintTime( stderr, "pairing field", time_init, clock_init, time_exit, clock_exit );
#endif
    if ( _debug_ >= 2 ) {
      fprintf( stderr, "\n" );
      BAL_PrintTransformation( stderr, theTr, "Next transformation = Incremental o Previous" );
      fprintf( stderr, "\n" );
    }

    /* elastic smoothing
     */
    if ( theTr->type == VECTORFIELD_2D ) sigma.z = -1.0;
    switch ( theTr->type ) {
    default :
      break;
    case VECTORFIELD_2D :
    case VECTORFIELD_3D :
      if ( _time_ )
        fprintf( stderr, "%s: elastic regularization\n", proc );
#ifndef WIN32
      time_init = _GetTime();
      clock_init = _GetClock();
#endif

      if ( BAL_SmoothVectorField( theTr, &sigma ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to smooth vectorfield\n", proc );
        BAL_FreeTransformation( &incTrsf );
        BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
        BAL_FreeImage( & Inrimage_flo_sub );
        return( -1 );
      }
#ifndef WIN32    
    time_exit = _GetTime();
    clock_exit = _GetClock();
    if ( _time_ )
      _PrintTime( stderr, "pairing field", time_init, clock_init, time_exit, clock_exit );
#endif
    }
    


    /* End condition:
       keeping both the previous and the current transformations will cost
       some memory for the non-linear ones: do we have to?
     */
    if ( param->rms_ending_condition ) {
      if ( _TransformCorners( &referenceCorners, &currentCorners, theTr ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to transform reference corners (#2)\n", proc );
        BAL_FreeTransformation( &incTrsf );
        BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
        BAL_FreeImage( & Inrimage_flo_sub );
        return( -1 );
      }
      if ( _computeRMS( &previousCorners, &currentCorners ) <= rms_threshold )
        rms_stop = 1;
      previousCorners = currentCorners;
    }

    if ( param->verbosef != NULL ) {
      BAL_PrintTransformation( param->verbosef, theTr, "    transformation at end of iteration" );
      fprintf(param->verbosef,"*********************************************************************\n");
    }


  } /* end of registration loop */



  /* special case for verbose
   */
  if ( _verbose_ == 1 ) fprintf(stderr,"\n" );


  BAL_FreeTransformation( &incTrsf );
  BAL_FreeBlocksAndField( &field, &blocs_flo, &blocs_ref );
  BAL_FreeImage( &Inrimage_flo_sub );

  return( 1 );
}












/*--------------------------------------------------*
 *
 * parameter checking
 *
 *--------------------------------------------------*/

/* this is an ad hoc procedure
 * to change the type of a transformation
 */
static int BAL_ChangeTransformationType( bal_transformation *t, enumTypeTransfo type, bal_image *ref )
{
  char *proc = "BAL_ChangeTransformationType";
  bal_transformation tmpTrsf;


#define _CHANGETRANSFORMATIONTYPE {                                                  \
  if ( BAL_AllocTransformation( &tmpTrsf, t->type, ref ) != 1 ) {                    \
    if ( _verbose_ )                                                                 \
        fprintf( stderr, "%s: can not allocate auxiliary transformation\n", proc );  \
    return( -1 );                                                                    \
  }                                                                                  \
  if ( BAL_CopyTransformation( t, &tmpTrsf ) != 1 ) {                                \
    BAL_FreeTransformation( &tmpTrsf );                                              \
    if ( _verbose_ )                                                                 \
        fprintf( stderr, "%s: can not copy into auxiliary transformation\n", proc ); \
    return( -1 );                                                                    \
  }                                                                                  \
  BAL_FreeTransformation( t );                                                       \
  if ( BAL_AllocTransformation( t, type, ref ) != 1 ) {                              \
    BAL_FreeTransformation( &tmpTrsf );                                              \
    if ( _verbose_ )                                                                 \
        fprintf( stderr, "%s: can not allocate result transformation\n", proc );     \
    return( -1 );                                                                    \
  }                                                                                  \
  if ( BAL_CopyTransformation( &tmpTrsf, t  ) != 1 ) {                               \
    BAL_FreeTransformation( t );                                                     \
    BAL_FreeTransformation( &tmpTrsf );                                              \
    if ( _verbose_ )                                                                 \
        fprintf( stderr, "%s: can not copy into result transformation\n", proc );    \
    return( -1 );                                                                    \
  }                                                                                  \
  BAL_FreeTransformation( &tmpTrsf );                                                \
}

  BAL_InitTransformation( &tmpTrsf );

  switch( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such new transformation type not handled yet\n", proc );
    return( -1 );

  case TRANSLATION_2D :
  case TRANSLATION_3D :
  case TRANSLATION_SCALING_2D :
  case TRANSLATION_SCALING_3D :
  case RIGID_2D :
  case RIGID_3D :
  case SIMILITUDE_2D :
  case SIMILITUDE_3D :
  case AFFINE_2D :
  case AFFINE_3D :

    switch( t->type ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such transformation type not handled yet\n", proc );
      return( -1 );

    case TRANSLATION_2D :
    case TRANSLATION_3D :
    case TRANSLATION_SCALING_2D :
    case TRANSLATION_SCALING_3D :
    case RIGID_2D :
    case RIGID_3D :
    case SIMILITUDE_2D :
    case SIMILITUDE_3D :
    case AFFINE_2D :
    case AFFINE_3D :
      /* linear -> linear
       */
      t->type = type;
      break;
    case VECTORFIELD_2D :
    case VECTORFIELD_3D :
      /* non-linear -> linear
       */
      if ( _verbose_ )
        fprintf( stderr, "%s: change from non-linear type to linear one not handled\n", proc );
      return( -1 );
    }
    break;

  case VECTORFIELD_2D :

    switch( t->type ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such transformation type not handled yet\n", proc );
      return( -1 );

    case TRANSLATION_3D :
    case TRANSLATION_SCALING_3D :
    case RIGID_3D :
    case SIMILITUDE_3D :
    case AFFINE_3D :
      /* 3D linear -> 2D vector field
       */
      /* is it a 3D linear transformation matrix ?
       * test off-diagonal terms of the rotation part
       * then diagonal term
       * then translation term
       */
      if ( fabs(t->mat.m[2]) > 0.0001 ||  fabs(t->mat.m[6]) > 0.0001
           || fabs(t->mat.m[8]) > 0.0001 || fabs(t->mat.m[9]) > 0.0001
           || fabs(t->mat.m[10]-1.0) > 0.0001
           || fabs(t->mat.m[11]) > 0.0001 ) {
        if ( _verbose_ ) {
          fprintf( stderr, "%s: warning, transformation from 3D linear matrix\n", proc );
          fprintf( stderr, "\t into 2D vector field may cause troubles\n" );
        }
        /* should we return one error here?
         */
      }

    case TRANSLATION_2D :
    case TRANSLATION_SCALING_2D :
    case RIGID_2D :
    case SIMILITUDE_2D :
    case AFFINE_2D :
      /* 2D linear -> 2D vector field
       */
      _CHANGETRANSFORMATIONTYPE;
      break;

    case VECTORFIELD_2D :
      break;

    case VECTORFIELD_3D :
      /* 3D non-linear -> 2D non-linear
       */
      if ( _verbose_ ) {
        fprintf( stderr, "%s: warning, transformation from 3D vector field\n", proc );
        fprintf( stderr, "\t into 2D vector field is not handled\n" );
      }
      return( -1 );
    }
    break;

  case VECTORFIELD_3D :

    switch( t->type ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such transformation type not handled yet\n", proc );
      return( -1 );

    case TRANSLATION_2D :
    case TRANSLATION_SCALING_2D :
    case RIGID_2D :
    case SIMILITUDE_2D :
    case AFFINE_2D :
    case TRANSLATION_3D :
    case TRANSLATION_SCALING_3D :
    case RIGID_3D :
    case SIMILITUDE_3D :
    case AFFINE_3D :
    case VECTORFIELD_2D :
    case VECTORFIELD_3D :
      _CHANGETRANSFORMATIONTYPE;
      break;
    }
    break;

  }

  return( 1 );
}






/*--------------------------------------------------*
 *
 * RMS
 *
 *--------------------------------------------------*/

static int _TransformCorners( imageCorners *f, imageCorners *t, bal_transformation *theTr )
{
  char *proc = "_TransformCorners";
  int x, y, z;
  
  _MATRIX V;
  _MATRIX R;

  switch ( theTr->type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation type not handled yet\n", proc );
    return( -1 );

  case TRANSLATION_2D :
  case TRANSLATION_3D :
  case TRANSLATION_SCALING_2D :
  case TRANSLATION_SCALING_3D :
  case RIGID_2D :
  case RIGID_3D :
  case SIMILITUDE_2D :
  case SIMILITUDE_3D :
  case AFFINE_2D :
  case AFFINE_3D :
    
    if ( _alloc_mat( &V, 4, 1) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate vector #1\n", proc );
      return( -1 );
    }
    
    if ( _alloc_mat( &R, 4, 1) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate vector #2\n", proc );
      _free_mat( &V );
      return( -1 );
    }

    for ( z=0; z<2; z++ )
    for ( y=0; y<2; y++ )
    for ( x=0; x<2; x++ ) {
      V.m[0] = f->corner[z][y][x].x;    
      V.m[1] = f->corner[z][y][x].y;    
      V.m[2] = f->corner[z][y][x].z;    
      V.m[3] = 1.0;   
      _mult_mat( &(theTr->mat), &V, &R );
      t->corner[z][y][x].x = R.m[0];
      t->corner[z][y][x].y = R.m[1];
      t->corner[z][y][x].z = R.m[2];
    }

    _free_mat( &V );
    _free_mat( &R );

    break;
    
  }
  return( 1 );
}



static double _computeRMS( imageCorners *c1, imageCorners *c2 )
{
  int x, y, z;
  double d = 0;

  for ( z=0; z<2; z++ )
  for ( y=0; y<2; y++ )
  for ( x=0; x<2; x++ ) {
    d += (c1->corner[z][y][x].x - c2->corner[z][y][x].x)*(c1->corner[z][y][x].x - c2->corner[z][y][x].x)
      + (c1->corner[z][y][x].x - c2->corner[z][y][x].y)*(c1->corner[z][y][x].y - c2->corner[z][y][x].y)
      + (c1->corner[z][y][x].x - c2->corner[z][y][x].z)*(c1->corner[z][y][x].z - c2->corner[z][y][x].z);
  }
  return( sqrt( d / 8.0 ) );
}







/*--------------------------------------------------*
 *
 * BLOCKS and FIELDS management
 *
 *--------------------------------------------------*/






static int BAL_AllocateBlocksAndField( FIELD *field, 
                                         BLOCS *blocs_flo,
                                         BLOCS *blocs_ref,
                                         bal_image *image_flo,
                                         bal_image *image_ref,
                                         bal_blockmatching_param *param )
{
  char * proc = "BAL_AllocateBlocksAndField";
  bal_sizePoint imagedim;
  bal_integerPoint oneSpacing;

  oneSpacing.x = 1;
  oneSpacing.y = 1;
  oneSpacing.z = 1;


  imagedim.x = image_flo->ncols;
  imagedim.y = image_flo->nrows;
  imagedim.z = image_flo->nplanes;

  if ( BAL_AllocateBlocks( blocs_flo, &imagedim, &(param->block_dim), &(param->block_spacing) ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate floating blocks\n", proc );
    return( -1 );
  }
  
  /* borders for statistics computation
   */
  blocs_flo->border.x = param->block_border.x;
  blocs_flo->border.y = param->block_border.y;
  blocs_flo->border.z = param->block_border.z;
  
  if ( blocs_flo->border.x < 0 ) blocs_flo->border.x = 0;
  if ( blocs_flo->border.y < 0 ) blocs_flo->border.y = 0;
  if ( blocs_flo->border.z < 0 ) blocs_flo->border.z = 0;

  /* selection criteria 
     - thresholds
     - percentage
  */
  blocs_flo->selection = param->floating_selection;



  /* the step between two successive blocks is implicitly 1
     for the reference blocks
  */
  imagedim.x = image_ref->ncols;
  imagedim.y = image_ref->nrows;
  imagedim.z = image_ref->nplanes;
  if ( BAL_AllocateBlocks( blocs_ref, &imagedim, &(param->block_dim), &oneSpacing ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate reference blocks\n", proc );
    BAL_FreeBlocks( blocs_flo );
    return( -1 );
  }

  /* borders for statistics computation
   */
  blocs_flo->border.x = param->block_border.x;
  blocs_flo->border.y = param->block_border.y;
  blocs_flo->border.z = param->block_border.z;
  
  if ( blocs_ref->border.x < 0 ) blocs_ref->border.x = 0;
  if ( blocs_ref->border.y < 0 ) blocs_ref->border.y = 0;
  if ( blocs_ref->border.z < 0 ) blocs_ref->border.z = 0;

  /* selection criteria 
     - thresholds
     - percentage
  */
  blocs_ref->selection = param->reference_selection;
  


  if ( BAL_AllocateField( field, blocs_flo->n_allocated_blocks ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate field\n", proc );
    BAL_FreeBlocks( blocs_ref );
    BAL_FreeBlocks( blocs_flo );
    return( -1 );
  }
  
  if (param->verbosef != NULL) {
    fprintf(param->verbosef, "Blocs ref\t=\t%lu\nBlocs flo\t=\t%lu\n", 
            blocs_ref->n_allocated_blocks, blocs_flo->n_allocated_blocks );
  }
  
  return( 1 );
}



static void BAL_FreeBlocksAndField( FIELD *field, BLOCS *blocs_flo, BLOCS *blocs_ref )
{
  BAL_FreeField( field );
  BAL_FreeBlocks( blocs_ref );
  BAL_FreeBlocks( blocs_flo );
}











/*--------------------------------------------------*
 *
 * MISC
 *
 *--------------------------------------------------*/


/***
    Ecrit l'image contenant les voxels actifs lors du recalage
***/
static void  WriteImageVoxelsActifs( size_t n_allocated_blocks, BLOCS *blocs, 
                                     bal_image *Inrimage_sub, char *nom_image )
{
  bal_image Inrimage_tmp;
  size_t i;
  int a, b, c, u, v, w;
  int seuil_bas = blocs->selection.low_threshold;
  int seuil_haut = blocs->selection.high_threshold;


  if ( BAL_InitAllocImage( &Inrimage_tmp, NULL, 
                           Inrimage_sub->ncols, Inrimage_sub->nrows,
                           Inrimage_sub->nplanes, Inrimage_sub->vdim,
                           Inrimage_sub->type) != 1 )
    return;
  
  if ( 0 ) {
    fprintf( stderr, "WriteImageVoxelsActifs: image = '%s'\n", nom_image );
    fprintf( stderr, "\t seuils = %d %d\n", seuil_bas, seuil_haut );
    fprintf( stderr, "\t nb blocs a ecrire = %lu\n", n_allocated_blocks );
  }
  
#define _WRITE2D( TYPE ) {                                  \
  for (i=0; i<n_allocated_blocks; i++)                              \
    if ( blocs->pointer[i]->valid == 1) {                  \
      a = blocs->pointer[i]->origin.x; b = blocs->pointer[i]->origin.y;   \
      for (u=0; u<blocs->blockdim.x; u++)               \
      for (v=0; v<blocs->blockdim.y; v++) {             \
        j = ((TYPE***)Inrimage_sub->array)[0][b+v][a+u];  \
        if ( j > seuil_bas && j < seuil_haut )            \
          ((TYPE***)Inrimage_tmp.array)[0][b+v][a+u] = j; \
      }                                                   \
    }                                                     \
}

#define _WRITE3D( TYPE ) {                                \
  for (i=0; i<n_allocated_blocks; i++)                              \
    if ( blocs->pointer[i]->valid == 1) {                  \
      a = blocs->pointer[i]->origin.x; b = blocs->pointer[i]->origin.y; c = blocs->pointer[i]->origin.z; \
      for (u=0; u<blocs->blockdim.x; u++)               \
      for (v=0; v<blocs->blockdim.y; v++)               \
      for (w=0; w<blocs->blockdim.z; w++) {             \
        j = ((TYPE***)Inrimage_sub->array)[c+w][b+v][a+u]; \
        if ( j > seuil_bas && j < seuil_haut )            \
          ((TYPE***)Inrimage_tmp.array)[c+w][b+v][a+u] = j; \
      }                                                   \
    }                                                     \
}

#define _WRITE( TYPE ) {  \
  TYPE j;                 \
  if ( Inrimage_sub->nplanes == 1 ) {  \
    _WRITE2D( TYPE )      \
  }                       \
  else {                  \
    _WRITE3D( TYPE )      \
  }                       \
}

  switch( Inrimage_sub->type ) {
  default :
    fprintf( stderr, "WriteImageVoxelsActifs: such image type not handled yet\n" );
    BAL_FreeImage( &Inrimage_tmp );
    return;
    break;
  case UCHAR :
    _WRITE( unsigned char )
      break;
  case USHORT :
    _WRITE( unsigned short int )
      break;
  case SSHORT :
    _WRITE( short int )
      break;
  }
  
  Inrimage_tmp.vx = Inrimage_sub->vx;
  Inrimage_tmp.vy = Inrimage_sub->vy;
  Inrimage_tmp.vz = Inrimage_sub->vz;
  
  BAL_WriteImage( &Inrimage_tmp, nom_image);
  BAL_FreeImage( &Inrimage_tmp );
  
  return;
}










