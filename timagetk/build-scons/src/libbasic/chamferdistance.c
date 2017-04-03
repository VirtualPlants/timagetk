/*************************************************************************
 * chamferdistance.c - computation of chamfer distances
 *
 * $Id$
 *
 * Copyright (c) INRIA 2008
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Tue Oct  7 15:31:55 CEST 2008
 *
 * ADDITIONS, CHANGES
 *
 * - 
 *
 *
 *
 *
 */

#include <pixel-operation.h>
#include <chamferdistance.h>



static int _verbose_ = 1;

void setVerboseInChamferDistance( int v )
{
  _verbose_ = v;
}

void incrementVerboseInChamferDistance(  )
{
  _verbose_ ++;
}

void decrementVerboseInChamferDistance(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





/*------------------------------------------------------------
 *
 * Chamfer masks stuff
 *
 * To be done:
 * - build predefined masks
 *
 *------------------------------------------------------------*/


void initChamferMask( typeChamferMask *theMask )
{
  theMask->nb = 0;
  theMask->list = NULL;
  theMask->normalizationfactor = -1;
}



void freeChamferMask( typeChamferMask *theMask )
{
  if ( theMask->list != NULL )
    free( theMask->list );
  initChamferMask( theMask );
}



int allocChamferMask( typeChamferMask *theMask, int n )
{
  char *proc = "allocChamferMask";
  
  if ( n <= 0 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: invalid number of mask points\n", proc );
      return( -1 );
    }
  }
  
  if ( theMask->list != NULL || theMask->nb > 0 )
    free( theMask->list );

  theMask->list = (typeChamferPoint*)malloc( n * sizeof(typeChamferPoint) );
  theMask->nb = n;
  theMask->normalizationfactor = -1;
  return( 1 );
}



static int compareChamferPoints( typeChamferPoint *pt1, typeChamferPoint *pt2 )
{
  if ( pt1->z < pt2->z ) return( -1 );
  if ( pt1->z == pt2->z ) {
    if ( pt1->y < pt2->y ) return( -1 );
    if ( pt1->y == pt2->y ) {
      if ( pt1->x < pt2->x ) return( -1 );
      if ( pt1->x == pt2->x ) return( 0 );
      return( 1 );
    }
    return( 1 );
  }
  return( 1 );
}



static void sortChamferPoints( typeChamferPoint *tab, int left, int right )
{
  typeChamferPoint tmp;
  int i, last;

  if ( left >= right ) return;
  
  tmp = tab[left];   tab[left] = tab[(left+right)/2];   tab[(left+right)/2] = tmp;
  
  last = left;
  for ( i = left+1; i <= right; i++ )       
    if ( compareChamferPoints( &tab[i], &tab[left] ) < 0 ) {
      tmp = tab[++last];   tab[last] = tab[i];   tab[i] = tmp;
    }

  tmp = tab[left];   tab[left] = tab[last];   tab[last] = tmp;
  
  sortChamferPoints( tab, left, last-1 );
  sortChamferPoints( tab, last+1, right );

}



void sortChamferMask( typeChamferMask* theMask )
{
  sortChamferPoints( theMask->list, 0, theMask->nb-1 );
}



int getChamferMaskMin( typeChamferMask* theMask )
{
  int i;
  int min = 0;

  for ( i=0; i<theMask->nb; i++ ) {
    if ( min == 0 && theMask->list[i].inc > 0 ) 
      min = theMask->list[i].inc;
    if ( min > theMask->list[i].inc && theMask->list[i].inc > 0 ) 
      min = theMask->list[i].inc;
  }
  
  return( min );
}



int getForwardChamferMask( typeChamferMask* theMask, typeChamferMask* theForwardMask )
{
  char *proc = "getForwardChamferMask";
  typeChamferPoint center = {0,0,0,0,0};
  int i, n;

  freeChamferMask( theForwardMask );

  if ( allocChamferMask( theForwardMask, theMask->nb ) <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating\n", proc );
    return( -1 );
  }
  
  for ( n=0, i=0; i<theMask->nb; i++ ) {
    if ( compareChamferPoints( &(theMask->list[i]), &center ) < 0 ) {
      theForwardMask->list[n] = theMask->list[i];
      n++;
    }
  }

  theForwardMask->nb = n;

  return( 1 );
}



int getBackwardChamferMask( typeChamferMask* theMask, typeChamferMask* theBackwardMask )
{
  char *proc = "getBackwardChamferMask";
  typeChamferPoint center = {0,0,0,0,0};
  int i, n;

  freeChamferMask( theBackwardMask );

  if ( allocChamferMask( theBackwardMask, theMask->nb ) <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating\n", proc );
    return( -1 );
  }
  
  for ( n=0, i=0; i<theMask->nb; i++ ) {
    if ( compareChamferPoints( &(theMask->list[i]), &center ) > 0 ) {
      theBackwardMask->list[n] = theMask->list[i];
      n++;
    }
  }

  theBackwardMask->nb = n;

  return( 1 );
}




int printChamferMaskAsImage( FILE* fout, typeChamferMask* theMask )
{
  char *proc = "printChamferMaskAsImage";
  int i;
  int xmin = 0;
  int xmax = 0;
  int ymin = 0;
  int ymax = 0;
  int zmin = 0;
  int zmax = 0;
  int dimx, dimy, dimz;
  int cx, cy, cz;
  int *buf = NULL;
  typeChamferPoint *pt;
  int x, y, z;

  for ( i=0; i<theMask->nb; i++ ) {
    if ( xmin > theMask->list[i].x ) xmin = theMask->list[i].x;
    if ( xmax < theMask->list[i].x ) xmax = theMask->list[i].x;
    if ( ymin > theMask->list[i].y ) ymin = theMask->list[i].y;
    if ( ymax < theMask->list[i].y ) ymax = theMask->list[i].y;
    if ( zmin > theMask->list[i].z ) zmin = theMask->list[i].z;
    if ( zmax < theMask->list[i].z ) zmax = theMask->list[i].z;
  }

  cx = ( xmax >= (-xmin) ) ? xmax : (-xmin);
  cy = ( ymax >= (-ymin) ) ? ymax : (-ymin);
  cz = ( zmax >= (-zmin) ) ? zmax : (-zmin);
  dimx = 2*cx+1;
  dimy = 2*cy+1;
  dimz = 2*cz+1;

  buf = (int*)malloc(dimx*dimy*dimz*sizeof(int));
  
  if ( buf == (int*)NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to allocate buffer\n", proc );
    }
    return( -1 );
  }

  for ( i=0; i<dimx*dimy*dimz; i++) buf[i] = 0;

  for ( i=0; i<theMask->nb; i++ ) {
    pt = &(theMask->list[i]);
    buf[ pt->x+cx + dimx * ( pt->y+cy + dimy * ( pt->z + cz ) ) ] =  pt->inc;
  }

  fprintf( fout, "\n" );

  for ( i=0, z=0; z<dimz; z++ ) {
    for ( y=0; y<dimy; y++ ) {
      for ( x=0; x<dimx; x++, i++ ) {
         if ( buf[i] != 0 )
           fprintf( fout, " %3d", buf[i] );
         else
           fprintf( fout, "   ." );
      }
      fprintf( fout, "\n" );
    }
    fprintf( fout, "\n" );
  }

  free( buf );
  return( 1 );
}





void printChamferMaskAsList( FILE* fout, typeChamferMask* theMask )
{
  int i;
  typeChamferPoint *pt;

  if ( 0 ) sortChamferMask ( theMask );

  for ( i=0; i<theMask->nb; i++ ) {
    pt = &(theMask->list[i]);
    fprintf( fout, "#%3d: (%3d, %3d, %3d) = %3d\n", i+1, pt->x, pt->y, pt->z, pt->inc );
  }
}





/*------------------------------------------------------------
 *
 * build masks
 *
 ------------------------------------------------------------*/


