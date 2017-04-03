/*************************************************************************
 * vt_mip.c -
 *
 * $Id: vt_mip.c,v 1.6 2002/12/11 12:05:39 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 */


#include <vt_mip.h>

/*------- Definition des fonctions statiques ----------*/

int VT_FIP( im, xy, xz, zy )
vt_image *im;
vt_image *xy;
vt_image *xz;
vt_image *zy;
{
    register int x, y, z;
    char *proc_name="VT_FIP";

    /*--- tests ---*/
    if ( VT_Test1Image( im, proc_name ) == -1 ) return( -1 );

    if ( xy != (vt_image*)NULL ) {
      if ( VT_Test1Image( xy, proc_name ) == -1 ) return( -1 );
      if ( im->type != xy->type ) {
        VT_Error( "incorrect XY projection type", proc_name );
        return( -1 );
      }
      if ( (xy->dim.x != im->dim.x) || (xy->dim.y != im->dim.y) || (xy->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the XY projection", proc_name );
        return( -1 );
      }
    }

    if ( xz != (vt_image*)NULL ) {
      if ( VT_Test1Image( xz, proc_name ) == -1 ) return( -1 );
      if ( im->type != xz->type ) {
        VT_Error( "incorrect XZ projection type", proc_name );
        return( -1 );
      }
      if ( (xz->dim.x != im->dim.x) || (xz->dim.y != im->dim.z) || (xz->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the XZ projection", proc_name );
        return( -1 );
      }
    }

    if ( zy != (vt_image*)NULL ) {
      if ( VT_Test1Image( zy, proc_name ) == -1 ) return( -1 );
      if ( im->type != zy->type ) {
        VT_Error( "incorrect ZY projection type", proc_name );
        return( -1 );
      }
      if ( (zy->dim.x != im->dim.z) || (zy->dim.y != im->dim.y) || (zy->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the ZY projection", proc_name );
        return( -1 );
      }
    }



#define _FIP_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxy, ***bxz, ***bzy;        \
      bim = (TYPE ***)(im->array);                \
      bxy = (TYPE ***)(xy->array);                \
      bxz = (TYPE ***)(xz->array);                \
      bzy = (TYPE ***)(zy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
        bzy[0][y][z] = 0;                       \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ )         \
        bxz[0][z][x] = 0;                       \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ )         \
        bxy[0][y][x] = 0;                       \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                \
        /*--- XY projection ---*/                        \
          if ( bxy[0][y][x] == 0 && bim[z][y][x] > 0 )  \
             bxy[0][y][x] = bim[z][y][x];                \
        /*--- XZ projection ---*/                        \
          if ( bxz[0][z][x] == 0 && bim[z][y][x] > 0 )  \
             bxz[0][z][x] = bim[z][y][x];                \
        /*--- ZY projection ---*/                        \
          if ( bzy[0][y][z] == 0 && bim[z][y][x] > 0 )  \
             bzy[0][y][z] = bim[z][y][x];                \
      }                                                 \
}

#define _FIP_XY_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxy;                        \
      bim = (TYPE ***)(im->array);                \
      bxy = (TYPE ***)(xy->array);                \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ )         \
        bxy[0][y][x] = 0;                       \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {        \
        /*--- XY projection ---*/                        \
          if ( bxy[0][y][x] == 0 && bim[z][y][x] > 0 )  \
             bxy[0][y][x] = bim[z][y][x];                \
      }                                                        \
}

#define _FIP_XZ_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxz;                       \
      bim = (TYPE ***)(im->array);                \
      bxz = (TYPE ***)(xz->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ )         \
        bxz[0][z][x] = 0;                       \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {        \
        /*--- XZ projection ---*/                \
          if ( bxz[0][z][x] == 0 && bim[z][y][x] > 0 )  \
             bxz[0][z][x] = bim[z][y][x];                \
      }                                                        \
}

