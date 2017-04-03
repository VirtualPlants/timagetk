
#include <math.h>

#include <vt_unix.h>

#include <vt_copy.h>

#include <vt_removeLine.h>



static int _verbose_ = 1;

void VT_SetVerboseInVtRemoveLine( int v )
{
  _verbose_ = v;
}

void VT_IncrementVerboseInVtRemoveLine(  )
{
  _verbose_ ++;
}

void VT_DecrementVerboseInVtRemoveLine(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





typedef struct {
  double intensityMin;
  double intensityMax;
  double intensityMean;
  double intensityStddev;

  double contrastMin;
  double contrastMax;
  double contrastMean;
  double contrastStddev;
  int contrastPos1;
  int contrastPos2;

  int index;
  double criteria;
  int flag;
} localMeasures;

typedef struct {
  localMeasures *data;
  localMeasures **pointer;
  int n_allocated_data;
} localMeasuresList;





static void _InitLocalMeasures( localMeasures *s )
{
  s->intensityMin = 0.0;
  s->intensityMax = 0.0;
  s->intensityMean = 0.0;
  s->intensityStddev = 0.0;
  s->flag = 0;
}



static void _InitLocalMeasuresList( localMeasuresList *l )
{
  l->data = (localMeasures *)NULL;
  l->pointer = (localMeasures **)NULL;
  l->n_allocated_data = 0;
}



static int _AllocateLocalMeasuresList( localMeasuresList *l, int size )
{
  char *proc = "_AllocateLocalMeasuresList";
  int i;

  if ( size <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative or zero size\n",  proc );
    return( -1 );
  }
  
  l->data = (localMeasures *)malloc( size * sizeof(localMeasures) );
  if ( l->data == (localMeasures *)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate data\n",  proc );
    return( -1 );
  }
  
  l->pointer = (localMeasures **)malloc( size * sizeof(localMeasures *) );
  if ( l->pointer == (localMeasures **)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate pointer\n",  proc );
    free( l->data );
    return( -1 );
  }

  for ( i=0; i<size; i++ ) {
    _InitLocalMeasures( &(l->data[i]) );
    l->pointer[i] = &(l->data[i]);
  }

  l->n_allocated_data = size;

  return( 0 );
}



static void _FreeLocalMeasuresList( localMeasuresList *l )
{
  if ( l->data != (localMeasures *)NULL )
    free( l->data );
  if ( l->pointer != (localMeasures **)NULL )
    free( l-> pointer );
  _InitLocalMeasuresList( l );
}



static void _PrintLocalMeasures( FILE *f, localMeasures *l )
{
  fprintf( f, "xy slice #%4d: contrast  range=[%5d - %5d], mean=%7.2f, #pos1=%7d, #pos1=%7d\n",
	   l->index, 
	   (int)l->contrastMin, (int)l->contrastMax, l->contrastMean, l->contrastPos1, l->contrastPos2 );
  if ( 0 )
    fprintf( f, "                intensity range=[%5d - %5d], mean=%7.2f, std-dev=%7.2f\n",
	     (int)l->intensityMin, (int)l->intensityMax, l->intensityMean, l->intensityStddev);
}



#ifdef _UNUSED_
static void _PrintLocalMeasuresListData( FILE *f, localMeasuresList *l )
{
  int i;
  
  for ( i=0; i<l->n_allocated_data; i++ )
    _PrintLocalMeasures( f, &(l->data[i]) );
}
#endif



static void _PrintLocalMeasuresListFlaggedData( FILE *f, localMeasuresList *l )
{
  int i;
  int n = 0;
  
  for ( i=0; i<l->n_allocated_data; i++ )
    if ( l->data[i].flag ) {
      _PrintLocalMeasures( f, &(l->data[i]) );
      n++;
    }
  fprintf( f, "... there were %d flagged slice\n", n );
}



#ifdef _UNUSED_
static void _PrintLocalMeasuresListPointer( FILE *f, localMeasuresList *l )
{
  int i;
  
  for ( i=0; i<l->n_allocated_data; i++ )
    _PrintLocalMeasures( f, l->pointer[i] );
}
#endif




static void _PrintLocalMeasuresListSelectedData( FILE *f, localMeasuresList *l )
{
  int i;
  
  for ( i=1010; i<1070; i++ )
    _PrintLocalMeasures( f, &(l->data[i]) );
}





static void _PrintLocalMeasuresListFirstPointer( FILE *f, localMeasuresList *l )
{
  int i;
  
  for ( i=0; i<20; i++ )
    _PrintLocalMeasures( f, l->pointer[i] );
}





int _CompareLocalMeasures( const void *a, const void *b ) 
{
  localMeasures **sa = (localMeasures **)a;
  localMeasures **sb = (localMeasures **)b;

  if ( (*sa)->criteria > (*sb)->criteria ) 
    return( -1 );
  if ( (*sa)->criteria < (*sb)->criteria ) 
    return(  1 );
  return( 0 );
}



#ifdef _UNUSED_
static void _SortLocalMeasuresWRTratioMeanStddev( localMeasuresList *l )
{
  int i;
  for ( i=0; i<l->n_allocated_data; i++ ) {
    l->data[i].criteria = l->data[i].intensityMean / l->data[i].intensityStddev;
  }
  qsort( l->pointer, l->n_allocated_data, sizeof(localMeasures *), &_CompareLocalMeasures );
}
#endif



static void _SortLocalMeasuresWRTmaxContrastPos( localMeasuresList *l )
{
  int i;
  for ( i=0; i<l->n_allocated_data; i++ ) {
    l->data[i].criteria = l->data[i].contrastPos1;
    if ( l->data[i].criteria < l->data[i].contrastPos2 )
      l->data[i].criteria = l->data[i].contrastPos2;
  }
  qsort( l->pointer, l->n_allocated_data, sizeof(localMeasures *), &_CompareLocalMeasures );
}









/************************************************************
 *
 *
 *
 ************************************************************/

#define ZXSLICEINTENSITYLOCALMEASURES( TYPE ) {                                   \
  TYPE ***theBuf = (TYPE***)image->array;                                         \
  d->index = y;                                                                   \
  d->intensityMin = theBuf[0][y][0];                                              \
  d->intensityMax = theBuf[0][y][0];                                              \
  d->intensityMean = 0;                                                           \
  d->intensityStddev = 0;                                                         \
  sum = 0.0;                                                                      \
  for ( z=0; z<(int)image->dim.z; z++ )                                                \
  for ( x=0; x<(int)image->dim.x; x++ ) {                                              \
    if ( d->intensityMin > theBuf[z][y][x] ) d->intensityMin = theBuf[z][y][x];   \
    if ( d->intensityMax < theBuf[z][y][x] ) d->intensityMax = theBuf[z][y][x];   \
    sum += theBuf[z][y][x];                                                       \
  }                                                                               \
  sum /= (double)n;                                                               \
  d->intensityMean = sum;                                                         \
  res = 0.0;                                                                      \
  for ( z=0; z<(int)image->dim.z; z++ )                                                \
  for ( x=0; x<(int)image->dim.x; x++ ) {                                              \
    res += (theBuf[z][y][x] - sum) * (theBuf[z][y][x] - sum);                     \
  }                                                                               \
  res /= (double)n;                                                               \
  d->intensityStddev = sqrt( res );                                               \
}





