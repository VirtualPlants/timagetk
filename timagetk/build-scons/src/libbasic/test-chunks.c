/*************************************************************************
 * test-chunks.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Fri Nov 16 22:45:44 CET 2012
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
#include <sys/time.h>
#include <math.h>
#include <string.h>

#include <chunks.h>


static char *usage = "\
 [-parallel|-no-parallel] [-omp-max-chunks %d] [-pthread-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n";

static char *detail = "\
[-parallel|-no-parallel] # use parallelism (or not)\n\
[-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
[-max-chunks %d] # maximal number of chunks\n\
[-parallel-scheduling|-ps default|static|dynamic-one|dynamic|guided] # type\n\
  of scheduling for open mp\n";


typedef struct local_parameter {
  int omp_maxchunks;
  int pthread_maxchunks;
  size_t n;
} local_parameter;

typedef struct test_parameter {
  int id;
  double res;
} test_parameter;


static void _ErrorParse( char *str, int flag );
static void _Parse( int argc, char *argv[], local_parameter *p );
static void _InitParam( local_parameter *par );
static double _GetTime();
static double _GetClock();
static char *_BaseName( char *p );

static char *program = NULL;





void *testFunction( void *par )
{
  typeChunk *c = (typeChunk *)par;
  test_parameter *t = (test_parameter*)(c->parameters);
  size_t i;
  double result=0.0;

  /* 
     fprintf( stderr, " ... starting chunk #%d\n", t->id );
  */

  for ( i=c->first; i<=c->last; i++ ) {
    result = result + sin(i) * tan(i);
  }
  t->res = result;
  c->ret = 1;
  return( (void*)NULL );
}



int parallelProcessing( size_t first, size_t last, char *desc )
{
  int j;
  test_parameter *t;
  typeChunks chunks;

  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;

  initChunks( &chunks );
  if ( buildChunks( &chunks, first, last, desc ) != 1 ) {
    fprintf( stderr, "unable to compute chunks\n" );
    return( -1 );
  }

  fprintf( stderr, "= processing '%s', #chunks=%d\n", desc, chunks.n_allocated_chunks );

  t = (test_parameter *)malloc( chunks.n_allocated_chunks * sizeof(test_parameter) );
  if ( t == (test_parameter *)NULL ) {
    freeChunks( &chunks );
    fprintf( stderr, "unable to allocate parameters\n" );
    return( -1 );
  }

  for ( j=0; j<chunks.n_allocated_chunks; j++ ) {
    t[j].id = j;
    chunks.data[j].parameters = (void*)(&t[j]);
  }

  if ( processChunks( &testFunction, &chunks, desc ) != 1 ) {
    free( t );
    freeChunks( &chunks );
    fprintf( stderr, "unable to process\n" );
    return( -1 );
  }

  free( t );
  freeChunks( &chunks );
    
  time_exit = _GetTime();
  clock_exit = _GetClock();
  
  if ( 1 ) { 
    fprintf( stderr, "= %s: elapsed (real) time = %f\n", _BaseName( program ), time_exit - time_init );
    fprintf( stderr, "\t       elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
    fprintf( stderr, "\t       ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
  }

  return( 1 );
}




int main ( int argc, char *argv[] )
{
  local_parameter p;

  size_t i;
  size_t first = 0;
  size_t last = 10000000;
  int maxchunks;
  

  program = argv[0];

  _InitParam( &p );
  _Parse( argc, argv, &p );

  for ( i=0; i<p.n; i++ ) {

    fprintf( stderr, "\n" );
    fprintf( stderr, "========================================\n" );
    fprintf( stderr, "= test #%lu [%lu - %lu]\n", i, first, last );
    fprintf( stderr, "========================================\n" );
    fprintf( stderr, "\n" );

    maxchunks = getMaxChunks();

    setParallelism( _NO_PARALLELISM_ );
    if ( parallelProcessing( first, last, "no parallelism" ) != 1 ) {
      fprintf( stderr, "error when processing\n" );
      exit( -1 );
    }

    setMaxChunks( maxchunks );

    maxchunks = getMaxChunks();

    setParallelism( _DEFAULT_PARALLELISM_ );
    if ( parallelProcessing( first, last, "default" ) != 1 ) {
      fprintf( stderr, "error when processing\n" );
      exit( -1 );
    }

    setMaxChunks( maxchunks );

    maxchunks = getMaxChunks();

    setParallelism( _OMP_PARALLELISM_ );
    if ( parallelProcessing( first, last, "open mp" ) != 1 ) {
      fprintf( stderr, "error when processing\n" );
      exit( -1 );
    }

    setMaxChunks( maxchunks );

    maxchunks = getMaxChunks();

    setParallelism( _PTHREAD_PARALLELISM_ );
    if ( parallelProcessing( first, last, "pthread" ) != 1 ) {
      fprintf( stderr, "error when processing\n" );
      exit( -1 );
    }

    setMaxChunks( maxchunks );

    
    last *= 10;
  }


  exit( 0 );
}





static void _ErrorParse( char *str, int flag )
{
  FILE *output = stderr;

  (void)fprintf( output, "Usage: %s %s", _BaseName(program), usage );
  if ( flag == 1 )
    (void)fprintf( output, "%s\n", detail );
  exit( -1 );
}





static void _Parse( int argc, char *argv[], local_parameter *p )
{
  int i;
  int status;

  program = argv[0];

  for ( i=1; i<argc; i++ ) {

   /* parallelism
     */
    if ( strcmp ( argv[i], "-parallel" ) == 0 ) {
      setParallelism( _DEFAULT_PARALLELISM_ );
    }

    else if ( strcmp ( argv[i], "-no-parallel" ) == 0 ) {
       setParallelism( _NO_PARALLELISM_ );
    }
    
    else if ( strcmp ( argv[i], "-parallelism-type" ) == 0 ||
               strcmp ( argv[i], "-parallel-type" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-parallelism-type", 0 );
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
	_ErrorParse( "-parallelism-type", 0 );
      }
    }


    else if ( strcmp ( argv[i], "-pthread-max-chunks" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-pthread-max-chunks", 0 );
      status = sscanf( argv[i], "%d", &(p->pthread_maxchunks) );
      if ( status <= 0 ) _ErrorParse( "-pthread-max-chunks", 0 );
    }

    else if ( strcmp ( argv[i], "-omp-max-chunks" ) == 0 ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-omp-max-chunks", 0 );
      status = sscanf( argv[i], "%d", &(p->omp_maxchunks) );
      if ( status <= 0 ) _ErrorParse( "-omp-max-chunks", 0 );
    }

    else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 || 
	      ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][3] == '\0') ) {
      i ++;
      if ( i >= argc)    _ErrorParse( "-parallel-scheduling", 0 );
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
	_ErrorParse( "-omp-scheduling", 0 );
      }
    }


    else {
      fprintf(stderr,"unknown option: '%s'\n",argv[i]);
    }
  }
}






static void _InitParam( local_parameter *p ) 
{
  p->pthread_maxchunks = -1;
  p->omp_maxchunks = -1;
  p->n = 4;
}


static double _GetTime() 
{
  struct timeval tv;
  gettimeofday(&tv, (void *)0);
  return ( (double) tv.tv_sec + tv.tv_usec*1e-6 );
}

static double _GetClock() 
{
  return ( (double) clock() / (double)CLOCKS_PER_SEC );
}








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