#define _FIP_ZY_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bzy;                       \
      bim = (TYPE ***)(im->array);                \
      bzy = (TYPE ***)(zy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
        bzy[0][y][z] = 0;                       \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {      \
        /*--- ZY projection ---*/                \
          if ( bzy[0][y][z] == 0 && bim[z][y][x] > 0 )  \
             bzy[0][y][z] = bim[z][y][x];                \
      }                                                              \
}

    switch ( im->type ) {
    case UCHAR :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _FIP_PROCESSING_( u8 )
      else {
        if ( xy != (vt_image*)NULL ) _FIP_XY_PROCESSING_( u8 )
        if ( xz != (vt_image*)NULL ) _FIP_XZ_PROCESSING_( u8 )
        if ( zy != (vt_image*)NULL ) _FIP_ZY_PROCESSING_( u8 )
      }
      break;
    case SCHAR :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _FIP_PROCESSING_( s8 )
      else {
        if ( xy != (vt_image*)NULL ) _FIP_XY_PROCESSING_( s8 )
        if ( xz != (vt_image*)NULL ) _FIP_XZ_PROCESSING_( s8 )
        if ( zy != (vt_image*)NULL ) _FIP_ZY_PROCESSING_( s8 )
      }
      break;
    case USHORT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _FIP_PROCESSING_( u16 )
      else {
        if ( xy != (vt_image*)NULL ) _FIP_XY_PROCESSING_( u16 )
        if ( xz != (vt_image*)NULL ) _FIP_XZ_PROCESSING_( u16 )
        if ( zy != (vt_image*)NULL ) _FIP_ZY_PROCESSING_( u16 )
      }
      break;
    case SSHORT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _FIP_PROCESSING_( s16 )
      else {
        if ( xy != (vt_image*)NULL ) _FIP_XY_PROCESSING_( s16 )
        if ( xz != (vt_image*)NULL ) _FIP_XZ_PROCESSING_( s16 )
        if ( zy != (vt_image*)NULL ) _FIP_ZY_PROCESSING_( s16 )
      }
      break;
    case SINT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _FIP_PROCESSING_( i32 )
      else {
        if ( xy != (vt_image*)NULL ) _FIP_XY_PROCESSING_( i32 )
        if ( xz != (vt_image*)NULL ) _FIP_XZ_PROCESSING_( i32 )
        if ( zy != (vt_image*)NULL ) _FIP_ZY_PROCESSING_( i32 )
      }
      break;
    default :
        VT_Error( "unable to deal with such image type", proc_name );
        return( -1 );
    }

    return( 1 );
}