static int _ComputeXZSliceIntensityLocalMeasures( vt_image *image,
						  localMeasures *d, int y )
{
  char *proc = "_ComputeXZSliceIntensityLocalMeasures";
  int x, z, n;
  double sum, res;
  
  n = image->dim.x * image->dim.z;
  
  switch( image->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled\n", proc );
    return( -1 );
    break;
  case SSHORT :
    ZXSLICEINTENSITYLOCALMEASURES( s16 )
    break;
  case USHORT :
    ZXSLICEINTENSITYLOCALMEASURES( u16 )
    break;
  }
  
  return( 0 );
}





#ifdef _UNUSED_
static int _ComputeImageIntensityLocalMeasures( vt_image *image,
						localMeasuresList *l )
{
  char *proc = "_ComputeImageIntensityLocalMeasures";
  int y;

  for ( y=0; y<(int)image->dim.y; y++ ) {
    if ( _ComputeXZSliceIntensityLocalMeasures( image, &(l->data[y]), y ) != 0 ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: error when computing intensity measures for xz slice #%d\n", proc, y );
      return( -1 );
    }
  }

  return( 0 );
}
#endif




#define IMAGECONTRASTLOCALMEASURES( TYPE ) {                              \
  TYPE ***theBuf = (TYPE***)image->array;                                   \
  int min1, min2;                                                         \
  int c, c1, c2;                                                          \
  for ( y=0; y<(int)image->dim.y; y++ ) {                                      \
    l->data[y].index = y;                                                 \
    l->data[y].contrastMin = 0.0;                                         \
    l->data[y].contrastMax = 0.0;                                         \
    l->data[y].contrastMean = 0;                                          \
    l->data[y].contrastStddev = 0;                                        \
    l->data[y].contrastPos1 = 0;                                          \
    l->data[y].contrastPos2 = 0;                                          \
    if ( y == 0 ) {                                                       \
      for ( z=0; z<(int)image->dim.z; z++ )                               \
      for ( x=0; x<(int)image->dim.x; x++ ) {                             \
        min2 = theBuf[z][y+1][x];                                         \
        c = c2 = theBuf[z][y][x] - min2;                                  \
        if ( c2 > 0 ) l->data[y].contrastPos2 ++;                         \
        l->data[y].contrastMean += c;                                     \
        if ( x == 0 && z == 0 ) {                                         \
          l->data[y].contrastMin = l->data[y].contrastMax = c;            \
        }                                                                 \
        else {                                                            \
          if ( l->data[y].contrastMin > c ) l->data[y].contrastMin = c;   \
          if ( l->data[y].contrastMax < c ) l->data[y].contrastMax = c;   \
        }                                                                 \
      }                                                                   \
    }                                                                     \
    else if ( y == (int)image->dim.y-1 ) {                                \
      for ( z=0; z<(int)image->dim.z; z++ )                               \
      for ( x=0; x<(int)image->dim.x; x++ ) {                             \
        min1 = theBuf[z][y-1][x];                                         \
        c = c1 = theBuf[z][y][x] - min1;                                  \
        if ( c1 > 0 ) l->data[y].contrastPos1 ++;                         \
        l->data[y].contrastMean += c;                                     \
        if ( x == 0 && z == 0 ) {                                         \
          l->data[y].contrastMin = l->data[y].contrastMax = c;            \
        }                                                                 \
        else {                                                            \
          if ( l->data[y].contrastMin > c ) l->data[y].contrastMin = c;   \
          if ( l->data[y].contrastMax < c ) l->data[y].contrastMax = c;   \
        }                                                                 \
      }                                                                   \
    }                                                                     \
    else {                                                                \
      for ( z=0; z<(int)image->dim.z; z++ )                               \
      for ( x=0; x<(int)image->dim.x; x++ ) {                             \
        min1 = theBuf[z][y-1][x];                                         \
        min2 = theBuf[z][y+1][x];                                         \
        c1 = theBuf[z][y][x] - min1;                                      \
        c2 = theBuf[z][y][x] - min2;                                      \
        c = (int)((double)(c1+c2) / 2.0 + 0.5 );                          \
        if ( c1 > 0 ) l->data[y].contrastPos1 ++;                         \
        if ( c2 > 0 ) l->data[y].contrastPos2 ++;                         \
        l->data[y].contrastMean += c;                                     \
        if ( x == 0 && z == 0 ) {                                         \
          l->data[y].contrastMin = l->data[y].contrastMax = c;            \
        }                                                                 \
        else {                                                            \
          if ( l->data[y].contrastMin > c ) l->data[y].contrastMin = c;   \
          if ( l->data[y].contrastMax < c ) l->data[y].contrastMax = c;   \
        }                                                                 \
      }                                                                   \
    }                                                                     \
    l->data[y].contrastMean  /= (double)n;                                \
  }                                                                       \
}





