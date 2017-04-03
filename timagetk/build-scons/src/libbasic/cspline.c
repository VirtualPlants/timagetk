/*************************************************************************
 * cspline.c - Cubic splines
 *
 * $Id: cspline.c,v 1.3 2006/04/14 08:38:55 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Alexis Roche
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Oct 11 23:10:01 MEST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */


#include <cspline.h>

static int _verbose_ = 1;


static void CubicSpline_Transform ( double * s, double * c, int N );
static double CubicBspline ( double x );
static double CubicBspline_FirstDeriv ( double x );
static double CubicBspline_SecndDeriv ( double x );
















int CSplineFilterOnBuffer( void *bufferIn,    /* input buffer */
                           bufferType typeIn, /* type of the input buffer */
                           void *bufferOut, /* output buffer */
                           bufferType typeOut, /* type of the output buffer */
                           int *bufferDims, /* buffers' dimensions */
                           int *derivatives /* order of derivatives to be computed */
                           )
{
  char *proc = "CSplineFilterOnBuffer";
  typeCSplineCoefficients *c = NULL;

  c = ComputeCSplineCoefficients( bufferIn, typeIn, bufferDims );
  if ( c == NULL ) {
    if( _verbose_ )
      fprintf( stderr, "%s: unable to compute cubic spline coefficients.\n", proc );
    return( -1 );
  }


  if ( CSplineFilterOnBufferWithCoefficients( c, bufferOut, typeOut, derivatives ) != 1 ) {
    FreeTypeCSplineCoefficients( &c );
    fprintf( stderr, "%s: unable to filter image.\n", proc );
    return( -1 );
  }
       

  FreeTypeCSplineCoefficients( &c );
  return( 1 );
}











