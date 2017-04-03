/*************************************************************************
 * api-interpolateImages.c -
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
 * sed -e "s/interpolateImages/execuTable/g" \
 *     -e "s/InterpolateImages/ExecuTable/g" \
 *     -e "s/interpolateimages/executable/g" \
 *     [api-]interpolateImages.[c,h] > [api-]execTable.[c,h]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>


#include <api-interpolateImages.h>


#include <api-intermediaryTrsf.h>



static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_interpolateImages( char *str, lineCmdParamInterpolateImages *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/



int API_interpolateImages( bal_image *image0,
                           bal_image *image1,
                           bal_transformation *T_1_t,
                           bal_image *imres,
                           double t, char *param_str_1, char *param_str_2 )
{
  char *proc = "API_interpolateImages";
  lineCmdParamInterpolateImages par;

  bal_image I_0, I_1;
  bal_transformation T_0_t;




  /* parameter initialization
   */
  API_InitParam_interpolateImages( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_interpolateImages( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_interpolateImages( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_interpolateImages( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  /* Definition + Computing I_1 = image1 o T_1_t  */

  if ( BAL_InitImage( &I_1, (char*)NULL, image1->ncols, image1->nrows, image1->nplanes, 1, image1->type ) != 1 ) {
      if ( _verbose_ )
        fprintf(stderr, "%s: unable to initialize an image\n", proc);
      return(0);
  }
  if (BAL_AllocImage( &I_1 ) != 1) {
      if ( _verbose_ )
        fprintf(stderr, "%s: unable to allocate an image\n", proc);
      return(0);
  }
  I_1.vx = image1->vx;
  I_1.vy = image1->vy;
  I_1.vz = image1->vz;

  if ( BAL_ResampleImage( image1, &I_1, T_1_t, par.interpolation ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute resampling\n", proc );
    return(-1);
  }

  /* Definition + Computing T_0_t */

  BAL_InitTransformation(&T_0_t);

  if (BAL_AllocTransformation(&T_0_t, T_1_t->type, &(T_1_t->vx)) != 1) {
      BAL_FreeImage(&I_1);
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate transformation\n", proc );
      return( -1 );
  }

  API_MultiplyTrsf(T_1_t, &T_0_t, -t/(1.0-t));

  /* Definition + Computing I_0 = image0 o T_0_t */

  if ( BAL_InitImage( &I_0, (char*)NULL, image0->ncols, image0->nrows, image0->nplanes, 1, image0->type ) != 1 ) {
      BAL_FreeImage(&I_1);
      BAL_FreeTransformation(&T_0_t);
      if ( _verbose_ )
        fprintf(stderr, "%s: unable to initialize an image\n", proc);
      return(0);
  }
  if (BAL_AllocImage( &I_0 ) != 1) {
      BAL_FreeImage(&I_1);
      BAL_FreeTransformation(&T_0_t);
      if ( _verbose_ )
        fprintf(stderr, "%s: unable to allocate an image\n", proc);
      return(0);
  }
  I_0.vx = image0->vx;
  I_0.vy = image0->vy;
  I_0.vz = image0->vz;

  if ( BAL_ResampleImage( image0, &I_0, &T_0_t, par.interpolation ) != 1 ) {
    BAL_FreeImage(&I_0);
    BAL_FreeImage(&I_1);
    BAL_FreeTransformation(&T_0_t);
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute resampling\n", proc );
    return(-1);
  }


  BAL_FreeTransformation(&T_0_t);

  /* Linear combination : imres = (1-t)*I_0 + t*I_1 */

  if (API_LinearCombination(&I_0, &I_1, imres, t) != 1)
  {
      BAL_FreeImage(&I_0);
      BAL_FreeImage(&I_1);
      if ( _verbose_ )
        fprintf(stderr, "%s: unable to apply the linear combination\n", proc);
      return(0);
  }

  /* frees */

  BAL_FreeImage(&I_0);
  BAL_FreeImage(&I_1);

  return( 1 );
}


/* API_LinearCombination : imres := (1-t)*I_0 + t*I_1 */

int API_LinearCombination(bal_image *I_0, bal_image *I_1, bal_image *imres, double t)
{
    char *proc="API_LinearCombination";
    size_t i,j,k;

    double v0, v1, vRes;

    unsigned char ***aU8=NULL, ***a0U8=NULL, ***a1U8=NULL;
    char ***aS8=NULL, ***a0S8=NULL, ***a1S8=NULL;
    unsigned short int ***aU16=NULL, ***a0U16=NULL, ***a1U16=NULL;
    short int ***aS16=NULL, ***a0S16=NULL, ***a1S16=NULL;
    float ***aR32=NULL, ***a0R32=NULL, ***a1R32=NULL;
    double ***aR64=NULL, ***a0R64=NULL, ***a1R64=NULL;

    if (I_0->nplanes != imres->nplanes || I_0->nrows != imres->nrows || I_0->ncols != imres->ncols ||
        I_1->nplanes != imres->nplanes || I_1->nrows != imres->nrows || I_1->ncols != imres->ncols )
    {
        fprintf(stderr, "%s: image sizes do not correspond, exiting...\n", proc);
        return(0);
    }



    switch(I_0->type) {
    default:
        fprintf(stderr, "%s: such image type not handled yet\n", proc);
        return(0);
    case UCHAR:
        a0U8=(unsigned char ***)I_0->array;
        break;
    case SCHAR:
        a0S8=(char ***)I_0->array;
        break;
    case USHORT:
        a0U16=(unsigned short int ***)I_0->array;
        break;
    case SSHORT:
        a0S16=(short int ***)I_0->array;
        break;
    case FLOAT:
        a0R32=(float ***)I_0->array;
        break;
    case DOUBLE:
        a0R64=(double ***)I_0->array;
        break;
    }

    switch(I_1->type) {
    default:
        fprintf(stderr, "%s: such image type not handled yet\n", proc);
        return(0);
    case UCHAR:
        a1U8=(unsigned char ***)I_1->array;
        break;
    case SCHAR:
        a1S8=(char ***)I_1->array;
        break;
    case USHORT:
        a1U16=(unsigned short int ***)I_1->array;
        break;
    case SSHORT:
        a1S16=(short int ***)I_1->array;
        break;
    case FLOAT:
        a1R32=(float ***)I_1->array;
        break;
    case DOUBLE:
        a1R64=(double ***)I_1->array;
        break;
    }

    switch(imres->type) {
    default:
        fprintf(stderr, "%s: such image type not handled yet\n", proc);
        return(0);
    case UCHAR:
        aU8=(unsigned char ***)imres->array;
        break;
    case SCHAR:
        aS8=(char ***)imres->array;
        break;
    case USHORT:
        aU16=(unsigned short int ***)imres->array;
        break;
    case SSHORT:
        aS16=(short int ***)imres->array;
        break;
    case FLOAT:
        aR32=(float ***)imres->array;
        break;
    case DOUBLE:
        aR64=(double ***)imres->array;
        break;
    }


    for (k=0;k<imres->nplanes;k++)
    for (j=0;j<imres->nrows;j++)
    for (i=0;i<imres->ncols;i++)
    {
        switch(I_0->type) {
        default:
            fprintf(stderr, "%s: such image type not handled yet\n", proc);
            return(0);
        case UCHAR:
            v0 = (double) a0U8[k][j][i];
            break;
        case SCHAR:
            v0 = (double) a0S8[k][j][i];
            break;
        case USHORT:
            v0 = (double) a0U16[k][j][i];
            break;
        case SSHORT:
            v0 = (double) a0S16[k][j][i];
            break;
        case FLOAT:
            v0 = (double) a0R32[k][j][i];
            break;
        case DOUBLE:
            v0 = (double) a0R64[k][j][i];
            break;
        }
        switch(I_1->type) {
        default:
            fprintf(stderr, "%s: such image type not handled yet\n", proc);
            return(0);
        case UCHAR:
            v1 = (double) a1U8[k][j][i];
            break;
        case SCHAR:
            v1 = (double) a1S8[k][j][i];
            break;
        case USHORT:
            v1 = (double) a1U16[k][j][i];
            break;
        case SSHORT:
            v1 = (double) a1S16[k][j][i];
            break;
        case FLOAT:
            v1 = (double) a1R32[k][j][i];
            break;
        case DOUBLE:
            v1 = (double) a1R64[k][j][i];
            break;
        }
        vRes = (1-t)*v0 + t*v1;
        switch(imres->type) {
        default:
            fprintf(stderr, "%s: such image type not handled yet\n", proc);
            return(0);
        case UCHAR:
            aU8[k][j][i] = (unsigned char)vRes;
            break;
        case SCHAR:
            aS8[k][j][i] = (char)vRes;
            break;
        case USHORT:
            aU16[k][j][i] = (unsigned short int)vRes;
            break;
        case SSHORT:
            aS16[k][j][i] = (short int)vRes;
            break;
        case FLOAT:
            aR32[k][j][i] = (float)vRes;
            break;
        case DOUBLE:
            aR64[k][j][i] = (double)vRes;
            break;
        }
    }

    return(1);
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



static char *usage = "[image-0] [image-1] [[-trsf] %s] [-format|-output|-output-format %s] [-n %d]\n\
 [-nearest|-linear|-cspline] [-interpolation nearest|linear|cspline]\n\
 [-template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]]\n\
 [-x %d] [-y %d] [-z %d]\n\
 [-template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]]\n\
 [-inversion-error %lf] [-inversion-iteration %d]\n\
 [-inversion-derivation-sigma %lf] [-inversion-error-image %s]\n\
 [-inversion-initialization zero|forward] [-inversion-forward-sigma %lf]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [output-image-type | -type s8|u8|s16|u16...]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'image-0' is equal to '-', stdin will be used\n\
 if 'image-1' is not specified or equal to '-', stdin will be used\n\
 if '-trsf' file name is not specified or equal to '-', stdin will be used\n\
 if two over the three are not specified, an error will occur\n\
# ...\n\
 -trsf %s: transformation T(0<-1) that registers image-1 on image-0\n\
 -output-format %s: format 'a la printf' of interpolated images to be processed\n\
                    must contain one '%f' (or '%1.1f' for example)\n\
                    e.g. I_%1.1f.inr\n\
 -n %d: number of transformations to be interpolated at regular intervals (defines t 'set')\n\
# parallelism parameters\n\
 -parallel|-no-parallel:\n\
 -max-chunks %d:\n\
 -parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread:\n\
 -omp-scheduling|-omps default|static|dynamic-one|dynamic|guided:\n\
# specific parameters\n\
 -interpolation nearest|linear|cspline: selection of interpolation mode\n\
 -nearest: nearest neighor interpolation mode (for binary or lable images)\n\
 -linear: bi- or tri-linear interpolation\n\
 -cspline: cubic spline\n\
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
# general image related parameters\n\
   output-image-type: -o 1    : unsigned char\n\
                      -o 2    : unsigned short int\n\
                      -o 2 -s : short int\n\
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





char *API_Help_interpolateImages( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_interpolateImages( char *program, char *str, int flag )
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



void API_InitParam_interpolateImages( lineCmdParamInterpolateImages *p )
{
    (void)strncpy( p->input_name_0, "\0", 1 );
    (void)strncpy( p->input_name_1, "\0", 1 );
    (void)strncpy( p->input_trsf, "\0", 1 );
    (void)strncpy( p->template_name, "\0", 1 );
    (void)strncpy( p->output_format, "\0", 1 );
    p->output_type = TYPE_UNKNOWN;

    p->n_img = 4;

    p->interpolation = LINEAR;

    p->flag_template = 0;

    p->template_dim.x = 0;
    p->template_dim.y = 0;
    p->template_dim.z = 0;

    p->template_voxel.x = 1.0;
    p->template_voxel.y = 1.0;
    p->template_voxel.z = 1.0;


    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_interpolateImages( FILE *theFile, char *program,
                                  lineCmdParamInterpolateImages *p, char *str )
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

  fprintf( f, "- input image 0 is " );
  if ( p->input_name_0 != (char*)NULL && p->input_name_0[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_name_0 );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- input image 1 is " );
  if ( p->input_name_1 != (char*)NULL && p->input_name_1[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_name_1 );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- input transformation is " );
  if ( p->input_trsf != (char*)NULL && p->input_trsf[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_trsf );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- template image is " );
  if ( p->flag_template == 1 )
    fprintf( f, "'%s'\n", p->template_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output format is " );
  if ( p->output_format != (char*)NULL && p->output_format[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_format );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- number of output images is " );
  fprintf( f, "%d\n", p->n_img);

  fprintf( f, "# specific parameter\n" );

  fprintf( f, "- interpolation mode = " );
  switch ( p->interpolation ) {
  default :      fprintf( f, "unknown\n" ); break;
  case NEAREST : fprintf( f, "nearest point\n" ); break;
  case LINEAR :  fprintf( f, "bi- or tri-linear\n" ); break;
  case CSPLINE : fprintf( f, "cubic spline\n" ); break;
  }

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


  fprintf( f, "# general image related parameters\n" );

  fprintf( f, "- output images type = " );
  switch ( p->output_type ) {
  default :     fprintf( f, "TYPE_UNKNOWN\n" ); break;
  case SCHAR :  fprintf( f, "SCHAR\n" ); break;
  case UCHAR :  fprintf( f, "UCHAR\n" ); break;
  case SSHORT : fprintf( f, "SSHORT\n" ); break;
  case USHORT : fprintf( f, "USHORT\n" ); break;
  case UINT :   fprintf( f, "UINT\n" ); break;
  case SINT :   fprintf( f, "INT\n" ); break;
  case ULINT :  fprintf( f, "ULINT\n" ); break;
  case FLOAT :  fprintf( f, "FLOAT\n" ); break;
  case DOUBLE : fprintf( f, "DOUBLE\n" ); break;
  }

  fprintf( f, "==================================================\n" );
}





/************************************************************
 *
 * parameters parsing
 *
 ************************************************************/



static void _API_ParseParam_interpolateImages( char *str, lineCmdParamInterpolateImages *p )
{
  char *proc = "_API_ParseParam_interpolateImages";
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

  API_ParseParam_interpolateImages( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_interpolateImages( int firstargc, int argc, char *argv[],
                                  lineCmdParamInterpolateImages *p )
{
  int i;
  int input0isread = 0;
  int input1isread = 0;
  int trsfisread = 0;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;
  int o=0, s=0, r=0;
  int iterations;
  double error, sigma;



  /* option line parsing
   */
  for ( i=firstargc; i<argc; i++ ) {

      /* strings beginning with '-'
       */
      if ( argv[i][0] == '-' ) {
          if ( argv[i][1] == '\0' ) {
            if ( input0isread == 0 ) {
              (void)strcpy( p->input_name_0,  "<" );  /* standart input */
              input0isread = 1;
            }
            else if ( input1isread == 0 ) {
              (void)strcpy( p->input_name_1,  "<" );  /* standart input */
              input1isread = 1;
            }
            else if ( trsfisread == 0 ) {
              (void)strcpy( p->input_trsf,  "<" );  /* standart input */
              trsfisread = 1;
            }
            else {
              API_ErrorParse_interpolateImages( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          else if ( strcmp ( argv[i], "-output-format" ) == 0 ||
                    strcmp ( argv[i], "-output" ) == 0 ||
                    strcmp ( argv[i], "-format" ) == 0) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -output-format ...\n", 0 );
             (void)strcpy( p->output_format,  argv[i] );
             outputisread = 1;
          }

          else if ( strcmp ( argv[i], "-trsf" ) == 0) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -trsf ...\n", 0 );
             (void)strcpy( p->input_trsf,  argv[i] );
             trsfisread = 1;
          }

          /* specific parameter
           */

          else if ( strcmp ( argv[i], "-n") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -n ...\n", 0 );
              status = sscanf( argv[i], "%d", &p->n_img );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -n ...\n", 0 );
          }

          else if ( strcmp ( argv[i], "-interpolation" ) == 0 ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -interpolation...\n", 0 );
            if ( strcmp ( argv[i], "nearest" ) == 0 ) {
               p->interpolation = NEAREST;
            }
            else if ( strcmp ( argv[i], "linear" ) == 0 ) {
              p->interpolation = LINEAR;
            }
            else if ( strcmp ( argv[i], "cspline" ) == 0 ) {
                p->interpolation = CSPLINE;
            }
            else {
              fprintf( stderr, "unknown interpolation mode: '%s'\n", argv[i] );
              API_ErrorParse_interpolateImages( (char*)NULL, "parsing -interpolation ...\n", 0 );
            }
          }

          else if ( strcmp ( argv[i], "-nearest" ) == 0 ) {
            p->interpolation = NEAREST;
          }
          else if ( strcmp ( argv[i], "-linear" ) == 0 ) {
            p->interpolation = LINEAR;
          }
          else if ( strcmp ( argv[i], "-cspline" ) == 0 ) {
              p->interpolation = CSPLINE;
          }
          else if ( (strcmp ( argv[i], "-template" ) == 0 ) || (strcmp ( argv[i], "-template-image" ) == 0 ) ) {
             p->flag_template = 1;
             i=i+1;
             if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template ...\n", 0 );
             (void)strcpy( p->template_name,  argv[i] );
          }

          /* template
           */

          else if ( strcmp ( argv[i], "-template-dimensions") == 0
                    || strcmp ( argv[i], "-template-dimension") == 0
                    || strcmp ( argv[i], "-template-dim") == 0
                    || strcmp ( argv[i], "-dimensions") == 0
                    || strcmp ( argv[i], "-dimension") == 0
                    || (strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-dimensions %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.x) );
            if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-dimensions %d", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.y) );
            if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
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
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -x ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.x) );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -x ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-y") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -y ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.y) );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -y ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-z") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -z ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -z ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-template-voxel") == 0
                    || strcmp ( argv[i], "-voxel-size") == 0
                    || (strcmp (argv[i], "-voxel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-pixel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-vs" ) == 0 && argv[i][3] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-voxel %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
            if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-voxel %lf", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.y) );
            if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
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
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-error ...\n", 0 );
              status = sscanf( argv[i], "%lf", &error );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-error ...\n", 0 );
              if ( error > 0.0 ) BAL_SetErrorMaxForVectorFieldInversionInBalTransformationTools( error );
          }
          else if ( strcmp ( argv[i], "-inversion-iteration") == 0
                    || strcmp ( argv[i], "-inversion-iterations") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
              status = sscanf( argv[i], "%d", &iterations );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-iterations ...\n", 0 );
              if ( iterations >= 0 ) BAL_SetIterationsMaxForVectorFieldInversionInBalTransformationTools( iterations );
          }
          else if ( strcmp ( argv[i], "-inversion-derivation-sigma") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
              status = sscanf( argv[i], "%lf", &sigma );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-derivation-sigma ...\n", 0 );
              if ( sigma > 0.0 ) BAL_SetDerivationSigmaForVectorFieldInversionInBalTransformationTools( sigma );
          }
          else if ( strcmp ( argv[i], "-inversion-initialization" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-initialization ...\n", 0 );
             if ( strcmp ( argv[i], "zero" ) == 0 ) {
                 BAL_SetInitializationForVectorFieldInversionInBalTransformationTools( ZERO );
             }
             else if ( strcmp ( argv[i], "forward" ) == 0 ) {
                 BAL_SetInitializationForVectorFieldInversionInBalTransformationTools( FORWARD_INTERPOLATION );
             }
             else {
               fprintf( stderr, "unknown initialization type: '%s'\n", argv[i] );
               API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-initialization ...\n", 0 );
             }
          }
          else if ( strcmp ( argv[i], "-inversion-forward-sigma") == 0 ) {
              i++;
              if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
              status = sscanf( argv[i], "%lf", &sigma );
              if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -inversion-forward-sigma ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_interpolateImages( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_interpolateImages( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }

          /* general image related parameters
           */

          else if ( strcmp ( argv[i], "-r" ) == 0 && argv[i][2] == '\0' ) {
             r = 1;
          }
          else if ( strcmp ( argv[i], "-s" ) == 0 && argv[i][2] == '\0' ) {
             s = 1;
          }
          else if ( strcmp ( argv[i], "-o" ) == 0 && argv[i][2] == '\0' ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_interpolateImages( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_interpolateImages( (char*)NULL, "parsing -type...\n", 0 );
            if ( strcmp ( argv[i], "s8" ) == 0 && argv[i][2] == '\0' ) {
               p->output_type = SCHAR;
            }
            else if ( strcmp ( argv[i], "u8" ) == 0 && argv[i][2] == '\0' ) {
               p->output_type = UCHAR;
            }
            else if ( strcmp ( argv[i], "s16" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = SSHORT;
            }
            else if ( strcmp ( argv[i], "u16" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = USHORT;
            }
            else if ( strcmp ( argv[i], "s32" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = SINT;
            }
            else if ( strcmp ( argv[i], "u32" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = UINT;
            }
            else if ( strcmp ( argv[i], "s64" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = SLINT;
            }
            else if ( strcmp ( argv[i], "u64" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = ULINT;
            }
            else if ( strcmp ( argv[i], "r32" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = FLOAT;
            }
            else if ( strcmp ( argv[i], "r64" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = DOUBLE;
            }
            else {
              API_ErrorParse_interpolateImages( (char*)NULL, "parsing -type...\n", 0 );
            }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_interpolateImages( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_interpolateImages( (char*)NULL, (char*)NULL, 0);
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
              API_ErrorParse_interpolateImages( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_interpolateImages( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( input0isread == 0 ) {
              (void)strcpy( p->input_name_0, argv[i] );
              input0isread = 1;
          }
          else if ( input1isread == 0 ) {
              (void)strcpy( p->input_name_1, argv[i] );
              input1isread = 1;
          }
          else if ( trsfisread == 0 ) {
              (void)strcpy( p->input_trsf, argv[i] );
              trsfisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_interpolateImages( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
  if ( input0isread == 0 || (input1isread == 0 && trsfisread == 0)) {
    API_ErrorParse_interpolateImages( (char*)NULL, "missing input image(s) ...\n", 0 );
  }
  if ( input1isread == 0 ) {
    (void)strcpy( p->input_name_1,  "<" );  /* standart input */
    input1isread = 1;
  }
  if ( trsfisread == 0) {
    (void)strcpy( p->input_trsf,  "<" );  /* standart input */
    trsfisread = 1;
  }
  if ( outputisread == 0 ) {
      API_ErrorParse_interpolateImages( (char*)NULL, "missing output format ...\n", 0 );
  }


  /* output image type
   */
  if ( (o != 0) || (s != 0) || (r != 0) ) {
    if ( (o == 1) && (s == 1) && (r == 0) ) p->output_type = SCHAR;
    else if ( (o == 1) && (s == 0) && (r == 0) ) p->output_type = UCHAR;
    else if ( (o == 2) && (s == 0) && (r == 0) ) p->output_type = USHORT;
    else if ( (o == 2) && (s == 1) && (r == 0) ) p->output_type = SSHORT;
    else if ( (o == 4) && (s == 1) && (r == 0) ) p->output_type = SINT;
    else if ( (o == 0) && (s == 0) && (r == 1) ) p->output_type = FLOAT;
    else {
        API_ErrorParse_interpolateImages( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}
