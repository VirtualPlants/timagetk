/*************************************************************************
 * bal-image-tools.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
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

#include <bal-image-tools.h>

static int _verbose_ = 1;



void BAL_SetVerboseInBalImageTools( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalImageTools(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalImageTools(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}




/*--------------------------------------------------
 *
 * IMAGE GRID
 *
 --------------------------------------------------*/

int BAL_DrawGrid( bal_image *theIm,
                  bal_integerPoint *offset,
                  bal_integerPoint *spacing )
{
  char *proc = "BAL_DrawGrid";
  size_t i, j, k;

  switch( theIm->type ) {

  default : 
    if ( _verbose_ ) 
      fprintf( stderr, "%s: such type not handled yet\n", proc );
    return( -1 );
    
  case UCHAR :
    {
      unsigned char ***theBuf = (unsigned char ***)(theIm->array);
      for ( k=0; k<theIm->nplanes; k++ )
      for ( j=0; j<theIm->nrows; j++ )
      for ( i=0; i<theIm->ncols; i++ )
        theBuf[k][j][i] = 0;
      
      if ( offset->x >= 0 && spacing->x >= 1 && theIm->ncols > 1 ) {
        for ( k=0; k<theIm->nplanes; k++ )
        for ( j=0; j<theIm->nrows; j++ )
        for ( i=offset->x; i<theIm->ncols; i+=spacing->x)
          theBuf[k][j][i] = 255;
      }

      if ( offset->y >= 0 && spacing->y >= 1 && theIm->nrows > 1 ) {
        for ( k=0; k<theIm->nplanes; k++ )
        for ( j=offset->y; j<theIm->nrows; j+=spacing->y )
        for ( i=0; i<theIm->ncols; i++ )
          theBuf[k][j][i] = 255;
      }
 
      if ( offset->z >= 0 && spacing->z >= 1 && theIm->nplanes > 1 ) {
        for ( k=offset->z; k<theIm->nplanes; k+=spacing->z )
        for ( j=0; j<theIm->nrows; j++ )
        for ( i=0; i<theIm->ncols; i++ )
          theBuf[k][j][i] = 255;
      }

    }
    break;

  }

  return( 1 );

}
