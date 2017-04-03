/*************************************************************************
 * morphotools.c -
 *
 * $Id: morphotools.c,v 1.3 2006/04/14 08:38:55 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Mar 14 08:28:36 MET 2001
 *
 *
 * ADDITIONS, CHANGES
 *
 * Thu Mar 15 09:52:38 CET 2012
 * - correction des procedures userDefined[Dilation,Erosion] pour prendre en 
 *   compte une taille d'element structurant plus grande que la dimension en x. 
 *   On avait born1 > born2, ce qui posait un probleme.
 * - correction de morphological[Dilation,Erosion] dans le cas de la cascade 
 *   d'operations induite par un element structurant pseudo-spherique specifie 
 *   par rayon. Il y avait un probleme si le premier element structurant
 *   elementaire n'etait pas utilise. L'introduction d'un pointeur (tmpBuf) pointant 
 *   vers le 'bon' buffer d'entree resout cela.
 *
 */

#include <morphotools.h>

static int _debug_ = 0;
static int _verbose_ = 1;
static int _binary_mode_ = 0;

static void userDefinedDilation( void *inputBuf,
                                 void *resultBuf,
                                 bufferType type,
                                 int *theDim,
                                 typeMorphoToolsList *userDefinedSE,
                                 int iterations );


static void userDefinedErosion( void *inputBuf,
                                 void *resultBuf,
                                 bufferType type,
                                 int *theDim,
                                 typeMorphoToolsList *userDefinedSE,
                                 int iterations );




static int _iterations_04connectivity( int R )
{
  switch( R ) {
  default : return( -1 );
  case 1 : return( 0 );
  case 2 : return( 1 );
  case 3 :
  case 4 : return( 2 );
  case 5 : return( 3 );
  case 6 :
  case 7 : return( 4 );
  case 8 :
  case 9 : return( 5 );
  case 10 : return( 6 );
  case 11 : return( 7 );
  case 12 :
  case 13 :
  case 14 : return( 8 );
  case 15 : return( 9 );
  case 16 :
  case 17 : return( 10 );
  case 18 : return( 11 );
  case 19 :
  case 20 : return( 12 );
  case 21 : return( 13 );
  case 22 : return( 11 );
  case 23 :
  case 24 : return( 12 );
  case 25 :
  case 26 : return( 16 );
  case 27 : return( 14 );
  case 28 : return( 15 );
  case 29 :
  case 30 : return( 16 );
  case 31 :
  case 32 : return( 17 );
  case 33 : return( 18 );
  case 34 : return( 19 );
  case 35 :
  case 36 :
  case 37 : return( 20 );
  case 38 : return( 21 );
  case 39 :
  case 40 : return( 22 );
  case 41 : return( 23 );
  case 42 :
  case 43 : return( 24 );
  case 44 :
  case 45 : return( 25 );
  case 46 : return( 26 );
  case 47 : return( 27 );
  case 48 :
  case 49 :
  case 50 : return( 28 );
    break;
  }
  return( -1 );
}


static int _iterations_08connectivity( int R )
{
  switch( R ) {
  default : return( -1 );
  case 1 :
  case 2 :
  case 3 : return( 1 );
  case 4 :
  case 5 :
  case 6 : return( 2 );
  case 7 :
  case 8 : return( 3 );
  case 9 :
  case 10 :
  case 11 :
  case 12 : return( 4 );
  case 13 : return( 5 );
  case 14 :
  case 15 :
  case 16 : return( 6 );
  case 17 :
  case 18 :
  case 19 : return( 7 );
  case 20 :
  case 21 : return( 8 );
  case 22 :
  case 23 : return( 10 );
  case 24 : return( 11 );
  case 25 : return( 9 );
  case 26 : return( 10 );
  case 27 :
  case 28 :
  case 29 : return( 12 );
  case 30 :
  case 31 : return( 13 );
  case 32 :
  case 33 :
  case 34 :
  case 35 : return( 14 );
  case 36 : return( 15 );
  case 37 :
  case 38 :
  case 39 : return( 16 );
  case 40 :
  case 41 :
  case 42 : return( 17 );
  case 43 :
  case 44 : return( 18 );
  case 45 :
  case 46 :
  case 47 :
  case 48 : return( 19 );
  case 49 : return( 20 );
  case 50 : return( 21 );
    break;
  }
  return( -1 );
}

static int _iterations_06connectivity( int R )
{
  switch( R ) {
  default : return( -1 );
  case 1 : return( 0 );
  case 2 : return( 1 );
  case 3 :
  case 4 : return( 2 );
  case 5 :
  case 6 : return( 3 );
  case 7 : return( 2 );
  case 8 : return( 5 );
  case 9 : return( 3 );
  case 10 : return( 4 );
  case 11 : return( 5 );
  case 12 :
  case 13 : return( 6 );
  case 14 :
  case 15 : return( 7 );
  case 16 : return( 8 );
  case 17 :
  case 18 : return( 9 );
  case 19 : return( 10 );
  case 20 :
  case 21 : return( 11 );
  case 22 : return( 12 );
  case 23 :
  case 24 : return( 13 );
  case 25 : return( 14 );
  case 26 : return( 12 );
  case 27 : return( 13 );
  case 28 : return( 16 );
  case 29 : return( 14 );
  case 30 :
  case 31 : return( 15 );
  case 32 : return( 16 );
  case 33 :
  case 34 : return( 17 );
  case 35 : return( 18 );
  case 36 :
  case 37 : return( 19 );
  case 38 : return( 20 );
  case 39 :
  case 40 : return( 21 );
  case 41 : return( 22 );
  case 42 :
  case 43 : return( 23 );
  case 44 : return( 24 );
  case 45 : return( 25 );
  case 46 : return( 23 );
  case 47 :
  case 48 : return( 24 );
  case 49 : return( 25 );
  case 50 : return( 26 );
    break;
  }
  return( -1 );
}

