/*************************************************************************
 * api-linearFilter.c -
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>
#include <zcross.h>

#include <recbuffer.h>

#include <linearFiltering-common.h>
#include <linearFiltering-contours.h>
#include <linearFiltering.h>

#include <vt_common.h>

#include <api-linearFilter.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_linearFilter( char *str, lineCmdParamLinearFilter *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/



int API_linearFilter( vt_image *image, vt_image *imres, char *param_str_1, char *param_str_2 )
{
  char *proc = "API_linearFilter";
  lineCmdParamLinearFilter par;
  int theDims[3];



  /* parameter initialization
   */
  API_InitParam_linearFilter( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_linearFilter( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_linearFilter( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_linearFilter( stderr, proc, &par, (char*)NULL );

  /* here is the stuff
   */
  theDims[0] = image->dim.x;
  theDims[1] = image->dim.y;
  theDims[2] = image->dim.z;

  switch ( par.typeOutput ) {

  default :
    return( -1 );

  case _FILTER_ :
    if ( par.sliceComputation > 0 )
        par.filter[2].derivative = NODERIVATIVE;
    if ( separableLinearFiltering( image->buf, image->type,
                                       imres->buf, imres->type, theDims,
                                       par.borderLengths, par.filter ) != 1 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to filter input image\n", proc );
      return( -1 );
    }
    break;

  case _GRADIENT_MODULUS_ :
    if ( par.sliceComputation ) {
      if ( gradientModulus2D( image->buf, image->type,
                                 imres->buf, imres->type, theDims,
                                 par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute 2D gradient modulus\n", proc );
          return( -1 );
      }
    }
    else {
      if ( gradientModulus( image->buf, image->type,
                               imres->buf, imres->type, theDims,
                               par.borderLengths, par.filter ) != 1 ) {

          if ( _verbose_ )
              fprintf( stderr, "%s: unable to compute gradient modulus\n", proc );
          return( -1 );
      }
    }
    break;

  case _HESSIAN_ :
    if ( par.sliceComputation ) {
      if ( gradientHessianGradient2D( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute 2D gradient.Hessian.gradient\n", proc );
          return( -1 );
      }
    }
    else {
      if ( gradientHessianGradient( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute gradient.Hessian.gradient\n", proc );
          return( -1 );
      }
    }
    break;

  case _LAPLACIAN_ :
    if ( par.sliceComputation ) {
      if ( laplacian2D( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute 2D laplacian\n", proc );
          return( -1 );
      }
    }
    else {
      if ( laplacian( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute laplacian\n", proc );
          return( -1 );
      }
    }
    break;

  case _ZCROSSINGS_HESSIAN_ :
    if ( par.sliceComputation ) {
      if ( gradientHessianGradientZeroCrossings2D( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute 2D zero-crossings of gradient.Hessian.gradient\n", proc );
          return( -1 );
      }
    }
    else {
      if ( gradientHessianGradientZeroCrossings( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute zero-crossings of gradient.Hessian.gradient\n", proc );
          return( -1 );
      }
    }
    break;

  case _ZCROSSINGS_LAPLACIAN_ :
    if ( par.sliceComputation ) {
      if ( laplacianZeroCrossings2D( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute 2D zero-crossings of laplacian\n", proc );
          return( -1 );
      }
    }
    else {
      if ( laplacianZeroCrossings( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute zero-crossings of laplacian\n", proc );
          return( -1 );
      }
    }
    break;

  case _GRADIENT_HESSIAN_ :
    if ( par.sliceComputation ) {
      if ( gradientOnGradientHessianGradientZC2D( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute gradient on 2D zero-crossings of gradient.Hessian.gradient\n", proc );
          return( -1 );
      }
    }
    else {
      if ( gradientOnGradientHessianGradientZC( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute gradient on zero-crossings of gradient.Hessian.gradient\n", proc );
          return( -1 );
      }
    }
    break;

  case _GRADIENT_LAPLACIAN_ :
    if ( par.sliceComputation ) {
      if ( gradientOnLaplacianZC2D( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute gradient on 2D zero-crossings of laplacian\n", proc );
          return( -1 );
      }
    }
    else {
      if ( gradientOnLaplacianZC( image->buf, image->type,
                                         imres->buf, imres->type, theDims,
                                         par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute gradient on zero-crossings of laplacian\n", proc );
          return( -1 );
      }
    }
    break;

  case _EXTREMA_GRADIENT_ :
    if ( par.sliceComputation ) {
      if ( gradientMaxima2D( image->buf, image->type,
                                imres->buf, imres->type, theDims,
                                par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute 2D extrema of the gradient\n", proc );
          return( -1 );
      }
    }
    else {
      if ( gradientMaxima( image->buf, image->type,
                              imres->buf, imres->type, theDims,
                              par.borderLengths, par.filter ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to compute extrema of the gradient\n", proc );
          return( -1 );
      }
    }
    break;
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



static char *usage = "[image-in] [image-out] [-x %d] [-y %d] [-z %d]\n\
 [-filter | -smoothing | -gradient | -gradient-modulus |-hessian | ...\n\
    ... | -laplacian | -zero-crossings-hessian | -zero-crossings-laplacian | ...\n\
    ... | -gradient-hessian | -gradient-laplacian | -gradient-extrema]\n\
 [-gaussian-type|-type [deriche|fidrich|young-1995|young-2002| ...\n\
    ... |gabor-young-2002|convolution]]\n\
 [-sigma %lf [%lf [%lf]]] [-alpha %lf [%lf [%lf]]] \n\
 [-border|cont %d [%d [%d]]] [-neg | -pos] [-2D] [-edges]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-inv] [-swap] [output-image-type | -type s8|u8|s16|u16...]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'image-in' is equal to '-', stdin will be used\n\
 if 'image-out' is not specified or equal to '-', stdout will be used\n\
 if both are not specified, stdin and stdout will be used\n\
# type of output\n\
 -filter (default): apply separable filtering (specified with -x, -y, or -z)\n\
                    along each axis\n\
 -x %d | -y %d | -z %d: derivation order along each axis (default is none)\n\
 -smoothing: equivalent to '-filter -x 0 -y 0 -z 0'\n\
 -gradient | -gradient-modulus: ||grad(I)||\n\
 -hessian: grad(I).H(I).grad(I)\n\
 -laplacian:\n\
 -zero-crossings-hessian:\n\
 -zero-crossings-laplacian:\n\
 -gradient-hessian: gradient modulus onto zero-crossings of hessian image\n\
 -gradient-laplacian: gradient modulus onto zero-crossings of laplacian image\n\
 -gradient-extrema:\n\
# filter definition\n\
 -gaussian-type deriche|fidrich|young-1995|young-2002|gabor-young-2002|convolution\n\
           implementation type (default is young-2002)\n\
           deriche|fidrich|young-1995|young-2002|gabor-young-2002 are recursive filters\n\
           convolution: convolution with a truncated gaussian mask\n\
 -sigma %lf [%lf [%lf]]]: sigma of the gaussian for each axis (default is 1.0)\n\
 -alpha %lf [%lf [%lf]]]: alpha of the Deriche filter  for each axis\n\
           set also the filter type to the Deriche filter instead of the gaussian one\n\
# other calculation parameters\n\
 -border|cont %d [%d [%d]]: repeat first and last values in 1D filtering to\n\
         avoid cut-off effects\n\
 -neg: zero-crossings are set on the negative values\n\
 -pos: zero-crossings are set on the positive values\n\
 -2D: 2D computation of XY slices in a 3D volume\n\
 -edges: edge-oriented normalization of first order derivative filters\n\
# parallelism parameters\n\
 -parallel|-no-parallel:\n\
 -max-chunks %d:\n\
 -parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread:\n\
 -omp-scheduling|-omps default|static|dynamic-one|dynamic|guided:\n\
# general image related parameters\n\
  -inv: inverse 'image-in'\n\
  -swap: swap 'image-in' (if encoded on 2 or 4 bytes)\n\
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





char *API_Help_linearFilter( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_linearFilter( char *program, char *str, int flag )
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



void API_InitParam_linearFilter( lineCmdParamLinearFilter *p )
{
    (void)strncpy( p->input_name, "\0", 1 );
    (void)strncpy( p->output_name, "\0", 1 );
    p->input_inv = 0;
    p->input_swap = 0;
    p->output_type = TYPE_UNKNOWN;

    p->typeOutput = _FILTER_;
    initFilteringCoefficients( &(p->filter[0]) );
    initFilteringCoefficients( &(p->filter[1]) );
    initFilteringCoefficients( &(p->filter[2]) );
    p->borderLengths[0] = 0;
    p->borderLengths[1] = 0;
    p->borderLengths[2] = 0;
    p->sliceComputation = 0;

    p->print_lineCmdParam = 0;

    p->print_time = 0;
}





void API_PrintParam_linearFilter( FILE *theFile, char *program,
                                  lineCmdParamLinearFilter *p, char *str )
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

  fprintf( f, "- input image is " );
  if ( p->input_name != (char*)NULL && p->input_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output image is " );
  if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_name );
  else
    fprintf( f, "'NULL'\n" );


  fprintf( f, "# output type\n" );

  fprintf( f, "- computation: " );
  switch ( p->typeOutput ) {
  default : fprintf( f, "unknown\n"); break;
  case _FILTER_ : fprintf( f, "_FILTER_\n"); break;
  case _GRADIENT_MODULUS_ : fprintf( f, "_GRADIENT_MODULUS_\n"); break;
  case _HESSIAN_ : fprintf( f, "_HESSIAN_\n"); break;
  case _LAPLACIAN_ : fprintf( f, "_LAPLACIAN_\n"); break;
  case _ZCROSSINGS_HESSIAN_ : fprintf( f, "_ZCROSSINGS_HESSIAN_\n"); break;
  case _ZCROSSINGS_LAPLACIAN_ : fprintf( f, "_ZCROSSINGS_LAPLACIAN_\n"); break;
  case _GRADIENT_HESSIAN_ : fprintf( f, "_GRADIENT_HESSIAN_\n"); break;
  case _GRADIENT_LAPLACIAN_ : fprintf( f, "_GRADIENT_LAPLACIAN_\n"); break;
  case _EXTREMA_GRADIENT_ : fprintf( f, "_EXTREMA_GRADIENT_\n"); break;
  }


  fprintf( f, "# filter definition\n" );

  printFilteringCoefficients( f, &(p->filter[0]), "filter along X" );
  printFilteringCoefficients( f, &(p->filter[1]), "filter along Y" );
  printFilteringCoefficients( f, &(p->filter[2]), "filter along Z" );


  fprintf( f, "# other calculation parameters\n" );

  fprintf( f, "- borders = [%d %d %d]\n",  p->borderLengths[0],
            p->borderLengths[1], p->borderLengths[2] );
  fprintf( f, "- slice computation = %d\n", p->sliceComputation );


  fprintf( f, "# general image related parameters\n" );

  fprintf( f, "- input image inverse = %d\n", p->input_inv );
  fprintf( f, "- input image swap = %d\n", p->input_swap );
  fprintf( f, "- output image type = " );
  switch ( p->output_type ) {
  default :     fprintf( f, "TYPE_UNKNOWN\n" ); break;
  case SCHAR :  fprintf( f, "SCHAR\n" ); break;
  case UCHAR :  fprintf( f, "UCHAR\n" ); break;
  case SSHORT : fprintf( f, "SSHORT\n" ); break;
  case USHORT : fprintf( f, "USHORT\n" ); break;
  case UINT :   fprintf( f, "UINT\n" ); break;
  case SINT :    fprintf( f, "INT\n" ); break;
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



static void _API_ParseParam_linearFilter( char *str, lineCmdParamLinearFilter *p )
{
  char *proc = "_API_ParseParam_linearFilter";
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

  API_ParseParam_linearFilter( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_linearFilter( int firstargc, int argc, char *argv[],
                                  lineCmdParamLinearFilter *p )
{
  int i, j;
  int inputisread = 0;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status, tmp;
  int bool_contours = 0;
  int maxchunks;
  int o=0, s=0, r=0;



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
              API_ErrorParse_linearFilter( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* type of output
           */
          else if ( strcmp ( argv[i], "-filter" ) == 0 ) {
             p->typeOutput = _FILTER_;
          }
          else if ( (strcmp ( argv[i], "-smoothing" ) == 0 && argv[i][10] == '\0') ) {
             p->typeOutput = _FILTER_;
             p->filter[0].derivative = DERIVATIVE_0;
             p->filter[1].derivative = DERIVATIVE_0;
             p->filter[2].derivative = ( p->sliceComputation > 0 ) ? NODERIVATIVE : DERIVATIVE_0;
          }
          else if ( (strcmp ( argv[i], "-gradient" ) == 0 && argv[i][9] == '\0')
                      || strcmp ( argv[i], "-gradient-modulus" ) == 0 ) {
             p->typeOutput = _GRADIENT_MODULUS_;
          }
          else if ( strcmp ( argv[i], "-hessian" ) == 0 ) {
             p->typeOutput = _HESSIAN_;
          }
          else if ( strcmp ( argv[i], "-laplacian" ) == 0 ) {
             p->typeOutput = _LAPLACIAN_;
          }
          else if ( strcmp ( argv[i], "-zero-crossings-hessian" ) == 0
                      || strcmp ( argv[i], "-zcrossings-hessian" ) == 0 ) {
             p->typeOutput = _ZCROSSINGS_HESSIAN_;
          }
          else if ( strcmp ( argv[i], "-zero-crossings-laplacian" ) == 0
                      || strcmp ( argv[i], "-zcrossings-laplacian" ) == 0 ) {
             p->typeOutput = _ZCROSSINGS_LAPLACIAN_;
          }
          else if ( strcmp ( argv[i], "-gradient-hessian" ) == 0 ) {
             p->typeOutput = _GRADIENT_HESSIAN_;
          }
          else if ( strcmp ( argv[i], "-gradient-laplacian" ) == 0 ) {
             p->typeOutput = _GRADIENT_LAPLACIAN_;
          }
          else if ( strcmp ( argv[i], "-gradient-extrema" ) == 0 ) {
             p->typeOutput = _EXTREMA_GRADIENT_;
          }

          else if ( strcmp ( argv[i], "-x" ) == 0 && argv[i][2] == '\0' ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -x...\n", 0 );
             status = sscanf( argv[i],"%d",&tmp );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing -x...\n", 0 );
             switch ( tmp ) {
             default :
               p->filter[0].derivative = NODERIVATIVE;   break;
             case 0 :
               p->filter[0].derivative = DERIVATIVE_0;   break;
             case 1 :
               p->filter[0].derivative = DERIVATIVE_1;   break;
             case 2 :
               p->filter[0].derivative = DERIVATIVE_2;   break;
             case 3 :
               p->filter[0].derivative = DERIVATIVE_3;   break;
             }
          }

          else if ( strcmp ( argv[i], "-y" ) == 0 && argv[i][2] == '\0' ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -y...\n", 0 );
             status = sscanf( argv[i],"%d",&tmp );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing -y...\n", 0 );
             switch ( tmp ) {
             default :
               p->filter[1].derivative = NODERIVATIVE;   break;
             case 0 :
               p->filter[1].derivative = DERIVATIVE_0;   break;
             case 1 :
               p->filter[1].derivative = DERIVATIVE_1;   break;
             case 2 :
               p->filter[1].derivative = DERIVATIVE_2;   break;
             case 3 :
               p->filter[1].derivative = DERIVATIVE_3;   break;
             }
          }

          else if ( strcmp ( argv[i], "-z" ) == 0 && argv[i][2] == '\0' ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -z...\n", 0 );
             status = sscanf( argv[i],"%d",&tmp );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing -z...\n", 0 );
             switch ( tmp ) {
             default :
               p->filter[2].derivative = NODERIVATIVE;   break;
             case 0 :
               p->filter[2].derivative = DERIVATIVE_0;   break;
             case 1 :
               p->filter[2].derivative = DERIVATIVE_1;   break;
             case 2 :
               p->filter[2].derivative = DERIVATIVE_2;   break;
             case 3 :
               p->filter[2].derivative = DERIVATIVE_3;   break;
             }
          }

          /* filter definition
           */
          else if ( strcmp ( argv[i], "-type" ) == 0
                      || strcmp ( argv[i], "-gaussian-type" ) == 0 ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -gaussian-type...\n", 0 );
             if ( strcmp ( argv[i], "deriche" ) == 0 ) {
               p->filter[0].type = GAUSSIAN_DERICHE;
               p->filter[1].type = GAUSSIAN_DERICHE;
               p->filter[2].type = GAUSSIAN_DERICHE;
             }
             else if ( strcmp ( argv[i], "fidrich" ) == 0 ) {
               p->filter[0].type = GAUSSIAN_FIDRICH;
               p->filter[1].type = GAUSSIAN_FIDRICH;
               p->filter[2].type = GAUSSIAN_FIDRICH;
             }
             else if ( strcmp ( argv[i], "young-1995" ) == 0 ) {
               p->filter[0].type = GAUSSIAN_YOUNG_1995;
               p->filter[1].type = GAUSSIAN_YOUNG_1995;
               p->filter[2].type = GAUSSIAN_YOUNG_1995;
             }
             else if ( strcmp ( argv[i], "young-2002" ) == 0 ) {
               p->filter[0].type = GAUSSIAN_YOUNG_2002;
               p->filter[1].type = GAUSSIAN_YOUNG_2002;
               p->filter[2].type = GAUSSIAN_YOUNG_2002;
             }
             else if ( strcmp ( argv[i], "gabor-young-2002" ) == 0 ) {
               p->filter[0].type = GABOR_YOUNG_2002;
               p->filter[1].type = GABOR_YOUNG_2002;
               p->filter[2].type = GABOR_YOUNG_2002;
             }
             else if ( strcmp ( argv[i], "convolution" ) == 0  ) {
               p->filter[0].type = GAUSSIAN_CONVOLUTION;
               p->filter[1].type = GAUSSIAN_CONVOLUTION;
               p->filter[2].type = GAUSSIAN_CONVOLUTION;
             }
             else {
               API_ErrorParse_linearFilter( (char*)NULL, "parsing -gaussian-type...\n", 0 );
             }
          }

          else if ( (strcmp ( argv[i], "-sigma" ) == 0) ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -sigma ...\n", 0 );
             status = sscanf( argv[i],"%lf",&(p->filter[0].coefficient) );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing -sigma ...\n", 0 );
             i ++;
             if ( i >= argc) {
               p->filter[1].coefficient = p->filter[0].coefficient;
               p->filter[2].coefficient = p->filter[0].coefficient;
             }
             else {
               status = sscanf( argv[i], "%lf", &(p->filter[1].coefficient) );
               if ( status <= 0 ) {
                 i--;
                 p->filter[1].coefficient = p->filter[0].coefficient;
                 p->filter[2].coefficient = p->filter[0].coefficient;
               }
               else {
                 i ++;
                 if ( i >= argc) p->filter[2].coefficient = 0.0;
                 else {
                   status = sscanf( argv[i], "%lf", &(p->filter[2].coefficient) );
                   if ( status <= 0 ) {
                      i--;
                      p->filter[2].coefficient = 0;
                   }
                 }
               }
             }
             for ( j=0; j<3; j++ ) {
               switch ( p->filter[j].type ) {
               case GAUSSIAN_DERICHE :
               case GAUSSIAN_FIDRICH :
               case GAUSSIAN_YOUNG_1995 :
               case GAUSSIAN_YOUNG_2002 :
               case GABOR_YOUNG_2002 :
               case GAUSSIAN_CONVOLUTION :
                 break;
               default :
                 p->filter[j].type = GAUSSIAN_CONVOLUTION;
                 break;
               }
             }
          }

          else if ( (strcmp ( argv[i], "-alpha" ) == 0) ||
                      (strcmp ( argv[i], "-a" ) == 0 && argv[i][2] == '\0') ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing [-alpha|-a]...\n", 0 );
             status = sscanf( argv[i],"%lf",&(p->filter[0].coefficient) );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing [-alpha|-a]...\n", 0 );
             i ++;
             if ( i >= argc) {
               p->filter[1].coefficient = p->filter[0].coefficient;
               p->filter[2].coefficient = p->filter[0].coefficient;
             }
             else {
               status = sscanf( argv[i], "%lf", &(p->filter[1].coefficient) );
               if ( status <= 0 ) {
                 i--;
                 p->filter[1].coefficient = p->filter[0].coefficient;
                 p->filter[2].coefficient = p->filter[0].coefficient;
               }
               else {
                 i ++;
                 if ( i >= argc) p->filter[2].coefficient = 0.0;
                 else {
                   status = sscanf( argv[i], "%lf", &(p->filter[2].coefficient) );
                   if ( status <= 0 ) {
                      i--;
                      p->filter[2].coefficient = 0;
                   }
                 }
               }
             }
             p->filter[0].type = ALPHA_DERICHE;
             p->filter[1].type = ALPHA_DERICHE;
             p->filter[2].type = ALPHA_DERICHE;
          }

          /* other calculation parameters
           */
          else if ( strcmp ( argv[i], "-cont" ) == 0
                      || strcmp ( argv[i], "-border" ) == 0) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -cont...\n", 0 );
             status = sscanf( argv[i],"%d",&(p->borderLengths[0]) );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing -cont...\n", 0 );
             i ++;
             if ( i >= argc) {
               p->borderLengths[2] = p->borderLengths[1] = p->borderLengths[0];
             }
             else {
               status = sscanf( argv[i], "%d", &(p->borderLengths[1]) );
               if ( status <= 0 ) {
                 i--;
                 p->borderLengths[2] = p->borderLengths[1] = p->borderLengths[0];
               }
               else {
                 i ++;
                 if ( i >= argc) p->borderLengths[2] = 0;
                 else {
                   status = sscanf( argv[i], "%d", &(p->borderLengths[2]) );
                   if ( status <= 0 ) {
                      i--;
                      p->borderLengths[2] = 0;
                   }
                 }
               }
             }
          }

          else if ( strcmp ( argv[i], "-neg" ) == 0 && argv[i][4] == '\0' ) {
             ZeroCrossings_Are_Negative();
          }
          else if ( strcmp ( argv[i], "-pos" ) == 0 && argv[i][4] == '\0' ) {
             ZeroCrossings_Are_Positive();
          }

          else if ( strcmp ( argv[i], "-2D" ) == 0 && argv[i][3] == '\0' ) {
             p->sliceComputation = 1;
          }

          else if ( strcmp ( argv[i], "-edges" ) == 0 ) {
             bool_contours = 1;
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
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_linearFilter( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_linearFilter( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }

          /* general image related parameters
           */
          else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
             p->input_inv = 1;
          }
          else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
             p->input_swap = 1;
          }

          else if ( strcmp ( argv[i], "-r" ) == 0 && argv[i][2] == '\0' ) {
             r = 1;
          }
          else if ( strcmp ( argv[i], "-s" ) == 0 && argv[i][2] == '\0' ) {
             s = 1;
          }
          else if ( strcmp ( argv[i], "-o" ) == 0 && argv[i][2] == '\0' ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_linearFilter( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_linearFilter( (char*)NULL, "parsing -type...\n", 0 );
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
              API_ErrorParse_linearFilter( (char*)NULL, "parsing -type...\n", 0 );
            }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_linearFilter( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_linearFilter( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
              else                     _VT_VERBOSE_ ++;
              incrementVerboseInLinearFilteringCommon( );
              incrementVerboseInLinearFilteringContours( );
              incrementVerboseInLinearFiltering( );
              incrementVerboseInRecBuffer( );
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              _verbose_ = 0;
              _VT_VERBOSE_ = 0;
              setVerboseInLinearFilteringCommon( 0 );
              setVerboseInLinearFilteringContours( 0 );
              setVerboseInLinearFiltering( 0 );
              setVerboseInRecBuffer( 0 );
          }
          else if ( (strcmp ( argv[i], "-debug" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0') ) {
              _debug_ = 1;
              _VT_DEBUG_ = 1;
              incrementDebugInLinearFiltering( );
          }
          else if ( (strcmp ( argv[i], "-no-debug" ) == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-nodebug" ) == 0 && argv[i][8] == '\0') ) {
              _debug_ = 0;
              _VT_DEBUG_ = 0;
              setDebugInLinearFiltering( 0 );
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
              API_ErrorParse_linearFilter( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_linearFilter( (char*)NULL, "too long file name ...\n", 0 );
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
              API_ErrorParse_linearFilter( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
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


  /* edge-oriented first order derivative filters
   */
  if ( bool_contours == 1 ) {
    if ( p->filter[0].derivative == DERIVATIVE_1 )
         p->filter[0].derivative =  DERIVATIVE_1_CONTOURS;
    if ( p->filter[1].derivative == DERIVATIVE_1 )
         p->filter[1].derivative =  DERIVATIVE_1_CONTOURS;
    if ( p->filter[2].derivative == DERIVATIVE_1 )
         p->filter[2].derivative =  DERIVATIVE_1_CONTOURS;
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
        API_ErrorParse_linearFilter( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}
