/*************************************************************************
 * chunks.c -
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef MAC
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif


#ifdef _OPENMP
#include <omp.h>
#endif
#include <pthread.h>

#include <chunks.h>







/* debug / verbose variables
 */

static int _verbose_ = 1;
static int _debug_ = 0;

void setVerboseInChunks( int v )
{
  _verbose_ = v;
}

void incrementVerboseInChunks(  )
{
  _verbose_ ++;
}

void decrementVerboseInChunks(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}

void setDebugInChunks( int v )
{
  _debug_ = v;
}

void incrementDebugInChunks(  )
{
  _debug_ ++;
}

void decrementDebugInChunks(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}





/* parallelism, scheduling
 */

static parallelismType _parallelism_ = _DEFAULT_PARALLELISM_;

static ompSchedulingType _omp_scheduling_ = _DYNAMIC_OMP_SCHEDULING_;


void setParallelism( parallelismType p )
{
  _parallelism_ = p;
}


parallelismType getParallelism( )
{
  return( _parallelism_ );
}


void setOmpScheduling( ompSchedulingType s )
{
  _omp_scheduling_ = s;
}


ompSchedulingType getOmpScheduling( )
{
  return( _omp_scheduling_ );
}





/* chunk related variable
 */

static int _max_chunks_ = -1;
static int _min_elements_ = 100;


void setMaxChunks( int c )
{
  _max_chunks_ = c;
}


int getMaxChunks( )
{
  return( _max_chunks_ );
}




/************************************************************
 *
 * chunks processing
 *
 ************************************************************/



static int _ompProcessChunks( _chunk_callfunction ftn, typeChunks *chunks, char *from )
{
  int n;

  if ( chunks->n_allocated_chunks > 1 ) {

    switch( chunks->ompScheduling ) {

    default :
    case _DEFAULT_OMP_SCHEDULING_ :
      if ( _debug_ ) fprintf( stderr, "%s: default openmp scheduling\n", from );    
#ifdef _OPENMP
#pragma omp parallel for
#endif
      for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
        if ( _debug_ >=2 ) {
          fprintf( stderr, "%s: processing chunk #%d/%d", from, n+1, chunks->n_allocated_chunks );
#ifdef _OPENMP
          fprintf( stderr, " attributed to thread #%d", omp_get_thread_num() );
#endif
          fprintf( stderr, "\n" );
        }
        (void)(*ftn)( &(chunks->data[n]) );
      }
      break;

    case _DYNAMIC_OMP_SCHEDULING_ :
      if ( _debug_ ) fprintf( stderr, "%s: dynamic openmp scheduling\n", from );    
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
      for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
        if ( _debug_ >=2 ) {
          fprintf( stderr, "%s: processing chunk #%d/%d", from, n+1, chunks->n_allocated_chunks );
#ifdef _OPENMP
          fprintf( stderr, " attributed to thread #%d", omp_get_thread_num() );
#endif
          fprintf( stderr, "\n" );
        }
        (void)(*ftn)( &(chunks->data[n]) );
      }
      break;

    case _DYNAMIC_ONE_OMP_SCHEDULING_ :
      if ( _debug_ ) fprintf( stderr, "%s: (dynamic,1) openmp scheduling\n", from );    
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic,1)
#endif
      for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
        if ( _debug_ >=2 ) {
          fprintf( stderr, "%s: processing chunk #%d/%d", from, n+1, chunks->n_allocated_chunks );
#ifdef _OPENMP
          fprintf( stderr, " attributed to thread #%d", omp_get_thread_num() );
#endif
          fprintf( stderr, "\n" );
        }
        (void)(*ftn)( &(chunks->data[n]) );
      }
      break;

    case _GUIDED_OMP_SCHEDULING_ :
      if ( _debug_ ) fprintf( stderr, "%s: (guided) openmp scheduling\n", from );    
