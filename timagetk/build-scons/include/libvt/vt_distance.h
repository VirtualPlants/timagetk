#ifndef _vt_distance_h_
#define _vt_distance_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

/*------------------------------------------------------------
* parameters of distance computations
* 
*------------------------------------------------------------*/
typedef struct vt_distance {
    float seuil;
    int type;        /* type de distance */
    int dim;         /* mode 2D ou 3D */
    int inc[5];      /* les 5 increments pour le calcul, ils concernent */
                     /* inc[0] : voisin par une face dans le plan       */
                     /* inc[1] : voisin par une arete dans le plan      */
	             /* inc[2] : voisin par une face dans le plan sup.  */
	             /* inc[3] : voisin par une arete dans le plan sup. */
	             /* inc[4] : voisin par un sommet dans le plan sup. */
    int type_image_eucmap; /* type des images (vecteur) pour l'euclidean distance mapping */
} vt_distance;

#define VT_UINFINI 65535
#define VT_SINFINI 26753 
/* on definit VT_SINFINI a 26753 au lieu de 32767, car 3*32767*32767 = 3221028867
   depasse les capacites de l'entier 2^31-1 = 2147483647. on choisit donc
   l'infini comme (int)sqrt(2147483647/3)-1
*/

#define VT_DIST_4          1   /* compatible avec VT_4CONNEXE (vt_common.h)  */
#define VT_DIST_6_2D       1
#define VT_DIST_8          2   /* compatible avec VT_8CONNEXE (vt_common.h)  */
#define VT_DIST_10_2D      2   
#define VT_DIST_18_2D      2   
#define VT_DIST_26_2D      2
#define VT_DIST_6          3   /* compatible avec VT_6CONNEXE (vt_common.h)  */
#define VT_DIST_10         4   /* compatible avec N10 (vt_common.h) */
#define VT_DIST_18         5   /* compatible avec N18 (vt_common.h) */
#define VT_DIST_26         6   /* compatible avec N26 (vt_common.h) */
#define VT_DIST_CHMFR_2D  10
#define VT_DIST_CHMFR3_2D 11
#define VT_DIST_CHMFR2_2D 12
#define VT_DIST_CHMFR     13
#define VT_DIST_CHMFR2    14
#define VT_DIST_CHMFR3    15
#define VT_DIST_USER      16
#define VT_DIST_EUCLI     20
#define VT_DIST_EUCLI_2   21

extern void VT_Distance( vt_distance *p );
extern int  VT_Dist( vt_image *resIm, vt_image *theIm, vt_distance *par );

#include <vt_eucmap.h>
#include <vt_eucmapsc.h>
#include <vt_eucmapss.h>


#ifdef __cplusplus
}
#endif

#endif /* _vt_distance_h_ */
