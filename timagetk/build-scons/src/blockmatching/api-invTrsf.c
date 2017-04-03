/*************************************************************************
 * api-invTrsf.c -
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
 * sed -e "s/invTrsf/execuTable/g" \
 *     -e "s/InvTrsf/ExecuTable/g" \
 *     -e "s/invtrsf/executable/g" \
 *     [api-]invTrsf.[c,h] > [api-]execTable.[c,h]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>


#include <api-invTrsf.h>

#include <bal-transformation-tools.h>




static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_invTrsf( char *str, lineCmdParamInvTrsf *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/

/* this one is kept for historical reasons,
 * ie Stracking compilation but should disappear
 */
int invTrsf( char* thetrsf_name,
             char* restrsf_name,
             char* template_image_name,
             bal_integerPoint dim,
             bal_doublePoint voxel,
             int verbose )
{
    char *proc = "invTrsf";
    char str[256], *s;

    _verbose_ = verbose;
    BAL_SetVerboseInBalTransformation( verbose );
    BAL_SetVerboseInBalTransformationTools( verbose );


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

    if ( API_INTERMEDIARY_invTrsf( thetrsf_name, restrsf_name, template_image_name,
                                   s, (char*)NULL ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: some error occurs\n", proc );
        return( -1 );
    }

    return( 0 );
}





int API_INTERMEDIARY_invTrsf( char* thetrsf_name,
                              char* restrsf_name,
                              char* template_image_name,
                              char *param_str_1, char *param_str_2 )

{
  char *proc = "API_INTERMEDIARY_invTrsf";
  bal_transformation theTrsf;
  bal_transformation resTrsf;
  bal_image templateImage;

  lineCmdParamInvTrsf par;



  /* parameter initialization
   */
  API_InitParam_invTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_invTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_invTrsf( param_str_2, &par );



  /***************************************************
   *
   *
   *
   ***************************************************/
  BAL_InitTransformation( &theTrsf );
  BAL_InitTransformation( &resTrsf );



  if ( thetrsf_name == NULL || thetrsf_name[0] == '\0' ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no input transformation\n", proc );
    return( -1 );
  }


  if ( BAL_ReadTransformation( &theTrsf, thetrsf_name ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to read '%s'\n", proc, thetrsf_name );
    return( -1 );
  }



  switch ( theTrsf.type ) {

  default :

    if ( _verbose_ )
      BAL_PrintTransformation( stderr, &theTrsf, "read transformation" );
    fprintf( stderr, "%s: such transformation type not handled yet\n", proc );
    BAL_FreeTransformation( &theTrsf );
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

    if ( BAL_AllocTransformation( &resTrsf, theTrsf.type, (bal_image *)NULL ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate result transformation (linear case)\n", proc );
      BAL_FreeTransformation( &theTrsf );
      return( -1 );
    }
    break;

  case VECTORFIELD_2D :
  case VECTORFIELD_3D :

    /* initializing result image
       - with reference image, if any
    */
    if ( template_image_name != NULL && template_image_name[0] != '\0' ) {
      if ( BAL_ReadImage( &templateImage, template_image_name, 1 ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read '%s'\n", proc, template_image_name );
        BAL_FreeTransformation( &theTrsf );
        return( -1 );
      }
    }

    /* initializing result image
       - with parameters, if any
    */
    else if ( par.template_dim.x > 0 && par.template_dim.y > 0 ) {
      if ( par.template_dim.z > 0 ) {
        if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, par.template_dim.z, 1, UCHAR ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize auxiliary image\n", proc );
          BAL_FreeTransformation( &theTrsf );
          return( -1 );
        }
      }
      else {
        if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, 1, 1, UCHAR ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize auxiliary image (dimz=1) \n", proc );
          BAL_FreeTransformation( &theTrsf );
          return( -1 );
        }
      }
      if ( par.template_voxel.x > 0.0 ) templateImage.vx = par.template_voxel.x;
      if ( par.template_voxel.y > 0.0 ) templateImage.vy = par.template_voxel.y;
      if ( par.template_voxel.z > 0.0 ) templateImage.vz = par.template_voxel.z;
    }

    /* initialisation with transformation
     */
    else {
      if ( BAL_InitImage( &templateImage, (char*)NULL,
                          theTrsf.vx.ncols, theTrsf.vx.nrows, theTrsf.vx.nplanes,
                          1, UCHAR ) != 1 ) {
        BAL_FreeTransformation( &theTrsf );
        fprintf( stderr, "%s: unable to initialize  auxiliary image (input vector field)\n", proc );
        return( -1 );
      }
      templateImage.vx = theTrsf.vx.vx;
      templateImage.vy = theTrsf.vx.vy;
      templateImage.vz = theTrsf.vx.vz;
    }

    if ( theTrsf.type == VECTORFIELD_2D ) {
      templateImage.nplanes = theTrsf.vx.nplanes;
    }


    if ( BAL_AllocTransformation( &resTrsf, theTrsf.type, &templateImage ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate result transformation (vector field)\n", proc );
      BAL_FreeImage( &templateImage );
      BAL_FreeTransformation( &theTrsf );
      return( -1 );
    }

    BAL_FreeImage( &templateImage );

  }


  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( API_invTrsf( &theTrsf, &resTrsf, param_str_1, param_str_2 ) != 1 ) {
      BAL_FreeTransformation( &theTrsf );
      BAL_FreeTransformation( &resTrsf );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to invert transformation '%s'\n", proc, thetrsf_name );
      return( -1 );
  }





  /* freeing input transformation
   */
  BAL_FreeTransformation( &theTrsf );



  /* writing output transformation
   */

  if ( restrsf_name != NULL && restrsf_name[0] != '\0' ) {
    if ( BAL_WriteTransformation( &resTrsf, restrsf_name ) != 1 ) {
        BAL_FreeTransformation( &resTrsf );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write '%s'\n", proc, restrsf_name );
        return( -1);
    }
  }
  else {
    if ( _verbose_ )
      fprintf( stderr, "%s: no output transformation name\n", proc );
    return( -1 );
  }


  BAL_FreeTransformation( &resTrsf );

  return( 1 );
}





int API_invTrsf( bal_transformation *theTrsf, bal_transformation *resTrsf, char *param_str_1, char *param_str_2 )
{
  char *proc = "API_invTrsf";
  lineCmdParamInvTrsf par;
  bal_image imError;



  /* parameter initialization
   */
  API_InitParam_invTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_invTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_invTrsf( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_invTrsf( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  BAL_InitImage( &imError, NULL, 0, 0, 0, 0, UCHAR );

  if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
       && par.error_name[0] != '\0' ) {
      if ( BAL_InitAllocImageFromImage( &imError, NULL, &(resTrsf->vx), UCHAR ) != 1 ) {
          if ( _verbose_ )
              fprintf( stderr, "%s: error when allocating error image\n", proc );
          return( -1 );
      }
      BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( &imError );
  }


  if ( BAL_InverseTransformation( theTrsf, resTrsf ) != 1 ) {
      if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
           && par.error_name[0] != '\0' ) {
          BAL_FreeImage( &imError );
          BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
      }
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to invert transformation\n", proc );
      return( -1 );
  }

  if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
       && par.error_name[0] != '\0' ) {
      if ( BAL_WriteImage( &imError, par.error_name ) != 1 ) {
          BAL_FreeImage( &imError );
          BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write error image '%s'\n", proc, par.error_name  );
          return( -1 );
      }
      BAL_FreeImage( &imError );
      BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
  }


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



static char *usage = "[transformation-in] [transformation-out]\n\
 [-template-image|-template|-dims|-t %s]\n\
 [-template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]]\n\
 [-x %d] [-y %d] [-z %d]\n\
 [-template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]]\n\
 [-inversion-error %lf] [-inversion-iteration %d]\n\
 [-inversion-derivation-sigma %lf] [-inversion-error-image %s]\n\
 [-inversion-initialization zero|forward] [-inversion-forward-sigma %lf]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
# specific options for vector field based transformations\n\
 -template-image|-template|-dims|-t %s: template image\n\
    to set image geometry for vector field transformation\n\
 -template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]: dimensions\n\
    of the resulting vector field\n\
 -x %d: dimension along X\n\
 -y %d: dimension along Y\n\
 -z %d: dimension along Z\n\
 -template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]:\n\
    voxel sizes of the resulting vector field\n\
# inversion of vector field based transformations\n\
 -inversion-error %lf: absolute error (in real world unit) to determine convergence\n\
 -inversion-iteration %d: maximal number of iterations to reach convergence\n\
 -inversion-derivation-sigma %lf: standard deviation of the gaussian used to compute derivatives\n\
 -inversion-error-image %s: create an image of convergence defaults\n\
 -inversion-initialization %s:\n\
    zero:\n\
    forward: forward interpolation from input vector field\n\
 -inversion-forward-sigma %lf: gaussian kernel for interpolation\n\
# parallelism parameters\n\
 -parallel|-no-parallel:\n\
 -max-chunks %d:\n\
 -parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread:\n\
 -omp-scheduling|-omps default|static|dynamic-one|dynamic|guided:\n\
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





char *API_Help_invTrsf( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_invTrsf( char *program, char *str, int flag )
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



void API_InitParam_invTrsf( lineCmdParamInvTrsf *p )
{
        (void)strncpy( p->input_name, "\0", 1 );
        (void)strncpy( p->output_name, "\0", 1 );

        (void)strncpy( p->template_name, "\0", 1 );

        p->template_dim.x = 0;
        p->template_dim.y = 0;
        p->template_dim.z = 0;

        p->template_voxel.x = 1.0;
        p->template_voxel.y = 1.0;
        p->template_voxel.z = 1.0;

        (void)strncpy( p->error_name, "\0", 1 );

        p->print_lineCmdParam = 0;
        p->print_time = 0;
}





void API_PrintParam_invTrsf( FILE *theFile, char *proc,
                                  lineCmdParamInvTrsf *p, char *str )
{

        FILE *f = theFile;
        if ( theFile == (FILE*)NULL ) f = stderr;

        fprintf( f, "==================================================\n" );
        fprintf( f, "= in line command parameters" );
        if ( proc != (char*)NULL )
          fprintf( f, " for '%s'", proc );
        if ( str != (char*)NULL )
          fprintf( f, "= %s\n", str );
        fprintf( f, "\n"  );
        fprintf( f, "==================================================\n" );


        fprintf( f, "# image names\n" );

        fprintf( f, "- input transformation is " );
        if ( p->input_name != (char*)NULL && p->input_name[0] != '\0' )
          fprintf( f, "'%s'\n", p->input_name );
        else
          fprintf( f, "'NULL'\n" );

        fprintf( f, "- output transformation is " );
        if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
          fprintf( f, "'%s'\n", p->output_name );
        else
          fprintf( f, "'NULL'\n" );

        fprintf( f, "# vector field transformation, image geometry\n" );

        fprintf( f, "- template image is " );
        if ( p->template_name != (char*)NULL && p->template_name[0] != '\0' )
          fprintf( f, "'%s'\n", p->template_name );
        else
          fprintf( f, "'NULL'\n" );

        fprintf( f, "- template image dimensions are [%d %d %d]\n",
                 p->template_dim.x, p->template_dim.y, p->template_dim.z );

        fprintf( f, "- template image voxel sizes are [%f %f %f]\n",
                 p->template_voxel.x, p->template_voxel.y, p->template_voxel.z );

        fprintf( f, "# vector field transformation, inversion monitoring\n" );

        fprintf( f, "- absolute error to determine convergence = %f\n",
                 BAL_GetErrorMaxForVectorFieldInversionInBalTransformationTools() );
        fprintf( f, "- maximal number of iterations to reach convergence = %d\n",
                 BAL_GetIterationsMaxForVectorFieldInversionInBalTransformationTools() );
        fprintf( f, "- gaussian sigma used to compute derivatives = %f\n",
                 BAL_GetDerivationSigmaForVectorFieldInversionInBalTransformationTools() );
        fprintf( f, "- image of inversion defaults is " );
        if ( p->error_name != (char*)NULL && p->error_name[0] != '\0' )
          fprintf( f, "'%s'\n", p->error_name );
        else
          fprintf( f, "'NULL'\n" );
        fprintf( f, "- initialization method: " );
        switch( BAL_GetInitializationForVectorFieldInversionInBalTransformationTools() ) {
        default :
          fprintf( f, "unknown method, this is embarassing\n" );
          break;
        case ZERO :
          fprintf( f, "'nul vectors'\n" );
          break;
        case FORWARD_INTERPOLATION :
          fprintf( f, "'forward interpolation'\n" );
          break;
        }
        fprintf( f, "- gaussian sigma used for forward interpolation = %f\n",
                 BAL_GetForwardSigmaForVectorFieldInversionInBalTransformationTools() );



        fprintf( f, "==================================================\n" );
}





/************************************************************
 *
 * parameters parsing
 *
 ************************************************************/



static void _API_ParseParam_invTrsf( char *str, lineCmdParamInvTrsf *p )
{
        char *proc = "_API_ParseParam_invTrsf";
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

        API_ParseParam_invTrsf( 0, argc, argv, p );

        free( argv );
}





void API_ParseParam_invTrsf( int firstargc, int argc, char *argv[],
                                  lineCmdParamInvTrsf *p )
{
  int i;
  int inputisread = 0;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;
  int iterations;
  double error, sigma;



  /* option line parsing
   */
  for ( i=firstargc; i<argc; i++ ) {

      /* strings beginning with '-'
       */
      if ( argv[i][0] == '-' ) {
          if ( argv[i][1] == '\0' ) {
            if ( inputisread == 0 ) {
              (void)strcpy( p->input_name,  "<" );  /* standart input */
              inputisread = 1;
            }
            else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name,  ">" );  /* standart output */
              outputisread = 1;
            }
            else {
              API_ErrorParse_invTrsf( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }


          /* vector field transformation, image geometry
           */
          else if ( strcmp ( argv[i], "-template-image") == 0
                    || (strcmp ( argv[i], "-template") == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-image", 0 );
                 (void)strcpy( p->template_name, argv[i] );
          }
          else if ( strcmp ( argv[i], "-template-dimensions") == 0
                    || strcmp ( argv[i], "-template-dimension") == 0
                    || strcmp ( argv[i], "-template-dim") == 0
                    || strcmp ( argv[i], "-dimensions") == 0
                    || strcmp ( argv[i], "-dimension") == 0
                    || (strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.x) );
            if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.y) );
            if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
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
              if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -x ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.x) );
              if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -x ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-y") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -y ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.y) );
              if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -y ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-z") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -z ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -z ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-template-voxel") == 0
                    || strcmp ( argv[i], "-voxel-size") == 0
                    || (strcmp (argv[i], "-voxel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-pixel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-vs" ) == 0 && argv[i][3] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
            if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.y) );
            if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
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



          /* vector field transformation, inversion monitoring
           */
          else if ( strcmp ( argv[i], "-inversion-error") == 0 && argv[i][16] == '\0') {
              i++;
              if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-error ...\n", 0 );
              status = sscanf( argv[i], "%lf", &error );
              if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-error ...\n", 0 );
              if ( error > 0.0 ) BAL_SetErrorMaxForVectorFieldInversionInBalTransformationTools( error );
          }
          else if ( strcmp ( argv[i], "-inversion-iteration") == 0
                    || strcmp ( argv[i], "-inversion-iterations") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
              status = sscanf( argv[i], "%d", &iterations );
              if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
              if ( iterations >= 0 ) BAL_SetIterationsMaxForVectorFieldInversionInBalTransformationTools( iterations );
          }
          else if ( strcmp ( argv[i], "-inversion-derivation-sigma") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
              status = sscanf( argv[i], "%lf", &sigma );
              if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
              if ( sigma > 0.0 ) BAL_SetDerivationSigmaForVectorFieldInversionInBalTransformationTools( sigma );
          }
          else if ( strcmp ( argv[i], "-inversion-error-image") == 0 ) {
              i++;
              if ( i >= argc) API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-error-image", 0 );
              (void)strcpy( p->error_name, argv[i] );
          }
          else if ( strcmp ( argv[i], "-inversion-initialization" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-initialization ...\n", 0 );
             if ( strcmp ( argv[i], "zero" ) == 0 ) {
                 BAL_SetInitializationForVectorFieldInversionInBalTransformationTools( ZERO );
             }
             else if ( strcmp ( argv[i], "forward" ) == 0 ) {
                 BAL_SetInitializationForVectorFieldInversionInBalTransformationTools( FORWARD_INTERPOLATION );
             }
             else {
               fprintf( stderr, "unknown initialization type: '%s'\n", argv[i] );
               API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-initialization ...\n", 0 );
             }
          }
          else if ( strcmp ( argv[i], "-inversion-forward-sigma") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
              status = sscanf( argv[i], "%lf", &sigma );
              if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
              if ( sigma > 0.0 ) BAL_SetForwardSigmaForVectorFieldInversionInBalTransformationTools( sigma );
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
             if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_invTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_invTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_invTrsf( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_invTrsf( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }


          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_invTrsf( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_invTrsf( (char*)NULL, (char*)NULL, 0);
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
              BAL_IncrementDebugInBalTransformationTools();
          }
          else if ( (strcmp ( argv[i], "-no-debug" ) == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-nodebug" ) == 0 && argv[i][8] == '\0') ) {
              _debug_ = 0;
              BAL_SetDebugInBalTransformationTools( 0 );
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
              API_ErrorParse_invTrsf( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_invTrsf( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( inputisread == 0 ) {
              (void)strcpy( p->input_name, argv[i] );
              inputisread = 1;
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_invTrsf( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
  if ( inputisread == 0 ) {
    (void)strcpy( p->input_name,  "<" );  /* standart input */
    inputisread = 1;
  }
  if ( outputisread == 0 ) {
    (void)strcpy( p->output_name,  ">" );  /* standart output */
    outputisread = 1;
  }

}
