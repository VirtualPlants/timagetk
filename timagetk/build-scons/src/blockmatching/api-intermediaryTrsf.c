/*************************************************************************
 * api-intermediaryTrsf.c -
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
 * sed -e "s/intermediaryTrsf/execuTable/g" \
 *     -e "s/IntermediaryTrsf/ExecuTable/g" \
 *     -e "s/intermediarytrsf/executable/g" \
 *     [api-]intermediaryTrsf.[c,h] > [api-]execTable.[c,h]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>


#include <api-intermediaryTrsf.h>


#include <api-invTrsf.h>

#include <bal-transformation-tools.h>





static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_intermediaryTrsf( char *str, lineCmdParamIntermediaryTrsf *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/



int API_intermediaryTrsf( bal_transformation *theTrsf,
                          bal_transformation *resTrsf,
                          double t,
                          char *param_str_1,
                          char *param_str_2 )
{
  char *proc = "API_intermediaryTrsf";
  lineCmdParamIntermediaryTrsf par;




  /* parameter initialization
   */
  API_InitParam_intermediaryTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_intermediaryTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_intermediaryTrsf( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_intermediaryTrsf( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if (API_intermediaryTrsfStuff(theTrsf, resTrsf, t, par.flag_to_0) != 1) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to do the intermediary transformation stuff\n", proc );
      return(0);
  }

  return( 1 );
}

int API_intermediaryTrsfStuff(bal_transformation *theTrsf,
                              bal_transformation *resTrsf,
                              double t,
                              int flag_to_0)
{
    char *proc = "API_intermediaryTrsfStuff";
    bal_transformation tmpTrsf;
    /* tmpTrsf definition for computation */

    BAL_InitTransformation(&tmpTrsf);

    if (BAL_AllocTransformation(&tmpTrsf,theTrsf->type,&(resTrsf->vx)) != 1) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to invert transformation\n", proc );
        return( -1 );
    }



    /* Multiply Transformation : T(t<-1) */
    API_MultiplyTrsf(theTrsf, &tmpTrsf, 1.0-t);

    /* Invert Transformation : T(1<-t) */
    if ( BAL_InverseTransformation( &tmpTrsf, resTrsf ) != 1 ) {
      BAL_FreeTransformation( &tmpTrsf );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to invert transformation\n", proc );
      return( -1 );
    }

    if ( flag_to_0 == 1 ) {
      /* Multiply Transformation : T(0<-t) */
      API_MultiplyTrsf(resTrsf, resTrsf, -t/(1.0-t));
    }

    BAL_FreeTransformation( &tmpTrsf );

    return(1);
}


