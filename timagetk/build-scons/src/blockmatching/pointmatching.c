/*************************************************************************
 * pointTrsf.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mar 24 sep 2013 03:15:35 CEST
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
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <chunks.h>

#include <bal-estimator.h>
#include <bal-field.h>
#include <bal-field-tools.h>
#include <bal-point.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>
#include <bal-vectorfield.h>


/*
  #define max(a,b) ((a)>(b) ? (a) : (b))
  #define min(a,b) ((a)<(b) ? (a) : (b))
*/


/* 
static int _debug_ = 0;
*/

/*------- Definition des fonctions statiques ----------*/

static char *program = NULL;

/*---------------- longueur maximale de ligne --------------------------------*/
/*----------------------------------------------------------------------------*/
static char *usage = "-reference %s -floating %s -result-transformation %s\n\
 [-units voxel|real]\n\
 [-result-transformation|-res-trsf %s]\n\
 [-result-voxel-transformation|-res-voxel-trsf %s] [-residuals %s]\n\
 [-transformation-type|-transformation|-trsf-type %s]\n\
 [-dim %d %d [%d] | [-x %d] [-y %d] [-z %d]] [-template %s]\n\
 [-voxel | -pixel | -vs %f %f [%f] | [-vx %f] [-vy %f] [-vz %f] ]\n\
 [-estimator-type|-estimator|-es-type wlts|lts|wls|ls]\n\
 [-lts-fraction %lf] [-lts-deviation %f] [-lts-iterations %d]\n\
 [-fluid-sigma|-lts-sigma[-ll|-hl] %lf %lf %lf]\n\
 [-vector-propagation-type|-propagation-type|-propagation direct|skiz]\n\
 [-vector-propagation-distance|-propagation-distance|-pdistance %f]\n\
 [-vector-fading-distance|-fading-distance|-fdistance %f]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-command-line %s] [-logfile %s]\n\
 [-verbose|-v] [-no-verbose|-nv] [-time] [-notime]\n\
 [-debug|-no-debug]\n\
 [-h|-help|--h|--help]\n";

static char *detail = "\
###\n\
  Computes a transformation with the paired points from the reference to the\n\
  floating. This is somehow the 'blockmatching' way, the resulting\n\
  transformation will allow to resample the floating image (from which the\n\
  floating points are drawn) onto the reference one.\n\
### File names ###\n\
-reference|-ref %s  # name of the reference point list (still points)\n\
-floating|-flo %s   # name of the point list to be registered (floating points)\n\
   points are assumed to be paired according to their rank\n\
[-units voxel|real] # units of the points (default is REAL)\n\
[-result-transformation|-res-trsf %s] # name of the result transformation\n\
  in 'real' coordinates. Goes from 'reference' to 'floating', ie allows to \n\
  resample 'floating' in the geometry of 'reference'.\n\
  If indicated, '-result-voxel-transformation' is ignored.\n\
[-result-voxel-transformation|-res-voxel-trsf %s] # name of the result\n\
  transformation in 'voxel' coordinates.\n\
[-residuals %s]     # name of the output file for the residual values\n\
### transformation type ###\n\
[-transformation-type|-transformation|-trsf-type %s] # transformation type\n\
  translation2D, translation3D, translation-scaling2D, translation-scaling3D,\n\
  rigid2D, rigid3D, rigid, similitude2D, similitude3D, similitude,\n\
  affine2D, affine3D, affine, vectorfield2D, vectorfield3D, vectorfield, vector\n\
### deformation field / voxel size ###\n\
   specifying the output vector field with the above options\n\
   is mandatory for this kind of transformation\n\
 [-dim %d %d [%d]]      # output image dimensions\n\
 [-x %d]                # X dimension of the ouput image\n\
 [-y %d]                # Y dimension of the ouput image\n\
 [-z %d]                # Z dimension of the ouput image\n\
 [-template %s]         # template image for the dimensions\n\
                          of the output image\n\
 [-voxel %f %f [%f]]    # output image voxel sizes\n\
   allows to specify the voxel sizes when points are given in voxel units\n\
   same voxel sizes is assumed for both the floating and reference points\n\
### transformation estimation ###\n\
[-estimator-type|-estimator|-es-type %s] # transformation estimator\n\
  wlts: weighted least trimmed squares\n\
  lts: least trimmed squares\n\
  wls: weighted least squares\n\
  ls: least squares\n\
[-lts-fraction %lf] # for trimmed estimations, fraction of pairs that are kept\n\
[-lts-deviation %lf] # for trimmed estimations, defines the threshold to discard\n\
  pairings, ie 'average + this_value * standard_deviation'\n\
[-lts-iterations %d] # for trimmed estimations, the maximal number of iterations\n\
### transformation estimation: vectorfield estimation dedicated parameters ###\n\
[-fluid-sigma|-lts-sigma] %lf %lf %lf] # sigma for fluid regularization,\n\
  ie field interpolation and regularization for pairings (only for vector field)\n\
[-vector-propagation-type|-propagation-type|-propagation direct|skiz] # \n\
  direct: exact propagation (but slow)\n\
  skiz: approximate propagation (but faster)\n\
[-vector-propagation-distance|-propagation-distance|-pdistance %f] # \n\
  distance propagation of initial pairings (ie displacements)\n\
  this implies the same displacement for the spanned sphere\n\
  (only for vectorfield). Distance is in world units (not voxels).\n\
[-vector-fading-distance|-fading-distance|-fdistance %f] # \n\
  area of fading for initial pairings (ie displacements)\n\
  this allows progressive transition towards null displacements\n\
  and thus avoid discontinuites. Distance is in world units (not voxels).\n\
### parallelism ###\n\
[-parallel|-no-parallel] # use parallelism (or not)\n\
[-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
[-max-chunks %d] # maximal number of chunks\n\
[-parallel-scheduling|-ps default|static|dynamic-one|dynamic|guided] # type\n\
  of scheduling for open mp\n\