static int _iterations_18connectivity( int R )
{
  switch( R ) {
  default : return( -1 );
  case 1 : return( 1 );
  case 2 :
  case 3 : return( 0 );
  case 4 : return( 2 );
  case 5 : return( 0 );
  case 6 :
  case 7 : return( 2 );
  case 8 : return( 0 );
  case 9 : return( 3 );
  case 10 : return( 2 );
  case 11 : return( 1 );
  case 12 : return( 0 );
  case 13 : return( 2 );
  case 14 : return( 1 );
  case 15 : return( 3 );
  case 16 : return( 2 );
  case 17 : return( 1 );
  case 18 : return( 3 );
  case 19 : return( 2 );
  case 20 : return( 1 );
  case 21 : return( 3 );
  case 22 : return( 2 );
  case 23 : return( 1 );
  case 24 : return( 3 );
  case 25 : return( 2 );
  case 26 :
  case 27 : return( 4 );
  case 28 : return( 2 );
  case 29 : return( 4 );
  case 30 : return( 3 );
  case 31 : return( 5 );
  case 32 :
  case 33 : return( 4 );
  case 34 : return( 5 );
  case 35 : return( 4 );
  case 36 : return( 3 );
  case 37 : return( 5 );
  case 38 : return( 4 );
  case 39 : return( 3 );
  case 40 : return( 5 );
  case 41 : return( 4 );
  case 42 : return( 3 );
  case 43 : return( 5 );
  case 44 : return( 4 );
  case 45 : return( 3 );
  case 46 : return( 6 );
  case 47 : return( 5 );
  case 48 : return( 7 );
  case 49 : return( 6 );
  case 50 : return( 5 );
    break;
  }
  return( -1 );
}

static int _iterations_26connectivity( int R )
{
  switch( R ) {
  default : return( -1 );
  case 1 : return( 0 );
  case 2 :
  case 3 : return( 1 );
  case 4 : return( 0 );
  case 5 : return( 2 );
  case 6 : return( 1 );
  case 7 : return( 2 );
  case 8 : return( 3 );
  case 9 : return( 2 );
  case 10 : return( 3 );
  case 11 : return( 4 );
  case 12 : return( 5 );
  case 13 : return( 4 );
  case 14 : return( 5 );
  case 15 : return( 4 );
  case 16 : return( 5 );
  case 17 : return( 6 );
  case 18 : return( 5 );
  case 19 : return( 6 );
  case 20 : return( 7 );
  case 21 : return( 6 );
  case 22 : return( 7 );
  case 23 : return( 8 );
  case 24 : return( 7 );
  case 25 :
  case 26 :
  case 27 : return( 8 );
  case 28 :
  case 29 : return( 9 );
  case 30 : return( 10 );
  case 31 : return( 9 );
  case 32 :
  case 33 :
  case 34 : return( 10 );
  case 35 : return( 11 );
  case 36 : return( 12 );
  case 37 : return( 11 );
  case 38 : return( 12 );
  case 39 : return( 13 );
  case 40 : return( 12 );
  case 41 : return( 13 );
  case 42 : return( 14 );
  case 43 : return( 13 );
  case 44 : return( 14 );
  case 45 : return( 15 );
  case 46 : return( 14 );
  case 47 : return( 15 );
  case 48 : return( 14 );
  case 49 : return( 15 );
  case 50 : return( 16 );
    break;
  }
  return( -1 );
}


void printPseudoSphereDecomposition( FILE *f, int radius, int dim, char *str )
{
  int sum = 0;
  int n;

  if ( str != (char*)NULL )
    fprintf( f, "Structuring element: '%s'\n", str );
  if ( dim == 2 ) {
    n = _iterations_08connectivity( radius );
    fprintf( f, "#iterations 8-connectivity = %d\n", n );
    sum += n;
    n = _iterations_04connectivity( radius );
    fprintf( f, "#iterations 4-connectivity = %d\n", n );
    sum += n;
  }
  else {
    n = _iterations_26connectivity( radius );
    fprintf( f, "#iterations 26-connectivity = %d\n", n );
    sum += n;
    n = _iterations_18connectivity( radius );
    fprintf( f, "#iterations 18-connectivity = %d\n", n );
    sum += n;    
    n = _iterations_06connectivity( radius );
    fprintf( f, "#iterations  6-connectivity = %d\n", n );
    sum += n;
  }
}