int CSplineFilterOnBufferWithCoefficients( typeCSplineCoefficients *theCoeff,
                                           void *bufferOut, /* output buffer */
                                           bufferType typeOut, /* type of the output buffer */
                                           int *derivative /* order of derivatives to be computed */
                                           )
{
  char *proc ="CSplineFilterOnBufferWithCoefficients";
  double (*bsplinx)(double);
  double (*bspliny)(double);
  double (*bsplinz)(double) = NULL;

  int dimx = theCoeff->theDim[0]-2;
  int dimy = theCoeff->theDim[1]-2;
  int dimz;

  int    i, j, n;
  int    o[27];
  double r, cx[3], cy[3], cz[3], c[27];
  
  float *tmpBuf = NULL;
  float *coeBuf = theCoeff->theCoeff;

  int x, y, z;



  /* dimensions selon Z
     les bords ont ete dupliques dans l'image des
     coefficients
   */
  if ( theCoeff->theDim[2] == 1 ) dimz = 1;
  else dimz = theCoeff->theDim[2]-2;



  switch( derivative[0] ) {
  default :
    if ( _verbose_ ) 
      fprintf( stderr, "%s: invalid derivation order in x\n", proc );
    return( -1 );
  case 0 :
    bsplinx = &CubicBspline;            break;
  case 1 :
    bsplinx = &CubicBspline_FirstDeriv; break;
  case 2:
    bsplinx = &CubicBspline_SecndDeriv; break;
  }

  switch( derivative[1] ) {
  default :
    if ( _verbose_ ) 
      fprintf( stderr, "%s: invalid derivation order in y\n", proc );
    return( -1 );
  case 0 :
    bspliny = &CubicBspline;            break;
  case 1 :
    bspliny = &CubicBspline_FirstDeriv; break;
  case 2:
    bspliny = &CubicBspline_SecndDeriv; break;
  }

  if ( dimz > 1 ) {
    switch( derivative[2] ) {
    default :
      if ( _verbose_ ) 
        fprintf( stderr, "%s: invalid derivation order in z\n", proc );
      return( -1 );
    case 0 :
      bsplinz = &CubicBspline;            break;
    case 1 :
      bsplinz = &CubicBspline_FirstDeriv; break;
    case 2:
      bsplinz = &CubicBspline_SecndDeriv; break;
    }
  }
  


  /* Pre-calcul des coefficients
     coeff[0] = bspline(  1 );
     coeff[1] = bspline(  0 );
     coeff[2] = bspline( -1 );
     Question: c'est inverse ?
  */
  for ( i = 0; i < 3; i ++ ) {
    cx [i] = (*bsplinx) ( (double)(1-i) );
    cy [i] = (*bspliny) ( (double)(1-i) );
  }
  if ( dimz > 1 ) {
    for ( i = 0; i < 3; i ++ )
      cz [i] = (*bsplinz) ( (double)(1-i) );
  }
  

  /* calcul des coefficients
     et des offsets associes
  */

  if ( dimz > 1 ) {
    for ( i=0, z=0; z<3; z++ )
    for ( y=0; y<3; y++ )
    for ( x=0; x<3; x++, i++ ) {
      c[i] = cx[x] * cy[y] * cz[z];
      o[i] = (z-1)*theCoeff->theDim[0]*theCoeff->theDim[1] +
             (y-1)*theCoeff->theDim[0] + (x-1);
    }
    n = 27;
  } else {
    for ( i=0, y=0; y<3; y++ )
    for ( x=0; x<3; x++, i++ ) {
      c[i] = cx[x] * cy[y];
      o[i] = (y-1)*theCoeff->theDim[0] + (x-1);
    }
    n = 9;
  }



  /*
  if ( 1 ) {
    fprintf( stderr, "\n" );
    for ( i=0; i<n; i++ ) {
      fprintf( stderr, " #[%4d]=%f ", o[i], c[i] );
      if ( (i+1)%3 == 0 ) fprintf( stderr, "\n" );
      if ( (i+1)%9 == 0 ) fprintf( stderr, "\n" );
    }
    fprintf( stderr, "\n" );
  }
  */



  if ( typeOut != FLOAT ) {
    tmpBuf = (float*)malloc( dimx*dimy* sizeof(float) );
    if ( tmpBuf == NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      return( -1 );
    }
  }


  /* cas 3D, on saute le premier plan 
   */
  if ( dimz > 1 ) coeBuf += theCoeff->theDim[0]*theCoeff->theDim[1];

  for ( z=0; z<dimz; z++ ) {

    if ( typeOut == FLOAT ) tmpBuf = &((float*)bufferOut)[z*dimx*dimy];
    
    coeBuf += theCoeff->theDim[0] + 1;

    for ( j=0, y=0; y<dimy; y++, coeBuf+=2 ) {
      for ( x=0; x<dimx; x++, coeBuf++, j++ ) {
        for (r=0.0, i=0; i<n; i++ )
          r += c[i] * coeBuf[ o[i] ];
        tmpBuf[j] = (float)r;
      }
    }

    coeBuf += dimx+1;

    if ( typeOut != FLOAT ) {
      switch ( typeOut ) {
      default :
        free( tmpBuf );
        if ( _verbose_ )
          fprintf( stderr, "%s: such output type not handled in switch\n", proc );
        return( -1 );
      case UCHAR :
        if ( ConvertBuffer( tmpBuf, FLOAT, &((u8*)bufferOut)[z*dimx*dimy],
                            typeOut, dimx*dimy ) != 1 ) {
          free( tmpBuf );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
        break;
      case SCHAR :
        if ( ConvertBuffer( tmpBuf, FLOAT, &((s8*)bufferOut)[z*dimx*dimy],
                            typeOut, dimx*dimy ) != 1 ) {
          free( tmpBuf );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
        break;
      case USHORT :
        if ( ConvertBuffer( tmpBuf, FLOAT, &((u16*)bufferOut)[z*dimx*dimy],
                            typeOut, dimx*dimy ) != 1 ) {
          free( tmpBuf );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
        break;
      case SSHORT :
        if ( ConvertBuffer( tmpBuf, FLOAT, &((s16*)bufferOut)[z*dimx*dimy],
                            typeOut, dimx*dimy ) != 1 ) {
          free( tmpBuf );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
      }
    }
  }




  if ( typeOut != FLOAT ) free( tmpBuf );
  return( 1 );
}


























/* Fonctions pour l'interpolation spline cubique. 
   A. Roche
*/


/* Cette fonction renvoie la valeur de la fonction B-spline
   cubique elementaire en x */

static double CubicBspline ( double x )
{
  double aux;
  
  if ( x >= 0.0 ) {
    if ( x >=  2.0 ) return( 0.0 );
    if ( x < 1 ) {
      aux = x*x;
      return( (double)2.0/(double)3.0 - aux + (double)0.5*aux*x );
    }
    aux = 2.0 - x;
    return( aux*aux*aux/(double)6.0 );
  } 
  
  /* ici x < 0
   */
  if ( x <= -2.0 ) return( 0.0 );
  if ( x > -1 ) {
    aux = x*x;
    return( (double)2.0/(double)3.0 - aux - (double)0.5*aux*x );
  }
  aux = 2.0 + x;
  return( aux*aux*aux/(double)6.0 );
}





static double CubicBspline_FirstDeriv ( double x ) 
{
  if ( x == 0.0 ) return( 0.0 );

  if ( x > 0.0 ) {
    if ( x >=  2.0 ) return( 0.0 );
    if ( x < 1 ) {
      return( (-2.0 + 1.5*x)*x );
    } 
    return( -0.5 * (2.0-x) * (2.0-x) );
  }

  /* ici x < 0
   */
  if ( x <= -2.0 ) return( 0.0 );
  if ( x > -1 ) {  
    return( (-2.0 - 1.5*x)*x );
  }
  return( 0.5 * (2.0+x) * (2.0+x) );
}




static double CubicBspline_SecndDeriv ( double x ) 
{
  if ( x > 0.0 ) {
    if ( x >=  2.0 ) return( 0.0 );
    if ( x < 1 ) {
      return( -2.0 + 3.0*x );
    }
    return( 2.0 - x );
  }

  /* ici x < 0
   */
  if ( x <= -2.0 ) return( 0.0 );
  if ( x > -1 ) {  
    return( -2.0 - 3.0*x );
  }
  return( 2.0 + x );
}













/* Interpolation cubique spline d'un signal S, en supposant 
   les points d'echantillonnage regulierement espaces. 

   Les 2 premiers arguments sont des pointeurs sur tableaux 
   de float SUPPOSES DEJA ALLOUES de taille N.
        - s: valeurs d'echantillon
        - c: coefficients B-spline a calucler

   L'algo est recursif, donc tres rapide par rapport a une 
   methode matricielle. Il est decrit dans:
   M. Unser, "Splines : a perfect fit for signal/image processing ", 
   IEEE Signal Processing Magazine, Nov. 1999, in press. 

   A. Roche
*/

static void CubicSpline_Transform ( double * s, double * c, int N )
{

  int k;
  double cp, cm;
  double z1_k;
  double * bufs, * bufc;
  
  /* Constants */
  const double  z1 = -0.26794919243112270648; /* -2 + sqrt(3) */
  const double cz1 =  0.28867513459481288226; /* z1/(z1^2-1) */


  /*  (void)memset( (void*) c, 0, N * sizeof( double ) ); */
  
  /* Initial value for the causal recursion.
     We use a mirror symmetric boundary condition for the discrete signal,
     yielding:
     
     cp(0) = (1/2-z1^(2N-2)) \sum_{k=0}^{2N-3} s(k) z1^k s(k),
     
     where we set: s(N)=s(N-2), s(N+1)=s(N-3), ..., s(2N-3)=s(1).
  */

  bufs = s;
  cp = * bufs;
  z1_k = 1;

  for ( k = 1; k < N; k++ ) {
    
    z1_k = z1 * z1_k;   /* == z1^k */
    
    bufs ++;            /* pointe sur s[k] */
    cp += (*bufs) * z1_k;
  }
  
  /* Quand on arrive ici, z1_k = z1^(N-1) */
  for ( k = 2; k < N; k++ ) {
    
    z1_k = z1 * z1_k;  
    
    bufs --;
    cp += (* bufs) * z1_k;
  }
  
  /* Quand on arrive ici: z1_k = z1^(2N-3) */
  z1_k = z1 * z1_k;
  cp = cp / ( 1 - z1_k );


  /* Stockage du premier coeff causal */
  bufc = c;
  (*bufc) = cp;

  /* Do the causal recursion */
  bufs = s;
  for ( k = 1; k < N; k ++ ) {
    bufs ++;
    cp = (* bufs) + z1 * cp;
    bufc ++;
    (*bufc) = cp;
  }

  /* Initial value for the anticausal recursion. */
  cm = cz1 * ( 2.0 * cp - (*bufs) );
  (*bufc) = 6.0*cm;
 

  /* Do the anti causal recursion. */
  for ( k = (N-2); k >= 0; k=k-1 ) {
    bufc --;     /* bufc pointe sur l'indice k */
    cm = z1 * ( cm - (*bufc));
    (*bufc) = 6.0*cm;
  }

}















/* calcul des coefficients de la spline cubique
   
   - cas 3D : theDim[2] > 1
     les coefficients sont un buffer 3D
     de taille (theDim[0]+2) * (theDim[1]+2) * (theDim[2]+2)

   - cas 2D : theDim[2] <= 1 (donc theDim[2]==1)
     les coefficients sont un buffer 3D
     de taille (theDim[0]+2) * (theDim[1]+2) * theDim[2]

   Aux bords, on duplique les valeurs calculees.
*/

typeCSplineCoefficients *ComputeCSplineCoefficients( void *theBuf,
                                                     bufferType theType,
                                                     int *theDim )
{
  char *proc = "ComputeCSplineCoefficients";
  typeCSplineCoefficients *theCSpline = NULL;
  
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];

  int dx2  = theDim[0]+2;
  int dxy2  = (theDim[0]+2)*(theDim[1]+2);
  
  int x, y, z;
  int ioffset;
  double *theLine = NULL;
  double *resLine = NULL;
  float *resBuf;

  int onlyCopy = 0;


  theCSpline = (typeCSplineCoefficients *)malloc( sizeof(typeCSplineCoefficients) );
  if ( theCSpline == (typeCSplineCoefficients *)NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate cubic spline coefficients structure\n",
               proc );
    return( NULL );
  }


  theCSpline->theDim[0] = theDim[0] + 2;
  theCSpline->theDim[1] = theDim[1] + 2;
  if ( theDim[2] > 1 ) theCSpline->theDim[2] = theDim[2] + 2;
  else                 theCSpline->theDim[2] = 1;




  /* on alloue un buffer de taille (dimx+2)*(dimy+2)*(dimz+2) 
     dans le cas 3D, ou (dimx+2)*(dimy+2) dans le cas 2D
  */

  theCSpline->theCoeff = (float*)malloc( theCSpline->theDim[0] * 
                                         theCSpline->theDim[1] *
                                         theCSpline->theDim[2] * sizeof( float ) );
  if ( theCSpline->theCoeff == (float*)NULL ) {
    free( theCSpline );
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate cubic spline coefficients buffer\n",
               proc );
    return( NULL );
  }
  

  /* pour faire les calculs on a besoin de deux lignes
     dont la longueur est la plus grande dimension
  */
  x = dimx;
  if ( x < dimy ) x = dimy;
  if ( x < dimz ) x = dimz;
  theLine = (double*)malloc( 2*x*sizeof( double ) );
  if ( theLine == (double*)NULL ) {
    FreeTypeCSplineCoefficients( &theCSpline );
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n",
               proc );
    return( NULL );
  }
  resLine = theLine;
  resLine += x;



    




  /* Convolution selon X,
     on recupere la ligne [z][y][x=0..dimx-1] dans l'image d'entree
     et on la place en [z+1][y+1][x=1..dimx] dans l'image des coefficients
  */


  /* on saute le premier plan de l'image des coefficients dans le cas 3D
   */
  resBuf = theCSpline->theCoeff;
  if ( theDim[2] > 1 ) resBuf += dxy2;

  for ( ioffset = 0, z=0; z<dimz; z++ ) {
    
    /* on avance jusqu'au debut de la premiere ligne pour copier
       dans l'image des coefficients
    */
    resBuf += dx2 + 1;



    for ( y=0; y<dimy; y++, ioffset += dimx ) {

      /* lecture de la ligne d'entree
       */

      switch ( theType ) {
      default :
        free( theLine );
        FreeTypeCSplineCoefficients( &theCSpline );
        if ( _verbose_ )
          fprintf( stderr, "%s: such image type not handled yet\n", proc );
        return( NULL );
      case UCHAR :
        {
          u8 *buf = (u8*)theBuf;
          buf += ioffset;
          for ( x=0; x<dimx; x++ ) theLine[x] = (double)buf[x];
        }
        break;
      case SCHAR :
        {
          s8 *buf = (s8*)theBuf;
          buf += ioffset;
          for ( x=0; x<dimx; x++ ) theLine[x] = (double)buf[x];
        }
        break;
      case USHORT :
        {
          u16 *buf = (u16*)theBuf;
          buf += ioffset;
          for ( x=0; x<dimx; x++ ) theLine[x] = (double)buf[x];
        }
        break;
      case SSHORT :
        {
          s16 *buf = (s16*)theBuf;
          buf += ioffset;
          for ( x=0; x<dimx; x++ ) theLine[x] = (double)buf[x];
        }
        break;
      case FLOAT :
        {
          r32 *buf = (r32*)theBuf;
          buf += ioffset;
          for ( x=0; x<dimx; x++ ) theLine[x] = (double)buf[x];
        }
        break;
      }
      
      
      /* la il faut calculer resLine
         ...
         memcpy( resLine, theLine, dimx*sizeof( double ) );
         ...
      */
      if ( onlyCopy ) {
        memcpy( resLine, theLine, dimx*sizeof( double ) );
      } else {
        CubicSpline_Transform( theLine, resLine, dimx );
      }


    
      /* on copie le resultat
         on avance de (dimx+2) pour se mettre au debut de la ligne suivante
       */
      for ( x=0; x<dimx; x++ ) resBuf[x] = (float)resLine[x];
      resBuf += dx2;


    } /* for ( y=0 ... ) */

    /* on se remet au debut du plan suivant
     */
    resBuf += (dimx+1);

  } /* Convolution selon X */





  




  /* Convolution selon Y,
     on recupere la ligne [z][y=1..dimy][x] dans l'image des coefficients
     et on la reecrit sur place
  */


  /* on saute le premier plan de l'image des coefficients dans le cas 3D
   */
  resBuf = theCSpline->theCoeff;
  if ( theDim[2] > 1 ) resBuf += dxy2;

  
  for ( z=0; z<dimz; z++ ) {
    
    /* on avance jusqu'au debut de la premiere ligne pour copier
       dans l'image des coefficients
    */
    resBuf += dx2 + 1;
    
    for ( x=0; x<dimx; x++, resBuf++ ) {
      
      for ( y=0; y<dimy; y++ ) theLine[y] = resBuf[y*dx2];


      /* la il faut calculer resLine
         ...
         memcpy( resLine, theLine, dimy*sizeof( double ) );
         ...
       */
      if ( onlyCopy ) {
        memcpy( resLine, theLine, dimy*sizeof( double ) );
      } else {
        CubicSpline_Transform( theLine, resLine, dimy );
      }
      
      

      for ( y=0; y<dimy; y++ ) resBuf[y*dx2] = (float)resLine[y];


      

    }
    
    /* on se remet au debut du plan suivant
     */
    resBuf += dimy*dx2+1;

  } /* Convolution selon Y */



  

  /* Convolution selon Z (dans le cas 3D),
     on recupere la ligne [z=1..dimz][y][x] dans l'image des coefficients
     et on la reecrit sur place
  */
  if ( theDim[2] > 1 ) {
    
    /* on saute le premier plan de l'image des coefficients
       on avance jusqu'a la premiere ligne
     */
    resBuf = theCSpline->theCoeff;
    resBuf += dxy2;
    resBuf += dx2 + 1;

    for ( y=0; y<dimy; y++, resBuf+=2 ) {
      for ( x=0; x<dimx; x++, resBuf++ ) {

        for ( z=0; z<dimz; z++ ) theLine[z] = resBuf[z*dxy2];


        /* la il faut calculer resLine
           ...
           memcpy( resLine, theLine, dimz*sizeof( double ) );
           ...
        */
        if ( onlyCopy ) {
          memcpy( resLine, theLine, dimz*sizeof( double ) );
        } else {
          CubicSpline_Transform( theLine, resLine, dimz );
        }

        for ( z=0; z<dimz; z++ ) resBuf[z*dxy2] = (float)resLine[z];
      }
    }
  }  /* Convolution selon Z */



  free( theLine );




  /* on fait l'effet miroir => on duplique les bords
     [0] = [2] et [dimx-1] = [dimx-3]
     1. on fait les bords X et Y dans les plans calcules
     2. on ajoute des plans selon Z dans le cas 3D
   */
  resBuf = theCSpline->theCoeff;
  if ( theDim[2] > 1 ) resBuf += dxy2;
  
  for ( z=0; z<dimz; z++, resBuf+=dxy2 ) {
    for ( y=1; y<=dimy; y++ ) {
      resBuf[y*dx2]         = resBuf[y*dx2+2];
      resBuf[y*dx2 + dx2-1] = resBuf[y*dx2 + dx2-3];
    }
    memcpy( &(resBuf[0]), &(resBuf[2*dx2]), dx2*sizeof(float) );
    memcpy( &(resBuf[(dimy+1)*dx2]), &(resBuf[(dimy-1)*dx2]), dx2*sizeof(float) );
  }

  if ( theDim[2] > 1 ) {
    resBuf = theCSpline->theCoeff;
    memcpy( &(resBuf[0]), &(resBuf[2*dxy2]), dxy2*sizeof(float) );
    memcpy( &(resBuf[(dimz+1)*dxy2]), &(resBuf[(dimz-1)*dxy2]), dxy2*sizeof(float) );
  }

  return( theCSpline );
}