#ifdef _OPENMP
#pragma omp parallel for schedule(guided)
#endif
      for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
        if ( _debug_ >=2 ) {
          fprintf( stderr, "%s: processing chunk #%d/%d", from, n+1, chunks->n_allocated_chunks );
#ifdef _OPENMP
          fprintf( stderr, " attributed to thread #%d", omp_get_thread_num() );
#endif
          fprintf( stderr, "\n" );
        }
        (void)(*ftn)( &(chunks->data[n]) );
      }
      break;

    case _STATIC_OMP_SCHEDULING_ :
      if ( _debug_ ) fprintf( stderr, "%s: (static) openmp scheduling\n", from );    
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
      for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
        if ( _debug_ >=2 ) {
          fprintf( stderr, "%s: processing chunk #%d/%d", from, n+1, chunks->n_allocated_chunks );
#ifdef _OPENMP
          fprintf( stderr, " attributed to thread #%d", omp_get_thread_num() );
#endif
          fprintf( stderr, "\n" );
        }
        (void)(*ftn)( &(chunks->data[n]) );
      }
      break;
    }

  }
  else {
    
    if ( _debug_ ) 
      fprintf( stderr, "%s: sequential loop\n", from );    
    for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
      if ( _debug_ >= 2 ) {
        fprintf( stderr, "%s: processing chunk #%d/%d", from, n+1, chunks->n_allocated_chunks );
        fprintf( stderr, " in a sequential way" );
        fprintf( stderr, "\n" );
      }
      (void)(*ftn)( &(chunks->data[n]) );
    }

  }



  /* check the returned values in case of error
   */
  for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
    if ( chunks->data[n].ret != 1 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: error when computing chunk #%d\n", from, n );
      }
      return( -1 );
    }
  }

  return( 1 );
}





static int _pthreadProcessChunks( _chunk_callfunction ftn, typeChunks *chunks, char *from )
{
  char *proc = "_pthreadProcessChunks";
  int n;
  int rc;
  void *status;
  pthread_t *thread;
  pthread_attr_t attr;

  if ( _debug_ ) fprintf( stderr, "%s: pthread scheduling\n", from );

  thread = (pthread_t*)malloc( chunks->n_allocated_chunks * sizeof( pthread_t ) );
  if ( thread == (pthread_t*)NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate array of %d threads\n", proc, chunks->n_allocated_chunks );
    return( -1 );
  }
  
  pthread_attr_init( &attr );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
  
  for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
    rc =  pthread_create( &thread[n], &attr, ftn, &(chunks->data[n]) );
    if ( rc ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: error when creating threads #%d/%d (returned code = %d)\n", proc, n,  chunks->n_allocated_chunks, rc );
      free( thread );
      return( -1 );
    }
  }

  pthread_attr_destroy(&attr);
  for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
    rc = pthread_join( thread[n], &status );
    if ( rc ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: error when joining threads #%d/%d (returned code = %d)\n", proc, n,  chunks->n_allocated_chunks, rc );
      free( thread );
      return( -1 );
    }
  }

  free( thread );
  return( 1 );
}





int processChunks( _chunk_callfunction ftn, typeChunks *chunks, char *from )
{
  char *proc = "processChunks";
  int n;


  switch( _parallelism_ ) {

  case _NO_PARALLELISM_ :
    if ( _debug_ >= 1 ) 
      fprintf( stderr, "%s: _NO_PARALLELISM_ case\n", proc );
    for ( n=0; n<chunks->n_allocated_chunks; n++ ) {
      (void)(*ftn)( &(chunks->data[n]) ); 
    }
    break;

  default :

  case _DEFAULT_PARALLELISM_ :
    if ( _debug_ >= 1 ) 
      fprintf( stderr, "%s: _DEFAULT_PARALLELISM_ case\n", proc );

#ifdef _OPENMP
  case _OMP_PARALLELISM_ :
    if ( _debug_ >= 1 ) 
      fprintf( stderr, "%s: _OMP_PARALLELISM_ case\n", proc );
    return ( _ompProcessChunks( ftn, chunks, from ) );
    break;
#endif
    
  case _PTHREAD_PARALLELISM_ :
    if ( _debug_ >= 1 ) 
      fprintf( stderr, "%s: _PTHREAD_PARALLELISM_ case\n", proc );
    return ( _pthreadProcessChunks( ftn, chunks, from ) );
    break;
  }


  return( 1 );
}





