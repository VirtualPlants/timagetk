/*************************************************************************
 * drawShapes.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer 11 sep 2013 15:34:22 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <convert.h>
#include <drawShapes.h>


static int _verbose_ = 1;






int drawCylinder( void *inputBuf,
          void *resultBuf,
          int *theDim,
          bufferType type,
          double *center,
          double *vector,
          double radius,
          double value )
{
  char *proc = "drawCylinder";
  size_t size;
  int x, y, z, i;
  double n, nvector[3], d, pt[3];
  double radius2 = radius*radius;

  size = (size_t)theDim[0] * (size_t)theDim[1] * (size_t)theDim[2];
  if ( ConvertBuffer( inputBuf, type, resultBuf, type, size ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to copy buffer\n", proc );
    return( 0 );
  }

  n = sqrt( vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2]);
  nvector[0] = vector[0] / n;
  nvector[1] = vector[1] / n;
  nvector[2] = vector[2] / n;

  switch( type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( 0 );
    
  case UCHAR :
    {
      u8 *theBuf = (u8*)resultBuf;
      u8 v;
      if ( value <= 0.0 ) v = 0;
      else if ( value >= 255.0 ) v = 255;
      else v = (int)(value + 0.5);
      for ( i=0, z=0; z<theDim[2]; z++ )
      for ( y=0; y<theDim[1]; y++ )
      for ( x=0; x<theDim[0]; x++, i++ ) {
	d = (x-center[0])*nvector[0] + (y-center[1])*nvector[1] + (y-center[2])*nvector[2];
	pt[0] = center[0] + d * nvector[0];
	pt[1] = center[1] + d * nvector[1];
	pt[2] = center[2] + d * nvector[2];
	d = (x-pt[0])*(x-pt[0]) + (y-pt[1])*(y-pt[1]) + (z-pt[2])*(z-pt[2]);
	if ( d <= radius2 )
        theBuf[i] = v;
      }
    }
    break;

  }
  return( 1 );
}





/*
 * line3d was dervied from DigitalLine.c published as "Digital Line Drawing"
 * by Paul Heckbert from "Graphics Gems", Academic Press, 1990
 * 
 * 3D modifications by Bob Pendleton. The original source code was in the public
 * domain, the author of the 3D version places his modifications in the
 * public domain as well.
 * 
 * line3d uses Bresenham's algorithm to generate the 3 dimensional points on a
 * line from (x1, y1, z1) to (x2, y2, z2)
 * 
 */

/* find maximum of a and b */
#define MAX(a,b) (((a)>(b))?(a):(b))

/* absolute value of a */
#define ABS(a) (((a)<0) ? -(a) : (a))

/* take sign of a, either -1, 0, or 1 */
#define ZSGN(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