static int _ComputeImageContrastLocalMeasures( vt_image *image,
					   localMeasuresList *l )
{
  char *proc = "_ComputeImageContrastLocalMeasures";
  int x, y, z, n;

  n = image->dim.x * image->dim.z;

  switch( image->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled\n", proc );
    return( -1 );
    break;
  case SSHORT :
    IMAGECONTRASTLOCALMEASURES( s16 )
    break;
  case USHORT :
    IMAGECONTRASTLOCALMEASURES( u16 )
    break;
  }

  return( 0 );
}









/************************************************************
 *
 *
 *
 ************************************************************/

#ifdef _UNUSED_
static int _CorrectLine( vt_image *theIm, vt_image *resIm, int y )
{
  char *proc = "_CorrectLine";
  int x, z;

  switch( theIm->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled\n", proc );
    return( -1 );
    break;
  case SSHORT :
    {
      s16 ***theBuf = (s16***)theIm->array;
      s16 ***resBuf = (s16***)resIm->array;
      if ( y == 0 ) {
  for ( z=0; z<(int)theIm->dim.z; z++ )
  for ( x=0; x<(int)theIm->dim.x; x++ ) {
	  resBuf[z][y][x] = theBuf[z][y+1][x];
	}
      }
      else if ( y == theIm->dim.y - 1 ) {
  for ( z=0; z<(int)theIm->dim.z; z++ )
  for ( x=0; x<(int)theIm->dim.x; x++ ) {
	  resBuf[z][y][x] = theBuf[z][y-1][x];
	}
      }
      else {
  for ( z=0; z<(int)theIm->dim.z; z++ )
  for ( x=0; x<(int)theIm->dim.x; x++ ) {
	  resBuf[z][y][x] = (int)( (double)(theBuf[z][y-1][x] + theBuf[z][y+1][x])/2.0 + 0.5 );
	}
      }
    }
    break;
 }

 return( 0 );
}
#endif





