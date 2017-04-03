/*************************************************************************
 * vt_3m.h -
 *
 * $Id: vt_3m.h,v 1.2 2000/08/25 17:58:08 greg Exp $
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
 *   Ajout de l'ecart-type
 *
 *
 */

#ifndef _vt_3m_h_
#define _vt_3m_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_image.h>

typedef struct vt_3m {
  double min;
  double max;
  double moy;
  double ect;
} vt_3m;

/* Procedure de calcul des minimum, moyenne et maximum.
 *
 * Le calcul a lieu sans normalisation par rapport au codage.
 *
 * RETURN:
 *  Retourne 0 si l'image n'est pas correcte.
 */
extern int VT_3m( vt_image *im, /* image whose min, mean and max have to be computed */
		  vt_image *mask,
		    vt_3m *m /* structure containing these 3 numbers */ );

#ifdef __cplusplus
}
#endif

#endif /* _vt_3m_h_ */
