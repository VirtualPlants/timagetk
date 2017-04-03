/*************************************************************************
 * reech4x4-coeff.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Mar 23 fév 2016 11:56:25 CET
 *
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <typedefs.h>
#include <chunks.h>

#include <reech4x4-coeff.h>




static int _verbose_ = 1;




void setVerboseInReech4x4Coeff( int v )
{
  _verbose_ = v;
}

void incrementVerboseInReech4x4Coeff(  )
{
  _verbose_ ++;
}

void decrementVerboseInReech4x4Coeff(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





typedef struct {
  int *theDim; /* dimensions of this buffer */
  float* resBuf; /* result buffer */
  int *resDim;  /* dimensions of this buffer */
  double* mat;   /* transformation matrix */
  int index;
} _LinearCoefficientParam;





int comparedouble ( const void * a, const void * b )
{
    double *da = (double*)a;
    double *db = (double*)b;

    if ( (*da) > (*db) ) return( -1 );
    if ( (*da) < (*db) ) return( 1 );
    return( 0 );
}





/* Resampling procedure / trilinear coefficient computation

   Work for 3D images, not for vectorial ones.
   
   (double* mat) is the matrix which permits to get
   from resBuf into theBuf. 
   If one only have the matrix from theBuf into resBuf,
   it must be inverted first.

   Soit x le point transforme et ix=(int)x;
   nous allons distinguer les cas suivants :
    x < -0.5               => resultat = 0
    -0.5 <= x < 0.0        => ix=0, on n'interpole pas selon X
    0.0 < x && ix < dimx-1 => on interpole selon X
    x < dimx-0.5           => ix=dimx-1, on n'interpole pas selon X
    x >= dimx-0.5          => resultat = 0

*/