static int _CorrectXZSliceRangeLinearInterpolation( vt_image *theIm, vt_image *resIm, int ymin, int ymax )
{
  char *proc = "_CorrectXZSliceRangeLinearInterpolation";
  int x, y, z;
  int i, k;
  int y1 = ymin - 1;
  int y2 = ymax + 1;
  double c1, c2;
  double value, sum, noise;
  int n;
  
  if ( y1 < 0 || y2 >= (int)theIm->dim.y ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: can not deal with borders slices, range was [%d-%d]\n", proc, ymin, ymax );
    return( 0 );
  }

  switch( theIm->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled\n", proc );
    return( -1 );
    break;
  case SSHORT :
    {
      s16 ***theBuf = (s16***)theIm->array;
      s16 ***resBuf = (s16***)resIm->array;
      for ( y=ymin; y<=ymax; y++ ) {

	c1 = (double)(y2-y) /  (double)(y2-y1);
	c2 = (double)(y-y1) /  (double)(y2-y1);

  for ( z=0; z<(int)theIm->dim.z; z++ )
  for ( x=0; x<(int)theIm->dim.x; x++ ) {
	  value = c1*(double)theBuf[z][y1][x] + c2*(double)theBuf[z][y2][x];
	  sum = 0.0;
	  n = 0;
	  for ( i=-1; i<=1; i++ ) {
      if ( x+i < 0 || x+i >= (int)theIm->dim.x ) continue;
	    for ( k=-1; k<=1; k++ ) {
        if ( z+k < 0 || z+k >= (int)theIm->dim.z ) continue;
	      sum += theBuf[z+k][y][x+i];
	      n ++;
	    }
	  }
	  noise = sum/(double)n - (double)theBuf[z][y][x];
	  noise *= value / (double)theBuf[z][y][x];
	  value += noise;
	  resBuf[z][y][x] = (int)( value + 0.5 );
	}

      }
    }
    break;
 }

 return( 0 );
}




