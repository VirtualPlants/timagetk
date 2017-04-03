/*************************************************************************
 * vt_copy.c - copie d'images
 *
 * $Id: vt_copy.c,v 1.6 2003/07/04 08:17:56 greg Exp $
 *
 * DESCRIPTION: 
 *
 *
 *
 *
 *
 * AUTHOR:
 * Gregoire Malandain
 *
 * CREATION DATE:
 * 1996
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 * - VT_CopyImage() : Thu Jul 22 11:10:07 MET DST 1999 (GM)
 *   traitement du cas ou les types sont identiques, mais pas
 *   les buffers => on utilise memcpy()
 *
 *
 */


#include <convert.h>
#include <vt_copy.h>

/*------- Definition des fonctions statiques ----------*/


/* Procedure de copie d'une image dans une autre.

   Ne realise la copie que si les deux images
   sont compatibles. Si le codage de l'image 
   destination est insuffisant pour coder une valeur,
   on prend la borne la plus proche (minimum ou maximum).
   Le passage de float en int se fait en prenant l'entier
   le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)
   Pas de modification (scaling) des valeurs.

ERROR
   Pas de test de depassement de capacite lors du passage du
   type FLOAT au type INT.

RETURN
   Retourne 0 si les images sont incompatibles, ou si
   elles ne sont pas allouees, ou si le type est inconnu.
*/