int VT_MIP( vt_image *im, vt_image *xy, vt_image *xz, vt_image *zy )
{
    register int x, y, z;
    char *proc_name="VT_MIP";

    /*--- tests ---*/
    if ( VT_Test1Image( im, proc_name ) == -1 ) return( -1 );

    if ( xy != (vt_image*)NULL ) {
      if ( VT_Test1Image( xy, proc_name ) == -1 ) return( -1 );
      if ( im->type != xy->type ) {
        VT_Error( "incorrect XY projection type", proc_name );
        return( -1 );
      }
      if ( (xy->dim.x != im->dim.x) || (xy->dim.y != im->dim.y) || (xy->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the XY projection", proc_name );
        return( -1 );
      }
    }

    if ( xz != (vt_image*)NULL ) {
      if ( VT_Test1Image( xz, proc_name ) == -1 ) return( -1 );
      if ( im->type != xz->type ) {
        VT_Error( "incorrect XZ projection type", proc_name );
        return( -1 );
      }
      if ( (xz->dim.x != im->dim.x) || (xz->dim.y != im->dim.z) || (xz->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the XZ projection", proc_name );
        return( -1 );
      }
    }

    if ( zy != (vt_image*)NULL ) {
      if ( VT_Test1Image( zy, proc_name ) == -1 ) return( -1 );
      if ( im->type != zy->type ) {
        VT_Error( "incorrect ZY projection type", proc_name );
        return( -1 );
      }
      if ( (zy->dim.x != im->dim.z) || (zy->dim.y != im->dim.y) || (zy->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the ZY projection", proc_name );
        return( -1 );
      }
    }



#define _MIP_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxy, ***bxz, ***bzy;        \
      bim = (TYPE ***)(im->array);                \
      bxy = (TYPE ***)(xy->array);                \
      bxz = (TYPE ***)(xz->array);                \
      bzy = (TYPE ***)(zy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- XY projection ---*/                                                \
        if ( z == 0 ) {                                                                \
          bxy[0][y][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxy[0][y][x] < bim[z][y][x] ) bxy[0][y][x] = bim[z][y][x];        \
        }                                                                        \
        /*--- XZ projection ---*/                                                \
        if ( y == 0 ) {                                                                \
          bxz[0][z][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxz[0][z][x] < bim[z][y][x] ) bxz[0][z][x] = bim[z][y][x];        \
        }                                                                        \
        /*--- ZY projection ---*/                                                \
        if ( x == 0 ) {                                                                \
          bzy[0][y][z] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bzy[0][y][z] < bim[z][y][x] ) bzy[0][y][z] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

#define _MIP_XY_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxy;                        \
      bim = (TYPE ***)(im->array);                \
      bxy = (TYPE ***)(xy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- XY projection ---*/                                                \
        if ( z == 0 ) {                                                                \
          bxy[0][y][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxy[0][y][x] < bim[z][y][x] ) bxy[0][y][x] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

#define _MIP_XZ_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxz;                       \
      bim = (TYPE ***)(im->array);                \
      bxz = (TYPE ***)(xz->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- XZ projection ---*/                                                \
        if ( y == 0 ) {                                                                \
          bxz[0][z][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxz[0][z][x] < bim[z][y][x] ) bxz[0][z][x] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

#define _MIP_ZY_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bzy;                       \
      bim = (TYPE ***)(im->array);                \
      bzy = (TYPE ***)(zy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- ZY projection ---*/                                                \
        if ( x == 0 ) {                                                                \
          bzy[0][y][z] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bzy[0][y][z] < bim[z][y][x] ) bzy[0][y][z] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

    switch ( im->type ) {
    case UCHAR :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MIP_PROCESSING_( u8 )
      else {
        if ( xy != (vt_image*)NULL ) _MIP_XY_PROCESSING_( u8 )
        if ( xz != (vt_image*)NULL ) _MIP_XZ_PROCESSING_( u8 )
        if ( zy != (vt_image*)NULL ) _MIP_ZY_PROCESSING_( u8 )
      }
      break;
    case SCHAR :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MIP_PROCESSING_( s8 )
      else {
        if ( xy != (vt_image*)NULL ) _MIP_XY_PROCESSING_( s8 )
        if ( xz != (vt_image*)NULL ) _MIP_XZ_PROCESSING_( s8 )
        if ( zy != (vt_image*)NULL ) _MIP_ZY_PROCESSING_( s8 )
      }
      break;
    case USHORT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MIP_PROCESSING_( u16 )
      else {
        if ( xy != (vt_image*)NULL ) _MIP_XY_PROCESSING_( u16 )
        if ( xz != (vt_image*)NULL ) _MIP_XZ_PROCESSING_( u16 )
        if ( zy != (vt_image*)NULL ) _MIP_ZY_PROCESSING_( u16 )
      }
      break;
    case SSHORT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MIP_PROCESSING_( s16 )
      else {
        if ( xy != (vt_image*)NULL ) _MIP_XY_PROCESSING_( s16 )
        if ( xz != (vt_image*)NULL ) _MIP_XZ_PROCESSING_( s16 )
        if ( zy != (vt_image*)NULL ) _MIP_ZY_PROCESSING_( s16 )
      }
      break;
    case SINT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MIP_PROCESSING_( i32 )
      else {
        if ( xy != (vt_image*)NULL ) _MIP_XY_PROCESSING_( i32 )
        if ( xz != (vt_image*)NULL ) _MIP_XZ_PROCESSING_( i32 )
        if ( zy != (vt_image*)NULL ) _MIP_ZY_PROCESSING_( i32 )
      }
      break;
    case FLOAT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MIP_PROCESSING_( r32 )
      else {
        if ( xy != (vt_image*)NULL ) _MIP_XY_PROCESSING_( r32 )
        if ( xz != (vt_image*)NULL ) _MIP_XZ_PROCESSING_( r32 )
        if ( zy != (vt_image*)NULL ) _MIP_ZY_PROCESSING_( r32 )
      }
      break;
    default :
        VT_Error( "unable to deal with such image type", proc_name );
        return( -1 );
    }

    return( 1 );
}






int VT_MinMIP( vt_image *im, vt_image *xy, vt_image *xz, vt_image *zy )
{
    register int x, y, z;
    char *proc_name="VT_MinMIP";

    /*--- tests ---*/
    if ( VT_Test1Image( im, proc_name ) == -1 ) return( -1 );

    if ( xy != (vt_image*)NULL ) {
      if ( VT_Test1Image( xy, proc_name ) == -1 ) return( -1 );
      if ( im->type != xy->type ) {
        VT_Error( "incorrect XY projection type", proc_name );
        return( -1 );
      }
      if ( (xy->dim.x != im->dim.x) || (xy->dim.y != im->dim.y) || (xy->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the XY projection", proc_name );
        return( -1 );
      }
    }

    if ( xz != (vt_image*)NULL ) {
      if ( VT_Test1Image( xz, proc_name ) == -1 ) return( -1 );
      if ( im->type != xz->type ) {
        VT_Error( "incorrect XZ projection type", proc_name );
        return( -1 );
      }
      if ( (xz->dim.x != im->dim.x) || (xz->dim.y != im->dim.z) || (xz->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the XZ projection", proc_name );
        return( -1 );
      }
    }

    if ( zy != (vt_image*)NULL ) {
      if ( VT_Test1Image( zy, proc_name ) == -1 ) return( -1 );
      if ( im->type != zy->type ) {
        VT_Error( "incorrect ZY projection type", proc_name );
        return( -1 );
      }
      if ( (zy->dim.x != im->dim.z) || (zy->dim.y != im->dim.y) || (zy->dim.z != (int)1) ) {
        VT_Error( "incorrect image dimension for the ZY projection", proc_name );
        return( -1 );
      }
    }



#define _MINMIP_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxy, ***bxz, ***bzy;        \
      bim = (TYPE ***)(im->array);                \
      bxy = (TYPE ***)(xy->array);                \
      bxz = (TYPE ***)(xz->array);                \
      bzy = (TYPE ***)(zy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- XY projection ---*/                                                \
        if ( z == 0 ) {                                                                \
          bxy[0][y][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxy[0][y][x] > bim[z][y][x] ) bxy[0][y][x] = bim[z][y][x];        \
        }                                                                        \
        /*--- XZ projection ---*/                                                \
        if ( y == 0 ) {                                                                \
          bxz[0][z][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxz[0][z][x] > bim[z][y][x] ) bxz[0][z][x] = bim[z][y][x];        \
        }                                                                        \
        /*--- ZY projection ---*/                                                \
        if ( x == 0 ) {                                                                \
          bzy[0][y][z] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bzy[0][y][z] > bim[z][y][x] ) bzy[0][y][z] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

#define _MINMIP_XY_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxy;                        \
      bim = (TYPE ***)(im->array);                \
      bxy = (TYPE ***)(xy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- XY projection ---*/                                                \
        if ( z == 0 ) {                                                                \
          bxy[0][y][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxy[0][y][x] > bim[z][y][x] ) bxy[0][y][x] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

#define _MINMIP_XZ_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bxz;                       \
      bim = (TYPE ***)(im->array);                \
      bxz = (TYPE ***)(xz->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- XZ projection ---*/                                                \
        if ( y == 0 ) {                                                                \
          bxz[0][z][x] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bxz[0][z][x] > bim[z][y][x] ) bxz[0][z][x] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

#define _MINMIP_ZY_PROCESSING_( TYPE ) {                \
      TYPE ***bim, ***bzy;                       \
      bim = (TYPE ***)(im->array);                \
      bzy = (TYPE ***)(zy->array);                \
      for ( z = 0; z < (int)im->dim.z; z ++ )        \
      for ( y = 0; y < (int)im->dim.y; y ++ )        \
      for ( x = 0; x < (int)im->dim.x; x ++ ) {                                        \
        /*--- ZY projection ---*/                                                \
        if ( x == 0 ) {                                                                \
          bzy[0][y][z] = bim[z][y][x];                                                \
        } else {                                                                \
          if ( bzy[0][y][z] > bim[z][y][x] ) bzy[0][y][z] = bim[z][y][x];        \
        }                                                                        \
      }                                                                                \
}

    switch ( im->type ) {
    case UCHAR :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MINMIP_PROCESSING_( u8 )
      else {
        if ( xy != (vt_image*)NULL ) _MINMIP_XY_PROCESSING_( u8 )
        if ( xz != (vt_image*)NULL ) _MINMIP_XZ_PROCESSING_( u8 )
        if ( zy != (vt_image*)NULL ) _MINMIP_ZY_PROCESSING_( u8 )
      }
      break;
    case SCHAR :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MINMIP_PROCESSING_( s8 )
      else {
        if ( xy != (vt_image*)NULL ) _MINMIP_XY_PROCESSING_( s8 )
        if ( xz != (vt_image*)NULL ) _MINMIP_XZ_PROCESSING_( s8 )
        if ( zy != (vt_image*)NULL ) _MINMIP_ZY_PROCESSING_( s8 )
      }
      break;
    case USHORT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MINMIP_PROCESSING_( u16 )
      else {
        if ( xy != (vt_image*)NULL ) _MINMIP_XY_PROCESSING_( u16 )
        if ( xz != (vt_image*)NULL ) _MINMIP_XZ_PROCESSING_( u16 )
        if ( zy != (vt_image*)NULL ) _MINMIP_ZY_PROCESSING_( u16 )
      }
      break;
    case SSHORT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MINMIP_PROCESSING_( s16 )
      else {
        if ( xy != (vt_image*)NULL ) _MINMIP_XY_PROCESSING_( s16 )
        if ( xz != (vt_image*)NULL ) _MINMIP_XZ_PROCESSING_( s16 )
        if ( zy != (vt_image*)NULL ) _MINMIP_ZY_PROCESSING_( s16 )
      }
      break;
    case SINT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MINMIP_PROCESSING_( i32 )
      else {
        if ( xy != (vt_image*)NULL ) _MINMIP_XY_PROCESSING_( i32 )
        if ( xz != (vt_image*)NULL ) _MINMIP_XZ_PROCESSING_( i32 )
        if ( zy != (vt_image*)NULL ) _MINMIP_ZY_PROCESSING_( i32 )
      }
      break;
    case FLOAT :
      if ( xy != (vt_image*)NULL && xz != (vt_image*)NULL && zy != (vt_image*)NULL )
        _MINMIP_PROCESSING_( r32 )
      else {
        if ( xy != (vt_image*)NULL ) _MINMIP_XY_PROCESSING_( r32 )
        if ( xz != (vt_image*)NULL ) _MINMIP_XZ_PROCESSING_( r32 )
        if ( zy != (vt_image*)NULL ) _MINMIP_ZY_PROCESSING_( r32 )
      }
      break;
    default :
        VT_Error( "unable to deal with such image type", proc_name );
        return( -1 );
    }

    return( 1 );
}





int VT_OneViewMIP( void *theBuf,       /* buffer 3D d'entree */
                   int *theDim,
                   void *resSlice,     /* buffer 2D resultat */
                   int *resDim,
                   void *auxBuf,       /* buffer 3D auxiliaire, peut etre NULL */
                   int *auxDim,
                   bufferType theType,    /* type des 3 buffers */
                   double rotationAngle,
                   float *rotationDir,
                   enumDirection projectionDirection,
                   enumInterpolation typeInterpolation )
{
  char *proc = "VT_OneViewMIP";
  void *tmpBuf;
  vt_image tmpIm;
  int tmpDim[3];
  double n;

  int i;

  double rotDir[3];
  int offset[2];
  double theCtr[3];
  double tmpCtr[3];

  double rotVec[3];
  double rotMat[9];
  double mat[16];

  int x, y, z;


  /* image auxiliaire pour le reechantillonnage
   */
  if ( auxBuf == NULL ) {
    n = sqrt( theDim[0] * theDim[0] + theDim[1] * theDim[1] + theDim[2] * theDim[2] );
    tmpDim[0] = tmpDim[1] = tmpDim[2] = 1 + (int)n;
    VT_Image( &tmpIm );
    VT_InitImage( &tmpIm, "tmp.inr", tmpDim[0], tmpDim[1], tmpDim[2], theType );
    if ( VT_AllocImage( &tmpIm ) != 1 ) {
      VT_Error( "unable to allocate auxiliary image", proc );
      return( -1 );
    }
    tmpBuf = tmpIm.buf;
  }
  else {
    tmpBuf = auxBuf;
    tmpDim[0] = auxDim[0];
    tmpDim[1] = auxDim[1];
    tmpDim[2] = auxDim[2];
  }



  /* mettre a zero le plan resultat
   */

#define _RAZ_SLICE_( TYPE ) { \
  for ( i = 0; i < resDim[0]*resDim[1]; i++ ) ((TYPE*)resSlice)[i] = 0; \
}
  
  switch( theType ) {
  case UCHAR  : _RAZ_SLICE_( u8 )    break;
  case SCHAR  : _RAZ_SLICE_( s8 )    break;
  case USHORT : _RAZ_SLICE_( u16 )   break;
  case SSHORT : _RAZ_SLICE_( s16 )   break;
  case FLOAT  : _RAZ_SLICE_( r32 )   break;
  default :
    VT_Error( "unable to deal with such image type", proc );
    if ( auxBuf == NULL ) VT_FreeImage( &tmpIm );
    return( -1 );
  }


  /* determination de l'axe de rotation
   */
  n = 0;
  if ( rotationDir != (float*)NULL ) {
    rotDir[0] = rotationDir[0];
    rotDir[1] = rotationDir[1];
    rotDir[2] = rotationDir[2];
    n = sqrt( rotDir[0]*rotDir[0] + rotDir[1]*rotDir[1] + rotDir[2]*rotDir[2] );
    if ( n > 0.0001 ) {
      rotDir[0] /= n;
      rotDir[1] /= n;
      rotDir[2] /= n;
    } 
  }
  if ( rotationDir == (float*)NULL || n <= 0.0001 ) {
    rotDir[0] = rotDir[1] = rotDir[2] = 0.0;
    switch( projectionDirection ) {
    default :
    case _Z_ : rotDir[1] = 1.0 ; break;
    case _Y_ : rotDir[2] = 1.0 ; break;
    case _X_ : rotDir[1] = 1.0 ; break;
    }
  }
  
  
  
  switch( projectionDirection ) {
  default :
  case _Z_ : 
    offset[0] = (int)( (double)(tmpDim[0] - resDim[0])/2.0 + 0.5 );
    offset[1] = (int)( (double)(tmpDim[1] - resDim[1])/2.0 + 0.5 );
    break;
  case _Y_ : 
    offset[0] = (int)( (double)(tmpDim[0] - resDim[0])/2.0 + 0.5 );
    offset[1] = (int)( (double)(tmpDim[2] - resDim[1])/2.0 + 0.5 );
    break;
  case _X_ : 
    offset[0] = (int)( (double)(tmpDim[2] - resDim[0])/2.0 + 0.5 );
    offset[1] = (int)( (double)(tmpDim[1] - resDim[1])/2.0 + 0.5 );
    break;
  }

  
  /* centres des images
   */
  theCtr[0] = ((double)theDim[0] - 1.0) / 2.0;
  theCtr[1] = ((double)theDim[1] - 1.0) / 2.0;
  theCtr[2] = ((double)theDim[2] - 1.0) / 2.0;

  tmpCtr[0] = ((double)tmpDim[0] - 1.0) / 2.0;
  tmpCtr[1] = ((double)tmpDim[1] - 1.0) / 2.0;
  tmpCtr[2] = ((double)tmpDim[2] - 1.0) / 2.0;

  
  /* vecteur et matrice rotation
   */
  rotVec[0] = rotationAngle * rotDir[0];
  rotVec[1] = rotationAngle * rotDir[1];
  rotVec[2] = rotationAngle * rotDir[2];

  RotationMatrixFromRotationVector( rotMat, rotVec );
  
  mat[ 0] = rotMat[0];   mat[ 1] = rotMat[1];   mat[ 2] = rotMat[2];
  mat[ 4] = rotMat[3];   mat[ 5] = rotMat[4];   mat[ 6] = rotMat[5];
  mat[ 8] = rotMat[6];   mat[ 9] = rotMat[7];   mat[10] = rotMat[8];
  mat[12] = 0.0;         mat[13] = 0.0;         mat[14] = 0.0;

  mat[ 3] = theCtr[0] - mat[ 0]*tmpCtr[0] - mat[ 1]*tmpCtr[1] - mat[ 2]*tmpCtr[2];
  mat[ 7] = theCtr[1] - mat[ 4]*tmpCtr[0] - mat[ 5]*tmpCtr[1] - mat[ 6]*tmpCtr[2];
  mat[11] = theCtr[2] - mat[ 8]*tmpCtr[0] - mat[ 9]*tmpCtr[1] - mat[10]*tmpCtr[2];
  mat[15] = 1.0;
  


  /* reechantillonage
   */
  switch( typeInterpolation ) {
  default :
  case _LINEAR_ :
    switch( theType ) {
    case UCHAR :
      Reech3DTriLin4x4_u8( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case SCHAR :
      Reech3DTriLin4x4_s8( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case USHORT :
      Reech3DTriLin4x4_u16( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case SSHORT :
      Reech3DTriLin4x4_s16( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case FLOAT :
      Reech3DTriLin4x4_r32( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    default :
      VT_Error( "unable to deal with such image type", proc );
      if ( auxBuf == NULL ) VT_FreeImage( &tmpIm );
      return( -1 );
    }
    break;
  case _NEAREST_ :
    switch( theType ) {
    case UCHAR :
      Reech3DNearest4x4_u8( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case SCHAR :
      Reech3DNearest4x4_s8( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case USHORT :
      Reech3DNearest4x4_u16( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case SSHORT :
      Reech3DNearest4x4_s16( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    case FLOAT :
      Reech3DNearest4x4_r32( theBuf, theDim, tmpBuf, tmpDim, mat );
      break;
    default :
      VT_Error( "unable to deal with such image type", proc );
      if ( auxBuf == NULL ) VT_FreeImage( &tmpIm );
      return( -1 );
    }
    break;
  }

#define _ANIMMIP_XY_PROCESSING_( TYPE ) {        \
      TYPE *bim, *bxy;                        \
      bim = (TYPE *)(tmpBuf);                \
      bxy = (TYPE *)(resSlice);                \
      for ( z = 0; z < tmpDim[2]; z ++ )        \
      for ( y = offset[1]; y < resDim[1]+offset[1]; y ++ )                \
      for ( x = offset[0]; x < resDim[0]+offset[0]; x ++ ) {                \
        /*--- XY projection ---*/                                        \
        if ( z == 0 ) {                                                        \
          bxy[(y-offset[1])*resDim[0]+(x-offset[0])] = bim[(z*tmpDim[1]+y)*tmpDim[0]+x];                \
        } else {                                                        \
          if ( bxy[(y-offset[1])*resDim[0]+(x-offset[0])] < bim[(z*tmpDim[1]+y)*tmpDim[0]+x] )       \
            bxy[(y-offset[1])*resDim[0]+(x-offset[0])]  = bim[(z*tmpDim[1]+y)*tmpDim[0]+x];          \
        }                                                                \
      }                                                                        \
}
  
#define _ANIMMIP_XZ_PROCESSING_( TYPE ) {        \
      TYPE *bim, *bxz;                       \
      bim = (TYPE *)(tmpBuf);                \
      bxz = (TYPE *)(resSlice);                \
      for ( z = offset[1]; z < resDim[1]+offset[1]; z ++ )                \
      for ( y = 0; y < tmpDim[1]; y ++ )                                \
      for ( x = offset[0]; x < resDim[0]+offset[0]; x ++ ) {                \
        /*--- XZ projection ---*/                                        \
        if ( y == 0 ) {                                                        \
          bxz[(z-offset[1])*resDim[0]+(x-offset[0])] = bim[(z*tmpDim[1]+y)*tmpDim[0]+x];                \
        } else {                                                        \
          if ( bxz[(z-offset[1])*resDim[0]+(x-offset[0])] < bim[(z*tmpDim[1]+y)*tmpDim[0]+x] )       \
            bxz[(z-offset[1])*resDim[0]+(x-offset[0])] = bim[(z*tmpDim[1]+y)*tmpDim[0]+x];           \
        }                                                                \
      }                                                                        \
}

#define _ANIMMIP_ZY_PROCESSING_( TYPE ) {        \
      TYPE *bim, *bzy;                       \
      bim = (TYPE *)(tmpBuf);                \
      bzy = (TYPE *)(resSlice);                \
      for ( z = offset[0]; z < resDim[0]+offset[0]; z ++ )           \
      for ( y = offset[1]; y < resDim[1]+offset[1]; y ++ )                \
      for ( x = 0; x < tmpDim[0]; x ++ ) {                                \
        /*--- ZY projection ---*/                                        \
        if ( x == 0 ) {                                                        \
          bzy[(y-offset[1])*resDim[0]+(z-offset[0])] = bim[(z*tmpDim[1]+y)*tmpDim[0]+x];                \
        } else {                                                        \
          if ( bzy[(y-offset[1])*resDim[0]+(z-offset[0])] < bim[(z*tmpDim[1]+y)*tmpDim[0]+x] )       \
            bzy[(y-offset[1])*resDim[0]+(z-offset[0])] = bim[(z*tmpDim[1]+y)*tmpDim[0]+x];                \
        }                                                                \
      }                                                                        \
}

  switch ( theType ) {
  case UCHAR :
    switch( projectionDirection ) {
    default :
    case _Z_ : _ANIMMIP_XY_PROCESSING_( u8 )   break;
    case _Y_ : _ANIMMIP_XZ_PROCESSING_( u8 )   break;
    case _X_ : _ANIMMIP_ZY_PROCESSING_( u8 )   break;
    }
    break;
  case SCHAR :
    switch( projectionDirection ) {
    default :
    case _Z_ : _ANIMMIP_XY_PROCESSING_( s8 )   break;
    case _Y_ : _ANIMMIP_XZ_PROCESSING_( s8 )   break;
    case _X_ : _ANIMMIP_ZY_PROCESSING_( s8 )   break;
    }
    break;
  case USHORT :
    switch( projectionDirection ) {
    default :
    case _Z_ : _ANIMMIP_XY_PROCESSING_( u16 )   break;
    case _Y_ : _ANIMMIP_XZ_PROCESSING_( u16 )   break;
    case _X_ : _ANIMMIP_ZY_PROCESSING_( u16 )   break;
    }
    break;
  case SSHORT :
    switch( projectionDirection ) {
    default :
    case _Z_ : _ANIMMIP_XY_PROCESSING_( s16 )   break;
    case _Y_ : _ANIMMIP_XZ_PROCESSING_( s16 )   break;
    case _X_ : _ANIMMIP_ZY_PROCESSING_( s16 )   break;
    }
    break;
  case FLOAT :
    switch( projectionDirection ) {
    default :
    case _Z_ : _ANIMMIP_XY_PROCESSING_( r32 )   break;
    case _Y_ : _ANIMMIP_XZ_PROCESSING_( r32 )   break;
    case _X_ : _ANIMMIP_ZY_PROCESSING_( r32 )   break;
    }
    break;
  default :
    VT_Error( "unable to deal with such image type", proc );
    if ( auxBuf == NULL ) VT_FreeImage( &tmpIm );
    return( -1 );
  }



  if ( auxBuf == NULL ) VT_FreeImage( &tmpIm );
  return( 1 );
}






int VT_AnimMIP( vt_image *theIm, 
                vt_image *resIm,
                float *rotationDir,
                enumDirection projectionDirection,
                enumInterpolation typeInterpolation )
{
  char *proc = "VT_AnimMIP";
  vt_image tmpIm;
  double n;
  int theDim[3], resDim[2], tmpDim[3];
  double incAngle;
  int iz, r;

  /*
  int iz, x, y, z;

  int offset[2];
  double incAngle;
  double theCtr[3], tmpCtr[3];
  double rotDir[3];
  double rotVec[3];
  double rotMat[9];
  double mat[16];
  double n;
  */

  if ( theIm->type != resIm->type ) {
    VT_Error( "images have different types", proc );
    return( -1 );
  }



  VT_Image( &tmpIm );
  n = theIm->dim.x*theIm->dim.x + theIm->dim.y*theIm->dim.y + theIm->dim.z*theIm->dim.z;
  n = sqrt( n );
  tmpDim[0] = tmpDim[1] = tmpDim[2] = 1 + (int)n;
  VT_Image( &tmpIm );
  VT_InitImage( &tmpIm, "tmp.inr", tmpDim[0], tmpDim[1], tmpDim[2], theIm->type );
  if ( VT_AllocImage( &tmpIm ) != 1 ) {
    VT_Error( "unable to allocate auxiliary image", proc );
    return( -1 );
  }



  VT_ZeroImage( resIm );



  incAngle = 2.0 * 3.1415926536 / (double)resIm->dim.z;

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;

  
  for ( iz = 0; iz < (int)resIm->dim.z; iz++ ) {

    fprintf( stderr, " ... processing %3d/%lu \r", iz, resIm->dim.z );

    switch( theIm->type ) {
    default :
      VT_Error( "unable to deal with such image type", proc );
      VT_FreeImage( &tmpIm );
      return( -1 );
    case UCHAR :
      r = VT_OneViewMIP( theIm->buf, theDim, 
                         &(((u8***)resIm->array)[iz][0][0]), resDim,
                         tmpIm.buf, tmpDim,
                         theIm->type, iz * incAngle,
                         rotationDir, projectionDirection, typeInterpolation );
      break;
    case SCHAR :
      r = VT_OneViewMIP( theIm->buf, theDim, 
                         &(((s8***)resIm->array)[iz][0][0]), resDim,
                         tmpIm.buf, tmpDim,
                         theIm->type, iz * incAngle,
                         rotationDir, projectionDirection, typeInterpolation );
      break;
    case USHORT :
      r = VT_OneViewMIP( theIm->buf, theDim, 
                         &(((u16***)resIm->array)[iz][0][0]), resDim,
                         tmpIm.buf, tmpDim,
                         theIm->type, iz * incAngle,
                         rotationDir, projectionDirection, typeInterpolation );
      break;
    case SSHORT :
      r = VT_OneViewMIP( theIm->buf, theDim, 
                         &(((s16***)resIm->array)[iz][0][0]), resDim,
                         tmpIm.buf, tmpDim,
                         theIm->type, iz * incAngle,
                         rotationDir, projectionDirection, typeInterpolation );
      break;
    case FLOAT :
      r = VT_OneViewMIP( theIm->buf, theDim, 
                         &(((r32***)resIm->array)[iz][0][0]), resDim,
                         tmpIm.buf, tmpDim,
                         theIm->type, iz * incAngle,
                         rotationDir, projectionDirection, typeInterpolation );
      break;
    }
    if ( r != 1 ) {
      VT_FreeImage( &tmpIm );
      return( -1 );
    }

  }

  VT_FreeImage( &tmpIm );
  return( 1 );
}






























