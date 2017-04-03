#ifndef _vt_caracterisation_h_
#define _vt_caracterisation_h_



#ifdef __cplusplus
extern "C" {
#endif



#include <vt_common.h>
#include <vt_neighborhood.h>

#include <vt_amincir.h>
#include <vt_etoilebarre.h>

#define VT_JUNCTION 255
#define VT_SIMPLE   200
#define VT_CC       100

        /*--- Pour les 2^26 configurations possibles du voisinage, on a 

	  Cbarre   Cetoile   Nombre de Conf.   Interpretation

	     0        1         1048576      : point interieur volume

	     1        0               1      : point isole
	     1        1        25985144      : point simple (frontiere)
	     1        2         3626396      : point de courbe (dont 3626203 faux points de courbes)
	     1        3          651784      : point de jonction entre 3 courbes
	     1        4          106054      : point de jonction entre 4 courbes
	     1        5           13512      : point de jonction entre 5 courbes
	     1        6            1180      : point de jonction entre 6 courbes
	     1        7              56      : point de jonction entre 7 courbes
	     1        8               1      : point de jonction entre 8 courbes

	     2        1        26073184      : point de surface
	     2        2          904832      : point de jonction entre 1 surface et 1 courbe
	     2        3           49216      : point de jonction entre 1 surface et 2 courbes
	     2        4            2688      : point de jonction entre 1 surface et 3 courbes
	     2        5              96      : point de jonction entre 1 surface et 4 courbes

	     3        1         7748352      : point de jonction entre 3 surfaces
	     3        2           63744      : point de jonction entre 3 surfaces et 1 courbe
	     3        3             768      : point de jonction entre 3 surfaces et 2 courbes

	     4        1          804352      : point de jonction entre 4 surfaces
	     4        2            1024      : point de jonction entre 4 surfaces et 1 courbe

	     5        1           27648      : point de jonction entre 5 surfaces

	     6        1             256      : point de jonction entre 6 surfaces
                              ----------
			       67108864
		                                                                                 ---*/

#define VT_LABEL_01   1   /* point interieur volume                               */
#define VT_LABEL_10   2   /* point isole                                          */
#define VT_LABEL_11   3   /* point simple (frontiere)                             */
#define VT_LABEL_12   4   /* point de courbe                                      */
#define VT_LABEL_13   5   /* point de jonction entre 3 courbes                    */
#define VT_LABEL_14   6   /* point de jonction entre 4 courbes                    */
#define VT_LABEL_15   7   /* point de jonction entre 5 courbes                    */
#define VT_LABEL_16   8   /* point de jonction entre 6 courbes                    */
#define VT_LABEL_17   9   /* point de jonction entre 7 courbes                    */
#define VT_LABEL_18  10   /* point de jonction entre 8 courbes                    */
#define VT_LABEL_21  11   /* point de surface                                     */
#define VT_LABEL_22  12   /* point de jonction entre 1 surface et 1 courbe        */
#define VT_LABEL_23  13   /* point de jonction entre 1 surface et 2 courbes       */
#define VT_LABEL_24  14   /* point de jonction entre 1 surface et 3 courbes       */
#define VT_LABEL_25  15   /* point de jonction entre 1 surface et 4 courbes       */
#define VT_LABEL_31  16   /* point de jonction entre 3 surfaces                   */
#define VT_LABEL_32  17   /* point de jonction entre 3 surfaces et 1 courbe       */
#define VT_LABEL_33  18   /* point de jonction entre 3 surfaces et 2 courbes      */
#define VT_LABEL_41  19   /* point de jonction entre 4 surfaces                   */
#define VT_LABEL_42  20   /* point de jonction entre 4 surfaces et 1 courbe       */
#define VT_LABEL_51  21   /* point de jonction entre 5 surfaces                   */
#define VT_LABEL_61  22   /* point de jonction entre 6 surfaces                   */
#define VT_LABEL_11J 31   /* point frontiere voisin d'une jonction entre surfaces */
#define VT_LABEL_12J 32   /* point jonction entre plusieurs courbes               */
#define VT_LABEL_21J 33   /* point jonction entre plusieurs surfaces              */
#define VT_LABEL_22J 34   /* point de jonction entre plusieurs surfaces et 1 courbe        */
#define VT_LABEL_23J 35   /* point de jonction entre plusieurs surfaces et 2 courbes       */
#define VT_LABEL_24J 36   /* point de jonction entre plusieurs surfaces et 3 courbes       */
#define VT_LABEL_25J 37   /* point de jonction entre plusieurs surfaces et 4 courbes       */

typedef struct vt_caracterisation{
    int bool_extended_junctions;
    Neighborhood connexite;
} vt_caracterisation;





extern void VT_InitCaracterisation( vt_caracterisation *par );
extern int  VT_Caracterisation( vt_image *im1, vt_image *im2, vt_caracterisation *par );
extern int  VT_CaracterisationComplete( vt_image *im1, vt_image *im2, vt_caracterisation *par );
extern int  VT_CaracterisationCurves( vt_image *im1, vt_image *im2 );

#ifdef __cplusplus
}
#endif

#endif
