/*************************************************************************
 * api-morpho.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sam 18 jul 2015 11:59:27 CEST
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>
#include <chamferdistance.h>

#include <vt_common.h>


#include <api-morpho.h>






static int _verbose_ = 1;
static int _debug_ = 0;



static int Neighborhood2Int ( Neighborhood N )
{
  int connectivity = 26;
  switch ( N ) {
  case N04 :
    connectivity = 4; break;
  case N06 :
    connectivity = 6; break;
  case N08 :
    connectivity = 8; break;
  case N10 :
    connectivity = 10; break;
  case N18 :
    connectivity = 18; break;
  case N26 :
    connectivity = 26; break;
  }
  return( connectivity );
}



static void _API_ParseParam_morpho( char *str, lineCmdParamMorpho *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/

/* userSE is used if and only if there is an user-defined
 * structuring element
 */

int API_morpho( vt_image *image, vt_image *imres,
                typeStructuringElement *userSE,
                char *param_str_1, char *param_str_2 )
{
  char *proc = "API_morpho";
  lineCmdParamMorpho par;
  typeStructuringElement SE, *theSE;
  int theDim[3];
  vt_image *imptr;
  vt_image imtmp;
  vt_image imtmp2;
  int tmpIsAllocated = 0;



  /* parameter initialization
   */
  API_InitParam_morpho( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_morpho( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_morpho( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_morpho( stderr, proc, &par, (char*)NULL );



  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( imres->dim.x != image->dim.x
       || imres->dim.y != image->dim.y
       || imres->dim.z != image->dim.z
       || imres->dim.v != image->dim.v ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: images do not have the same dimensions\n", proc );
      return( -1 );
  }


  /* structuring element
   */
  initStructuringElement( &SE );

  if ( par.euclidean_sphere ) {
      if ( buildStructuringElementAs3DSphere( &SE, par.radius ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to build structuring element as a sphere\n", proc );
        return( -1 );
      }
  }

  SE.nbIterations = par.nb_iterations;

  if ( par.dim == TwoD ) {
    SE.dimension = 2;
    switch ( par.neighborhood ) {
    default :
      break;
    case N06 :
      par.neighborhood = N04;
      break;
    case N10 :
    case N18 :
    case N26 :
      par.neighborhood = N08;
    }
  }
  SE.connectivity = Neighborhood2Int( par.neighborhood );

  if ( par.radius > 0 ) {
    SE.radius = par.radius;
    if ( _verbose_ >= 2 ) {
      printPseudoSphereDecomposition( stderr, SE.radius, SE.dimension, (char *)NULL );
    }
  }

  if ( par.binary_mode )
      useBinaryMorphologicalOperations();

  if ( userSE != NULL &&
       (userSE->userDefinedSE.nb >= 0 && userSE->userDefinedSE.list != NULL) ) {
      theSE = userSE;
  }
  else {
      theSE = &SE;
  }



  /* auxiliary image allocation, if required
   */
  VT_Image( &imtmp );
  VT_Image( &imtmp2 );

  if ( imres->type != image->type ) {
      VT_InitFromImage( &imtmp, image, (char*)NULL, image->type );
      if ( VT_AllocImage( &imtmp ) != 1 ) {
          freeStructuringElement( &SE );
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



  /* morphological operations
   */
  theDim[0] = image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;

  switch ( par.type_operation ) {

  default :

      if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
      freeStructuringElement( &SE );
      if ( _verbose_ )
          fprintf( stderr, "%s: unknown morphological operation\n", proc );
      return( -1 );

  case MORPHOLOGICAL_CONTRAST :

      VT_Image( &imtmp2 );
      VT_InitFromImage( &imtmp2, image, "tmp.inr", image->type );
      if ( VT_AllocImage( &imtmp2 ) != 1 ) {
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: second auxiliary image allocation failed\n", proc );
          return( -1 );
      }

      if ( morphologicalDilation( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
          VT_FreeImage( &imtmp2 );
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: error in dilation (contrast)\n", proc );
          return( -1 );
      }

      if ( morphologicalErosion( image->buf, imtmp2.buf, image->type, theDim, theSE ) != 1 ) {
        VT_FreeImage( &imtmp2 );
        if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
        freeStructuringElement( &SE );
        if ( _verbose_ )
            fprintf( stderr, "%s: error in erosion (contrast)\n", proc );
        return( -1 );
      }

      switch ( image->type ) {
      default :
          VT_FreeImage( &imtmp2 );
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: image type not handled yet (contrast)\n", proc );
          return( -1 );
      case UCHAR :
          {
              u8 *ori = (u8 *)image->buf;
              u8 *dil = (u8 *)imptr->buf;
              u8 *ero = (u8 *)imtmp2.buf;
              u8 *res = (u8 *)imptr->buf;
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, ori++, ero++, dil++, res++ ) {
                if ( (*dil)-(*ori) > (*ori)-(*ero) )
                  *res = *ero;
                else if ( (*dil)-(*ori) < (*ori)-(*ero) )
                  *res = *dil;
                else
                  *res = *ori;
              }
          }
          break;
      case SSHORT :
          {
              s16 *ori = (s16 *)image->buf;
              s16 *dil = (s16 *)imptr->buf;
              s16 *ero = (s16 *)imtmp2.buf;
              s16 *res = (s16 *)imptr->buf;
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, ori++, ero++, dil++, res++ ) {
                if ( (*dil)-(*ori) > (*ori)-(*ero) )
                  *res = *ero;
                else if ( (*dil)-(*ori) < (*ori)-(*ero) )
                  *res = *dil;
                else
                  *res = *ori;
              }
          }
          break;
      case USHORT :
          {
              u16 *ori = (u16 *)image->buf;
              u16 *dil = (u16 *)imptr->buf;
              u16 *ero = (u16 *)imtmp2.buf;
              u16 *res = (u16 *)imptr->buf;
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, ori++, ero++, dil++, res++ ) {
                if ( (*dil)-(*ori) > (*ori)-(*ero) )
                  *res = *ero;
                else if ( (*dil)-(*ori) < (*ori)-(*ero) )
                  *res = *dil;
                else
                  *res = *ori;
              }
          }
          break;
      }
      break;

  case MORPHOLOGICAL_GRADIENT :

      VT_Image( &imtmp2 );
      VT_InitFromImage( &imtmp2, image, "tmp.inr", image->type );
      if ( VT_AllocImage( &imtmp2 ) != 1 ) {
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: second auxiliary image allocation failed\n", proc );
          return( -1 );
      }

      if ( morphologicalDilation( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
          VT_FreeImage( &imtmp2 );
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: error in dilation (gradient)\n", proc );
          return( -1 );
      }

      if ( morphologicalErosion( image->buf, imtmp2.buf, image->type, theDim, theSE ) != 1 ) {
        VT_FreeImage( &imtmp2 );
        if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
        freeStructuringElement( &SE );
        if ( _verbose_ )
            fprintf( stderr, "%s: error in erosion (gradient)\n", proc );
        return( -1 );
      }

      switch ( image->type ) {
      default :
          VT_FreeImage( &imtmp2 );
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: image type not handled yet (gradient)\n", proc );
          return( -1 );
      case UCHAR :
          {
              u8 *dil = (u8 *)imptr->buf;
              u8 *ero = (u8 *)imtmp2.buf;
              u8 *res = (u8 *)imptr->buf;
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, ero++, dil++, res++ ) {
                  *res = (u8)((*dil)-(*ero));
              }
          }
          break;
      case SSHORT :
          {
              s16 *dil = (s16 *)imptr->buf;
              s16 *ero = (s16 *)imtmp2.buf;
              s16 *res = (s16 *)imptr->buf;
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, ero++, dil++, res++ ) {
                  *res = (s16)((*dil)-(*ero));
              }
          }
          break;
      case USHORT :
          {
              u16 *dil = (u16 *)imptr->buf;
              u16 *ero = (u16 *)imtmp2.buf;
              u16 *res = (u16 *)imptr->buf;
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, ero++, dil++, res++ ) {
                  *res = (u16)((*dil)-(*ero));
              }
          }
          break;
      }
      break;

  case MORPHOLOGICAL_HATCLOSING :

      if ( (userSE == NULL
            || (userSE->userDefinedSE.nb == 0 || userSE->userDefinedSE.list == NULL) )
           && par.chamfer != 0 ) {
          if ( morphologicalClosingWithDistance( image->buf, imptr->buf, image->type,
                         theDim, par.radius, SE.dimension ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in closing (hat)\n", proc );
              return( -1 );
          }
      }
      else {
          if ( morphologicalDilation( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation (closing hat)\n", proc );
              return( -1 );
          }
          if ( morphologicalErosion( imptr->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in erosion (closing hat)\n", proc );
              return( -1 );
          }
      }

      switch ( image->type ) {
      default :
          VT_FreeImage( &imtmp2 );
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: image type not handled yet (closing hat)\n", proc );
          return( -1 );
      case UCHAR :
          {
              u8 *in = (u8 *)(image->buf);
              u8 *out = (u8 *)(imptr->buf);
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, in++, out++ )
                  *out -= *in;
          }
          break;
      case SSHORT :
          {
              s16 *in = (s16 *)(image->buf);
              s16 *out = (s16 *)(imptr->buf);
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, in++, out++ )
                  *out -= *in;
          }
          break;
      case USHORT :
          {
              u16 *in = (u16 *)(image->buf);
              u16 *out = (u16 *)(imptr->buf);
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, in++, out++ )
                  *out -= *in;
          }
          break;
      }
      break;

  case MORPHOLOGICAL_HATOPENING :

      if ( (userSE == NULL
            || (userSE->userDefinedSE.nb == 0 || userSE->userDefinedSE.list == NULL) )
           && par.chamfer != 0 ) {
          if ( morphologicalOpeningWithDistance( image->buf, imptr->buf, image->type,
                         theDim, par.radius, SE.dimension ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in opening (hat)\n", proc );
              return( -1 );
          }
      }
      else {
          if ( morphologicalErosion( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in erosion (opening hat)\n", proc );
              return( -1 );
          }
          if ( morphologicalDilation( imptr->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation (opening hat)\n", proc );
              return( -1 );
          }
      }

      switch ( image->type ) {
      default :
          VT_FreeImage( &imtmp2 );
          if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
          freeStructuringElement( &SE );
          if ( _verbose_ )
              fprintf( stderr, "%s: image type not handled yet (closing hat)\n", proc );
          return( -1 );
      case UCHAR :
          {
              u8 *in = (u8 *)(image->buf);
              u8 *out = (u8 *)(imptr->buf);
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, in++, out++ )
                  *out = *in - *out;
          }
          break;
      case SSHORT :
          {
              s16 *in = (s16 *)(image->buf);
              s16 *out = (s16 *)(imptr->buf);
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, in++, out++ )
                  *out = *in - *out;
          }
          break;
      case USHORT :
          {
              u16 *in = (u16 *)(image->buf);
              u16 *out = (u16 *)(imptr->buf);
              register int i, v;
              v = image->dim.x * image->dim.y * image->dim.z;
              for ( i = 0; i < v ; i++, in++, out++ )
                  *out = *in - *out;
          }
          break;
      }
      break;

  case MORPHOLOGICAL_CLOSING :

      if ( (userSE == NULL
            || (userSE->userDefinedSE.nb == 0 || userSE->userDefinedSE.list == NULL) )
           && par.chamfer != 0 ) {
          if ( morphologicalClosingWithDistance( image->buf, imptr->buf, image->type,
                         theDim, par.radius, SE.dimension ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in closing\n", proc );
              return( -1 );
          }
      }
      else {
          if ( morphologicalDilation( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation (closing)\n", proc );
              return( -1 );
          }
          if ( morphologicalErosion( imptr->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in erosion (closing)\n", proc );
              return( -1 );
          }
      }
      break;

  case MORPHOLOGICAL_OPENING :

      if ( (userSE == NULL
            || (userSE->userDefinedSE.nb == 0 || userSE->userDefinedSE.list == NULL) )
           && par.chamfer != 0 ) {
          if ( morphologicalOpeningWithDistance( image->buf, imptr->buf, image->type,
                         theDim, par.radius, SE.dimension ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in opening\n", proc );
              return( -1 );
          }
      }
      else {
          if ( morphologicalErosion( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in erosion (opening)\n", proc );
              return( -1 );
          }
          if ( morphologicalDilation( imptr->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation (opening)\n", proc );
              return( -1 );
          }
      }
      break;

  case MORPHOLOGICAL_DILATION :

      if ( (userSE == NULL
            || (userSE->userDefinedSE.nb == 0 || userSE->userDefinedSE.list == NULL) )
           && par.chamfer != 0 ) {
          if ( morphologicalDilationWithDistance( image->buf, imptr->buf, image->type,
                         theDim, par.radius, SE.dimension ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation\n", proc );
              return( -1 );
          }
      }
      else {
          if ( morphologicalDilation( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation\n", proc );
              return( -1 );
          }
      }
      break;

  case MORPHOLOGICAL_EROSION :

      if ( (userSE == NULL
            || (userSE->userDefinedSE.nb == 0 || userSE->userDefinedSE.list == NULL) )
           && par.chamfer != 0 ) {
          if ( morphologicalErosionWithDistance( image->buf, imptr->buf, image->type,
                         theDim, par.radius, SE.dimension ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation\n", proc );
              return( -1 );
          }
      }
      else {
          if ( morphologicalErosion( image->buf, imptr->buf, image->type, theDim, theSE ) != 1 ) {
              if ( tmpIsAllocated ) VT_FreeImage( &imtmp );
              freeStructuringElement( &SE );
              if ( _verbose_ )
                  fprintf( stderr, "%s: error in dilation\n", proc );
              return( -1 );
          }
      }
      break;

  }
  /* end of morphological operations
   */



  freeStructuringElement( &SE );

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



static char *usage = "[image-in] [image-out]\n\
 [-operation dilation|erosion|closing|opening|closinghat|openinghat ...\n\
 ... closinghat|contrast|gradient]\n\
 [-dilation|-dil] [-erosion|-ero] [-closing|-clo|-fer] [-opening|-ope|-ouv]\n\
 [-hat-closing|-closing-hat|-hclo|-hfer]\n\
 [-hat-opening|-opening-hat|-hope|-houv]\n\
 [-contrast] [-gradient]\n\
 [-binary-computation|-binary|-bin]\n\
 [-structuring-element|-elt %s] [-connectivity|-con %d] [-iterations|-i %d]\n\
 [-radius|-R %d] [-euclidean-sphere|-sphere] [-chamfer]\n\
 [-2D]\n\
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
# morphological operations\n\
 -dilation|-dil: dilation (default)\n\
 -erosion|-ero: erosion\n\
 -closing|-clo|-fer: closing (dilation then erosion)\n\
 -opening|-ope|-ouv: opening (erosion then dilation)\n\
 -hat-closing|-closing-hat|-hclo|-hfer: hat transform (closing - 'image-in'),\n\
    enhances small dark structures\n\
 -hat-opening|-opening-hat|-hope|-houv: inverse hat transform\n\
    ('image-in' - opening), enhances small bright structures\n\
 -contrast:\n\
 -gradient: morphological gradient ('dilated' - 'eroded')\n\
 -binary-computation|-binary|-bin: consider image as binary\n\
# structuring element\n\
 -connectivity|-con %d: structuring element is one among the\n\
    4-, 6-, 8-, 10-, 18-, 26-neighborhoods. 4 and 8 are 2-D elements,\n\
    the others being 3-D (default = 26)\n\
 -iterations|-i %d: number of iterations\n\
 -structuring-element|-elt %s: file depicting a structuring element\n\
    the file has to begin with the dimension of the box\n\
    which contains the structuring element, eg:\n\
    'XDIM=3'\n\
    'YDIM=3'\n\
    'ZDIM=1'\n\
    lines beginning with '#' are ignored\n\
    points of the structuring element are indicated with positive\n\
    numbers except for the center which should be indicated with '+'\n\
    if it belongs to the SE, else with '-'. Eg:\n\
    '1 1 1'\n\
    '1 + 1'\n\
    '1 1 1'\n\
    is the classical 3x3 dilation (ie with the 8-neighborhood)\n\
 -radius|-R %d: the structuring element is the approximation of the\n\
    euclidean ball of the given radius by a combination of 4- and \n\
    8-neighborhoods in 2-D and 6-, 18- and 26-neighborhoods in 3-D.\n\
 -euclidean-sphere|-sphere: the structuring element is the true\n\
    euclidean sphere (to be used with -radius)\n\
 -chamfer: morphological operations are implemented as thresholding\n\
    of (5x5(x5)) chamfer distance map. The threshold is given by the\n\
    '-radius' option. Images are considered as binary.\n\
 -2D: slice by slice computation\n\
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





char *API_Help_morpho( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_morpho( char *program, char *str, int flag )
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



void API_InitParam_morpho( lineCmdParamMorpho *p )
{
    (void)strncpy( p->input_name, "\0", 1 );
    (void)strncpy( p->output_name, "\0", 1 );
    p->input_inv = 0;
    p->input_swap = 0;
    p->output_type = TYPE_UNKNOWN;

    (void)strncpy( p->element_name, "\0", 1 );

    p->type_operation = MORPHOLOGICAL_DILATION;
    p->binary_mode = 0;

    p->nb_iterations = 1;
    p->neighborhood = N26;
    p->dim = ThreeD;

    p->radius = 0;
    p->euclidean_sphere = 0;
    p->chamfer = 0;

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_morpho( FILE *theFile, char *program,
                                  lineCmdParamMorpho *p, char *str )
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

  fprintf( f, "# morphological operation\n" );

  switch( p->type_operation ) {
  fprintf( f, "- morphological operation is " );
  default : fprintf( f, "unknonw, how embarrassing ...\n" ); break;
  case MORPHOLOGICAL_DILATION : fprintf( f, "dilation\n" ); break;
  case MORPHOLOGICAL_EROSION : fprintf( f, "erosion\n" ); break;
  case MORPHOLOGICAL_CLOSING : fprintf( f, "closing\n" ); break;
  case MORPHOLOGICAL_OPENING : fprintf( f, "opening\n" ); break;
  case MORPHOLOGICAL_HATCLOSING : fprintf( f, "closing hat\n" ); break;
  case MORPHOLOGICAL_HATOPENING : fprintf( f, "opening hat\n" ); break;
  case MORPHOLOGICAL_CONTRAST : fprintf( f, "contrast enhancement\n" ); break;
  case MORPHOLOGICAL_GRADIENT : fprintf( f, "morphological gradient\n" ); break;
  }
  fprintf( f, "- consider images as binary: " );
  if ( p->binary_mode ) fprintf( f, "yes\n" );
  else fprintf( f, "no\n" );


  fprintf( f, "# structuring element\n" );

  fprintf( f, "- structuring element file is " );
  if ( p->element_name != (char*)NULL && p->element_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->element_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- number of iterations = %d\n", p->nb_iterations );
  fprintf( f, "- connectivity is " );
  switch ( p->neighborhood ) {
  default :     fprintf( f, "unknown, how embarrassing\n" ); break;
  case N04 : fprintf( f, " 4\n" ); break;
  case N06 : fprintf( f, " 6\n" ); break;
  case N08 : fprintf( f, " 8\n" ); break;
  case N10 : fprintf( f, "10\n" ); break;
  case N18 : fprintf( f, "18\n" ); break;
  case N26 : fprintf( f, "26\n" ); break;
  }
  fprintf( f, "- dimension computation is " );
  switch ( p->dim ) {
  default :     fprintf( f, "unknown, how embarrassing\n" ); break;
  case VT_1D  : fprintf( f, " 1\n" ); break;
  case VT_2D  : fprintf( f, " 2\n" ); break;
  case VT_3D  : fprintf( f, " 3\n" ); break;
  case VT_4D  : fprintf( f, " 4\n" ); break;
  }
  fprintf( f, "- (pseudo)sphere radius is %d\n", p->radius );
  fprintf( f, "- euclidean/real sphere: " );
  if ( p->euclidean_sphere ) fprintf( f, "yes\n" );
  else fprintf( f, "no\n" );
  fprintf( f, "- use chamfer distance (+threshold): " );
  if ( p->chamfer ) fprintf( f, "yes\n" );
  else fprintf( f, "no\n" );



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



static void _API_ParseParam_morpho( char *str, lineCmdParamMorpho *p )
{
  char *proc = "_API_ParseParam_morpho";
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

  API_ParseParam_morpho( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_morpho( int firstargc, int argc, char *argv[],
                                  lineCmdParamMorpho *p )
{
  int i;
  int inputisread = 0;
  int outputisread = 0;
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
            else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name,  ">" );  /* standart output */
              outputisread = 1;
            }
            else {
              API_ErrorParse_morpho( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* morphological operation
           */
          else if ( strcmp ( argv[i], "-operation" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -operation ...\n", 0 );
             if ( strcmp ( argv[i], "dilation" ) == 0 && argv[i][8] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_DILATION;
             }
             else if ( strcmp ( argv[i], "erosion" ) == 0 && argv[i][7] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_EROSION;
             }
             else if ( strcmp ( argv[i], "closing" ) == 0 && argv[i][7] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_CLOSING;
             }
             else if ( strcmp ( argv[i], "opening" ) == 0 && argv[i][7] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_OPENING;
             }
             else if ( strcmp ( argv[i], "closinghat" ) == 0 && argv[i][10] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_HATCLOSING;
             }
             else if ( strcmp ( argv[i], "openinghat" ) == 0 && argv[i][10] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_HATOPENING;
             }
             else if ( strcmp ( argv[i], "contrast" ) == 0 && argv[i][8] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_CONTRAST;
             }
             else if ( strcmp ( argv[i], "gradient" ) == 0 && argv[i][8] == '\0' ) {
                 p->type_operation = MORPHOLOGICAL_GRADIENT;
             }
             else {
               fprintf( stderr, "unknown morphological operation: '%s'\n", argv[i] );
               API_ErrorParse_morpho( (char*)NULL, "parsing -operation ...\n", 0 );
             }
          }
          else if ( (strcmp ( argv[i], "-dilation" ) == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-dil" ) == 0 && argv[i][4] == '\0') ) {
            p->type_operation = MORPHOLOGICAL_DILATION;
          }
          else if ( (strcmp ( argv[i], "-erosion" ) == 0 && argv[i][8] == '\0')
                    || (strcmp ( argv[i], "-ero" ) == 0 && argv[i][4] == '\0') ) {
            p->type_operation = MORPHOLOGICAL_EROSION;
          }
          else if ( (strcmp ( argv[i], "-closing" ) == 0 && argv[i][8] == '\0' )
                    || (strcmp ( argv[i], "-fer" ) == 0 && argv[i][4] == '\0')
                    || (strcmp ( argv[i], "-clo" ) == 0 && argv[i][4] == '\0') ) {
            p->type_operation = MORPHOLOGICAL_CLOSING;
          }
          else if ( (strcmp ( argv[i], "-opening" ) == 0 && argv[i][8] == '\0' )
                    || (strcmp ( argv[i], "-ouv" ) == 0 && argv[i][4] == '\0')
                    || (strcmp ( argv[i], "-ope" ) == 0 && argv[i][4] == '\0') ) {
            p->type_operation = MORPHOLOGICAL_OPENING;
          }
          else if ( (strcmp ( argv[i], "-closinghat" ) == 0 && argv[i][11] == '\0' )
                    || (strcmp ( argv[i], "-hfer" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "-hclo" ) == 0 && argv[i][5] == '\0') ) {
            p->type_operation = MORPHOLOGICAL_HATCLOSING;
          }
          else if ( (strcmp ( argv[i], "-openinghat" ) == 0 && argv[i][11] == '\0' )
                    || (strcmp ( argv[i], "-houv" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "-hope" ) == 0 && argv[i][5] == '\0') ) {
            p->type_operation = MORPHOLOGICAL_HATOPENING;
          }
          else if ( strcmp ( argv[i], "-contrast" ) == 0  && argv[i][9] == '\0') {
            p->type_operation = MORPHOLOGICAL_CONTRAST;
          }
          else if ( strcmp ( argv[i], "-gradient" ) == 0  && argv[i][9] == '\0') {
            p->type_operation = MORPHOLOGICAL_GRADIENT;
          }
          else if ( strcmp ( argv[i], "-binary-computation" ) == 0
                    || (strcmp ( argv[i], "-binary" ) == 0 && argv[i][7] == '\0' )
                    || (strcmp ( argv[i], "-bin" ) == 0 && argv[i][4] == '\0')) {
            p->binary_mode = 1;
          }


          /* structuring element
           */
          else if ( strcmp( argv[i], "-connectivity" ) == 0
                    || ( strcmp ( argv[i], "-con" ) == 0 && argv[i][4] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -connectivity ...\n", 0 );
            status = sscanf( argv[i],"%d",&connectivity );
            if ( status <= 0 ) API_ErrorParse_morpho( (char*)NULL, "parsing -connectivity ...\n", 0 );
            switch( connectivity ) {
            default :
                API_ErrorParse_morpho( (char*)NULL, "parsing -connectivity: unknown connectivity\n", 0 );
            case 4 :
              p->neighborhood = N04;   break;
            case 6 :
              p->neighborhood = N06;   break;
            case 8 :
              p->neighborhood = N08;   break;
            case 10 :
              p->neighborhood = N10;   break;
            case 18 :
              p->neighborhood = N18;   break;
            case 26 :
              p->neighborhood = N26;   break;
            }
          }
          else if ( strcmp ( argv[i], "-iterations" ) == 0
                    || (strcmp ( argv[i], "-i" ) == 0 && argv[i][2] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -iterations ...\n", 0 );
            status = sscanf( argv[i],"%d",&p->nb_iterations );
            if ( status <= 0 ) API_ErrorParse_morpho( (char*)NULL, "parsing -iterations ...\n", 0 );
          }


          else if ( strcmp ( argv[i], "-structuring-element" ) == 0
                    || (strcmp ( argv[i], "-elt" ) == 0 && argv[i][4] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -structuring-element ...\n", 0 );
            (void)strcpy( p->element_name, argv[i] );
          }

          else if ( (strcmp ( argv[i], "-radius" ) == 0 && argv[i][7] == '\0')
                  || (strcmp ( argv[i], "-R" ) == 0 && argv[i][2] == '\0')
                  || (strcmp ( argv[i], "-r" ) == 0 && argv[i][2] == '\0') ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -radius ...\n", 0 );
            status = sscanf( argv[i],"%d",&p->radius );
            if ( status <= 0 ) API_ErrorParse_morpho( (char*)NULL, "parsing -radius ...\n", 0 );
          }

          else if ( strcmp ( argv[i], "-euclidean-sphere" ) == 0
                    || (strcmp ( argv[i], "-sphere" ) == 0 && argv[i][7] == '\0') ) {
            p->euclidean_sphere = 1;
          }

          else if ( strcmp ( argv[i], "-chamfer" ) == 0 ) {
            p->chamfer = 1;
          }
          else if ( strcmp ( argv[i], "-2D" ) == 0 ) {
            p->dim = TwoD;
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
             if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
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
               API_ErrorParse_morpho( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_morpho( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
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
               API_ErrorParse_morpho( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
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
             if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_morpho( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_morpho( (char*)NULL, "parsing -type...\n", 0 );
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
              API_ErrorParse_morpho( (char*)NULL, "parsing -type...\n", 0 );
            }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_morpho( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_morpho( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
              else                     _VT_VERBOSE_ ++;
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
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
              API_ErrorParse_morpho( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_morpho( (char*)NULL, "too long file name ...\n", 0 );
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
              API_ErrorParse_morpho( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
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
        API_ErrorParse_morpho( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}
