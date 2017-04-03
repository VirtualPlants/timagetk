/*************************************************************************
 * api-interpolateTrsfs.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gael Michelin (gael.michelin@inria.fr)
 *
 * CREATION DATE:
 * Mer  2 sep 2015 14:52:10 CEST
 *
 * ADDITIONS, CHANGES
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>


#include <api-invTrsf.h>
#include <api-interpolateTrsfs.h>

#include <bal-transformation-tools.h>




static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_interpolateTrsfs( char *str, lineCmdParamInterpolateTrsfs *p );


/************************************************************
 *
 * main API
 *
 ************************************************************/




int API_INTERMEDIARY_interpolateTrsfs( char* thetrsf_name,
                                     char* template_image_name,
                                     char *param_str_1, char *param_str_2 )
{
    char *proc = "API_INTERMEDIARY_interpolateTrsfs";
    bal_transformation theTrsf;
    bal_transformation resTrsf;
    bal_image templateImage;

    lineCmdParamInterpolateTrsfs par;



    /* parameter initialization
     */
    API_InitParam_interpolateTrsfs( &par );

    /* parameter parsing
     */
    if ( param_str_1 != (char*)NULL )
        _API_ParseParam_interpolateTrsfs( param_str_1, &par );
    if ( param_str_2 != (char*)NULL )
        _API_ParseParam_interpolateTrsfs( param_str_2, &par );



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

    if ( API_interpolateTrsfs( &theTrsf, &resTrsf, param_str_1, param_str_2 ) != 1 ) {
        BAL_FreeTransformation( &theTrsf );
        BAL_FreeTransformation( &resTrsf );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to interpolate the transformation '%s'\n", proc, thetrsf_name );
        return( -1 );
    }





    /* freeing transformations
     */
    BAL_FreeTransformation( &theTrsf );
    BAL_FreeTransformation( &resTrsf );

    return(1);
}


/* this API does the job, ie invert the read input
 * transformation into the allocated output one
 */

int API_interpolateTrsfs( bal_transformation *theTrsf,
                        bal_transformation *resTrsf,
                        char *param_str_1,
                        char *param_str_2 )
{
    char *proc = "API_interpolateTrsfs";
    lineCmdParamInterpolateTrsfs par;
    bal_image imError;
    int i;
    double t;
    bal_transformation tmpTrsf;
    char name[STRINGLENGTH];


    /* parameter initialization
     */
    API_InitParam_interpolateTrsfs( &par );

    /* parameter parsing
     */
    if ( param_str_1 != (char*)NULL )
        _API_ParseParam_interpolateTrsfs( param_str_1, &par );
    if ( param_str_2 != (char*)NULL )
        _API_ParseParam_interpolateTrsfs( param_str_2, &par );

    if ( par.print_lineCmdParam )
        API_PrintParam_interpolateTrsfs( stderr, proc, &par, (char*)NULL );

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

    /* tmpTrsf definition for computation */

    BAL_InitTransformation(&tmpTrsf);

    if (BAL_AllocTransformation(&tmpTrsf,theTrsf->type,&(resTrsf->vx)) != 1) {
        if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
             && par.error_name[0] != '\0' ) {
            BAL_FreeImage( &imError );
            BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
        }
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to invert transformation\n", proc );
        return( -1 );
    }

    for (i = 0 ; i<par.n_img ; i++) {
      /* Multiply Transformation : T(t<-1) */
      t = ((double)i+1)/(par.n_img+1);
      API_MultiplyTrsf(theTrsf, &tmpTrsf, 1.0-t);

      /* Invert Transformation : T(1<-t) */
      if ( BAL_InverseTransformation( &tmpTrsf, resTrsf ) != 1 ) {
        BAL_FreeTransformation( &tmpTrsf );
        if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
             && par.error_name[0] != '\0' ) {
            BAL_FreeImage( &imError );
            BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
        }
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to invert transformation\n", proc );
        return( -1 );
      }

      /* Write transformation T(1<-t) */
      sprintf( name, par.output_format_1, t );

      if ( BAL_WriteTransformation(resTrsf, name) != 1 ) {
          BAL_FreeTransformation( &tmpTrsf );
          if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
               && par.error_name[0] != '\0' ) {
              BAL_FreeImage( &imError );
              BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
          }
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write transformation %s\n", proc, name );
          return( -1 );
      }

      /* Multiply Transformation : T(0<-t) */
      API_MultiplyTrsf(resTrsf, resTrsf, -t/(1.0-t));

      /* Write transformation T(0<-t) */
      sprintf( name, par.output_format_0, t );

      if ( BAL_WriteTransformation(resTrsf, name) != 1 ) {
          BAL_FreeTransformation( &tmpTrsf );
          if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
               && par.error_name[0] != '\0' ) {
              BAL_FreeImage( &imError );
              BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
          }
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write transformation %s\n", proc, name );
          return( -1 );
      }


      /* imError */
      if ( (theTrsf->type == VECTORFIELD_2D || theTrsf->type == VECTORFIELD_3D)
         && par.error_name[0] != '\0' ) {
        if ( BAL_WriteImage( &imError, par.error_name ) != 1 ) {
            BAL_FreeTransformation( &tmpTrsf );
            BAL_FreeImage( &imError );
            BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to write error image '%s'\n", proc, par.error_name  );
            return( -1 );
        }
        BAL_FreeImage( &imError );
        BAL_SetImageInverseErrorsForVectorFieldInversionInBalTransformationTools( (bal_image*) NULL );
      }
    }

    BAL_FreeTransformation( &tmpTrsf );

    return( 1 );
}