void API_MultiplyTrsf(bal_transformation *trsf, bal_transformation *res, double coef)
{
    size_t i, j, k;

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
        for (i = 0 ; i < (size_t)(trsf->mat.l * trsf->mat.c - 1); i++ )
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



static char *usage = "[transformation-in] [transformation-out] [-t %f]\n\
 [-t-to-1 | -t-to-0] [-template-image|-template %s]\n\
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
 if 'transformation-in' is equal to '-', stdin will be used\n\
 if 'transformation-out' is not specified or equal to '-', stdout will be used\n\
 if both are not specified, stdin and stdout will be used\n\
 -t %f: mandatory option to select the intermediary time-point between 0.0 and 1.0\n\
 -t-to-1: computes the intermediary transformation from time t to time 1 (default)\n\
 -t-to-0: computes the intermediary transformation from time t to time 0\n\
# specific options for vector field based transformations\n\
 -template-image|-template %s: template image\n\
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





char *API_Help_intermediaryTrsf( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_intermediaryTrsf( char *program, char *str, int flag )
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



void API_InitParam_intermediaryTrsf( lineCmdParamIntermediaryTrsf *p )
{
    (void)strncpy( p->input_name, "\0", 1 );
    (void)strncpy( p->output_name, "\0", 1 );
    (void)strncpy( p->template_name, "\0", 1 );
    p->print_lineCmdParam = 0;
    p->print_time = 0;

    p->flag_to_0 = 0;
    p->flag_template = 0;

    p->template_dim.x = 0;
    p->template_dim.y = 0;
    p->template_dim.z = 0;

    p->template_voxel.x = 1.0;
    p->template_voxel.y = 1.0;
    p->template_voxel.z = 1.0;

    p->t = -1;

}





void API_PrintParam_intermediaryTrsf( FILE *theFile, char *program,
                                  lineCmdParamIntermediaryTrsf *p, char *str )
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


  fprintf( f, "# transformation names\n" );

  fprintf( f, "- input transformation is " );
  if ( p->input_name != (char*)NULL && p->input_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- template image is " );
  if ( p->flag_template == 1 )
    fprintf( f, "'%s'\n", p->template_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation is " );
  if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation goes from t to " );
  fprintf( f, "%d\n", (p->flag_to_0 == 0) ? 1 : 0 );

  fprintf( f, "- input parameter t is " );
  fprintf( f, "%f\n", p->t );

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



static void _API_ParseParam_intermediaryTrsf( char *str, lineCmdParamIntermediaryTrsf *p )
{
  char *proc = "_API_ParseParam_intermediaryTrsf";
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

  API_ParseParam_intermediaryTrsf( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_intermediaryTrsf( int firstargc, int argc, char *argv[],
                                  lineCmdParamIntermediaryTrsf *p )
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
              (void)strcpy( p->output_name,  ">" );
              outputisread = 1;
            }
            else {
              API_ErrorParse_intermediaryTrsf( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* ...
           */


          else if ( (strcmp ( argv[i], "-t-on-0" ) == 0 ) || (strcmp ( argv[i], "-t-to-0" ) == 0 ) ) {
             p->flag_to_0 = 1;
          }

          else if ( (strcmp ( argv[i], "-t-on-1" ) == 0 ) || (strcmp ( argv[i], "-t-to-1" ) == 0 ) ) {
             p->flag_to_0 = 0;
          }

          else if ( strcmp ( argv[i], "-t" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -t ...\n", 0 );
             status = sscanf( argv[i], "%lf", &(p->t) );
             if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -t ...\n", 0 );
             if ( p->t <= 0 || p->t>=1 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -t ...  value must be > 0.0 and < 1.0 \n", 0 );
          }

          /* vector field transformation, image geometry
           */

          else if ( (strcmp ( argv[i], "-template" ) == 0 ) || (strcmp ( argv[i], "-template-image" ) == 0 ) ) {
             p->flag_template = 1;
             i=i+1;
             if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template ...\n", 0 );
             (void)strcpy( p->template_name,  argv[i] );
          }


          else if ( strcmp ( argv[i], "-template-dimensions") == 0
                    || strcmp ( argv[i], "-template-dimension") == 0
                    || strcmp ( argv[i], "-template-dim") == 0
                    || strcmp ( argv[i], "-dimensions") == 0
                    || strcmp ( argv[i], "-dimension") == 0
                    || (strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.x) );
            if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.y) );
            if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
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
              if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -x ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.x) );
              if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -x ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-y") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -y ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.y) );
              if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -y ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-z") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -z ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -z ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-template-voxel") == 0
                    || strcmp ( argv[i], "-voxel-size") == 0
                    || (strcmp (argv[i], "-voxel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-pixel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-vs" ) == 0 && argv[i][3] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
            if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.y) );
            if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
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
              if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-error ...\n", 0 );
              status = sscanf( argv[i], "%lf", &error );
              if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-error ...\n", 0 );
              if ( error > 0.0 ) BAL_SetErrorMaxForVectorFieldInversionInBalTransformationTools( error );
          }
          else if ( strcmp ( argv[i], "-inversion-iteration") == 0
                    || strcmp ( argv[i], "-inversion-iterations") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
              status = sscanf( argv[i], "%d", &iterations );
              if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
              if ( iterations >= 0 ) BAL_SetIterationsMaxForVectorFieldInversionInBalTransformationTools( iterations );
          }
          else if ( strcmp ( argv[i], "-inversion-derivation-sigma") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
              status = sscanf( argv[i], "%lf", &sigma );
              if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
              if ( sigma > 0.0 ) BAL_SetDerivationSigmaForVectorFieldInversionInBalTransformationTools( sigma );
          }
          else if ( strcmp ( argv[i], "-inversion-initialization" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-initialization ...\n", 0 );
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
              if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
              status = sscanf( argv[i], "%lf", &sigma );
              if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_intermediaryTrsf( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }

          /* general image related parameters
           */

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_intermediaryTrsf( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_intermediaryTrsf( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              _verbose_ = 0;
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
              API_ErrorParse_intermediaryTrsf( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_intermediaryTrsf( (char*)NULL, "too long file name ...\n", 0 );
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
              API_ErrorParse_intermediaryTrsf( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
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
      (void)strcpy( p->output_name,  "<" );  /* standart input */
  }

  /* option '-t' is mandatory
   */
  if ( p->t < 0 )
    API_ErrorParse_intermediaryTrsf( (char*)NULL, "option '-t' must be specified (value > 0.0 and < 1.0) \n", 0 );
}
