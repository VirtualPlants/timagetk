/*************************************************************************
 * vt_temporal-mtaching.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun 30 sep 2013 21:37:35 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <fcntl.h>

#include <unistd.h>

#include <vt_temporal-matching.h>



static int _verbose_ = 1;



void initmeasureList( measureListType *l )
{
  l->data = (measureType*)NULL;
  l->dt = 1.0;
  l->n_data = 0;
  l->n_allocated_data = 0;
} 



void freeMeasureList( measureListType *l )
{
  if ( l->data != (measureType*)NULL ) {
    free( l->data );
  } 
  initmeasureList( l );
} 



void fprintfMeasureList( FILE *f, measureListType *l )
{
  int i;
  
  for ( i=0; i<l->n_data; i++ ) {
    fprintf( f, "#%03d: ", i );
    if ( l->data[i].measure > 0 ) 
      fprintf( f, "%f ", l->data[i].measure );
    else 
      fprintf( f, "0" );
    fprintf( f, "\n" );
  }
}






static int _size_to_be_allocated_ = 100;

static int addMeasureToMeasureList( measureListType *l, float m )
{
  char *proc = "addMeasureToMeasureList";
  int s =  l->n_allocated_data;
  measureType *data;

  if ( l->n_data == l->n_allocated_data ) {
    s += _size_to_be_allocated_;
    data = (measureType*)malloc( s * sizeof(measureType) );
    if ( data == (measureType*)NULL ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
    } 
    if ( l->n_allocated_data > 0 ) {
      (void)memcpy( data, l->data, l->n_allocated_data*sizeof(measureType) );
      free( l->data );
    }
    l->n_allocated_data = s;
    l->data = data;
  }

  l->data[l->n_data].measure = m;
  l->data[l->n_data].time = l->n_data * l->dt;

  l->n_data ++;

  return( 1 );
} 




int readMeasureList(  measureListType *l, char *filename )
{
  char *proc = "readMeasureList";
  FILE *f;
  float m;
  int i;

  f = fopen( filename, "r" );
  if ( f == (FILE*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to open '%s'\n", proc, filename );
    return( -1 );
  }

  i = 0;
  while ( fscanf( f, "%f", &m ) == 1 ) {
    if ( addMeasureToMeasureList( l, m ) != 1 ) {
      fclose( f );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to add measure #%d (=%f) to list\n", proc, i, m );
      return( -1 );
    }
    i ++;
  }

  if ( 1 && _verbose_ )
    fprintf( stderr, "%s: read %d values\n", proc, i );

  fclose( f );


  return( 1 );
}



int extractMeasureMaxima(  measureListType *max,  measureListType *l )
{
  char *proc = "extractMeasureMaxima";
  int i, n;

  max->data = (measureType*)malloc( l->n_data * sizeof( measureType ) );
  if ( max->data == (measureType*)NULL ) {
     if ( _verbose_ )
	fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
  }

  max->n_allocated_data = max->n_data = l->n_data;

  for ( i=0; i<l->n_data; i++ ) {
    max->data[i].measure = 0.0;
    max->data[i].time = l->data[i].time;
  }

  for ( i=1; i<l->n_data-1; i++ ) {
    if ( l->data[i-1].measure >= l->data[i].measure 
	 || l->data[i].measure <= l->data[i+1].measure )
      continue;
    if ( 1.1 * (float)l->data[i-1].measure >= l->data[i].measure 
	 && l->data[i].measure <= 1.1 * (float)l->data[i+1].measure )
      continue;
    max->data[i].measure = l->data[i].measure;
  }

  for ( i=0, n=0; i<l->n_data; i++ )
    if ( max->data[i].measure > 0 ) n++;

  fprintf( stderr, "%s: %d maxima\n", proc, n );

  return( 1 );
}






typedef struct {
  int measure;
  int sumShift;
  int used;
} similarityMeasureType;
  



int temporalRegistration( measureListType *ref,
			  measureListType *flo,
			  int leftBoundary,
			  int rightBoundary,
			  int maxShift )
{
  char *proc = "temporalRegistration";
  similarityMeasureType* similarityMeasure = (similarityMeasureType*)NULL;
  similarityMeasureType firstMeasure;
  int shift, r, f;
  int rmin, rmax;
  int matchedr, matcheds;

  similarityMeasure = (similarityMeasureType*)malloc( (rightBoundary-leftBoundary+1) 
						* sizeof(similarityMeasureType) );
  if ( similarityMeasure == (similarityMeasureType*)NULL ) {
    if ( _verbose_ )
	fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
  }

  /* calcul de la measure
   */
  for ( shift = leftBoundary; shift <= rightBoundary; shift ++ ) {

    if ( 0 )
      fprintf( stdout, "processing shift = %d\n", shift );

    similarityMeasure[shift-leftBoundary].measure = 0;
    similarityMeasure[shift-leftBoundary].sumShift = 0;



    for ( f=0; f<flo->n_data; f++ ) {

      if ( flo->data[f].measure > 0.0 ) {

	if ( 0 )
	  fprintf( stdout, " processing floating maxima at %d", f );

	rmin = f + shift - maxShift;
	if ( rmin < 0 ) rmin = 0;
	rmax = f + shift + maxShift;
	if ( rmax >= ref->n_data ) rmax = ref->n_data - 1;

	if ( 0 )
	  fprintf( stdout, " vs [%d %d]\n", rmin, rmax );

	for ( matcheds=maxShift+1, r=rmin; r<=rmax; r++ ) {
	  if ( ref->data[r].measure > 0 ) {
	    if ( 0 )
	      fprintf( stdout, " test r=%d -> matcheds=%d, abs=%d\n",
		       r, matcheds, abs( r-f-shift ) );
	    if ( matcheds > abs( r-f-shift ) ) {
	      matcheds = abs( r-f-shift );
	      matchedr = r;
	    }
	  }
	}

	if ( matcheds <= maxShift ) {
	  if ( 0 )
	    fprintf( stdout, "  -> found a match at r = %d\n", matchedr );
	  similarityMeasure[shift-leftBoundary].measure ++;
	  similarityMeasure[shift-leftBoundary].sumShift += matcheds;
	}
	else {
	  if ( 0 )
	    fprintf( stdout, "  -> found no match\n" );
	}
      }
      
    }

  }

  for ( shift = leftBoundary; shift <= rightBoundary; shift ++ ) {
    fprintf( stdout, "shift = %4d ; measure = %2d, sumShift=%4d", 
	     shift, similarityMeasure[shift-leftBoundary].measure,
	     similarityMeasure[shift-leftBoundary].sumShift );
    fprintf( stdout, " meanShift=%f", (float)similarityMeasure[shift-leftBoundary].sumShift/(float)similarityMeasure[shift-leftBoundary].measure );
    fprintf( stdout, "\n" );
  }


  /* first non-null measure 
   */
  for ( shift = leftBoundary; shift <= rightBoundary; shift ++ ) {
    if ( similarityMeasure[shift-leftBoundary].measure > 0 ) {
      firstMeasure = similarityMeasure[shift-leftBoundary];
      fprintf( stdout, "%s: first non-null measure for shift=%d\n", proc, shift );
      shift = rightBoundary;
    }
  }



  return( 1 );
}