int morphologicalDilation( void *inputBuf,
                            void *resultBuf,
                            bufferType type,
                            int *theDim,
                            typeStructuringElement *theSE )
{
  char *proc = "morphologicalDilation";
  void *tmpBuf;
  int connectivity = 0;
  int iterations   = 1;
  int sum_iterations = 0;
  int s;
  typeStructuringElement SE;
  

  if ( theSE == NULL ) {
    connectivity = 26;
  } else {
    if ( theSE->userDefinedSE.nb == 0 || theSE->userDefinedSE.list == NULL ) {

      if ( theSE->radius >= 1 ) {

        if ( theSE->dimension == 2 || theDim[2] == 1 ) {

          initStructuringElement( &SE );

          SE.connectivity = 8;
          SE.nbIterations = _iterations_08connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;
          SE.connectivity = 4;
          SE.nbIterations = _iterations_04connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;

          if ( sum_iterations <= 0 ) {
            if ( _verbose_ ) {
              fprintf( stderr, "%s: no dilation (radius too large?)\n", proc );
            }
            return( 0 );
          }

          tmpBuf = inputBuf;

          SE.connectivity = 8;
          SE.nbIterations = _iterations_08connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( morphologicalDilation( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during first 2D dilation\n", proc );
              }
              return( 0 );
            }
            tmpBuf = resultBuf;
          }

          SE.connectivity = 4;
          SE.nbIterations = _iterations_04connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( morphologicalDilation( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during second 2D dilation\n", proc );
              }
              return( 0 );
            }
          }
          return( 1 );

        }

        else {

          initStructuringElement( &SE );

          SE.connectivity = 26;
          SE.nbIterations = _iterations_26connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;
          SE.connectivity = 18;
          SE.nbIterations = _iterations_18connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;
          SE.connectivity = 6;
          SE.nbIterations = _iterations_06connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;

          if ( sum_iterations <= 0 ) {
            if ( _verbose_ ) {
              fprintf( stderr, "%s: no dilation (radius too large?)\n", proc );
            }
            return( 0 );
          }

          tmpBuf = inputBuf;

          SE.connectivity = 26;
          SE.nbIterations = _iterations_26connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( _debug_ )
              fprintf( stderr, "%s: %d iterations of 3D dilation using 26-element\n", proc, SE.nbIterations );
            if ( morphologicalDilation( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during first 3D dilation\n", proc );
              }
              return( 0 );
            }
            tmpBuf = resultBuf;
          }

          SE.connectivity = 18;
          SE.nbIterations = _iterations_18connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( _debug_ )
              fprintf( stderr, "%s: %d iterations of 3D dilation using 18-element\n", proc, SE.nbIterations );
            if ( morphologicalDilation( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during second 3D dilation\n", proc );
              }
              return( 0 );
            }
            tmpBuf = resultBuf;
          }

          SE.connectivity = 6;
          SE.nbIterations = _iterations_06connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( _debug_ )
              fprintf( stderr, "%s: %d iterations of 3D dilation using 06-element\n", proc, SE.nbIterations );
            if ( morphologicalDilation( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during third 3D dilation\n", proc );
              }
              return( 0 );
            }
          }
          return( 1 );

        }

      }



      connectivity = theSE->connectivity;
      iterations   = theSE->nbIterations;
      
      if ( theSE->dimension == 2 ) {
        switch( connectivity ) {
        default : break;
        case 6 :
          connectivity = 4; break;
        case 10 :
        case 18 :
        case 26 :
          connectivity = 8; break;
        }
      }

      if ( iterations <= 0 ) {
        if ( inputBuf == resultBuf ) return( 1 );
        s = theDim[0] * theDim[1] * theDim[2];
        switch( type ) {
        default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such type not handled yet\n", proc );
          }
          return( 0 );
        case SCHAR :
          s *= sizeof( s8 );   break;
        case UCHAR :
          s *= sizeof( u8 );   break;
        case SSHORT :
          s *= sizeof( s16 );   break;
        case USHORT :
          s *= sizeof( u16 );   break;
        case FLOAT :
          s *= sizeof( r32 );   break;
        case DOUBLE :
          s *= sizeof( r64 );   break;
        }
        (void)memcpy( resultBuf, inputBuf, s );
        return( 1 );
      }

    }

  }

  if ( theSE == NULL ||
       (theSE->userDefinedSE.nb == 0 || theSE->userDefinedSE.list == NULL) ) {

    switch( type ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such type not handled yet\n", proc );
      }
      return( 0 );
    case UCHAR :
      if ( _binary_mode_ ) {
        BinaryDilation_u8( inputBuf, resultBuf, theDim, connectivity, iterations );
      } else {
        GreyLevelDilation_u8( inputBuf, resultBuf, theDim, connectivity, iterations );
      }
      break;
    case USHORT :
      GreyLevelDilation_u16( inputBuf, resultBuf, theDim, connectivity, iterations );
      break;
    case SSHORT :
      GreyLevelDilation_s16( inputBuf, resultBuf, theDim, connectivity, iterations );
      break;
    case FLOAT :
      GreyLevelDilation_r32( inputBuf, resultBuf, theDim, connectivity, iterations );
      break;
    }

  } else {
    userDefinedDilation( inputBuf, resultBuf, type, theDim,
                       &(theSE->userDefinedSE), theSE->nbIterations );
  }

  return( 1 );
}  

















int morphologicalErosion( void *inputBuf,
                           void *resultBuf,
                           bufferType type,
                           int *theDim,
                           typeStructuringElement *theSE )
{
  char *proc = "morphologicalErosion";
  void *tmpBuf;
  int connectivity = 0;
  int iterations   = 1;
  int sum_iterations = 0;
  int s;
  typeStructuringElement SE;


  if ( theSE == NULL ) {
    connectivity = 26;
  } else {
    if ( theSE->userDefinedSE.nb == 0 || theSE->userDefinedSE.list == NULL ) {
 
      if ( theSE->radius >= 1 ) {

        if ( theSE->dimension == 2 || theDim[2] == 1 ) {

          initStructuringElement( &SE );

          SE.connectivity = 8;
          SE.nbIterations = _iterations_08connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;
          SE.connectivity = 4;
          SE.nbIterations = _iterations_04connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;

          if ( sum_iterations <= 0 ) {
            if ( _verbose_ ) {
              fprintf( stderr, "%s: no erosion (radius too large?)\n", proc );
            }
            return( 0 );
          }

          tmpBuf = inputBuf;

          SE.connectivity = 8;
          SE.nbIterations = _iterations_08connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( morphologicalErosion( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during first 2D erosion\n", proc );
              }
              return( 0 );
            }
            tmpBuf = resultBuf;
          }
          SE.connectivity = 4;
          SE.nbIterations = _iterations_04connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( morphologicalErosion( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during second 2D erosion\n", proc );
              }
              return( 0 );
            }
          }
          return( 1 );

        }

        else {

          initStructuringElement( &SE );

          SE.connectivity = 26;
          SE.nbIterations = _iterations_26connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;
          SE.connectivity = 18;
          SE.nbIterations = _iterations_18connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;
          SE.connectivity = 6;
          SE.nbIterations = _iterations_06connectivity( theSE->radius );
          sum_iterations += SE.nbIterations;

          if ( sum_iterations <= 0 ) {
            if ( _verbose_ ) {
              fprintf( stderr, "%s: no erosion (radius too large?)\n", proc );
            }
            return( 0 );
          }

          tmpBuf = inputBuf;

          SE.connectivity = 26;
          SE.nbIterations = _iterations_26connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( _debug_ )
              fprintf( stderr, "%s: %d iterations of 3D erosion using 26-element\n", proc, SE.nbIterations );
            if ( morphologicalErosion( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during first 3D erosion\n", proc );
              }
              return( 0 );
            }
            tmpBuf = resultBuf;
          }

          SE.connectivity = 18;
          SE.nbIterations = _iterations_18connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( _debug_ )
              fprintf( stderr, "%s: %d iterations of 3D erosion using 18-element\n", proc, SE.nbIterations );
            if ( morphologicalErosion( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during second 3D erosion\n", proc );
              }
              return( 0 );
            }
            tmpBuf = resultBuf;
          }

          SE.connectivity = 6;
          SE.nbIterations = _iterations_06connectivity( theSE->radius );
          if ( SE.nbIterations > 0 ) {
            if ( _debug_ )
              fprintf( stderr, "%s: %d iterations of 3D erosion using 06-element\n", proc, SE.nbIterations );
            if ( morphologicalErosion( tmpBuf, resultBuf, type, theDim, &SE ) != 1 ) {
              if ( _verbose_ ) {
                fprintf( stderr, "%s: error during third 3D erosion\n", proc );
              }
              return( 0 );
            }
          }
          return( 1 );

        }

      }



      connectivity = theSE->connectivity;
      iterations   = theSE->nbIterations;

      if ( theSE->dimension == 2 ) {
        switch( connectivity ) {
        default : break;
        case 6 :
          connectivity = 4; break;
        case 10 :
        case 18 :
        case 26 :
          connectivity = 8; break;
        }
      }

      if ( iterations <= 0 ) {
        if ( inputBuf == resultBuf ) return( 1 );
        s = theDim[0] * theDim[1] * theDim[2];
        switch( type ) {
        default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such type not handled yet (in copy)\n", proc );
          }
          return( 0 );
        case SCHAR :
          s *= sizeof( s8 );   break;
        case UCHAR :
          s *= sizeof( u8 );   break;
        case SSHORT :
          s *= sizeof( s16 );   break;
        case USHORT :
          s *= sizeof( u16 );   break;
        case FLOAT :
          s *= sizeof( r32 );   break;
        case DOUBLE :
          s *= sizeof( r64 );   break;
        }
        (void)memcpy( resultBuf, inputBuf, s );
        return( 1 );
      }

    }
    
  }

  if ( theSE == NULL ||
       (theSE->userDefinedSE.nb == 0 || theSE->userDefinedSE.list == NULL) ) {

    switch( type ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such type not handled yet (in erosion with connectivity)\n", proc );
      }
      return( 0 );
    case UCHAR :
      if ( _binary_mode_ )
        BinaryErosion_u8( inputBuf, resultBuf, theDim, connectivity, iterations );
      else 
        GreyLevelErosion_u8( inputBuf, resultBuf, theDim, connectivity, iterations );
      break;
    case USHORT :
      GreyLevelErosion_u16( inputBuf, resultBuf, theDim, connectivity, iterations );
      break;
    case SSHORT :
      GreyLevelErosion_s16( inputBuf, resultBuf, theDim, connectivity, iterations );
      break;
    case FLOAT :
      GreyLevelErosion_r32( inputBuf, resultBuf, theDim, connectivity, iterations );
      break;
    }

  } else {
    userDefinedErosion( inputBuf, resultBuf, type, theDim, 
                       &(theSE->userDefinedSE), theSE->nbIterations );
  }

  return( 1 );
}  
