#define XZSLICERANGEINTENSITYRESCALING( TYPE ) {                                            \
  TYPE ***theBuf = (TYPE***)theIm->array;                                                   \
  TYPE ***resBuf = (TYPE***)resIm->array;                                                   \
  for ( y=ymin; y<=ymax; y++ ) {                                                            \
    if ( _ComputeXZSliceIntensityLocalMeasures( theIm, &meas, y ) != 0 ) {                  \
      if ( _verbose_ )                                                                      \
        fprintf( stderr, "%s: unable to compute intensity measures for y=%d\n", proc, y2 ); \
      return( -1 );                                                                         \
    }                                                                                       \
    if ( ymin == 0 ) {                                                                      \
      a = meas2.intensityStddev / meas.intensityStddev;                                     \
      b = meas2.intensityMean - a * meas.intensityMean;                                     \
    }                                                                                       \
    else if ( ymax == (int)theIm->dim.y-1 )  {                                              \
      a = meas1.intensityStddev / meas.intensityStddev;                                     \
      b = meas1.intensityMean - a * meas.intensityMean;                                     \
    }                                                                                       \
    else {                                                                                  \
      c1 = (double)(y2-y) /  (double)(y2-y1);                                               \
      c2 = (double)(y-y1) /  (double)(y2-y1);                                               \
      a = ( c1*meas1.intensityStddev + c2*meas2.intensityStddev ) /  meas.intensityStddev;  \
      b = ( c1*meas1.intensityMean + c2*meas2.intensityMean ) - a * meas.intensityMean;     \
    }                                                                                       \
    for ( z=0; z<(int)theIm->dim.z; z++ )                                                   \
    for ( x=0; x<(int)theIm->dim.x; x++ ) {                                                 \
      value = a*(double)theBuf[z][y][x] + b;                                                \
      resBuf[z][y][x] = (int)( value + 0.5 );                                               \
    }                                                                                       \
  }                                                                                         \
}


static int _CorrectXZSliceRangeIntensityRescaling( vt_image *theIm, vt_image *resIm, int ymin, int ymax )
{
  char *proc = "_CorrectXZSliceRangeIntensityRescaling";
  int x, y, z;
  int y1 = ymin - 1;
  int y2 = ymax + 1;
  double c1, c2;
  double a, b;
  double value;
  localMeasures meas1, meas2, meas;

  _InitLocalMeasures( &meas1 );
  _InitLocalMeasures( &meas2 );
  _InitLocalMeasures( &meas );

  if ( y1 >= 0 ) {
    if ( _ComputeXZSliceIntensityLocalMeasures( theIm, &meas1, y1 ) != 0 ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to compute intensity measures for y=%d\n", proc, y1 );
      return( -1 );
    }
  }

  if ( y2 < (int)theIm->dim.y ) {
    if ( _ComputeXZSliceIntensityLocalMeasures( theIm, &meas2, y2 ) != 0 ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to compute intensity measures for y=%d\n", proc, y2 );
      return( -1 );
    }
  }
  
  switch( theIm->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled\n", proc );
    return( -1 );
    break;
  case SSHORT :
    XZSLICERANGEINTENSITYRESCALING( s16 )
    break;
  case USHORT :
    XZSLICERANGEINTENSITYRESCALING( u16 )
 }

 return( 0 );
}





typedef enum {
  _LINEAR_INTERPOLATION_,
  _INTENSITY_RESCALING_
} enumCorrectionType;