/*------------------------------------------------------------
 *
 *
 *
 *------------------------------------------------------------ */



static char *_BaseName( char *p )
{
  int l;
  if ( p == (char*)NULL ) return( (char*)NULL );
  l = strlen( p ) - 1;
  while ( l >= 0 && p[l] != '/' ) l--;
  if ( l < 0 ) l = 0;
  if ( p[l] == '/' ) l++;
  return( &(p[l]) );
}



static void writeScilabPlot( FILE *f, int fd, measureListType *l, char *s )
{
  char *proc = "writeScilabPlot";
  float *d = (float*)NULL;
  int i;

  d = (float*)malloc( l->n_data * sizeof(float) );
  if ( d == (float*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return;
  }

  for ( i=0; i<l->n_data; i++ )
    d[i] = l->data[i].time;

  if ( write( fd, d, l->n_data * sizeof(float) ) == -1 ) {
    fprintf( stderr, "%s: error when writing time\n", proc );
  }

  for ( i=0; i<l->n_data; i++ )
    d[i] = l->data[i].measure;

  if ( write( fd, d, l->n_data * sizeof(float) ) == -1 ) {
    fprintf( stderr, "%s: error when writing measure\n", proc );
  }
  
  fprintf( f, "\n" );
  fprintf( f, "//\n" );
  if ( s == (char*)NULL )
    fprintf( f, "// read data\n" );
  else
    fprintf( f, "// read data '%s'\n", s );
  fprintf( f, "//\n" );
  fprintf( f, "\n" );
  
  if ( s == (char*)NULL )
    fprintf( f, "TIME=mget( %d, 'f', f);\n", l->n_data );
  else 
    fprintf( f, "TIME_%s=mget( %d, 'f', f);\n", s, l->n_data );

  if ( s == (char*)NULL )
    fprintf( f, "DATA=mget( %d, 'f', f);\n", l->n_data );
  else 
    fprintf( f, "DATA_%s=mget( %d, 'f', f);\n", s,l->n_data );

  fprintf( f, "\n" );

  if ( s == (char*)NULL )
    fprintf( f, "plot( TIME, DATA, \"k-\", \"thickness\", 2 );\n" );
  else 
    fprintf( f, "plot( TIME_%s, DATA_%s, \"k-\", \"thickness\", 2 );\n", s, s );
  
  fprintf( f, "\n" );
  fprintf( f, "\n" );
  fprintf( f, "\n" );

}





void writeScilab( measureListType *theOriginalRef,
		  measureListType *theOriginalFlo,
		  char *name )
{
  char *proc = "writeScilab";

  char *template;
  char *defaultname = "curves";
  char *filename = (char*)NULL;
  FILE *f;
  int fd;

  template = ( name != (char*)NULL ) ? name : defaultname;
  filename = (char*)malloc( strlen( template ) + 5 );
  if ( filename == (char*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate file name\n", proc );
    return;
  }

  /* open files
   */
  sprintf( filename, "%s.raw", template );

  fd = open( filename, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH );
  if ( fd == -1 ) {
    free( filename );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to open '%s' for writing\n", proc, filename );
    return;
  }

  sprintf( filename, "%s.sce", template );
  f = fopen( filename, "w" );

  if ( f == (FILE*)NULL ) {
    free( filename );
    close( fd );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to open '%s' for writing\n", proc, filename );
    return;
  }

  free( filename );


  /*
   */

  fprintf( f, "\n" );
  fprintf( f, "f=mopen('%s.raw','r');\n", _BaseName( template ) );
  fprintf( f, "\n" );


  /* figures
   */
  fprintf( f, "\n" );
  fprintf( f, "\n" );
  fprintf( f, "\n" );
  fprintf( f, "figure;\n" );
  fprintf( f, "set(gca(),\"auto_clear\",\"off\");\n" );
  fprintf( f, "\n" );
  fprintf( f, "\n" );
  fprintf( f, "\n" );

  writeScilabPlot( f, fd, theOriginalRef, "refOriginal" );
  writeScilabPlot( f, fd, theOriginalFlo, "floOriginal" );

  fprintf( f, "\n" );
  fprintf( f, "mclose(f);\n" );
  fprintf( f, "\n" );
  
  /* close files
   */
  close( fd );
  fclose( f );  
}