static void userDefinedDilation( void *inputBuf,
                                 void *resultBuf,
                                 bufferType type,
                                 int *theDim,
                                 typeMorphoToolsList *userDefinedSE,
                                 int iterations )
{
  char *proc = "userDefinedDilation";
  int tmpMustBeAllocated = 0;
  void *tmpBuf = NULL;
  int bufferSize;

  typeMorphoToolsPoint *list;

  int i, j;
  int n;

  int mdx = 0, pdx = 0;
  int mdy = 0, pdy = 0;
  int mdz = 0, pdz = 0;
  int x, y, z;
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int born1x, born2x;
  int born1y, born2y;
  int born1z, born2z;

  int _INSIDE_Z_;
  int _INSIDE_Y_;



  if ( iterations <= 0 ) return;
  if ( userDefinedSE == NULL ||
       userDefinedSE->nb <= 0 ||
       userDefinedSE->list == NULL ) return;

  if ( iterations > 1 || inputBuf == resultBuf ) {
    tmpMustBeAllocated = 1;
  }

  bufferSize = theDim[0]*theDim[1]*theDim[2];
  switch( type ) {
  case UCHAR :
  case SCHAR :
    bufferSize *= sizeof( u8 );
    break;
  case USHORT :
  case SSHORT :
    bufferSize *= sizeof( u16 );
    break;
  case SINT :
    bufferSize *= sizeof( i32 );
    break;
  case FLOAT :
    bufferSize *= sizeof( r32 );
    break;
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such type not handled in switch (buffer size)\n",
               proc );
    }
    return;
  }
  
  if ( tmpMustBeAllocated ) {
    tmpBuf = (void*)malloc( bufferSize );
    if ( tmpBuf == NULL ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      }
      return;
    }
    
  } else {
    /*  iterations == 1 && inputBuf != resultBuf
     */
    tmpBuf = resultBuf;
  }


  /* spatial extend of the structuring element
     offset's computation
   */
  list  = userDefinedSE->list;
  for ( n=0; n<userDefinedSE->nb; n++ ) {
    if ( mdx > list[n].x ) mdx = list[n].x;
    if ( pdx < list[n].x ) pdx = list[n].x;
    if ( mdy > list[n].y ) mdy = list[n].y;
    if ( pdy < list[n].y ) pdy = list[n].y;
    if ( mdz > list[n].z ) mdz = list[n].z;
    if ( pdz < list[n].z ) pdz = list[n].z;
    list[n].o = list[n].x + list[n].y * dimx + list[n].z * dimx*dimy;
  }
  born1x = (-mdx);   born2x = dimx-pdx;
  born1y = (-mdy);   born2y = dimy-pdy;
  born1z = (-mdz);   born2z = dimz-pdz;


  switch ( type ) {
  case UCHAR :
    {
      u8 *theBuf = (u8*)inputBuf;
      u8 *auxBuf = (u8*)tmpBuf;
      u8 *resBuf = (u8*)resultBuf;
      u8 *theFoo;
      int _DEFAULT_VALUE_ = 0;
      int v;
      
      /* iteration #1: (theBuf -> inputBuf) => (auxBuf -> tmpBuf)
         iteration #2: (theBuf -> tmpBuf)   => (auxBuf -> resultBuf)
                       theBuf = auxBuf;
                       auxBuf = resBuf;
         iteration #n: swap theBuf and auxBuf
                       theFoo = theBuf;
                       theBuf = auxBuf;
                       auxBuf = theFoo;
         
         result in: theBuf
      */
      
      for ( j=0; j<iterations; j++ ) {

        for (i=0, z=0; z<dimz; z++ ) {

          _INSIDE_Z_ = 0;
          if ( z >= born1z && z < born2z ) _INSIDE_Z_= 1;

          for ( y=0; y<dimy; y++ ) {

            _INSIDE_Y_ = 0;
            if ( _INSIDE_Z_== 1 && y >= born1y && y < born2y ) _INSIDE_Y_ = 1;

            /* here list[n].x have to be tested
               against pdx and mdx
            */
            if ( born1x >= born2x ) {
              for ( x=0; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
            }
            else {
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=0; x<born1x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
              /* here no test on list[n].x
               */
              for ( x=born1x; x<born2x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=born2x; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
            }
          }
        }

        if ( j == 0 ) {
          theBuf = auxBuf;
          auxBuf = resBuf;
        } else if ( j > 0 ) {
          theFoo = theBuf;
          theBuf = auxBuf;
          auxBuf = theFoo;
        }

      }
      
      if ( theBuf != (u8*)resultBuf ) {
        memcpy( resultBuf, (void*)theBuf, bufferSize );
      }
    }
    break;
  case USHORT :
    {
      u16 *theBuf = (u16*)inputBuf;
      u16 *auxBuf = (u16*)tmpBuf;
      u16 *resBuf = (u16*)resultBuf;
      u16 *theFoo;
      int _DEFAULT_VALUE_ = 0;
      int v;
      
      /* iteration #1: (theBuf -> inputBuf) => (auxBuf -> tmpBuf)
         iteration #2: (theBuf -> tmpBuf)   => (auxBuf -> resultBuf)
                       theBuf = auxBuf;
                       auxBuf = resBuf;
         iteration #n: swap theBuf and auxBuf
                       theFoo = theBuf;
                       theBuf = auxBuf;
                       auxBuf = theFoo;
         
         result in: theBuf
      */
      
      for ( j=0; j<iterations; j++ ) {

        for (i=0, z=0; z<dimz; z++ ) {

          _INSIDE_Z_ = 0;
          if ( z >= born1z && z < born2z ) _INSIDE_Z_= 1;

          for ( y=0; y<dimy; y++ ) {

            _INSIDE_Y_ = 0;
            if ( _INSIDE_Z_== 1 && y >= born1y && y < born2y ) _INSIDE_Y_ = 1;

            /* here list[n].x have to be tested
               against pdx and mdx
            */
            if ( born1x >= born2x ) {
              for ( x=0; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
            }
            else {
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=0; x<born1x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
              /* here no test on list[n].x
               */
              for ( x=born1x; x<born2x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=born2x; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
            }
          }
        }

        if ( j == 0 ) {
          theBuf = auxBuf;
          auxBuf = resBuf;
        } else if ( j > 0 ) {
          theFoo = theBuf;
          theBuf = auxBuf;
          auxBuf = theFoo;
        }

      }
      
      if ( theBuf != (u16*)resultBuf ) {
        memcpy( resultBuf, (void*)theBuf, bufferSize );
      }
    }
    break;
  case SSHORT :
    {
      s16 *theBuf = (s16*)inputBuf;
      s16 *auxBuf = (s16*)tmpBuf;
      s16 *resBuf = (s16*)resultBuf;
      s16 *theFoo;
      int _DEFAULT_VALUE_ = -32768;
      int v;
      
      /* iteration #1: (theBuf -> inputBuf) => (auxBuf -> tmpBuf)
         iteration #2: (theBuf -> tmpBuf)   => (auxBuf -> resultBuf)
                       theBuf = auxBuf;
                       auxBuf = resBuf;
         iteration #n: swap theBuf and auxBuf
                       theFoo = theBuf;
                       theBuf = auxBuf;
                       auxBuf = theFoo;
         
         result in: theBuf
      */
      
      for ( j=0; j<iterations; j++ ) {

        for (i=0, z=0; z<dimz; z++ ) {

          _INSIDE_Z_ = 0;
          if ( z >= born1z && z < born2z ) _INSIDE_Z_= 1;

          for ( y=0; y<dimy; y++ ) {

            _INSIDE_Y_ = 0;
            if ( _INSIDE_Z_== 1 && y >= born1y && y < born2y ) _INSIDE_Y_ = 1;

            /* here list[n].x have to be tested
               against pdx and mdx
            */
            if ( born1x >= born2x ) {
              for ( x=0; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
            }
            else {
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=0; x<born1x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
              /* here no test on list[n].x
               */
              for ( x=born1x; x<born2x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=born2x; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v < theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = v;
              }
            }
          }
        }

        if ( j == 0 ) {
          theBuf = auxBuf;
          auxBuf = resBuf;
        } else if ( j > 0 ) {
          theFoo = theBuf;
          theBuf = auxBuf;
          auxBuf = theFoo;
        }

      }
      
      if ( theBuf != (s16*)resultBuf ) {
        memcpy( resultBuf, (void*)theBuf, bufferSize );
      }
    }
    break;
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such type not handled in switch (processing)\n",
               proc );
    }
    if ( tmpMustBeAllocated ) free( tmpBuf );
    return;
  }

  



  if ( tmpMustBeAllocated ) free( tmpBuf );
}












static void userDefinedErosion( void *inputBuf,
                                 void *resultBuf,
                                 bufferType type,
                                 int *theDim,
                                 typeMorphoToolsList *userDefinedSE,
                                 int iterations )
{
  char *proc = "userDefinedErosion";
  int tmpMustBeAllocated = 0;
  void *tmpBuf = NULL;
  int bufferSize;

  typeMorphoToolsPoint *list;

  int i, j;
  int n;

  int mdx = 0, pdx = 0;
  int mdy = 0, pdy = 0;
  int mdz = 0, pdz = 0;
  int x, y, z;
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int born1x, born2x;
  int born1y, born2y;
  int born1z, born2z;

  int _INSIDE_Z_;
  int _INSIDE_Y_;



  if ( iterations <= 0 ) return;
  if ( userDefinedSE == NULL ||
       userDefinedSE->nb <= 0 ||
       userDefinedSE->list == NULL ) return;

  if ( iterations > 1 || inputBuf == resultBuf ) {
    tmpMustBeAllocated = 1;
  }

  bufferSize = theDim[0]*theDim[1]*theDim[2];
  switch( type ) {
  case UCHAR :
  case SCHAR :
    bufferSize *= sizeof( u8 );
    break;
  case USHORT :
  case SSHORT :
    bufferSize *= sizeof( u16 );
    break;
  case SINT :
    bufferSize *= sizeof( i32 );
    break;
  case FLOAT :
    bufferSize *= sizeof( r32 );
    break;
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such type not handled in switch (buffer size)\n",
               proc );
    }
    return;
  }
  
  if ( tmpMustBeAllocated ) {
    tmpBuf = (void*)malloc( bufferSize );
    if ( tmpBuf == NULL ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      }
      return;
    }
    
  } else {
    /*  iterations == 1 && inputBuf != resultBuf
     */
    tmpBuf = resultBuf;
  }


  /* spatial extend of the structuring element
     offset's computation
   */
  list  = userDefinedSE->list;
  for ( n=0; n<userDefinedSE->nb; n++ ) {
    if ( mdx > list[n].x ) mdx = list[n].x;
    if ( pdx < list[n].x ) pdx = list[n].x;
    if ( mdy > list[n].y ) mdy = list[n].y;
    if ( pdy < list[n].y ) pdy = list[n].y;
    if ( mdz > list[n].z ) mdz = list[n].z;
    if ( pdz < list[n].z ) pdz = list[n].z;
    list[n].o = list[n].x + list[n].y * dimx + list[n].z * dimx*dimy;
  }
  born1x = (-mdx);   born2x = dimx-pdx;
  born1y = (-mdy);   born2y = dimy-pdy;
  born1z = (-mdz);   born2z = dimz-pdz;



  switch ( type ) {
  case UCHAR :
    {
      u8 *theBuf = (u8*)inputBuf;
      u8 *auxBuf = (u8*)tmpBuf;
      u8 *resBuf = (u8*)resultBuf;
      u8 *theFoo;
      int _DEFAULT_VALUE_ = 256;
      int v;
      
      /* iteration #1: (theBuf -> inputBuf) => (auxBuf -> tmpBuf)
         iteration #2: (theBuf -> tmpBuf)   => (auxBuf -> resultBuf)
                       theBuf = auxBuf;
                       auxBuf = resBuf;
         iteration #n: swap theBuf and auxBuf
                       theFoo = theBuf;
                       theBuf = auxBuf;
                       auxBuf = theFoo;
         
         result in: theBuf
      */
      
      for ( j=0; j<iterations; j++ ) {

        for (i=0, z=0; z<dimz; z++ ) {
          _INSIDE_Z_ = 0;
          if ( z >= born1z && z < born2z ) _INSIDE_Z_= 1;
          for ( y=0; y<dimy; y++ ) {
            _INSIDE_Y_ = 0;
            if ( _INSIDE_Z_== 1 && y >= born1y && y < born2y ) _INSIDE_Y_ = 1;

            /* here list[n].x have to be tested
               against pdx and mdx
            */
            if ( born1x >= born2x ) {
              for ( x=0; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
            }
            else {
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=0; x<born1x && x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
              /* here no test on list[n].x
               */
              for ( x=born1x; x<born2x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=born2x; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
            }
          }
        }

        if ( j == 0 ) {
          theBuf = auxBuf;
          auxBuf = resBuf;
        } else if ( j > 0 ) {
          theFoo = theBuf;
          theBuf = auxBuf;
          auxBuf = theFoo;
        }

      }
      
      if ( theBuf != (u8*)resultBuf ) {
        memcpy( resultBuf, (void*)theBuf, bufferSize );
      }
    }
    break;
  case USHORT :
    {
      u16 *theBuf = (u16*)inputBuf;
      u16 *auxBuf = (u16*)tmpBuf;
      u16 *resBuf = (u16*)resultBuf;
      u16 *theFoo;
      int _DEFAULT_VALUE_ = 65536;
      int v;
      
      /* iteration #1: (theBuf -> inputBuf) => (auxBuf -> tmpBuf)
         iteration #2: (theBuf -> tmpBuf)   => (auxBuf -> resultBuf)
                       theBuf = auxBuf;
                       auxBuf = resBuf;
         iteration #n: swap theBuf and auxBuf
                       theFoo = theBuf;
                       theBuf = auxBuf;
                       auxBuf = theFoo;
         
         result in: theBuf
      */
      
      for ( j=0; j<iterations; j++ ) {

        for (i=0, z=0; z<dimz; z++ ) {
          _INSIDE_Z_ = 0;
          if ( z >= born1z && z < born2z ) _INSIDE_Z_= 1;
          for ( y=0; y<dimy; y++ ) {
            _INSIDE_Y_ = 0;
            if ( _INSIDE_Z_== 1 && y >= born1y && y < born2y ) _INSIDE_Y_ = 1;

            /* here list[n].x have to be tested
               against pdx and mdx
            */
            if ( born1x >= born2x ) {
              for ( x=0; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
            }
            else {
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=0; x<born1x && x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
              /* here no test on list[n].x
               */
              for ( x=born1x; x<born2x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=born2x; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
            }
          }
        }

        if ( j == 0 ) {
          theBuf = auxBuf;
          auxBuf = resBuf;
        } else if ( j > 0 ) {
          theFoo = theBuf;
          theBuf = auxBuf;
          auxBuf = theFoo;
        }

      }
      
      if ( theBuf != (u16*)resultBuf ) {
        memcpy( resultBuf, (void*)theBuf, bufferSize );
      }      
    }
    break;
  case SSHORT :
    {
      s16 *theBuf = (s16*)inputBuf;
      s16 *auxBuf = (s16*)tmpBuf;
      s16 *resBuf = (s16*)resultBuf;
      s16 *theFoo;
      int _DEFAULT_VALUE_ = 32768;
      int v;
      
      /* iteration #1: (theBuf -> inputBuf) => (auxBuf -> tmpBuf)
         iteration #2: (theBuf -> tmpBuf)   => (auxBuf -> resultBuf)
                       theBuf = auxBuf;
                       auxBuf = resBuf;
         iteration #n: swap theBuf and auxBuf
                       theFoo = theBuf;
                       theBuf = auxBuf;
                       auxBuf = theFoo;
         
         result in: theBuf
      */
      
      for ( j=0; j<iterations; j++ ) {

        for (i=0, z=0; z<dimz; z++ ) {
          _INSIDE_Z_ = 0;
          if ( z >= born1z && z < born2z ) _INSIDE_Z_= 1;
          for ( y=0; y<dimy; y++ ) {
            _INSIDE_Y_ = 0;
            if ( _INSIDE_Z_== 1 && y >= born1y && y < born2y ) _INSIDE_Y_ = 1;

            /* here list[n].x have to be tested
               against pdx and mdx
            */
            if ( born1x >= born2x ) {
              for ( x=0; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
            }
            else {
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=0; x<born1x && x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x < 0 ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
              /* here no test on list[n].x
               */
              for ( x=born1x; x<born2x; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
              /* here list[n].x have to be tested
                 against mdx
              */
              for ( x=born2x; x<dimx; x++, i++ ) {
                v = _DEFAULT_VALUE_;
                if ( _INSIDE_Y_ == 1 ) {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                } else {
                  for ( n=0; n<userDefinedSE->nb; n++ ) {
                    if ( x+list[n].x >= dimx ) continue;
                    if ( y+list[n].y < 0 ) continue;
                    if ( y+list[n].y >= dimy ) continue;
                    if ( z+list[n].z < 0 ) continue;
                    if ( z+list[n].z >= dimz ) continue;
                    if ( v > theBuf[i+list[n].o] ) v = theBuf[i+list[n].o];
                  }
                }
                auxBuf[i] = ( v == _DEFAULT_VALUE_ ) ? 0 : v;
              }
            }
          }
        }

        if ( j == 0 ) {
          theBuf = auxBuf;
          auxBuf = resBuf;
        } else if ( j > 0 ) {
          theFoo = theBuf;
          theBuf = auxBuf;
          auxBuf = theFoo;
        }

      }
      
      if ( theBuf != (s16*)resultBuf ) {
        memcpy( resultBuf, (void*)theBuf, bufferSize );
      }      
    }
    break;
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such type not handled in switch (processing)\n",
               proc );
    }
    if ( tmpMustBeAllocated ) free( tmpBuf );
    return;
  }

  



  if ( tmpMustBeAllocated ) free( tmpBuf );
}








void useBinaryMorphologicalOperations()
{
  _binary_mode_ = 1;
}
void useGreyLevelMorphologicalOperations()
{
  _binary_mode_ = 0;
}







void initStructuringElement ( typeStructuringElement *SE )
{
  SE->nbIterations = 1;
  SE->connectivity = 26;
  SE->userDefinedSE.nb = 0;
  SE->userDefinedSE.list = NULL;
  SE->radius = 0;
  SE->dimension = 3;
}

void freeStructuringElement ( typeStructuringElement *SE )
{
  if ( SE->userDefinedSE.list != NULL ) 
    free( SE->userDefinedSE.list );
  initStructuringElement( SE );
}



void printStructuringElement ( FILE *f, typeStructuringElement *SE, char *str )
{
  int i;
  
  if ( str != (char*)NULL )
    fprintf( f, "Structuring element: '%s'\n", str );
  fprintf( f, "#iterations  = %d\n", SE->nbIterations );
  fprintf( f, "connectivity = %d\n", SE->connectivity );
  fprintf( f, "radius       = %d\n", SE->radius );
  fprintf( f, "dimension    = %d\n", SE->dimension );
  if ( SE->userDefinedSE.nb > 0 ) {
    for ( i=0; i<SE->userDefinedSE.nb; i++ )
      fprintf( f, "pt[%3d] = (%4d, %4d, %4d)\n", i, 
               SE->userDefinedSE.list[i].x,
               SE->userDefinedSE.list[i].y,
               SE->userDefinedSE.list[i].z );
  }
  
}



int buildStructuringElementAs3DSphere( typeStructuringElement *SE, int radius )
{
  char *proc = "buildStructuringElementAs3DSphere";
  int *buf = (int *)NULL;
  int i, x, y, z, d, n;
  typeMorphoToolsPoint *list = NULL;

  SE->userDefinedSE.nb = 0;

  d = 2*radius + 1;
  buf = (int *)malloc( d*d*d * sizeof( int ) );
  if ( buf == (int *)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( 0 );
  }

  for ( i=0, z=0; z<d; z++ )
  for ( y=0; y<d; y++ )
    for ( x=0; x<d; x++, i++ ) {
    buf[i] = (z-radius)*(z-radius)+(y-radius)*(y-radius)+(x-radius)*(x-radius);
    if ( buf[i] > radius*radius )
      buf[i] = 0;
    else buf[i] = 1;
  }
  
  n = 0;
  for ( i=0, z=0; z<d; z++ )
  for ( y=0; y<d; y++ )
    for ( x=0; x<d; x++, i++ )
    if ( buf[i] > 0 ) n ++;
  
  list = (typeMorphoToolsPoint *)malloc( n * sizeof(typeMorphoToolsPoint) );
  if ( list == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: allocation failed.\n", proc );
    }
    return( 0 );
  }

  SE->userDefinedSE.nb = n;
  n = 0;

  for ( i=0, z=0; z<d; z++ )
  for ( y=0; y<d; y++ )
  for ( x=0; x<d; x++, i++ ) {
    if ( buf[i] > 0 ) {
      list[n].x = x - radius;
      list[n].y = y - radius;
      list[n].z = z - radius;
      n++;
    }
  }
  SE->userDefinedSE.list = list;
  
  free( buf );

  return( 1 );
}



int buildStructuringElementFromImage( unsigned char *theBuf,
                                      int *theDim,
                                      int *theCenter,
                                      typeStructuringElement *SE )
{
  char *proc = "buildStructuringElementFromImage";
  int i, x, y, z;
  int n = 0;
  typeMorphoToolsPoint *list = NULL;

  SE->userDefinedSE.nb = 0;

  for ( i=0, z=0; z<theDim[2]; z++ )
  for ( y=0; y<theDim[1]; y++ )
  for ( x=0; x<theDim[0]; x++, i++ ) {
    if ( theBuf[i] > 0 ) n++;
  }

  if ( n == 0 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: empty structuring element in image.\n", proc );
    }
    return( 0 );
  }

  
  list = (typeMorphoToolsPoint *)malloc( n * sizeof(typeMorphoToolsPoint) );
  if ( list == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: allocation failed.\n", proc );
    }
    return( 0 );
  }



  SE->userDefinedSE.nb = n;
  n = 0;

  for ( i=0, z=0; z<theDim[2]; z++ )
  for ( y=0; y<theDim[1]; y++ )
  for ( x=0; x<theDim[0]; x++, i++ ) {
    if ( theBuf[i] > 0 ) {
      list[n].x = x - theCenter[0];
      list[n].y = y - theCenter[1];
      list[n].z = z - theCenter[2];
      n++;
    }
  }
  SE->userDefinedSE.list = list;
  
  return( 1 );
}







