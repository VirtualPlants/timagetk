/*************************************************************************
 * api-composeTrsf.c -
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
 * sed -e "s/composeTrsf/execuTable/g" \
 *     -e "s/ComposeTrsf/ExecuTable/g" \
 *     -e "s/composetrsf/executable/g" \
 *     [api-]composeTrsf.[c,h] > [api-]execTable.[c,h]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>


#include <bal-transformation-tools.h>

#include <api-composeTrsf.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_composeTrsf( char *str, lineCmdParamComposeTrsf *p );


static void _FreeTransformationList( bal_transformation **array, int n );
static bal_transformation ** _ReadTransformationList( stringList *list );


static int _StreamingComputation( stringList *trsf_names,
                                  char *output_name,
                                  lineCmdParamComposeTrsf *par );





/************************************************************
 *
 * main API
 *
 ************************************************************/

/* this one is kept for historical reasons,
 * ie Stracking compilation but should disappear
 */
int composeTrsf( char *restrsf_name,
                 char *template_image_name,
                 bal_integerPoint dim,
                 bal_doublePoint voxel,
                 /*
                  * int first_trsf,
                  * int last_trsf,
                  */
                 char** argv,
                 int argc,
                 int* is_a_trsf,
                 int isDebug,
                 int isVerbose )
{
    char *proc = "composeTrsf";
    char str[256], *s;

    stringList input_names;
    int i;

    _debug_ = isDebug;

    _verbose_ = isVerbose;
    BAL_SetVerboseInBalImage( isVerbose );
    BAL_SetVerboseInBalTransformation( isVerbose );
    BAL_SetVerboseInBalTransformationTools( isVerbose );

    if ( _verbose_ )
        fprintf( stderr, "Warning, '%s' is obsolete\n", proc );

    initStringList( &input_names );
    for ( i=0; i<argc; i++ ) {
        if ( is_a_trsf[i] ) {
            if ( addStringToList( argv[i], &input_names ) != 1 ) {
                if ( _verbose_ )
                    fprintf( stderr, "%s: unable to add '%s' to transformation list\n", proc, argv[i] );
                return( -1 );
            }
        }
    }

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

    if ( API_INTERMEDIARY_composeTrsf( &input_names, restrsf_name,
                                       template_image_name,
                                       str, (char*)NULL ) != 1 ) {
        freeStringList( &input_names );
        if ( _verbose_ )
            fprintf( stderr, "%s: some error occurs\n", proc );
        return( -1 );
    }

    freeStringList( &input_names );

    return( 0 );
}





