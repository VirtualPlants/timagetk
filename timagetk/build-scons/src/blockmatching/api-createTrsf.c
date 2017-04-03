/*************************************************************************
 * api-createTrsf.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer 24 jui 2015 17:33:43 CEST
 *
 * ADDITIONS, CHANGES
 *
 * to generate files:
 * sed -e "s/createTrsf/execuTable/g" \
 *     -e "s/CreateTrsf/ExecuTable/g" \
 *     -e "s/createtrsf/executable/g" \
 *     [api-]createTrsf.[c,h] > [api-]execTable.[c,h]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>


#include <bal-lineartrsf-tools.h>
#include <bal-transformation.h>
#include <bal-transformation-tools.h>

#include <api-createTrsf.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_createTrsf( char *str, lineCmdParamCreateTrsf *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/

/* these ones are kept for historical reasons,
 * ie Stracking compilation but should disappear
 */

int createRandomTrsf( char *restrsf_name,
                      char *template_image_name,
                      bal_doublePoint fixedpoint,
                      enumTypeTransfo transformation_type,
                      int print )
{
    char *proc = "createRandomTrsf";
    char str[256], *s;

    if ( _verbose_ )
        fprintf( stderr, "Warning, '%s' is obsolete\n", proc );

    s = str;

    switch( transformation_type ) {
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: unknown transformation type\n", proc );
        return( -1 );
    case TRANSLATION_2D :
        sprintf( s, "-transformation-type translation2D " );
        break;
    case TRANSLATION_3D :
        sprintf( s, "-transformation-type translation3D " );
        break;
    case TRANSLATION_SCALING_2D :
        sprintf( s, "-transformation-type translation-scaling2D " );
        break;
    case TRANSLATION_SCALING_3D :
        sprintf( s, "-transformation-type translation-scaling3D " );
        break;
    case RIGID_2D :
        sprintf( s, "-transformation-type rigid2D " );
        break;
    case RIGID_3D :
        sprintf( s, "-transformation-type rigid3D " );
        break;
    case SIMILITUDE_2D :
        sprintf( s, "-transformation-type similitude2D " );
        break;
    case SIMILITUDE_3D :
        sprintf( s, "-transformation-type similitude3D " );
        break;
    case AFFINE_2D :
        sprintf( s, "-transformation-type affine2D " );
        break;
    case AFFINE_3D :
        sprintf( s, "-transformation-type affine3D " );
        break;
    case VECTORFIELD_2D :
        sprintf( s, "-transformation-type vectorfield2D " );
        break;
    case VECTORFIELD_3D :
        sprintf( s, "-transformation-type vectorfield3D " );
        break;
    }
    s = &(str[strlen(str)]);

    sprintf( s, "-transformation-value random " );
    s = &(str[strlen(str)]);

    sprintf( s, "-fixedpoint %lf %lf %lf ", fixedpoint.x, fixedpoint.y, fixedpoint.z );
    s = &(str[strlen(str)]);

    if ( print ) {
        sprintf( s, "-print-transformation " );
        s = &(str[strlen(str)]);
    }

    if ( API_INTERMEDIARY_createTrsf( restrsf_name,
                                      (char*)NULL,
                                      template_image_name,
                                      str, (char*)NULL ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: some error occurs\n", proc );
        return( -1 );
    }

    return( 1 );
}





int createVectorTrsf( char *restrsf_name,
                      char *template_image_name,
                      bal_integerPoint dim,
                      bal_doublePoint voxel,
                      enumTypeTransfo transformation_type,
                      enumValueTransfo vector_type,
                      int isDebug,
                      int isVerbose )
{
    char *proc = "createVectorTrsf";
    char str[256], *s;

    _debug_ = isDebug;

    _verbose_ = isVerbose;
    BAL_SetVerboseInBalImage( isVerbose );
    BAL_SetVerboseInBalTransformation( isVerbose );
    BAL_SetVerboseInBalTransformationTools( isVerbose );

    if ( _verbose_ )
        fprintf( stderr, "Warning, '%s' is obsolete\n", proc );

    s = str;

    if ( dim.x > 0 && dim.y > 0 ) {
        sprintf( s, "-dim %d %d ", dim.x, dim.y );
        s = &(str[strlen(str)]);
        if ( dim.z > 0 ) {
            sprintf( s, "%d ", dim.z );
            s = &(str[strlen(str)]);
        }
    }

    if ( voxel.x > 0 && voxel.y > 0 ) {
        sprintf( s, "-voxel %f %f ", voxel.x, voxel.y );
        s = &(str[strlen(str)]);
        if ( voxel.z > 0 ) {
            sprintf( s, "%f ", voxel.z );
            s = &(str[strlen(str)]);
        }
    }

    switch( transformation_type ) {
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: unknown transformation type\n", proc );
        return( -1 );
    case TRANSLATION_2D :
        sprintf( s, "-transformation-type translation2D " );
        break;
    case TRANSLATION_3D :
        sprintf( s, "-transformation-type translation3D " );
        break;
    case TRANSLATION_SCALING_2D :
        sprintf( s, "-transformation-type translation-scaling2D " );
        break;
    case TRANSLATION_SCALING_3D :
        sprintf( s, "-transformation-type translation-scaling3D " );
        break;
    case RIGID_2D :
        sprintf( s, "-transformation-type rigid2D " );
        break;
    case RIGID_3D :
        sprintf( s, "-transformation-type rigid3D " );
        break;
    case SIMILITUDE_2D :
        sprintf( s, "-transformation-type similitude2D " );
        break;
    case SIMILITUDE_3D :
        sprintf( s, "-transformation-type similitude3D " );
        break;
    case AFFINE_2D :
        sprintf( s, "-transformation-type affine2D " );
        break;
    case AFFINE_3D :
        sprintf( s, "-transformation-type affine3D " );
        break;
    case VECTORFIELD_2D :
        sprintf( s, "-transformation-type vectorfield2D " );
        break;
    case VECTORFIELD_3D :
        sprintf( s, "-transformation-type vectorfield3D " );
        break;
    }
    s = &(str[strlen(str)]);

    switch( vector_type ) {
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: unknown transformation value\n", proc );
        return( -1 );
    case _RANDOM_ :
        sprintf( s, "-transformation-value random " );
        break;
    case _IDENTITY_ :
        sprintf( s, "-transformation-value identity " );
        break;
    case _SINUS_2D_ :
        sprintf( s, "-transformation-value sinus2D " );
        break;
    case _SINUS_3D_ :
        sprintf( s, "-transformation-value sinus3D " );
        break;
    }
    s = &(str[strlen(str)]);


    if ( API_INTERMEDIARY_createTrsf( restrsf_name,
                                      template_image_name,
                                      (char*)NULL,
                                      str, (char*)NULL ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: some error occurs\n", proc );
        return( -1 );
    }

    return( 0 );
}





int API_INTERMEDIARY_createTrsf( char *output_name,
                                 char *template_vector,
                                 char *template_fixedpoint,
                                 char *param_str_1, char *param_str_2 )
{
  char *proc = "API_INTERMEDIARY_createTrsf";
  lineCmdParamCreateTrsf par;
  bal_transformation restrsf;
  bal_image templateImage;
  bal_image *templatePtr = (bal_image*)NULL;


  /* parameter initialization
   */
  API_InitParam_createTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_createTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_createTrsf( param_str_2, &par );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  /* transformation allocation
   */

  BAL_InitTransformation( &restrsf );

  /* initializing template image
   * - with reference image, if any
   */
  if ( template_vector != NULL && template_vector[0] != '\0' ) {
      if ( BAL_ReadImage( &templateImage, template_vector, 1 ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to read '%s'\n", proc, template_vector );
          return( -1 );
      }
      templatePtr = &templateImage;
  }

  /* initializing result image
   * - with parameters, if any
   */
  else if ( par.template_dim.x > 0 && par.template_dim.y > 0 ) {
    if ( par.template_dim.z > 0 ) {
      if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, par.template_dim.z, 1, UCHAR ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize auxiliary image\n", proc );
          return( -1 );
      }
    }
    else {
      if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, 1, 1, UCHAR ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize auxiliary image (dimz=1) \n", proc );
          return( -1 );
      }
    }
    templatePtr = &templateImage;
    if ( par.template_voxel.x > 0.0 ) templateImage.vx = par.template_voxel.x;
    if ( par.template_voxel.y > 0.0 ) templateImage.vy = par.template_voxel.y;
    if ( par.template_voxel.z > 0.0 ) templateImage.vz = par.template_voxel.z;
  }



  switch( par.transformation_type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such transformation type not handled yet\n", proc );
    return( -1 );

  case VECTORFIELD_2D :
  case VECTORFIELD_3D :
      if ( templatePtr == (bal_image*)NULL ) {
          if ( _verbose_ ) {
              fprintf( stderr, "%s: NULL template, unable to allocate vector field\n", proc );
          }
          return( -1 );
      }
      if ( BAL_AllocTransformation( &restrsf, par.transformation_type, templatePtr ) != 1 ) {
          if ( templatePtr != (bal_image*)NULL ) {
              BAL_FreeImage( &templateImage );
              templatePtr = (bal_image*)NULL;
          }
          if ( _verbose_ ) {
              fprintf( stderr, "%s: unable to allocate vector field\n", proc );
          }
          return( -1 );
      }

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
      if ( BAL_AllocTransformation( &restrsf, par.transformation_type, templatePtr ) != 1 ) {
          if ( templatePtr != (bal_image*)NULL ) {
              BAL_FreeImage( &templateImage );
              templatePtr = (bal_image*)NULL;
          }
          if ( _verbose_ ) {
              fprintf( stderr, "%s: unable to allocate linear transformation\n", proc );
          }
          return( -1 );
      }
      break;
  }

  if ( templatePtr != (bal_image*)NULL ) {
      BAL_FreeImage( &templateImage );
      templatePtr = (bal_image*)NULL;
  }

  /* fixed point image
   */

  if ( template_fixedpoint != NULL && template_fixedpoint[0] != '\0' ) {
      if ( BAL_ReadImage( &templateImage, template_fixedpoint, 1 ) != 1 ) {
          BAL_FreeTransformation( &restrsf );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to read '%s'\n", proc, template_fixedpoint );
          return( -1 );
      }
      templatePtr = &templateImage;
  }

  if ( API_createTrsf( &restrsf, templatePtr, param_str_1, param_str_2 ) != 1 ) {
      if ( templatePtr != (bal_image*)NULL ) {
          BAL_FreeImage( &templateImage );
          templatePtr = (bal_image*)NULL;
      }
      BAL_FreeTransformation( &restrsf );
      if ( _verbose_ )
          fprintf( stderr, "%s: an error occurs computation\n", proc );
      return( -1 );
  }
  
  if ( templatePtr != (bal_image*)NULL ) {
      BAL_FreeImage( &templateImage );
      templatePtr = (bal_image*)NULL;
  }

  if ( BAL_WriteTransformation( &restrsf, output_name ) != 1 ) {
      BAL_FreeTransformation( &restrsf );
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to write '%s'\n", proc, output_name );
      return( -1 );
  }

  return( 1 );
}





int API_createTrsf( bal_transformation *restrsf,
                    bal_image *template_fixedpoint,
                    char *param_str_1, char *param_str_2 )
{
  char *proc = "API_createTrsf";
  lineCmdParamCreateTrsf par;



  /* parameter initialization
   */
  API_InitParam_createTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_createTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_createTrsf( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_createTrsf( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  /* transformation value
   */

  switch( par.transformation_value ) {
  default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such transformation value not handled yet\n", proc );
      break;

  case _RANDOM_ :
      if ( BAL_SetTransformationToRandom( restrsf ) != 1 ) {
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to generate random transformation\n", proc );
          return( -1 );
      }
      break;

  case _IDENTITY_ :
      BAL_SetTransformationToIdentity( restrsf );
      break;
  case _SINUS_2D_ :
      if (  BAL_Sinusoid2DVectorField( restrsf ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to generate 2D sinus transformation\n", proc );
        return( -1);
      }
      break;
  case _SINUS_3D_ :
      if (  BAL_Sinusoid3DVectorField( restrsf ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to generate 2D sinus transformation\n", proc );
        return( -1);
      }
      break;
  }

  /* fixed point
   */
  if ( template_fixedpoint != (bal_image*)NULL ) {
      par.fixedpoint.x = ((template_fixedpoint->ncols-1) * template_fixedpoint->vx) / 2.0;
      par.fixedpoint.y = ((template_fixedpoint->nrows-1) * template_fixedpoint->vy) / 2.0;
      par.fixedpoint.z = ((template_fixedpoint->nplanes-1) * template_fixedpoint->vz) / 2.0;
  }

  if ( par.fixedpoint.x > -9000 && par.fixedpoint.y > -9000 && par.fixedpoint.z > -9000 ) {
    switch ( restrsf->type ) {
    case VECTORFIELD_2D :
    case VECTORFIELD_3D :
    default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such type not handled yet for input transformation\n", proc );
        break;
    case TRANSLATION_2D :
    case TRANSLATION_SCALING_2D :
    case RIGID_2D :
    case SIMILITUDE_2D :
    case AFFINE_2D :
      restrsf->mat.m[3] = par.fixedpoint.x - restrsf->mat.m[0] * par.fixedpoint.x - restrsf->mat.m[1] * par.fixedpoint.y;
      restrsf->mat.m[7] = par.fixedpoint.y - restrsf->mat.m[4] * par.fixedpoint.x - restrsf->mat.m[5] * par.fixedpoint.y;
      break;
    case TRANSLATION_3D :
    case TRANSLATION_SCALING_3D :
    case RIGID_3D :
    case SIMILITUDE_3D :
    case AFFINE_3D :
      restrsf->mat.m[ 3] = par.fixedpoint.x - restrsf->mat.m[0] * par.fixedpoint.x - restrsf->mat.m[1] * par.fixedpoint.y - restrsf->mat.m[ 2] * par.fixedpoint.z;
      restrsf->mat.m[ 7] = par.fixedpoint.y - restrsf->mat.m[4] * par.fixedpoint.x - restrsf->mat.m[5] * par.fixedpoint.y - restrsf->mat.m[ 6] * par.fixedpoint.z;
      restrsf->mat.m[11] = par.fixedpoint.z - restrsf->mat.m[8] * par.fixedpoint.x - restrsf->mat.m[9] * par.fixedpoint.y - restrsf->mat.m[10] * par.fixedpoint.z;
      break;
    }
  }


  if ( par.print_transformation )
      BAL_PrintTransformation( stderr, restrsf, "generated transformation" );

  return( 1 );
}






/************************************************************
 *
 * static functions
 *
 ************************************************************/



static char **_Str2Array( int *argc, char *str )
{
  char *proc = "_Str2Array";
  int n = 0;
  char *s = str;
  char **array, **a;

  if ( s == (char*)NULL || strlen( s ) == 0 ) {
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: empty input string\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  /* go to the first valid character
   */
  while ( *s == ' ' || *s == '\n' || *s == '\t' )
    s++;

  if ( *s == '\0' ) {
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: weird, input string contains only separation characters\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  /* count the number of strings
   */
  for ( n = 0; *s != '\0'; ) {
    n ++;
    while ( *s != ' ' && *s != '\n' && *s != '\t' && *s != '\0' )
      s ++;
    while ( *s == ' ' || *s == '\n' || *s == '\t' )
      s ++;
  }

  if ( _verbose_ >= 5 )
    fprintf( stderr, "%s: found %d strings\n", proc, n );

  /* the value of the strings will be duplicated
   * so that the input string can be freed
   */
  array = (char**)malloc( n * sizeof(char*) + (strlen(str)+1) * sizeof(char) );
  if ( array == (char**)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation failed\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  a = array;
  a += n;
  s = (char*)a;
  (void)strncpy( s, str, strlen( str ) );
  s[ strlen( str ) ] = '\0';

  while ( *s == ' ' || *s == '\n' || *s == '\t' ) {
    *s = '\0';
    s++;
  }

  for ( n = 0; *s != '\0'; ) {
    array[n] = s;
    n ++;
    while ( *s != ' ' && *s != '\n' && *s != '\t' && *s != '\0' )
      s ++;
    while ( *s == ' ' || *s == '\n' || *s == '\t' ) {
      *s = '\0';
      s ++;
    }
  }

  *argc = n;
  return( array );
}





/************************************************************
 *
 * help / documentation
 *
 ************************************************************/



static char *usage = "[[-res] transformation-out]\n\
 [-transformation-type|-transformation|-trsf-type translation2D| ...\n\
 ... |translation3D|translation-scaling2D|translation-scaling3D| ...\n\
 ... |rigid2D|rigid3D|similitude2D|similitude3D|affine2D|affine3D| ...\n\
 ... |vectorfield2D|vectorfield3D]\n\
 [-template-image|-template|-dims|-t %s]\n\
 [-template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]]\n\
 [-x %d] [-y %d] [-z %d]\n\
 [-template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]]\n\
 [-transformation-value|-value identity|random|sinus2D|sinus3D]\n\
 [-identity|-id] [-random|-rand]\n\
 [-angle-range %lf %lf] [-scale-range %lf %lf] [-shear-range %lf %lf]\n\
 [-translation-range %lf %lf]\n\
 [-sinusoid-amplitude %lf [%lf [%lf]]] [-sinusoid-period %lf [%lf [%lf]]]\n\
 [-srandom %ld]n\
 [-template-fixedpoint %s] [-fixedpoint %lf %lf [%lf]]\n\
 [-print-transformation|-print-trsf|-print]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [output-image-type | -type s8|u8|s16|u16...]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'transformation-out' is not specified or equal to '-', stdout will be used\n\
# transformation type ###\n\
 -transformation-type|-transformation|-trsf-type %s: ... \n\
    translation2D, translation3D, translation-scaling2D, translation-scaling3D,\n\
    rigid2D, rigid3D, rigid, similitude2D, similitude3D, similitude,\n\
    affine2D, affine3D, affine, vectorfield2D, vectorfield3D, vectorfield, vector\n\
# template image for vector field creation\n\
 -template-image|-template|-dims|-t %s: template image\n\
    to set image geometry for vector field transformation\n\
 -template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]: dimensions\n\
    of the resulting vector field\n\
 -x %d: dimension along X\n\
 -y %d: dimension along Y\n\
 -z %d: dimension along Z\n\
 -template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]:\n\
    voxel sizes of the resulting vector field\n\
# specific parameters\n\
 -transformation-value|-value %s: value of the transformation\n\
    identity:\n\
    random:\n\
    sinus2D: sinusoidal vectorfield (2D deformation)\n\
    sinus3D: sinusoidal vectorfield (3D deformation)\n\
 -identity|-id: short for '-transformation-value identity'\n\
 -random|-rand: short for '-transformation-value random'\n\
 -angle-range %lf %lf: to tune random linear transformation\n\
 -scale-range %lf %lf: to tune random linear transformation\n\
 -shear-range %lf %lf: to tune random linear transformation\n\
 -translation-range: %lf %lf: to tune random linear transformation\n\
 -sinusoid-amplitude %lf [%lf [%lf]]: to tune sinusoidal vectorfield\n\
 -sinusoid-period %lf [%lf [%lf]]: to tune sinusoidal vectorfield\n\
 -srandom %ld: seed for random number generator\n\
# fixed point\n\
 -template-fixedpoint %s: the center of field of view is left invariant\n\
    (linear transformations)\n\
 -fixedpoint %lf %lf [%lf]: point to be left invariant\n\
# ...\n\
# parallelism parameters\n\
 -parallel|-no-parallel:\n\
 -max-chunks %d:\n\
 -parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread:\n\
 -omp-scheduling|-omps default|static|dynamic-one|dynamic|guided:\n\
# general image related parameters\n\
   output-image-type: -o 1    : unsigned char\n\
                      -o 2    : unsigned short int\n\
                      -o 2 -s : short int\n\
                      -o 4 -s : int\n\
                      -r      : float\n\
  -type s8|u8|s16|u16|... \n\
   default is type of input image\n\
# general parameters \n\
  -verbose|-v: increase verboseness\n\
    parameters being read several time, use '-nv -v -v ...'\n\
    to set the verboseness level\n\
  -noverbose|-nv: no verboseness at all\n\
  -debug|-D: increase debug level\n\
  -nodebug: no debug indication\n\
  -print-parameters|-param:\n\
  -print-time|-time:\n\
  -no-time|-notime:\n\
  -h: print option list\n\
  -help: print option list + details\n\
";





char *API_Help_createTrsf( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_createTrsf( char *program, char *str, int flag )
{
    if ( flag >= 0 ) {
        if ( program != (char*)NULL )
           (void)fprintf(stderr,"Usage: %s %s\n", program, usage);
        else
            (void)fprintf(stderr,"Command line options: %s\n", usage);
    }
    if ( flag == 1 ) {
      (void)fprintf( stderr, "--------------------------------------------------\n" );
      (void)fprintf(stderr,"%s",detail);
      (void)fprintf( stderr, "--------------------------------------------------\n" );
    }
    if ( str != (char*)NULL )
      (void)fprintf(stderr,"Error: %s\n",str);
    exit(0);
}





/************************************************************
 *
 * parameters management
 *
 ************************************************************/



void API_InitParam_createTrsf( lineCmdParamCreateTrsf *p )
{
    (void)strncpy( p->output_name, "\0", 1 );
    p->transformation_type = AFFINE_3D;


    (void)strncpy( p->template_name, "\0", 1 );

    p->template_dim.x = 0;
    p->template_dim.y = 0;
    p->template_dim.z = 0;

    p->template_voxel.x = -1.0;
    p->template_voxel.y = -1.0;
    p->template_voxel.z = -1.0;

    p->transformation_value = _RANDOM_;

    (void)strncpy( p->template_fixedpoint, "\0", 1 );
    p->fixedpoint.x = -9999.0;
    p->fixedpoint.y = -9999.0;
    p->fixedpoint.z = -9999.0;

    p->print_transformation = 0;

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_createTrsf( FILE *theFile, char *program,
                                  lineCmdParamCreateTrsf *p, char *str )
{
  FILE *f = theFile;
  if ( theFile == (FILE*)NULL ) f = stderr;

  fprintf( f, "==================================================\n" );
  fprintf( f, "= in line command parameters" );
  if ( program != (char*)NULL )
    fprintf( f, " for '%s'", program );
  if ( str != (char*)NULL )
    fprintf( f, "= %s\n", str );
  fprintf( f, "\n"  );
  fprintf( f, "==================================================\n" );


  fprintf( f, "# image names\n" );

  fprintf( f, "- output transformation is " );
  if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation type is " );
  BAL_PrintTransformationType( f, p->transformation_type );

  fprintf( f, "# template for vector field geometry\n" );

  fprintf( f, "- template image is " );
  if ( p->template_name != (char*)NULL && p->template_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->template_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- template image dimensions are [%d %d %d]\n",
           p->template_dim.x, p->template_dim.y, p->template_dim.z );

  fprintf( f, "- template image voxel sizes are [%f %f %f]\n",
           p->template_voxel.x, p->template_voxel.y, p->template_voxel.z );

  fprintf( f, "# specific parameters\n" );

  fprintf( f, "- transformation values = " );
  switch( p->transformation_value ) {
  default : fprintf( f, "unknown, this is embarrasing ...\n" ); break;
  case _RANDOM_   : fprintf( f, "RANDOM\n" ); break;
  case _IDENTITY_ : fprintf( f, "IDENTITY\n" ); break;
  case _SINUS_2D_ : fprintf( f, "2D SINUSOID\n" ); break;
  case _SINUS_3D_ : fprintf( f, "3D SINUSOID\n" ); break;
  }

  fprintf( f, "- template image for fixed point is " );
  if ( p->template_fixedpoint != (char*)NULL && p->template_fixedpoint[0] != '\0' )
    fprintf( f, "'%s'\n", p->template_fixedpoint );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- fixed point is [%f %f %f]\n",
           p->template_voxel.x, p->template_voxel.y, p->template_voxel.z );

  fprintf( f, "==================================================\n" );
}





/************************************************************
 *
 * parameters parsing
 *
 ************************************************************/



static void _API_ParseParam_createTrsf( char *str, lineCmdParamCreateTrsf *p )
{
  char *proc = "_API_ParseParam_createTrsf";
  char **argv;
  int i, argc;

  if ( str == (char*)NULL || strlen(str) == 0 )
      return;

  argv = _Str2Array( &argc, str );
  if ( argv == (char**)NULL || argc == 0 ) {
      if ( _debug_ ) {
          fprintf( stderr, "%s: weird, no arguments were found\n", proc );
      }
      return;
  }

  if ( _debug_ > 4 ) {
      fprintf( stderr, "%s: translation from\n", proc );
      fprintf( stderr, "   '%s'\n", str );
      fprintf( stderr, "into\n" );
      for ( i=0; i<argc; i++ )
          fprintf( stderr, "   argv[%2d] = '%s'\n", i, argv[i] );
  }

  API_ParseParam_createTrsf( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_createTrsf( int firstargc, int argc, char *argv[],
                                  lineCmdParamCreateTrsf *p )
{
  int i;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;
  double min, max, s[3];
  long int init;



  /* option line parsing
   */
  for ( i=firstargc; i<argc; i++ ) {

      /* strings beginning with '-'
       */
      if ( argv[i][0] == '-' ) {
          if ( argv[i][1] == '\0' ) {
            if ( outputisread == 0 ) {
              (void)strcpy( p->output_name,  ">" );  /* standart output */
              outputisread = 1;
            }
            else {
              API_ErrorParse_createTrsf( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          else if ( strcmp ( argv[i], "-res" ) == 0  && argv[i][4] == '\0' ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_createTrsf( (char*)NULL, "parsing -res ...\n", 0 );
              if ( outputisread )
                  API_ErrorParse_createTrsf( (char*)NULL, "parsing -res: output already parsed ...\n", 0 );
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }

          else  if ( strcmp ( argv[i], "-transformation-type" ) == 0
                     || strcmp ( argv[i], "-transformation" ) == 0
                     || (strcmp ( argv[i], "-trsf-type" ) == 0 && argv[i][10] == '\0') ) {
              i ++;
              if ( i >= argc)
                  API_ErrorParse_createTrsf( (char*)NULL, "parsing -transformation-type", 0 );
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
                API_ErrorParse_createTrsf( (char*)NULL, "parsing -transformation-type", 0 );
              }
          }

          /* template
           */

          else if ( strcmp ( argv[i], "-template-image") == 0
                    || (strcmp ( argv[i], "-template") == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-image", 0 );
                 (void)strcpy( p->template_name, argv[i] );
          }
          else if ( strcmp ( argv[i], "-template-dimensions") == 0
                    || strcmp ( argv[i], "-template-dimension") == 0
                    || strcmp ( argv[i], "-template-dim") == 0
                    || strcmp ( argv[i], "-dimensions") == 0
                    || strcmp ( argv[i], "-dimension") == 0
                    || (strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.x) );
            if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.y) );
            if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            i ++;
            if ( i >= argc) p->template_dim.z = 1;
            else {
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) {
                i--;
                p->template_dim.z = 1;
              }
            }
          }
          else if ( strcmp ( argv[i], "-x") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -x ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.x) );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -x ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-y") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -y ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.y) );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -y ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-z") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -z ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -z ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-template-voxel") == 0
                    || strcmp ( argv[i], "-voxel-size") == 0
                    || (strcmp (argv[i], "-voxel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-pixel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-vs" ) == 0 && argv[i][3] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
            if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.y) );
            if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            i ++;
            if ( i >= argc) p->template_voxel.z = 1;
            else {
              status = sscanf( argv[i], "%lf", &(p->template_voxel.z) );
              if ( status <= 0 ) {
                i--;
                p->template_voxel.z = 1;
              }
            }
          }

          /* transformation value
           */
          else if ( strcmp( argv[i], "-transformation-value" ) == 0
                    || (strcmp( argv[i], "-value" ) == 0 && argv[i][6] == '\0') ) {
              i ++;
              if ( i >= argc)
                  API_ErrorParse_createTrsf( (char*)NULL, "parsing -transformation-value", 0 );
              if ( strcmp ( argv[i], "identity" ) == 0 ) {
                p->transformation_value = _IDENTITY_;
              }
              else if ( strcmp ( argv[i], "random" ) == 0 ) {
                  p->transformation_value = _RANDOM_;
              }
              else if ( strcmp ( argv[i], "sinus2D" ) == 0 ) {
                  p->transformation_value = _SINUS_2D_;
              }
              else if ( strcmp ( argv[i], "sinus3D" ) == 0 ) {
                  p->transformation_value = _SINUS_3D_;
              }
              else {
                fprintf( stderr, "unknown transformation value: '%s'\n", argv[i] );
                API_ErrorParse_createTrsf( (char*)NULL, "parsing -transformation-value", 0 );
              }
          }
          else if ( strcmp ( argv[i], "-random" ) == 0
                    || (strcmp ( argv[i], "-rand" ) == 0 && argv[i][5] == '\0' ) )  {
            p->transformation_value = _RANDOM_;
          }
          else if ( strcmp ( argv[i], "-identity" ) == 0
                    || (strcmp ( argv[i], "-id" ) == 0 && argv[i][3] == '\0' ) )  {
            p->transformation_value = _IDENTITY_;
          }

          else if ( strcmp ( argv[i], "-angle-range" ) == 0 ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -angle-range %lf", 0 );
              status = sscanf( argv[i], "%lf", &min );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -angle-range %lf", 0 );
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -angle-range %lf %lf", 0 );
              status = sscanf( argv[i], "%lf", &max );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -angle-range %lf %lf", 0 );
              BAL_SetMinAngleForRandomTransformation( min );
              BAL_SetMaxAngleForRandomTransformation( max );
          }
          else if ( strcmp ( argv[i], "-scale-range" ) == 0 ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -scale-range %lf", 0 );
              status = sscanf( argv[i], "%lf", &min );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -scale-range %lf", 0 );
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -scale-range %lf %lf", 0 );
              status = sscanf( argv[i], "%lf", &max );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -scale-range %lf %lf", 0 );
              BAL_SetMinScaleForRandomTransformation( min );
              BAL_SetMaxScaleForRandomTransformation( max );
          }
          else if ( strcmp ( argv[i], "-shear-range" ) == 0 ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -shear-range %lf", 0 );
              status = sscanf( argv[i], "%lf", &min );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -shear-range %lf", 0 );
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -shear-range %lf %lf", 0 );
              status = sscanf( argv[i], "%lf", &max );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -shear-range %lf %lf", 0 );
              BAL_SetMinShearForRandomTransformation( min );
              BAL_SetMaxShearForRandomTransformation( max );
          }
          else if ( strcmp ( argv[i], "-translation-range" ) == 0 ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -translation-range %lf", 0 );
              status = sscanf( argv[i], "%lf", &min );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -translation-range %lf", 0 );
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -translation-range %lf %lf", 0 );
              status = sscanf( argv[i], "%lf", &max );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -translation-range %lf %lf", 0 );
              BAL_SetMinTranslationForRandomTransformation( min );
              BAL_SetMaxTranslationForRandomTransformation( max );
          }
          else if ( strcmp ( argv[i], "-sinusoid-amplitude" ) == 0 ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -sinusoid-amplitude %lf", 0 );
              status = sscanf( argv[i], "%lf", &(s[0]) );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -sinusoid-amplitude %lf", 0 );
              i ++;
              if ( i >= argc) {
                  s[2] = s[1] = s[0];
                  i --;
              }
              else {
                  status = sscanf( argv[i], "%lf", &(s[1]) );
                  if ( status <= 0 ) {
                      s[2] = s[1] = s[0];
                      i --;
                  }
                  else {
                      i ++;
                      if ( i >= argc) {
                          s[2] = s[1];
                          i --;
                      }
                      else {
                          status = sscanf( argv[i], "%lf", &(s[2]) );
                          if ( status <= 0 ) {
                              s[2] = s[1];
                              i --;
                          }
                      }
                  }
              }
              BAL_SetSinusoidAmplitudeForVectorFieldTransformation( s );
          }
          else if ( strcmp ( argv[i], "-sinusoid-period" ) == 0 ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -sinusoid-period %lf", 0 );
              status = sscanf( argv[i], "%lf", &(s[0]) );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -sinusoid-period %lf", 0 );
              i ++;
              if ( i >= argc) {
                  s[2] = s[1] = s[0];
                  i --;
              }
              else {
                  status = sscanf( argv[i], "%lf", &(s[1]) );
                  if ( status <= 0 ) {
                      s[2] = s[1] = s[0];
                      i --;
                  }
                  else {
                      i ++;
                      if ( i >= argc) {
                          s[2] = s[1];
                          i --;
                      }
                      else {
                          status = sscanf( argv[i], "%lf", &(s[2]) );
                          if ( status <= 0 ) {
                              s[2] = s[1];
                              i --;
                          }
                      }
                  }
              }
              BAL_SetSinusoidPeriodForVectorFieldTransformation( s );
          }

          else if ( strcmp ( argv[i], "-srandom" ) == 0 ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -srandom %ld", 0 );
              status = sscanf( argv[i], "%ld", &init );
              if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -srandom %ld", 0 );
              _SetRandomSeed( init );
          }

          /* fixed point
           */
          else if ( strcmp ( argv[i], "-template-fixedpoint") == 0 ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_createTrsf( (char*)NULL, "parsing -template-fixedpoint", 0 );
                 (void)strcpy( p->template_fixedpoint, argv[i] );
          }
          else if ( strcmp (argv[i], "-fixedpoint" ) == 0 ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -fixedpoint %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->fixedpoint.x) );
            if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -fixedpoint %lf", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -fixedpoint %lf %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->fixedpoint.y) );
            if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -fixedpoint %lf %lf", 0 );
            i ++;
            if ( i >= argc) p->fixedpoint.z = 0;
            else {
              status = sscanf( argv[i], "%lf", &(p->fixedpoint.z) );
              if ( status <= 0 ) {
                i--;
                p->fixedpoint.z = 0;
              }
            }
          }

          else if ( strcmp ( argv[i], "-print-transformation" ) == 0
                    || (strcmp ( argv[i], "-print-trsf" ) == 0)
                    || (strcmp ( argv[i], "-print" ) == 0 && argv[i][6] == '\0') ) {
              p->print_transformation = 1;
          }

          /* parallelism parameters
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
             if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_createTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_createTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_createTrsf( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_createTrsf( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_createTrsf( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_createTrsf( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              BAL_IncrementVerboseInBalTransformation();
              BAL_IncrementVerboseInBalTransformationTools();
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              _verbose_ = 0;
              BAL_SetVerboseInBalTransformation( 0 );
              BAL_SetVerboseInBalTransformationTools( 0 );
          }
          else if ( (strcmp ( argv[i], "-debug" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0') ) {
              if ( _debug_ <= 0 ) _debug_ = 1;
              else                _debug_ ++;
          }
          else if ( (strcmp ( argv[i], "-no-debug" ) == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-nodebug" ) == 0 && argv[i][8] == '\0') ) {
              _debug_ = 0;
          }

          else if ( strcmp ( argv[i], "-print-parameters" ) == 0
                    || (strcmp ( argv[i], "-param" ) == 0 && argv[i][6] == '\0') ) {
             p->print_lineCmdParam = 1;
          }

          else if ( strcmp ( argv[i], "-print-time" ) == 0
                     || (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
             p->print_time = 1;
          }
          else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')
                      || (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
             p->print_time = 0;
          }

          /* unknown option
           */
          else {
              sprintf(text,"unknown option %s\n",argv[i]);
              API_ErrorParse_createTrsf( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_createTrsf( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_createTrsf( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
  if ( outputisread == 0 ) {
    (void)strcpy( p->output_name,  ">" );  /* standart output */
    outputisread = 1;
  }

}