int VT_CopyImage( vt_image *theIm /* image to be copied */,
              vt_image *resIm /* image where the copy is to be done */ )
{
    char *proc="VT_CopyImage";

    double *pl = (double*)NULL;
    register double *p;
    int size;
    register int s;
    long int int_min, int_max;

    size_t z, v;

    
    if ( VT_Test2Image( resIm, theIm, proc ) == 0 ) return( 0 );


    /* switch to convert procedures
     */
    v = theIm->dim.v * theIm->dim.x * theIm->dim.y * theIm->dim.z;
    if ( ConvertBuffer( theIm->buf, theIm->type,
                        resIm->buf, resIm->type, v ) != 1 ) {
      VT_Error( "unable to convert buffer", proc );
      return( 0 );
    }
    return( 1 );


    /* obsolete version
     */

    int_min = -2147483647;
    int_max =  2147483647;
    /*--- tests ---*/
    if ( ( resIm->type == theIm->type ) && ( resIm->buf == theIm->buf ) )
            return( 1 );

    /* Thu Jul 22 11:10:07 MET DST 1999 (GM)
     * cas ou les types sont identiques, mais pas 
     * les buffers
     */
    if ( resIm->type == theIm->type ) {
      size = 0;
      switch ( theIm->type ) {
      case SCHAR : size = sizeof( s8 ); break;
      case UCHAR : size = sizeof( u8 ); break;
      case SSHORT : size = sizeof( s16 ); break;
      case USHORT : size = sizeof( u16 ); break;
      case SINT : size = sizeof( i32 ); break;
      case UINT : size = sizeof( u32 ); break;
      case FLOAT : size = sizeof( r32 ); break;
      default :
        VT_Error( "input image type unknown or not supported", proc );
        return( 0 );
      }
      (void)memcpy( resIm->buf, theIm->buf, VT_SizeImage(theIm) );
      return( 1 );
    }



    /*--- plan intermediaire :
          passer par un plan intermediaire n'est pas optimal
          mais rend le code plus agreable a lire (et a corriger) ---*/
    size = theIm->dim.v * theIm->dim.x * theIm->dim.y;
    pl = (double*)VT_Malloc( (unsigned int)(size * sizeof(double)) );
    if ( pl == (double*)NULL ) {
        VT_Error( "allocation failed for auxiliary buffer", proc );
        return( 0 );
    }



    /*--- la copie se fait plan par plan ---*/
#define _COPY_SLICE( TYPE ) {                        \
      TYPE *in = (TYPE*)(theIm->buf);                \
      in += z * size;                                \
      while ( s-- > 0 ) *p++ = (double)(*in++);        \
    }

    for ( z = 0; z < theIm->dim.z; z ++ ) {
        /*--- on remplit le plan intermediaire double ---*/
        p = pl;
        s = size;
        switch ( theIm->type ) {
        case SCHAR :
          _COPY_SLICE( s8 )
          break;
        case UCHAR :
          _COPY_SLICE( u8 )
          break;
        case SSHORT :
          _COPY_SLICE( s16 )
          break;
        case USHORT :
          _COPY_SLICE( u16 )
          break;
        case SINT :
          _COPY_SLICE( i32 )
          break;
        case UINT :
          _COPY_SLICE( u32 )
          break;
        case FLOAT :
          _COPY_SLICE( r32 )
          break;
        case DOUBLE :
          _COPY_SLICE( r64 )
          break;
        default :
            VT_Error( "input image type unknown or not supported", proc );
            VT_Free( (void**)&pl );
            return( 0 );
        }
        /*--- on vide le plan intermediaire double ---*/
        p = pl;
        s = size;
        switch ( resIm->type ) {
        case SCHAR :
            {
            s8 *out;
            out = (s8 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) {
                if ( *p < -128 )     *out++ = (char)-128;
                else if ( *p > 127 ) *out++ = (char)127;
                else {
                    if ( *p < 0 ) *out++ = (char)(*p - 0.5);
                    else          *out++ = (char)(*p + 0.5);
                }
                p ++;
            }}
            break;
        case UCHAR :
            {
            u8 *out;
            out = (u8 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) {
                if ( *p < 0 ) *out++ = (unsigned char)0;
                else if ( *p > 255 ) *out++ = (unsigned char)255;
                else *out++ = (unsigned char)( *p + 0.5 );
                p ++;
            }}
            break;
        case SSHORT :
            {
            s16 *out;
            out = (s16 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) {
                if ( *p < -32768 ) *out++ = (short int)-32768;
                else if ( *p > 32767 ) *out++ = (short int)32767;
                else {
                    if ( *p < 0 ) *out++ = (short int)(*p - 0.5);
                    else          *out++ = (short int)(*p + 0.5);
                }
                p ++;
            }}
            break;
        case USHORT :
            {
            u16 *out;
            out = (u16 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) {
                 if ( *p < 0 ) *out++ = (unsigned short int)0;
                 else if ( *p > 65535 ) *out++ = (unsigned short int)65535;
                 else *out++ = (unsigned short int)( *p + 0.5 );
                 p ++;
            }}
            break;
        case SINT :
            {
            i32 *out;
            out = (i32 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) {
                if ( *p < int_min ) *out++ = (int)int_min;
                else if ( *p > int_max ) *out++ = (int)int_max;
                else {
                    if ( *p < 0 ) *out++ = (int)(*p - 0.5);
                    else          *out++ = (int)(*p + 0.5);
                }
                p ++;
            }}
            break;
        case UINT :
            {
            u32 *out;
            out = (u32 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) {
                if ( *p < 0 ) *out++ = (int)0;
                else if ( *p > int_max ) *out++ = (int)int_max;
                else {
                    if ( *p < 0 ) *out++ = (int)(*p - 0.5);
                    else          *out++ = (int)(*p + 0.5);
                }
                p ++;
            }}
            break;
        case FLOAT :
            {
            r32 *out;
            out = (r32 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) *out++ = (float)(*p++);
            }
            break;
        case DOUBLE :
            {
            r64 *out;
            out = (r64 *)(resIm->buf);
            out += z * size;
            while ( s-- > 0 ) *out++ = (double)(*p++);
            }
            break;
        default :
            VT_Error( "output image type unknown or not supported", proc );
            VT_Free( (void**)&pl );
            return( 0 );
        }
    }
    VT_Free( (void**)&pl );
    return( 1 );
}





#define _NORMA_SLICE( TYPE ) {                                        \
      TYPE *in = (TYPE*)(theIm->buf);                                \
      in += z * size;                                                \
      while ( s-- > 0 ) *p++ = ((double)(*in++) - mi) * c;        \
    }