int buildChamferMaskFromImage( void *theBuf,
                                  bufferType type,
                                  int *theDim,
                                  typeChamferMask *theMask )
{
  char *proc = "buildChamferMaskFromImage";

  int i, x, y, z;
  int n;

  freeChamferMask( theMask );

  n = theDim[0]*theDim[1]*theDim[2];
  if ( allocChamferMask( theMask, n ) <= 0 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to allocate chamfer mask\n", proc );
      return( -1 );
    }
  }
  
  n = 0;
  switch ( type ) {
    
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such type not handled yet\n", proc );
    return( -1 );

  case UCHAR :
    {
      u8 *buf = (u8*)theBuf;
      for ( i=0, z=0; z<theDim[2]; z++ )
      for ( y=0; y<theDim[1]; y++ )
      for ( x=0; x<theDim[0]; x++, i++ ) {
         if ( buf[i] > 0 ) {
           theMask->list[n].x = x;
           theMask->list[n].y = y;
           theMask->list[n].z = z;
           theMask->list[n].inc = buf[i];
           n++;
         }
      }
    }
    break;

  } /* end of switch( type ) */

  /* the mask center is assumed to be the buffer center
   */
  for ( i=0; i<n; i++ ) {
    theMask->list[i].x -= (int)((theDim[0]-1)/2);
    theMask->list[i].y -= (int)((theDim[1]-1)/2);
    theMask->list[i].z -= (int)((theDim[2]-1)/2);
  }

  theMask->nb = n;

  return( 1 );
}






