/*************************************************************************
 * test-reech-pnm.c - 
 *
 * $Id: test-reech-pnm.c,v 1.2 2006/04/14 08:38:55 greg Exp $
 *
 * Copyright (c) INRIA 2000 
 * DESCRIPTION: 
 *
 * Input must be of PBM/PGM/PPM raw format, output will be the same.
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Fri Jun 16 11:41:55 MET DST 2000
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iopnm.h>
#include <typedefs.h>
#include <reech4x4.h>


static int _verbose_ = 0;

static char program[256];
static char *usage = "image-in image-out [-nearest] [-mat %s] [-mat-example] [-inv] [-v] [-help]";
static char *details ="\n\
Linear resampling\n\
--------------\n\
\n\
\t -mat : 4x4 matrix (homogenous coordinates) should be of the form\n\
\t        R00 R01 R02 T0\n\
\t        R10 R11 R12 T1\n\
\t        R20 R21 R22 T2\n\
\t        0.0 0.0 0.0 1.0\n\
\t   The 3x3 upper left coefficients are the rotation matrix R while the Ti\n\
\t   are the 3D translation vector. Hence the transformation equation is\n\
\t   M' = R M + T\n\
\t        M is the point in the output image (with integer coordinates) \n\
\t        M' is its correposdant in the input image (with real corrdinates)\n\
\t        the value of M' is interpolated and is attributed to M\n\
\t -nearest : use the value of the nearest neighbor instead of a linear interpolation\n\
\t -mat-example : print a matrix on stdout\n\
\t -inv : inverse the matrix\n\
\t -v | -verbose\n\
\t -nv | -no-verbose\n\
\t -h | -help : print this message";

static void ErrorMessage( char *str, int flag )
{
  (void)fprintf(stderr,"Usage: %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",details);
  (void)fprintf(stderr,"Error: %s",str);
  exit(0);
}

static int  ReadMatrice( char *name, double *mat );
static int  InverseMat4x4( double *matrice, double *inv );




typedef enum {
  _NEAREST_,
  _LINEAR_
} enumInterpolation;





int main( int argc, char* argv[] )
{
  char nameImageIn[256];
  char nameImageOut[256];
  int nbNames = 0;
  int i;

  void *bufferIn = (void*)NULL;
  void *bufferOut = (void*)NULL;
  int bufferDims[3] = {0,0,0};
  int nbytes;
  bufferType TYPE = UCHAR;
  double matrix[16] = { 0.8660254, -0.5, 0, 80.5147742,
		     0.5, 0.8660254, 0, -46.4852258,
		     0, 0, 1, 0, 0, 0, 0, 1 };
  int inverse = 0;
  enumInterpolation typeInterpolation = _LINEAR_;



  strcpy( program, argv[0] );

  if ( argc == 1 ) ErrorMessage( "\n", 1 );

  for ( i=1; i<argc; i++ ) {
    if ( argv[i][0] == '-' ) {

      if ( (strcmp ( argv[i], "-help" ) == 0) || 
	   (strcmp ( argv[i], "-h" ) == 0) ) {
	ErrorMessage( "help message\n", 1 );
      }
      
      else if ( (strcmp ( argv[i], "-mat-example" ) == 0) ) {
	printf( "0.8660254 -0.5       0.0  80.5147742\n" );
	printf( "0.5        0.8660254 0.0  -46.4852258\n" );
	printf( "0.0        0.0       1.0  0.0\n " );
	printf( "0.0        0.0       0.0  1.0\n " );
	fprintf( stderr, "Rotation of 30 degrees in the XY plane\n");
	fprintf( stderr, "whose center is (127,127)\n" );
      }

      else if ( (strcmp ( argv[i], "-mat" ) == 0) ) {
	i += 1;
	if ( i >= argc)    ErrorMessage( "parsing -mat...\n", 0 );
	if ( ReadMatrice( argv[i], matrix ) != 1 ) {
	  ErrorMessage( "reading matrix...\n", 0 );
	}
      }

      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	inverse = 1;
      }

      else if ( strcmp ( argv[i], "-nearest" ) == 0 ) {
	typeInterpolation = _NEAREST_;
      }

      else if ( (strcmp ( argv[i], "-verbose" ) == 0) || 
		(strcmp ( argv[i], "-v" ) == 0) ) {
	_verbose_ = 1;
	incrementVerboseInReech4x4();
      }

      else if ( (strcmp ( argv[i], "-no-verbose" ) == 0) || 
		(strcmp ( argv[i], "-nv" ) == 0) ) {
	_verbose_ = 0;
	setVerboseInReech4x4( 0 );
      }

      else {
	sprintf( nameImageIn, "unknown option %s\n", argv[i] );
	ErrorMessage( nameImageIn, 0);
      }
    }

    else if ( argv[i][0] != 0 ) {
      if ( nbNames == 0 ) {
	strcpy( nameImageIn, argv[i] );
      } 
      else if ( nbNames == 1 ) {
	strcpy( nameImageOut, argv[i] );
      } 
      else {
	sprintf( nameImageIn, "too many image name (%s)\n", argv[i] );
	ErrorMessage( nameImageIn, 0);
      }
      nbNames ++;
    }
  }



  bufferIn = _readPnmImage( nameImageIn, &bufferDims[0], &bufferDims[1], &bufferDims[2], &nbytes );
  if ( nbytes == 2 ) TYPE = USHORT;

  bufferOut = (void*)malloc( bufferDims[0] * bufferDims[1] * bufferDims[2] * nbytes * sizeof(unsigned char) );


  if ( inverse ) {
    double tmp[16];
    if ( InverseMat4x4( matrix, tmp ) != 4 ) {
      ErrorMessage( "Warning: matrix is not inversible\n", 0);
    }
    for (i=0;i<16;i++) matrix[i] = tmp[i];
  } 
  
  switch ( typeInterpolation ) {
  case _LINEAR_ :
  default :
    switch ( TYPE ) {
    default : break;
    case UCHAR :
      Reech3DTriLin4x4_u8( bufferIn, bufferDims, bufferOut, bufferDims, matrix );
      break;
    case USHORT :
      Reech3DTriLin4x4_u16( bufferIn, bufferDims, bufferOut, bufferDims, matrix );
      break;
    }
    break;
  case _NEAREST_ :
    switch ( TYPE ) {
    default : break;
    case UCHAR :
      Reech3DNearest4x4_u8( bufferIn, bufferDims, bufferOut, bufferDims, matrix );
      break;
    case USHORT :
      Reech3DNearest4x4_u16( bufferIn, bufferDims, bufferOut, bufferDims, matrix );
      break;
    }
    break;
  }
      
  _writePnmImage( nameImageOut, bufferDims[0], bufferDims[1], bufferDims[2], nbytes, bufferOut );
    
  exit( 0 );
}








#define TEXTLENGTH 256

static int ReadMatrice( char *name, double *mat )
{
  FILE *fopen(), *fp;
  char text[TEXTLENGTH];
  int nbelts = 0;
  int status;
  
  /* lecture de 4 double par ligne
     On prevoit le cas ou la ligne commence par "O8 xxxxx ...
     */

  fp = fopen( name, "r" );
  if ( fp == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "   can not open file '%s'\n", name );
    return( 0 );
  }
  
  while ( (nbelts < 16) && (fgets( text, TEXTLENGTH, fp ) != NULL) ) {
    if ( (text[0] == 'O') && (text[1] == '8') ) {
      status = sscanf( &(text[2]), "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    } else {
      status = sscanf( text, "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    }
    if ( status == 4 ) nbelts += 4;
  }
  fclose( fp );

  if ( _verbose_ > 0 ) {
    fprintf( stderr, " reading matrix '%s'\n", name );
    fprintf( stderr, " %d elements\n", nbelts );
    fprintf( stderr,"   %f %f %f %f\n", mat[0], mat[1], mat[2], mat[3] );
    fprintf( stderr,"   %f %f %f %f\n", mat[4], mat[5], mat[6], mat[7] );
    fprintf( stderr,"   %f %f %f %f\n", mat[8], mat[9], mat[10], mat[11] );
    fprintf( stderr,"   %f %f %f %f\n", mat[12], mat[13], mat[14], mat[15] );
  }
  if ( nbelts == 16 ) return ( 1 );
  return( 0 );
}











#define TINY 1e-12


static int InverseMat4x4( double *matrice, double *inv )
{
  register int i, j, k;
  int kmax, rang = 4;
  register double c, max;
  double mat [16];
  
  for (i=0; i<16; i++ ) {
    mat[i] = matrice[i] ;
    inv[i] = 0.0;
  }
  inv[0] = inv[5] = inv[10] = inv[15] = 1.0;
  
  for ( j=0; j<4; j++ ) {
    if ( (mat[j*4+j] > (-TINY)) && (mat[j*4+j] < TINY) ) {
      /* recherche du plus grand element non nul sur la colonne j */
      kmax = j;
      max = 0.0;
      for (k=j+1; k<4; k++ ) {
	c = ( mat[k*4+j] > 0.0 ) ? mat[k*4+j] : (-mat[k*4+j]) ;
	if ( (c > TINY) && (c > max) ) { max = c; kmax = k; }
      }
      if ( kmax == j ) {
	/* la ligne est nulle */
	rang --;
      } else {
	/* sinon, on additionne */
	for ( i=0; i<4; i++ ) {
	  mat[j*4+i] += mat[kmax*4+i];
	  inv[j*4+i] += inv[kmax*4+i];
	}
      }
    }
    if ( (mat[j*4+j] < (-TINY)) || (mat[j*4+j] > TINY) ) {
      /* les autres lignes */
      for (k=0; k<4; k++) {
	if ( k != j ) {
	  c = mat[k*4 + j] / mat[j*4 + j];
	  for ( i=0; i<4; i++ ) {
	    mat[k*4 + i] -= c * mat[j*4 + i];
	    inv[k*4 + i] -= c * inv[j*4 + i];
	  }
	}
      }
      /* la ligne */
      c = mat[j*4 + j];
      for ( i=0; i<4; i++ ) {
	mat[j*4 + i] /= c;
	inv[j*4 + i] /= c;
      }
    }
  }

  return( rang );
}