#define _INTEGER_SLICE( TYPE ) {                                \
      TYPE *out = (TYPE*)(resIm->buf);                                \
      out += z * size;                                                \
      while ( s-- > 0 ) *out++ = (TYPE)((double)0.5 + (*p++));        \
    }
#define _NORMAI_SLICE( TYPE ) {                                        \
      TYPE *in = (TYPE*)(theIm->buf);                                \
      in += z * size;                                                \
      while ( s-- > 0 ) *p++ = min + ((double)(*in++) - mi) * c;\
    }
#define _SINTEGER_SLICE( TYPE ) {                                \
      TYPE *out = (TYPE*)(resIm->buf);                                \
      out += z * size;                                                \
      while ( s-- > 0 ) {                                       \
         if ( *p > 0 ) *out++ = (TYPE)((*p++) + (double)0.5);        \
         else          *out++ = (TYPE)((*p++) - (double)0.5);        \
      } \
    } 
#define _UINTEGER_SLICE( TYPE ) {                                \
      TYPE *out = (TYPE*)(resIm->buf);                                \
      out += z * size;                                                \
      while ( s-- > 0 ) *out++ = (TYPE)((double)0.5 + (*p++));        \
    }
#define _NORMAM_SLICE( TYPE ) {                                \
      TYPE *in = (TYPE*)(theIm->buf);                        \
      in += z * size;                                        \
      while ( s-- > 0 ) *p++ = ((double)(*in++)) * c;        \
    }


/* Procedure de copie avec normalisation d'une image dans une autre.

   Ne realise la copie que si les deux images
   sont compatibles. La normalisation est faite :
   - entre 0 et 127 pour un codage char,
   - entre 0 et 255 pour un codage unsigned char,
   - entre 0 et 32767 (2^15 - 1) pour un codage short int,
   - entre 0 et 65535 (2^16 - 1) pour un codage unsigned short,
   - entre 0 et 2147483647 (2^31 - 1) pour un codage int,
   - entre 0.0 et 1.0 pour un codage float.

   Le passage de float en int se fait en prenant l'entier
   le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)

RETURN
   Retourne 0 si les images sont incompatibles, ou si
   elles ne sont pas allouees, ou si le type est inconnu,
   ou si il n'y pas assez de memoire pour l'allocation 
   d'un tableau intermediaire.
*/