int drawLine( void *inputBuf,
           void *resultBuf,
           int *theDim,
           bufferType type,
           int *pt1,
           int *pt2,
           double value )
{
  char *proc = "drawLine";
  size_t size;
  int ix, ixy;
  int dx, dy, dz;  
  int ax, ay, az;
  int sx, sy, sz;
  int x, y, z;
  int xd, yd, zd;

  ix = theDim[0];
  ixy = theDim[0] * theDim[1];
  size = (size_t)theDim[0] * (size_t)theDim[1] * (size_t)theDim[2];
  if ( ConvertBuffer( inputBuf, type, resultBuf, type, size ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to copy buffer\n", proc );
    return( 0 );
  }

  dx = pt2[0] - pt1[0];
  dy = pt2[1] - pt1[1];
  dz = pt2[2] - pt1[2];

  ax = ABS(dx) << 1;
  ay = ABS(dy) << 1;
  az = ABS(dz) << 1;

  sx = ZSGN(dx);
  sy = ZSGN(dy);
  sz = ZSGN(dz);

  x = pt1[0];
  y = pt1[1];
  z = pt1[2];

  switch( type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( 0 );
    
  case UCHAR :
    {
      u8 *theBuf = (u8*)resultBuf;
      u8 v;
      if ( value <= 0.0 ) v = 0;
      else if ( value >= 255.0 ) v = 255;
      else v = (int)(value + 0.5);

      /* x dominant
       */
      if (ax >= MAX(ay, az)) {
        yd = ay - (ax >> 1);
        zd = az - (ax >> 1);
        for (;;) {
        theBuf[z*ixy + y*ix + x] = v;
        if (x == pt2[0]) {
          break;
        }
        if (yd >= 0) {
          y += sy;
          yd -= ax;
        }
        if (zd >= 0) {
          z += sz;
          zd -= ax;
        }
        x += sx;
        yd += ay;
        zd += az;
        }
      }
      /* y dominant
       */
      else if (ay >= MAX(ax, az)) {
        xd = ax - (ay >> 1);
        zd = az - (ay >> 1);
        for (;;) {
        theBuf[z*ixy + y*ix + x] = v;
        if (y == pt2[1]) {
          break;
        }
        if (xd >= 0) {
          x += sx;
          xd -= ay;
        }
        if (zd >= 0) {
          z += sz;
          zd -= ay;
        }
        y += sy;
        xd += ax;
        zd += az;
        }
      }
      /* z dominant 
       */
      else if (az >= MAX(ax, ay)) {
        xd = ax - (az >> 1);
        yd = ay - (az >> 1);
        for (;;) {
        theBuf[z*ixy + y*ix + x] = v;
        if (z == pt2[2]) {
          break;
        }
        if (xd >= 0) {
          x += sx;
          xd -= az;
        }
        if (yd >= 0) {
          y += sy;
          yd -= az;
        }
        z += sz;
        xd += ax;
        yd += ay;
        }
      }
    }
    break;

  }
  return( 1 );
}





int drawRectangle( void *inputBuf,
           void *resultBuf,
           int *theDim,
           bufferType type,
           int *pt1,
           int *pt2,
           double value )
{
  char *proc = "drawRectangle";
  size_t size;
  int x, y, z, i;

  size = (size_t)theDim[0] * (size_t)theDim[1] * (size_t)theDim[2];
  if ( ConvertBuffer( inputBuf, type, resultBuf, type, size ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to copy buffer\n", proc );
    return( 0 );
  }

  switch( type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( 0 );
    
  case UCHAR :
    {
      u8 *theBuf = (u8*)resultBuf;
      u8 v;
      if ( value <= 0.0 ) v = 0;
      else if ( value >= 255.0 ) v = 255;
      else v = (int)(value + 0.5);
      for ( i=0, z=0; z<theDim[2]; z++ )
      for ( y=0; y<theDim[1]; y++ )
      for ( x=0; x<theDim[0]; x++, i++ ) {
	if ( x >= pt1[0] && x <= pt2[0] 
           && y >= pt1[1] && y <= pt2[1]
           && z >= pt1[2] && z <= pt2[2] ) {
        theBuf[i] = v;
	}
      }
    }
    break;

  }
  return( 1 );
}





int drawSphere( void *inputBuf,
		void *resultBuf,
		int *theDim,
		bufferType type,
		double *center,
		double radius,
		double value )
{
  char *proc = "drawSphere";
  size_t size;
  int x, y, z, i;
  double r2;
  double radius2 = radius*radius;

  size = (size_t)theDim[0] * (size_t)theDim[1] * (size_t)theDim[2];
  if ( ConvertBuffer( inputBuf, type, resultBuf, type, size ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to copy buffer\n", proc );
    return( 0 );
  }

  switch( type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( 0 );
    
  case UCHAR :
    {
      u8 *theBuf = (u8*)resultBuf;
      u8 v;
      if ( value <= 0.0 ) v = 0;
      else if ( value >= 255.0 ) v = 255;
      else v = (int)(value + 0.5);
      for ( i=0, z=0; z<theDim[2]; z++ )
      for ( y=0; y<theDim[1]; y++ )
      for ( x=0; x<theDim[0]; x++, i++ ) {
	r2 = (x-center[0])*(x-center[0])
        + (y-center[1])*(y-center[1])
        + (z-center[2])*(z-center[2]);
	if ( r2 <= radius2 )
        theBuf[i] = v;
      }
    }
    break;

  }
  return( 1 );
}





