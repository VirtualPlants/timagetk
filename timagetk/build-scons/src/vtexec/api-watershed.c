/*************************************************************************
 * api-watershed.c -
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
#include <vt_common.h>

#include <api-watershed.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_watershed( char *str, lineCmdParamWatershed *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/



int API_watershed( vt_image *gradientImage,
                   vt_image *seedsImage,
                   vt_image *imres,
                   char *param_str_1,
                   char *param_str_2 )
{
  char *proc = "API_watershed";
  lineCmdParamWatershed par;
  vt_image imtmp, *imptr;
  int tmpIsAllocated = 0;
  int theDim[3];
  int m;



  /* parameter initialization
   */
  API_InitParam_watershed( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_watershed( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_watershed( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_watershed( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( imres->type != seedsImage->type ) {
      printf("ici");
      VT_InitFromImage( &imtmp, seedsImage, (char*)NULL, seedsImage->type );
      if ( VT_AllocImage( &imtmp ) != 1 ) {
          if ( _verbose_ )
              fprintf( stderr, "%s: auxiliary image allocation failed\n", proc );
          return( -1 );
      }
      tmpIsAllocated = 1;
      imptr = &imtmp;
  }
  else {
      imptr = imres;
  }

  if ( gradientImage->dim.x != seedsImage->dim.x || gradientImage->dim.x != imres->dim.x
       || gradientImage->dim.y != seedsImage->dim.y || gradientImage->dim.y != imres->dim.y
       || gradientImage->dim.z != seedsImage->dim.z || gradientImage->dim.z != imres->dim.z
       || gradientImage->dim.v != seedsImage->dim.v || gradientImage->dim.v != imres->dim.v ) {
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: images do not have the same dimensions\n", proc );
      return( -1 );
  }


  theDim[0] = seedsImage->dim.x;
  theDim[1] = seedsImage->dim.y;
  theDim[2] = seedsImage->dim.z;


  if ( par.maxIterations >= 0 )
      watershed_setMaxNumberOfIterations( par.maxIterations );

  if ( par.allocatedBunchSize > 0 ) {
    watershed_setNumberOfPointsForAllocation( par.allocatedBunchSize );
  }
  else {
    /* rule of thumb
       nb voxels / (nb gradient levels * 50)
    */
    m = (int)( (double)(theDim[0]*theDim[1]*theDim[2]) / (double)(256*50) );
    if ( m > watershed_getNumberOfPointsForAllocation() ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: bunch of allocated points, change %d for %d\n",
                 proc, watershed_getNumberOfPointsForAllocation(), m );
      }
      watershed_setNumberOfPointsForAllocation( m );
    }
  }

  watershed_setlabelchoice( par.labelChoice );

  if ( watershed( gradientImage->buf, gradientImage->type,
                  seedsImage->buf, imres->buf, seedsImage->type, theDim ) != 1 ) {
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: error when processing\n", proc );
      return( -1 );
  }

  if ( imptr != imres ) {
      if ( VT_CopyImage( imptr, imres ) != 1 ) {
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to copy output image\n", proc );
          return( -1 );
      }
  }

  if ( tmpIsAllocated ) VT_FreeImage( &imtmp );

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



static char *usage = "[[-seeds|-label] image-seeds] [-gradient] image-gradient [image-out]\n\
 [-labelchoice|-l  first|min|most]\n\
 [-max-iterations|-iterations|-i %d]\n\
 [-memory|-m %d]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-inv] [-swap] [output-image-type | -type s8|u8|s16|u16...]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'image-seeds' is equal to '-', stdin will be used\n\
 if 'image-out' is not specified or equal to '-', stdout will be used\n\
 if both are not specified, stdin and stdout will be used\n\
# images\n\
 -seeds|-label image-seeds: seeds image, where each marker should have\n\
    an unique value/label\n\
 -gradient image-gradient: where the labels will be propagated\n\
 image-out: if no output type is given (which is advised) the same type\n\
    than the input image will be used\n\
# watershed controls\n\
 -labelchoice|-l first|min|most: how to deal with conflicts, ie where several\n\
    labels met.\n\
    first: use the first encountered label when parsing the neighborhood\n\
      (historical (and default) behavior)\n\
    min: use the label of minimal value\n\
    most: use the most represented label\n\
 -max-iterations|-iterations|-i %d: maximal number of iterations, allows to\n\
    stop the algorithm before convergence\n\
 -memory|-m %d: memory allocation tuning, kept for historical reason, but should\n\
    not be changed\n\
    setting it low allows a better memory management (at the detriment of speed)\n\
    setting it high allows a better computational times\n\
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





char *API_Help_watershed( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_watershed( char *program, char *str, int flag )
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



void API_InitParam_watershed( lineCmdParamWatershed *p )
{
    (void)strncpy( p->input_seeds_name, "\0", 1 );
    (void)strncpy( p->input_gradient_name, "\0", 1 );
    (void)strncpy( p->output_name, "\0", 1 );
    p->input_inv = 0;
    p->input_swap = 0;
    p->output_type = TYPE_UNKNOWN;

    p->labelChoice = _FIRST_ENCOUNTERED_NEIGHBOR_;
    p->maxIterations = -1;
    p->allocatedBunchSize = -1;

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_watershed( FILE *theFile, char *program,
                                  lineCmdParamWatershed *p, char *str )
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

  fprintf( f, "- input seeds image is " );
  if ( p->input_seeds_name != (char*)NULL && p->input_seeds_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_seeds_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- input gradient image is " );
  if ( p->input_gradient_name != (char*)NULL && p->input_gradient_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_gradient_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output image is " );
  if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_name );
  else
    fprintf( f, "'NULL'\n" );


  fprintf( f, "# watershed controls\n" );

  fprintf( f, "- label choice for conflicts is " );
  switch ( p->labelChoice ) {
  default :     fprintf( f, "unknown, this is embarrassing\n" ); break;
  case _FIRST_ENCOUNTERED_NEIGHBOR_ :  fprintf( f, "first encountered label\n" ); break;
  case _MIN_LABEL_ : fprintf( f, "label of minimal value\n" ); break;
  case _MOST_REPRESENTED_ :     fprintf( f, "most represented label\n" ); break;
  }

  fprintf( f, "- maximal number of iterations = %d\n", p->maxIterations );

  fprintf( f, "- allocated bunch size = %d\n", p->allocatedBunchSize );


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



static void _API_ParseParam_watershed( char *str, lineCmdParamWatershed *p )
{
  char *proc = "_API_ParseParam_watershed";
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

  API_ParseParam_watershed( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_watershed( int firstargc, int argc, char *argv[],
                                  lineCmdParamWatershed *p )
{
  int i;
  int seedsisread = 0;
  int gradientisread = 0;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;
  int o=0, s=0, r=0;



  /* option line parsing
   */
  for ( i=firstargc; i<argc; i++ ) {

      /* strings beginning with '-'
       */
      if ( argv[i][0] == '-' ) {
          if ( argv[i][1] == '\0' ) {
            if ( seedsisread  == 0 ) {
              (void)strcpy( p->input_seeds_name,  "<" );  /* standart input */
              seedsisread  = 1;
            }
            else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name,  ">" );  /* standart output */
              outputisread = 1;
            }
            else {
              API_ErrorParse_watershed( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }


          /* input images
           */
          else if ( (strcmp ( argv[i], "-seeds" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-label" ) == 0 && argv[i][6] == '\0') ) {
              if ( seedsisread  == 0 ) {
                  (void)strcpy( p->input_seeds_name, argv[i] );
                  seedsisread  = 1;
              }
              else
                  API_ErrorParse_watershed( (char*)NULL, "parsing -seeds, seeds image already given ...\n", 0 );
          }
          else if ( (strcmp ( argv[i], "-gradient" ) == 0 && argv[i][9] == '\0') ) {
              if ( gradientisread  == 0 ) {
                  (void)strcpy( p->input_gradient_name, argv[i] );
                  gradientisread  = 1;
              }
              else
                  API_ErrorParse_watershed( (char*)NULL, "parsing -gradient, gradient image already given ...\n", 0 );
          }


          /* watershed controls
           */

          else if ( strcmp ( argv[i], "-labelchoice" ) == 0
                    || ( strcmp ( argv[i], "-l" ) == 0 && argv[i][2] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -labelchoice ...\n", 0 );
            if ( strcmp ( argv[i], "first" ) == 0 ) {
              p->labelChoice = _FIRST_ENCOUNTERED_NEIGHBOR_;
            }
            else if ( strcmp ( argv[i], "min" ) == 0 ) {
              p->labelChoice = _MIN_LABEL_;
            }
            else if ( strcmp ( argv[i], "most" ) == 0 ) {
              p->labelChoice = _MOST_REPRESENTED_;
            }
            else {
                fprintf( stderr, "unknown label choice for conflicts: '%s'\n", argv[i] );
                API_ErrorParse_watershed( (char*)NULL, "parsing -labelchoice ...\n", 0 );
            }
          }

          else if ( strcmp ( argv[i], "-max-iterations" ) == 0
                    || strcmp ( argv[i], "-iterations" ) == 0
                    || ( strcmp ( argv[i], "-i" ) == 0 && argv[i][2] == '\0') ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -max-iterations ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->maxIterations) );
              if ( status <= 0 ) API_ErrorParse_watershed( (char*)NULL, "parsing -max-iterations ...\n", 0 );
          }

          else if ( strcmp ( argv[i], "-memory" ) == 0
                    || ( strcmp ( argv[i], "-m" ) == 0 && argv[i][2] == '\0') ) {
              i ++;
              if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -memory ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->allocatedBunchSize) );
              if ( status <= 0 ) API_ErrorParse_watershed( (char*)NULL, "parsing -memory ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_watershed( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_watershed( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_watershed( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_watershed( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_watershed( (char*)NULL, "parsing -type...\n", 0 );
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
              API_ErrorParse_watershed( (char*)NULL, "parsing -type...\n", 0 );
            }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_watershed( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_watershed( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              incrementVerboseInWatershed( );
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
              else                     _VT_VERBOSE_ ++;
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              setVerboseInWatershed( 0 );
              _verbose_ = 0;
              _VT_VERBOSE_ = 0;
          }
          else if ( (strcmp ( argv[i], "-debug" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0') ) {
              _debug_ = 1;
              _VT_DEBUG_ = 1;
          }
          else if ( (strcmp ( argv[i], "-no-debug" ) == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-nodebug" ) == 0 && argv[i][8] == '\0') ) {
              _debug_ = 0;
              _VT_DEBUG_ = 0;
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
              API_ErrorParse_watershed( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_watershed( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( seedsisread == 0 ) {
              (void)strcpy( p->input_seeds_name, argv[i] );
              seedsisread = 1;
          }
          else if ( gradientisread == 0 ) {
              (void)strcpy( p->input_gradient_name, argv[i] );
              gradientisread = 1;
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name, argv[i]);
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_watershed( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
  if ( seedsisread  == 0 ) {
    (void)strcpy( p->input_seeds_name,  "<" );  /* standart input */
    seedsisread  = 1;
  }
  if ( outputisread == 0 ) {
    (void)strcpy( p->output_name,  ">" );  /* standart output */
    outputisread = 1;
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
        API_ErrorParse_watershed( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}