int readStructuringElement( char *fileName,
                            typeStructuringElement *SE )
{
  char *proc = "readStructuringElement";
  FILE *fp;
  char *tmp, *str = NULL;
  int elt_size = 0;
  int str_size = 512;
  int dimx = 0;
  int dimy = 0;
  int dimz = 0;
  char *theBuf = NULL;
  int x, y, z, n=0;
  int v, nbpts=0;
  int centre[3];
  
  str = (char*)malloc( str_size * sizeof(char) );
  if ( str == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to allocate auxiliary string.\n", proc );
    }
    return( 0 );
  }
  

  fp = fopen ( fileName, "r" );
  if ( fp == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to open file '%s'.\n", proc, fileName );
    }
    return( 0 );
  }



  while( fgets( str, str_size, fp ) ) {

    if ( str[0] == '#' ) continue;

    if ( strncmp( str, "XDIM=", 5 ) == 0 ) {
      if ( sscanf(str+5, "%d", &dimx ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read X dimension in '%s'\n",
                   proc, fileName );
        if ( theBuf != NULL ) free( theBuf );
        free( str );
        fclose( fp );
        return( 0 );
      }
      continue;
    }
    if ( strncmp( str, "YDIM=", 5 ) == 0 ) {
      if ( sscanf(str+5, "%d", &dimy ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read Y dimension in '%s'\n",
                   proc, fileName );
        if ( theBuf != NULL ) free( theBuf );
        free( str );
        fclose( fp );
        return( 0 );
      }
      continue;
    }
    if ( strncmp( str, "ZDIM=", 5 ) == 0 ) {
      if ( sscanf(str+5, "%d", &dimz ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read Z dimension in '%s'\n",
                   proc, fileName );
        if ( theBuf != NULL ) free( theBuf );
        free( str );
        fclose( fp );
        return( 0 );
      }
      continue;
    }
    
    if ( theBuf == NULL && dimx > 0 && dimy > 0 && dimz > 0 ) {
      elt_size = dimx*dimy*dimz;
      theBuf = (char*)malloc( elt_size * sizeof(char) );
      if ( theBuf == NULL ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate buffer.\n", proc );
        free( str );
        fclose( fp );
        return( 0 );
      }
    }
    
    tmp = str;
    while ( *tmp != '\n' && *tmp != '\0' && *tmp != EOF && n < elt_size ) {
      /* skip trailing whitespace
       */
      while ( *tmp == ' ' || *tmp == '\t' )
        tmp++;
      if ( *tmp == '\n' || *tmp == EOF )
        continue;

      /* read '.'
       */
      if ( tmp[0] == '.' && ( tmp[1] == ' ' ||
                              tmp[1] == '\0' ||
                              tmp[1] == '\t' ||
                              tmp[1] == '\n' ||
                              tmp[1] == EOF ) ) {
        theBuf[n] = 0;
        n++;
        tmp ++;
        continue;
      }

      /* read '+'
       */
      if ( tmp[0] == '+' && ( tmp[1] == ' ' ||
                              tmp[1] == '\0' ||
                              tmp[1] == '\t' ||
                              tmp[1] == '\n' ||
                              tmp[1] == EOF ) ) {
        theBuf[n] = 100;
        n++;
        tmp ++;
        continue;
      }

      if ( tmp[0] == '-' && ( tmp[1] == ' ' ||
                              tmp[1] == '\0' ||
                              tmp[1] == '\t' ||
                              tmp[1] == '\n' ||
                              tmp[1] == EOF ) ) {
        theBuf[n] = -100;
        n++;
        tmp ++;
        continue;
      }

      /* read a number
       */
      if ( sscanf( tmp, "%d", &v ) != 1 ) {
        if ( _verbose_ ) {
          fprintf( stderr, "%s: error in reading '%s'\n",
                   proc, fileName );
          fprintf( stderr, "\t in line '%s' at [%ld]:'%s'.\n", str, tmp-str, tmp );
        }
        if ( theBuf != NULL ) free( theBuf );
        free( str );
        fclose( fp );
        return( 0 );
      }
      if ( v > 0 ) theBuf[n] = 1;
      else         theBuf[n] = 0;
      n++;

      /* skip a number 
       */
      while ( (*tmp >= '0' && *tmp <= '9') || *tmp == '.' || *tmp == '-' )
        tmp++;
    }
  }

  free( str );
  fclose( fp );
  
  

  nbpts = 0;
  centre[0] = (dimx+1)/2 - 1;
  centre[1] = (dimy+1)/2 - 1;
  centre[2] = (dimz+1)/2 - 1;

  for ( n=0, z=0; z<dimz; z++ )
  for ( y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++,n++ ) {
    if ( theBuf[n] > 0 ) nbpts ++;
    if ( theBuf[n] == 100 || theBuf[n] == -100 ) {
      centre[0] = x;
      centre[1] = y;
      centre[2] = z;
    }
  }

  if ( _verbose_ >= 2 ) {
    printf( "\n" );
    for ( n=0, z=0; z<dimz; z++ ) {
      for ( y=0; y<dimy; y++ ) {
        for ( x=0; x<dimx; x++,n++ ) {
          printf( " %4d", theBuf[n] );
      }
        printf( "\n" );
      }
      printf( "\n" );
    }
    printf( " nb d'elements = %d\n", nbpts );
    printf( " centre = [%d %d %d]\n", centre[0], centre[1], centre[2] );
    printf( "\n" );
  }

  SE->userDefinedSE.list = (typeMorphoToolsPoint *)malloc(nbpts*sizeof(typeMorphoToolsPoint));
  if ( SE->userDefinedSE.list == NULL ) {
    fprintf( stderr, "%s: unable to allocate structuring element.\n", proc );
    if ( theBuf != NULL ) free( theBuf );
    return( 0 );
  }
  SE->userDefinedSE.nb = nbpts;
  
  for ( v=0, n=0, z=0; z<dimz; z++ )
  for ( y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++,n++ ) {
    
    if ( theBuf[n] > 0 ) {
      SE->userDefinedSE.list[v].x = x - centre[0];
      SE->userDefinedSE.list[v].y = y - centre[1];
      SE->userDefinedSE.list[v].z = z - centre[2];
      v++;
    }
  }


  if ( theBuf != NULL ) free( theBuf );

  return( 1 );
}

void MorphoTools_verbose()
{
  if ( _verbose_ <= 0 ) _verbose_ = 1;
  else _verbose_ ++;
}

void MorphoTools_noverbose()
{
  _verbose_ = 0;
}

