/****************************************************
 * t04t08.h - 
 *
 * $Id: t04t08.h,v 1.1 2000/08/08 14:48:27 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */


#ifndef _t04t08_h_
#define _t04t08_h_

#ifdef __cplusplus
extern "C" {
#endif



/* Compte les 4-composantes connexes dans un 8-voisinage.

   Les points consideres sont ceux dont la valeur est 
   egale a 0. Le point central n'est pas compte
   (il est considere comme appartenant a l'objet).
   On ne compte que les 4-composantes qui sont 
   4-adjacentes au point central.

RETURN
   Retourne le nombre de composantes connexes.

*/

#if defined(_STATISTICS_)
  extern int Compute_T04( int neighbors[9], int *TESTS_nb );
#else
  extern int Compute_T04( int neighbors[9] );
#endif

/* Compte les 8-composantes connexes dans un 8-voisinage.

DESCRIPTION:
   - Les points consideres sont ceux dont la valeur est 
   differente de 0. Le point central n'est pas compte
   (il est considere comme appartenant au fond).

   - Le voisinage est numerote comme suit
        0  1  2 
        3  4  5 
        6  7  8 

RETURN:
   Retourne le nombre de composantes connexes.

*/

#if defined(_STATISTICS_)
  extern int Compute_T08( int neighbors[9], int *TESTS_nb );
#else
  extern int Compute_T08( int neighbors[9] );
#endif




/* Evalue t04 et t08 dans un 8-voisinage.

DESCRIPTION:
   - t08 est le nombre de 8-composantes connexes
   de l'objet dans un 8-voisinage.
   Les points consideres sont ceux dont la valeur est 
   differente de 0. Le point central n'est pas compte
   (il est considere comme appartenant au fond).

   - t04 est le nombre de 4-composantes connexes 
   du fond dans un 8-voisinage.
   Les points consideres sont ceux dont la valeur est 
   egale a 0. Le point central n'est pas compte
   (il est considere comme appartenant a l'objet).
   On ne compte que les 4-composantes qui sont 
   4-adjacentes au point central.

   - Le voisinage est numerote comme suit
        0  1  2
        3  4  5
        6  7  8

*/
#if defined(_STATISTICS_)
  extern void Compute_T04_and_T08( int neighbors[9], int *t04, int *t08, int *TESTS_nb );
#else
  extern void Compute_T04_and_T08( int neighbors[9], int *t04, int *t08 );
#endif



#ifdef __cplusplus
}
#endif

#endif /* _t04t08_h_ */