static void _set01neighbors( typeChamferMask *theMask, int inc )
{
  int i, j;
  
  for ( j = -1; j <= 1; j++ )
  for ( i = -1; i <= 1; i++ ) {

    if ( (i != 0 && j == 0 ) ||
          (i == 0 && j != 0 ) ) {

      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = 0;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}

static void _set11neighbors( typeChamferMask *theMask, int inc )
{
  int i, j;
  
  for ( j = -1; j <= 1; j++ )
  for ( i = -1; i <= 1; i++ ) {
    
    if ( i == 0 || j == 0 ) 
      continue;
    
      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = 0;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
  }
}

static void _set12neighbors( typeChamferMask *theMask, int inc )
{
  int i, j;
  
  for ( j = -2; j <= 2; j++ )
  for ( i = -2; i <= 2; i++ ) {
    
    if ( i == 0 || j == 0 ) 
      continue;

    if ( ( (i == -2 || i == 2) && (j == -1 || j == 1) ) ||
          ( (j == -2 || j == 2) && (i == -1 || i == 1) ) ) {
      
      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = 0;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}

static void _set001neighbors( typeChamferMask *theMask, int inc )
{
  int i, j, k;
  
  for ( k = -1; k <= 1; k++ )
  for ( j = -1; j <= 1; j++ )
  for ( i = -1; i <= 1; i++ ) {
    
    if ( (i != 0 && j == 0 && k == 0 ) ||
          (i == 0 && j != 0 && k == 0 ) ||
          (i == 0 && j == 0 && k != 0 ) ) {

      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = k;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}

static void _set011neighbors( typeChamferMask *theMask, int inc )
{
  int i, j, k;
  
  for ( k = -1; k <= 1; k++ )
  for ( j = -1; j <= 1; j++ )
  for ( i = -1; i <= 1; i++ ) {
    
    if ( (i != 0 && j != 0 && k == 0 ) ||
          (i != 0 && j == 0 && k != 0 ) ||
          (i == 0 && j != 0 && k != 0 ) ) {
      
      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = k;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}

static void _set111neighbors( typeChamferMask *theMask, int inc )
{
  int i, j, k;
  
  for ( k = -1; k <= 1; k++ )
  for ( j = -1; j <= 1; j++ )
  for ( i = -1; i <= 1; i++ ) {
    
    if ( (i != 0 && j != 0 && k != 0 ) ) {
      
      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = k;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}

static void _set012neighbors( typeChamferMask *theMask, int inc )
{
  int i, j, k;
  
  for ( k = -2; k <= 2; k++ )
  for ( j = -2; j <= 2; j++ )
  for ( i = -2; i <= 2; i++ ) {
    
    if ( (abs(i) == 0 && ((abs(j) == 1 && abs(k) == 2)||(abs(k) == 1 && abs(j) == 2))) ||
          (abs(j) == 0 && ((abs(k) == 1 && abs(i) == 2)||(abs(i) == 1 && abs(k) == 2))) ||
          (abs(k) == 0 && ((abs(i) == 1 && abs(j) == 2)||(abs(j) == 1 && abs(i) == 2))) ) {
      
      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = k;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}

static void _set112neighbors( typeChamferMask *theMask, int inc )
{
  int i, j, k;
  
  for ( k = -2; k <= 2; k++ )
  for ( j = -2; j <= 2; j++ )
  for ( i = -2; i <= 2; i++ ) {
    
    if ( k == 0 || j == 0 || i == 0 ) 
      continue;
    
    if ( (abs(i) == 2 && abs(j) == 1 && abs(k) == 1) ||
          (abs(i) == 1 && abs(j) == 2 && abs(k) == 1) ||
          (abs(i) == 1 && abs(j) == 1 && abs(k) == 2) ) {
      
      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = k;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}

static void _set122neighbors( typeChamferMask *theMask, int inc )
{
  int i, j, k;
  
  for ( k = -2; k <= 2; k++ )
  for ( j = -2; j <= 2; j++ )
  for ( i = -2; i <= 2; i++ ) {
    
    if ( k == 0 || j == 0 || i == 0 ) 
      continue;
    
    if ( (abs(i) == 2 && abs(j) == 2 && abs(k) == 1) ||
          (abs(i) == 1 && abs(j) == 2 && abs(k) == 2) ||
          (abs(i) == 2 && abs(j) == 1 && abs(k) == 2) ) {

      theMask->list[theMask->nb].x = i;
      theMask->list[theMask->nb].y = j;
      theMask->list[theMask->nb].z = k;
      theMask->list[theMask->nb].inc = inc;
      theMask->nb ++;
    }
  }
}


int buildPredefinedChamferMask( enumDistance type, typeChamferMask *theMask )
{
  char *proc = "buildPredefinedChamferMask";
  int n = 0;

  freeChamferMask( theMask );

  switch( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such distance type not handled yet\n", proc );
    return( -1 );
    
  case _DISTANCE04_ :
  case _DISTANCE08_ :
  case _CHAMFER3x3_ :
    n = 9;
    break;
  case _CHAMFER5x5_ :
    n = 25;
    break;
  case _DISTANCE06_ :
  case _DISTANCE10_ :
  case _DISTANCE18_ :
  case _DISTANCE26_ :
  case _CHAMFER3x3x3_ :
    n = 27;
    break;
  case _CHAMFER5x5x5_ :
    n = 125;
    break;
  }
  
  if ( allocChamferMask( theMask, n ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }

  theMask->nb = 0;
  
  switch( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such distance type not handled yet\n", proc );
    return( -1 );

  case _DISTANCE08_ :
    _set11neighbors( theMask, 1 );
  case _DISTANCE04_ :
    _set01neighbors( theMask, 1 );
    break;

  case _CHAMFER3x3_ :
    _set11neighbors( theMask, 4 );
    _set01neighbors( theMask, 3 );
    break;

  case _CHAMFER5x5_ :
    _set12neighbors( theMask, 11 );
    _set11neighbors( theMask, 7 );
    _set01neighbors( theMask, 5 );
    break;

  case _DISTANCE26_ :    
    _set111neighbors( theMask, 1 );
  case _DISTANCE18_ :
    _set011neighbors( theMask, 1 );
  case _DISTANCE06_ :
    _set001neighbors( theMask, 1 );
    break;

  case _DISTANCE10_ :
    _set11neighbors( theMask, 1 );
    _set001neighbors( theMask, 1 );
    break;

  case _CHAMFER3x3x3_ :
    _set111neighbors( theMask, 5 );
    _set011neighbors( theMask, 4 );
    _set001neighbors( theMask, 3 );
    break;

  case _CHAMFER5x5x5_ :
    _set122neighbors( theMask, 15 );
    _set112neighbors( theMask, 12 );
    _set012neighbors( theMask, 11 );
    _set111neighbors( theMask, 9 );
    _set011neighbors( theMask, 7 );
    _set001neighbors( theMask, 5 );
    break;
  }

  if ( theMask->nb >= n ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: such weird error occurs ...\n", proc );
    freeChamferMask( theMask );
    return( -1 );
  }

  return( 1 );
}




int buildShapeBasedInterpolationMasks( typeChamferMask* theMask, typeChamferMask* resInitMask, typeChamferMask* resMask )
{
  char *proc = "buildShapeBasedInterpolationMasks";
  int i, d;

  freeChamferMask( resMask );
  freeChamferMask( resInitMask );
  
  if ( allocChamferMask( resMask, theMask->nb ) <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating result mask\n", proc );
    return( -1 );
  }

  if ( allocChamferMask( resInitMask, theMask->nb ) <= 0 ) {
    freeChamferMask( resMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating initialization mask\n", proc );
    return( -1 );
  }
  
  
  /* I do not have really checked this
   */
  if ( theMask->normalizationfactor > 0.0 ) {
    resMask->normalizationfactor = 2.0 * theMask->normalizationfactor;
    resInitMask->normalizationfactor = theMask->normalizationfactor;
  }


  /* This should to be documented
   */
  for ( i=0; i<theMask->nb; i++ ) {
    resMask->list[i] = theMask->list[i];
    resInitMask->list[i] = theMask->list[i];
    
    resMask->list[i].inc *= 2;

    d = 0;
    if ( resMask->list[i].x > 0 ) d = resMask->list[i].x;
    else                          d = (-resMask->list[i].x);
    if ( resMask->list[i].y > 0 && resMask->list[i].y > d ) 
      d = resMask->list[i].y;
    else if ( resMask->list[i].y < 0 && (-resMask->list[i].y) > d )
      d = (-resMask->list[i].y);
    if ( resMask->list[i].z > 0 && resMask->list[i].z > d ) 
      d = resMask->list[i].z;
    else if ( resMask->list[i].z < 0 && (-resMask->list[i].z) > d )
      d = (-resMask->list[i].z);

    resInitMask->list[i].inc = (int)( (2*d-1)*(double)resMask->list[i].inc/(2*d) + 0.5 );
    
  }

  return( 1 );
}





/*------------------------------------------------------------
 *
 * Positive Chamfer Map
 *
 *------------------------------------------------------------*/


#define _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_ {                \
    if ( theDist[i] == 0 ) continue;                                     \
    updatedValue = theDist[i];                                           \
    for ( n=0; n<theMask->nb; n++ ) {                                    \
      if ( 0 <= x+thePts[n].x && x+thePts[n].x < dimx                    \
            && 0 <= y+thePts[n].y && y+thePts[n].y < dimy                \
            && 0 <= z+thePts[n].z && z+thePts[n].z < dimz ) {            \
        currentValue = theDist[i+thePts[n].o] + thePts[n].inc;           \
        if ( updatedValue > currentValue ) updatedValue = currentValue;  \
      }                                                                  \
    }                                                                    \
    theDist[i] = updatedValue;                                           \
}

#define _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITHOUT_TESTS_ {             \
    if ( theDist[i] == 0 ) continue;                                     \
    updatedValue = theDist[i];                                           \
    for ( n=0; n<theMask->nb; n++ ) {                                    \
        currentValue = theDist[i+thePts[n].o] + thePts[n].inc;           \
        if ( updatedValue > currentValue ) updatedValue = currentValue;  \
      }                                                                  \
    theDist[i] = updatedValue;                                           \
}

#define _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_ {                    \
    if ( theDist[i] == 0 ) continue;                                     \
    updatedValue = theDist[i];                                           \
    for ( n=0; n<theMask->nb; n++ ) {                                    \
      if ( 0 <= x+thePts[n].x && x+thePts[n].x < dimx                    \
            && 0 <= y+thePts[n].y && y+thePts[n].y < dimy                \
            && 0 <= z+thePts[n].z && z+thePts[n].z < dimz ) {            \
        currentValue = theDist[i+thePts[n].o] + thePts[n].inc;           \
        if ( updatedValue > currentValue ) {                             \
          updatedValue = currentValue;                                   \
          updatedLabel = theLabel[i+thePts[n].o];                        \
        }                                                                \
      }                                                                  \
    }                                                                    \
    theDist[i] = updatedValue;                                           \
    theLabel[i] = updatedLabel;                                          \
}

#define _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITHOUT_TESTS_ {                 \
    if ( theDist[i] == 0 ) continue;                                     \
    updatedValue = theDist[i];                                           \
    for ( n=0; n<theMask->nb; n++ ) {                                    \
        currentValue = theDist[i+thePts[n].o] + thePts[n].inc;           \
        if ( updatedValue > currentValue ) {                             \
          updatedValue = currentValue;                                   \
          updatedLabel = theLabel[i+thePts[n].o];                        \
        }                                                                \
      }                                                                  \
    theDist[i] = updatedValue;                                           \
    theLabel[i] = updatedLabel;                                          \
}

#define _POSITIVE_NEIGHBORS_SKIZ_FORWARD_UPDATE_( TYPE ) {               \
  TYPE *theLabel = (TYPE*)bufLabel;                                      \
  for ( i=0, z=0; z<dimz; z++ ) {                                        \
    _insidez_ = ( ( 0 <= z+zmin ) && ( z+zmax < dimz ) );                \
    for ( y=0; y<dimy; y++ ) {                                           \
      _insidey_ = _insidez_ && ( ( 0 <= y+ymin ) && ( y+ymax < dimy ) ); \
      if ( (-xmin) >= dimx-xmax ) {                                      \
        for ( x=0; x<dimx; x++, i++ ) {                                  \
          _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                    \
        }                                                                \
      }                                                                  \
      else {                                                             \
        for ( x=0; x<(-xmin); x++, i++ ) {                               \
          _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                    \
        }                                                                \
        for ( x=(-xmin); x<dimx-xmax; x++, i++ ) {                       \
          if ( ! _insidey_ ) {                                           \
            _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                  \
          }                                                              \
          else {                                                         \
            _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITHOUT_TESTS_               \
          }                                                              \
        }                                                                \
        for ( x=dimx-xmax; x<dimx; x++, i++ ) {                          \
          _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                    \
        }                                                                \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}


#define _POSITIVE_NEIGHBORS_SKIZ_BACKWARD_UPDATE_( TYPE ) {              \
  TYPE *theLabel = (TYPE*)bufLabel;                                      \
  for ( i=dimx*dimy*dimz-1, z=dimz-1; z>=0; z-- ) {                      \
    _insidez_ = ( ( 0 <= z+zmin ) && ( z+zmax < dimz ) );                \
    for ( y=dimy-1; y>=0; y-- ) {                                        \
      _insidey_ = _insidez_ && ( ( 0 <= y+ymin ) && ( y+ymax < dimy ) ); \
      if ( dimx-xmax-1 < (-xmin) ) {                                     \
        for ( x=dimx-1; x>=0; x--, i-- ) {                               \
          _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                    \
        }                                                                \
      }                                                                  \
      else {                                                             \
        for ( x=dimx-1; x>=(dimx-xmax); x--, i-- ) {                     \
          _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                    \
        }                                                                \
        for ( x=dimx-xmax-1; x>=(-xmin); x--, i-- ) {                    \
          if ( ! _insidey_ ) {                                           \
            _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                  \
          }                                                              \
          else {                                                         \
            _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITHOUT_TESTS_               \
          }                                                              \
        }                                                                \
        for ( x=(-xmin)-1; x>=0; x--, i-- ) {                            \
          _POSITIVE_NEIGHBORS_SKIZ_UPDATE_WITH_TESTS_                    \
        }                                                                \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}


static int _PositiveForwardChamferLoop_u16( u16 *theDist,
                                             void *bufLabel,
                                             bufferType typeLabel,
                                             int *theDim,
                                             typeChamferMask *theMask )
{
  char *proc = "_PositiveForwardChamferLoop_u16";

  int n;

  typeChamferPoint* thePts;

  int xmin = 0;
  int xmax = 0;
  int ymin = 0; 
  int ymax = 0;
  int zmin = 0;
  int zmax = 0;
  
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];

  int x, y, z, i;

  int _insidez_;
  int _insidey_;

  int updatedValue, currentValue;
  int updatedLabel;




  if ( theMask == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: null pointer on mask\n", proc );
      return( -1 );
    }
  }
  
  thePts = theMask->list;
  
  for ( n=0; n<theMask->nb; n++ ) {
    if ( xmin > thePts[n].x ) xmin = thePts[n].x;
    if ( xmax < thePts[n].x ) xmax = thePts[n].x;
    if ( ymin > thePts[n].y ) ymin = thePts[n].y;
    if ( ymax < thePts[n].y ) ymax = thePts[n].y;
    if ( zmin > thePts[n].z ) zmin = thePts[n].z;
    if ( zmax < thePts[n].z ) zmax = thePts[n].z;
  }

  for ( n=0; n<theMask->nb; n++ ) {
    thePts[n].o = thePts[n].z * dimx*dimy + thePts[n].y * dimx + thePts[n].x;
  }

  /* distance only case
   */
  if ( bufLabel == (void*)NULL ) {
    for ( i=0, z=0; z<dimz; z++ ) {
      _insidez_ = ( ( 0 <= z+zmin ) && ( z+zmax < dimz ) );
      for ( y=0; y<dimy; y++ ) {
        _insidey_ = _insidez_ && ( ( 0 <= y+ymin ) && ( y+ymax < dimy ) );
        if ( (-xmin) >= dimx-xmax ) {
          for ( x=0; x<dimx; x++, i++ ) {
            _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
          }
        }
        else {
          for ( x=0; x<(-xmin); x++, i++ ) {
            _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
          }
          for ( x=(-xmin); x<dimx-xmax; x++, i++ ) {
            if ( ! _insidey_ ) {
              _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
            }
            else {
              _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITHOUT_TESTS_
            }
          }
          for ( x=dimx-xmax; x<dimx; x++, i++ ) {
            _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
          }
        }
      }
    }
    return( 1 );
  }

  /* skiz case
   */
  switch( typeLabel ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such label image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _POSITIVE_NEIGHBORS_SKIZ_FORWARD_UPDATE_( u8 );
    break;
  case SSHORT :
    _POSITIVE_NEIGHBORS_SKIZ_FORWARD_UPDATE_( s16 );
    break;
  case USHORT :
    _POSITIVE_NEIGHBORS_SKIZ_FORWARD_UPDATE_( u16 );
    break;
  }
  return( 1 );
}





static int _PositiveBackwardChamferLoop_u16( u16 *theDist,
                                              void *bufLabel,
                                              bufferType typeLabel,
                                              int *theDim,
                                              typeChamferMask *theMask )
{
  char *proc = "_PositiveBackwardChamferLoop_u16";

  int n;

  typeChamferPoint* thePts;

  int xmin = 0;
  int xmax = 0;
  int ymin = 0; 
  int ymax = 0;
  int zmin = 0;
  int zmax = 0;
  
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];

  int x, y, z, i;

  int _insidez_;
  int _insidey_;

  int updatedValue, currentValue;
  int updatedLabel;





  if ( theMask == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: null pointer on mask\n", proc );
      return( -1 );
    }
  }
  
  thePts = theMask->list;
  
  for ( n=0; n<theMask->nb; n++ ) {
    if ( xmin > thePts[n].x ) xmin = thePts[n].x;
    if ( xmax < thePts[n].x ) xmax = thePts[n].x;
    if ( ymin > thePts[n].y ) ymin = thePts[n].y;
    if ( ymax < thePts[n].y ) ymax = thePts[n].y;
    if ( zmin > thePts[n].z ) zmin = thePts[n].z;
    if ( zmax < thePts[n].z ) zmax = thePts[n].z;
  }

  for ( n=0; n<theMask->nb; n++ ) {
    thePts[n].o = thePts[n].z * dimx*dimy + thePts[n].y * dimx + thePts[n].x;
  }

  /* distance only case
   */
  if ( bufLabel == (void*)NULL ) {
    for ( i=dimx*dimy*dimz-1, z=dimz-1; z>=0; z-- ) {
      _insidez_ = ( ( 0 <= z+zmin ) && ( z+zmax < dimz ) );
      for ( y=dimy-1; y>=0; y-- ) {
        _insidey_ = _insidez_ && ( ( 0 <= y+ymin ) && ( y+ymax < dimy ) );
        if ( dimx-xmax-1 < (-xmin) ) {
          for ( x=dimx-1; x>=0; x--, i-- ) {
            _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
          }
        }
        else {
          for ( x=dimx-1; x>=(dimx-xmax); x--, i-- ) {
            _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
          }
          for ( x=dimx-xmax-1; x>=(-xmin); x--, i-- ) {
            if ( ! _insidey_ ) {
              _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
            }
            else {
              _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITHOUT_TESTS_
            }
          }
          for ( x=(-xmin)-1; x>=0; x--, i-- ) {
            _POSITIVE_NEIGHBORS_DISTANCE_UPDATE_WITH_TESTS_
          }
        }
      }
    }
    return( 1 );
  }


  /* skiz case
   */
  switch( typeLabel ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such label image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _POSITIVE_NEIGHBORS_SKIZ_BACKWARD_UPDATE_( u8 );
    break;
  case SSHORT :
    _POSITIVE_NEIGHBORS_SKIZ_BACKWARD_UPDATE_( s16 );
    break;
  case USHORT :
    _POSITIVE_NEIGHBORS_SKIZ_BACKWARD_UPDATE_( u16 );
    break;
  }
  return( 1 );
}



static int _InitializePositiveChamferMap( void *inputBuf,
                                     bufferType typeIn,
                                     void *outputBuf,
                                     bufferType typeOut,
                                     int *theDim,
                                     double threshold )
{
  char *proc = "_InitializePositiveChamferMap";
  int iThreshold = 0;
  int i, v=theDim[0]*theDim[1]*theDim[2];
  
  if ( threshold >= 0.0 ) iThreshold = (int)(threshold + 0.5);
  else                    iThreshold = (int)(threshold - 0.5);


  switch ( typeOut ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such output type is not handled yet\n", proc );
    break;
    
  case USHORT :
    {
      unsigned short int infinity = 65535;
      unsigned short int *resBuf = (unsigned short int *)outputBuf;
  
      switch ( typeIn ) {

      default :
         if ( _verbose_ )
           fprintf( stderr, "%s: such input type is not handled yet\n", proc );
         break;

      case UCHAR :
         {
           unsigned char *theBuf = (unsigned char *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? infinity : 0;
         }
         break;
      case SCHAR :
         {
           char *theBuf = (char *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? infinity : 0;
         }
         break;
      case USHORT :
         {
           unsigned short int *theBuf = (unsigned short int *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? infinity : 0;
         }
         break;
      case SSHORT :
         {
           short int *theBuf = (short int *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? infinity : 0;
         }
         break;
      case FLOAT :
         {
           float *theBuf = (float *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < threshold ) ? infinity : 0;
         }
         break;

      }
 
    } /* end of case USHORT for typeOut */
    break;

  }

  return( 1 );
}




/* compute a chamfer distance
 * theDist has been initialized with 0, infinity
 * if given, theLabel is used for a skiz computation
 */
static int _ComputePositiveChamferMapInInitializedImage( void *theDist,
                                                         bufferType typeDist,
                                                         void *theLabel,
                                                         bufferType typeLabel,
                                                         int *theDim,
                                                         typeChamferMask *theMask )
{
  char *proc = "_ComputePositiveChamferMapInInitializedImage";
  typeChamferMask theForwardMask;
  typeChamferMask theBackwardMask;

  initChamferMask( &theForwardMask );
  initChamferMask( &theBackwardMask );

  if ( getForwardChamferMask( theMask, &theForwardMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when extracting the forward mask\n", proc );
    return( -1 );
  }

  if ( getBackwardChamferMask( theMask, &theBackwardMask ) != 1 ) {
    freeChamferMask( &theForwardMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when extracting the backward mask\n", proc );
    return( -1 );
  }

  switch( typeDist ) {

  case USHORT :
    if ( _PositiveForwardChamferLoop_u16( (u16*)theDist,
                                          theLabel, typeLabel,
                                          theDim, &theForwardMask ) != 1 ) {
        freeChamferMask( &theBackwardMask );
        freeChamferMask( &theForwardMask );
        if ( _verbose_ )
            fprintf( stderr, "%s: error when performing the forward run\n", proc );
          return( -1 );
    }
    if ( _PositiveBackwardChamferLoop_u16( (u16*)theDist,
                                          theLabel, typeLabel,
                                          theDim, &theBackwardMask ) != 1 ) {
        freeChamferMask( &theBackwardMask );
        freeChamferMask( &theForwardMask );
        if ( _verbose_ )
            fprintf( stderr, "%s: error when performing the backward run\n", proc );
          return( -1 );
    }
    break;

  default :
    freeChamferMask( &theBackwardMask );
    freeChamferMask( &theForwardMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such type not handled yet\n", proc );
    return( -1 );
  }

  freeChamferMask( &theBackwardMask );
  freeChamferMask( &theForwardMask );

  return( 1 );
}




int _ComputePositiveChamferMap( void *inputBuf,
                                bufferType typeIn,
                                void *outputBuf,
                                bufferType typeOut,
                                int *theDim,
                                double threshold,
                                typeChamferMask *inputMask,
                                int _divide_ )
{
  char *proc = "_ComputePositiveChamferMap";
  typeChamferMask tmpMask;
  typeChamferMask *theMask = (typeChamferMask *)NULL;
  unsigned short int *tmpBuf = (unsigned short int *)NULL;
  int v = theDim[0]*theDim[1]*theDim[2];


  
  /* NULL input mask
   */

  initChamferMask( &tmpMask );
  if ( inputMask == (typeChamferMask *)NULL ) {
    if ( buildPredefinedChamferMask( _CHAMFER3x3x3_,  &tmpMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build default chamfer mask\n", proc );
      return( -1 );
    }
    theMask = &tmpMask;
  }
  else {
    theMask = inputMask;
  }


  
  /* auxiliary buffer allocation
   */
  switch( typeOut ) {
  case USHORT :
  case SSHORT :
    tmpBuf = (unsigned short int *)outputBuf;
    break;
  default :
    tmpBuf = (unsigned short int *)malloc( v * sizeof(unsigned short int) );
    if ( tmpBuf == (unsigned short int *)NULL ) {
      if ( _verbose_)
         fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
      return( -1 );
    }
  }



  /* initialization
   */

  if ( _InitializePositiveChamferMap( inputBuf, typeIn, tmpBuf, USHORT, theDim, threshold ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to initialize distance map\n", proc );
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    return( -1 );
  }

  

  /* computation 
   */
  if ( _ComputePositiveChamferMapInInitializedImage( tmpBuf, USHORT,
                                                     (void*)NULL, TYPE_UNKNOWN,
                                                     theDim, theMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance map\n", proc );
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    return( -1 );
  }

  
  /* here, we divide by the minimal value of the chamfer mask
     to get a minimal distance of 1 in the result image
  */
  if ( _divide_ ) {
    if ( _NormalizePositiveChamferMap( (void*)tmpBuf, USHORT, theDim, theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to normalize distance map\n", proc );
      if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
      if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
      return( -1 );
    }
  }

  if ( ConvertBuffer( (void*)tmpBuf, USHORT, outputBuf, typeOut, v ) != 1 ) {
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    return( -1 );
  }

  if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
  if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
  return( 1 );

}





/*------------------------------------------------------------
 *
 * Chamfer map normalization
 *
 *------------------------------------------------------------*/

int _NormalizePositiveChamferMap( void *inputBuf,
                                      bufferType typeIn,
                                      int *theDim,
                                      typeChamferMask *theMask )
{
  char *proc = "_NormalizePositiveChamferMap";
  double normalizationfactor;
  int v = theDim[0]*theDim[1]*theDim[2];
  int i;

  /* ... */

  if ( theMask->normalizationfactor > 0.0 ) {
    normalizationfactor = theMask->normalizationfactor;
  }
  else {
    normalizationfactor = getChamferMaskMin( theMask );
  }
  if ( 0.99 < normalizationfactor && normalizationfactor < 1.01 ) { 
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: nothing to do\n", proc );
    return( 1 );
  }



  switch( typeIn ) {

  default :
    if ( divideImageDouble( inputBuf, typeIn, normalizationfactor,
                               inputBuf, typeIn, theDim ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: error when dividing\n", proc );
    }
    return( 1 );
      
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int*)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = (unsigned short int)( (double)*theBuf / normalizationfactor + 0.5 );
    }
    break;
    
  case SSHORT :
    {
      short int *theBuf = (short int*)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = (short int)( (double)*theBuf / normalizationfactor + 0.5 );
    }
    break;

  }

  return( 1 );
}






/*------------------------------------------------------------
 *
 * Chamfer maps using predefined masks
 *
 *------------------------------------------------------------*/



int Compute2DNormalizedChamfer3x3WithThreshold( void *inputBuf,
                                          bufferType typeIn,
                                          void *outputBuf,
                                          bufferType typeOut,
                                          int *theDim,
                                          double threshold )
{
  char *proc = "Compute2DNormalizedChamfer3x3WithThreshold";
  typeChamferMask theMask;
  int ret;
  
  initChamferMask( &theMask );
  if ( buildPredefinedChamferMask( _CHAMFER3x3_, &theMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to build chamfer mask\n", proc );
    return( -1 );
  }

  ret = _ComputePositiveChamferMap( inputBuf, typeIn, outputBuf, typeOut,
                                          theDim, threshold, &theMask, 1 );
  freeChamferMask( &theMask );
  return( ret );
}



int Compute2DNormalizedChamfer5x5WithThreshold( void *inputBuf,
                                          bufferType typeIn,
                                          void *outputBuf,
                                          bufferType typeOut,
                                          int *theDim,
                                          double threshold )
{
  char *proc = "Compute2DNormalizedChamfer5x5WithThreshold";
  typeChamferMask theMask;
  int ret;
  
  initChamferMask( &theMask );
  if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to build chamfer mask\n", proc );
    return( -1 );
  }

  ret = _ComputePositiveChamferMap( inputBuf, typeIn, outputBuf, typeOut,
                                          theDim, threshold, &theMask, 1 );
  freeChamferMask( &theMask );
  return( ret );
}



int Compute3DNormalizedChamfer3x3x3( void *inputBuf,
                              bufferType typeIn,
                              void *outputBuf,
                              bufferType typeOut,
                              int *theDim )
{
  char *proc = "Compute3DNormalizedChamfer3x3x3";
  typeChamferMask theMask;
  int ret;
  
  initChamferMask( &theMask );
  if ( buildPredefinedChamferMask( _CHAMFER3x3x3_, &theMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to build chamfer mask\n", proc );
    return( -1 );
  }

  ret = _ComputePositiveChamferMap( inputBuf, typeIn, outputBuf, typeOut,
                                          theDim, 1.0, &theMask, 1 );
  freeChamferMask( &theMask );
  return( ret );
}



int Compute3DNormalizedChamfer5x5x5( void *inputBuf,
                              bufferType typeIn,
                              void *outputBuf,
                              bufferType typeOut,
                              int *theDim )
{
  char *proc = "Compute3DNormalizedChamfer5x5x5";
  typeChamferMask theMask;
  int ret;
  
  initChamferMask( &theMask );
  if ( buildPredefinedChamferMask( _CHAMFER5x5x5_, &theMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to build chamfer mask\n", proc );
    return( -1 );
  }

  ret = _ComputePositiveChamferMap( inputBuf, typeIn, outputBuf, typeOut,
                                          theDim, 1.0, &theMask, 1 );
  freeChamferMask( &theMask );
  return( ret );
}










/*------------------------------------------------------------
 *
 * Negative / Positive Chamfer Map
 *    for shape-based interpolation
 *
 *------------------------------------------------------------*/

#define _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITH_TESTS_ {                        \
    if ( theBuf[i] == 0 ) continue;                                           \
    updatedValue = theBuf[i];                                                 \
    if ( theBuf[i] < 0 ) {                                                    \
      for ( n=0; n<theMask->nb; n++ ) {                                       \
        if ( 0 <= x+thePts[n].x && x+thePts[n].x < dimx                       \
              && 0 <= y+thePts[n].y && y+thePts[n].y < dimy                    \
              && 0 <= z+thePts[n].z && z+thePts[n].z < dimz ) {                \
          if ( theBuf[i+thePts[n].o] < 0 ) {                                  \
            currentValue = theBuf[i+thePts[n].o] - thePts[n].inc;             \
            if ( updatedValue < currentValue ) updatedValue = currentValue;   \
          }                                                                   \
        }                                                                     \
      }                                                                       \
    } else {                                                                  \
      for ( n=0; n<theMask->nb; n++ ) {                                       \
        if ( 0 <= x+thePts[n].x && x+thePts[n].x < dimx                       \
              && 0 <= y+thePts[n].y && y+thePts[n].y < dimy                    \
              && 0 <= z+thePts[n].z && z+thePts[n].z < dimz ) {                \
          if ( theBuf[i+thePts[n].o] > 0 ) {                                  \
            currentValue = theBuf[i+thePts[n].o] + thePts[n].inc;             \
            if ( updatedValue > currentValue ) updatedValue = currentValue;   \
          }                                                                   \
        }                                                                     \
      }                                                                       \
    }                                                                         \
    theBuf[i] = updatedValue;                                                 \
}

#define _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITHOUT_TESTS_ {                   \
    if ( theBuf[i] == 0 ) continue;                                         \
    updatedValue = theBuf[i];                                               \
    if ( theBuf[i] < 0 ) {                                                  \
      for ( n=0; n<theMask->nb; n++ ) {                                     \
        if ( theBuf[i+thePts[n].o] < 0 ) {                                  \
          currentValue = theBuf[i+thePts[n].o] - thePts[n].inc;             \
          if ( updatedValue < currentValue ) updatedValue = currentValue;   \
        }                                                                   \
      }                                                                     \
    } else {                                                                \
      for ( n=0; n<theMask->nb; n++ ) {                                     \
        if ( theBuf[i+thePts[n].o] > 0 ) {                                  \
          currentValue = theBuf[i+thePts[n].o] + thePts[n].inc;             \
          if ( updatedValue > currentValue ) updatedValue = currentValue;   \
        }                                                                   \
      }                                                                     \
    }                                                                                     \
    theBuf[i] = updatedValue;                                               \
  }



static void _PositiveNegativeForwardChamferLoop_s16( s16 *theBuf,
                                 int *theDim,
                                 typeChamferMask *theMask )
{
  char *proc = "_PositiveNegativeForwardChamferLoop_s16";

  int n;

  typeChamferPoint* thePts;

  int xmin = 0;
  int xmax = 0;
  int ymin = 0; 
  int ymax = 0;
  int zmin = 0;
  int zmax = 0;
  
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];

  int x, y, z, i;

  int _insidez_;
  int _insidey_;

  int updatedValue, currentValue;





  if ( theMask == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: null pointer on mask\n", proc );
      return;
    }
  }
  
  thePts = theMask->list;
  
  for ( n=0; n<theMask->nb; n++ ) {
    if ( xmin > thePts[n].x ) xmin = thePts[n].x;
    if ( xmax < thePts[n].x ) xmax = thePts[n].x;
    if ( ymin > thePts[n].y ) ymin = thePts[n].y;
    if ( ymax < thePts[n].y ) ymax = thePts[n].y;
    if ( zmin > thePts[n].z ) zmin = thePts[n].z;
    if ( zmax < thePts[n].z ) zmax = thePts[n].z;
  }

  for ( n=0; n<theMask->nb; n++ ) {
    thePts[n].o = thePts[n].z * dimx*dimy + thePts[n].y * dimx + thePts[n].x;
  }

  for ( i=0, z=0; z<dimz; z++ ) {
    _insidez_ = ( ( 0 <= z+zmin ) && ( z+zmax < dimz ) );
    for ( y=0; y<dimy; y++ ) {
      _insidey_ = _insidez_ && ( ( 0 <= y+ymin ) && ( y+ymax < dimy ) );

      for ( x=0; x<(-xmin); x++, i++ ) {
         _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITH_TESTS_
      }
      for ( x=(-xmin); x<dimx-xmax; x++, i++ ) {
         if ( ! _insidey_ ) {
           _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITH_TESTS_
         }
         else {
           _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITHOUT_TESTS_
         }
      }
      for ( x=dimx-xmax; x<dimx; x++, i++ ) {
         _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITH_TESTS_
      }
      
    }
  }

}





static void _PositiveNegativeBackwardChamferLoop_s16( s16 *theBuf,
                                 int *theDim,
                                 typeChamferMask *theMask )
{
  char *proc = "_PositiveNegativeBackwardChamferLoop_s16";

  int n;

  typeChamferPoint* thePts;

  int xmin = 0;
  int xmax = 0;
  int ymin = 0; 
  int ymax = 0;
  int zmin = 0;
  int zmax = 0;
  
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];

  int x, y, z, i;

  int _insidez_;
  int _insidey_;

  int updatedValue, currentValue;





  if ( theMask == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: null pointer on mask\n", proc );
      return;
    }
  }
  
  thePts = theMask->list;
  
  for ( n=0; n<theMask->nb; n++ ) {
    if ( xmin > thePts[n].x ) xmin = thePts[n].x;
    if ( xmax < thePts[n].x ) xmax = thePts[n].x;
    if ( ymin > thePts[n].y ) ymin = thePts[n].y;
    if ( ymax < thePts[n].y ) ymax = thePts[n].y;
    if ( zmin > thePts[n].z ) zmin = thePts[n].z;
    if ( zmax < thePts[n].z ) zmax = thePts[n].z;
  }

  for ( n=0; n<theMask->nb; n++ ) {
    thePts[n].o = thePts[n].z * dimx*dimy + thePts[n].y * dimx + thePts[n].x;
  }

  for ( i=dimx*dimy*dimz-1, z=dimz-1; z>=0; z-- ) {
    _insidez_ = ( ( 0 <= z+zmin ) && ( z+zmax < dimz ) );
    for ( y=dimy-1; y>=0; y-- ) {
      _insidey_ = _insidez_ && ( ( 0 <= y+ymin ) && ( y+ymax < dimy ) );

      for ( x=dimx-1; x>=(dimx-xmax); x--, i-- ) {
         _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITH_TESTS_
      }
      for ( x=dimx-xmax-1; x>=(-xmin); x--, i-- ) {
         if ( ! _insidey_ ) {
           _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITH_TESTS_
         }
         else {
           _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITHOUT_TESTS_
         }
      }
      for ( x=(-xmin)-1; x>=0; x--, i-- ) {
         _POSITIVE_NEGATIVE_NEIGHBORS_RUN_WITH_TESTS_
      }
      
    }
  }

}





static int _InitializePositiveNegativeChamferMap( void *inputBuf,
                                     bufferType typeIn,
                                     void *outputBuf,
                                     bufferType typeOut,
                                     int *theDim,
                                     double threshold )
{
  char *proc = "_InitializePositiveNegativeChamferMap";
  int iThreshold = 0;
  int i, v=theDim[0]*theDim[1]*theDim[2];
  
  if ( threshold >= 0.0 ) iThreshold = (int)(threshold + 0.5);
  else                    iThreshold = (int)(threshold - 0.5);


  switch ( typeOut ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such output type is not handled yet\n", proc );
    break;
    
  case SSHORT :
    {
      short int p_infinity =  32767;
      short int n_infinity = -32767;
      short int *resBuf = (short int *)outputBuf;
  
      switch ( typeIn ) {

      default :
         if ( _verbose_ )
           fprintf( stderr, "%s: such input type is not handled yet\n", proc );
         break;

      case UCHAR :
         {
           unsigned char *theBuf = (unsigned char *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? p_infinity : n_infinity;
         }
         break;
      case SCHAR :
         {
           char *theBuf = (char *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? p_infinity : n_infinity;
         }
         break;
      case USHORT :
         {
           unsigned short int *theBuf = (unsigned short int *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? p_infinity : n_infinity;
         }
         break;
      case SSHORT :
         {
           short int *theBuf = (short int *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < iThreshold ) ? p_infinity : n_infinity;
         }
         break;
      case FLOAT :
         {
           float *theBuf = (float *)inputBuf;
           for ( i=0; i<v; i++, theBuf++, resBuf++ )
             *resBuf = ( *theBuf < threshold ) ? p_infinity : n_infinity;
         }
         break;

      }
 
    } /* end of case SSHORT for typeOut */
    break;

  }

  return( 1 );
}


static void _InitializeBorderPositiveNegativeChamferMap( s16 *theBuf,
                                                               int *theDim,
                                                               typeChamferMask *theMask )
{
  char *proc = "_InitializeBorderPositiveNegativeChamferMap";

  typeChamferPoint* thePts;

  int xmin = 0;
  int xmax = 0;
  int ymin = 0; 
  int ymax = 0;
  int zmin = 0;
  int zmax = 0;
  
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  
  int x, y, z, i;
  int n;

  if ( theMask == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: null pointer on mask\n", proc );
      return;
    }
  }
  
  thePts = theMask->list;
  
  for ( n=0; n<theMask->nb; n++ ) {
    if ( xmin > thePts[n].x ) xmin = thePts[n].x;
    if ( xmax < thePts[n].x ) xmax = thePts[n].x;
    if ( ymin > thePts[n].y ) ymin = thePts[n].y;
    if ( ymax < thePts[n].y ) ymax = thePts[n].y;
    if ( zmin > thePts[n].z ) zmin = thePts[n].z;
    if ( zmax < thePts[n].z ) zmax = thePts[n].z;
  }

  for ( n=0; n<theMask->nb; n++ ) {
    thePts[n].o = thePts[n].z * dimx*dimy + thePts[n].y * dimx + thePts[n].x;
  }
  
  for ( i=0, z=0; z<dimz; z++ )
  for ( y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++, i++ ) {

    if ( theBuf[i] > 0 ) {
      for ( n=0; n<theMask->nb; n++ ) { 
         if ( 0 <= x+thePts[n].x && x+thePts[n].x < dimx
              && 0 <= y+thePts[n].y && y+thePts[n].y < dimy
              && 0 <= z+thePts[n].z && z+thePts[n].z < dimz ) {
           if ( theBuf[i+thePts[n].o] < 0 )
             if ( theBuf[i] > thePts[n].inc ) theBuf[i] = thePts[n].inc;
         }
      }
    }
    else {
      for ( n=0; n<theMask->nb; n++ ) { 
         if ( 0 <= x+thePts[n].x && x+thePts[n].x < dimx
              && 0 <= y+thePts[n].y && y+thePts[n].y < dimy
              && 0 <= z+thePts[n].z && z+thePts[n].z < dimz ) {
           if ( theBuf[i+thePts[n].o] > 0 )
             if ( theBuf[i] < (-thePts[n].inc) ) theBuf[i] = (-thePts[n].inc);
         }
      }
    }
    
  }
}





static int _ComputePositiveNegativeChamferMapInInitializedImage( void *theDist,
                                               bufferType typeDist,
                                               int *theDim,
                                               typeChamferMask *theMask )
{
  char *proc = "_ComputePositiveNegativeChamferMapInInitializedImage";
  typeChamferMask theForwardMask;
  typeChamferMask theBackwardMask;

  initChamferMask( &theForwardMask );
  initChamferMask( &theBackwardMask );

  if ( getForwardChamferMask( theMask, &theForwardMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when extracting the forward mask\n", proc );
    return( -1 );
  }

  if ( getBackwardChamferMask( theMask, &theBackwardMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when extracting the backward mask\n", proc );
    freeChamferMask( &theBackwardMask );
    return( -1 );
  }

  switch( typeDist ) {

  case SSHORT :
    _PositiveNegativeForwardChamferLoop_s16( (s16*)theDist, theDim, &theForwardMask );
    _PositiveNegativeBackwardChamferLoop_s16( (s16*)theDist, theDim, &theBackwardMask );
    break;

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such type not handled yet\n", proc );
    freeChamferMask( &theBackwardMask );
    freeChamferMask( &theForwardMask );
    return( -1 );
  }

  freeChamferMask( &theBackwardMask );
  freeChamferMask( &theForwardMask );

  return( 1 );
 
}




int _ComputePositiveNegativeChamferMap( void *inputBuf,
                                      bufferType typeIn,
                                      void *outputBuf,
                                      bufferType typeOut,
                                      int *theDim,
                                      double threshold,
                                      typeChamferMask *inputMask,
                                      int _divide_ )
{
  char *proc = "_ComputePositiveNegativeChamferMap";
  typeChamferMask tmpMask, *theMask;
  typeChamferMask resMask, resInitMask;
  short int *tmpBuf = (short int *)NULL;
  int v = theDim[0]*theDim[1]*theDim[2];

  
  /* NULL input mask
   */

  initChamferMask( &tmpMask );
  if ( inputMask == (typeChamferMask *)NULL ) {
    if ( buildPredefinedChamferMask( _CHAMFER3x3x3_,  &tmpMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build default chamfer mask\n", proc );
      return( -1 );
    }
    theMask = &tmpMask;
  }
  else {
    theMask = inputMask;
  }



  /* masks for shape based interpolation
   */
  initChamferMask( &resInitMask );
  initChamferMask( &resMask );
  if ( buildShapeBasedInterpolationMasks( theMask, &resInitMask, &resMask ) != 1 ) {
    freeChamferMask( &resInitMask );
    freeChamferMask( &resMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to build shape based interpolation masks\n", proc );
    return( -1 );
  }



  /* auxiliary buffer allocation
   */
  switch( typeOut ) {
  case USHORT :
  case SSHORT :
    tmpBuf = (short int *)outputBuf;
    break;
  default :
    tmpBuf = (short int *)malloc( v * sizeof(short int) );
    if ( tmpBuf == (short int *)NULL ) {
      if ( _verbose_)
         fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
      freeChamferMask( &resInitMask );
      freeChamferMask( &resMask );
      return( -1 );
    }
  }



  /* initialization
   */

  if ( _InitializePositiveNegativeChamferMap( inputBuf, typeIn, tmpBuf, SSHORT, theDim, threshold ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to initialize distance map\n", proc );
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    freeChamferMask( &resInitMask );
    freeChamferMask( &resMask );
    return( -1 );
  }

  _InitializeBorderPositiveNegativeChamferMap( tmpBuf, theDim, &resInitMask );




  /* computation 
   */
  if ( _ComputePositiveNegativeChamferMapInInitializedImage( tmpBuf, SSHORT, theDim, &resMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance map\n", proc );
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    freeChamferMask( &resInitMask );
    freeChamferMask( &resMask );
    return( -1 );
  }

  if ( _divide_ ) {
    if ( _NormalizePositiveChamferMap( (void*)tmpBuf, USHORT, theDim, &resInitMask ) != 1 ) {
      if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance map\n", proc );
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    freeChamferMask( &resInitMask );
    freeChamferMask( &resMask );
    return( -1 );
    }
  }

  freeChamferMask( &resInitMask );
  freeChamferMask( &resMask );

  if ( ConvertBuffer( (void*)tmpBuf, SSHORT, outputBuf, typeOut, v ) != 1 ) {
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    return( -1 );
  }

 
  if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
  if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );

  return( 1 );
}






/*------------------------------------------------------------
 *
 * SKIZ: computation of influence zones
 *
 *------------------------------------------------------------*/



int skizWithChamferDistance( void *labelBuf,
                                bufferType typeLabel,
                                void *distBuf,
                                bufferType typeDist,
                                int *theDim,
                                typeChamferMask *inputMask,
                                int _divide_ )
{
  char *proc = "skizWithChamferDistance";
  typeChamferMask tmpMask;
  typeChamferMask *theMask = (typeChamferMask *)NULL;
  unsigned short int *tmpBuf = (unsigned short int *)NULL;
  int v = theDim[0]*theDim[1]*theDim[2];



  /* NULL input mask
   */

  initChamferMask( &tmpMask );
  if ( inputMask == (typeChamferMask *)NULL ) {
    if ( buildPredefinedChamferMask( _CHAMFER3x3x3_,  &tmpMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build default chamfer mask\n", proc );
      return( -1 );
    }
    theMask = &tmpMask;
  }
  else {
    theMask = inputMask;
  }



  /* auxiliary buffer allocation
   */
  switch( typeDist ) {
  case USHORT :
  case SSHORT :
    if ( distBuf != (void*)NULL ) {
      tmpBuf = (unsigned short int *)distBuf;
      break;
    }
  default :
    tmpBuf = (unsigned short int *)malloc( v * sizeof(unsigned short int) );
    if ( tmpBuf == (unsigned short int *)NULL ) {
      if ( _verbose_)
         fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
      return( -1 );
    }
  }



  /* initialization from a label image
   */

  if ( _InitializePositiveChamferMap( labelBuf, typeLabel, tmpBuf, USHORT, theDim, 1.0 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to initialize distance map\n", proc );
    if ( tmpBuf != distBuf ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    return( -1 );
  }



  /* computation
   */
  if ( _ComputePositiveChamferMapInInitializedImage( tmpBuf, USHORT,
                                                     labelBuf, typeLabel,
                                                     theDim, theMask ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance map\n", proc );
    if ( tmpBuf != distBuf ) free( tmpBuf );
    if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
    return( -1 );
  }

  if ( distBuf != (void*)NULL ) {
      /* here, we divide by the minimal value of the chamfer mask
         to get a minimal distance of 1 in the result image
      */      if ( _divide_ ) {
        if ( _NormalizePositiveChamferMap( (void*)tmpBuf, USHORT, theDim, theMask ) != 1 ) {
          if ( _verbose_ )
             fprintf( stderr, "%s: unable to normalize distance map\n", proc );
          if ( tmpBuf != distBuf ) free( tmpBuf );
          if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
          return( -1 );
        }
      }
      if ( ConvertBuffer( (void*)tmpBuf, USHORT, distBuf, typeDist, v ) != 1 ) {
        if ( tmpBuf != distBuf ) free( tmpBuf );
        if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
        return( -1 );
      }
  }

  if ( tmpBuf != distBuf ) free( tmpBuf );
  if ( inputMask == (typeChamferMask *)NULL ) freeChamferMask( &tmpMask );
  return( 1 );

}









/*------------------------------------------------------------
 *
 * Mathematical Morphology Operations
 *
 *------------------------------------------------------------*/



int morphologicalDilationWithDistance( void *inputBuf,
                                           void *resultBuf,
                                           bufferType type,
                                           int *theDim,
                                           int radius, int dimension )
{
  char *proc = "morphologicalDilationWithDistance";
  unsigned short int *distBuf = (unsigned short int *)NULL;
  unsigned short int infinity = 65535;
  typeChamferMask theMask;
  int threshold = 1;
  int i, v=theDim[0]*theDim[1]*theDim[2];
  int distance_threshold;

  

  if ( radius <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative radius\n", proc );
    return( -1 );
   }

  /* allocation 
   */
  initChamferMask( &theMask );
  if ( theDim[2] == 1 || dimension == 2 ) {
    if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 2D chamfer mask\n", proc );
      return( -1 );
    }
  }
  else {
    if ( buildPredefinedChamferMask( _CHAMFER5x5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 3D chamfer mask\n", proc );
      return( -1 );
    }
  }
  distBuf = (unsigned short int *)malloc( v * sizeof( unsigned short int ) );
  if ( distBuf == (unsigned short int *)NULL ) {
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }



  /* initialisation 
   */
  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf < threshold ) ? infinity : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf < threshold ) ? infinity : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf < threshold ) ? infinity : 0;
    }
    break;
  }

  

  /* calcul de distance
   */
  if ( _ComputePositiveChamferMapInInitializedImage( distBuf, USHORT, 
                                                     (void*)NULL, TYPE_UNKNOWN,
                                                     theDim, &theMask ) != 1 ) {
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance\n", proc );
    return( -1 );
  }

  
  /* seuillage
   */
  distance_threshold = radius * getChamferMaskMin( &theMask );

  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] <= distance_threshold ) ? 255 : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] <= distance_threshold ) ? 65535 : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] <= distance_threshold ) ? 32767 : 0;
    }
    break;
  }

  free( distBuf );
  freeChamferMask( &theMask );

  return( 1 );
}





int morphologicalErosionWithDistance( void *inputBuf,
                                           void *resultBuf,
                                           bufferType type,
                                           int *theDim,
                                           int radius, int dimension )
{
  char *proc = "morphologicalErosionWithDistance";
  unsigned short int *distBuf = (unsigned short int *)NULL;
  unsigned short int infinity = 65535;
  typeChamferMask theMask;
  int threshold = 1;
  int i, v=theDim[0]*theDim[1]*theDim[2];
  int distance_threshold;

  

  if ( radius <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative radius\n", proc );
    return( -1 );
   }

  /* allocation 
   */
  initChamferMask( &theMask );
  if ( theDim[2] == 1 || dimension == 2 ) {
    if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 2D chamfer mask\n", proc );
      return( -1 );
    }
  }
  else {
    if ( buildPredefinedChamferMask( _CHAMFER5x5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 3D chamfer mask\n", proc );
      return( -1 );
    }
  }
  distBuf = (unsigned short int *)malloc( v * sizeof( unsigned short int ) );
  if ( distBuf == (unsigned short int *)NULL ) {
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }



  /* initialisation 
   */
  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf >= threshold ) ? infinity : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf >= threshold ) ? infinity : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf >= threshold ) ? infinity : 0;
    }
    break;
  }

  

  /* calcul de distance
   */
  if ( _ComputePositiveChamferMapInInitializedImage( distBuf, USHORT, 
                                                     (void*)NULL, TYPE_UNKNOWN,
                                                     theDim, &theMask ) != 1 ) {
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance\n", proc );
    return( -1 );
  }

  
  /* seuillage
   */
  distance_threshold = radius * getChamferMaskMin( &theMask );

  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] > distance_threshold ) ? 255 : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] > distance_threshold ) ? 65535 : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] > distance_threshold ) ? 32767 : 0;
    }
    break;
  }

  free( distBuf );
  freeChamferMask( &theMask );

  return( 1 );
}





int morphologicalClosingWithDistance( void *inputBuf,
                                          void *resultBuf,
                                          bufferType type,
                                          int *theDim,
                                          int radius, int dimension )
{
  char *proc = "morphologicalClosingWithDistance";
  unsigned short int *distBuf = (unsigned short int *)NULL;
  unsigned short int infinity = 65535;
  typeChamferMask theMask;
  int threshold = 1;
  int i, v=theDim[0]*theDim[1]*theDim[2];
  int distance_threshold;

  

  if ( radius <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative radius\n", proc );
    return( -1 );
   }

  /* allocation 
   */
  initChamferMask( &theMask );
  if ( theDim[2] == 1 || dimension == 2 ) {
    if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 2D chamfer mask\n", proc );
      return( -1 );
    }
  }
  else {
    if ( buildPredefinedChamferMask( _CHAMFER5x5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 3D chamfer mask\n", proc );
      return( -1 );
    }
  }
  distBuf = (unsigned short int *)malloc( v * sizeof( unsigned short int ) );
  if ( distBuf == (unsigned short int *)NULL ) {
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }



  /* initialisation 
   */
  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf < threshold ) ? infinity : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf < threshold ) ? infinity : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf < threshold ) ? infinity : 0;
    }
    break;
  }

  

  /* calcul de distance
   */
  if ( _ComputePositiveChamferMapInInitializedImage( distBuf, USHORT, 
                                                     (void*)NULL, TYPE_UNKNOWN,
                                                     theDim, &theMask ) != 1 ) {
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance (1)\n", proc );
    return( -1 );
  }


  
  /* seuillage
   */
  distance_threshold = radius * getChamferMaskMin( &theMask );
  for ( i=0; i<v; i++ )
    distBuf[i] = ( distBuf[i] <= distance_threshold ) ? infinity : 0;



  /* calcul de distance
   */
  if ( _ComputePositiveChamferMapInInitializedImage( distBuf, USHORT, 
                                                     (void*)NULL, TYPE_UNKNOWN,
                                                     theDim, &theMask ) != 1 ) {
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance (2)\n", proc );
    return( -1 );
  }

  

  /* seuillage
   */
  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] > distance_threshold ) ? 255 : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] > distance_threshold ) ? 65535 : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] > distance_threshold ) ? 32767 : 0;
    }
    break;
  }

  free( distBuf );
  freeChamferMask( &theMask );

  return( 1 );
}