### misc writing stuff ###\n\
[-command-line %s]           # write the command line\n\
[-logfile %s]                # write some output in this logfile\n\
[-verbose] [-no-verbose]\n\
[-h|-help|--h|--help]\n\
\n";





typedef struct local_parameter {

  /* file names
     - images
     - transformations
  */
  char *floating_points;
  char *reference_points;
  
  char *result_real_transformation;
  char *result_voxel_transformation;
 
  char *result_residual;

  /* parameters for  matching
   */
  enumTypeTransfo transformation_type;
  bal_estimator estimator;

  /* geometry information
   */
  enumUnitTransfo points_unit;

  char *template_image_name;

  bal_integerPoint dim;
  bal_doublePoint voxel;


  /* writing stuff
   */
  char *command_line_file;
  int print_time;

} local_parameter;



#define FILENAMELENGTH 128



static void _ErrorParse( char *str, int flag );
static void _Parse( int argc, char *argv[], local_parameter *p );
static void _InitParam( local_parameter *par );
static double _GetTime();
static double _GetClock();
static char *_BaseName( char *p );








int main(int argc, char *argv[])
{
  local_parameter p;

  bal_transformation theResTransformation;
  bal_image template;

  bal_typeFieldPointList theFloatingPoints;
  bal_typeFieldPointList theReferencePoints;

  FIELD theField;

  
  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


  /***************************************************
   *
   * parsing parameters
   *
   ***************************************************/
  program = argv[0];


  /* no arguments
   */
  if ( argc == 1 ) _ErrorParse( NULL, 0 );

  
  /* parsing parameters 
   */
  _InitParam( &p );
  _Parse( argc, argv, &p );



  /* last initialization
   */
  BAL_InitTypeFieldPointList( &theFloatingPoints );
  BAL_InitTypeFieldPointList( &theReferencePoints );
  BAL_InitTransformation( &theResTransformation );


  if ( BAL_InitImage( &template, (char*)NULL, 1, 1, 1, 1, UCHAR ) != 1 ) {
    fprintf( stderr, "%s: unable to initialize auxiliary image\n", program );
    _ErrorParse( "error when initializing template image", 0 );
  }


  /* reading a template image to get geometry information
   * especially for vector field transformations
   */
  if ( p.template_image_name != NULL ) {
    if ( BAL_ReadImage( &template, p.template_image_name, 1 ) != 1 ) {
      fprintf( stderr, "%s: unable to read '%s'\n", program, p.template_image_name );
      _ErrorParse( "error when reading template image", 0 );
    }
    p.dim.x = template.ncols;
    p.dim.y = template.nrows;
    p.dim.z = template.nplanes;
    p.voxel.x = template.vx;
    p.voxel.y = template.vy;
    p.voxel.z = template.vz;
    BAL_FreeImage( &template );
  }



  /* initializating transformation
   */
  theResTransformation.type = p.transformation_type;
  switch ( p.transformation_type ) {
  default :
    _ErrorParse( "such transformation type non handled yet", 0 );
    break;
    
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
    if ( BAL_AllocTransformation( &theResTransformation, p.transformation_type, NULL ) != 1 ) {
      _ErrorParse( "error when allocating linear transformation", 0 );
    }
    break;

  case VECTORFIELD_2D :
  case VECTORFIELD_3D :

    if ( p.dim.x > 0 && p.dim.y > 0 ) {
      if ( p.dim.z > 0 ) {
        if ( BAL_InitImage( &template, (char*)NULL, p.dim.x, p.dim.y, p.dim.z, 1, UCHAR ) != 1 ) {
          fprintf( stderr, "%s: unable to initialize auxiliary image\n", program );
          _ErrorParse( "error when initializing template image", 0 );
        }
      }
      else {
        if ( BAL_InitImage( &template, (char*)NULL, p.dim.x, p.dim.y, 1, 1, UCHAR ) != 1 ) {
          fprintf( stderr, "%s: unable to initialize auxiliary image (dimz=1) \n", program );
          _ErrorParse( "error when initializing template image", 0 );
        }
      }
      if ( p.voxel.x > 0.0 ) template.vx = p.voxel.x;
      if ( p.voxel.y > 0.0 ) template.vy = p.voxel.y;
      if ( p.voxel.z > 0.0 ) template.vz = p.voxel.z;
    }
    else {
      fprintf( stderr, "%s: negative dimensions, unable to initialize auxiliary image\n", program );
      _ErrorParse( "error when initializing template image", 0 );
    }
    
    if ( BAL_AllocTransformation( &theResTransformation, p.transformation_type, &template ) != 1 ) {
      BAL_FreeImage( &template );
      fprintf( stderr, "%s: unable to allocate result transformation\n", program );
      _ErrorParse( "error when allocating vector field transformation", 0 );
    }
    BAL_FreeImage( &template );

    break;

  }



  /* reading points 
   */

  if ( p.floating_points == (char*)NULL ) {
    _ErrorParse( "no floating point list", 0 );
  }
  else {
    if ( BAL_ReadTypeFieldPointList( &theFloatingPoints, p.floating_points ) != 1 ) {
      BAL_FreeTransformation( &theResTransformation );
      fprintf( stderr, "%s: error when opening/reading '%s'\n", program,  p.floating_points );
      _ErrorParse( "error when reading/opening floating point list", 0 );
    }
  }

  theFloatingPoints.unit =  p.points_unit;



  if ( p.reference_points == (char*)NULL ) {
    _ErrorParse( "no reference point list", 0 );
  }
  else {
    if ( BAL_ReadTypeFieldPointList( &theReferencePoints, p.reference_points ) != 1 ) {
      BAL_FreeTypeFieldPointList( &theFloatingPoints );      
      BAL_FreeTransformation( &theResTransformation );
      fprintf( stderr, "%s: error when opening/reading '%s'\n", program,  p.reference_points );
      _ErrorParse( "error when reading/opening reference point list", 0 );
    }
  }

  theReferencePoints.unit =  p.points_unit;


  if ( theFloatingPoints.n_data != theReferencePoints.n_data ) {
    fprintf( stderr, "%s: list of points have different lengths\n", program );
    fprintf( stderr, "   '%s' contains %d points\n", p.floating_points, 
             theFloatingPoints.n_data );
    fprintf( stderr, "   '%s' contains %d points\n", p.reference_points, 
             theReferencePoints.n_data );
    if ( theFloatingPoints.n_data > theReferencePoints.n_data )
      theFloatingPoints.n_data = theReferencePoints.n_data;
    else 
      theReferencePoints.n_data = theFloatingPoints.n_data;
    fprintf( stderr, "   uses the first %d of both lists\n", theFloatingPoints.n_data );
  }





  /* displacement field 
   */
  if ( BAL_AllocateField( &theField, theFloatingPoints.n_data ) != 1 ) {
    BAL_FreeTypeFieldPointList( &theFloatingPoints );
    BAL_FreeTypeFieldPointList( &theReferencePoints );
    BAL_FreeTransformation( &theResTransformation );
    _ErrorParse( "error when allocating displacement field", 0 );
  }
  theField.vx = p.voxel.x;
  theField.vy = p.voxel.y;
  theField.vz = p.voxel.z;


  /* won't work in the points are in voxel units, with different
     voxel sizes
     build a displacement field where
     origin = reference point
     displacement = floating point - reference point
  */
  if ( BAL_ComputePairingFieldFromPointList( &theField,
                                             &theFloatingPoints,
                                             &theReferencePoints ) != 1 ) {
    BAL_FreeField( &theField );
    BAL_FreeTypeFieldPointList( &theFloatingPoints );
    BAL_FreeTypeFieldPointList( &theReferencePoints );
    BAL_FreeTransformation( &theResTransformation );
    _ErrorParse( "error when computing displacement field", 0 );
  }


  /* transformation computation procedure assumes that
     the points are in voxel units (in the reference referential)
     Well, this is due to historical reasons, which is definitively not
     a good reason
  */
  if ( p.points_unit == REAL_UNIT ) {
    size_t i;
    for ( i=0; i<theField.n_computed_pairs; i++ ) {
      theField.data[i].origin.x /= theField.vx;
      theField.data[i].origin.y /= theField.vy;
      theField.data[i].origin.z /= theField.vz;
      theField.data[i].vector.x /= theField.vx;
      theField.data[i].vector.y /= theField.vy;
      theField.data[i].vector.z /= theField.vz;
    }
    theField.unit = VOXEL_UNIT;
  }


  /* computes transformation
   * the resulting transformation allows
   * - to reasmple a floating image into a reference one,
   * - to transform the reference points into the floating frame
   * it goes from the reference frame to the floating one
   */
  if ( BAL_ComputeIncrementalTransformation( &theResTransformation, 
                                             &theField, &(p.estimator) ) != 1 ) {
    BAL_FreeField( &theField );
    BAL_FreeTypeFieldPointList( &theFloatingPoints );
    BAL_FreeTypeFieldPointList( &theReferencePoints );
    BAL_FreeTransformation( &theResTransformation );
    _ErrorParse( "error when computing transformation", 0 );
  }


  BAL_FreeTypeFieldPointList( &theFloatingPoints );
  BAL_FreeTypeFieldPointList( &theReferencePoints );
  

  
  /* computes residuals
   */
  if ( p.result_residual != NULL ) {
    if ( BAL_ComputeTransformationResiduals( &theResTransformation, 
                                             &theField ) != 1 ) {
      BAL_FreeTransformation( &theResTransformation );
      _ErrorParse( "error when computing residuals", 0 );
    }
    {
      FILE *f = fopen( p.result_residual, "w" );
      size_t i;
      double sum = 0.0;
      if ( f == (FILE*)NULL ) {
        BAL_FreeTransformation( &theResTransformation );
        _ErrorParse( "error when opening residual file", 0 );
      }
      for ( i=0; i<theField.n_computed_pairs; i++ )
        fprintf( f, "%f\n", sqrt( theField.data[i].error ) );
      fprintf( f, "\n" );
      for ( i=0; i<theField.n_selected_pairs; i++ )
        sum += sqrt( theField.pointer[i]->error );
      fprintf( f, "# ----------------------------------------\n" );
      fprintf( f, "# average on %lu points\n", theField.n_selected_pairs );
      fprintf( f, "# ----------------------------------------\n" );
      fprintf( f, "%f\n", sum / (double)theField.n_selected_pairs );
      fclose( f );
    }
  }



  BAL_FreeField( &theField );



  /* writing transformation
   */

  if ( p.result_real_transformation != NULL ) {
    if ( BAL_WriteTransformation( &theResTransformation, p.result_real_transformation ) != 1 ) {
      BAL_FreeTransformation( &theResTransformation );
      fprintf( stderr, "%s: unable to write real result transformation '%s'\n", 
               program, p.result_real_transformation  );
      _ErrorParse( "error when writing transformation", 0 );
    }
  }

  if ( p.result_voxel_transformation != NULL ) {
    template.vx = p.voxel.x;
    template.vy = p.voxel.y;
    template.vz = p.voxel.z;
    if ( BAL_ChangeTransformationToVoxelUnit( &template, &template, 
                                              &theResTransformation,
                                              &theResTransformation ) != 1 ) {
      BAL_FreeTransformation( &theResTransformation );
      _ErrorParse( "unable to convert 'real' transformation into the 'voxel' world", 0 );
    }

    if ( BAL_WriteTransformation( &theResTransformation, p.result_voxel_transformation ) != 1 ) {
      BAL_FreeTransformation( &theResTransformation );
      fprintf( stderr, "%s: unable to write voxel result transformation '%s'\n",
               program, p.result_voxel_transformation);
      _ErrorParse( "error when writing transformation", 0 );
    }
  }

  BAL_FreeTransformation( &theResTransformation );
  

  /* end
   */
  time_exit = _GetTime();
  clock_exit = _GetClock();

  if ( p.print_time ) { 
    fprintf( stderr, "%s: elapsed (real) time = %f\n", _BaseName(program), time_exit - time_init );
    fprintf( stderr, "\t       elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
    fprintf( stderr, "\t       ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
  }

  return( 0 );
}







/************************************************************
 *
 *
 *
 ************************************************************/


static void _ErrorParse( char *str, int flag )
{
  (void)fprintf( stderr, "Usage: %s %s", _BaseName(program), usage );
  if ( flag > 0 ) (void)fprintf( stderr, "%s\n", detail );
  if ( str != NULL ) (void)fprintf( stderr, "Error: %s\n", str );
  exit( -1 );
}



/************************************************************
 *
 * reading parameters is done in two steps
 * 1. one looks for the transformation type
 *    -> this implies dedicated initialization
 * 2. the other parameters are read
 * 
 ************************************************************/

static void _Parse( int argc, char *argv[], local_parameter *p )
{
  int i;
  int status;
  int maxchunks;

  program = argv[0];
  


  /* reading the  parameters
   */

  for ( i=1; i<argc; i++ ) {

    /* transformation type
    */
    if ( strcmp ( argv[i], "-transformation-type" ) == 0 
         || strcmp ( argv[i], "-transformation" ) == 0
         || strcmp ( argv[i], "-trsf-type" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-transformation-type", 0 );
      if ( strcmp ( argv[i], "translation2D" ) == 0 ) {
        p->transformation_type = TRANSLATION_2D;
      }
      else if ( strcmp ( argv[i], "translation3D" ) == 0 ) {
        p->transformation_type = TRANSLATION_3D;
      }
      else if ( strcmp ( argv[i], "translation" ) == 0 && argv[i][11] == '\0') {
        p->transformation_type = TRANSLATION_3D;
      }
      else if ( strcmp ( argv[i], "translation-scaling2D" ) == 0 ) {
        p->transformation_type = TRANSLATION_SCALING_2D;
      }
      else if ( strcmp ( argv[i], "translation-scaling3D" ) == 0 ) {
        p->transformation_type = TRANSLATION_SCALING_3D;
      }
      else if ( strcmp ( argv[i], "rigid2D" ) == 0 ) {
        p->transformation_type = RIGID_2D;
      }
      else if ( strcmp ( argv[i], "rigid3D" ) == 0 ) {
        p->transformation_type = RIGID_3D;
      }
      else if ( (strcmp ( argv[i], "rigid" ) == 0 && argv[i][5] == '\0') ) {
        p->transformation_type = RIGID_3D;
      }
      else if ( strcmp ( argv[i], "similitude2D" ) == 0 ) {
        p->transformation_type = SIMILITUDE_2D;
      }
      else if ( strcmp ( argv[i], "similitude3D" ) == 0 ) {
        p->transformation_type = SIMILITUDE_3D;
      }
      else if ( strcmp ( argv[i], "similitude" ) == 0 ) {
        p->transformation_type = SIMILITUDE_3D;
      }
      else if ( strcmp ( argv[i], "affine2D" ) == 0 ) {
        p->transformation_type = AFFINE_2D;
      }
      else if ( strcmp ( argv[i], "affine3D" ) == 0 ) {
        p->transformation_type = AFFINE_3D;
      }
      else if ( strcmp ( argv[i], "affine" ) == 0 ) {
        p->transformation_type = AFFINE_3D;
      }
      /*
        else if ( strcmp ( argv[i], "spline" ) == 0 ) {
        p->transformation_type = SPLINE;
        }
      */
      else if ( strcmp ( argv[i], "vectorfield" ) == 0 
                || strcmp ( argv[i], "vector" ) == 0 ) {
        p->transformation_type = VECTORFIELD_3D;
      }
      else if ( strcmp ( argv[i], "vectorfield3D" ) == 0 
                || strcmp ( argv[i], "vector3D" ) == 0 ) {
        p->transformation_type = VECTORFIELD_3D;
      }
      else if ( strcmp ( argv[i], "vectorfield2D" ) == 0 
                || strcmp ( argv[i], "vector2D" ) == 0 ) {
        p->transformation_type = VECTORFIELD_2D;
      }
      else {
        fprintf( stderr, "unknown transformation type: '%s'\n", argv[i] );
        _ErrorParse( "-transformation-type", 0 );
      }
    }

    
    /* image file names 
     */
    else if ( strcmp ( argv[i], "-reference") == 0
              || (strcmp ( argv[i], "-ref") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -reference", 0 );
      p->reference_points = argv[i];
    }
    else if ( strcmp ( argv[i], "-floating") == 0
              || (strcmp ( argv[i], "-flo") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -floating", 0 );
      p->floating_points = argv[i];
    }



    /* transformation file names 
     */
    else if ( strcmp ( argv[i], "-result-transformation" ) == 0
              || strcmp ( argv[i], "-res-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -result-transformation", 0 );
      p->result_real_transformation = argv[i];
    }
    else if ( strcmp ( argv[i], "-result-voxel-transformation" ) == 0
              || strcmp ( argv[i], "-res-voxel-trsf" ) == 0 ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -result-voxel-transformation", 0 );
      p->result_voxel_transformation = argv[i];
    }



    /* transformation file names 
     */
    else if ( strcmp ( argv[i], "-residuals" ) == 0 ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -residuals", 0 );
      p->result_residual = argv[i];
    }



    /* estimator definition and computation 
     */
    else if ( strcmp ( argv[i], "-estimator-type") == 0 
              || strcmp ( argv[i], "-estimator") == 0
              || strcmp ( argv[i], "-es-type") == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-estimator-type", 0 );
      if ( (strcmp ( argv[i], "ltsw" ) == 0 && argv[i][4] == '\0')
           || (strcmp ( argv[i], "wlts" ) == 0 && argv[i][4] == '\0') ) {
        p->estimator.type = TYPE_WLTS;
      }
      else if ( strcmp ( argv[i], "lts" ) == 0 && argv[i][3] == '\0' ) {
        p->estimator.type = TYPE_LTS;
      }
      else if ( (strcmp ( argv[i], "lsw" ) == 0 && argv[i][3] == '\0')
                || (strcmp ( argv[i], "wls" ) == 0 && argv[i][3] == '\0') ) {
        p->estimator.type = TYPE_WLS;
      }
      else if ( strcmp ( argv[i], "ls" ) == 0 && argv[i][2] == '\0' ) {
        p->estimator.type = TYPE_LS;
      }
      else {
        fprintf( stderr, "unknown estimator type: '%s'\n", argv[i] );
        _ErrorParse( "-estimator-type", 0 );
      }
    }

    else if ( strcmp ( argv[i], "-lts-fraction" ) == 0 
              || strcmp ( argv[i], "-lts-cut" ) == 0) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-lts-fraction", 0 );
      status = sscanf( argv[i], "%lf", &(p->estimator.retained_fraction) );
      if ( status <= 0 ) _ErrorParse( "-lts-fraction", 0 );
    }

    else if ( strcmp ( argv[i], "-lts-deviation" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-lts-deviation", 0 );
      status = sscanf( argv[i], "%lf", &(p->estimator.standard_deviation_threshold) );
      if ( status <= 0 ) _ErrorParse( "-lts-deviation", 0 );
    }

    else if ( strcmp ( argv[i], "-lts-iterations" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-lts-iterations", 0 );
      status = sscanf( argv[i], "%d", &(p->estimator.max_iterations) );
      if ( status <= 0 ) _ErrorParse( "-lts-iterations", 0 );
    }
    
    else if ( (strcmp (argv[i], "-fluid-sigma" ) == 0 && argv[i][12] == '\0')
              || (strcmp (argv[i], "-lts-sigma" ) == 0 && argv[i][10] == '\0') ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "parsing -lts-sigma %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->estimator.sigma.x) );
      if ( status <= 0 ) _ErrorParse( "parsing -lts-sigma %lf", 0 );
      i ++;
      if ( i >= argc) {
        p->estimator.sigma.y = p->estimator.sigma.x;
        p->estimator.sigma.z = p->estimator.sigma.x;
      }
      else {
        status = sscanf( argv[i], "%lf", &(p->estimator.sigma.y) );
        if ( status <= 0 ) {
          i--;
          p->estimator.sigma.y = p->estimator.sigma.x;
          p->estimator.sigma.z = p->estimator.sigma.x;
        }
        else {
          i ++;
          if ( i >= argc) p->estimator.sigma.z = 0;
          else {
            status = sscanf( argv[i], "%lf", &(p->estimator.sigma.z) );
            if ( status <= 0 ) {
              i--;
              p->estimator.sigma.z = 0;
            }
          }
        }
      }
    }

    else if ( strcmp (argv[i], "-vector-propagation-type" ) == 0
               || strcmp (argv[i], "-propagation-type" ) == 0
               || ( strcmp (argv[i], "-propagation" ) == 0 && argv[i][12] == '\0') ) {
      i++;
      if ( strcmp (argv[i], "direct" ) == 0 ) {
        p->estimator.propagation.type = TYPE_DIRECT_PROPAGATION;
      }
      else if ( strcmp (argv[i], "skiz" ) == 0 ) {
        p->estimator.propagation.type = TYPE_SKIZ_PROPAGATION;
      }
      else {
        _ErrorParse( "unknown propagation type for '-vector-propagation-type'", 0 );
      }
    }

    else if ( strcmp (argv[i], "-vector-propagation-distance" ) == 0
               || strcmp (argv[i], "-propagation-distance" ) == 0
               || (strcmp (argv[i], "-pdistance" ) == 0 && argv[i][10] == '\0') ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-vector-propagation-distance", 0 );
      status = sscanf( argv[i], "%f", &(p->estimator.propagation.constant) );
      if ( status <= 0 ) _ErrorParse( "-vector-propagation-distance", 0 );
    }
    else if ( strcmp (argv[i], "-vector-fading-distance" ) == 0
               || strcmp (argv[i], "-fading-distance" ) == 0
               || (strcmp (argv[i], "-fdistance" ) == 0 && argv[i][10] == '\0') ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-vector-fading-distance", 0 );
      status = sscanf( argv[i], "%f", &(p->estimator.propagation.fading) );
      if ( status <= 0 ) _ErrorParse( "-vector-fading-distance", 0 );
    }


    /* geometry information
     */

   /* estimator definition and computation 
     */
    else if ( strcmp ( argv[i], "-units") == 0 
              || strcmp ( argv[i], "-unit") == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-units", 0 );
      if ( (strcmp ( argv[i], "real" ) == 0 && argv[i][4] == '\0') ) {
        p->points_unit = REAL_UNIT;
      }
      else if ( (strcmp ( argv[i], "voxel" ) == 0 && argv[i][5] == '\0')
                || (strcmp ( argv[i], "pixel" ) == 0 && argv[i][5] == '\0') ) {
        p->points_unit = VOXEL_UNIT;
      }
      else {
        fprintf( stderr, "unknown units: '%s'\n", argv[i] );
        _ErrorParse( "-units", 0 );
      }
    }

    else if ( strcmp ( argv[i], "-template") == 0
         || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
         || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -reference", 0 );
      p->template_image_name = argv[i];
    }
    
    else if ( strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0' ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "parsing -dim %d", 0 );
      status = sscanf( argv[i], "%d", &(p->dim.x) );
      if ( status <= 0 ) _ErrorParse( "parsing -dim %d", 0 );
      i ++;
      if ( i >= argc)    _ErrorParse( "parsing -dim %d %d", 0 );
      status = sscanf( argv[i], "%d", &(p->dim.y) );
      if ( status <= 0 ) _ErrorParse( "parsing -dim %d %d", 0 );
      i ++;
      if ( i >= argc) p->dim.z = 1;
      else {
        status = sscanf( argv[i], "%d", &(p->dim.z) );
        if ( status <= 0 ) {
          i--;
          p->dim.z = 1;
        }
      }
    }

    else if ( strcmp ( argv[i], "-x") == 0 && argv[i][2] == '\0' ) {
      i += 1;
      if ( i >= argc)    _ErrorParse( "parsing -x...\n", 0 );
      status = sscanf( argv[i],"%d",&(p->dim.x) );
      if ( status <= 0 ) _ErrorParse( "parsing -x...\n", 0 );
    }
    else if ( strcmp ( argv[i], "-y" ) == 0  && argv[i][2] == '\0' ) {
      i += 1;
      if ( i >= argc)    _ErrorParse( "parsing -y...\n", 0 );
      status = sscanf( argv[i],"%d",&(p->dim.y) );
      if ( status <= 0 ) _ErrorParse( "parsing -y...\n", 0 );
    }
    else if ( strcmp ( argv[i], "-z" ) == 0  && argv[i][2] == '\0' ) {
      i += 1;
      if ( i >= argc)    _ErrorParse( "parsing -z...\n", 0 );
      status = sscanf( argv[i],"%d",&(p->dim.z) );
      if ( status <= 0 ) _ErrorParse( "parsing -z...\n", 0 );
    }

    else if ( strcmp (argv[i], "-voxel" ) == 0 
              || strcmp ( argv[i], "-pixel" ) == 0
              || (strcmp ( argv[i], "-vs" ) == 0  && argv[i][3] == '\0') ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "parsing -voxel %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->voxel.x) );
      if ( status <= 0 ) _ErrorParse( "parsing -voxel %lf", 0 );
      i ++;
      if ( i >= argc)    _ErrorParse( "parsing -voxel %lf %lf", 0 );
      status = sscanf( argv[i], "%lf", &(p->voxel.y) );
      if ( status <= 0 ) _ErrorParse( "parsing -voxel %lf %lf", 0 );
      i ++;
      if ( i >= argc) p->voxel.z = 1;
      else {
        status = sscanf( argv[i], "%lf", &(p->voxel.z) );
        if ( status <= 0 ) {
          i--;
          p->voxel.z = 1;
        }
      }
    }

    else if ( strcmp ( argv[i], "-vx" ) == 0   && argv[i][3] == '\0' ) {
      i += 1;
      if ( i >= argc)    _ErrorParse( "parsing -vx...\n", 0 );
      status = sscanf( argv[i], "%lf", &(p->voxel.x) );
      if ( status <= 0 ) _ErrorParse( "parsing -vx...\n", 0 );
    }
    else if ( strcmp ( argv[i], "-vy" ) == 0   && argv[i][3] == '\0' ) {
      i += 1;
      if ( i >= argc)    _ErrorParse( "parsing -vy...\n", 0 );
      status = sscanf( argv[i], "%lf", &(p->voxel.y) );
      if ( status <= 0 ) _ErrorParse( "parsing -vy...\n", 0 );
    }
    else if ( strcmp ( argv[i], "-vz" ) == 0   && argv[i][3] == '\0' ) {
      i += 1;
      if ( i >= argc)    _ErrorParse( "parsing -vz...\n", 0 );
      status = sscanf( argv[i], "%lf", &(p->voxel.z) );
      if ( status <= 0 ) _ErrorParse( "parsing -vz...\n", 0 );
    }


    /* parallelism
     */
    else if ( strcmp ( argv[i], "-parallel" ) == 0 ) {
      setParallelism( _DEFAULT_PARALLELISM_ );
    }

    else if ( strcmp ( argv[i], "-no-parallel" ) == 0 ) {
       setParallelism( _NO_PARALLELISM_ );
    }
    
    else if ( strcmp ( argv[i], "-parallelism-type" ) == 0 ||
               strcmp ( argv[i], "-parallel-type" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-parallelism-type", 0 );
      if ( strcmp ( argv[i], "default" ) == 0 ) {
        setParallelism( _DEFAULT_PARALLELISM_ );
      }
      else if ( strcmp ( argv[i], "none" ) == 0 ) {
        setParallelism( _NO_PARALLELISM_ );
      }
      else if ( strcmp ( argv[i], "openmp" ) == 0 || strcmp ( argv[i], "omp" ) == 0 ) {
        setParallelism( _OMP_PARALLELISM_ );
      }
      else if ( strcmp ( argv[i], "pthread" ) == 0 || strcmp ( argv[i], "thread" ) == 0 ) {
        setParallelism( _PTHREAD_PARALLELISM_ );
      }
      else {
        fprintf( stderr, "unknown parallelism type: '%s'\n", argv[i] );
        _ErrorParse( "-parallelism-type", 0 );
      }
    }


    else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-max-chunks", 0 );
      status = sscanf( argv[i], "%d", &maxchunks );
      if ( status <= 0 ) _ErrorParse( "-max-chunks", 0 );
      if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
    }

    else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 || 
              ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][3] == '\0') ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-parallel-scheduling", 0 );
      if ( strcmp ( argv[i], "default" ) == 0 ) {
        setOmpScheduling( _DEFAULT_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "static" ) == 0 ) {
        setOmpScheduling( _STATIC_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "dynamic-one" ) == 0 ) {
        setOmpScheduling( _DYNAMIC_ONE_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "dynamic" ) == 0 ) {
        setOmpScheduling( _DYNAMIC_OMP_SCHEDULING_ );
      }
      else if ( strcmp ( argv[i], "guided" ) == 0 ) {
        setOmpScheduling( _GUIDED_OMP_SCHEDULING_ );
      }
      else {
        fprintf( stderr, "unknown omp scheduling type: '%s'\n", argv[i] );
        _ErrorParse( "-omp-scheduling", 0 );
      }
    }



    /* misc writing stuff
     */
    else if ( strcmp ( argv[i], "-command-line") == 0 ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -command-line", 0 );
      p->command_line_file = argv[i];
    }

    else if ( (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
      p->print_time = 1;
    }
    else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')  
              || (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
      p->print_time = 0;
    }


    else if ( (strcmp ( argv[i], "-verbose") == 0 && argv[i][8] == '\0')
              || (strcmp ( argv[i], "-v") == 0 && argv[i][2] == '\0') ) {
      incrementVerboseInChunks(  );
      BAL_IncrementVerboseInBalVectorField();
    }
    else if ( (strcmp ( argv[i], "-no-verbose") == 0) 
              || (strcmp ( argv[i], "-nv") == 0 && argv[i][3] == '\0') ) {
      decrementVerboseInChunks(  );
      BAL_SetVerboseInBalVectorField( 0 );
    }

    else if ( ( strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0' )
              || ( strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0' ) ) {
      _ErrorParse( NULL, 1 );
    }

    else if ( ( strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0' )
              || ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) ) {
      _ErrorParse( NULL, 1 );
    }
    
    else {
      fprintf(stderr,"unknown option: '%s'\n",argv[i]);
    }
  }
}





static void _InitParam( local_parameter *p ) 
{
  /* file names
     - images
     - transformations
  */
  p->floating_points = NULL;
  p->reference_points = NULL;

  p->result_real_transformation = NULL;
  p->result_voxel_transformation = NULL;

  p->result_residual = NULL;

  /* parameters for matching
   */
  p->transformation_type = RIGID_3D;
  BAL_InitEstimator( &(p->estimator) );
  p->estimator.type = TYPE_LS;

  /* geometry information
   */
  p->points_unit = REAL_UNIT;
  p->template_image_name = NULL;
  
  p->dim.x = 256;
  p->dim.y = 256;
  p->dim.z = 1;

  p->voxel.x = 1.0;
  p->voxel.y = 1.0;
  p->voxel.z = 1.0;

  /* writing stuff
   */
  p->command_line_file = NULL;

  p->print_time = 0;
} 





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






static char *_BaseName( char *p )
{
  int l;
  if ( p == (char*)NULL ) return( (char*)NULL );
  l = strlen( p ) - 1;
  while ( l >= 0 && p[l] != '/' ) l--;
  if ( l < 0 ) l = 0;
  if ( p[l] == '/' ) l++;
  return( &(p[l]) );
}