void API_MultiplyTrsf(bal_transformation *trsf, bal_transformation *res, double coef)
{
    int i,j,k;

    float ***arrayIn=NULL, ***arrayOut=NULL;
    double *m=NULL, *M=NULL;

    switch (trsf->type)
    {
    case VECTORFIELD_2D:
    case VECTORFIELD_3D:
        arrayIn=(float***)trsf->vx.array;
        arrayOut=(float***)res->vx.array;
        for (k = 0 ; k < trsf->vx.nplanes ; k++ )
        for (j = 0 ; j < trsf->vx.nrows ; j++ )
        for (i = 0 ; i < trsf->vx.ncols ; i++ )
            arrayOut[k][j][i]=(float)coef*arrayIn[k][j][i];
        arrayIn=(float***)trsf->vy.array;
        arrayOut=(float***)res->vy.array;
        for (k = 0 ; k < trsf->vy.nplanes ; k++ )
        for (j = 0 ; j < trsf->vy.nrows ; j++ )
        for (i = 0 ; i < trsf->vy.ncols ; i++ )
            arrayOut[k][j][i]=(float)coef*arrayIn[k][j][i];
        arrayIn=(float***)trsf->vz.array;
        arrayOut=(float***)res->vz.array;
        for (k = 0 ; k < trsf->vz.nplanes ; k++ )
        for (j = 0 ; j < trsf->vz.nrows ; j++ )
        for (i = 0 ; i < trsf->vz.ncols ; i++ )
            arrayOut[k][j][i]=(float)coef*arrayIn[k][j][i];
        break;
    default:
        m=(double*)trsf->mat.m;
        M=(double*)res->mat.m;
        for (i = 0 ; i < trsf->mat.l * trsf->mat.c - 1 ; i++ )
            M[i] = coef * m[i];
        M[0] += 1-coef;
        M[5] += 1-coef;
        M[10] += 1-coef;
    }
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



static char *usage = "[transformation-in] [-formats %s %s] [-n %d]\n\
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
 [transformation-in]: transformation T(0<-1) from which the program computes\n\
                      intermediate transformations T(0<-t) and T(1<-t)\n\
 -formats %s %s: format 'a la printf' of transformations to be processed\n\
                 both must contain one '%f' (or '%1.1f' for example)\n\
                 depicts transformations of the form T_{i<-ref}\n\
 -n %d: number of transformations to be interpolated at regular intervals (defines t 'set')\n\
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



char *API_Help_interpolateTrsfs( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}

void API_ErrorParse_interpolateTrsfs( char *program, char *str, int flag )
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



void API_InitParam_interpolateTrsfs( lineCmdParamInterpolateTrsfs *p )
{
        (void)strncpy( p->input_name, "\0", 1 );
        (void)strncpy( p->output_format_0, "\0", 1 );
        (void)strncpy( p->output_format_1, "\0", 1 );

        (void)strncpy( p->template_name, "\0", 1 );

        p->template_dim.x = 0;
        p->template_dim.y = 0;
        p->template_dim.z = 0;

        p->template_voxel.x = 1.0;
        p->template_voxel.y = 1.0;
        p->template_voxel.z = 1.0;

        p->n_img=4;

        (void)strncpy( p->error_name, "\0", 1 );

        p->print_lineCmdParam = 0;
        p->print_time = 0;
}

void API_PrintParam_interpolateTrsfs( FILE *theFile, char *proc,
                                         lineCmdParamInterpolateTrsfs *p,
                                         char *str )
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

        fprintf( f, "- output transformations (0<-t) format is " );
        if ( p->output_format_0 != (char*)NULL && p->output_format_0[0] != '\0' )
          fprintf( f, "'%s'\n", p->output_format_0 );
        else
          fprintf( f, "'NULL'\n" );

        fprintf( f, "- output transformations (1<-t) format is " );
        if ( p->output_format_1 != (char*)NULL && p->output_format_1[0] != '\0' )
          fprintf( f, "'%s'\n", p->output_format_1 );
        else
          fprintf( f, "'NULL'\n" );

        fprintf( f, "- number of output transformations is " );
        fprintf( f, "%d\n", p->n_img);

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
        /*fprintf( f, "- image of inversion defaults is " );
        if ( p->error_name != (char*)NULL && p->error_name[0] != '\0' )
          fprintf( f, "'%s'\n", p->error_name );
        else
          fprintf( f, "'NULL'\n" );*/
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



static void _API_ParseParam_interpolateTrsfs( char *str, lineCmdParamInterpolateTrsfs *p )
{
        char *proc = "_API_ParseParam_interpolateTrsfs";
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

        API_ParseParam_interpolateTrsfs( 0, argc, argv, p );

        free( argv );
}




void API_ParseParam_interpolateTrsfs( int firstargc, int argc, char *argv[],
                                 lineCmdParamInterpolateTrsfs *p )
{
    int i;
    int inputisread = 0;
    int output0isread = 0;
    int output1isread = 0;
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
              else {
                API_ErrorParse_interpolateTrsfs( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
              }
            }

            else if ( strcmp ( argv[i], "-n") == 0 ) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -n ...\n", 0 );
                status = sscanf( argv[i], "%d", &p->n_img );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -n ...\n", 0 );
            }

            else if ( strcmp ( argv[i], "-output-0-format" ) == 0 ||
                      strcmp ( argv[i], "-output-0" ) == 0 ||
                      strcmp ( argv[i], "-format-0" ) == 0) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -format-0 ...\n", 0 );
               (void)strcpy( p->output_format_0,  argv[i] );
               output0isread = 1;
            }

            else if ( strcmp ( argv[i], "-output-1-format" ) == 0 ||
                      strcmp ( argv[i], "-output-1" ) == 0 ||
                      strcmp ( argv[i], "-format-1" ) == 0) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -format-1 ...\n", 0 );
               (void)strcpy( p->output_format_1,  argv[i] );
            }

            else if ( strcmp ( argv[i], "-output-formats" ) == 0 ||
                      strcmp ( argv[i], "-formats" ) == 0 ) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -formats ...\n", 0 );
               (void)strcpy( p->output_format_0,  argv[i] );
                output0isread = 1;
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -formats ...\n", 0 );
               (void)strcpy( p->output_format_1,  argv[i] );
                output1isread = 1;
            }


            /* vector field transformation, image geometry
             */
            else if ( strcmp ( argv[i], "-template-image") == 0
                      || (strcmp ( argv[i], "-template") == 0 && argv[i][9] == '\0')
                      || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
                      || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
                   i++;
                   if ( i >= argc) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-image", 0 );
                   (void)strcpy( p->template_name, argv[i] );
            }
            else if ( strcmp ( argv[i], "-template-dimensions") == 0
                      || strcmp ( argv[i], "-template-dimension") == 0
                      || strcmp ( argv[i], "-template-dim") == 0
                      || strcmp ( argv[i], "-dimensions") == 0
                      || strcmp ( argv[i], "-dimension") == 0
                      || (strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0') ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-dimensions %d", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.x) );
              if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-dimensions %d", 0 );
              i ++;
              if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.y) );
              if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
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
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -x ...\n", 0 );
                status = sscanf( argv[i], "%d", &(p->template_dim.x) );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -x ...\n", 0 );
            }
            else if ( strcmp ( argv[i], "-y") == 0 && argv[i][2] == '\0' ) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -y ...\n", 0 );
                status = sscanf( argv[i], "%d", &(p->template_dim.y) );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -y ...\n", 0 );
            }
            else if ( strcmp ( argv[i], "-z") == 0 && argv[i][2] == '\0' ) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -z ...\n", 0 );
                status = sscanf( argv[i], "%d", &(p->template_dim.z) );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -z ...\n", 0 );
            }
            else if ( strcmp ( argv[i], "-template-voxel") == 0
                      || strcmp ( argv[i], "-voxel-size") == 0
                      || (strcmp (argv[i], "-voxel" ) == 0 && argv[i][6] == '\0')
                      || (strcmp (argv[i], "-pixel" ) == 0 && argv[i][6] == '\0')
                      || (strcmp (argv[i], "-vs" ) == 0 && argv[i][3] == '\0') ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-voxel %lf", 0 );
              status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
              if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-voxel %lf", 0 );
              i ++;
              if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
              status = sscanf( argv[i], "%lf", &(p->template_voxel.y) );
              if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
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
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-error ...\n", 0 );
                status = sscanf( argv[i], "%lf", &error );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-error ...\n", 0 );
                if ( error > 0.0 ) BAL_SetErrorMaxForVectorFieldInversionInBalTransformationTools( error );
            }
            else if ( strcmp ( argv[i], "-inversion-iteration") == 0
                      || strcmp ( argv[i], "-inversion-iterations") == 0 ) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
                status = sscanf( argv[i], "%d", &iterations );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
                if ( iterations >= 0 ) BAL_SetIterationsMaxForVectorFieldInversionInBalTransformationTools( iterations );
            }
            else if ( strcmp ( argv[i], "-inversion-derivation-sigma") == 0 ) {
                i++;
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
                status = sscanf( argv[i], "%lf", &sigma );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
                if ( sigma > 0.0 ) BAL_SetDerivationSigmaForVectorFieldInversionInBalTransformationTools( sigma );
            }
            else if ( strcmp ( argv[i], "-inversion-error-image") == 0 ) {
                i++;
                if ( i >= argc) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-error-image", 0 );
                (void)strcpy( p->error_name, argv[i] );
            }
            else if ( strcmp ( argv[i], "-inversion-initialization" ) == 0 ) {
               i ++;
               if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-initialization ...\n", 0 );
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
                if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
                status = sscanf( argv[i], "%lf", &sigma );
                if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
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
               if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
                 API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
               }
            }

            else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
               i ++;
               if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -max-chunks ...\n", 0 );
               status = sscanf( argv[i], "%d", &maxchunks );
               if ( status <= 0 ) API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -max-chunks ...\n", 0 );
               if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
            }

            else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                     ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
               i ++;
               if ( i >= argc)    API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
                 API_ErrorParse_interpolateTrsfs( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
               }
            }


            /* general parameters
             */
            else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                      || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
               API_ErrorParse_interpolateTrsfs( (char*)NULL, (char*)NULL, 1);
            }
            else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                      || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
               API_ErrorParse_interpolateTrsfs( (char*)NULL, (char*)NULL, 0);
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
                API_ErrorParse_interpolateTrsfs( (char*)NULL, text, 0);
            }
        }

        /* strings beginning with a character different from '-'
         */
        else {
            if ( strlen( argv[i] ) >= STRINGLENGTH ) {
                fprintf( stderr, "... parsing '%s'\n", argv[i] );
                API_ErrorParse_interpolateTrsfs( (char*)NULL, "too long file name ...\n", 0 );
            }
            else if ( inputisread == 0 ) {
                (void)strcpy( p->input_name, argv[i] );
                inputisread = 1;
            }
            else if ( output0isread == 0 ) {
                (void)strcpy( p->output_format_0, argv[i] );
                output0isread = 1;
            }
            else if ( output1isread == 0 ) {
                (void)strcpy( p->output_format_1, argv[i] );
                output1isread = 1;
            }
            else {
                fprintf( stderr, "... parsing '%s'\n", argv[i] );
                API_ErrorParse_interpolateTrsfs( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
            }
        }
    }

    /* if not enough file names
     */
    if ( inputisread == 0 ) {
      (void)strcpy( p->input_name,  "<" );  /* standart input */
      inputisread = 1;
    }

}