int morphologicalOpeningWithDistance( void *inputBuf,
                                          void *resultBuf,
                                          bufferType type,
                                          int *theDim,
                                          int radius, int dimension )
{
  char *proc = "morphologicalOpeningWithDistance";
  unsigned short int *distBuf = (unsigned short int *)NULL;
  unsigned short int infinity = 65535;
  typeChamferMask theMask;
  int threshold = 1;
  int i, v=theDim[0]*theDim[1]*theDim[2];
  int distance_threshold;

  

  if ( radius <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: negative radius\n", proc );
    return( -1 );
   }

  /* allocation 
   */
  initChamferMask( &theMask );
  if ( theDim[2] == 1 || dimension == 2 ) {
    if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 2D chamfer mask\n", proc );
      return( -1 );
    }
  }
  else {
    if ( buildPredefinedChamferMask( _CHAMFER5x5x5_, &theMask ) != 1 ) {
      if ( _verbose_ )
         fprintf( stderr, "%s: unable to build 3D chamfer mask\n", proc );
      return( -1 );
    }
  }
  distBuf = (unsigned short int *)malloc( v * sizeof( unsigned short int ) );
  if ( distBuf == (unsigned short int *)NULL ) {
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }



  /* initialisation 
   */
  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf >= threshold ) ? infinity : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf >= threshold ) ? infinity : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)inputBuf;
      for ( i=0; i<v; i++, theBuf++ )
         distBuf[i] = ( *theBuf >= threshold ) ? infinity : 0;
    }
    break;
  }

  

  /* calcul de distance
   */
  if ( _ComputePositiveChamferMapInInitializedImage( distBuf, USHORT, 
                                                     (void*)NULL, TYPE_UNKNOWN,
                                                     theDim, &theMask ) != 1 ) {
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance (1)\n", proc );
    return( -1 );
  }


  
  /* seuillage
   */
  distance_threshold = radius * getChamferMaskMin( &theMask );
  for ( i=0; i<v; i++ )
    distBuf[i] = ( distBuf[i] <= distance_threshold ) ? infinity : 0;



  /* calcul de distance
   */
  if ( _ComputePositiveChamferMapInInitializedImage( distBuf, USHORT, 
                                                     (void*)NULL, TYPE_UNKNOWN,
                                                     theDim, &theMask ) != 1 ) {
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute distance (2)\n", proc );
    return( -1 );
  }

  

  /* seuillage
   */
  switch( type ) {
  default :
    free( distBuf );
    freeChamferMask( &theMask );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type is not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] <= distance_threshold ) ? 255 : 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] <= distance_threshold ) ? 65535 : 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)resultBuf;
      for ( i=0; i<v; i++, theBuf++ )
         *theBuf = ( distBuf[i] <= distance_threshold ) ? 32767 : 0;
    }
    break;
  }

  free( distBuf );
  freeChamferMask( &theMask );

  return( 1 );
}