void FreeTypeCSplineCoefficients( typeCSplineCoefficients **t )
{
  free( (*t)->theCoeff );
  free( *t );
  *t = NULL;
}



void InitTypeCSplineCoefficients( typeCSplineCoefficients *t )
{
  t->theDim[0] = 0;
  t->theDim[1] = 0;
  t->theDim[2] = 0;
  t->theCoeff = NULL;
}

















int Reech3DCSpline4x4WithCoefficients( typeCSplineCoefficients *theCoeff,
                       r32* resBuf,  /* result buffer */
                       int *resDim,  /* dimensions of this buffer */
                       double *mat,
                       int slice, int *derivative )
{
  char *proc = "Reech3DCSpline4x4";
  register int i, j, k=slice, ix, iy, iz;
  register double x, y, z;
  register double dx, dy, dz;
  register double res;
  
  int rdimx=resDim[0];
  int rdimy=resDim[1];

  register r32* rbuf = resBuf;
  register float *tbuf;
  float *bufCoeff = theCoeff->theCoeff;

  int tdimx  = theCoeff->theDim[0];
  int tdimxy = theCoeff->theDim[0] * theCoeff->theDim[1];

  int tdimx3 = theCoeff->theDim[0]-3;
  int tdimy3 = theCoeff->theDim[1]-3;
  int tdimz3 = theCoeff->theDim[2]-3;
 
  register double ddimx = (double)theCoeff->theDim[0]-2.0-0.5;
  register double ddimy = (double)theCoeff->theDim[1]-2.0-0.5;
  register double ddimz = (double)theCoeff->theDim[2]-2.0-0.5;

  double cx[4], cy[4], cz[4], cyz;
  int offset[64];

  int l, m, n, o;

  double (*bsplinx)(double);
  double (*bspliny)(double);
  double (*bsplinz)(double);


  if ( derivative == (int*) NULL ) {
    bsplinx = & CubicBspline;
    bspliny = & CubicBspline;
    bsplinz = & CubicBspline;
  } else {
    switch ( derivative[0] ) {
    default :
      if ( _verbose_ ) 
        fprintf( stderr, "%s: invalid derivation order in x\n", proc );
      return( -1 );
    case 0 : bsplinx = & CubicBspline;            break;
    case 1 : bsplinx = & CubicBspline_FirstDeriv; break;
    case 2 : bsplinx = & CubicBspline_SecndDeriv; break;
    }
    
    switch ( derivative[1] ) {
    default :
      if ( _verbose_ ) 
        fprintf( stderr, "%s: invalid derivation order in y\n", proc );
      return( -1 );
    case 0 : bspliny = & CubicBspline;            break;
    case 1 : bspliny = & CubicBspline_FirstDeriv; break;
    case 2 : bspliny = & CubicBspline_SecndDeriv; break;
    }
    
    switch ( derivative[2] ) {
    default :
      if ( _verbose_ ) 
        fprintf( stderr, "%s: invalid derivation order in z\n", proc );
      return( -1 );
    case 0 : bsplinz = & CubicBspline;            break;
    case 1 : bsplinz = & CubicBspline_FirstDeriv; break;
    case 2 : bsplinz = & CubicBspline_SecndDeriv; break;
    }
  }
  
  

  for ( o = 0, l = 0; l < 4; l ++ )
  for ( m = 0; m < 4; m ++ )
  for ( n = 0; n < 4; n ++, o ++ ) {
    offset[o] = l * tdimxy + m * tdimx + n;
  }
  
  

  for ( j = 0; j < rdimy; j ++ )
  for ( i = 0; i < rdimx; i ++, rbuf ++ ) {
    /* computation of the corresponding point after deformation 
       
       (x,y,x) -> (ix=(int)x,iy=(int)x,iz=(int)x)
       les coefficients de la spline a prendre en compte
       sont [1+ix+{-1,0,1,2}, 1+iy+{-1,0,1,2}, 1+iz+{-1,0,1,2}]
       
       On ajoute 1 aux coordonnees, car on a ajoute
       une bordure aux coefficients de la spline.
     */
    
    x = mat[0] * i +  mat[1] * j + mat[2] * k + mat[3];
    /*
    if ((x < 0) || ( x > tdimx3)) { *rbuf = 0; continue; }
    */
    if ((x < -0.5) || ( x > ddimx)) { *rbuf = 0; continue; }

    y = mat[4] * i +  mat[5] * j + mat[6] * k + mat[7];
    /*
    if ((y < 0) || ( y > tdimy3)) { *rbuf = 0; continue; }
    */
    if ((y < -0.5) || ( y > ddimy)) { *rbuf = 0; continue; }

    z = mat[8] * i +  mat[9] * j + mat[10] * k + mat[11];
    /*
    if ((z < 0) || ( z > tdimz3)) { *rbuf = 0; continue; }
    */
    if ((z < -0.5) || ( z > ddimz)) { *rbuf = 0; continue; }
    
    
    /* ici on a 0 <   x   < theCoeff->theDim[0]-3
                0 <= ix   < theCoeff->theDim[0]-3
                1 <= ix+1 < theCoeff->theDim[0]-2
                0 <= ix+1-1 && ix+1+2 < theCoeff->theDim[0]

       maintenant on a -0.5 <         x   <        theCoeff->theDim[0]-2.5
                          0 <= ix   <= theCoeff->theDim[0]-3
                          1 <= ix+1 <= theCoeff->theDim[0]-2

       les extrema pour les coefficients verifient
                          0 <= ix+1-1 <= theCoeff->theDim[0]-3
                          3 <= ix+1+2 <= theCoeff->theDim[0]
       dans ce dernier cas, on suppose que ca va etre 0 ...
    */
    ix = (int)x;    dx = x-ix;
    iy = (int)y;    dy = y-iy;
    iz = (int)z;    dz = z-iz;



    if ( x < 0.0 ) { x = 0.0; ix = 0; dx = 0.0; }
    if ( y < 0.0 ) { y = 0.0; iy = 0; dy = 0.0; }
    if ( z < 0.0 ) { z = 0.0; iz = 0; dz = 0.0; }

    /* Pre-calcul des valeurs de bsplines pour chaque dimension:
       
       (*bsplinx)(dx-1), (*bsplinx)(dx), (*bsplinx)(dx+1), (*bsplinx)(dx+2)  
       (*bspliny)(dy-1), (*bspliny)(dy), (*bspliny)(dy+1), (*bspliny)(dy+2)  
       (*bsplinz)(dz-1), (*bsplinz)(dz), (*bsplinz)(dz+1), (*bsplinz)(dz+2) 
       
       A. Roche
       
       Dans l'image des coefficients,
       (*bsplinx)(dx-1) correspond a ix, 
       (*bsplinx)(dy-1) correspond a iy, 
       (*bsplinx)(dz-1) correspond a iz, 
       
    */
    for ( l = 0; l < 4; l ++ ) {
      cx[l] = (*bsplinx)( dx - (double)(l-1) );
      cy[l] = (*bspliny)( dy - (double)(l-1) );
      cz[l] = (*bsplinz)( dz - (double)(l-1) );
    }

    tbuf = bufCoeff;
    tbuf += iz * tdimxy + iy * tdimx + ix;



    /* 
     * are we on the border or not ? 
     */
    if ( (ix < tdimx3) && (iy < tdimy3) && (iz < tdimz3) ) {
      

      for ( res = 0.0, o = 0, l = 0; l < 4; l ++ )
      for ( m = 0; m < 4; m ++ ) {
        cyz = cz[l]*cy[m];
        for ( n = 0; n < 4; n ++, o ++ ) {
          res += cx[n]*cyz*tbuf[ offset[ o ] ];
        }
      }
      *rbuf = (float)res;
      continue;
    } 

    /* 
     * here, we are sure we are on some border
     * if ix == tdimx3  => ix = theCoeff->theDim[0]-3
     */
    
    *rbuf = 0.0;

    if ( ix == tdimx3 ) {
      if ( iy == tdimy3 ) {
        if ( iz == tdimz3 ) {

          for ( res = 0.0, o = 0, l = 0; l < 3; l++, o+=4 )
          for ( m = 0; m < 3; m++, o++ ) {
            cyz = cz[l]*cy[m];
            for ( n = 0; n < 3; n ++, o ++ )
              res += cx[n]*cyz*tbuf[ offset[ o ] ];
          }
          *rbuf = (float)res;
          continue;
        }
        for ( res = 0.0, o = 0, l = 0; l < 4; l++, o+=4 )
        for ( m = 0; m < 3; m++, o++ ) {
          cyz = cz[l]*cy[m];
          for ( n = 0; n < 3; n ++, o ++ )
            res += cx[n]*cyz*tbuf[ offset[ o ] ];
        }
        *rbuf = (float)res;
        continue;
      }

      if ( iz == tdimz3 ) {
        for ( res = 0.0, o = 0, l = 0; l < 3; l++ )
        for ( m = 0; m < 4; m++, o++ ) {
          cyz = cz[l]*cy[m];
          for ( n = 0; n < 3; n ++, o ++ )
            res += cx[n]*cyz*tbuf[ offset[ o ] ];
        }
        *rbuf = (float)res;
        continue;
      }
      for ( res = 0.0, o = 0, l = 0; l < 4; l++ )
      for ( m = 0; m < 4; m++, o++ ) {
        cyz = cz[l]*cy[m];
        for ( n = 0; n < 3; n ++, o ++ )
          res += cx[n]*cyz*tbuf[ offset[ o ] ];
      }
      *rbuf = (float)res;
      continue;
      
    }

    if ( iy == tdimy3 ) {

      if ( iz == tdimz3 ) {
        for ( res = 0.0, o = 0, l = 0; l < 3; l++, o+=4 )
        for ( m = 0; m < 3; m++ ) {
          cyz = cz[l]*cy[m];
          for ( n = 0; n < 4; n ++, o ++ )
            res += cx[n]*cyz*tbuf[ offset[ o ] ];
        }
        *rbuf = (float)res;
        continue;
      }
      for ( res = 0.0, o = 0, l = 0; l < 4; l++, o+=4 )
      for ( m = 0; m < 3; m++ ) {
        cyz = cz[l]*cy[m];
        for ( n = 0; n < 4; n ++, o ++ )
          res += cx[n]*cyz*tbuf[ offset[ o ] ];
      }
      *rbuf = (float)res;
      continue;
      
    }

    if ( iz == tdimz3 ) {
      for ( res = 0.0, o = 0, l = 0; l < 3; l++ )
      for ( m = 0; m < 4; m++ ) {
        cyz = cz[l]*cy[m];
        for ( n = 0; n < 4; n ++, o ++ )
          res += cx[n]*cyz*tbuf[ offset[ o ] ];
      }
      *rbuf = (float)res;
      continue;
    }
    for ( res = 0.0, o = 0, l = 0; l < 4; l++ )
    for ( m = 0; m < 4; m++ ) {
      cyz = cz[l]*cy[m];
      for ( n = 0; n < 4; n ++, o ++ )
        res += cx[n]*cyz*tbuf[ offset[ o ] ];
    }
    *rbuf = (float)res;
    continue;
    
  }
  
  return( 1 );
}



















