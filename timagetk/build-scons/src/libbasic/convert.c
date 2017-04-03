/*************************************************************************
 * convert.c - conversion between types
 *
 * $Id: convert.c,v 1.5 2003/07/04 08:16:47 greg Exp $
 *
 * LICENSE:
 * GPL v3.0 (see gpl-3.0.txt for details)
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * June, 9 1998
 *
 * ADDITIONS, CHANGES
 *
 * - Tue Feb 22 11:25:39 MET 2000 (G. Malandain)
 *   add in ConvertBuffer():
 *       USHORT to UCHAR
 *       USHORT to SHORT
 * - Fri Jan 10 15:20:39 MET 2014 (G. Michelin)
 *   add in ConvertBuffer():
 *       UCHAR to UINT
 *
 */

#include <convert.h>

static int _verbose_ = 1;

void setVerboseInConvert( int v )
{
  _verbose_ = v;
}

void incrementVerboseInConvert(  )
{
  _verbose_ ++;
}

void decrementVerboseInConvert(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





void printType( FILE *f, bufferType type )
{
  switch ( type ) {
  default : fprintf( f, "not defined, how embarrasing ..." ); break;
  case TYPE_UNKNOWN : fprintf( f, "TYPE_UNKNOWN" ); break;
  case UCHAR  : fprintf( f, "UCHAR" ); break;
  case SCHAR  : fprintf( f, "SCHAR" ); break;
  case USHORT : fprintf( f, "USHORT" ); break;
  case SSHORT : fprintf( f, "SSHORT" ); break;
  case UINT   : fprintf( f, "UINT" ); break;
  case SINT    : fprintf( f, "SINT" ); break;
  case ULINT  : fprintf( f, "ULINT" ); break;
  case SLINT  : fprintf( f, "SLINT" ); break;
  case FLOAT  : fprintf( f, "FLOAT" ); break;
  case DOUBLE  : fprintf( f, "DOUBLE" ); break;
  }
  fprintf( f, "\n" );
}





int ConvertBuffer( void *bufferIn,
                    bufferType typeIn,
                    void *bufferOut,
                    bufferType typeOut,
                    size_t bufferLength )
{
  char *proc = "ConvertBuffer";
  register size_t i;
  register int min, max;
  register u8 *u8buf;
  register s8 *s8buf;
  register u16 *u16buf;
  register s16 *s16buf;
  register u32 *u32buf;
  register s32 *s32buf = NULL;
  register r32 *r32buf;
  register r64 *r64buf;

  if ( (typeOut == typeIn) && (bufferOut == bufferIn) )
    return( 1 );

  if ( bufferLength <= 0 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: buffer length is negative or zero.\n", proc );
      return( -1 );
  }
  if ( (bufferIn == (void*)NULL) || (bufferOut == (void*)NULL) ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: NULL buffer(s).\n", proc );
      return( -1 );
  }
  
  switch ( typeOut ) {
  case SCHAR :
    s8buf = (s8*)bufferOut;
    min = -128; max = 127;
    switch( typeIn ) {
    case SCHAR :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(s8) );
      break;
    case FLOAT :
      r32buf = (r32*)bufferIn;
      for (i=bufferLength; i>0; i--, s8buf++, r32buf++ ) {
        if ( *r32buf < min ) *s8buf = min;
        else if ( *r32buf < 0.0 ) *s8buf = (int)(*r32buf - 0.5);
        else if ( *r32buf < max ) *s8buf = (int)(*r32buf + 0.5);
        else *s8buf = max;
      }
      break;
    case DOUBLE :
      r64buf = (r64*)bufferIn;
      for (i=bufferLength; i>0; i--, s8buf++, r64buf++ ) {
        if ( *r64buf < min ) *s8buf = min;
        else if ( *r64buf < 0.0 ) *s8buf = (int)(*r64buf - 0.5);
        else if ( *r64buf < max ) *s8buf = (int)(*r64buf + 0.5);
        else *s8buf = max;
      }
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = SCHAR */




    
  case UCHAR :
    u8buf = (u8*)bufferOut;
    min = 0; max = 255;
    switch( typeIn ) {
    case UCHAR :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(u8) );
      break;
    case SSHORT :
      s16buf = (s16*)bufferIn;
      for (i=bufferLength; i>0; i--, u8buf++, s16buf++ ) {
        if ( *s16buf < min ) *u8buf = min;
        else if ( *s16buf < max ) *u8buf = (u8)*s16buf;
        else *s16buf = max;
      }
      break;
    case USHORT :
      u16buf = (u16*)bufferIn;
      for (i=bufferLength; i>0; i--, u8buf++, u16buf++ ) {
        if ( *u16buf < max ) *u8buf = (u8)*u16buf;
        else *u8buf = max;
      }
      break;
    case FLOAT :
      r32buf = (r32*)bufferIn;
      for (i=bufferLength; i>0; i--, u8buf++, r32buf++ ) {
        if ( *r32buf < min ) *u8buf = min;
        else if ( *r32buf < max ) *u8buf = (int)(*r32buf + 0.5);
        else *u8buf = max;
      }
      break;
    case DOUBLE :
      r64buf = (r64*)bufferIn;
      for (i=bufferLength; i>0; i--, u8buf++, r64buf++ ) {
        if ( *r64buf < min ) *u8buf = min;
        else if ( *r64buf < max ) *u8buf = (int)(*r64buf + 0.5);
        else *u8buf = max;
      }
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = UCHAR */





    
  case SSHORT :
    s16buf = (s16*)bufferOut;
    min = -32768; max = 32767;
    switch( typeIn ) {
    case UCHAR :
      u8buf = (u8*)bufferIn;
      for (i=bufferLength; i>0; i--, u8buf++, s16buf++ ) {
        *s16buf = (s16)*u8buf;
      }
      break;
    case SSHORT :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(s16) );
      break;
    case USHORT :
      u16buf = (u16*)bufferIn;
      for (i=bufferLength; i>0; i--, s16buf++, u16buf++ ) {
        if ( *u16buf < max ) *s16buf = (s16)*u16buf;
        else *s16buf = max;
      }
      break;
    case FLOAT :
      r32buf = (r32*)bufferIn;
      for (i=bufferLength; i>0; i--, s16buf++, r32buf++ ) {
        if ( *r32buf < min ) *s16buf = min;
        else if ( *r32buf < 0.0 ) *s16buf = (int)(*r32buf - 0.5);
        else if ( *r32buf < max ) *s16buf = (int)(*r32buf + 0.5);
        else *s16buf = max;
      }
      break;
    case DOUBLE :
      r64buf = (r64*)bufferIn;
      for (i=bufferLength; i>0; i--, s16buf++, r64buf++ ) {
        if ( *r64buf < min ) *s16buf = min;
        else if ( *r64buf < 0.0 ) *s16buf = (int)(*r64buf - 0.5);
        else if ( *r64buf < max ) *s16buf = (int)(*r64buf + 0.5);
        else *s16buf = max;
      }
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = SSHORT */




    
  case USHORT :
    u16buf = (u16*)bufferOut;
    min = 0; max = 65535;
    switch( typeIn ) {
    case UCHAR :
      u8buf = (u8*)bufferIn;
      for (i=bufferLength; i>0; i--, u8buf++, u16buf++ ) {
        *u16buf = (u16)*u8buf;
      }
      break;
    case SSHORT :
      s16buf = (s16*)bufferIn;
      for (i=bufferLength; i>0; i--, s16buf++, u16buf++ ) {
        *u16buf = ( *s16buf < 0 ) ? 0 : (u16)*s16buf;
      }
      break;
    case USHORT :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(u16) );
      break;
    case FLOAT :
      r32buf = (r32*)bufferIn;
      for (i=bufferLength; i>0; i--, u16buf++, r32buf++ ) {
        if ( *r32buf < min ) *u16buf = min;
        else if ( *r32buf < 0.0 ) *u16buf = (int)(*r32buf - 0.5);
        else if ( *r32buf < max ) *u16buf = (int)(*r32buf + 0.5);
        else *u16buf = max;
      }
      break;
    case DOUBLE :
      r64buf = (r64*)bufferIn;
      for (i=bufferLength; i>0; i--, u16buf++, r64buf++ ) {
        if ( *r64buf < min ) *u16buf = min;
        else if ( *r64buf < 0.0 ) *u16buf = (int)(*r64buf - 0.5);
        else if ( *r64buf < max ) *u16buf = (int)(*r64buf + 0.5);
        else *u16buf = max;
      }
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = USHORT */





    
  case SINT :
    s32buf = (s32*)bufferOut;
    switch( typeIn ) {
    case SCHAR :
      s8buf = (s8*)bufferIn;
      for (i=bufferLength; i>0; i--, s8buf++, s32buf++ ) {
        *s32buf = (s32)*s8buf;
      }
      break;
    case UCHAR :
      u8buf = (u8*)bufferIn;
      for (i=bufferLength; i>0; i--, u8buf++, s32buf++ ) {
        *s32buf = (s32)*u8buf;
      }
      break;
    case SSHORT :
      s16buf = (s16*)bufferIn;
      for (i=bufferLength; i>0; i--, s16buf++, s32buf++ ) {
        *s32buf = (s32)*s16buf;
      }
      break;
    case USHORT :
      u16buf = (u16*)bufferIn;
      for (i=bufferLength; i>0; i--, u16buf++, s32buf++ ) {
        *s32buf = (s32)*u16buf;
      }
      break;
    case SINT :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(s32) );
      break;
    case FLOAT :
      r32buf = (r32*)bufferIn;
      for (i=bufferLength; i>0; i--, s32buf++, r32buf++ ) {
        *s32buf = (s32)(*r32buf);
      }
      break;
    case DOUBLE :
      r64buf = (r64*)bufferIn;
      for (i=bufferLength; i>0; i--, s32buf++, r64buf++ ) {
        *s32buf = (s32)(*r64buf);
      }
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = INT */





    
  case UINT :
    u32buf = (u32*)bufferOut;
    switch( typeIn ) {
    case UCHAR :
      u8buf = (u8*)bufferIn;
      for (i=bufferLength; i>0; i--, u32buf++, u8buf++ ) {
        *u32buf = (u32)*u8buf;
      }
      break;
    case UINT :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(u32) );
      break;
    case SINT :
      s32buf = (s32*)bufferIn;
      for (i=bufferLength; i>0; i--, u32buf++, s32buf++ ) {
        if ( *s32buf <= 0 ) *u32buf = (int)0;
        else *u32buf = *s32buf;
      }
      break;
    case FLOAT :
      r32buf = (r32*)bufferIn;
      for (i=bufferLength; i>0; i--, s32buf++, r32buf++ ) {
        if ( *r32buf <= 0.0 ) *s32buf = (int)0;
        else *u32buf = (int)(*r32buf + 0.5);
      }
      break;
    case DOUBLE :
      r64buf = (r64*)bufferIn;
      for (i=bufferLength; i>0; i--, s32buf++, r64buf++ ) {
        if ( *r64buf <= 0.0 ) *s32buf = (int)0;
        else *u32buf = (int)(*r64buf + 0.5);
      }
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = INT */





    
  case FLOAT :
    r32buf = (r32*)bufferOut;
    switch( typeIn ) {
    case UCHAR :
      u8buf = (u8*)bufferIn;
      for (i=bufferLength; i>0; i--, r32buf++, u8buf++ ) {
        *r32buf = (float)(*u8buf);
      }
      break;
    case SCHAR :
      s8buf = (s8*)bufferIn;
      for (i=bufferLength; i>0; i--, r32buf++, s8buf++ ) {
        *r32buf = (float)(*s8buf);
      }
      break;
    case USHORT :
      u16buf = (u16*)bufferIn;
      for (i=bufferLength; i>0; i--, r32buf++, u16buf++ ) {
        *r32buf = (float)(*u16buf);
      }
      break;
    case SSHORT :
      s16buf = (s16*)bufferIn;
      for (i=bufferLength; i>0; i--, r32buf++, s16buf++ ) {
        *r32buf = (float)(*s16buf);
      }
      break;
    case SINT :
      s32buf = (s32*)bufferIn;
      for (i=bufferLength; i>0; i--, r32buf++, s32buf++ ) {
        *r32buf = (float)(*s32buf);
      }
      break;
    case FLOAT :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(r32) );
      break;
    case DOUBLE :
      r64buf = (r64*)bufferIn;
      for (i=bufferLength; i>0; i--, r32buf++, r64buf++ ) {
        *r32buf = (float)(*r64buf);
      }
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = FLOAT */




    
  case DOUBLE :
    r64buf = (r64*)bufferOut;
    switch( typeIn ) {
    case UCHAR :
      u8buf = (u8*)bufferIn;
      for (i=bufferLength; i>0; i--, r64buf++, u8buf++ ) {
        *r64buf = (double)(*u8buf);
      }
      break;
    case SCHAR :
      s8buf = (s8*)bufferIn;
      for (i=bufferLength; i>0; i--, r64buf++, s8buf++ ) {
        *r64buf = (double)(*s8buf);
      }
      break;
    case USHORT :
      u16buf = (u16*)bufferIn;
      for (i=bufferLength; i>0; i--, r64buf++, u16buf++ ) {
        *r64buf = (double)(*u16buf);
      }
      break;
    case SSHORT :
      s16buf = (s16*)bufferIn;
      for (i=bufferLength; i>0; i--, r64buf++, s16buf++ ) {
        *r64buf = (double)(*s16buf);
      }
      break;
    case SINT :
      s32buf = (s32*)bufferIn;
      for (i=bufferLength; i>0; i--, r64buf++, s32buf++ ) {
        *r64buf = (double)(*s32buf);
      }
      break;
    case FLOAT :
      r32buf = (r32*)bufferIn;
      for (i=bufferLength; i>0; i--, r32buf++, r64buf++ ) {
        *r64buf = (double)(*r32buf);
      }
      break;
    case DOUBLE :
      if ( bufferOut == bufferIn ) return( 1 );
      (void)memcpy( bufferOut, bufferIn, bufferLength * sizeof(r64) );
      break;
    default :
        if ( _verbose_ )
            fprintf( stderr, "%s: such conversion not yet implemented.\n", proc );
        return( -1 );
    }
    break; /* end case typeOut = DOUBLE */



   
  default :
      if ( _verbose_ )
          fprintf( stderr, "%s: such output type not yet handled.\n", proc );
      return( -1 );
  }

  return( 1 );

}





void Convert_r32_to_s8( r32 *theBuf,
                        s8 *resBuf,
                        size_t size )
{
  register size_t i;
  register r32* tb = theBuf;
  register s8* rb = resBuf;
  
  for ( i=0; i<size; i++, tb++, rb++ ) {
    if ( *tb < -128.0 ) {
      *rb = -128;
    } else if ( *tb < 0.0 ) {
      *rb = (int)(*tb - 0.5);
    } else if ( *tb < 127.0 ) {
      *rb = (int)(*tb + 0.5);
    } else {
      *rb = 127;
    }
  }
}





void Convert_r32_to_u8( r32 *theBuf,
                        u8 *resBuf,
                        size_t size )
{
  register size_t i;
  register r32* tb = theBuf;
  register u8* rb = resBuf;
  
  for ( i=0; i<size; i++, tb++, rb++ ) {
    if ( *tb < 0.0 ) {
      *rb = 0;
    } else if ( *tb < 255.0 ) {
      *rb = (int)(*tb + 0.5);
    } else {
      *rb = 255;
    }
  }
}





void Convert_r32_to_s16( r32 *theBuf,
                         s16 *resBuf,
                         size_t size )
{
  register size_t i;
  register r32* tb = theBuf;
  register s16* rb = resBuf;
  
  for ( i=0; i<size; i++, tb++, rb++ ) {
    if ( *tb < -32768.0 ) {
      *rb = -32768;
    } else if ( *tb < 0.0 ) {
      *rb = (int)(*tb - 0.5);
    } else if ( *tb < 32767.0 ) {
      *rb = (int)(*tb + 0.5);
    } else {
      *rb = 32767;
    }
  }
}





void Convert_r32_to_u16( r32 *theBuf,
                         u16 *resBuf,
                         size_t size )
{
  register size_t i;
  register r32* tb = theBuf;
  register u16* rb = resBuf;
  
  for ( i=0; i<size; i++, tb++, rb++ ) {
    if ( *tb < 0.0 ) {
      *rb = 0;
    } else if ( *tb < 65535.0 ) {
      *rb = (int)(*tb + 0.5);
    } else {
      *rb = 65535;
    }
  }
}




      
int ConvertScaleBuffer( void *bufferIn,
                         bufferType typeIn,
                         void *bufferOut,
                         bufferType typeOut,
                         size_t bufferLength )
{
  char *proc = "ConvertScaleBuffer";
  register size_t i;
  register u8 *u8buf;
  register s8 *s8buf;
  register u16 *u16buf;
  register s16 *s16buf;
  register s32 *s32buf;
  register r32 *r32buf;
  /*
  register u32 *u32buf;
  register r32 *r32buf;
  register r64 *r64buf;
  */
  register double v;
  double a, theMin, theMax;
  double min, max;

  if ( (bufferIn == (void*)NULL) || (bufferOut == (void*)NULL) ) {
      if ( _verbose_ )
    fprintf( stderr, " Fatal error in %s: NULL buffer(s).\n", proc );
    return( -1 );
  }
  
  if ( bufferOut == bufferIn )
    return( 1 );

  if ( bufferLength <= 0 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: buffer length is negative or zero.\n", proc );
      return( -1 );
  }
  if ( (bufferIn == (void*)NULL) || (bufferOut == (void*)NULL) ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: NULL buffer(s).\n", proc );
      return( -1 );
  }



  switch ( typeIn ) {
  default :
      if ( _verbose_ )
          fprintf( stderr, "%s: such input type not handled yet for minimum/maximum computation.\n", proc );
      return( -1 );
  case SCHAR :
    {
      s8buf = (s8*)bufferIn;
      theMin = theMax = (double)(*s8buf);   s8buf++;
      for (i=bufferLength-1; i>0; i--, s8buf++) {
        v = (double)(*s8buf);
        if ( theMin > v ) theMin = v;
        else if ( theMax < v ) theMax = v;
      }
    }
    break;
  case UCHAR :
    {
      u8buf = (u8*)bufferIn;
      theMin = theMax = (double)(*u8buf);   u8buf++;
      for (i=bufferLength-1; i>0; i--, u8buf++) {
        v = (double)(*u8buf);
        if ( theMin > v ) theMin = v;
        else if ( theMax < v ) theMax = v;
      }
    }
    break;
  case SSHORT :
    {
      s16buf = (s16*)bufferIn;
      theMin = theMax = (double)(*s16buf);   s16buf++;
      for (i=bufferLength-1; i>0; i--, s16buf++) {
        v = (double)(*s16buf);
        if ( theMin > v ) theMin = v;
        else if ( theMax < v ) theMax = v;
      }
    }
    break;
  case USHORT :
    {
      u16buf = (u16*)bufferIn;
      theMin = theMax = (double)(*u16buf);   u16buf++;
      for (i=bufferLength-1; i>0; i--, u16buf++) {
        v = (double)(*u16buf);
        if ( theMin > v ) theMin = v;
        else if ( theMax < v ) theMax = v;
      }
    }
    break;
  case SINT :
    {
      s32buf = (s32*)bufferIn;
      theMin = theMax = (double)(*s32buf);   s32buf++;
      for (i=bufferLength-1; i>0; i--, s32buf++) {
        v = (double)(*s32buf);
        if ( theMin > v ) theMin = v;
        else if ( theMax < v ) theMax = v;
      }
    }
    break;
  case FLOAT :
    {
      r32buf = (r32*)bufferIn;
      theMin = theMax = (double)(*r32buf);   r32buf++;
      for (i=bufferLength-1; i>0; i--, r32buf++) {
        v = (double)(*r32buf);
        if ( theMin > v ) theMin = v;
        else if ( theMax < v ) theMax = v;
      }
    }
    break;
  }



  if ( theMax <= theMin ) {
      if ( _verbose_ )
          fprintf( stderr, " Fatal error in %s: input buffer is of constant value.\n", proc );
      return( -1 );
  }   


  switch ( typeOut ) {
  default :
      if ( _verbose_ )
          fprintf( stderr, "%s: such output type not handled yet for normalisation.\n", proc );
      return( -1 );

  case SCHAR :
    {
      s8 *rbuf = (s8*)bufferOut;
      min = -128;   max = 127;
      a = ((double)max - (double)min)/(theMax-theMin);
      switch ( typeIn ) {
      default :
          if ( _verbose_ )
              fprintf( stderr, "%s: such input type not handled yet for normalisation.\n", proc );
          return( -1 );
      case SCHAR :
        {
          s8buf = (s8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s8buf++, rbuf++) {
            v = min + a*((double)(*s8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s8)(v - 0.5);
            else *rbuf = (s8)(v + 0.5);
          }
        }
        break;
      case UCHAR :
        {
          u8buf = (u8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u8buf++, rbuf++) {
            v = min + a*((double)(*u8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s8)(v - 0.5);
            else *rbuf = (s8)(v + 0.5);
          }
        }
        break;
      case SSHORT :
        {
          s16buf = (s16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s16buf++, rbuf++) {
            v = min + a*((double)(*s16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s8)(v - 0.5);
            else *rbuf = (s8)(v + 0.5);
          }
        }
        break;
      case USHORT :
        {
          u16buf = (u16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u16buf++, rbuf++) {
            v = min + a*((double)(*u16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s8)(v - 0.5);
            else *rbuf = (s8)(v + 0.5);
          }
        }
        break;
      case FLOAT :
        {
          r32buf = (r32*)bufferIn;
          for (i=bufferLength-1; i>0; i--, r32buf++, rbuf++) {
            v = a*((double)(*r32buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s8)(v - 0.5);
            else *rbuf = (s8)(v + 0.5);
          }
        }
        break;
       }
    }
    break;

  case UCHAR :
    {
      u8 *rbuf = (u8*)bufferOut;
      min = 0;   max = 255;
      a = ((double)max - (double)min)/(theMax-theMin);
      switch ( typeIn ) {
      default :
          if ( _verbose_ )
              fprintf( stderr, "%s: such input type not handled yet for normalisation.\n", proc );
          return( -1 );
      case SCHAR :
        {
          s8buf = (s8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s8buf++, rbuf++) {
            v = a*((double)(*s8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u8)(0.0);
            else *rbuf = (u8)(v + 0.5);
          }
        }
        break;
      case UCHAR :
        {
          u8buf = (u8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u8buf++, rbuf++) {
            v = a*((double)(*u8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u8)(0.0);
            else *rbuf = (u8)(v + 0.5);
          }
        }
        break;
      case SSHORT :
        {
          s16buf = (s16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s16buf++, rbuf++) {
            v = a*((double)(*s16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u8)(0.0);
            else *rbuf = (u8)(v + 0.5);
          }
        }
        break;
      case USHORT :
        {
          u16buf = (u16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u16buf++, rbuf++) {
            v = a*((double)(*u16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u8)(0.0);
            else *rbuf = (u8)(v + 0.5);
          }
        }
        break;
      case FLOAT :
        {
          r32buf = (r32*)bufferIn;
          for (i=bufferLength-1; i>0; i--, r32buf++, rbuf++) {
            v = a*((double)(*r32buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u8)(0.0);
            else *rbuf = (u8)(v + 0.5);
          }
        }
        break;
      }
    }
    break;

  case SSHORT :
    {
      s16 *rbuf = (s16*)bufferOut;
      min = -32768;   max = 32767;
      a = ((double)max - (double)min)/(theMax-theMin);
      switch ( typeIn ) {
      default :
          if ( _verbose_ )
              fprintf( stderr, "%s: such input type not handled yet for normalisation.\n", proc );
          return( -1 );
      case SCHAR :
        {
          s8buf = (s8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s8buf++, rbuf++) {
            v = min + a*((double)(*s8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s16)(v - 0.5);
            else *rbuf = (s16)(v + 0.5);
          }
        }
        break;
      case UCHAR :
        {
          u8buf = (u8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u8buf++, rbuf++) {
            v = min + a*((double)(*u8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s16)(v - 0.5);
            else *rbuf = (s16)(v + 0.5);
          }
        }
        break;
      case SSHORT :
        {
          s16buf = (s16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s16buf++, rbuf++) {
            v = min + a*((double)(*s16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s16)(v - 0.5);
            else *rbuf = (s16)(v + 0.5);
          }
        }
        break;
      case USHORT :
        {
          u16buf = (u16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u16buf++, rbuf++) {
            v = min + a*((double)(*u16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s16)(v - 0.5);
            else *rbuf = (s16)(v + 0.5);
          }
        }
        break;
      case FLOAT :
        {
          r32buf = (r32*)bufferIn;
          for (i=bufferLength-1; i>0; i--, r32buf++, rbuf++) {
            v = a*((double)(*r32buf) - theMin);
            if ( v < 0.0 ) *rbuf = (s16)(v - 0.5);
            else *rbuf = (s16)(v + 0.5);
          }
        }
        break;
      }
    }
    break;

  case USHORT :
    {
      u16 *rbuf = (u16*)bufferOut;
      min = 0;   max = 65535;
      a = ((double)max - (double)min)/(theMax-theMin);
      switch ( typeIn ) {
      default :
          if ( _verbose_ )
              fprintf( stderr, "%s: such input type not handled yet for normalisation.\n", proc );
          return( -1 );
      case SCHAR :
        {
          s8buf = (s8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s8buf++, rbuf++) {
            v = a*((double)(*s8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u16)(0.0);
            else *rbuf = (u16)(v + 0.5);
          }
        }
        break;
      case UCHAR :
        {
          u8buf = (u8*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u8buf++, rbuf++) {
            v = a*((double)(*u8buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u16)(0.0);
            else *rbuf = (u16)(v + 0.5);
          }
        }
        break;
      case SSHORT :
        {
          s16buf = (s16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, s16buf++, rbuf++) {
            v = a*((double)(*s16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u16)(0.0);
            else *rbuf = (u16)(v + 0.5);
          }
        }
        break;
      case USHORT :
        {
          u16buf = (u16*)bufferIn;
          for (i=bufferLength-1; i>0; i--, u16buf++, rbuf++) {
            v = a*((double)(*u16buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u16)(0.0);
            else *rbuf = (u16)(v + 0.5);
          }
        }
        break;
      case FLOAT :
        {
          r32buf = (r32*)bufferIn;
          for (i=bufferLength-1; i>0; i--, r32buf++, rbuf++) {
            v = a*((double)(*r32buf) - theMin);
            if ( v < 0.0 ) *rbuf = (u16)(0.0);
            else *rbuf = (u16)(v + 0.5);
          }
        }
        break;
      }
    }
    break;

  }
  return( 1 );
}
