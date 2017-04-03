#ifndef _vt_t06t26_simple_h_
#define _vt_t06t26_simple_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Determine si un point est simple ou pas.

DESCRIPTION:
   - On considere la 26-connexite pour l'objet
   c'est-a-dire les points de valeurs non nulle,
   et la 6-connexite pour le fond,
   c'est-a-dire les points de valeurs nulles.

   - Le voisinage est numerote comme suit
        0  1  2  -   9 10 11  -  18 19 20
        3  4  5  -  12 13 14  -  21 22 23
        6  7  8  -  15 16 17  -  24 25 26

RETURN:
   - 1 si le point est simple

   - 0 sinon

*/
extern int Compute_Simple( int neighbors[27] );

#ifdef __cplusplus
}
#endif

#endif /* _vt_t06t26_simple_h_ */