int Reech2DCSpline4x4WithCoefficients( typeCSplineCoefficients *theCoeff,
                       r32* resBuf,  /* result buffer */
                       int *resDim,  /* dimensions of this buffer */
                       double *mat,
                       int *derivative )
{
  char *proc = "Reech2DCSpline4x4";
  register int i, j, ix, iy;
  register double x, y;
  register double dx, dy;
  register double res;
  
  int rdimx=resDim[0];
  int rdimy=resDim[1];

  register r32* rbuf = resBuf;
  register float *tbuf;
  float *bufCoeff = theCoeff->theCoeff;

  int tdimx  = theCoeff->theDim[0];

  int tdimx3 = theCoeff->theDim[0]-3;
  int tdimy3 = theCoeff->theDim[1]-3;
 
  register double ddimx = (double)theCoeff->theDim[0]-2.0-0.5;
  register double ddimy = (double)theCoeff->theDim[1]-2.0-0.5;

  double cx[4], cy[4];
  int offset[16];

  int l, m, n, o;

  double (*bsplinx)(double);
  double (*bspliny)(double);


  if ( derivative == (int*) NULL ) {
    bsplinx = & CubicBspline;
    bspliny = & CubicBspline;
  } else {
    switch ( derivative[0] ) {
    default :
      if ( _verbose_ ) 
        fprintf( stderr, "%s: invalid derivation order in x\n", proc );
      return( -1 );
    case 0 : bsplinx = & CubicBspline;            break;
    case 1 : bsplinx = & CubicBspline_FirstDeriv; break;
    case 2 : bsplinx = & CubicBspline_SecndDeriv; break;
    }
    
    switch ( derivative[1] ) {
    default :
      if ( _verbose_ ) 
        fprintf( stderr, "%s: invalid derivation order in y\n", proc );
      return( -1 );
    case 0 : bspliny = & CubicBspline;            break;
    case 1 : bspliny = & CubicBspline_FirstDeriv; break;
    case 2 : bspliny = & CubicBspline_SecndDeriv; break;
    }
    
  }
  
  

  for ( o = 0, m = 0; m < 4; m ++ )
  for ( n = 0; n < 4; n ++, o ++ ) {
    offset[o] = m * tdimx + n;
  }
  
  

  for ( j = 0; j < rdimy; j ++ )
  for ( i = 0; i < rdimx; i ++, rbuf ++ ) {
    /* computation of the corresponding point after deformation 
       
       (x,y,x) -> (ix=(int)x,iy=(int)x,iz=(int)x)
       les coefficients de la spline a prendre en compte
       sont [1+ix+{-1,0,1,2}, 1+iy+{-1,0,1,2}, 1+iz+{-1,0,1,2}]
       
       On ajoute 1 aux coordonnees, car on a ajoute
       une bordure aux coefficients de la spline.
     */
    x = mat[0] * i +  mat[1] * j              + mat[3];
    /*
    if ((x < 0) || ( x > tdimx3)) { *rbuf = 0; continue; }
    */
    if ((x < -0.5) || ( x > ddimx)) { *rbuf = 0; continue; }

    y = mat[4] * i +  mat[5] * j              + mat[7];
    /*
    if ((y < 0) || ( y > tdimy3)) { *rbuf = 0; continue; }
    */
    if ((y < -0.5) || ( y > ddimy)) { *rbuf = 0; continue; }
    
    /* ici on a 0 <   x   < theCoeff->theDim[0]-3
                0 <= ix   < theCoeff->theDim[0]-3
                1 <= ix+1 < theCoeff->theDim[0]-2
                0 <= ix+1-1 && ix+1+2 < theCoeff->theDim[0]
       maintenant on a -0.5 <         x   <        theCoeff->theDim[0]-2.5
                          0 <= ix   <= theCoeff->theDim[0]-3
                          1 <= ix+1 <= theCoeff->theDim[0]-2

       les extrema pour les coefficients verifient
                          0 <= ix+1-1 <= theCoeff->theDim[0]-3
                          3 <= ix+1+2 <= theCoeff->theDim[0]
       dans ce dernier cas, on suppose que ca va etre 0 ...
     */
    ix = (int)x;    dx = x-ix;
    iy = (int)y;    dy = y-iy;



    if ( x < 0.0 ) { x = 0.0; ix = 0; dx = 0.0; }
    if ( y < 0.0 ) { y = 0.0; iy = 0; dy = 0.0; }

    
    /* Pre-calcul des valeurs de bsplines pour chaque dimension:
       
       (*bsplinx)(dx-1), (*bsplinx)(dx), (*bsplinx)(dx+1), (*bsplinx)(dx+2)  
       (*bspliny)(dy-1), (*bspliny)(dy), (*bspliny)(dy+1), (*bspliny)(dy+2)  
       
       A. Roche
       
       Dans l'image des coefficients,
       (*bsplinx)(dx-1) correspond a ix, 
       (*bsplinx)(dy-1) correspond a iy, 

    */
    for ( l = 0; l < 4; l ++ ) {
      cx[l] = (*bsplinx)( dx - (double)(l-1) );
      cy[l] = (*bspliny)( dy - (double)(l-1) );
    }



    tbuf = bufCoeff;
    tbuf += iy * tdimx + ix;
    
    /* 
     * are we on the border or not ? 
     */
    if ( (ix < tdimx3) && (iy < tdimy3) ) {
      

      for ( res = 0.0, o = 0, m = 0; m < 4; m ++ )
      for ( n = 0; n < 4; n ++, o ++ )
        res += cx[n]*cy[m]*tbuf[ offset[ o ] ];
      *rbuf = (float)res;
      continue;
    }

    /* 
     * here, we are sure we are on some border
     * if ix == tdimx3  => ix = theCoeff->theDim[0]-3
     */
    
    *rbuf = 0.0;
    
    if ( ix == tdimx3 ) {
      if ( iy == tdimy3 ) {

        for ( res = 0.0, o = 0, m = 0; m < 3; m ++, o++ )
        for ( n = 0; n < 3; n ++, o ++ )
          res += cx[n]*cy[m]*tbuf[ offset[ o ] ];
        *rbuf = (float)res;
        continue;

      }

      for ( res = 0.0, o = 0, m = 0; m < 4; m ++, o++ )
      for ( n = 0; n < 3; n ++, o ++ )
        res += cx[n]*cy[m]*tbuf[ offset[ o ] ];
      *rbuf = (float)res;
      continue;

    }

    if ( iy == tdimy3 ) {
      for ( res = 0.0, o = 0, m = 0; m < 3; m ++ )
      for ( n = 0; n < 4; n ++, o ++ )
        res += cx[n]*cy[m]*tbuf[ offset[ o ] ];
      *rbuf = (float)res;
      continue;
    }
    
    for ( res = 0.0, o = 0, m = 0; m < 4; m ++ )
    for ( n = 0; n < 4; n ++, o ++ )
        res += cx[n]*cy[m]*tbuf[ offset[ o ] ];
    *rbuf = (float)res;
    continue;
   

  }
  
  return( 1 );
}























