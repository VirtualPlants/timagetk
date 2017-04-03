/*************************************************************************
 * api-connexe.c -
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


#include <api-connexe.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_connexe( char *str, lineCmdParamConnexe *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/

/* compute connected components from 'image'
 * if 'imseeds' is given (!= NULL), compute seeded connected
 * components (ie connected components that contain foreground
 * of 'imseeds')
 */

int API_connexe( vt_image *image,
                 vt_image *imseeds,
                 vt_image *imres,
                 char *param_str_1, char *param_str_2 )
{
  char *proc = "API_connexe";
  lineCmdParamConnexe par;

  vt_image imtmp;
  vt_image *imptr;
  int r;
  vt_3m m;


  /* parameter initialization
   */
  API_InitParam_connexe( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_connexe( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_connexe( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_connexe( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/


  if ( imres->type != USHORT ) {
      VT_Image( &imtmp );
      VT_InitFromImage( &imtmp, image, (char*)NULL, USHORT );
      if ( VT_AllocImage( &imtmp ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc );
          return( -1 );
      }
      imptr = &imtmp;
  }
  else {
      imptr = imres;
  }

  /* seeded connected component extraction
   * seeds from image
   */
  if ( imseeds != (vt_image*)NULL ) {

      if ( imseeds->type != UCHAR ) {
          if ( imres->type != USHORT ) VT_FreeImage( &imtmp );
          if ( _verbose_ )
             fprintf( stderr, "%s: seeds image should be of unsigned char type\n", proc );
          return( -1 );
      }

      r = VT_ConnectedComponentsWithSeeds( image, imptr, par.low_threshold, imseeds, &(par.cpar) );

      switch( r ) {
      case 1 :
          break;
      case -2 :
          if ( _verbose_ )
              fprintf( stderr, "%s: warning, no connected component was found\n", proc );
          VT_FillImage( imptr, 0.0 );
          break;
      default :
          if ( imres->type != USHORT ) VT_FreeImage( &imtmp );
          if ( _verbose_ )
              fprintf( stderr, "%s: warning, something went wrong\n", proc );
          return( -1 );
      }
  }

  /* connected component extraction
   * other cases
   */
  else {

      switch( par.typeComputation ) {
      case  _VT_CONNECTED:
          r = VT_ConnectedComponents( image, imptr, par.low_threshold, &(par.cpar) );
          break;
      case _VT_HYSTERESIS :
          r = VT_Hysteresis( image, imptr, par.low_threshold, par.high_threshold, &(par.cpar) );
          break;
      case _VT_SEEDPT :
          r = VT_ConnectedComponentWithOneSeed( image, imptr, par.low_threshold, &(par.seed), &(par.cpar) );
          break;
      case _VT_SEEDSIM :
          if ( imres->type != USHORT ) VT_FreeImage( &imtmp );
          if ( _verbose_ )
             fprintf( stderr, "%s: weird, this case should not occur\n", proc );
          return( -1 );
          break;
      default :
          if ( imres->type != USHORT ) VT_FreeImage( &imtmp );
          if ( _verbose_ )
             fprintf( stderr, "%s: embarrassing, this case has not been considered\n", proc );
          return( -1 );
      }

      switch( r ) {
      case 1 :
          break;
      case -2 :
          if ( par.typeComputation == _VT_SEEDPT ) {
              if ( _verbose_ )
                  fprintf( stderr, "%s: warning, no connected component was found\n", proc );
              VT_FillImage( imptr, 0.0 );
              break;
          }
      default :
          if ( imres->type != USHORT ) VT_FreeImage( &imtmp );
          if ( _verbose_ )
              fprintf( stderr, "%s: warning, something went wrong\n", proc );
          return( -1 );
      }
  }

  if ( imres->type != USHORT ) {
      switch( par.cpar.type_output ) {
      default :
          VT_FreeImage( &imtmp );
          if ( _verbose_ )
              fprintf( stderr, "%s: warning, something went wrong\n", proc );
          return( -1 );
      case VT_BINAIRE :
          if ( VT_Threshold( &imtmp, imres, (float)1.0 ) != 1 ) {
              VT_FreeImage( &imtmp );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error when thresholding auxiliary image\n", proc );
              return( -1 );
          }
          break;
      case VT_GREY :
      case VT_SIZE :
          if ( VT_3m( &imtmp, (vt_image*)NULL, &m ) == -1 ) {
              VT_FreeImage( &imtmp );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error when computing statistics of auxiliary image\n", proc );
              return( -1 );
          }
          switch ( imres->type ) {
          case SCHAR :
              if ( (int)(m.max + 0.5) > 127 ) {
                  if ( _verbose_ )
                      fprintf( stderr, "%s: warning, overflow will occur in result image values\n", proc );
              }
              break;
          case UCHAR :
              if ( (int)(m.max + 0.5) > 255 ) {
                  if ( _verbose_ )
                      fprintf( stderr, "%s: warning, overflow will occur in result image values\n", proc );
              }
              break;
          case SSHORT :
              if ( (int)(m.max + 0.5) > 32767 ) {
                  if ( _verbose_ )
                      fprintf( stderr, "%s: warning, overflow will occur in result image values\n", proc );
              }
              break;
          case USHORT :
              if ( (int)(m.max + 0.5) > 65535 ) {
                  if ( _verbose_ )
                      fprintf( stderr, "%s: warning, overflow will occur in result image values\n", proc );
              }
              break;
          default :
              break;
          }
          if ( VT_CopyImage( &imtmp, imres  ) != 1 ) {
              VT_FreeImage( &imtmp );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error when copying auxiliary image\n", proc );
              return( -1 );
          }
          break;
      }
      VT_FreeImage( &imtmp );
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



static char *usage = "[image-in] [image-out]\n\
 [-seed-image|-seeds %s]\n\
 [-low-threshold|-lt|-sb %f] [-high-threshold|-ht|-sh %f]\n\
 [-min-size-cc|-scc|-tcc %d]\n\
 [-max-number-cc|-ncc %d | -max]\n\
 [-connectivity|-con %d]\n\
 [-binary-output|-bin] [-label-output|-label]\n\
 [-size-output|-size] [-sort-sizes|-sort]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-2D]\n\
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
# computation related parameters\n\
 -seed-image|-seeds %s: seeds image for seeded connected component extraction\n\
    implies binary output (unless changed afterwards)\n\
 -seed-point|-seed %d %d [%d]: seed point for seeded connected component extraction\n\
    implies binary output (unless changed afterwards)\n\
 -low-threshold|-lt|-sb %f: low threshold to binarize input image\n\
 -high-threshold|-ht|-sh %f: high threshold for hysteresys thresholding\n\
   implies binary output (unless changed afterwards)\n\
# connected components extraction parameters\n\
 -min-size-cc|-scc|-tcc %d: minimal size of connected components\n\
 -max-number-cc|-ncc %d: maximal number  of connected components\n\
  the connected components of larger size are kept\n\
 -max: keep the largest connected component (equivalent to '-max-number-cc 1')\n\
    implies binary output (unless changed afterwards)\n\
 -connectivity|-con %d: connectivity (4, 8, 6, 10, 18 or 26 (default))\n\
 -binary-output|-bin: all valid connected components have the same value\n\
 -label-output|-label: one label per connected component\n\
 -size-output|-size: the value attributed to points of a connected component is\n\
    the size of the connected components (allows to select w.r.t to size afterwards)\n\
 -sort-sizes|-sort: the labels ordered the connected components with decreasing size\n\
    implies -label-output (unless changed afterwards)\n\
 -2D: slice by slice computation; each XY slice of a 3D volume is processed as an\n\
    independent 2D image (eg labels restarted at 1 for each slice)\n\
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





char *API_Help_connexe( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_connexe( char *program, char *str, int flag )
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



void API_InitParam_connexe( lineCmdParamConnexe *p )
{
    (void)strncpy( p->input_name, "\0", 1 );
    (void)strncpy( p->output_name, "\0", 1 );
    p->input_inv = 0;
    p->input_swap = 0;
    p->output_type = TYPE_UNKNOWN;

    p->typeComputation = _VT_CONNECTED;

    (void)strncpy( p->input_seeds_name, "\0", 1 );
    p->seed.x = p->seed.y = p->seed.z = -1;

    p->low_threshold = (float)1.0;
    p->high_threshold = (float)1.0;

    VT_Connexe( &(p->cpar) );

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_connexe( FILE *theFile, char *program,
                                  lineCmdParamConnexe *p, char *str )
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



  fprintf( f, "# computation related parameters\n" );

  fprintf( f, "- computation type is " );
  switch ( p->typeComputation ) {
  default :     fprintf( f, "unknown, this is embarrassing\n" ); break;
  case _VT_CONNECTED :  fprintf( f, "connected component extraction\n" ); break;
  case _VT_HYSTERESIS : fprintf( f, "hysteresis thresholding\n" ); break;
  case _VT_SEEDPT :     fprintf( f, "seeded connected component extraction with a point\n" ); break;
  case _VT_SEEDSIM :    fprintf( f, "seeded connected component extraction from an image\n" ); break;
  }

  fprintf( f, "- input seeds image is " );
  if ( p->input_seeds_name != (char*)NULL && p->input_seeds_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_seeds_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- seed point if [%d,%d,%d]\n", p->seed.x, p->seed.y, p->seed.z );

  fprintf( f, "- low threshold = %f\n", p->low_threshold );
  fprintf( f, "- high threshold = %f\n", p->high_threshold );



  fprintf( f, "# connected components extraction parameters\n" );
  fprintf( f, "- minimal size of connected components = %d\n", p->cpar.min_size );
  fprintf( f, "- maximal number of connected components = %d\n", p->cpar.max_nbcc );
  fprintf( f, "- connectivity is " );
  switch ( p->cpar.type_connexite ) {
  default :     fprintf( f, "unknown, this is embarrassing\n" ); break;
  case N04 : fprintf( f, " 4\n" ); break;
  case N06 : fprintf( f, " 6\n" ); break;
  case N08 : fprintf( f, " 8\n" ); break;
  case N10 : fprintf( f, "10\n" ); break;
  case N18 : fprintf( f, "18\n" ); break;
  case N26 : fprintf( f, "26\n" ); break;
  }
  fprintf( f, "- output type is " );
  switch ( p->cpar.type_output ) {
  default :     fprintf( f, "unknown, this is embarrassing\n" ); break;
  case VT_BINAIRE : fprintf( f, "binary image\n" ); break;
  case VT_GREY    : fprintf( f, "connected component labels\n" ); break;
  case VT_SIZE    : fprintf( f, "connected component sizes\n" ); break;
  }
  fprintf( f, "- slice by slice computation is " );
  if ( p->cpar.dim == VT_2D ) {
      fprintf( f, "1\n" );
  }
  else {
      fprintf( f, "0\n" );
  }
  fprintf( f, "- verbose is %d\n", p->cpar.verbose );
  fprintf( f, "- threshold is %d\n", p->cpar.threshold );


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



static void _API_ParseParam_connexe( char *str, lineCmdParamConnexe *p )
{
  char *proc = "_API_ParseParam_connexe";
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

  API_ParseParam_connexe( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_connexe( int firstargc, int argc, char *argv[],
                                  lineCmdParamConnexe *p )
{
  int i;
  int inputisread = 0;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;
  int o=0, s=0, r=0;
  int connectivity;



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
              API_ErrorParse_connexe( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* computation related parameters
           */

          else if ( strcmp ( argv[i], "-seed-image" ) == 0
                    || ( strcmp ( argv[i], "-seeds" ) == 0 && argv[i][6] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -seed-image ...\n", 0 );
            strcpy( p->input_seeds_name, argv[i] );
            p->typeComputation = _VT_SEEDSIM;
            p->cpar.type_output = VT_BINAIRE;
          }
          else if ( strcmp ( argv[i], "-seed-point" ) == 0
                    || ( strcmp ( argv[i], "-seed" ) == 0 && argv[i][5] == '\0') ) {
              i += 1;
              if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -seed-point ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->seed.x) );
              if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -seed-point ...\n", 0 );
              i += 1;
              if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -seed-point ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->seed.y) );
              if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -seed-point ...\n", 0 );
              p->seed.z = 0;
              i += 1;
              if ( i >= argc) {
                  i --;
              }
              else {
                  status = sscanf( argv[i], "%d", &(p->seed.z) );
                  if ( status <= 0 ) {
                      p->seed.z = 0;
                      i --;
                  }
              }
              p->typeComputation = _VT_SEEDPT;
              p->cpar.type_output = VT_BINAIRE;
          }
          else if ( (strcmp ( argv[i], "-low-threshold" ) == 0)
                    || (strcmp ( argv[i], "-lt" ) == 0 && argv[i][3] == '\0')
                    || (strcmp ( argv[i], "-sb" ) == 0 && argv[i][3] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -low-threshold ...\n", 0 );
            status = sscanf( argv[i],"%f",&(p->low_threshold) );
            if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -low-threshold ...\n", 0 );
          }
          else if ( (strcmp ( argv[i], "-high-threshold" ) == 0)
                    || (strcmp ( argv[i], "-ht" ) == 0 && argv[i][3] == '\0')
                    || (strcmp ( argv[i], "-sh" ) == 0 && argv[i][3] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -high-threshold ...\n", 0 );
            status = sscanf( argv[i],"%f",&(p->high_threshold) );
            if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -high-threshold ...\n", 0 );
            p->typeComputation = _VT_HYSTERESIS;
            p->cpar.type_output = VT_BINAIRE;
          }




          /* connected components extraction parameters
           */

          else if ( strcmp ( argv[i], "-min-size-cc" ) == 0
                    || ( strcmp ( argv[i], "-scc" ) == 0 && argv[i][4] == '\0')
                    || ( strcmp ( argv[i], "-tcc" ) == 0 && argv[i][4] == '\0') ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -min-size-cc ...\n", 0 );
             status = sscanf( argv[i], "%d", &(p->cpar.min_size) );
             if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -min-size-cc ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-max-number-cc" ) == 0
                    || ( strcmp ( argv[i], "-ncc" ) == 0 && argv[i][4] == '\0') ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -max-number-cc ...\n", 0 );
             status = sscanf( argv[i], "%d", &(p->cpar.max_nbcc) );
             if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -max-number-cc ...\n", 0 );
          }
          else if ( ( strcmp ( argv[i], "-max" ) == 0 && argv[i][4] == '\0') ) {
              p->cpar.max_nbcc = 1;
              p->cpar.type_output = VT_BINAIRE;
          }
          else if ( strcmp( argv[i], "-connectivity" ) == 0
                    || ( strcmp ( argv[i], "-con" ) == 0 && argv[i][4] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -connectivity ...\n", 0 );
            status = sscanf( argv[i],"%d",&connectivity );
            if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -connectivity ...\n", 0 );
            switch( connectivity ) {
            default :
                API_ErrorParse_connexe( (char*)NULL, "parsing -connectivity: unknown connectivity\n", 0 );
            case 4 :
              p->cpar.type_connexite = N04;   break;
            case 6 :
              p->cpar.type_connexite = N06;   break;
            case 8 :
              p->cpar.type_connexite = N08;   break;
            case 10 :
              p->cpar.type_connexite = N10;   break;
            case 18 :
              p->cpar.type_connexite = N18;   break;
            case 26 :
              p->cpar.type_connexite = N26;   break;
            }
          }
          else if ( strcmp( argv[i], "-binary-output" ) == 0
                    || (strcmp ( argv[i], "-bin" ) == 0 && argv[i][4] == '\0') ) {
              p->cpar.type_output = VT_BINAIRE;
          }
          else if ( strcmp( argv[i], "-label-output" ) == 0
                    || ( strcmp ( argv[i], "-label" ) == 0 && argv[i][6] == '\0')
                    || ( strcmp ( argv[i], "-labels" ) == 0 && argv[i][7] == '\0') ) {
              p->cpar.type_output = VT_GREY;
          }
          else if ( strcmp( argv[i], "-size-output" ) == 0
                    || ( strcmp ( argv[i], "-size" ) == 0 && argv[i][5] == '\0')
                    || ( strcmp ( argv[i], "-sizes" ) == 0 && argv[i][6] == '\0') ) {
              p->cpar.type_output = VT_SIZE;
          }
          else if ( strcmp( argv[i], "-sort-sizes" ) == 0
                    || ( strcmp ( argv[i], "-sort" ) == 0 && argv[i][5] == '\0') ) {
              p->cpar.type_output = VT_GREY;
              p->cpar.max_nbcc = _EQUIVALENCE_ARRAY_SIZE_;
          }
          else if ( strcmp ( argv[i], "-2D" ) == 0 && argv[i][3] == '\0' ) {
              p->cpar.dim = VT_2D;
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
             if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_connexe( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_connexe( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_connexe( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_connexe( (char*)NULL, "parsing -type...\n", 0 );
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
              API_ErrorParse_connexe( (char*)NULL, "parsing -type...\n", 0 );
            }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_connexe( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_connexe( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( p->cpar.verbose <= 0 ) p->cpar.verbose = 1;
              else                        p->cpar.verbose ++;
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
              else                     _VT_VERBOSE_ ++;
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              p->cpar.verbose = 0;
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
              API_ErrorParse_connexe( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_connexe( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( inputisread == 0 ) {
              (void)strcpy( p->input_name, argv[i]);
              inputisread = 1;
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_connexe( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
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
        API_ErrorParse_connexe( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}
