/*************************************************************************
 * api-meanImages.c -
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
 * sed -e "s/meanImages/execuTable/g" \
 *     -e "s/MeanImages/ExecuTable/g" \
 *     -e "s/meanimages/executable/g" \
 *     [api-]meanImages.[c,h] > [api-]execTable.[c,h]
 *
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>
#include <convert.h>
#include <pixel-operation.h>
#include <local-operation.h>
#include <string-tools.h>

#include <vt_common.h>

#include <api-meanImages.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_meanImages( char *str, lineCmdParamMeanImages *p );



/* static procedures
 */

static void _FreeImageList( vt_image **array, int n );
static vt_image ** _ReadImageList( stringList *list );



static int _InMemoryComputationWithoutMasks( vt_image **imageStructure,
                                             int n,
                                             vt_image *imres,
                                             lineCmdParamMeanImages *par );
static int _InMemoryComputationWithMasks( vt_image **imageStructure,
                                          vt_image **maskStructure,
                                          int n,
                                          vt_image *imres,
                                          lineCmdParamMeanImages *par );



static int _StreamingComputationWithoutMasks( stringList *imageFileList,
                                              char *output_name,
                                              lineCmdParamMeanImages *par );
static int _StreamingComputationWithMasks( stringList *imageFileList,
                                           stringList *maskFileList,
                                           char *output_name,
                                           lineCmdParamMeanImages *par );





/************************************************************
 *
 * main APIs
 *
 ************************************************************/

int API_INTERMEDIARY_meanImages( stringList *image_names,
                              stringList *mask_names,
                              char *output_name,
                              char *param_str_1, char *param_str_2 )
{
    char *proc = "API_INTERMEDIARY_meanImages";
    lineCmdParamMeanImages par;

    vt_image **imageStructure = (vt_image**)NULL;
    vt_image **maskStructure = (vt_image**)NULL;
    vt_image imres;
    ImageType output_type;


    if ( image_names == (stringList*)NULL || image_names->n <= 0 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: no input image names?!", proc );
        return( -1 );
    }
    if ( mask_names != (stringList*)NULL && mask_names->n > 0
         && mask_names->n != image_names->n ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: numbers of image and mask names differ", proc );
        return( -1 );
    }



    /* parameter initialization
     */
    API_InitParam_meanImages( &par );

    /* parameter parsing
     */
    if ( param_str_1 != (char*)NULL )
        _API_ParseParam_meanImages( param_str_1, &par );
    if ( param_str_2 != (char*)NULL )
        _API_ParseParam_meanImages( param_str_2, &par );


    /***************************************************
     *
     *
     *
     ***************************************************/

    switch( par.computation ) {
    default :
        if ( par.print_lineCmdParam )
            API_PrintParam_meanImages( stderr, proc, &par, (char*)NULL );
        API_FreeParam_meanImages( &par );
        if ( _verbose_ )
            fprintf( stderr, "%s: unknown computation type, this is embarrasing ...\n", proc );
        return( -1 );

    case _MEMORY_ :

        imageStructure = _ReadImageList( image_names );
        if ( imageStructure == (vt_image**)NULL ) {
          API_FreeParam_meanImages( &par );
          if ( _verbose_ )
            fprintf( stderr, "%s: error when reading images\n", proc );
          return( -1 );
        }

        if ( mask_names != (stringList*)NULL && mask_names->n > 0 ) {
            maskStructure = _ReadImageList( mask_names );
            if ( maskStructure == (vt_image**)NULL ) {
              _FreeImageList( imageStructure, image_names->n );
              VT_Free( (void**)&imageStructure );
              API_FreeParam_meanImages( &par );
              if ( _verbose_ )
                fprintf( stderr, "%s: error when reading masks\n", proc );
              return( -1 );
            }
            if ( maskStructure[0]->dim.v != imageStructure[0]->dim.v
                 || maskStructure[0]->dim.x != imageStructure[0]->dim.x
                 || maskStructure[0]->dim.y != imageStructure[0]->dim.y
                 || maskStructure[0]->dim.z != imageStructure[0]->dim.z ) {
                _FreeImageList( maskStructure, mask_names->n );
                VT_Free( (void**)&maskStructure );
                _FreeImageList( imageStructure, image_names->n );
                VT_Free( (void**)&imageStructure );
                API_FreeParam_meanImages( &par );
                if ( _verbose_ )
                  fprintf( stderr, "%s: images and masks have different dimensions\n", proc );
                return( -1 );
            }
        }

        output_type = par.output_type;
        if ( output_type == TYPE_UNKNOWN ) {
            switch( par.operation ) {
            case _MAX_ :
            case _MIN_ :
            case _MEDIAN_ :
            case _QUANTILE_ :
                output_type = imageStructure[0]->type;
                break;
            case _MEAN_ :
            case _ROBUST_MEAN_ :
                output_type = imageStructure[0]->type;
                break;
            case _STDDEV_ :
            case _SUM_ :
            case _VAR_ :
            default :
                output_type = FLOAT;
                break;
            }
        }

        VT_InitFromImage( &imres, imageStructure[0], par.output_name, output_type );
        if ( VT_AllocImage( &imres ) != 1 ) {
            if ( mask_names != (stringList*)NULL && mask_names->n > 0 ) {
                _FreeImageList( maskStructure, mask_names->n );
                VT_Free( (void**)&maskStructure );
            }
            _FreeImageList( imageStructure, image_names->n );
            VT_Free( (void**)&imageStructure );
            API_FreeParam_meanImages( &par );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to allocate result image\n", proc );
          return( -1 );
        }

        /************************************************************
         *
         *  here is the stuff
         *
         ************************************************************/

        if ( API_meanImages( imageStructure, maskStructure, image_names->n, &imres, param_str_1, param_str_2 ) != 1 ) {
            VT_FreeImage( &imres );
            if ( mask_names != (stringList*)NULL && mask_names->n > 0 ) {
                _FreeImageList( maskStructure, mask_names->n );
                VT_Free( (void**)&maskStructure );
            }
            _FreeImageList( imageStructure, image_names->n );
            VT_Free( (void**)&imageStructure );
            API_FreeParam_meanImages( &par );
            if ( _verbose_ )
                fprintf( stderr, "%s: an error occurs during 'in memory' procedures\n", proc );
            return( -1 );
        }


        /* memory freeing
         */
        if ( mask_names != (stringList*)NULL && mask_names->n > 0 ) {
            _FreeImageList( maskStructure, mask_names->n );
            VT_Free( (void**)&maskStructure );
        }
        _FreeImageList( imageStructure, image_names->n );
        VT_Free( (void**)&imageStructure );

        /* output writing
         */
        if ( VT_WriteInrimage( &imres ) == -1 ) {
          VT_FreeImage( &imres );
          API_FreeParam_meanImages( &par );
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to write output image '%s'\n",
                       proc, par.output_name );
          return( -1 );
        }

        VT_FreeImage( &imres );

         /* end of case _MEMORY_
         */
        break;

    case _STREAMING_ :

        if ( par.print_lineCmdParam )
            API_PrintParam_meanImages( stderr, proc, &par, (char*)NULL );

        /* streaming computation with masks
         */
        if ( mask_names->n > 0 ) {
            if ( _StreamingComputationWithMasks( image_names, mask_names, output_name, &par ) != 1 ) {
                API_FreeParam_meanImages( &par );
                if ( _verbose_ )
                    fprintf( stderr, "%s: an error occurs during 'streaming' procedures\n", proc );
                return( -1 );
            }
        }
        /* streaming computation without masks
         */
        else {
            if ( _StreamingComputationWithoutMasks( image_names, output_name, &par ) != 1 ) {
                API_FreeParam_meanImages( &par );
                if ( _verbose_ )
                    fprintf( stderr, "%s: an error occurs during 'streaming' procedures\n", proc );
                return( -1 );
            }
        }

        /* end of case _STREAMING_
         */
        break;
    }

    API_FreeParam_meanImages( &par );

    return( 1 );
}





