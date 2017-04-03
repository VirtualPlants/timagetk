#ifndef _vt_greynumbers_h_
#define _vt_greynumbers_h_

#ifdef __cplusplus
extern "C" {
#endif


/* Calcule des nombres topologiques en niveaux de gris et en 2D.
 *
 * DESCRIPTION:
 *
 * - On considere d'abord le voisinage seuille strictement
 *   avec la valeur du point central, les points strictement
 *   au-dessus du seuil sont mis a 1 (objet), les autres a 0 
 *   (fond).
 * - tpp est le nombre de 8-composantes connexes de l'objet
 *   dans le 8-voisinage.
 * - tm est le nombre de 4-composantes connexes 
 *   du fond dans un 8-voisinage. On ne compte que les 
 *   4-composantes qui sont 4-adjacentes au point central.
 *
 * - On considere ensuite le voisinage seuille largement
 *   avec la valeur du point central, les points egaux ou
 *   au-dessus du seuil sont mis a 1 (objet), les autres a 0 
 *   (fond).
 * - tp est le nombre de 8-composantes connexes de l'objet
 *   dans le 8-voisinage.
 * - tmm est le nombre de 4-composantes connexes 
 *   du fond dans un 8-voisinage. On ne compte que les 
 *   4-composantes qui sont 4-adjacentes au point central.
 *
   - Le voisinage est numerote comme suit
        0  1  2
        3  4  5
        6  7  8
 */
extern void Compute_allTs_2D( int neighbors[9],
			      int *tp,
			      int *tpp,
			      int *tm,
			      int *tmm );


/* Calcule des nombres topologiques en niveaux de gris et en 3D.
 *
 * DESCRIPTION:
 *
 * - On considere d'abord le voisinage seuille strictement
 *   avec la valeur du point central, les points strictement
 *   au-dessus du seuil sont mis a 1 (objet), les autres a 0 
 *   (fond).
 * - tpp est le nombre de 26-composantes connexes de l'objet
 *   dans le 26-voisinage.
 * - tm est le nombre de 6-composantes connexes 
 *   du fond dans un 26-voisinage. On ne compte que les 
 *   6-composantes qui sont 6-adjacentes au point central.
 *
 * - On considere ensuite le voisinage seuille largement
 *   avec la valeur du point central, les points egaux ou
 *   au-dessus du seuil sont mis a 1 (objet), les autres a 0 
 *   (fond).
 * - tp est le nombre de 26-composantes connexes de l'objet
 *   dans le 26-voisinage.
 * - tmm est le nombre de 6-composantes connexes 
 *   du fond dans un 26-voisinage. On ne compte que les 
 *   6-composantes qui sont 6-adjacentes au point central.
 *
   - Le voisinage est numerote comme suit
        0  1  2  -   9 10 11  -  18 19 20
        3  4  5  -  12 13 14  -  21 22 23
        6  7  8  -  15 16 17  -  24 25 26
 */
extern void Compute_allTs_3D( int neighbors[27],
			      int *tp,
			      int *tpp,
			      int *tm,
			      int *tmm );



#ifdef __cplusplus
}
#endif

#endif /* _vt_greynumbers_h_ */