static int _CorrectXZSliceRange( vt_image *theIm, vt_image *resIm, int ymin, int ymax )
{
  char *proc = "_CorrectXZSliceRange";
  enumCorrectionType correctionType = _INTENSITY_RESCALING_;

  if ( ymin <= 0 && ymax >= (int)theIm->dim.y-1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: range was [%d-%d], covers the whole image\n", proc, ymin, ymax );
    return( -1 );
  }


  switch ( correctionType ) {
  default : 
  case _LINEAR_INTERPOLATION_ :
    return( _CorrectXZSliceRangeLinearInterpolation( theIm, resIm, ymin, ymax ) );
    break;
  case _INTENSITY_RESCALING_ :
    return( _CorrectXZSliceRangeIntensityRescaling( theIm, resIm, ymin, ymax ) );
    break;
  }
  
  if ( _verbose_ )
    fprintf( stderr, "%s: correction type not handled in switch\n", proc );
  return( -1 );
}





/************************************************************
 *
 *
 *
 ************************************************************/

int VP_RemoveLines( vt_image *theIm, vt_image *resIm, double coefficient )
{
  char *proc = "VP_RemoveLines";
  localMeasuresList slist;
  int i, y, ymin, ymax;
  double criteriaThreshold;
  int nslices;

  if ( VT_CopyImage( theIm, resIm ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to copy image\n", proc );
    return( -1 );
  }



  /* localMeasures 
   */

  _InitLocalMeasuresList( &slist );
  
  if ( _AllocateLocalMeasuresList( &slist, theIm->dim.y ) != 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate localMeasures\n", proc );
    return( -1 );
  }

  if ( 1 ) {
    if ( _ComputeImageContrastLocalMeasures( theIm, &slist ) != 0 ) {
      _FreeLocalMeasuresList( &slist );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to compute contrast measures\n", proc );
      return( -1 );
    }
  }

  
  if ( 0 ) {
    fprintf( stdout, "\n" );
    fprintf( stdout, "Selection of data\n" );
    _PrintLocalMeasuresListSelectedData( stdout, &slist );
    fprintf( stdout, "\n" );
  }



  _SortLocalMeasuresWRTmaxContrastPos( &slist );

  if ( 0 ) {
    fprintf( stdout, "\n" );
    fprintf( stdout, "First sorted data\n" );
    _PrintLocalMeasuresListFirstPointer( stdout, &slist );
    fprintf( stdout, "\n" );
  }

  
  




  /* selection
   */
  criteriaThreshold = (double)(theIm->dim.x * theIm->dim.z) * coefficient;
  for ( nslices=0, i=0; i<slist.n_allocated_data; i++ ) {
    if ( slist.data[i].contrastPos1 > criteriaThreshold ||
	 slist.data[i].contrastPos2 > criteriaThreshold ) {
      slist.data[i].flag = 1;
      nslices ++;
    }
    else
      slist.data[i].flag = 0;
  }
  
  if ( _verbose_ >= 2 ) {
    fprintf( stdout, "\n" );
    fprintf( stdout, "Flagged XZ slices\n" );
    _PrintLocalMeasuresListFlaggedData( stdout, &slist );
    fprintf( stdout, "\n" );
  }

  fprintf( stdout, "... %d slices are tagged for rescaling\n", nslices );

  /* removal processing 
   */


  for ( y=0; y<(int)theIm->dim.y; y++ ) {
    if ( slist.data[y].flag == 0 ) continue;
    ymin = ymax = y;
    while ( slist.data[ymax].flag && ymax < (int)theIm->dim.y-1 ) ymax ++;
    if ( ymax < (int)theIm->dim.y-1 ) {
      ymax --;
    }
    else if ( ymax == (int)theIm->dim.y-1 && ! slist.data[ymax].flag ) {
      ymax --;
    }
    fprintf( stdout, "... processing xz slices [%4d - %4d]\n",
	     ymin, ymax );
    
    if ( _CorrectXZSliceRange( theIm, resIm, ymin, ymax ) != 0 ) {
      _FreeLocalMeasuresList( &slist );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to correct slice range [%d - %d]\n", proc, ymin,  ymax );
      return( -1 );
    }


    y = ymax;
  }
    



  _FreeLocalMeasuresList( &slist );

  return( 0 );
}