/************************************************************
 *
 * chunks construction (ad hoc function)
 *
 ************************************************************/


#ifdef MAC

static int _getNCPU()
{
  int n=1;

  int mib[4];
  size_t lmib = 4;
    
  int ncpu;
  size_t lncpu = sizeof( ncpu );

  int nactivecpu;
  size_t lnactivecpu = sizeof( nactivecpu );

  int nphysicalcpu;
  size_t lnphysicalcpu = sizeof( nphysicalcpu );
  int nphysicalcpumax;
  size_t lnphysicalcpumax = sizeof( nphysicalcpumax );
  int nlogicalcpu;
  size_t lnlogicalcpu = sizeof( nlogicalcpu );
  int nlogicalcpumax;
  size_t lnlogicalcpumax = sizeof( nlogicalcpumax );

  int navailablecpu;
  size_t lnavailablecpu = sizeof( ncpu );
    
  switch( _parallelism_ ) {

  case _NO_PARALLELISM_ :
    return( 1 );

  default :

  case _DEFAULT_PARALLELISM_ :

#ifdef _OPENMP
  case _OMP_PARALLELISM_ :
    return( omp_get_max_threads() );
#endif

  case _PTHREAD_PARALLELISM_ :

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;  
    sysctl( mib, 2, &ncpu, &lncpu, NULL, 0);
    
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;  
    sysctl( mib, 2, &navailablecpu, &lnavailablecpu, NULL, 0);
    
    sysctlnametomib( "hw.ncpu", mib, &lmib );
    sysctl( mib, 2, &ncpu, &lncpu, NULL, 0);
    
    sysctlnametomib( "hw.activecpu", mib, &lmib );
    sysctl( mib, 2, &nactivecpu, &lnactivecpu, NULL, 0);
    
    sysctlnametomib( "hw.physicalcpu", mib, &lmib );
    sysctl( mib, 2, &nphysicalcpu, &lnphysicalcpu, NULL, 0);
    
    sysctlnametomib( "hw.physicalcpu_max", mib, &lmib );
    sysctl( mib, 2, &nphysicalcpumax, &lnphysicalcpumax, NULL, 0);
    
    sysctlnametomib( "hw.logicalcpu", mib, &lmib );
    sysctl( mib, 2, &nlogicalcpu, &lnlogicalcpu, NULL, 0);
    
    sysctlnametomib( "hw.logicalcpu_max", mib, &lmib );
    sysctl( mib, 2, &nlogicalcpumax, &lnlogicalcpumax, NULL, 0);
    
    if ( _debug_ >= 4 ) {
      fprintf( stderr, "ncpu = %d\n", ncpu );
      fprintf( stderr, "navailablecpu = %d\n", navailablecpu );
      fprintf( stderr, "nactivecpu = %d\n", nactivecpu );
      fprintf( stderr, "nphysicalcpu = %d\n", nphysicalcpu );
      fprintf( stderr, "nphysicalcpumax = %d\n", nphysicalcpumax );
      fprintf( stderr, "nlogicalcpu = %d\n", nlogicalcpu );
      fprintf( stderr, "nlogicalcpumax = %d\n", nlogicalcpumax );
    }
  
    if ( n < ncpu ) n = ncpu;
    if ( n < nactivecpu ) n = nactivecpu;
    if ( n < nphysicalcpu ) n = nphysicalcpu;
    if ( n < nphysicalcpumax ) n = nphysicalcpumax;
    if ( n < nlogicalcpu ) n = nlogicalcpu;
    if ( n < nlogicalcpumax ) n = nlogicalcpumax;
    if ( n < navailablecpu ) n = navailablecpu;
    return( n );
  }

  return( 1 );
}

#else

static int _getNCPU()
{
  return( sysconf( _SC_NPROCESSORS_ONLN ) );
}

#endif



