#ifndef _vt_filters_h_
#define _vt_filters_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_copy.h>




#define _VT_BORD_CONT 1

/* structure pour les operations de filtrage.

STRUCTURE
   Cette structure contient les champs suivants :

    vt_fpt coeff_filter : coefficient (alpha, sigma, ...) pour le filtrage

    vt_ipt order_filter : ordre de derivation du filtre

    vt_ipt type_bord    : type de continuite au bord

    vt_ipt long_bord    : longueur de continuite au bord

FIELDS
    Le champ order_filter contient l'ordre de derivation
    du filtre selon x, y et z. 0 correspond au lissage,
    1 a la derivee premiere, etc.

    Le champ type_bord contient le type de continuite au bord :
    
    _VT_BORD_CONT : on ne considere que les valeurs extremes.

    Le champ long_bord contient le nombre de points a considerer
    pour la continuite.
*/
    
typedef struct vt_filters {
    vt_fpt coeff_filter;
    vt_ipt order_filter;
    vt_ipt type_bord;
    vt_ipt long_bord;
} vt_filters;



extern int  VT_LaplacianWithFD( vt_image *r,  vt_image *t, int d );




extern void VT_Filters( vt_filters *par );
extern int  VT_RecursiveGaussian( vt_image *r,  vt_image *t, vt_filters *p );



#ifdef __cplusplus
}
#endif

#endif  /* _vt_filters_h_ */
