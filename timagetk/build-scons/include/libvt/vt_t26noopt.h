#ifndef _vt_t26noopt_h_
#define _vt_t26noopt_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Compte les 26-composantes connexes dans un 26-voisinage.

DESCRIPTION:
   - Les points consideres sont ceux dont la valeur est 
   differente de 0. Le point central n'est pas compte
   (il est considere comme appartenant au fond).

   - Le voisinage est numerote comme suit
        0  1  2  -   9 10 11  -  18 19 20
        3  4  5  -  12 13 14  -  21 22 23
        6  7  8  -  15 16 17  -  24 25 26


RETURN:
   Retourne le nombre de composantes connexes.

*/
extern int Compute_NotOptT26( int neighbors[27] );



#ifdef __cplusplus
}
#endif

#endif /* _vt_t26noopt_h_ */
