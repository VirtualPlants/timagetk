/*************************************************************************
 * vt_3m.c -
 *
 * $Id: vt_3m.c,v 1.3 2000/08/25 17:58:08 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * ?
 *
 * ADDITIONS, CHANGES
 *
 * - Fri Aug 25 18:15:02 MET DST 2000 (Gregoire Malandain)
 *   Ajout du calcul de l'ecart-type
 *
 * * Thu May  4 2000 (Gregoire Malandain)
 *   add DOUBLE type in VT_3m()
 *
 */


#include <vt_3m.h>
#include <histogram.h>



/*
static int _verbose_ = 1;
*/
/*
static int _debug_ = 0;
*/



int VT_3m( vt_image *im /* image whose min, mean and max have to be computed */,
	   vt_image *mask,
	   vt_3m *m  /* structure containing these 3 numbers */ )
{
  int theDim[3];
  
  if ( (im == (vt_image *)NULL) || (im->buf == (void*)NULL) ) {
    VT_Error( "NULL image or NULL buffer image", "VT_3m" );
    return( 0 );
  }
  
  theDim[0] = im->dim.x * im->dim.v;
  theDim[1] = im->dim.y;
  theDim[2] = im->dim.z;

  m->min = m->moy = m->max = m->ect = (double)0.0;
  
  if ( mask == (vt_image*)NULL ) {
    
    return( minMeanMaxStddevValues( im->buf, im->type, (void*)NULL, TYPE_UNKNOWN, theDim, 
				    &(m->min), &(m->moy), &(m->max), &(m->ect) ) );
  }

  return( minMeanMaxStddevValues( im->buf, im->type, mask->buf, mask->type, theDim, 
				  &(m->min), &(m->moy), &(m->max), &(m->ect) ) );
}