int API_INTERMEDIARY_composeTrsf( stringList *trsf_names,
                                  char *output_name,
                                  char* template_image_name,
                                  char *param_str_1, char *param_str_2 )
{
    char *proc = "API_INTERMEDIARY_composeTrsf";
    lineCmdParamComposeTrsf par;

    bal_transformation **trsfsStructure = (bal_transformation**)NULL;
    bal_transformation resTrsf;
    bal_image templateImage;
    bal_image *templatePtr = (bal_image*)NULL;



    if ( trsf_names == (stringList*)NULL || trsf_names->n <= 0 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: no input transformation names?!", proc );
        return( -1 );
    }



    /* parameter initialization
     */
    API_InitParam_composeTrsf( &par );

    /* parameter parsing
     */
    if ( param_str_1 != (char*)NULL )
        _API_ParseParam_composeTrsf( param_str_1, &par );
    if ( param_str_2 != (char*)NULL )
        _API_ParseParam_composeTrsf( param_str_2, &par );


    switch( par.computation ) {
    default :
        if ( par.print_lineCmdParam )
            API_PrintParam_composeTrsf( stderr, proc, &par, (char*)NULL );
        API_FreeParam_composeTrsf( &par );
        if ( _verbose_ )
            fprintf( stderr, "%s: unknown computation type, this is embarrasing ...\n", proc );
        return( -1 );

    case _MEMORY_ :

        BAL_InitTransformation( &resTrsf );

        /* reading input transformations
         */
        trsfsStructure = _ReadTransformationList( trsf_names );
        if ( trsfsStructure == (bal_transformation**)NULL ) {
            API_FreeParam_composeTrsf( &par );
            if ( _verbose_ )
                fprintf( stderr, "%s: error when reading transformation \n", proc );
            return( -1 );
        }

        /* is there any template geometry ?
         */

        /* initializing result image
         * - with reference image, if any
         */

        if ( template_image_name != NULL && template_image_name[0] != '\0' ) {
          if ( BAL_ReadImage( &templateImage, template_image_name, 1 ) != 1 ) {
              _FreeTransformationList( trsfsStructure, trsf_names->n );
              API_FreeParam_composeTrsf( &par );
              if ( _verbose_ )
                fprintf( stderr, "%s: unable to read '%s'\n", proc, template_image_name );
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
                _FreeTransformationList( trsfsStructure, trsf_names->n );
                API_FreeParam_composeTrsf( &par );
                if ( _verbose_ )
                  fprintf( stderr, "%s: unable to initialize auxiliary image\n", proc );
                return( -1 );
            }
          }
          else {
            if ( BAL_InitImage( &templateImage, (char*)NULL, par.template_dim.x, par.template_dim.y, 1, 1, UCHAR ) != 1 ) {
                _FreeTransformationList( trsfsStructure, trsf_names->n );
                API_FreeParam_composeTrsf( &par );
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

        if ( BAL_AllocTransformationListComposition( &resTrsf, trsfsStructure,
                                                     trsf_names->n, templatePtr ) != 1 ) {
             if ( templatePtr != (bal_image*)NULL ) {
                 BAL_FreeImage( &templateImage );
                 templatePtr = (bal_image*)NULL;
             }
             _FreeTransformationList( trsfsStructure, trsf_names->n );
             API_FreeParam_composeTrsf( &par );
             if ( _verbose_ )
               fprintf( stderr, "%s: unable to allocate result transformation\n", proc );
             return( -1 );
        }

        if ( templatePtr != (bal_image*)NULL ) {
            BAL_FreeImage( &templateImage );
            templatePtr = (bal_image*)NULL;
        }

        /************************************************************
         *
         *  here is the stuff
         *
         ************************************************************/

        if ( API_composeTrsf( trsfsStructure, trsf_names->n, &resTrsf,
                              param_str_1, param_str_2 ) !=1 ) {
            BAL_FreeTransformation( &resTrsf );
            _FreeTransformationList( trsfsStructure, trsf_names->n );
            API_FreeParam_composeTrsf( &par );
            if ( _verbose_ )
                fprintf( stderr, "%s: an error occurs during 'in memory' procedures\n", proc );
            return( -1 );
        }

        _FreeTransformationList( trsfsStructure, trsf_names->n );

        if ( BAL_WriteTransformation( &resTrsf, output_name ) != 1 ) {
            BAL_FreeTransformation( &resTrsf );
            API_FreeParam_composeTrsf( &par );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to write result transformation '%s'\n",
                       proc, output_name );
            return( -1 ) ;
        }

        BAL_FreeTransformation( &resTrsf );

        break;

    case _STREAMING_ :

        if ( par.print_lineCmdParam )
            API_PrintParam_composeTrsf( stderr, proc, &par, (char*)NULL );

        if ( _StreamingComputation( trsf_names, output_name, &par ) != 1 ) {
            API_FreeParam_composeTrsf( &par );
            if ( _verbose_ )
                fprintf( stderr, "%s: an error occurs during 'streaming' procedure\n", proc );
            return( -1 );
        }

        break;
    }

    API_FreeParam_composeTrsf( &par );

    return( 1 );
}





/* when passing from linear transformations (matrices) to non-linear
 * ones (vector field), an image geometry is required. It can either be
 * defined in the parameters or by an existing image.
 *
 * I've prefered to pass this image (if any) as a parameter to avoid any
 * I/O (reading) in the procedure. An other solution could have been
 * to already define the resulting transformation as a vector field, but
 * - it may not yield the same results than the first implementation
 *   (matrice product is changed into matrice x vector field product)
 * - the type of the vector field has to be set (2D or 3D), and a 3D
 *   vector field my be used to encode a 2D vector field defined onto
 *   an image (if 3D is set as default for 3D images).
 *
 * Whatever, if no image template is given, the vector field geometry is used,
 * which may be plenty satisfactory.
 */

int API_composeTrsf( bal_transformation **trsfsStructure,
                     int n,
                     bal_transformation *resTrsf,
                     char *param_str_1, char *param_str_2 )
{
  char *proc = "API_composeTrsf";
  lineCmdParamComposeTrsf par;



  if ( trsfsStructure == (bal_transformation**)NULL
       || n <= 0 ) {
      if ( _verbose_ >= 2 )
          fprintf( stderr, "%s: no input transformations\n", proc );
      return( -1 );
  }



  /* parameter initialization
   */
  API_InitParam_composeTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_composeTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_composeTrsf( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_composeTrsf( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( BAL_TransformationListComposition( resTrsf, trsfsStructure, n ) != 1 ) {
      API_FreeParam_composeTrsf( &par );
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute transformation composition\n", proc );
      return( -1 );
  }

  API_FreeParam_composeTrsf( &par );

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
 [[-transformations|-trsfs %s %s ...] | ...\n\
  [-transformation-list|-trsf-list|-list %s] | ...\n\
  [-transformation-format|-trsf-format|-format %s]]\n\
 [-first|-f %d] [-last|-l %d]\n\
 [-template-image|-template|-dims|-t %s]\n\
 [-template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]]\n\
 [-x %d] [-y %d] [-z %d]\n\
 [-template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]]\n\
 [-computation streaming|memory]\n\
 [-streaming | -memory]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [output-image-type | -type s8|u8|s16|u16...]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 \n\
 Transformations are composed in the order they are given.\n\
 The line '-trsfs T1 T2 ... TN' assumes that transformation\n\
 Ti goes from image I(i+1) to I(i) [then allows to resample\n\
 I(i) in the same frame than I(i+1)], ie  Ti = T_{I(i) <- I(i+1)}.\n\
 The resulting transformation will goes from I(N+1) to I(1)\n\
 [then allows to resample I(1) in the same frame than I(N+1)]\n\
\n\
 if 'transformation-out' is not specified or equal to '-', stdout will be used\n\
# data\n\
 -transformations|-trsfs %s %s ...: all names following '-transformations'\n\
    are considered as input transformations, until a '-' is encountered as\n\
    first character\n\
 -transformation-list|-trsf-list|-list %s: text file containing a list\n\
    of transformation file names\n\
 -transformation-format|-trsf-format|-format %s: format 'a la printf' of\n\
    transformations to be processed\n\
    should contain a '%d', to be used with '-first' and '-last'\n\
 -first|-f %d: first value of index for 'formats'\n\
 -last|-l %d:  last value of index for 'formats'\n\
 -res %s: output transformation\n\
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





char *API_Help_composeTrsf( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_composeTrsf( char *program, char *str, int flag )
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



void API_InitParam_composeTrsf( lineCmdParamComposeTrsf *p )
{
    initStringList( &(p->input_names) );
    (void)strncpy( p->input_list, "\0", 1 );
    (void)strncpy( p->input_format, "\0", 1 );

    p->firstindex = 0;
    p->lastindex = 0;

    (void)strncpy( p->output_name, "\0", 1 );

    (void)strncpy( p->template_name, "\0", 1 );

    p->template_dim.x = 0;
    p->template_dim.y = 0;
    p->template_dim.z = 0;

    p->template_voxel.x = -1.0;
    p->template_voxel.y = -1.0;
    p->template_voxel.z = -1.0;

    p->computation = _MEMORY_;

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}



void API_FreeParam_composeTrsf( lineCmdParamComposeTrsf *p )
{
    freeStringList( &(p->input_names) );
    API_InitParam_composeTrsf( p );
}





void API_PrintParam_composeTrsf( FILE *theFile, char *program,
                                  lineCmdParamComposeTrsf *p, char *str )
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

  fprintf( f, "- first index for format = %d\n", p->firstindex );
  fprintf( f, "- last index for format = %d\n", p->lastindex );

  fprintf( f, "- output image is " );
  if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "# template for output image geometry\n" );

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
  fprintf( f, "- computation type is " );
  switch( p->computation ) {
  default :            fprintf( f, "unknown, this is embarrasing ...\n" ); break;
  case _MEMORY_ :      fprintf( f, "memory\n" ); break;
  case _STREAMING_ :   fprintf( f, "streaming\n" ); break;
  }

  fprintf( f, "==================================================\n" );
}





/************************************************************
 *
 * parameters parsing
 *
 ************************************************************/



static void _API_ParseParam_composeTrsf( char *str, lineCmdParamComposeTrsf *p )
{
  char *proc = "_API_ParseParam_composeTrsf";
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

  API_ParseParam_composeTrsf( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_composeTrsf( int firstargc, int argc, char *argv[],
                                  lineCmdParamComposeTrsf *p )
{
  char *proc = "API_ParseParam_composeTrsf";
  int i;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;



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
              API_ErrorParse_composeTrsf( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* names
           */
          else if ( (strcmp ( argv[i], "-transformations" ) == 0 && argv[i][16] == '\0')
                    || (strcmp ( argv[i], "-trsfs" ) == 0 && argv[i][6] == '\0') ) {
              i++;
              while ( i<argc && argv[i][0] != '-' ) {
                  if ( addStringToList( argv[i], &(p->input_names) ) != 1 ) {
                      if ( _verbose_ )
                          fprintf( stderr, "%s: unable to add '%s' to transformation list\n", proc, argv[i] );
                      API_ErrorParse_composeTrsf( (char*)NULL, "parsing -transformations ...\n", 0 );
                  }
                i++;
              }
              i--;
          }
          else if ( strcmp ( argv[i], "-transformation-list" ) == 0
                    || (strcmp ( argv[i], "-trsf-list" ) == 0 && argv[i][10] == '\0')
                    || (strcmp ( argv[i], "-list" ) == 0 && argv[i][5] == '\0') ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_composeTrsf( (char*)NULL, "parsing -transformation-list ...\n", 0 );
              (void)strcpy( p->input_list, argv[i] );
          }
          else if ( strcmp ( argv[i], "-transformation-format" ) == 0
                    || (strcmp ( argv[i], "-trsf-format" ) == 0 && argv[i][12] == '\0')
                    || (strcmp ( argv[i], "-format" ) == 0 && argv[i][7] == '\0') ) {
            i++;
            if ( i >= argc )
                API_ErrorParse_composeTrsf( (char*)NULL, "parsing -transformation-format ...\n", 0 );
            (void)strcpy( p->input_format, argv[i] );
          }

          else if ( (strcmp ( argv[i], "-first" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-f" ) == 0 && argv[i][2] == '\0') ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_composeTrsf( (char*)NULL, "parsing -first ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->firstindex) );
              if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -first ...\n", 0 );
          }
          else if ( (strcmp ( argv[i], "-last" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "-l" ) == 0 && argv[i][2] == '\0') ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_composeTrsf( (char*)NULL, "parsing -last ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->lastindex) );
              if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -last ...\n", 0 );
          }

          else if ( strcmp ( argv[i], "-res" ) == 0  && argv[i][4] == '\0' ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_composeTrsf( (char*)NULL, "parsing -res ...\n", 0 );
              if ( outputisread )
                  API_ErrorParse_composeTrsf( (char*)NULL, "parsing -res: output already parsed ...\n", 0 );
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }

          /* template
           */

          else if ( strcmp ( argv[i], "-template-image") == 0
                    || (strcmp ( argv[i], "-template") == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-image", 0 );
                 (void)strcpy( p->template_name, argv[i] );
          }
          else if ( strcmp ( argv[i], "-template-dimensions") == 0
                    || strcmp ( argv[i], "-template-dimension") == 0
                    || strcmp ( argv[i], "-template-dim") == 0
                    || strcmp ( argv[i], "-dimensions") == 0
                    || strcmp ( argv[i], "-dimension") == 0
                    || (strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.x) );
            if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.y) );
            if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
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
              if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -x ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.x) );
              if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -x ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-y") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -y ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.y) );
              if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -y ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-z") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -z ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -z ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-template-voxel") == 0
                    || strcmp ( argv[i], "-voxel-size") == 0
                    || (strcmp (argv[i], "-voxel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-pixel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-vs" ) == 0 && argv[i][3] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
            if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.y) );
            if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
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

          /* computation / processing
           */
          else if ( strcmp ( argv[i], "-computation" ) == 0 ) {
              i++;
              if ( i >= argc )
                  API_ErrorParse_composeTrsf( (char*)NULL, "parsing -computation ...\n", 0 );
              if ( strcmp ( argv[i], "memory" ) == 0 ) {
                  p->computation = _MEMORY_;
              }
              else if ( strcmp ( argv[i], "streaming" ) == 0 ) {
                  p->computation = _STREAMING_;
              }
              else {
                  API_ErrorParse_composeTrsf( (char*)NULL, "parsing -computation: unknown mode ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_composeTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_composeTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_composeTrsf( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_composeTrsf( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }

          /* general image related parameters
           */

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_composeTrsf( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_composeTrsf( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              BAL_IncrementVerboseInBalImage();
              BAL_IncrementVerboseInBalTransformation();
              BAL_IncrementVerboseInBalTransformationTools();
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              _verbose_ = 0;
              BAL_SetVerboseInBalImage( 0 );
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
              API_ErrorParse_composeTrsf( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_composeTrsf( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_composeTrsf( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
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










/**************************************************
 *
 * transformation array
 *
 **************************************************/

static void _FreeTransformationList( bal_transformation **array, int n )
{
  int i;

  if ( array == (bal_transformation**)NULL ) return;
  if ( n <= 0 ) return;

  for ( i=0; i<n; i++ ) {
    if ( array[i] != (bal_transformation*)NULL ) {
      BAL_FreeTransformation( array[i] );
      free( array[i] );
    }
  }
}



static bal_transformation ** _ReadTransformationList( stringList *list )
{
  char *proc = "_ReadTransformationList";
  bal_transformation **a = (bal_transformation**)NULL;
  int i;

  if ( list->n <= 0 ) return( (bal_transformation**)NULL );

  a = (bal_transformation**)malloc( list->n * sizeof( bal_transformation* ) );
  if ( a == (bal_transformation**)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate array\n", proc );
    return( (bal_transformation**)NULL );
  }

  for ( i=0; i<list->n; i++ ) a[i] = (bal_transformation*)NULL;

  for ( i=0; i<list->n; i++ ) {
      a[i] = (bal_transformation*)malloc( sizeof(bal_transformation) );
      if ( a[i] == (bal_transformation*)NULL ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to allocate transformation #%d\n",
                     proc, i );
          _FreeTransformationList( a, list->n );
          free( a );
          return( (bal_transformation**)NULL );
      }
      if ( BAL_ReadTransformation( a[i], list->data[i] ) != 1 ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to read transformation #%d '%s'\n",
                     proc, i, list->data[i] );
          _FreeTransformationList( a, list->n );
          free( a );
          return( (bal_transformation**)NULL );
      }
  }

  return( a );
}







/**************************************************
 *
 * streaming computation
 *
 **************************************************/

static int _StreamingComputation( stringList *trsf_names,
                                  char *output_name,
                                  lineCmdParamComposeTrsf *par )
{
    char *proc = "_StreamingComputation";

    int i;
    bal_transformation theTrsf;
    bal_transformation *resTrsf;
    bal_transformation tmp1Trsf;
    bal_transformation tmp2Trsf;
    bal_image imTemplate;

    BAL_InitTransformation( &theTrsf );
    BAL_InitTransformation( &tmp1Trsf );
    BAL_InitTransformation( &tmp2Trsf );

    /* first transformation
     */
    if ( _debug_ )
        fprintf( stderr, "%s: first transformation is '%s'\n",
                 proc, trsf_names->data[trsf_names->n-1] );

    if ( BAL_ReadTransformation( &tmp1Trsf, trsf_names->data[trsf_names->n-1]) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to read '%s'\n", proc, trsf_names->data[trsf_names->n-1] );
      return( -1 );
    }
    resTrsf = &tmp1Trsf;

    /* loop over all other transformations
     */
    for ( i = trsf_names->n-2; i >=0; i-- ) {

        if ( _debug_ )
            fprintf( stderr, "%s: transformation to be composed = '%s'\n",
                     proc, trsf_names->data[i] );

        if ( BAL_ReadTransformation( &theTrsf, trsf_names->data[i] ) != 1 ) {
            BAL_FreeTransformation( &theTrsf );
            BAL_FreeTransformation( resTrsf );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to read '%s'\n", proc, trsf_names->data[i] );
            return( -1 );
        }

        /* check whether the current transformation is a matrix
         * and the next one is a vector field
         * if yes, change the current into a vector field
         */

        if ( BAL_IsTransformationLinear( resTrsf ) == 1 ) {
            if ( BAL_IsTransformationVectorField( &theTrsf ) == 1 ) {

              /* initializing result transformation
               * - with reference image, if any
               * - with parameters, if any
               * - with transformation
               */

              /* initialisation
               * - with reference image, if any
               */
              if ( par->template_name[0] != '\0' ) {
                  if ( BAL_ReadImage( &imTemplate, par->template_name, 0 ) != 1 ) {
                    BAL_FreeTransformation( &theTrsf );
                    BAL_FreeTransformation( resTrsf );
                    if ( _verbose_ )
                        fprintf( stderr, "%s: can not read reference image '%s'\n",
                                 proc, par->template_name );
                    return( -1 );
                  }
              }

              /* initialisation
               * - with parameters, if any
               */
              else if ( par->template_dim.x > 0 && par->template_dim.y > 0 ) {
                if ( par->template_dim.z > 0 ) {
                  if ( BAL_InitImage( &imTemplate, (char*)NULL, par->template_dim.x,
                                      par->template_dim.y, par->template_dim.z, 1, UCHAR ) != 1 ) {
                    BAL_FreeTransformation( &theTrsf );
                    BAL_FreeTransformation( resTrsf );
                    if ( _verbose_ )
                        fprintf( stderr, "%s: unable to initialize result transformation (parameters, 3D case)\n", proc );
                    return( -1 );
                  }
                }
                else {
                  if ( BAL_InitImage( &imTemplate, (char*)NULL, par->template_dim.x,
                                      par->template_dim.y, 1, 1, UCHAR ) != 1 ) {
                    BAL_FreeTransformation( &theTrsf );
                    BAL_FreeTransformation( resTrsf );
                    if ( _verbose_ )
                        fprintf( stderr, "%s: unable to initialize result transformation (parameters, 2D case)\n", proc );
                    return( -1 );
                  }
                }
                if ( par->template_voxel.x > 0.0 ) imTemplate.vx = par->template_voxel.x;
                if ( par->template_voxel.y > 0.0 ) imTemplate.vy = par->template_voxel.y;
                if ( par->template_voxel.z > 0.0 ) imTemplate.vz = par->template_voxel.z;
              }

              /* initialisation with transformation
               */
              else {
                if ( BAL_InitImage( &imTemplate, (char*)NULL,
                        theTrsf.vx.ncols, theTrsf.vx.nrows, theTrsf.vx.nplanes,
                        1, UCHAR ) != 1 ) {
                  BAL_FreeTransformation( &theTrsf );
                  BAL_FreeTransformation( resTrsf );
                  if ( _verbose_ )
                      fprintf( stderr, "%s: unable to initialize result transformation (vector field)\n", proc );
                  return( -1 );
                }
                imTemplate.vx = theTrsf.vx.vx;
                imTemplate.vy = theTrsf.vx.vy;
                imTemplate.vz = theTrsf.vx.vz;
              }


              /* allocation of a new auxiliary transformation
               */
              if ( BAL_AllocTransformation( &tmp2Trsf, theTrsf.type, &imTemplate ) != 1 ) {
                BAL_FreeTransformation( &theTrsf );
                BAL_FreeTransformation( resTrsf );
                if ( _verbose_ )
                    fprintf( stderr, "%s: unable to allocate new auxiliary transformation\n", proc );
                return( -1 );
              }

              BAL_FreeImage( &imTemplate );

              if ( BAL_CopyTransformation( resTrsf, &tmp2Trsf ) != 1 ) {
                BAL_FreeTransformation( &tmp2Trsf );
                BAL_FreeTransformation( &theTrsf );
                BAL_FreeTransformation( resTrsf );
                if ( _verbose_ )
                    fprintf( stderr, "%s: unable to copy transformation\n", proc );
                return( -1 );
              }

              BAL_FreeTransformation( resTrsf );
              resTrsf =  &tmp2Trsf;

            }
        }



        /* compose transformations
         * int BAL_TransformationComposition( bal_transformation *t_res,
         *                                    bal_transformation *t1,
         *                                    bal_transformation *t2 )
         * with t_res = t1 o t2
         */


        if ( BAL_TransformationComposition( resTrsf, &theTrsf, resTrsf ) != 1 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeTransformation( resTrsf );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to compose '%s' with intermediary result\n", proc, trsf_names->data[i] );
          return( -1 ) ;
        }

        /* free the additional transformation
         * for next reading
         */

         BAL_FreeTransformation( &theTrsf );
    }

    if ( BAL_WriteTransformation( resTrsf, output_name ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to write '%s'\n", proc, output_name );
      BAL_FreeTransformation( resTrsf );
      return( -1 );
    }

    BAL_FreeTransformation( resTrsf );

    return( 1 );
}