int VT_NormaImage( vt_image *theIm, /* image to be copied */
                   vt_image *resIm  /* image where the copy is to be done */ )
{
        double *pl = (double*)NULL;
        register double *p, c, mi;
        size_t z, size;
        register int s;
        vt_3m m;
        char *proc="VT_NormaImage";

        if ( VT_Test2Image( resIm, theIm, proc ) == 0 ) return( 0 );
        /*--- coefficients de normalisation ---*/
        if ( VT_3m( theIm, (vt_image*)NULL, &m ) == 0 ) {
                VT_Error( "computation of minimum and maximum not possible", proc );
                return( 0 );
        }
        if ( m.min >= m.max ) {
                VT_Error( "normalization of a constant image not possible", proc );
                return( 0 );
        }
        mi = m.min;
        switch ( resIm->type ) {
        case SCHAR :
                c = (double)127.0;        break;
        case UCHAR :
                c = (double)255.0;        break;
        case SSHORT :
                c = (double)32767.0;      break;
        case USHORT :
                c = (double)65535.0;      break;
        case UINT :
        case SINT :
                c = (double)2147483647.0; break;
        case FLOAT :
                c = (double)1.0;          break;
        case DOUBLE :
                c = (double)1.0;          break;
        default :
                VT_Error( "output image type unknown or not supported", proc );
                return( 0 );
        }

        if ( m.max > m.min ) c /= (m.max - m.min);
        /*--- la normalisation de x en y se fait par :
              y = c * ( x - mi )
              le retour sera 0 si l'image est constante ---*/

        if ( _VT_DEBUG_ ) {
          fprintf( stderr, "%s: normalisation    I_out = ( I_in - %g ) * %g\n",
                   proc, mi, c );
        }

        size = theIm->dim.v * theIm->dim.x * theIm->dim.y;
        pl = (double*)VT_Malloc( (unsigned int)(size * sizeof(double)) );
        if ( pl == (double*)NULL ) {
                VT_Error( "allocation failed", proc );
                return( 0 );
        }
        /*--- la normalisation se fait plan par plan ---*/
        for ( z = 0; z < theIm->dim.z; z ++ ) {
                /*--- on remplit le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( theIm->type ) {
                case SCHAR :
                  _NORMA_SLICE( s8 )
                  break;
                case UCHAR :
                  _NORMA_SLICE( u8 )
                  break;
                case SSHORT :
                  _NORMA_SLICE( s16 )
                  break;
                case USHORT :
                  _NORMA_SLICE( u16 )
                  break;
                case SINT :
                  _NORMA_SLICE( i32 )
                  break;
                case UINT :
                  _NORMA_SLICE( u32 )
                  break;
                case FLOAT :
                  _NORMA_SLICE( r32 )
                  break;
                case DOUBLE :
                  _NORMA_SLICE( r64 )
                  break;
                default :
                        VT_Error( "input image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
                /*--- on vide le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( resIm->type ) {
                case SCHAR :
                  _INTEGER_SLICE( s8 )
                  break;
                case UCHAR :
                  _INTEGER_SLICE( u8 )
                  break;
                case SSHORT :
                  _INTEGER_SLICE( s16 )
                  break;
                case USHORT :
                  _INTEGER_SLICE( u16 )
                  break;
                case SINT :
                  _INTEGER_SLICE( i32 )
                  break;
                case UINT :
                  _INTEGER_SLICE( u32 )
                  break;
                case FLOAT :
                        {
                        r32 *out;
                        out = (r32 *)(resIm->buf);
                        out += z * size;
                        while ( s-- > 0 ) *out++ = (float)(*p++);
                        }
                        break;
                case DOUBLE :
                        {
                        r64 *out;
                        out = (r64 *)(resIm->buf);
                        out += z * size;
                        while ( s-- > 0 ) *out++ = (double)(*p++);
                        }
                        break;
                default :
                        VT_Error( "output image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
        }
        VT_Free( (void**)&pl );
        return( 1 );
}





int VT_NormaImageWithinInterval( vt_image *theIm, /* image to be copied */
                               vt_image *resIm,  /* image where the copy is to be done */
                               double min,
                               double max )
{
        double *pl = (double*)NULL;
        register double *p, c, mi;
        size_t z, size;
        register int s;
        vt_3m m;
        char *proc="VT_NormaImageWithinInterval";

        if ( VT_Test2Image( resIm, theIm, proc ) == 0 ) return( 0 );
        if ( min >= max ) {
          VT_Error( "bad parameters", proc );
          return( 0 );
        }

        if ( VT_3m( theIm, (vt_image*)NULL, &m ) == 0 ) {
          VT_Error( "computation of minimum and maximum not possible", proc );
          return( 0 );
        }
        if ( m.min >= m.max ) {
          VT_Error( "normalization of a constant image not possible", proc );
          return( 0 );
        }

        mi = m.min;
        switch ( resIm->type ) {
        case SCHAR :
                c = (double)127.0;        break;
        case UCHAR :
                c = (double)255.0;        break;
        case SSHORT :
                c = (double)32767.0;      break;
        case USHORT :
                c = (double)65535.0;      break;
        case SINT :
                c = (double)2147483647.0; break;
        case FLOAT :
                c = (double)1.0;          break;
        case DOUBLE :
                c = (double)1.0;          break;
        default :
                VT_Error( "output image type unknown or not supported", proc );
                return( 0 );
        }

        c = ( max - min ) / (m.max - m.min);
        /*--- la normalisation de x en y se fait par :
              y = min + c * ( x - mi )
              le retour sera 0 si l'image est constante ---*/

        if ( _VT_DEBUG_ ) {
          fprintf( stderr, "%s: normalisation    I_out = %g + ( I_in - %g ) * %g\n",
                   proc, min, mi, c );
        }

        size = theIm->dim.x * theIm->dim.y;
        pl = (double*)VT_Malloc( (unsigned int)(size * sizeof(double)) );
        if ( pl == (double*)NULL ) {
                VT_Error( "allocation failed", proc );
                return( 0 );
        }
        /*--- la normalisation se fait plan par plan ---*/
        for ( z = 0; z < theIm->dim.z; z ++ ) {
                /*--- on remplit le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( theIm->type ) {
                case SCHAR :
                  _NORMAI_SLICE( s8 )
                  break;
                case UCHAR :
                  _NORMAI_SLICE( u8 )
                  break;
                case SSHORT :
                  _NORMAI_SLICE( s16 )
                  break;
                case USHORT :
                  _NORMAI_SLICE( u16 )
                  break;
                case SINT :
                  _NORMAI_SLICE( i32 )
                  break;
                case FLOAT :
                  _NORMAI_SLICE( r32 )
                  break;
                case DOUBLE :
                  _NORMAI_SLICE( r64 )
                  break;
                default :
                        VT_Error( "input image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
                /*--- on vide le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( resIm->type ) {
                case SCHAR :
                  _SINTEGER_SLICE( s8 )
                  break;
                case UCHAR :
                  _UINTEGER_SLICE( u8 )
                  break;
                case SSHORT :
                  _SINTEGER_SLICE( s16 )
                  break;
                case USHORT :
                  _UINTEGER_SLICE( u16 )
                  break;
                case SINT :
                  _SINTEGER_SLICE( i32 )
                  break;
                case FLOAT :
                        {
                        r32 *out;
                        out = (r32 *)(resIm->buf);
                        out += z * size;
                        while ( s-- > 0 ) *out++ = (float)(*p++);
                        }
                        break;
                case DOUBLE :
                        {
                        r64 *out;
                        out = (r64 *)(resIm->buf);
                        out += z * size;
                        while ( s-- > 0 ) *out++ = (double)(*p++);
                        }
                        break;
                default :
                        VT_Error( "output image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
        }
        VT_Free( (void**)&pl );
        return( 1 );
}





#define _NORMAFROM_SLICE( TYPE ) {                           \
      TYPE *in = (TYPE*)(theIm->buf);                           \
      in += z * size;                                           \
      while ( s-- > 0 ) {                                  \
        if ( (double)*in <= min ) { *p++ = 0;                   \
        } else if (  (double)*in >= max ) { *p++ = c;      \
        } else { *p++ = ((double)(*in) - min) * c / ( max - min ); } \
        in++;                                              \
      }                                                    \
}


int VT_NormaImageFromInterval( vt_image *theIm, /* image to be copied */
                               vt_image *resIm,  /* image where the copy is to be done */
                               double min,
                               double max )
{
        double *pl = (double*)NULL;
        register double *p, c, mi;
        size_t z, size;
        register int s;
        vt_3m m;
        char *proc="VT_NormaImageFromInterval";

        if ( VT_Test2Image( resIm, theIm, proc ) == 0 ) return( 0 );
        if ( min >= max ) {
          VT_Error( "bad parameters", proc );
          return( 0 );
        }

        if ( VT_3m( theIm, (vt_image*)NULL, &m ) == 0 ) {
          VT_Error( "computation of minimum and maximum not possible", proc );
          return( 0 );
        }
        if ( m.min >= m.max ) {
          VT_Error( "normalization of a constant image not possible", proc );
          return( 0 );
        }


        switch ( resIm->type ) {
        case SCHAR :
                c = (double)127.0;        break;
        case UCHAR :
                c = (double)255.0;        break;
        case SSHORT :
                c = (double)32767.0;      break;
        case USHORT :
                c = (double)65535.0;      break;
        case SINT :
                c = (double)2147483647.0; break;
        case FLOAT :
                c = (double)1.0;          break;
        case DOUBLE :
                c = (double)1.0;          break;
        default :
                VT_Error( "output image type unknown or not supported", proc );
                return( 0 );
        }

        mi = min;
        /*--- la normalisation de x en y se fait par :
              y = min + c * ( x - mi )
              le retour sera 0 si l'image est constante ---*/

        if ( _VT_DEBUG_ ) {
          fprintf( stderr, "%s: normalisation    I_out = %g + ( I_in - %g ) * %g\n",
                   proc, min, mi, c/( max - min ) );
        }

        size = theIm->dim.x * theIm->dim.y;
        pl = (double*)VT_Malloc( (unsigned int)(size * sizeof(double)) );
        if ( pl == (double*)NULL ) {
                VT_Error( "allocation failed", proc );
                return( 0 );
        }
        /*--- la normalisation se fait plan par plan ---*/
        for ( z = 0; z < theIm->dim.z; z ++ ) {
                /*--- on remplit le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( theIm->type ) {
                case SCHAR :
                  _NORMAFROM_SLICE( s8 )
                  break;
                case UCHAR :
                  _NORMAFROM_SLICE( u8 )
                  break;
                case SSHORT :
                  _NORMAFROM_SLICE( s16 )
                  break;
                case USHORT :
                  _NORMAFROM_SLICE( u16 )
                  break;
                case SINT :
                  _NORMAFROM_SLICE( i32 )
                  break;
                case FLOAT :
                  _NORMAFROM_SLICE( r32 )
                  break;
                case DOUBLE :
                  _NORMAFROM_SLICE( r64 )
                  break;
                default :
                        VT_Error( "input image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
                /*--- on vide le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( resIm->type ) {
                case SCHAR :
                  _SINTEGER_SLICE( s8 )
                  break;
                case UCHAR :
                  _UINTEGER_SLICE( u8 )
                  break;
                case SSHORT :
                  _SINTEGER_SLICE( s16 )
                  break;
                case USHORT :
                  _UINTEGER_SLICE( u16 )
                  break;
                case SINT :
                  _SINTEGER_SLICE( i32 )
                  break;
                case FLOAT :
                        {
                        r32 *out;
                        out = (r32 *)(resIm->buf);
                        out += z * size;
                        while ( s-- > 0 ) *out++ = (float)(*p++);
                        }
                        break;
                case DOUBLE :
                        {
                        r64 *out;
                        out = (r64 *)(resIm->buf);
                        out += z * size;
                        while ( s-- > 0 ) *out++ = (double)(*p++);
                        }
                        break;
                default :
                        VT_Error( "output image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
        }
        VT_Free( (void**)&pl );
        return( 1 );
}





/* Procedure de copie avec normalisation (par multiplication) d'une image dans une autre.

   Ne realise la copie que si les deux images
   sont compatibles. La normalisation est faite :
   - entre 0 et 127 pour un codage char,
   - entre 0 et 255 pour un codage unsigned char,
   - entre 0 et 32767 (2^15 - 1) pour un codage short int,
   - entre 0 et 65535 (2^16 - 1) pour un codage unsigned short,
   - entre 0 et 2147483647 (2^31 - 1) pour un codage int,
   - entre 0.0 et 1.0 pour un codage float.

   Le passage de float en int se fait en prenant l'entier
   le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)

RETURN
   Retourne 0 si les images sont incompatibles, ou si
   elles ne sont pas allouees, ou si le type est inconnu,
   ou si il n'y pas assez de memoire pour l'allocation 
   d'un tableau intermediaire.
*/

int VT_NormaMultImage( vt_image *theIm, /* image to be copied */
                   vt_image *resIm  /* image where the copy is to be done */ )
{
        double *pl = (double*)NULL;
        register double *p, c;
        size_t z, size;
        register int s;
        vt_3m m;
        char *proc="VT_NormaMultImage";

        if ( VT_Test2Image( resIm, theIm, proc ) == 0 ) return( 0 );
        /*--- coefficients de normalisation ---*/
        if ( VT_3m( theIm, (vt_image*)NULL, &m ) == 0 ) {
                VT_Error( "computation of minimum and maximum not possible", proc );
                return( 0 );
        }
        switch ( resIm->type ) {
        case SCHAR :
                c = (double)127.0;        break;
        case UCHAR :
                c = (double)255.0;        break;
        case SSHORT :
                c = (double)32767.0;      break;
        case USHORT :
                c = (double)65535.0;      break;
        case SINT :
                c = (double)2147483647.0; break;
        case FLOAT :
                c = (double)1.0;          break;
        default :
                VT_Error( "output image type unknown or not supported", proc );
                return( 0 );
        }

        c /= m.max;
        /*--- la normalisation de x en y se fait par :
              y = c * x
              le retour sera 0 si l'image est constante ---*/

        size = theIm->dim.x * theIm->dim.y;
        pl = (double*)VT_Malloc( (unsigned int)(size * sizeof(double)) );
        if ( pl == (double*)NULL ) {
                VT_Error( "allocation failed", proc );
                return( 0 );
        }
        /*--- la normalisation se fait plan par plan ---*/
        for ( z = 0; z < theIm->dim.z; z ++ ) {
                /*--- on remplit le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( theIm->type ) {
                case SCHAR :
                  _NORMAM_SLICE( s8 )
                  break;
                case UCHAR :
                  _NORMAM_SLICE( u8 )
                  break;
                case SSHORT :
                  _NORMAM_SLICE( s16 )
                  break;
                case USHORT :
                  _NORMAM_SLICE( u16 )
                  break;
                case SINT :
                  _NORMAM_SLICE( i32 )
                  break;
                case FLOAT :
                  _NORMAM_SLICE( r32 )
                  break;
                default :
                        VT_Error( "input image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
                /*--- on vide le plan intermediaire double ---*/
                p = pl;
                s = size;
                switch ( resIm->type ) {
                case SCHAR :
                  _INTEGER_SLICE( s8 )
                  break;
                case UCHAR :
                  _INTEGER_SLICE( u8 )
                  break;
                case SSHORT :
                  _INTEGER_SLICE( s16 )
                  break;
                case USHORT :
                  _INTEGER_SLICE( u16 )
                  break;
                case SINT :
                  _INTEGER_SLICE( i32 )
                  break;
                case FLOAT :
                        {
                        r32 *out;
                        out = (r32 *)(resIm->buf);
                        out += z * size;
                        while ( s-- > 0 ) *out++ = (float)(*p++);
                        }
                        break;
                default :
                        VT_Error( "output image type unknown or not supported", proc );
                        VT_Free( (void**)&pl );
                        return( 0 );
                }
        }
        VT_Free( (void**)&pl );
        return( 1 );
}





/* Extraction d'une fenetre d'une image dans une autre.

   L'image resultat est deja allouee.
   On ne recopie que la partie commune (qui peut etre
   plus petite que la fenetre), le reste
   est inchange.
*/

int VT_CopyWindow( vt_image *theIm /* input image */,
                   vt_image *resIm /* output image */,
                   vt_ipt *theCor /* left upper corner in the input image */,
                   vt_ipt *resCor /* left upper corner in the output image */,
                   vt_ipt *dim /* window dimensions */ )
{
    vt_ipt local_dim;
    register int h, i, j, k, iv;
    register double dv;
    int tx, ty, tz, rx, ry, rz, dvx;

    if ( VT_Test1Image( theIm, "VT_CopyWindow" ) == 0 ) return( 0 );
    if ( VT_Test1Image( resIm, "VT_CopyWindow" ) == 0 ) return( 0 );
    if ( theIm->dim.v != resIm->dim.v ) {
        VT_Error( "Input and output images have different V dimensions", "VT_CopyWindow" );
        return( 0 );
    }

    /*--- verifications des coins ---*/
    if ( (theCor->x < 0) || (theCor->x >= (int)theIm->dim.x) ||
         (theCor->y < 0) || (theCor->y >= (int)theIm->dim.y) ||
         (theCor->z < 0) || (theCor->z >= (int)theIm->dim.z) ) {
        VT_Error( "Upper left corner of the first image is not valid", "VT_CopyWindow" );
        return( 0 );
    }
    if ( (resCor->x < 0) || (resCor->x >= (int)resIm->dim.x) ||
         (resCor->y < 0) || (resCor->y >= (int)resIm->dim.y) ||
         (resCor->z < 0) || (resCor->z >= (int)resIm->dim.z) ) {
        VT_Error( "Upper left corner of the second image is not valid", "VT_CopyWindow" );
        return( 0 );
    }
    if ( (dim->x <= 0) || (dim->y <= 0) || (dim->z <= 0) ) {
        VT_Error( "Window dimensions are not valid", "VT_CopyWindow" );
        return( 0 );
    }

    /*--- calcul des dimensions communes ---*/
    local_dim = *dim;
    if ( (theCor->x + local_dim.x - 1) >= (int)theIm->dim.x )
        local_dim.x = theIm->dim.x - theCor->x;
    if ( (theCor->y + local_dim.y - 1) >= (int)theIm->dim.y )
        local_dim.y = theIm->dim.y - theCor->y;
    if ( (theCor->z + local_dim.z - 1) >= (int)theIm->dim.z )
        local_dim.z = theIm->dim.z - theCor->z;
    if ( (resCor->x + local_dim.x - 1) >= (int)resIm->dim.x )
        local_dim.x = resIm->dim.x - resCor->x;
    if ( (resCor->y + local_dim.y - 1) >= (int)resIm->dim.y )
        local_dim.y = resIm->dim.y - resCor->y;
    if ( (resCor->z + local_dim.z - 1) >= (int)resIm->dim.z )
        local_dim.z = resIm->dim.z - resCor->z;
    
    /*--- initialisations ---*/
    dvx = local_dim.x * theIm->dim.v;

    /*--- copie effective ---*/
    switch ( resIm->type ) {
    case UCHAR :
        {
        unsigned char ***resBuf;
        resBuf = (unsigned char***)(resIm->array);

        switch( theIm->type ) {
        case UCHAR :
            {
            unsigned char ***theBuf;
            theBuf = (unsigned char***)(theIm->array);

            for ( k = 0, tz = theCor->z, rz = resCor->z; k < local_dim.z; k ++, tz ++, rz ++ )
            for ( j = 0, ty = theCor->y, ry = resCor->y; j < local_dim.y; j ++, ty ++, ry ++ )
            for ( i = 0, tx = theCor->x, rx = resCor->x; i < dvx; i ++, tx ++, rx ++ )
                resBuf[rz][ry][rx] = theBuf[tz][ty][tx];
            }
            break;
        default :
            for ( k = 0, tz = theCor->z, rz = resCor->z; k < local_dim.z; k ++, tz ++, rz ++ )
            for ( j = 0, ty = theCor->y, ry = resCor->y; j < local_dim.y; j ++, ty ++, ry ++ )
            for ( i = 0, tx = theCor->x, rx = resCor->x; i < local_dim.x; i ++, tx ++, rx ++ )
            for ( h = 0; h < (int)theIm->dim.v; h ++ ) {
                dv = _VT_GetVXYZvalue( theIm, h, tx, ty, tz );
                if ( dv < 0 )        iv = 0;
                else if ( dv > 255 ) iv = 255;
                else                 iv = (int)(dv + 0.5);
                resBuf[rz][ry][rx * theIm->dim.v + h] = (unsigned char)iv;
            }
        }}
        break;
    default :
        for ( k = 0, tz = theCor->z, rz = resCor->z; k < local_dim.z; k ++, tz ++, rz ++ )
        for ( j = 0, ty = theCor->y, ry = resCor->y; j < local_dim.y; j ++, ty ++, ry ++ )
        for ( i = 0, tx = theCor->x, rx = resCor->x; i < local_dim.x; i ++, tx ++, rx ++ )
        for ( h = 0; h <= (int)theIm->dim.v; h ++ ) {
            dv = _VT_GetVXYZvalue( theIm, h, tx, ty, tz );
            _VT_SetVXYZvalue( resIm, h, rx, ry, rz, dv );
        }
    }
    return( 1 );
}


