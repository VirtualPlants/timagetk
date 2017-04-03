/*************************************************************************
 * api-regionalext.c -
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

#include <api-regionalext.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_regionalext( char *str, lineCmdParamRegionalext *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/


/* the ouput image is the difference image
 */

int API_regionalext( vt_image *image, vt_image *imres,
                     char *param_str_1, char *param_str_2 )
{
  char *proc = "API_regionalext";
  lineCmdParamRegionalext par;

  int tmpIsAllocated = 0;
  vt_image imtmp;
  vt_image *imptr;

  int dim[3];
  int i, v = 0;
  vt_3m m;
  double t, a=1.0, b=0.0;
  double min, max, sum;

  int h, hmin, hmax;
  double hm;



  /* parameter initialization
   */
  API_InitParam_regionalext( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_regionalext( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_regionalext( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_regionalext( stderr, proc, &par, (char*)NULL );



  /* here is the stuff
   */

  /**********************************************
   *
   * allocation of an auxiliary image if
   * - the input image type is not s8, u8, s16, u16
   * - the output image type is different from the input one
   *
   **********************************************/

  if ( imres->dim.x != image->dim.x
       || imres->dim.y != image->dim.y
       || imres->dim.z != image->dim.z
       || imres->dim.v != image->dim.v ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: images do not have the same dimensions\n", proc );
      return( -1 );
  }


  VT_Image( &imtmp );

  switch( image->type ) {

  case SCHAR :
  case UCHAR :
  case USHORT :
  case SSHORT :
      if ( imres->type == image->type ) {
          imptr = imres;
      }
      else {
          VT_InitFromImage( &imtmp, image, (char*)NULL, image->type );
          if ( VT_AllocImage( &imtmp ) != 1 ) {
              if ( _verbose_ )
                  fprintf( stderr, "%s: auxiliary image allocation failed\n", proc );
              return( -1 );
          }
          tmpIsAllocated = 1;
          imptr = &imtmp;
      }
    break;

  case UINT :
  case FLOAT :
  case DOUBLE :
      if ( imres->type == USHORT ) {
          imptr = imres;
      }
      else {
          VT_InitFromImage( &imtmp, image, (char*)NULL, USHORT );
          if ( VT_AllocImage( &imtmp ) != 1 ) {
              if ( _verbose_ )
                  fprintf( stderr, "%s: auxiliary image allocation failed\n", proc );
              return( -1 );
          }
          tmpIsAllocated = 1;
          imptr = &imtmp;
      }
    break;

  default :
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: such input image type not handled yet\n", proc );
      return( -1 );
  }



  /* copy of the input image into the result one
   * just to not use the input one (will be changed)
   */

  m.min = m.moy = m.max = m.ect = (double)0.0;

  dim[0] = image->dim.x;
  dim[1] = image->dim.y;
  dim[2] = image->dim.z;
  v = dim[0] * dim[1] * dim[2];

  switch( image->type ) {

  default :
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: such input image type not handled yet\n", proc );
      return( -1 );

  case SCHAR :
  case UCHAR :
  case USHORT :
  case SSHORT :
      if ( VT_CopyImage( image, imptr ) != 1 ) {
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to copy input image\n", proc );
          return( -1 );
      }
      break;

  case UINT :
  case FLOAT :
  case DOUBLE :
    {
    u16 *theBuf = (u16*)imptr->buf;

    if ( VT_3m( image, (vt_image*)NULL, &m ) == -1 ) {
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute minimum, mean and maximum\n", proc );
      return( -1 );
    }

    if ( m.max == m.min ) {
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: uniform input image\n", proc );
      return( -1 );
    }
    a = 65535.0 / ( m.max - m.min );
    b = - m.min;

    if ( _verbose_ >= 1 ) {
        fprintf( stderr, "%s: input image statistics: min=%lf - mean=%lf - max=%lf\n", proc, m.min, m.moy, m.max );
        fprintf( stderr, "\t input image normalized by (int)((I + %lf) * %lf + 0.5)\n", b, a );
    }

    switch ( image->type ) {
    case UINT :
      {
        u32 *buf = (u32*)image->buf;
        for ( i=0; i<v; i++ ) {
          t = ((double)buf[i] + b) * a;
          if ( t < 0 ) theBuf[i] = 0;
          else if ( t > 65535 ) theBuf[i] = 65535;
          else theBuf[i] = (int)( t + 0.5 );
        }
      }
      break;
    case FLOAT :
      {
        r32 *buf = (r32*)image->buf;
        for ( i=0; i<v; i++ ) {
          t = ((double)buf[i] + b) * a;
          if ( t < 0 ) theBuf[i] = 0;
          else if ( t > 65535 ) theBuf[i] = 65535;
          else theBuf[i] = (int)( t + 0.5 );
        }
      }
      break;
    case DOUBLE :
      {
        r64 *buf = (r64*)image->buf;
        for ( i=0; i<v; i++ ) {
          t = ((double)buf[i] + b) * a;
          if ( t < 0 ) theBuf[i] = 0;
          else if ( t > 65535 ) theBuf[i] = 65535;
          else theBuf[i] = (int)( t + 0.5 );
        }
      }
      break;
    default :
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: such image type not handled yet (but should here)\n", proc );
      return( -1 );
    }

    min = max = theBuf[0];
    sum = 0;
    for ( i=0; i<v; i++ ) {
      if ( min > theBuf[i] ) min = theBuf[i];
      if ( max < theBuf[i] ) max = theBuf[i];
      sum += theBuf[i];
    }

    if ( _verbose_ >=2 ) {
        fprintf( stderr, "\t auxi. image statistics: min=%lf - mean=%lf - max=%lf\n", min, sum/v, max );
    }
    }
    break;
  }



  /* parameters modification according to imptr image type
   */

  switch ( image->type ) {
  case SCHAR :
  case UCHAR :
  case USHORT :
  case SSHORT :
    h = (int)( par.height + 0.5 );
    if ( h < 1 ) h = 1;
    hmin = (int)( par.heightmin + 0.5 );
    if ( hmin < 1 ) hmin = 1;
    hmax = (int)( par.heightmax + 0.5 );
    if ( hmax < 1 ) hmax = 1;
    break;
  case UINT :
  case FLOAT :
  case DOUBLE :
    /* a is the scaling factor for conversion to USHORT
     */
    h = (int)(par.height * a + 0.5);
    hmin = (int)(par.heightmin * a + 0.5);
    hmax = (int)(par.heightmax * a + 0.5);
    break;
  default :
    h = 1;
  }

  switch ( image->type ) {
  case UCHAR :
  case USHORT :
    hm = par.heightMultiplier;
    if ( hm <= 0.0 || hm > 1.0 ) hm = 1.0;
    break;
  case SSHORT :
  case UINT :
  case FLOAT :
  case DOUBLE :
    if ( m.min < 0.0 ) {
      hm = 1.0;
      if ( _verbose_ >=2 ) {
          fprintf( stderr, "%s: height multiplier set to 1.0\n", proc );
      }
    }
    else {
      /* hm = par.heightMultiplier * a;
         GM: no idea why I've multiplied by 'a' here :(
         Wed Jun 15 17:11:14 CEST 2011
      */
      hm = par.heightMultiplier;
    }
    break;
  default :
    hm = 1.0;
  }



  /**********************************************
   *
   * processing
   *
   **********************************************/

  /*----------------------------------------
     hierarchical extrema
    ----------------------------------------*/

  if ( par.heightmin > 0.0
       && par.heightmax >= par.heightmin ) {

    switch ( par.typeExtremum ) {
    default :
      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      if ( _verbose_ )
          fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
      return( -1 );
    case _REGIONAL_MAX_ :
        if ( regionalHierarchicalMaxima( imptr->buf, imptr->buf, imptr->type,
                                         dim, hmin, hmax, par.connectivity ) != 1 ) {
            if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
            if ( _verbose_ )
                fprintf( stderr, "%s: unable to compute regional hierarchical maxima\n", proc );
            return( -1 );
        }
        break;
    case _REGIONAL_MIN_ :
        if ( regionalHierarchicalMinima( imptr->buf, imptr->buf, imptr->type,
                                         dim, hmin, hmax, par.connectivity ) != 1 ) {
            if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
            if ( _verbose_ )
                fprintf( stderr, "%s: unable to compute regional hierarchical minima\n", proc );
            return( -1 );
        }
        break;
    }


  }



  /*----------------------------------------
    non-hierarchical extrema
    ie classical regional extrema
   ----------------------------------------*/
  else {

    switch ( par.typeExtremum ) {
    default :
        if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
        if ( _verbose_ )
            fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
        return( -1 );
    case _REGIONAL_MAX_ :
        if ( regionalMaxima( imptr->buf, imptr->buf, imptr->type, dim,
                             h, hm, par.connectivity ) != 1 ) {
            if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
            if ( _verbose_ )
                fprintf( stderr, "%s: unable to compute regional maxima\n", proc );
            return( -1 );
        }
        break;
    case _REGIONAL_MIN_ :
        if ( regionalMinima( imptr->buf, imptr->buf, imptr->type, dim,
                             h, hm, par.connectivity ) != 1 ) {
            if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
            if ( _verbose_ )
                fprintf( stderr, "%s: unable to compute regional minima\n", proc );
            return( -1 );
        }
        break;
    }
    /* imptr is a 'difference' image
     */

  }


  /* this is over
   */

  /* this test is not necessary since VT_CopyImage()
   * should be able to deal with identical image
   */
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



static char *usage = "[image-in] [[-difference-output-image|-diff] image-out]\n\
 [[-binary-output-image | -binary] %s]\n\
 [-binary-type s8|u8|s16|u16...]\n\
 [-minima | -min | -maxima | -max]\n\
 [-height|-h %lf] [-height-multiplier|-hm %lf]\n\
 [-height-min|-hmin %lf] [-height_max|-hmax %lf]\n\
 [-connectivity|-con|-c %d]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [-inv] [-swap] [output-image-type]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'image-in' is equal to '-', stdin will be used\n\
 if 'image-out' is not specified or equal to '-', stdout will be used\n\
 if both are not specified, stdin and stdout will be used\n\
# output binary image (only for regional minima/maxima computation)\n\
 the default output image is a 'difference' image\n\
 this option allows to get the thresholded 'difference' image\n\
 -difference-output-image|-diff %s: difference image name\n\
 -binary-output-image | -binary %s: binary image name\n\
 -binary-type 8|u8|s16|u16...: binary image type\n\
# extrema\n\
 -minima | -min:\n\
 -maxima | -max:\n\
# initialization\n\
 -height|-h %lf:\n\
 -height-multiplier|-hm %lf:\n\
  initialization for regional maxima: min ( I - height, I_min + (I-I_min) * multiplier)\n\
  initialization for regional minima: max ( I + height, I_max - (I_max-I) * multiplier)\n\
# hierarchical extrema\n\
  -height-min|-hmin %lf:\n\
  -height_max|-hmax %lf:\n\
# computation\n\
 -connectivity|-con|-c %d\n\
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





char *API_Help_regionalext( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_regionalext( char *program, char *str, int flag )
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



void API_InitParam_regionalext( lineCmdParamRegionalext *p )
{
    (void)strncpy( p->input_name, "\0", 1 );
    (void)strncpy( p->output_name, "\0", 1 );
    p->input_inv = 0;
    p->input_swap = 0;
    p->output_type = TYPE_UNKNOWN;

    (void)strncpy( p->binary_output_name, "\0", 1 );
    p->binary_output_type = UCHAR;

    p->typeExtremum = _REGIONAL_MAX_;

    p->height = 1.0;
    p->heightMultiplier = 1.0;

    p->heightmin = 0.0;
    p->heightmax = 0.0;

    p->connectivity = 26;

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_regionalext( FILE *theFile, char *program,
                                  lineCmdParamRegionalext *p, char *str )
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

  fprintf( f, "# binary image\n" );

  fprintf( f, "- output binary image is " );
  if ( p->binary_output_name != (char*)NULL && p->binary_output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->binary_output_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output binary image type = " );
  switch ( p->binary_output_type ) {
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

  fprintf( f, "# extremum type\n" );

  fprintf( f, "- extremum type = " );
  switch ( p->typeExtremum ) {
  default :
      fprintf( f, "unknown\n" ); break;
  case _REGIONAL_MAX_ :
      fprintf( f, "maxima\n" ); break;
  case _REGIONAL_MIN_ :
      fprintf( f, "minima\n" ); break;
  }

  fprintf( f, "# initialization\n" );

  fprintf( f, "- absolute height substractor = %f\n", p->height );
  fprintf( f, "- height multiplier = %f\n", p->heightMultiplier );

  fprintf( f, "# hierarchical extremum\n" );

  fprintf( f, "- minimal height = %f\n", p->heightmin );
  fprintf( f, "- maximal height = %f\n", p->heightmax );

  fprintf( f, "# connectivity\n" );

  fprintf( f, "- connectivity = %d\n", p->connectivity );

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



static void _API_ParseParam_regionalext( char *str, lineCmdParamRegionalext *p )
{
  char *proc = "_API_ParseParam_regionalext";
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

  API_ParseParam_regionalext( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_regionalext( int firstargc, int argc, char *argv[],
                                  lineCmdParamRegionalext *p )
{
  int i;
  int inputisread = 0;
  int binaryisread = 0;
  int differenceisread = 0;
  char text[STRINGLENGTH];
  int status;
  int connectivity;
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
            else if ( differenceisread == 0 ) {
              (void)strcpy( p->output_name,  ">" );  /* standart output */
              differenceisread = 1;
            }
            else {
              API_ErrorParse_regionalext( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* difference image
           */

          else if ( strcmp ( argv[i], "-difference-output-image" ) == 0
                    || strcmp ( argv[i], "-difference-image" ) == 0
                    || (strcmp ( argv[i], "-diff" ) == 0 && argv[i][5] == '\0') ) {
              if ( differenceisread == 0 ) {
                  i ++;
                  if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -difference-output-image ...\n", 0 );
                  (void)strcpy( p->output_name, argv[i] );
                  differenceisread = 1;
              }
              else
                  API_ErrorParse_regionalext( (char*)NULL, "parsing -difference-output-image, difference image already given ...\n", 0 );
          }

          else if ( strcmp ( argv[i], "-binary-output-image" ) == 0
                    || (strcmp ( argv[i], "-binary" ) == 0 && argv[i][7] == '\0') ) {
              if ( binaryisread == 0 ) {
                  i ++;
                  if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -binary-output-image ...\n", 0 );
                  (void)strcpy( p->binary_output_name, argv[i] );
                  binaryisread = 1;
              }
              else
                  API_ErrorParse_regionalext( (char*)NULL, "parsing -binary-output-image, binary image already given ...\n", 0 );
          }

          else if ( strcmp ( argv[i], "-binary-type" ) == 0 && argv[i][12] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -binary-type...\n", 0 );
            if ( strcmp ( argv[i], "s8" ) == 0 && argv[i][2] == '\0' ) {
               p->binary_output_type = SCHAR;
            }
            else if ( strcmp ( argv[i], "u8" ) == 0 && argv[i][2] == '\0' ) {
               p->binary_output_type = UCHAR;
            }
            else if ( strcmp ( argv[i], "s16" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = SSHORT;
            }
            else if ( strcmp ( argv[i], "u16" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = USHORT;
            }
            else if ( strcmp ( argv[i], "s32" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = SINT;
            }
            else if ( strcmp ( argv[i], "u32" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = UINT;
            }
            else if ( strcmp ( argv[i], "s64" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = SLINT;
            }
            else if ( strcmp ( argv[i], "u64" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = ULINT;
            }
            else if ( strcmp ( argv[i], "r32" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = FLOAT;
            }
            else if ( strcmp ( argv[i], "r64" ) == 0 && argv[i][3] == '\0' ) {
              p->binary_output_type = DOUBLE;
            }
            else {
              API_ErrorParse_regionalext( (char*)NULL, "parsing -binary-type...\n", 0 );
            }
          }

          /* extremum type
           */

          else if ( strcmp ( argv[i], "-maxima" ) == 0
                    || (strcmp ( argv[i], "-max" ) == 0 && argv[i][4] == '\0') ) {
            p->typeExtremum = _REGIONAL_MAX_;
          }
          else if ( strcmp ( argv[i], "-minima" ) == 0
                    || (strcmp ( argv[i], "-min" ) == 0 && argv[i][4] == '\0') ) {
            p->typeExtremum = _REGIONAL_MIN_;
          }

          /* initialization
           */

          else if ( (strcmp ( argv[i], "-height" ) == 0 && argv[i][7] == '\0')
                    || (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -height...\n", 0 );
            status = sscanf( argv[i],"%lf",&(p->height) );
            if ( status <= 0 ) API_ErrorParse_regionalext( (char*)NULL, "parsing -height...\n", 0 );
          }
          else if ( (strcmp ( argv[i], "-height-multiplier" ) == 0 && argv[i][18] == '\0')
                    || (strcmp ( argv[i], "-hm" ) == 0 && argv[i][3] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -height-multiplier...\n", 0 );
            status = sscanf( argv[i],"%lf",&(p->heightMultiplier) );
            if ( status <= 0 ) API_ErrorParse_regionalext( (char*)NULL, "parsing -height-multiplier...\n", 0 );
          }


          /* hierarchical extremum
           */

          else if ( (strcmp ( argv[i], "-height-min" ) == 0 && argv[i][11] == '\0')
                    || (strcmp ( argv[i], "-hmin" ) == 0 && argv[i][5] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -height-min...\n", 0 );
            status = sscanf( argv[i],"%lf",&(p->heightmin) );
            if ( status <= 0 ) API_ErrorParse_regionalext( (char*)NULL, "parsing -height-min...\n", 0 );
          }
          else if ( (strcmp ( argv[i], "-height-max" ) == 0 && argv[i][11] == '\0')
                    || (strcmp ( argv[i], "-hmax" ) == 0 && argv[i][5] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -height-max...\n", 0 );
            status = sscanf( argv[i],"%lf",&(p->heightmax) );
            if ( status <= 0 ) API_ErrorParse_regionalext( (char*)NULL, "parsing -height-max...\n", 0 );
          }

          /* connectivity
           */

          else if ( strcmp ( argv[i], "-connectivity" ) == 0 ||
                    (strcmp ( argv[i], "-c" ) == 0  && argv[i][2] == '\0') ||
                    (strcmp ( argv[i], "-con" ) == 0 && argv[i][4] == '\0')  ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -connectivity...\n", 0 );
            status = sscanf( argv[i],"%d",&connectivity );
            if ( status <= 0 ) API_ErrorParse_regionalext( (char*)NULL, "parsing -connectivity...\n", 0 );
            switch ( connectivity ) {
            case  4 :
            case  8 :
            case  6 :
            case 10 :
            case 18 :
            case 26 :
              p->connectivity = connectivity;
              break;
            default :
                API_ErrorParse_regionalext( (char*)NULL, "parsing -connectivity: connectivity is not handled...\n", 0 );
            }
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
             if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_regionalext( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_regionalext( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_regionalext( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_regionalext( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_regionalext( (char*)NULL, "parsing -type...\n", 0 );
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
              API_ErrorParse_regionalext( (char*)NULL, "parsing -type...\n", 0 );
            }
          }


          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_regionalext( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_regionalext( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
             incrementVerboseInRegionalExt();
             if ( _verbose_ <= 0 ) _verbose_ = 1;
             else                  _verbose_ ++;
             if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
             else                     _VT_VERBOSE_ ++;
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
             setVerboseInRegionalExt( 0 );
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
              API_ErrorParse_regionalext( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_regionalext( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( inputisread == 0 ) {
              (void)strcpy( p->input_name, argv[i] );
              inputisread = 1;
          }
          else if ( differenceisread == 0 ) {
              (void)strcpy( p->output_name, argv[i] );
              differenceisread = 1;
          }
          else if ( binaryisread == 0 ) {
              (void)strcpy( p->binary_output_name, argv[i] );
              binaryisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_regionalext( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
  if ( inputisread == 0 ) {
    (void)strcpy( p->input_name,  "<" );  /* standart input */
    inputisread = 1;
  }
  if ( differenceisread == 0 ) {
    (void)strcpy( p->output_name,  ">" );  /* standart output */
    differenceisread = 1;
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
        API_ErrorParse_regionalext( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}
