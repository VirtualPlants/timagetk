/*************************************************************************
 * skiz.h -
 *
 * $Id: vt_skiz.h,v 1.2 2001/04/09 10:24:47 greg Exp $
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
 * ???
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 * - Mon Mar 19 19:16:47 MET 2001
 *   L'image de distance est passee en parametre.
 *
 *
 */

#ifndef _vt_skiz_h_
#define _vt_skiz_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_image.h>
#include <vt_inrimage.h>

/* Squelette par zone d'influences avec contraintes.
 *
 * DESCRIPTION:
 * - On propage les etiquettes dans l'image masque *imMask
 * de telle sorte que chaque point de l'image masque 
 * ait l'etiquette du point etiquette le plus proche 
 * (au sens de la distance du chanfrein) de 
 * l'image des etiquettes *imLabels.
 *
 * - La  distance est calculee a l'interieur de l'image
 * masque, c'est-a-dire dans un espace non-convexe
 * a priori.
 *
 * - De plus, une contrainte de 6-connexite a ete ajoutee pour 
 * la propagation. Un point ne peut avoir comme etiquette 
 * que l'etiquette d'un de ses 6-voisins. Une 6-composante
 * ne contenant pas d'etiquette sera donc eliminee. Cela permet
 * d'eviter le passage d'une etiquette par une 18- ou une 26-connexion
 * seulement.
 *
 * - La contrainte precedente est une approximation. Il faudrait 
 * verifier qu'il existe un 6-chemin compose de points de meme etiquette
 * entre le point courant et le point utilise pour mettre a jour la 
 * distance, et non verifier que le point utilise pour mettre a jour la 
 * distance a la meme etiquette qu'un 6-voisin du point central 
 * (si je me fais bien comprendre). Aussi, il est possible de limiter 
 * l'expansion au cours d'une iteration en fixant une distance maximale
 * par le biais d'un rayon, voir VT_SetRadiusOfConstrainedSkiz().
 * La distance maximale est egale a inc[0]*(ce rayon).
 * Lorsqu'il n'y a plus de changements pour cette distance, 
 * elle est incrementee de inc[0]*(ce rayon).
 *
 * - A chaque iteration on realise une propagation FORWARD et BACKWARD
 * et on compte le nombre de points qui ont vu leur etiquette modifiee.
 * On recommence tant que ce nombre n'est pas nul.
 *
 * PARAMETRES:
 * - les images doivent avoir meme tailles et etre toutes de type
 *   unsigned char.
 * - le masque pour la distance du chanfrein est de taille 3x3(x3)
 *   ou 5x5(x5). Sinon il est choisi a 3x3(x3).
 * - int *inc : tableau d'increments
 *   dans le cas d'une distance 3x3x3 les increments dans le 
 *   voisinage sont (on donne l'indice de l'increment dans le
 *   tableau int *inc)
 * -         1 0 1         4 3 4
 * -   z=0   0 x .   z=1   3 2 3
 * -         . . .         4 3 4 
 * - dans le cas d'une distance 5x5x5 les increments dans le 
 *   voisinage sont (on donne l'indice de l'increment dans le
 *   tableau int *inc)
 * -       . 2 . 2 .         8 7 6 7 8          . 11 . 11  .
 * -       2 1 0 1 2         7 5 4 5 7         11 10 9 10 11
 * - z=0   . 0 x . .   z=1   6 4 3 4 6   z=2    .  9 .  9  .  
 * -       . . . . .         7 5 4 5 7         11 10 9 10 11
 * -       . . . . .         8 7 6 7 8          . 11 . 11  .
 * -
 * - Ne pas utiliser les memes coefficients dans les differents plans
 *   (ici les masques forward) permet d'introduire une anisotropie eventuelle
 *   dans la distance.
 *
 */
extern int VT_ConstrainedSkizWithChamfer( vt_image *imLabels /* image de labels */,
					  vt_image *imMask /* image masque */,
					  vt_image *imOutput /*image resultat */,
					  int chamferSize /* 3 ou 5 */,
					  int *inc /* increments pour la distance du chanfrein */ );
extern int VT_ConstrainedSkizInDistanceMapWithChamfer( vt_image *imLabels, 
						vt_image *imMask,
						vt_image *imDist,
						vt_image *imOutput,
						int chamferSize,
						int *inc );

/* Fixe le rayon qui limite l'expansion au cours d'une iteration.
 *
 * DESCRIPTION:
 * A cause de l'approximation decrite dans 
 * VT_ConstrainedSkizWithChamfer() a propos de la
 * propagation en 6-connexite, il peut apparaitre
 * des configurations ou il n'existe pas de 6-chemin
 * entre le point courant et le point servant a mettre 
 * a jour la distance, mais qu'un 6-voisin ait le meme
 * label que ce point. Ceci est du egalement au sens de
 * propagation des distances. 
 *
 * Aussi il est parfois necessaire de limiter la
 * propagation au cours d'une iteration. Si le rayon est 
 * positif, la propagation est limitee aux distances
 * inferieures a inc[0]*rayon. Sinon elle n'est pas
 * limitee.
 * 
 */
extern void VT_SetRadiusOfConstrainedSkiz( int radius );


/* Ecriture de l'image de distance.
 *
 * DESCRIPTION:
 * Permet de donner un nom a l'image de distance
 * (image temporaire) qui sera alors ecrite
 * a la fin de la procedure, quand les distances
 * n'evolueront plus.
 */
extern void VT_SetSkizDistanceImageName( char *name );

/* Desactive l'ecriture de l'image de distance.
 *
 * DESCRIPTION:
 * Met a False le flag commandant l'ecriture de 
 * l'image de distance. Pour reactiver celle-ci,
 * il faudra passer par VT_SetSkizDistanceImageName().
 */
extern void VT_DontWriteSkizDistanceImage( );

#ifdef __cplusplus
}
#endif

#endif /* _vt_skiz_h_ */
