
#include <vt_morpho.h>

static int Neighborhood2Int ();

int BinaryDilation( vt_image *theIm,
		     vt_image *resIm,
		     Neighborhood N,
		     int iterations )
{
  int connectivity;
  int theDim[3];

  if ( (theIm->dim.x != resIm->dim.x) ||
       (theIm->dim.y != resIm->dim.y) ||
       (theIm->dim.z != resIm->dim.z) ||
       (theIm->type != resIm->type) )
    return ( -1 );

  
  connectivity = Neighborhood2Int( N );
  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  switch ( theIm->type ) {
  case UCHAR :
    BinaryDilation_u8( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  default :
    return( GreyLevelDilation( theIm, resIm, N, iterations ) );
  }

  return( 1 );
}






int BinaryErosion( vt_image *theIm,
		    vt_image *resIm,
		    Neighborhood N,
		    int iterations )
{
  int connectivity;
  int theDim[3];

  if ( (theIm->dim.x != resIm->dim.x) ||
       (theIm->dim.y != resIm->dim.y) ||
       (theIm->dim.z != resIm->dim.z) ||
       (theIm->type != resIm->type) )
    return ( -1 );
  
  if ( theIm->type != UCHAR ) 
    return( GreyLevelErosion( theIm, resIm, N, iterations ) );
  
  connectivity = Neighborhood2Int( N );
  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  switch ( theIm->type ) {
  case UCHAR :
    BinaryErosion_u8( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  default :
    return( GreyLevelErosion( theIm, resIm, N, iterations ) );
  }

  return( 1 );
}






int GreyLevelDilation( vt_image *theIm,
			vt_image *resIm,
			Neighborhood N,
			int iterations )
{
  char *proc="GreyLevelDilation";
  int connectivity;
  int theDim[3];

  if ( (theIm->dim.x != resIm->dim.x) ||
       (theIm->dim.y != resIm->dim.y) ||
       (theIm->dim.z != resIm->dim.z) ||
       (theIm->type != resIm->type) )
    return ( -1 );
  
  connectivity = Neighborhood2Int( N );
  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  switch ( theIm->type ) {
  case UCHAR :
    GreyLevelDilation_u8( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  case SSHORT :
    GreyLevelDilation_s16( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  case FLOAT :
    GreyLevelDilation_r32( (void*)(theIm->buf), (void*)(resIm->buf),
			   theDim, connectivity, iterations );
    break;
    /*
  case SCHAR :
    GreyLevelDilation_s8( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  case USHORT :
    GreyLevelDilation_u16( (void*)(theIm->buf), (void*)(resIm->buf),
			   theDim, connectivity, iterations );
    break;
  case SSHORT :
    GreyLevelDilation_s16( (void*)(theIm->buf), (void*)(resIm->buf),
			   theDim, connectivity, iterations );
    break;
    */
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}







int GreyLevelErosion( vt_image *theIm,
			vt_image *resIm,
			Neighborhood N,
			int iterations )
{
  char *proc="GreyLevelErosion";
  int connectivity;
  int theDim[3];

  if ( (theIm->dim.x != resIm->dim.x) ||
       (theIm->dim.y != resIm->dim.y) ||
       (theIm->dim.z != resIm->dim.z) ||
       (theIm->type != resIm->type) )
    return ( -1 );
  
  connectivity = Neighborhood2Int( N );
  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  switch ( theIm->type ) {
  case UCHAR :
    GreyLevelErosion_u8( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  case SSHORT :
    GreyLevelErosion_s16( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  case FLOAT :
    GreyLevelErosion_r32( (void*)(theIm->buf), (void*)(resIm->buf),
			   theDim, connectivity, iterations );
    break;
    /*
  case SCHAR :
    GreyLevelErosion_s8( (void*)(theIm->buf), (void*)(resIm->buf),
		       theDim, connectivity, iterations );
    break;
  case USHORT :
    GreyLevelErosion_u16( (void*)(theIm->buf), (void*)(resIm->buf),
			   theDim, connectivity, iterations );
    break;
  case SSHORT :
    GreyLevelErosion_s16( (void*)(theIm->buf), (void*)(resIm->buf),
			   theDim, connectivity, iterations );
    break;
    */
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}


static int Neighborhood2Int ( Neighborhood N )
{
  int connectivity = 26;
  switch ( N ) {
  case N04 :
    connectivity = 4; break;
  case N06 :
    connectivity = 6; break;
  case N08 :
    connectivity = 8; break;
  case N10 :
    connectivity = 10; break;
  case N18 :
    connectivity = 18; break;
  case N26 :
    connectivity = 26; break;
  }
  return( connectivity );
}