int API_meanImages( vt_image **imageStructure,
                    vt_image **maskStructure,
                    int n,
                    vt_image *imres, char *param_str_1, char *param_str_2 )
{
  char *proc = "API_meanImages";
  lineCmdParamMeanImages par;



  /* parameter initialization
   */
  API_InitParam_meanImages( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_meanImages( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_meanImages( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_meanImages( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  /* in memory computation with masks
   */
  if ( maskStructure != (vt_image**)NULL ) {
      if ( _InMemoryComputationWithMasks( imageStructure, maskStructure, n, imres, &par ) != 1 ) {
          API_FreeParam_meanImages( &par );
          if ( _verbose_ )
              fprintf( stderr, "%s: an error occurs during 'in memory' procedures\n", proc );
          return( -1 );
      }
  }
  /* in memory computation without masks
   */
  else {
      if ( _InMemoryComputationWithoutMasks( imageStructure, n, imres, &par ) != 1 ) {
          API_FreeParam_meanImages( &par );
          if ( _verbose_ )
              fprintf( stderr, "%s: an error occurs during 'in memory' procedures\n", proc );
          return( -1 );
      }
  }


  API_FreeParam_meanImages( &par );
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



static char *usage = "\
 [[-images %s %s ...] | [-image-list|-list|-refl %s] | ...\n\
  [-image-format|-format %s]]\n\
 [[-masks %s %s ...] | [-mask-list|-maskl %s] | [-mask-format %s]\n\
 [-first|-f %d] [-last|-l %d]\n\
 [[-res] image-out]\n\
 [-operation maximum|minimum|median|quantile|robust-mean| ...\n\
  ... sum|variance|var|stddev]\n\
 [-max|-min|-mean|-median|-quantile|-robust-mean|-rmean|-sum|...\n\
  ... -var|-stddev]\n\
 [-window %d %d [%d]]\n\
 [-quantile-value|-q %lf]\n\
 [-lts-cut|-lts-fraction %lf]\n\
 [-computation streaming|memory]\n\
 [-streaming | -memory]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-inv] [-swap] [output-image-type | -type s8|u8|s16|u16...]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'image-out' is not specified or equal to '-', stdout will be used\n\
# data\n\
 -images %s %s ...: all names following '-images' are considered as\n\
    input images, until a '-' is encountered as first character\n\
 -image-list|-list|-refl %s: text file containing a list of image file names\n\
 -image-format|-format %s: format 'a la printf' of images to be processed\n\
    should contain a '%d', to be used with '-first' and '-last'\n\
 -masks %s %s ...: all names following '-masks' are considered as\n\
    input masks, until a '-' is encountered as first character\n\
 -mask-list|-maskl %s: text file containing a list of image file names\n\
 -mask-format %s: format 'a la printf' of masks\n\
    should contain a '%d', to be used with '-first' and '-last'\n\
 -first|-f %d: first value of index for 'formats'\n\
 -last|-l %d:  last value of index for 'formats'\n\
 -res %s: output image\n\
# operation type\n\
 -maximum|-max: maximum value over the images\n\
 -minimum|-min: minimum value over the images\n\
 -mean        : average value over the images (default)\n\
 -median      : median value over the images \n\
 -quantile    : quantile value over the sorted values \n\
 -robust-mean|-rmean : trimmed estimation of the mean\n\
 -sum         : sum of the values over the images \n\
 -var         : variance \n\
 -stddev      : standard deviation\n\
# operation parameters\n\
 -window %d %d [%d]: window size for processing, default is 1 1 1\n\
 -quantile-value|-q %lf: quantile of the retained value\n\
    0:   minimum value, thus '-quantile-value 0'   <=> '-min'\n\
    0.5: median value,  thus '-quantile-value 0.5' <=> '-median'\n\
    1:   maximum value, thus '-quantile-value 1'   <=> '-max'\n\
 -lts-cut|-lts-fraction %lf: fraction of points to be kept for the\n\
    calculation of the robust mean (trimmed estimation)\n\
# computation / processing\n\
 -streaming: when possible, images are read one at the time\n\
 -memory: all images are read and stored into memory before processing\n\
    (default)\n\
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





char *API_Help_meanImages( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_meanImages( char *program, char *str, int flag )
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



void API_InitParam_meanImages( lineCmdParamMeanImages *p )
{
    initStringList( &(p->input_names) );
    (void)strncpy( p->input_list, "\0", 1 );
    (void)strncpy( p->input_format, "\0", 1 );

    initStringList( &(p->mask_names) );
    (void)strncpy( p->mask_list, "\0", 1 );
    (void)strncpy( p->mask_format, "\0", 1 );

    p->firstindex = 0;
    p->lastindex = 0;

    (void)strncpy( p->output_name, "\0", 1 );
    p->input_inv = 0;
    p->input_swap = 0;
    p->output_type = TYPE_UNKNOWN;

    p->operation = _MEAN_;

    p->window.x =  p->window.y = p->window.z = 1;
    p->quantile = 0.50;
    p->lts_fraction = 0.75;

    p->computation = _MEMORY_;

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}



void API_FreeParam_meanImages( lineCmdParamMeanImages *p )
{
    freeStringList( &(p->input_names) );
    freeStringList( &(p->mask_names) );
    API_InitParam_meanImages( p );
}






void API_PrintParam_meanImages( FILE *theFile, char *program,
                                  lineCmdParamMeanImages *p, char *str )
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

  printStringList( f, &(p->input_names), "- input image names are" );

  fprintf( f, "- input image list is " );
  if ( p->input_list != (char*)NULL && p->input_list[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_list );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- input image format is " );
  if ( p->input_format != (char*)NULL && p->input_format[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_format );
  else
    fprintf( f, "'NULL'\n" );

  printStringList( f, &(p->mask_names), "- input mask names are" );

  fprintf( f, "- input mask list is " );
  if ( p->mask_list != (char*)NULL && p->mask_list[0] != '\0' )
    fprintf( f, "'%s'\n", p->mask_list );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- input mask format is " );
  if ( p->mask_format != (char*)NULL && p->mask_format[0] != '\0' )
    fprintf( f, "'%s'\n", p->mask_format );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- first index for format = %d\n", p->firstindex );
  fprintf( f, "- last index for format = %d\n", p->lastindex );

  fprintf( f, "- output image is " );
  if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_name );
  else
    fprintf( f, "'NULL'\n" );


  fprintf( f, "# specific parameters\n" );
  fprintf( f, "- operation type is " );
  switch( p->operation ) {
  default :            fprintf( f, "unknown, this is embarrasing ...\n" ); break;
  case _MAX_ :         fprintf( f, "maximum\n" ); break;
  case _MEAN_ :        fprintf( f, "mean\n" ); break;
  case _MEDIAN_ :      fprintf( f, "median\n" ); break;
  case _MIN_ :         fprintf( f, "robust mean\n" ); break;
  case _QUANTILE_ :    fprintf( f, "minimum\n" ); break;
  case _ROBUST_MEAN_ : fprintf( f, "robust mean\n" ); break;
  case _STDDEV_ :      fprintf( f, "standard deviation\n" ); break;
  case _SUM_ :         fprintf( f, "sum\n" ); break;
  case _VAR_ :         fprintf( f, "variance\n" ); break;
  }
  fprintf( f, "- window size is [%d,%d,%d]\n", p->window.x, p->window.y, p->window.z );
  fprintf( f, "- quantile (for 'quantile' operation ) = %f\n", p->quantile );
  fprintf( f, "- lts fraction (for 'robust mean' operation ) = %f\n", p->lts_fraction );
  fprintf( f, "- computation type is " );
  switch( p->computation ) {
  default :            fprintf( f, "unknown, this is embarrasing ...\n" ); break;
  case _MEMORY_ :      fprintf( f, "memory\n" ); break;
  case _STREAMING_ :   fprintf( f, "streaming\n" ); break;
  }


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



static void _API_ParseParam_meanImages( char *str, lineCmdParamMeanImages *p )
{
  char *proc = "_API_ParseParam_meanImages";
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

  API_ParseParam_meanImages( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_meanImages( int firstargc, int argc, char *argv[],
                                  lineCmdParamMeanImages *p )
{
  char *proc = "API_ParseParam_meanImages";
  int i;
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
              if ( outputisread == 0 ) {
                (void)strcpy( p->output_name,  ">" );  /* standart output */
                outputisread = 1;
              }
              else {
                  API_ErrorParse_meanImages( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
              }
          }

          /* names
           */
          else if ( (strcmp ( argv[i], "-images" ) == 0 && argv[i][7] == '\0') ) {
              i++;
              while ( i<argc && argv[i][0] != '-' ) {
                  if ( addStringToList( argv[i], &(p->input_names) ) != 1 ) {
                      if ( _verbose_ )
                          fprintf( stderr, "%s: unable to add '%s' to image list\n", proc, argv[i] );
                      API_ErrorParse_meanImages( (char*)NULL, "parsing -images ...\n", 0 );
                  }
                i++;
              }
              i--;
          }
          else if ( strcmp ( argv[i], "-image-list" ) == 0
                    || (strcmp ( argv[i], "-list" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "-refl" ) == 0 && argv[i][5] == '\0') ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -image-list ...\n", 0 );
              (void)strcpy( p->input_list, argv[i] );
          }
          else if ( strcmp ( argv[i], "-image-format" ) == 0
                    || (strcmp ( argv[i], "-format" ) == 0 && argv[i][7] == '\0') ) {
            i++;
            if ( i >= argc )
                API_ErrorParse_meanImages( (char*)NULL, "parsing -image-format ...\n", 0 );
            (void)strcpy( p->input_format, argv[i] );
          }

          else if ( (strcmp ( argv[i], "-masks" ) == 0 && argv[i][6] == '\0') ) {
              i++;
              while ( i<argc && argv[i][0] != '-' ) {
                  if ( addStringToList( argv[i], &(p->mask_names) ) != 1 ) {
                      if ( _verbose_ )
                          fprintf( stderr, "%s: unable to add '%s' to mask list\n", proc, argv[i] );
                      API_ErrorParse_meanImages( (char*)NULL, "parsing -masks ...\n", 0 );
                  }
                i++;
              }
              i--;
          }
          else if ( strcmp ( argv[i], "-mask-list" ) == 0
                    || (strcmp ( argv[i], "-maskl" ) == 0 && argv[i][6] == '\0') ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -mask-list ...\n", 0 );
              (void)strcpy( p->mask_list, argv[i] );
          }
          else if ( strcmp ( argv[i], "-mask-format" ) == 0 ) {
            i++;
            if ( i >= argc )
                API_ErrorParse_meanImages( (char*)NULL, "parsing -mask-format ...\n", 0 );
            (void)strcpy( p->mask_format, argv[i] );
          }

          else if ( (strcmp ( argv[i], "-first" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-f" ) == 0 && argv[i][2] == '\0') ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -first ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->firstindex) );
              if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -first ...\n", 0 );
          }
          else if ( (strcmp ( argv[i], "-last" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "-l" ) == 0 && argv[i][2] == '\0') ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -last ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->lastindex) );
              if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -last ...\n", 0 );
          }

          else if ( strcmp ( argv[i], "-res" ) == 0  && argv[i][4] == '\0' ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -res ...\n", 0 );
              if ( outputisread )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -res: output already parsed ...\n", 0 );
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }


          /* operation type
           */
          else if ( strcmp ( argv[i], "-operation" ) == 0 ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -operation ...\n", 0 );
              if ( strcmp ( argv[i], "maximum" ) == 0
                   || (strcmp ( argv[i], "max" ) == 0 && argv[i][3] == '\0') ) {
                  p->operation = _MAX_;
              }
              else if ( strcmp ( argv[i], "minimum" ) == 0
                        || (strcmp ( argv[i], "min" ) == 0 && argv[i][3] == '\0') ) {
                       p->operation = _MIN_;
              }
              else if ( strcmp ( argv[i], "median" ) == 0  ) {
                  p->operation = _MEDIAN_;
              }
              else if ( strcmp ( argv[i], "mean" ) == 0 && argv[i][4] == '\0' ) {
                p->operation = _MEAN_;
              }
              else if ( strcmp ( argv[i], "quantile" ) == 0 ) {
                p->operation = _QUANTILE_;
              }
              else if ( strcmp ( argv[i], "robust-mean" ) == 0
                        || (strcmp ( argv[i], "rmean" ) == 0 && argv[i][5] == '\0') ) {
                p->operation = _ROBUST_MEAN_;
              }
              else if ( strcmp ( argv[i], "sum" ) == 0 && argv[i][3] == '\0' ) {
                p->operation = _SUM_;
              }
              else if ( strcmp ( argv[i], "stddev" ) == 0 && argv[i][6] == '\0' ) {
                p->operation = _STDDEV_;
              }
              else if ( strcmp ( argv[i], "var" ) == 0 && argv[i][3] == '\0' ) {
                p->operation = _VAR_;
              }


              else {
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -operation: unknown mode ...\n", 0 );
              }
          }

          else if ( strcmp ( argv[i], "-maximum" ) == 0
                    || (strcmp ( argv[i], "-max" ) == 0 && argv[i][4] == '\0') ) {
            p->operation = _MAX_;
          }
          else if ( strcmp ( argv[i], "-minimum" ) == 0
                    || (strcmp ( argv[i], "-min" ) == 0 && argv[i][4] == '\0') ) {
            p->operation = _MIN_;
          }
          else if ( strcmp ( argv[i], "-median" ) == 0 ) {
            p->operation = _MEDIAN_;
          }
          else if ( strcmp ( argv[i], "-mean" ) == 0 && argv[i][5] == '\0' ) {
            p->operation = _MEAN_;
          }
          else if ( strcmp ( argv[i], "-quantile" ) == 0 ) {
            p->operation = _QUANTILE_;
          }
          else if ( strcmp ( argv[i], "-robust-mean" ) == 0
                    || (strcmp ( argv[i], "-rmean" ) == 0 && argv[i][6] == '\0') ) {
            p->operation = _ROBUST_MEAN_;
          }
          else if ( strcmp ( argv[i], "-sum" ) == 0  && argv[i][4] == '\0' ) {
            p->operation = _SUM_;
          }
          else if ( strcmp ( argv[i], "-stddev" ) == 0  && argv[i][7] == '\0' ) {
            p->operation = _STDDEV_;
          }
          else if ( strcmp ( argv[i], "-var" ) == 0  && argv[i][4] == '\0' ) {
            p->operation = _VAR_;
          }


          /* operation parameters
           */
          else if ( (strcmp (argv[i], "-window" ) == 0 && argv[i][7] == '\0')
                    || (strcmp (argv[i], "-w") == 0 && argv[i][2] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -window %d ...\n", 0 );
            status = sscanf( argv[i], "%d", &(p->window.x) );
            if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -window %d ...\n", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -window %d %d ...\n", 0 );
            status = sscanf( argv[i], "%d", &(p->window.y) );
            if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -window %d %d ...\n", 0 );
            i ++;
            if ( i >= argc) p->window.z = 1;
            else {
              status = sscanf( argv[i], "%d", &(p->window.z) );
              if ( status <= 0 ) {
                i--;
                p->window.z = 1;
              }
            }
          }
          else if ( strcmp ( argv[i], "-quantile-value" ) == 0
                    || (strcmp ( argv[i], "-q" ) == 0  && argv[i][2] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -quantile-value ...\n", 0 );
            status = sscanf( argv[i], "%lf", &(p->quantile) );
            if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -quantile-value ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-lts-fraction" ) == 0
                    || strcmp ( argv[i], "-lts-cut" ) == 0) {
            i ++;
            if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -lts-fraction ...\n", 0 );
            status = sscanf( argv[i], "%lf", &(p->lts_fraction) );
            if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -lts-fraction ...\n", 0 );
          }

          /* computation / processing
           */
          else if ( strcmp ( argv[i], "-computation" ) == 0 ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -computation ...\n", 0 );
              if ( strcmp ( argv[i], "memory" ) == 0 ) {
                  p->computation = _MEMORY_;
              }
              else if ( strcmp ( argv[i], "streaming" ) == 0 ) {
                  p->computation = _STREAMING_;
              }
              else {
                  API_ErrorParse_meanImages( (char*)NULL, "parsing -computation: unknown mode ...\n", 0 );
              }
          }
          else if ( strcmp ( argv[i], "-memory" ) == 0 ) {
              p->computation = _MEMORY_;
          }
          else if ( strcmp ( argv[i], "-streaming" ) == 0 ) {
              p->computation = _STREAMING_;
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
             if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_meanImages( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_meanImages( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_meanImages( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_meanImages( (char*)NULL, "parsing -type...\n", 0 );
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
              API_ErrorParse_meanImages( (char*)NULL, "parsing -type...\n", 0 );
            }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_meanImages( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_meanImages( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
              else                     _VT_VERBOSE_ ++;
              incrementVerboseInStringTools();
              incrementVerboseInConvert();
              incrementVerboseInPixelOperation();
              incrementVerboseInLocalOperation();
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              _verbose_ = 0;
              _VT_VERBOSE_ = 0;
              setVerboseInStringTools( 0 );
              setVerboseInConvert( 0 );
              setVerboseInPixelOperation( 0 );
              setVerboseInLocalOperation( 0 );
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
              API_ErrorParse_meanImages( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_meanImages( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_meanImages( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
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
        API_ErrorParse_meanImages( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}









/**************************************************
 *
 * image array
 *
 **************************************************/

static void _FreeImageList( vt_image **array, int n )
{
  int i;

  if ( array == (vt_image**)NULL ) return;
  if ( n <= 0 ) return;

  for ( i=0; i<n; i++ ) {
    if ( array[i] != (vt_image*)NULL ) {
      VT_FreeImage( array[i] );
      VT_Free( (void**)&(array[i]) );
    }
  }
}



static vt_image ** _ReadImageList( stringList *list )
{
  char *proc = "_ReadImageList";
  vt_image **a = (vt_image**)NULL;
  int i;

  if ( list->n <= 0 ) return( (vt_image**)NULL );

  a = (vt_image**)malloc( list->n * sizeof( vt_image* ) );
  if ( a == (vt_image**)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate array\n", proc );
    return( (vt_image**)NULL );
  }

  for ( i=0; i<list->n; i++ ) a[i] = (vt_image*)NULL;

  for ( i=0; i<list->n; i++ ) {
     a[i] = _VT_Inrimage( list->data[i] );
     if ( a[i] ==  (vt_image*)NULL ) {
       _FreeImageList( a, list->n );
       free( a );
       if ( _verbose_ )
         fprintf( stderr, "%s: unable to read image #%d '%s'\n", proc, i, list->data[i] );
       return( (vt_image**)NULL );
     }
  }

  for ( i=1; i<list->n; i++ ) {
    if ( a[i]->dim.v != a[0]->dim.v
         || a[i]->dim.x != a[0]->dim.x
         || a[i]->dim.y != a[0]->dim.y
         || a[i]->dim.z != a[0]->dim.z ) {
      _FreeImageList( a, list->n );
      free( a );
       if ( _verbose_ )
         fprintf( stderr, "%s: image #%d '%s' has different dimensions than image #0 '%s'\n",
                  proc, i, list->data[i], list->data[0]);
       return( (vt_image**)NULL );
    }
    if ( a[i]->type != a[0]->type ) {
      _FreeImageList( a, list->n );
      free( a );
       if ( _verbose_ )
         fprintf( stderr, "%s: image #%d '%s' has a different type than image #0 '%s'\n",
                  proc, i, list->data[i], list->data[0]);
       return( (vt_image**)NULL );
    }
  }

  return( a );
}







/**************************************************
 *
 * common procedures
 *
 **************************************************/





static int _AllocAuxiliaryImages( vt_image *imRes,
                                  vt_image *imSum,
                                  vt_image *imSumSqr,
                                  vt_image *image,
                                  char *name,
                                  typeMeanImagesOperation operation )
{
  char * proc = "_AllocAuxiliaryImages";

  switch ( operation ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such operation not handled yet\n", proc );
    return( -1 );
  case _MAX_ :
  case _MIN_ :
    VT_InitFromImage( imRes, image, name, image->type );
    if ( VT_AllocImage( imRes ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate result image\n", proc);
      return( -1 );
    }
    break;

  case _STDDEV_ :
  case _VAR_ :
    VT_InitFromImage( imSumSqr, image, name, FLOAT );
    if ( VT_AllocImage( imSumSqr ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate sum of squares image\n", proc );
      return( -1 );
    }

  case _MEAN_ :
  case _SUM_ :
    VT_InitFromImage( imSum, image, name, FLOAT );
    if ( VT_AllocImage( imSum ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate sum image\n", proc );
      return( -1 );
    }
    break;
  }

  return( 1 );
}





static int _AllocInitAuxiliaryWithMasks( vt_image *imTotalRes,
                                         vt_image *imTotalSum,
                                         vt_image *imTotalSumSqr,
                                         vt_image *imTotalMask,
                                         vt_image *image,
                                         vt_image *mask,
                                         char *name,
                                         typeMeanImagesOperation operation )
{
  char *proc = "_AllocInitAuxiliaryWithMasks";

  unsigned short int *bufTotalMask = NULL;

  int theDim[3] = {0,0,0};
  size_t v = 0;
  size_t i;



  /* allocations
   */
  if ( _AllocAuxiliaryImages( imTotalRes, imTotalSum, imTotalSumSqr,
                              image, name, operation ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary images\n", proc);
    return( -1 );
  }

  VT_InitFromImage( imTotalMask, image, name, USHORT );
  if ( VT_AllocImage( imTotalMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate sum of masks image\n", proc );
    return( -1 );
  }
  bufTotalMask = (unsigned short int *)imTotalMask->buf;



  theDim[0] = image->dim.v * image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;
  v = (size_t)theDim[0] * (size_t)theDim[1] * (size_t)theDim[2];



  /* initialisation of auxiliary images with the first image
   */
  switch ( operation ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such operation not handled yet\n", proc );
    return( -1 );
  case _MAX_ :
  case _MIN_ :
    if ( maskImage( image->buf, image->type,
                    mask->buf, mask->type,
                    imTotalRes->buf, imTotalRes->type,
                    theDim ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error during min/max init\n", proc );
      return( -1 );
    }
    break;
  case _STDDEV_ :
  case _VAR_ :
    if ( sqrImage( image->buf, image->type,
                   imTotalSumSqr->buf, imTotalSumSqr->type,
                   theDim ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error during stddev/var init (1)\n", proc );
      return( -1 );
    }
    if ( maskImage( imTotalSumSqr->buf, imTotalSumSqr->type,
                    mask->buf, mask->type,
                    imTotalSumSqr->buf, imTotalSumSqr->type,
                    theDim ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error during stddev/var init (2)\n", proc );
      return( -1 );
    }
  case _SUM_ :
  case _MEAN_ :
    if ( ConvertBuffer( image->buf, image->type,
                        imTotalSum->buf, imTotalSum->type, v ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error during conversion (2)\n", proc );
      return( -1 );
    }
    if ( maskImage( imTotalSum->buf, imTotalSum->type,
                    mask->buf, mask->type,
                    imTotalSum->buf, imTotalSum->type,
                    theDim ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error during sum/mean init (2)\n", proc );
      return( -1 );
    }
    break;
  }

  switch ( mask->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such mask image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      u8 *theMask = (u8*)mask->buf;
      for ( i=0; i<v; i++ )
        bufTotalMask[i] = (theMask[i] > 0) ? 1 : 0;
    }
    break;
  case USHORT :
    {
      u16 *theMask = (u16*)mask->buf;
      for ( i=0; i<v; i++ )
        bufTotalMask[i] = (theMask[i] > 0) ? 1 : 0;
    }
    break;
  }

  return( 1 );
}





static int _UpdateAuxiliaryWithMasks( vt_image *imTotalRes,
                                   vt_image *imTotalSum,
                                   vt_image *imTotalSumSqr,
                                   vt_image *imTotalMask,
                                   vt_image *image,
                                   vt_image *mask,
                                   typeMeanImagesOperation operation )
{
  char * proc = "_UpdateAuxiliaryWithMasks";

  float *bufTotalSum = NULL;
  float *bufTotalSumSqr = NULL;
  unsigned short int *bufTotalMask = (unsigned short int *)imTotalMask->buf;

  size_t v = 0;
  size_t i;


  switch ( operation ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such operation not handled yet\n", proc );
    return( -1 );
  case _MAX_ :
  case _MIN_ :
    break;

  case _STDDEV_ :
  case _VAR_ :
    bufTotalSumSqr = (float*)imTotalSumSqr->buf;

  case _MEAN_ :
  case _SUM_ :
    bufTotalSum = (float*)imTotalSum->buf;
    break;
  }



  v = (size_t)image->dim.v * (size_t)image->dim.x * (size_t)image->dim.y * (size_t)image->dim.z;



#define _MAX_WITHMASKS_( TYPE ) {       \
  TYPE *theBuf = (TYPE*)image->buf;     \
  TYPE *resBuf = (TYPE*)imTotalRes->buf; \
  for ( i=0; i<v; i++ ) {               \
    if ( theMask[i] == 0 ) continue;    \
    if ( bufTotalMask[i] == 0 ) resBuf[i] = theBuf[i]; \
    else if ( resBuf[i] < theBuf[i] ) resBuf[i] = theBuf[i]; \
    bufTotalMask[i] ++;                 \
  }                                     \
}

#define _MIN_WITHMASKS_( TYPE ) {       \
  TYPE *theBuf = (TYPE*)image->buf;     \
  TYPE *resBuf = (TYPE*)imTotalRes->buf; \
  for ( i=0; i<v; i++ ) {               \
    if ( theMask[i] == 0 ) continue;    \
    if ( bufTotalMask[i] == 0 ) resBuf[i] = theBuf[i]; \
    else if ( resBuf[i] > theBuf[i] ) resBuf[i] = theBuf[i]; \
    bufTotalMask[i] ++;                 \
  }                                     \
}

#define _SQR_WITHMASKS_( TYPE ) {       \
  TYPE *theBuf = (TYPE*)image->buf;     \
  for ( i=0; i<v; i++ ) {               \
    if ( theMask[i] == 0 ) continue;    \
    bufTotalSumSqr[i] += (float)theBuf[i] * (float)theBuf[i]; \
    bufTotalSum[i] += (float)theBuf[i]; \
    bufTotalMask[i] ++;                 \
  }                                     \
}

#define _SUM_WITHMASKS_( TYPE ) {       \
  TYPE *theBuf = (TYPE*)image->buf;     \
  for ( i=0; i<v; i++ ) {               \
    if ( theMask[i] == 0 ) continue;    \
    bufTotalSum[i] += (float)theBuf[i]; \
    bufTotalMask[i] ++;                 \
  }                                     \
}


#define _OPERATION_WITHMASKS_( TYPEM ) { \
  TYPEM *theMask = (TYPEM*)mask->buf; \
  switch ( operation ) {              \
  default :                           \
    if ( _verbose_ )                  \
      fprintf( stderr, "%s: such operation not handled yet\n", proc ); \
    return( -1 );                     \
  case _MAX_ :                        \
    switch( image->type ) {           \
    default :                         \
      if ( _verbose_ )                \
        fprintf( stderr, "%s: such image type handled yet\n", proc ); \
      return( -1 );                   \
    case UCHAR :                      \
      _MAX_WITHMASKS_( u8 );          \
      break;                          \
    case USHORT :                     \
      _MAX_WITHMASKS_( u16 );         \
      break;                          \
    case SSHORT :                     \
      _MAX_WITHMASKS_( s16 );         \
      break;                          \
    }                                 \
    break;                            \
  case _MIN_ :                        \
    switch( image->type ) {           \
    default :                         \
      if ( _verbose_ )                \
        fprintf( stderr, "%s: such image type handled yet\n", proc ); \
      return( -1 );                   \
    case UCHAR :                      \
      _MIN_WITHMASKS_( u8 );          \
      break;                          \
    case USHORT :                     \
      _MIN_WITHMASKS_( u16 );         \
      break;                          \
    case SSHORT :                     \
      _MIN_WITHMASKS_( s16 );         \
      break;                          \
    }                                 \
    break;                            \
  case _STDDEV_ :                     \
  case _VAR_ :                        \
    switch( image->type ) {           \
    default :                         \
      if ( _verbose_ )                \
        fprintf( stderr, "%s: such image type handled yet\n", proc ); \
      return( -1 );                   \
    case UCHAR :                      \
      _SQR_WITHMASKS_( u8 );          \
      break;                          \
    case USHORT :                     \
      _SQR_WITHMASKS_( u16 );         \
      break;                          \
    case SSHORT :                     \
      _SQR_WITHMASKS_( s16 );         \
      break;                          \
    }                                 \
    break;                            \
  case _SUM_ :                        \
  case _MEAN_ :                       \
    switch( image->type ) {           \
    default :                         \
      if ( _verbose_ )                \
        fprintf( stderr, "%s: such image type handled yet\n", proc ); \
      return( -1 );                   \
    case UCHAR :                      \
      _SUM_WITHMASKS_( u8 );          \
      break;                          \
    case USHORT :                     \
      _SUM_WITHMASKS_( u16 );         \
      break;                          \
    case SSHORT :                     \
      _SUM_WITHMASKS_( s16 );         \
      break;                          \
    }                                 \
    break;                            \
  }                                   \
  break;                              \
}


  switch ( mask->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such mask image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _OPERATION_WITHMASKS_( u8 );
    break;
  case USHORT :
    _OPERATION_WITHMASKS_( u16 );
    break;
  }

  return( 1 );
}





static vt_image * _ResultFromAuxiliaryWithMasks( vt_image *imTotalRes,
                                       vt_image *imTotalSum,
                                       vt_image *imTotalSumSqr,
                                       vt_image *imTotalMask,
                                       typeMeanImagesOperation operation )
{
  char * proc = "_ResultFromAuxiliaryWithMasks";

  vt_image *imResult = (vt_image*)NULL;

  float *bufTotalSum = NULL;
  float *bufTotalSumSqr = NULL;
  unsigned short int *bufTotalMask = (unsigned short int *)imTotalMask->buf;

  size_t v = 0;
  size_t i;


  switch ( operation ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such operation not handled yet\n", proc );
    return( (vt_image*)NULL );
  case _MAX_ :
  case _MIN_ :
    imResult = imTotalRes;
    break;

  case _SUM_ :
    imResult = imTotalSum;
    break;

  case _VAR_ :
    imResult = imTotalSum;
    v = (size_t)imResult->dim.v * (size_t)imResult->dim.x * (size_t)imResult->dim.y * (size_t)imResult->dim.z;
    bufTotalSum = (float*)imTotalSum->buf;
    bufTotalSumSqr = (float*)imTotalSumSqr->buf;
    for ( i=0; i<v; i++ ) {
      if ( bufTotalMask[i] == 0 ) continue;
      bufTotalSum[i] /= (float)bufTotalMask[i];
      bufTotalSumSqr[i] /= (float)bufTotalMask[i];
      bufTotalSum[i] = bufTotalSumSqr[i] - bufTotalSum[i] * bufTotalSum[i];
    }
    break;

  case _STDDEV_ :
    imResult = imTotalSum;
    v = (size_t)imResult->dim.v * (size_t)imResult->dim.x * (size_t)imResult->dim.y * (size_t)imResult->dim.z;
    bufTotalSum = (float*)imTotalSum->buf;
    bufTotalSumSqr = (float*)imTotalSumSqr->buf;
    for ( i=0; i<v; i++ ) {
      if ( bufTotalMask[i] == 0 ) continue;
      bufTotalSum[i] /= (float)bufTotalMask[i];
      bufTotalSumSqr[i] /= (float)bufTotalMask[i];
      bufTotalSum[i] = sqrt( bufTotalSumSqr[i] - bufTotalSum[i] * bufTotalSum[i] );
    }
    break;

  case _MEAN_ :
    imResult = imTotalSum;
    v = (size_t)imResult->dim.v * (size_t)imResult->dim.x * (size_t)imResult->dim.y * (size_t)imResult->dim.z;
    bufTotalSum = (float*)imTotalSum->buf;
    for ( i=0; i<v; i++ ) {
      if ( bufTotalMask[i] == 0 ) continue;
      bufTotalSum[i] /= (float)bufTotalMask[i];
    }
    break;
  }

  return( imResult );
}





static int _addSqrImage( void *bufferIn1, bufferType typeIn1,
                        void *bufferIn2, bufferType typeIn2,
                        void *bufferOut, bufferType typeOut,
                        int *bufferDims )
{
  char *proc = "_addSqrImage";
  size_t i, v;

  v = (size_t)bufferDims[0] * (size_t)bufferDims[1] * (size_t)bufferDims[2];

#define _ADDSQR_OPERATION( TYPE1, TYPE2 ) { \
  TYPE1 *theBuf1 = (TYPE1*)bufferIn1;       \
  TYPE2 *theBuf2 = (TYPE2*)bufferIn2;       \
  switch ( typeOut ) {                      \
  default :                                 \
    if ( _verbose_ )                            \
      fprintf( stderr, "%s: such output image type not handled yet\n", proc ); \
    return( -1 );                           \
  case FLOAT :                              \
    {                                       \
       r32 *resBuf = (r32*)bufferOut;       \
       for ( i=0; i<v; i++ )                \
          resBuf[i] = (r32)theBuf1[i] + (r32)theBuf2[i]; \
    }                                       \
    break;                                  \
  case DOUBLE :                             \
    {                                       \
       r64 *resBuf = (r64*)bufferOut;       \
       for ( i=0; i<v; i++ )                \
          resBuf[i] = (r64)theBuf1[i] + (r64)theBuf2[i]; \
    }                                       \
    break;                                  \
  }                                         \
}

  switch ( typeIn1 ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such first input image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such second input image type not handled yet\n", proc );
      return( -1 );
    case FLOAT :
      _ADDSQR_OPERATION( u8, r32 );
      break;
    case DOUBLE :
      _ADDSQR_OPERATION( u8, r64 );
      break;
    }
    break;
  case USHORT :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such second input image type not handled yet\n", proc );
      return( -1 );
    case FLOAT :
      _ADDSQR_OPERATION( u16, r32 );
      break;
    case DOUBLE :
      _ADDSQR_OPERATION( u16, r64 );
      break;
    }
    break;
  case SSHORT :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such second input image type not handled yet\n", proc );
      return( -1 );
    case FLOAT :
      _ADDSQR_OPERATION( s16, r32 );
      break;
    case DOUBLE :
      _ADDSQR_OPERATION( s16, r64 );
      break;
    }
    break;
  case FLOAT :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such second input image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      _ADDSQR_OPERATION( r32, u8 );
      break;
    case USHORT :
      _ADDSQR_OPERATION( r32, u16 );
      break;
    case SSHORT :
      _ADDSQR_OPERATION( r32, s16 );
      break;
    case FLOAT :
      _ADDSQR_OPERATION( r32, r32 );
      break;
    case DOUBLE :
      _ADDSQR_OPERATION( r32, r64 );
      break;
    }
    break;
  case DOUBLE :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such second input image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      _ADDSQR_OPERATION( r64, u8 );
      break;
    case USHORT :
      _ADDSQR_OPERATION( r64, u16 );
      break;
    case SSHORT :
      _ADDSQR_OPERATION( r64, s16 );
      break;
    case FLOAT :
      _ADDSQR_OPERATION( r64, r32 );
      break;
    case DOUBLE :
      _ADDSQR_OPERATION( r64, r64 );
      break;
    }
    break;
  }

  return( 1 );
}






/**************************************************
 *
 * in memory computation
 *
 **************************************************/





static int _InMemoryComputationWithoutMasks( vt_image **imageStructure,
                                             int n,
                                             vt_image *imres,
                                             lineCmdParamMeanImages *par )
{
  char *proc = "_InMemoryComputationWithoutMasks";

  int theDim[3] = {0,0,0};
  int winDim[3] = {0,0,0};

  bufferType resType;
  vt_image *imTotalRes;
  vt_image imTmp;
  int i, r;

  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );


  switch ( par->operation ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such operation not handled yet\n", proc );
  return( -1 );
  case _MAX_ :
  case _MEDIAN_ :
  case _MEAN_ :
  case _MIN_ :
  case _QUANTILE_ :
  case _ROBUST_MEAN_ :
    resType = imageStructure[0]->type;
    break;
  case _STDDEV_ :
  case _SUM_ :
  case _VAR_ :
    resType = FLOAT;
    break;
  }

  VT_Image( &imTmp );
  if ( resType == imres->type ) {
      imTotalRes = imres;
  }
  else {
      VT_InitFromImage( &imTmp, imageStructure[0], par->output_name, resType );
      if ( VT_AllocImage( &imTmp ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc);
        return( -1 );
      }
      imTotalRes = &imTmp;
  }

  if ( resType != imres->type ) VT_FreeImage( &imTmp );




  theDim[0] = imageStructure[0]->dim.v * imageStructure[0]->dim.x;
  theDim[1] = imageStructure[0]->dim.y;
  theDim[2] = imageStructure[0]->dim.z;
  winDim[0] = imageStructure[0]->dim.v * par->window.x;
  winDim[1] = par->window.y;
  winDim[2] = par->window.z;


#define _OPERATION_MEMORY_WITHOUTMASKS_( TYPE ) {                       \
  TYPE **buffers = (TYPE**)NULL;                                        \
  buffers = (TYPE**)malloc( n * sizeof( TYPE* ) );                      \
  if ( buffers == (TYPE**)NULL ) {                                      \
    if ( resType != imres->type ) VT_FreeImage( &imTmp );               \
    if ( _verbose_ )                                                    \
      fprintf( stderr, "%s: unable to allocate buffer array\n", proc);  \
    return( -1 );                                                       \
  }                                                                     \
  for ( i=0; i<n; i++ )                                                 \
    buffers[i] = (TYPE*)(imageStructure[i]->buf);                       \
  switch ( par->operation ) {                                           \
  default : r = 0; break;                                               \
  case _MAX_ :                                                          \
    r = maxFilteringBuffers( (void**)buffers, imageStructure[0]->type,  \
                             n, imTotalRes->buf, imTotalRes->type,      \
                             theDim, winDim );                          \
    break;                                                              \
  case _MEAN_ :                                                         \
    r = meanFilteringBuffers( (void**)buffers, imageStructure[0]->type, \
                                n, imTotalRes->buf, imTotalRes->type,   \
                                theDim, winDim );                       \
    break;                                                              \
  case _MEDIAN_ :                                                       \
    r = medianFilteringBuffers( (void**)buffers, imageStructure[0]->type, \
                                n, imTotalRes->buf, imTotalRes->type,   \
                                theDim, winDim );                       \
    break;                                                              \
  case _MIN_ :                                                          \
    r = minFilteringBuffers( (void**)buffers, imageStructure[0]->type,  \
                                n, imTotalRes->buf, imTotalRes->type,   \
                                theDim, winDim );                       \
    break;                                                              \
  case _QUANTILE_ :                                                     \
    r = quantileFilteringBuffers( (void**)buffers, imageStructure[0]->type, \
                                  n, imTotalRes->buf, imTotalRes->type, \
                                  theDim, winDim, par->quantile );      \
    break;                                                              \
  case _ROBUST_MEAN_ :                                                  \
    r = robustMeanFilteringBuffers( (void**)buffers, imageStructure[0]->type, \
                                    n, imTotalRes->buf, imTotalRes->type, \
                                    theDim, winDim, par->lts_fraction ); \
    break;                                                              \
  case _STDDEV_ :                                                       \
    r = stddevFilteringBuffers( (void**)buffers, imageStructure[0]->type, \
                                n, imTotalRes->buf, imTotalRes->type,   \
                                theDim, winDim );                       \
    break;                                                              \
  case _SUM_ :                                                          \
    r = sumFilteringBuffers( (void**)buffers, imageStructure[0]->type,  \
                             n, imTotalRes->buf, imTotalRes->type,      \
                             theDim, winDim );                          \
    break;                                                              \
  case _VAR_ :                                                          \
    r = varFilteringBuffers( (void**)buffers, imageStructure[0]->type,  \
                             n, imTotalRes->buf, imTotalRes->type,      \
                             theDim, winDim );                          \
    break;                                                              \
  }                                                                     \
  free( buffers );                                                      \
  if ( r != 1 ) {                                                       \
    if ( resType != imres->type ) VT_FreeImage( &imTmp );               \
    if ( _verbose_ )                                                    \
      fprintf( stderr, "%s: error when filtering buffer array\n", proc); \
    return( -1 );                                                       \
  }                                                                     \
}

  switch( imageStructure[0]->type ) {
  default :
    if ( resType != imres->type ) VT_FreeImage( &imTmp );
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc);
    return( -1 );
  case UCHAR :
    _OPERATION_MEMORY_WITHOUTMASKS_( u8 );
    break;
  case SCHAR :
    _OPERATION_MEMORY_WITHOUTMASKS_( s8 );
    break;
  case USHORT :
    _OPERATION_MEMORY_WITHOUTMASKS_( u16 );
    break;
  case SSHORT :
    _OPERATION_MEMORY_WITHOUTMASKS_( s16 );
    break;
  case FLOAT :
    _OPERATION_MEMORY_WITHOUTMASKS_( r32 );
    break;
  }



  /* processing is over
   */
  if ( resType != imres->type ) {
      if ( VT_CopyImage( &imTmp, imres ) != 1 ) {
          VT_FreeImage( &imTmp );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to copy result image\n", proc );
          return( -1 );
      }
      VT_FreeImage( &imTmp );
  }

  return( 1 );
}





static int _InMemoryComputationWithMasks( vt_image **imageStructure,
                                          vt_image **maskStructure,
                                          int n,
                                          vt_image *imres,
                                          lineCmdParamMeanImages *par )
{
  char *proc = "_InMemoryComputationWithMasks";

  vt_image imTotalSum;
  vt_image imTotalSumSqr;
  vt_image imTotalRes;
  vt_image imTotalMask;
  vt_image *imLocal;

  int nimages;

  ImageType output_type = TYPE_UNKNOWN;



  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );

  VT_Image ( &imTotalSum );
  VT_Image ( &imTotalSumSqr );
  VT_Image ( &imTotalRes );
  VT_Image ( &imTotalMask );



  if ( par->window.x != 1 || par->window.y != 1 || par->window.z != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: computation with window > 1x1x1 not handled yet\n", proc );
    return( -1 );
  }


  output_type = par->output_type;
  if ( output_type == TYPE_UNKNOWN ) {
      switch( par->operation ) {
      case _MAX_ :
      case _MIN_ :
      case _MEDIAN_ :
      case _QUANTILE_ :
          output_type = imLocal->type;
          break;
      case _MEAN_ :
      case _ROBUST_MEAN_ :
          output_type = imLocal->type;
          break;
      case _STDDEV_ :
      case _SUM_ :
      case _VAR_ :
      default :
          output_type = FLOAT;
          break;
      }
  }



  /* we can go
   */

#define _DEALLOCATIONS_MEMORY_WITHMASKS_ { \
  VT_FreeImage ( &imTotalSum );         \
  VT_FreeImage ( &imTotalSumSqr );      \
  VT_FreeImage ( &imTotalRes );         \
  VT_FreeImage ( &imTotalMask );        \
}



  /* allocation and initialisation of auxiliary images
   */
  if ( _AllocInitAuxiliaryWithMasks( &imTotalRes, &imTotalSum,
                                  &imTotalSumSqr, &imTotalMask,
                                  imageStructure[0], maskStructure[0],
                                  par->output_name, par->operation ) != 1 ) {
    _DEALLOCATIONS_MEMORY_WITHMASKS_;
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate or initialize auxiliary images\n", proc );
    return( -1 );
  }




  /* process other images
   */
  for ( nimages=1; nimages<n; nimages++ ) {

    if ( _UpdateAuxiliaryWithMasks( &imTotalRes, &imTotalSum,
                                 &imTotalSumSqr, &imTotalMask,
                                 imageStructure[nimages], maskStructure[nimages],
                                 par->operation ) != 1 ) {
      _DEALLOCATIONS_MEMORY_WITHMASKS_;
      if ( _verbose_ ) {
        fprintf( stderr, "%s: unable to update auxiliary images\n", proc );
        fprintf( stderr, "\t while processing image #%d '%s'\n", nimages, imageStructure[nimages]->name );
      }
      return( -1 );
    }

  }



  /* last computations
   * _ResultFromAuxiliaryWithMasks() return either imTotalRes or imTotalSum
   * in case of success
   */
  imLocal = _ResultFromAuxiliaryWithMasks( &imTotalRes, &imTotalSum,
                                           &imTotalSumSqr, &imTotalMask, par->operation );

  VT_FreeImage ( &imTotalSumSqr );
  VT_FreeImage ( &imTotalMask );

  if ( VT_CopyImage( imLocal, imres ) != 1 ) {
      VT_FreeImage ( &imTotalSum );
      VT_FreeImage ( &imTotalRes );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to copy auxiliary result image\n", proc );
      return( -1 );
  }

  VT_FreeImage ( &imTotalSum );
  VT_FreeImage ( &imTotalRes );

  return( 1 );
}







/**************************************************
 *
 * streaming computation
 *
 **************************************************/





static int _StreamingComputationWithoutMasks( stringList *imageFileList,
                                              char *output_name,
                                              lineCmdParamMeanImages *par )
{
  char *proc = "_StreamingComputationWithoutMasks";

  vt_image *imLocal = (vt_image*)NULL;
  vt_image *imResult = (vt_image*)NULL;

  vt_image imTotalSum;
  vt_image imTotalSumSqr;
  vt_image imTotalRes;

  vt_image imLocalSum;
  vt_image imLocalSumSqr;
  vt_image imLocalRes;

  float *bufTotalSum = NULL;
  float *bufTotalSumSqr = NULL;

  int theDim[3] = {0,0,0};
  int winDim[3] = {0,0,0};
  int nOffsets[3]= {0,0,0};
  int pOffsets[3]= {0,0,0};

  int nimages;
  size_t v = 0;
  size_t i;

  ImageType output_type = TYPE_UNKNOWN;



  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );

  /* some initializations
   */
  winDim[0] = par->window.x;
  winDim[1] = par->window.y;
  winDim[2] = par->window.z;
  nOffsets[0] = -(int)(winDim[0] / 2); pOffsets[0] = winDim[0] - 1 + nOffsets[0];
  nOffsets[1] = -(int)(winDim[1] / 2); pOffsets[1] = winDim[1] - 1 + nOffsets[1];
  nOffsets[2] = -(int)(winDim[2] / 2); pOffsets[2] = winDim[2] - 1 + nOffsets[2];

  VT_Image ( &imTotalSum );
  VT_Image ( &imTotalSumSqr );
  VT_Image ( &imTotalRes );

  VT_Image ( &imLocalSum );
  VT_Image ( &imLocalSumSqr );
  VT_Image ( &imLocalRes );

  /* we can go
     il y aurait sans doute des ameliorations a faire dans le cas d'une fenetre 1x1x1
   */

#define _DEALLOCATIONS_STREAMING_WITHOUTMASKS_ { \
  VT_FreeImage ( &imTotalSum );      \
  VT_FreeImage ( &imTotalSumSqr );   \
  VT_FreeImage ( &imTotalRes );      \
                                     \
  VT_FreeImage ( &imLocalSum );      \
  VT_FreeImage ( &imLocalSumSqr );   \
  VT_FreeImage ( &imLocalRes );      \
                                     \
  if ( imLocal != NULL ) {           \
    VT_FreeImage( imLocal );         \
    VT_Free( (void**)&imLocal );     \
  }                                  \
}


  /* loop over images
   */
  for ( nimages = 0; nimages < imageFileList->n; nimages++ ) {

    if ( _verbose_ >= 2 )
      fprintf( stderr, "%d: image='%s'\n", nimages, imageFileList->data[nimages] );

    /* read input image #nimages
     */
    imLocal = _VT_Inrimage( imageFileList->data[nimages] );
    if ( imLocal == (vt_image*)NULL ) {
      _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
      if ( _verbose_ )
        fprintf( stderr, "%s: error when opening %s\n", proc, imageFileList->data[nimages] );
      return( -1 );
    }



    /* first reading: allocations and initialisations of auxiliary images
     */
    if ( nimages == 0 ) {

      /* allocations
       */
      if ( _AllocAuxiliaryImages( &imTotalRes, &imTotalSum, &imTotalSumSqr,
                                  imLocal, output_name, par->operation ) != 1 ) {
        _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate global auxiliary images\n", proc);
        return( -1 );
      }

      if ( winDim[0] > 1 || winDim[1] > 1 || winDim[2] > 1 ) {
        if ( _AllocAuxiliaryImages( &imLocalRes, &imLocalSum, &imLocalSumSqr,
                                    imLocal, output_name, par->operation ) != 1 ) {
          _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to allocate local auxiliary images\n", proc);
          return( -1 );
        }
      }



      theDim[0] = imLocal->dim.v * imLocal->dim.x;
      theDim[1] = imLocal->dim.y;
      theDim[2] = imLocal->dim.z;
      v = (size_t)theDim[0] * (size_t)theDim[1] * (size_t)theDim[2];



      /* initialisations
       */
      if ( winDim[0] == 1 && winDim[1] == 1 && winDim[2] == 1 ) {

        switch ( par->operation ) {
        default :
          _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
          if ( _verbose_ )
            fprintf( stderr, "%s: such operation not handled yet\n", proc );
          return( -1 );
        case _MAX_ :
        case _MIN_ :
          if ( ConvertBuffer( imLocal->buf, imLocal->type,
                              imTotalRes.buf, imTotalRes.type, v ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during conversion (1)\n", proc );
            return( -1 );
          }
          break;
        case _STDDEV_ :
        case _VAR_ :
          if ( sqrImage( imLocal->buf, imLocal->type,
                         imTotalSumSqr.buf, imTotalSumSqr.type,
                         theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during stddev/var init (1)\n", proc );
            return( -1 );
          }
        case _SUM_ :
        case _MEAN_ :
          if ( ConvertBuffer( imLocal->buf, imLocal->type,
                              imTotalSum.buf, imTotalSum.type, v ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during conversion (1)\n", proc );
            return( -1 );
          }
          break;
        }

      }
      else {

        switch ( par->operation ) {
        default :
          _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
          if ( _verbose_ )
            fprintf( stderr, "%s: such operation not handled yet\n", proc );
          return( -1 );
        case _MAX_ :
          if ( maxFiltering( imLocal->buf, imLocal->type,
                             imTotalRes.buf, imTotalRes.type,
                             theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during max filtering (init)\n", proc );
            return( -1 );
          }
          break;
        case _MIN_ :
          if ( minFiltering( imLocal->buf, imLocal->type,
                             imTotalRes.buf, imTotalRes.type,
                             theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during min filtering (init)\n", proc );
            return( -1 );
          }
          break;
        case _STDDEV_ :
        case _VAR_ :
          if ( sumSquaresFiltering( imLocal->buf, imLocal->type,
                                    imTotalSumSqr.buf, imTotalSumSqr.type,
                                    theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during sum of squares filtering (init)\n", proc );
            return( -1 );
          }
          break;
        case _SUM_ :
        case _MEAN_ :
          if ( sumFiltering( imLocal->buf, imLocal->type,
                             imTotalSum.buf, imTotalSum.type,
                             theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during sum filtering (init)\n", proc );
            return( -1 );
          }
          break;
        }

      }

      output_type = par->output_type;
      if ( output_type == TYPE_UNKNOWN ) {
          switch( par->operation ) {
          case _MAX_ :
          case _MIN_ :
          case _MEDIAN_ :
          case _QUANTILE_ :
              output_type = imLocal->type;
              break;
          case _MEAN_ :
          case _ROBUST_MEAN_ :
              output_type = imLocal->type;
              break;
          case _STDDEV_ :
          case _SUM_ :
          case _VAR_ :
          default :
              output_type = FLOAT;
              break;
          }
      }

    }

    /* other images case
     */
    else {

      if ( winDim[0] == 1 && winDim[1] == 1 && winDim[2] == 1 ) {

        switch ( par->operation ) {
        default :
          _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
          if ( _verbose_ )
            fprintf( stderr, "%s: such operation not handled yet\n", proc );
          return( -1 );
        case _MAX_ :
          if ( maxImages( imLocal->buf, imLocal->type,
                          imTotalRes.buf, imTotalRes.type,
                          imTotalRes.buf, imTotalRes.type,
                          theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during max filtering\n", proc );
            return( -1 );
          }
          break;
        case _MIN_ :
          if ( minImages( imLocal->buf, imLocal->type,
                          imTotalRes.buf, imTotalRes.type,
                          imTotalRes.buf, imTotalRes.type,
                          theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during min filtering\n", proc );
            return( -1 );
          }
          break;
        case _STDDEV_ :
        case _VAR_ :
          if ( _addSqrImage( imTotalSumSqr.buf, imTotalSumSqr.type,
                             imLocal->buf, imLocal->type,
                             imTotalSumSqr.buf, imTotalSumSqr.type,
                             theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during sum of squares filtering\n", proc );
            return( -1 );
          }
        case _SUM_ :
        case _MEAN_ :
          if ( addImages( imLocal->buf, imLocal->type,
                          imTotalSum.buf, imTotalSum.type,
                          imTotalSum.buf, imTotalSum.type,
                          theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during sum filtering\n", proc );
            return( -1 );
          }
          break;
        }

      }
      else {

        switch ( par->operation ) {
        default :
          _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
          if ( _verbose_ )
            fprintf( stderr, "%s: such operation not handled yet\n", proc );
          return( -1 );
        case _MAX_ :
          if ( maxFiltering( imLocal->buf, imLocal->type,
                             imLocalRes.buf, imLocalRes.type,
                             theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during local max filtering\n", proc );
            return( -1 );
          }
          if ( maxImages( imLocalRes.buf, imLocalRes.type,
                          imTotalRes.buf, imTotalRes.type,
                          imTotalRes.buf, imTotalRes.type,
                          theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during max filtering\n", proc );
            return( -1 );
          }
          break;
        case _MIN_ :
          if ( minFiltering( imLocal->buf, imLocal->type,
                             imLocalRes.buf, imLocalRes.type,
                             theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during local min filtering\n", proc );
            return( -1 );
          }
          if ( minImages( imLocalRes.buf, imLocalRes.type,
                          imTotalRes.buf, imTotalRes.type,
                          imTotalRes.buf, imTotalRes.type,
                          theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during min filtering\n", proc );
            return( -1 );
          }
          break;
        case _STDDEV_ :
        case _VAR_ :
          if ( sumSquaresFiltering( imLocal->buf, imLocal->type,
                                    imLocalSumSqr.buf, imLocalSumSqr.type,
                                    theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during local sum of squares filtering\n", proc );
            return( -1 );
          }
          if ( addImages( imLocalSumSqr.buf, imLocalSumSqr.type,
                          imTotalSumSqr.buf, imTotalSumSqr.type,
                          imTotalSumSqr.buf, imTotalSumSqr.type,
                          theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during sum of squares filtering\n", proc );
            return( -1 );
          }
        case _SUM_ :
        case _MEAN_ :
          if ( sumFiltering( imLocal->buf, imLocal->type,
                             imLocalSum.buf, imLocalSum.type,
                             theDim, winDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during local sum filtering\n", proc );
            return( -1 );
          }
          if ( addImages( imLocalSum.buf, imLocalSum.type,
                          imTotalSum.buf, imTotalSum.type,
                          imTotalSum.buf, imTotalSum.type,
                          theDim ) != 1 ) {
            _DEALLOCATIONS_STREAMING_WITHOUTMASKS_;
            if ( _verbose_ )
              fprintf( stderr, "%s: error during sum filtering\n", proc );
            return( -1 );
          }
          break;
        }

      }

    }



    if ( imLocal != NULL ) {
      VT_FreeImage( imLocal );
      VT_Free( (void**)&imLocal );
    }

  }
  /* end of loop over images
   */



  /* processing is over
   */

  VT_FreeImage ( &imLocalSum );
  VT_FreeImage ( &imLocalSumSqr );
  VT_FreeImage ( &imLocalRes );



 /* last computations
   */
  switch ( par->operation ) {
  default :
    VT_FreeImage ( &imTotalSumSqr );
    VT_FreeImage ( &imTotalSum );
    VT_FreeImage ( &imTotalRes );
    if ( _verbose_ )
      fprintf( stderr, "%s: such operation not handled yet\n", proc );
    return( -1 );
  case _MAX_ :
  case _MIN_ :
    imResult = &imTotalRes;
    break;

  case _SUM_ :
    imResult = &imTotalSum;
    break;

  case _VAR_ :
    imResult = &imTotalSum;
    v = (size_t)imResult->dim.v * (size_t)imResult->dim.x * (size_t)imResult->dim.y * (size_t)imResult->dim.z;
    bufTotalSum = (float*)imTotalSum.buf;
    bufTotalSumSqr = (float*)imTotalSumSqr.buf;
    for ( i=0; i<v; i++ ) {
      bufTotalSum[i] /= (float)imageFileList->n;
      bufTotalSumSqr[i] /= (float)imageFileList->n;
      bufTotalSum[i] = bufTotalSumSqr[i] - bufTotalSum[i] * bufTotalSum[i];
    }
    break;

  case _STDDEV_ :
    imResult = &imTotalSum;
    v = (size_t)imResult->dim.v * (size_t)imResult->dim.x * (size_t)imResult->dim.y * (size_t)imResult->dim.z;
    bufTotalSum = (float*)imTotalSum.buf;
    bufTotalSumSqr = (float*)imTotalSumSqr.buf;
    for ( i=0; i<v; i++ ) {
      bufTotalSum[i] /= (float)imageFileList->n;
      bufTotalSumSqr[i] /= (float)imageFileList->n;
      bufTotalSum[i] = sqrt( bufTotalSumSqr[i] - bufTotalSum[i] * bufTotalSum[i] );
    }
    break;

  case _MEAN_ :
    imResult = &imTotalSum;
    v = (size_t)imResult->dim.v * (size_t)imResult->dim.x * (size_t)imResult->dim.y * (size_t)imResult->dim.z;
    bufTotalSum = (float*)imTotalSum.buf;
    for ( i=0; i<v; i++ ) {
      bufTotalSum[i] /= (float)imageFileList->n;
    }
    break;
  }



  VT_FreeImage ( &imTotalSumSqr );



  if ( output_type == TYPE_UNKNOWN || output_type == imResult->type ) {
    if ( VT_WriteInrimage( imResult ) == -1 ) {
      VT_FreeImage ( &imTotalSum );
      VT_FreeImage ( &imTotalRes );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to write output image\n", proc );
      return( -1 );
    }
  }
  else {
     VT_InitFromImage( &imTotalSumSqr, imResult, par->output_name, output_type );
    if ( VT_AllocImage( &imTotalSumSqr ) != 1 ) {
      VT_FreeImage ( &imTotalSum );
      VT_FreeImage ( &imTotalRes );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary result image\n", proc );
      return( -1 );
    }
    v = (size_t)imResult->dim.v * (size_t)imResult->dim.x * (size_t)imResult->dim.y * (size_t)imResult->dim.z;
    if ( ConvertBuffer( imResult->buf, imResult->type,
                   imTotalSumSqr.buf, imTotalSumSqr.type,
                        v ) != 1 ) {
      VT_FreeImage ( &imTotalSumSqr );
      VT_FreeImage ( &imTotalSum );
      VT_FreeImage ( &imTotalRes );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to convert image\n", proc );
      return( -1 );
    }
    if ( VT_WriteInrimage( &imTotalSumSqr ) == -1 ) {
      VT_FreeImage ( &imTotalSumSqr );
      VT_FreeImage ( &imTotalSum );
      VT_FreeImage ( &imTotalRes );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to write output image\n", proc );
      return( -1 );
    }
    VT_FreeImage ( &imTotalSumSqr );
  }

  VT_FreeImage ( &imTotalSum );
  VT_FreeImage ( &imTotalRes );

  return( 1 );
}





static int _StreamingComputationWithMasks( stringList *imageFileList,
                                           stringList *maskFileList,
                                           char *output_name,
                                           lineCmdParamMeanImages *par )
{
  char *proc = "_StreamingComputationWithMasks";

  vt_image *imLocal = (vt_image*)NULL;
  vt_image *imLocalMask = (vt_image*)NULL;

  vt_image imTotalSum;
  vt_image imTotalSumSqr;
  vt_image imTotalRes;
  vt_image imTotalMask;
  vt_image imTmp;

  int nimages;

  ImageType output_type = TYPE_UNKNOWN;



  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );

  VT_Image ( &imTotalSum );
  VT_Image ( &imTotalSumSqr );
  VT_Image ( &imTotalRes );
  VT_Image ( &imTotalMask );

  if ( maskFileList == (stringList*)NULL || maskFileList->n == 0 ) {
    return ( _StreamingComputationWithoutMasks( imageFileList, output_name, par ) );
  }



  if ( par->window.x != 1 || par->window.y != 1 || par->window.z != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: computation with window > 1x1x1 not handled yet\n", proc );
    return( -1 );
  }



  /* we can go
   */

#define _DEALLOCATIONS_STREAMING_WITHMASKS_ { \
  VT_FreeImage ( &imTotalSum );      \
  VT_FreeImage ( &imTotalSumSqr );   \
  VT_FreeImage ( &imTotalRes );      \
  VT_FreeImage ( &imTotalMask );     \
                                     \
  if ( imLocal != NULL ) {           \
    VT_FreeImage( imLocal );         \
    VT_Free( (void**)&imLocal );     \
  }                                  \
  if ( imLocalMask != NULL ) {       \
    VT_FreeImage( imLocalMask );     \
    VT_Free( (void**)&imLocalMask ); \
  }                                  \
}



  /* loop over images
   */
  for ( nimages = 0; nimages < imageFileList->n; nimages++ ) {

    if ( _verbose_ >= 2 )
      fprintf( stderr, "%d: image='%s'\n", nimages, imageFileList->data[nimages] );

    /* read input image #nimages
     */
    imLocal = _VT_Inrimage( imageFileList->data[nimages] );
    if ( imLocal == (vt_image*)NULL ) {
      _DEALLOCATIONS_STREAMING_WITHMASKS_;
      if ( _verbose_ )
        fprintf( stderr, "%s: error when opening %s\n", proc, imageFileList->data[nimages] );
      return( -1 );
    }

    imLocalMask = _VT_Inrimage( maskFileList->data[nimages] );
    if ( imLocalMask == (vt_image*)NULL ) {
      _DEALLOCATIONS_STREAMING_WITHMASKS_;
      if ( _verbose_ )
        fprintf( stderr, "%s: error when opening %s\n", proc, maskFileList->data[nimages]  );
      return( -1 );
    }



    /* first reading : allocations of auxiliary images
     */
    if ( nimages == 0 ) {

      if ( _AllocInitAuxiliaryWithMasks( &imTotalRes, &imTotalSum,
                                      &imTotalSumSqr, &imTotalMask,
                                      imLocal, imLocalMask,
                                      output_name, par->operation ) != 1 ) {
        _DEALLOCATIONS_STREAMING_WITHMASKS_;
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate or initialize auxiliary images\n", proc );
        return( -1 );
      }

      output_type = par->output_type;
      if ( output_type == TYPE_UNKNOWN ) {
          switch( par->operation ) {
          case _MAX_ :
          case _MIN_ :
          case _MEDIAN_ :
          case _QUANTILE_ :
              output_type = imLocal->type;
              break;
          case _MEAN_ :
          case _ROBUST_MEAN_ :
              output_type = imLocal->type;
              break;
          case _STDDEV_ :
          case _SUM_ :
          case _VAR_ :
          default :
              output_type = FLOAT;
              break;
          }
      }

    }

    else {

      if ( _UpdateAuxiliaryWithMasks( &imTotalRes, &imTotalSum,
                                   &imTotalSumSqr, &imTotalMask,
                                   imLocal, imLocalMask,
                                   par->operation ) != 1 ) {
        _DEALLOCATIONS_STREAMING_WITHMASKS_;
        if ( _verbose_ ) {
          fprintf( stderr, "%s: unable to update auxiliary images\n", proc );
          fprintf( stderr, "\t while processing image #%d '%s'\n", nimages, imageFileList->data[nimages] );
        }
        return( -1 );
      }

    }

    if ( imLocal != NULL ) {
      VT_FreeImage( imLocal );
      VT_Free( (void**)&imLocal );
    }
    if ( imLocalMask != NULL ) {
      VT_FreeImage( imLocalMask );
      VT_Free( (void**)&imLocalMask );
    }

  }
  /* end of loop over images
   */




  /* last computations
   * _ResultFromAuxiliaryWithMasks() return either imTotalRes or imTotalSum
   * in case of success
   */
  imLocal = _ResultFromAuxiliaryWithMasks( &imTotalRes, &imTotalSum,
                                           &imTotalSumSqr, &imTotalMask, par->operation );

  VT_FreeImage ( &imTotalSumSqr );
  VT_FreeImage ( &imTotalMask );

  if ( imLocal == (vt_image*)NULL ) {
      VT_FreeImage ( &imTotalSum );
      VT_FreeImage ( &imTotalRes );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to get result\n", proc );
      return( -1 );
  }

  if ( imLocal->type == output_type ) {

      if ( VT_WriteInrimage( imLocal ) == -1 ) {
          VT_FreeImage ( &imTotalSum );
          VT_FreeImage ( &imTotalRes );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write output image\n", proc );
          return( -1 );
      }

  }
  else {

      VT_InitFromImage( &imTmp, imLocal, output_name, output_type );
      if ( VT_AllocImage( &imTmp ) != 1 ) {
          VT_FreeImage ( &imTotalSum );
          VT_FreeImage ( &imTotalRes );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to allocate auxiliary result image\n", proc );
          return( -1 );
      }
      if ( VT_CopyImage( imLocal, &imTmp ) != 1 ) {
          VT_FreeImage( &imTmp );
          VT_FreeImage ( &imTotalSum );
          VT_FreeImage ( &imTotalRes );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to copy auxiliary result image\n", proc );
          return( -1 );
      }
      if ( VT_WriteInrimage( &imTmp ) == -1 ) {
          VT_FreeImage( &imTmp );
          VT_FreeImage ( &imTotalSum );
          VT_FreeImage ( &imTotalRes );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write output image\n", proc );
          return( -1 );
      }
      VT_FreeImage( &imTmp );

  }

  VT_FreeImage ( &imTotalSum );
  VT_FreeImage ( &imTotalRes );

  return( 1 );
}