static void _setMaxChunks( )
{  
  switch( _parallelism_ ) {

  case _NO_PARALLELISM_ :
    _max_chunks_ = 1;
    break;

  default :

  case _DEFAULT_PARALLELISM_ :

#ifdef _OPENMP
  case _OMP_PARALLELISM_ :
    if ( omp_get_max_threads() == 1 ) {
      _max_chunks_ = 1;
    }
    else {
      if ( _max_chunks_ <= 0 ) _max_chunks_ = 100;
    }
    break;
#endif

  case _PTHREAD_PARALLELISM_ :
    if ( _max_chunks_ <= 0 ) _max_chunks_ = _getNCPU();
    break;
  }

  if ( _max_chunks_ == 1 ) _parallelism_ = _NO_PARALLELISM_;
}





int buildChunks( typeChunks *chunks, size_t first, size_t last, char *from )
{
  char *proc = "buildChunks";
  size_t size = last-first+1;
  int n;

  _setMaxChunks( );
  n = _max_chunks_;

  chunks->ompScheduling = _omp_scheduling_;



  /* only one chunk
   */
  if ( n == 1 ) {
    if ( allocBuildOneChunk( chunks, first, last ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when allocating one chunk\n", proc );
      return( -1 );
    }
    return( 1 );
  }

  

  /* what to do when there are too few elements ?
   */
  if ( size < (size_t)n*_min_elements_ ) {
    
    n = size / _min_elements_;
    if ( size % _min_elements_ > 0 ) n ++;

    if ( n == 1 ) {
      if ( allocBuildOneChunk( chunks, first, last ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when allocating one chunk (too few element case)\n", proc );
        return( -1 );
      }
      return( 1 );
    }
  }

  if ( allocBuildEqualChunks( chunks, first, last, n ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when building %d chunks (call from %s)\n", proc, n, from );
    return( -1 );
  }
  
  if ( _debug_ ) {
    fprintf( stderr, "\n" );
    if ( from != (char*)NULL )
      fprintf( stderr, "%s: has computed %d chunks from '%s'\n", proc, chunks->n_allocated_chunks, from );
    else
      fprintf( stderr, "%s: has computed %d chunks from '%s'\n", proc, chunks->n_allocated_chunks, from );
  }
  
  return( 1 );
}





/************************************************************
 *
 * chunks construction (generic functions)
 *
 ************************************************************/


int allocBuildOneChunk( typeChunks *chunks, size_t first, size_t last )
{
  char *proc = "oneChunk";

  if ( allocChunks( chunks, 1 ) <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating one chunk\n", proc );
    return( -1 );
  }
  chunks->data[0].first = first;
  chunks->data[0].last = last;
  return( 1 );
}



int buildEqualChunks( typeChunk *chunk, size_t first, size_t last, int n )
{
  char *proc = "buildEqualChunks";
  size_t totalSize = last+1-first;
  size_t f = first;
  size_t i, chunkSize;
  
  if ( n <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative number of chunks\n", proc );
    return( -1 );
  }
    
  chunkSize = totalSize / n;

  if ( chunkSize <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative or null chunk size (too many chunks ?)\n", proc );
    return( -1 );
  }

  for ( i=0; i<(size_t)n; i++, f+=chunkSize ) {
    /* chunk of size chunkSize+1
     */
    if ( chunkSize * (n-i) < totalSize ) {
      totalSize -= chunkSize + 1;
      chunk[ i ].first = f;
      chunk[ i ].last  = f+(chunkSize+1)-1;
      f++;
    }
    /* chunk of size chunkSize
     */
    else {
      totalSize -= chunkSize;
      chunk[ i ].first = f;
      chunk[ i ].last  = f+chunkSize-1;
    }
  }

  return( 1 );
}



int allocBuildEqualChunks( typeChunks *chunks, size_t first, size_t last, int nchunks )
{
  char *proc = "allocBuildEqualChunks";
  size_t size = last - first + 1;
  int n = nchunks;
  
  if ( n <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative number of chunks\n", proc );
    return( -1 );
  }

  if ( size < (size_t)n ) {
    n = size;
  }

  if ( allocChunks( chunks, n ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating chunks\n", proc );
    return( -1 );
  }
  
  if ( buildEqualChunks( &(chunks->data[0]), first, last, n ) != 1 ) {
    freeChunks( chunks );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when calculating chunks\n", proc );
    return( -1 );
  }
  
  return( 1 );
}



int allocBuildInequalChunks( typeChunks *chunks, size_t first, size_t last, 
                             double fraction,   /* fraction of data to be put in one bucket */
                             int chunks_bucket, /* number of chunks for one bucket */
                             size_t minimal_chunk_size,
                             int maximal_chunk_number )
{
  char *proc = "allocBuildInequalChunks";
  size_t totalSize = last+1-first;
  size_t f = first;
  size_t i, chunkSize;
  
  int maxchunks;
  int n, nchunks;
  
  if ( maximal_chunk_number <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative maximal number of chunks\n", proc );
    return( -1 );
  }
  
  if ( chunks_bucket <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative number of chunks in a bucket\n", proc );
    return( -1 );
  }
  
  if ( fraction < 0.0 || 1.0 <= fraction ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: fraction should be in ]0,1[\n", proc );
    return( -1 );
  }
  


  /* calculating the number of chunks
   */
  for ( nchunks=0, maxchunks=maximal_chunk_number, totalSize=last+1-first; totalSize > 0 && maxchunks > 0;  ) {

    /* put the remaining data in the remaining chunks
       -> equal repartition in maxchunks chunks
    */
    if ( maxchunks <= chunks_bucket ) {
      nchunks += maxchunks;
      totalSize = 0;
      if ( _debug_ ) fprintf( stderr, "(1) add %d chunks -> %d\n", maxchunks, nchunks );
      continue;
    }
    
    /* the remaining data fits in one bucket
       -> equal repartition in chunks_bucket chunks
    */
    if ( totalSize / chunks_bucket < minimal_chunk_size ) {
      nchunks += chunks_bucket;
      totalSize = 0;
      if ( _debug_ ) fprintf( stderr, "(2) add %d chunks -> %d\n", chunks_bucket, nchunks );
      continue;
    }
    
    /* the considered fraction of data in the bucket
       yield too small chunks
       -> equal repartition in min( totalSize / minimal_chunk_size, maxchunks)
    */
    if ( (fraction * totalSize) / chunks_bucket < minimal_chunk_size ) {
      n = totalSize / minimal_chunk_size;
      if ( n > maxchunks ) n = maxchunks;
      nchunks += n;
      totalSize = 0;
      if ( _debug_ ) fprintf( stderr, "(3) add %d chunks -> %d\n", n, nchunks );
      continue;
    }
    
    /* after filling the bucket, there will be too much
       remaining data
       -> equal repartition in  maxchunks
    */
    if ( (maxchunks <= 2*chunks_bucket) &&
         (fraction * totalSize) / chunks_bucket < ((1-fraction) * totalSize) / (maxchunks-chunks_bucket) ) {
      nchunks += maxchunks;
      totalSize = 0;
      if ( _debug_ ) fprintf( stderr, "(4) add %d chunks -> %d\n", maxchunks, nchunks );
      continue;
    }

    /* generic case
     */
    chunkSize = (fraction * totalSize) / chunks_bucket;
    
    nchunks += chunks_bucket;
    maxchunks -= chunks_bucket;
    totalSize -= chunks_bucket * chunkSize;
    
    if ( _debug_ ) fprintf( stderr, "(5) add %d chunks -> %d\n", chunks_bucket, nchunks );
  }
  
  
  
  /* allocating the chunks
   */
  if ( allocChunks( chunks, nchunks ) <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating chunks\n", proc );
    return( -1 );
  }




  /* filling chunks
   */
  for ( nchunks=0, maxchunks=maximal_chunk_number, totalSize=last+1-first, f=first; totalSize > 0 && maxchunks > 0;  ) {
    
    /* put the remaining data in the remaining chunks
       -> equal repartition in maxchunks chunks
    */
    if ( maxchunks <= chunks_bucket ) {
      if ( buildEqualChunks( &(chunks->data[nchunks]), f, last, maxchunks ) != 1 ) {
        freeChunks( chunks );
        if ( _verbose_ )
          fprintf( stderr, "%s: error when calculating chunks\n", proc );
        return( -1 );
      }
      totalSize = 0;
      continue;
    }
    
    /* the remaining data fits in one bucket
       -> equal repartition in chunks_bucket chunks
    */
    if ( totalSize / chunks_bucket < minimal_chunk_size ) {
      if ( buildEqualChunks( &(chunks->data[nchunks]), f, last, chunks_bucket ) != 1 ) {
        freeChunks( chunks );
        if ( _verbose_ )
          fprintf( stderr, "%s: error when calculating chunks\n", proc );
        return( -1 );
      }
      totalSize = 0;
      continue;
    }
    
    /* the considered fraction of data in the bucket
       yield too small chunks
       -> equal repartition in min( totalSize / minimal_chunk_size, maxchunks)
    */
    if ( (fraction * totalSize) / chunks_bucket < minimal_chunk_size ) {
      n = totalSize / minimal_chunk_size;
      if ( n > maxchunks ) n = maxchunks;
      if ( buildEqualChunks( &(chunks->data[nchunks]), f, last, n ) != 1 ) {
        freeChunks( chunks );
        if ( _verbose_ )
          fprintf( stderr, "%s: error when calculating chunks\n", proc );
        return( -1 );
      }
      totalSize = 0;
      continue;
    }
    
    /* after filling the bucket, there will be too much
       remaining data
       -> equal repartition in  maxchunks
    */
    if ( (maxchunks <= 2*chunks_bucket) &&
         (fraction * totalSize) / chunks_bucket < ((1-fraction) * totalSize) / (maxchunks-chunks_bucket) ) {
      if ( buildEqualChunks( &(chunks->data[nchunks]), f, last, maxchunks ) != 1 ) {
        freeChunks( chunks );
        if ( _verbose_ )
          fprintf( stderr, "%s: error when calculating chunks\n", proc );
        return( -1 );
      }
      totalSize = 0;
      continue;
    }

    /* generic case
     */
    chunkSize = (fraction * totalSize) / chunks_bucket;
    for (i=0; i<(size_t)chunks_bucket; i++, f+=chunkSize, totalSize-=chunkSize, nchunks++, maxchunks-- ) {
      chunks->data[ nchunks ].first = f;
      chunks->data[ nchunks ].last  = f+chunkSize-1;
    }
    
  }
  
  return( 1 );
}





/************************************************************
 *
 * management
 *
 ************************************************************/



void initChunk( typeChunk *chunk )
{
  chunk->first = 1;
  chunk->last = 0;
  chunk->parameters = (void*)NULL;
  chunk->ret = 0;
}



void initChunks( typeChunks *chunks )
{
  chunks->data = (typeChunk*)NULL;
  chunks->n_allocated_chunks = 0;
  chunks->ompScheduling = _DEFAULT_OMP_SCHEDULING_;
}



void freeChunks( typeChunks *chunks )
{
  if ( chunks->data != (typeChunk*)NULL ) free( chunks->data );
  initChunks( chunks );
}



int allocChunks( typeChunks *chunks, int n )
{
  char *proc = "allocChunks";
  int i;
  
  if ( n <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative or null number of chunks\n", proc );
    return( -1 );
  }
    
  chunks->data = (typeChunk*)malloc( n *sizeof(typeChunk) );
  if ( chunks->data == (typeChunk*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating %d chunks\n", proc, n );
    return( -1 );
  }

  for ( i=0; i<n; i++ )
    initChunk( &(chunks->data[i] ) );

  chunks->n_allocated_chunks = n;

  return( 1 );
}



void printChunks( FILE *theFile, typeChunks *chunks, char *s )
{
  char *proc = "printChunks";
  int i;
  FILE *f = theFile;

  if ( f == NULL ) f = stdout;
  
  if ( s != (char *)NULL )
    fprintf( f, "%s: information on '%s'\n", proc, s );
  
  if ( chunks->n_allocated_chunks <= 0 || chunks->data == (typeChunk*)NULL ) {
    fprintf( f, "empty chunks\n" );
    return;
  }
    
  for ( i=0; i<chunks->n_allocated_chunks; i++ )
    fprintf( f, "#%3d [%12lu %12lu] = %12lu\n", 
             i, chunks->data[i].first, chunks->data[i].last, chunks->data[i].last + 1 - chunks->data[i].first );
  fprintf( f, "\n" );
}



/************************************************************
 *
 * test
 *
 ************************************************************/
