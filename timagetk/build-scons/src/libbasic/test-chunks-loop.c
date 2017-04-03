/*************************************************************************
 * test-chunks-loop.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sat Jan  5 22:38:45 CET 2013
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */





#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <chunks.h>







typedef struct {
  size_t dimx;
  size_t dimy;
  size_t dimz;
  FILE *f;
} _LoopParam;


void *_ParseLoop ( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _LoopParam *p = (_LoopParam *)parameter;
  size_t dimx = p->dimx;
  size_t dimy = p->dimy;
  size_t dimxy = dimx*dimy;
  int xf, xl;
  int yf, yl;
  int zf, zl;
  int x, y, z;
  
  z = zf = first / dimxy;
  y = yf = (first - zf*dimxy) / dimx;
  x = xf = first - zf*dimxy - yf*dimx;

  zl = last / dimxy;
  yl = (last - zl*dimxy) / dimx;
  xl = last - zl*dimxy - yl*dimx;

  if ( 0 ) {
    for ( ; z<=zl; z++, y=0 )
    for ( ; (y<dimy && z<zl) || (y<=yl && z==zl); y++, x=0 )
    for ( ; (x<dimx && (z<zl || (y<yl && z==zl))) || (x<=xl && y==yl && z==zl); x++ )
      fprintf( p->f, "%3d %3d %3d\n", x, y, z );
  }
  else {
    for ( ; z<zl; z++, y=0 )
    for ( ; y<dimy; y++, x=0 )
    for ( ; x<dimx; x++ )
      fprintf( p->f, "%3d %3d %3d\n", x, y, z );
    /* z = zl */
    for ( ; y<yl; y++, x=0 )
    for ( ; x<dimx; x++ )
      fprintf( p->f, "%3d %3d %3d\n", x, y, z );
    /* z = zl, y = yl */
    for ( ; x<=xl; x++ )
      fprintf( p->f, "%3d %3d %3d\n", x, y, z );
  }
  

  chunk->ret = 1;
  return( (void*)NULL );
}





int main ( int argc, char *argv[] )
{
  size_t dimx;
  size_t dimy;
  size_t dimz;
  int x, y,z;
  
  size_t first = 0;
  size_t last;
  
  typeChunks chunks;
  _LoopParam p;
  int i;

  int t, ntests=1;

  char name[256];	      
  FILE *f;


  initChunks( &chunks );
  srandom( time( 0 ) );


  for ( t=0; t<ntests; t++ ) {
    dimx = 10 * (double)random()/(double)(RAND_MAX) + 1;
    dimy = 10 * (double)random()/(double)(RAND_MAX) + 1;
    dimz = 10 * (double)random()/(double)(RAND_MAX) + 1;
    first = 0;
    last = dimx*dimy*dimz - 1;

    fprintf( stderr, " %02lux%02lux%02lu-n%03d", dimx, dimy, dimz, t );


    sprintf( name, "res-%02lux%02lux%02lu-n%03d.txt", dimx, dimy, dimz, t );
    f = fopen( name, "w" );
    for ( z=0; z<dimz; z++ )
    for ( y=0; y<dimy; y++ )
    for ( x=0; x<dimx; x++ )
      fprintf( f, "%3d %3d %3d\n", x, y, z );
    fclose( f );

    
    p.dimx = dimx;
    p.dimy = dimy;
    p.dimz = dimz;

    if ( 0 ) {
    sprintf( name, "one-%02lux%02lux%02lu-n%03d.txt", dimx, dimy, dimz, t );
    f = fopen( name, "w" );
    p.f = f;
    if ( _ParseLoop( (void *)&p, first, last ) != 1 ) {
      fprintf( stderr, "%s: unable to use function\n", argv[0] );
      return( -1 );
    }
    fclose( f );
    }


    sprintf( name, "tst-%02lux%02lux%02lu-n%03d.txt", dimx, dimy, dimz, t );
    f = fopen( name, "w" );
    p.f = f;

    initChunks( &chunks );
    if ( buildChunks( &chunks, first, last, argv[0] ) != 1 ) {
      fprintf( stderr, "%s: unable to compute chunks\n", argv[0] );
      return( -1 );
    }
    
    for ( i=0; i<chunks.n_allocated_chunks; i++ ) 
      chunks.data[i].parameters = (void*)(&p);

    if ( 1 ) fprintf( stderr, "\n#%3d: use %d chunks\n", t, chunks.n_allocated_chunks );

    if ( processChunks( &_ParseLoop, &chunks, argv[0] ) != 1 ) {
      fprintf( stderr, "%s: unable to compute loops\n", argv[0] );
      return( -1 );
    }

    freeChunks( &chunks );

    fclose( f );

  }

  fprintf( stderr, "\n" );

  return( 0 );
}