static void *_Coeff3DTriLin4x4 ( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _LinearCoefficientParam *p = (_LinearCoefficientParam *)parameter;
  
  int *theDim = p->theDim;
  void* resBuf = p->resBuf;
  int *resDim = p->resDim;
  double* mat = p->mat; 
  int index = p->index;

  double v[8];
  int n;

  size_t i, j, k;
  register int ix, iy, iz;
  register double x, y, z, dx, dy, dz, dxdy,dxdz,dydz,dxdydz;
  size_t rdimx=resDim[0], rdimy=resDim[1];
  int tdimx=theDim[0], tdimy=theDim[1], tdimz=theDim[2];

  register int t1dimx=tdimx-1, t1dimy=tdimy-1, t1dimz=tdimz-1;
  register double ddimx = (double)tdimx-0.5, ddimy = (double)tdimy-0.5;
  register double ddimz = (double)tdimz-0.5;
  register float *rbuf = (float*)resBuf;

  size_t ifirst, jfirst, kfirst;
  size_t ilast, jlast, klast;
  size_t iend, jend;

  k = kfirst = first / (rdimx*rdimy);
  j = jfirst = (first - kfirst*(rdimx*rdimy)) / rdimx;
  i = ifirst = (first - kfirst*(rdimx*rdimy) - jfirst*rdimx);

  klast = last / (rdimx*rdimy);
  jlast = (last - klast*(rdimx*rdimy)) / rdimx;
  ilast = (last - klast*(rdimx*rdimy) - jlast*rdimx);

  rbuf += first;

  for ( ; k<=klast; k++, j=0 ) {
    if ( _verbose_ > 1 )
      fprintf( stderr, "Processing slice %lu\r", k );
    jend = (k==klast) ? jlast+1 : rdimy;
    for ( ; j<jend; j++, i=0 ) {
      iend = (j==jlast && k==klast) ? ilast+1 : rdimx;
      for ( ; i<iend; i++, rbuf++ ) {
        /* computation of the corresponding point coordinates in theBuf */
        x = mat[0] * i +  mat[1] * j + mat[2] * k + mat[3];
        if ((x <= -0.5) || ( x >= ddimx)) { *rbuf = 0; continue; }
        y = mat[4] * i +  mat[5] * j + mat[6] * k + mat[7];
        if ((y <= -0.5) || ( y >= ddimy)) { *rbuf = 0; continue; }
        z = mat[8] * i +  mat[9] * j + mat[10] * k + mat[11];
        if ((z <= -0.5) || ( z >= ddimz)) { *rbuf = 0; continue; }


        /* here, the point lies on the borders or completely inside
           the image */
        ix = (int)x;
        iy = (int)y;
        iz = (int)z;

        for ( n=0; n<8; n++ ) v[n] = 0;

        /* are we on the border or not ? */
        if ( (x > 0.0) && (ix < t1dimx) &&
             (y > 0.0) && (iy < t1dimy) &&
             (z > 0.0) && (iz < t1dimz) ) {
          /* the corresponding point is in the box defined
             by (ix[+1],iy[+1],iz[+1]) */
          dx = x - ix;
          dy = y - iy;
          dz = z - iz;
          dxdy = dx*dy;
          dxdz = dx*dz;
          dydz = dy*dz;
          dxdydz = dxdy*dz;

          /* we have
             v[7]=dxdydz;            coefficient of tbuf(ix+1,iy+1,iz+1)
             v[6]=dxdz-dxdydz;       coefficient of tbuf(ix+1,iy,  iz+1)
             v[5]=dxdy-dxdydz;       coefficient of tbuf(ix+1,iy+1,iz  )
             v[4]=dx-dxdy-v[6];      coefficient of tbuf(ix+1,iy  ,iz  )
             v[3]=dydz-dxdydz;       coefficient of tbuf(ix  ,iy+1,iz+1)
             v[2]=dz-dydz-v[6];      coefficient of tbuf(ix  ,iy  ,iz+1)
             v[1]=dy-dydz-v[5];      coefficient of tbuf(ix  ,iy+1,iz  )
             v[0]=1-dy-dz+dydz-v[4]; coefficient of tbuf(ix  ,iy  ,iz  )
          */
          v[7]=dxdydz;
          v[6]=dxdz-dxdydz;
          v[5]=dxdy-dxdydz;
          v[4]=dx-dxdy-v[6];
          v[3]=dydz-dxdydz;
          v[2]=dz-dydz-v[6];
          v[1]=dy-dydz-v[5];
          v[0]=1-dy-dz+dydz-v[4];
          qsort( v, 8, sizeof(double), &comparedouble );
          *rbuf = v[ index ];
          continue;
        }
        /* here, we are sure we are on some border */
        if ( (x < 0.0) || (ix == t1dimx) ) {
          if ( (y < 0.0) || (iy == t1dimy) ) {
            if ( (z < 0.0) || (iz == t1dimz) ) {
              *rbuf = 1;
              continue;
            }
            dz = z - iz;
            v[1] = dz;
            v[0] = 1-dz;
            qsort( v, 2, sizeof(double), &comparedouble );
            *rbuf = v[ index ];
            continue;
          }
          dy = y - iy;
          if ( (z < 0.0) || (iz == t1dimz) ) {
              v[1] = dy;
              v[0] = 1-dy;
              qsort( v, 2, sizeof(double), &comparedouble );
              *rbuf = v[ index ];
              continue;
          }
          dz = z - iz;
          v[3] = dy*dz;
          v[2] = (1-dy)*dz;
          v[1] = dy*(1-dz);
          v[0] = (1-dy)*(1-dz);
          qsort( v, 4, sizeof(double), &comparedouble );
          *rbuf = v[ index ];
          continue;
        }
        /* here we are sure that the border is either
           along the Y or the Z axis */
        dx = x - ix;
        if ( (y < 0.0) || (iy == t1dimy) ) {
          if ( (z < 0.0) || (iz == t1dimz) ) {
              v[1] = dx;
              v[0] = 1-dx;
              qsort( v, 2, sizeof(double), &comparedouble );
              *rbuf = v[ index ];
              continue;
          }
          dz = z - iz;
          v[3] = dx*dz;
          v[2] = (1-dx)*dz;
          v[1] = dx*(1-dz);
          v[0] = (1-dx)*(1-dz);
          qsort( v, 4, sizeof(double), &comparedouble );
          *rbuf = v[ index ];
          continue;
        }
        /* here we are sure that the border is along the Z axis */
        dy = y - iy;
        v[3] = dx*dy;
        v[2] = (1-dx)*dy;
        v[1] = dx*(1-dy);
        v[0] = (1-dx)*(1-dy);
        qsort( v, 4, sizeof(double), &comparedouble );
        *rbuf = v[ index ];
        continue;
      }
    }
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





void Coeff3DTriLin4x4 ( int *theDim, /* dimensions of this buffer */
                        float* resBuf, /* result buffer */
                        int *resDim,  /* dimensions of this buffer */
                        double* mat,   /* transformation matrix */
                        int index
                        )
{
  char *proc = "Coeff3DTriLin4x4";
  size_t first = 0;
  size_t last;
  int i;
  typeChunks chunks;
  _LinearCoefficientParam p;
  
  /* preparing parallelism
   */
  first = 0;
  last = (size_t)resDim[2] * (size_t)resDim[1] * (size_t)resDim[0] - 1;
  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return;
  }
  
  p.theDim = theDim;
  p.resBuf = resBuf;
  p.resDim = resDim;
  p.mat = mat;
  p.index = index;
  if ( index < 0 ) index = 0;
  if ( index > 7 ) index = 7;

  
  for ( i=0; i<chunks.n_allocated_chunks; i++ ) 
    chunks.data[i].parameters = (void*)(&p);
  
  /* processing
   */
  if ( processChunks( &_Coeff3DTriLin4x4, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to resample image\n", proc );
    freeChunks( &chunks );
    return;
  }
  
  freeChunks( &chunks );
}




















static void *_Coeff2DTriLin4x4 ( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _LinearCoefficientParam *p = (_LinearCoefficientParam *)parameter;
  
  int *theDim = p->theDim;
  void* resBuf = p->resBuf;
  int *resDim = p->resDim;
  double* mat = p->mat; 
  int index = p->index;

  double v[4];
  int n;
  
  register int i, j, k, ix, iy;
  register double x, y, dx, dy, dxdy;
  int rdimx=resDim[0], rdimy=resDim[1];
  int tdimx=theDim[0], tdimy=theDim[1];

  register int t1dimx=tdimx-1, t1dimy=tdimy-1;
  register double ddimx = (double)tdimx-0.5, ddimy = (double)tdimy-0.5;
  register float *rbuf = (float*)resBuf;
  
  int ifirst, jfirst, kfirst;
  int ilast, jlast, klast;
  int iend, jend;
  
  k = kfirst = first / (rdimx*rdimy);
  j = jfirst = (first - kfirst*(rdimx*rdimy)) / rdimx;
  i = ifirst = (first - kfirst*(rdimx*rdimy) - jfirst*rdimx);
  
  klast = last / (rdimx*rdimy);
  jlast = (last - klast*(rdimx*rdimy)) / rdimx;
  ilast = (last - klast*(rdimx*rdimy) - jlast*rdimx);
  
  rbuf += first;
  
  for ( ; k<=klast; k++, j=0 ) {
    if ( _verbose_ > 1 )
      fprintf( stderr, "Processing slice %d\r", k );
    /* tbuf represente le premier point du plan */
    jend = (k==klast) ? jlast+1 : rdimy;
    for ( ; j<jend; j++, i=0 ) {
      iend = (j==jlast && k==klast) ? ilast+1 : rdimx;
      for ( ; i<iend; i++, rbuf++ ) {
        /* computation of the corresponding point coordinates in theBuf */
        x = mat[0] * i +  mat[1] * j +              mat[3];
        if ((x <= -0.5) || ( x >= ddimx)) { *rbuf = 0; continue; }
        y = mat[4] * i +  mat[5] * j +              mat[7];
        if ((y <= -0.5) || ( y >= ddimy)) { *rbuf = 0; continue; }

        /* here, the point lies on the borders or completely inside
           the image */
        ix = (int)x;
        iy = (int)y;

        for ( n=0; n<4; n++ ) v[n] = 0;

        /* are we on the border or not ? */
        if ( (x > 0.0) && (ix < t1dimx) &&
             (y > 0.0) && (iy < t1dimy) ) {
          dx = x - ix;
          dy = y - iy;
          dxdy = dx*dy;
          /* we have
             v[5]=dxdy;         coefficient of tbuf(ix+1,iy+1)
             v[4]=dx-dxdy;      coefficient of tbuf(ix+1,iy  )
             v[1]=dy-dxdy;      coefficient of tbuf(ix  ,iy+1)
             v[0]=1-dx-dy+dxdy; coefficient of tbuf(ix  ,iy  )
          */
          v[3] = dxdy;
          v[2] = dx-dxdy;
          v[1] = dy-dxdy;
          v[0] = 1-dx-v[1];
          qsort( v, 4, sizeof(double), &comparedouble );
          *rbuf = v[ index ];
          continue;
        }

        /* here, we are sure we are on some border */
        if ( (x < 0.0) || (ix == t1dimx) ) {
          /* we just look at y */
          if ( (y < 0.0) || (iy == t1dimy) ) {
            *rbuf = 1;
            continue;
          }
          dy = y - iy;
          v[1] = dy;
          v[0] = 1-dy;
          qsort( v, 2, sizeof(double), &comparedouble );
          *rbuf = v[ index ];
          continue;
        }
        dx = x - ix;
        v[1] = dx;
        v[0] = 1-dx;
        qsort( v, 2, sizeof(double), &comparedouble );
        *rbuf = v[ index ];
        continue;
      }
    }
  }
  chunk->ret = 1;
  return( (void*)NULL );
}





void Coeff2DTriLin4x4( int *theDim, /* dimensions of this buffer */
                       float* resBuf, /* result buffer */
                       int *resDim,  /* dimensions of this buffer */
                       double* mat,   /* transformation matrix */
                       int index
                       )
{
  char *proc = "Coeff2DTriLin4x4";
  size_t first = 0;
  size_t last;
  int i;
  typeChunks chunks;
  _LinearCoefficientParam p;
  
  /* preparing parallelism
   */
  first = 0;
  last = (size_t)resDim[2] * (size_t)resDim[1] * (size_t)resDim[0] - 1;
  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return;
  }
  
  p.theDim = theDim;
  p.resBuf = resBuf;
  p.resDim = resDim;
  p.mat = mat;
  p.index = index;
  if ( index < 0 ) index = 0;
  if ( index > 7 ) index = 7;
  
  for ( i=0; i<chunks.n_allocated_chunks; i++ ) 
    chunks.data[i].parameters = (void*)(&p);
  
  /* processing
   */
  if ( processChunks( &_Coeff2DTriLin4x4, &chunks, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to resample image\n", proc );
    freeChunks( &chunks );
    return;
  }
  
  freeChunks( &chunks );
}