int ReechCSpline4x4( void* theBuf, bufferType theType, int *theDim,
                              void* resBuf, bufferType resType, int *resDim,
                              double *mat,
                              int *derivative )
{
  char *proc = "Reech3DCSpline4x4";
  typeCSplineCoefficients *c;
  r32 *resSlice = NULL;
  int z;

  c = ComputeCSplineCoefficients( theBuf, theType, theDim );
  if ( c == NULL ) {
    if( _verbose_ )
      fprintf( stderr, "%s: unable to compute cubic spline coefficients.\n", proc );
    return( -1 );
  }
  


  if ( resType != FLOAT ) {
    resSlice = (r32*)malloc( resDim[0]*resDim[1]*sizeof( r32 ) );
    if ( resSlice == NULL ) {
      FreeTypeCSplineCoefficients( &c );
      if( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary slice\n", proc );
      return( -1 );
    }
  }
  
  
  for ( z = 0; z < resDim[2]; z ++ ) {

    if ( resType == FLOAT ) {
      resSlice  = resBuf;
      resSlice += z*resDim[0]*resDim[1];
    }

    if ( theDim[2] > 1 ) {
      if ( Reech3DCSpline4x4WithCoefficients( c, resSlice, resDim, mat,
                                            z, derivative ) != 1 ) {
        FreeTypeCSplineCoefficients( &c );
        if ( resType != FLOAT ) free( resSlice );
        if( _verbose_ )
          fprintf( stderr, "%s: unable to compute 3D slice #%d\n", proc, z );
      }
    } else {
      if ( Reech2DCSpline4x4WithCoefficients( c, resSlice, resDim, mat,
                                              derivative ) != 1 ) {
        FreeTypeCSplineCoefficients( &c );
        if ( resType != FLOAT ) free( resSlice );
        if( _verbose_ )
          fprintf( stderr, "%s: unable to compute 2D slice #%d\n", proc, z );
      }
    }

    if ( resType != FLOAT ) {
      switch( resType ) {
      default :
        FreeTypeCSplineCoefficients( &c );
        if ( resType != FLOAT ) free( resSlice );
        if ( _verbose_ )
          fprintf( stderr, "%s: such output type not handled in switch\n", proc );
        return( -1 );
      case UCHAR :
        if ( ConvertBuffer( resSlice, FLOAT, &((u8*)resBuf)[z*resDim[0]*resDim[1]],
                            resType, resDim[0]*resDim[1] ) != 1 ) {
          if ( resType != FLOAT ) free( resSlice );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
        break;
      case SCHAR :
        if ( ConvertBuffer( resSlice, FLOAT, &((s8*)resBuf)[z*resDim[0]*resDim[1]],
                            resType, resDim[0]*resDim[1] ) != 1 ) {
          if ( resType != FLOAT ) free( resSlice );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
        break;
      case USHORT :
        if ( ConvertBuffer( resSlice, FLOAT, &((u16*)resBuf)[z*resDim[0]*resDim[1]],
                            resType, resDim[0]*resDim[1] ) != 1 ) {
          if ( resType != FLOAT ) free( resSlice );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
        break;
      case SSHORT :
        if ( ConvertBuffer( resSlice, FLOAT, &((s16*)resBuf)[z*resDim[0]*resDim[1]],
                            resType, resDim[0]*resDim[1] ) != 1 ) {
          if ( resType != FLOAT ) free( resSlice );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to convert such image type\n", proc );
          return( -1 );
        }
        break;
      }
    }
  }

  FreeTypeCSplineCoefficients( &c );
  if ( resType != FLOAT ) free( resSlice );

  return( 1 );
}
