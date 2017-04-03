
#include <vt_t06noopt.h>

/*-------------------- statistics --------------------*/
#if defined(_STATISTICS_)

extern int TESTS_nb;
extern int EQUIV_nb;
#define GET_VAL(tab,index) ( ++TESTS_nb ? tab[index] : tab[index] )

#define _T06_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  EQUIV_nb++; \
  label_neighbor[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL; i<6; i++ ) \
    if ( label_neighbor[i] == OLD_LABEL ) label_neighbor[i] = NEW_LABEL; \
}

#else

#define GET_VAL(tab,index) tab[index]

#define _T06_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  label_neighbor[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL+1; i<6; i++ ) \
    if ( label_neighbor[i] == OLD_LABEL ) label_neighbor[i] = NEW_LABEL; \
}

#endif
/*----------------------------------------------------*/

#define _SET_EQUIVALENCE( A_CLASS, B_CLASS ) { \
  a_label = label_neighbor[A_CLASS]; \
  b_label = label_neighbor[B_CLASS]; \
  if ( a_label < b_label ) \
    _T06_EQUIVALENCE( a_label, b_label ) \
  else if ( b_label <  a_label ) \
    _T06_EQUIVALENCE( b_label, a_label ) \
  }

/* Compte les 6-composantes connexes dans un 18-voisinage.

   Les points consideres sont ceux dont la valeur est 
   egale a 0. Le point central n'est pas compte
   (il est considere comme appartenant a l'objet).
   On ne compte que les 6-composantes qui sont 
   6-adjacentes au point central.

RETURN
   Retourne le nombre de composantes connexes.

*/

int Compute_NotOptT06( int neighbors[27] )
{
    
    /*--- the neighborhood is numbered as follows 

        0  1  2  -   9 10 11  -  18 19 20
        3  4  5  -  12 13 14  -  21 22 23
        6  7  8  -  15 16 17  -  24 25 26

      A point belongs to the background if its value is 0, else
      it belongs to the object. We consider here the connected
      components of the background.

      The maximal number of 6-connected components 6-adjacent to
      the central point (#13) is obviously 6 (one per each 6-neighbor).
      We attribute one label per each possible connected component.
	
      Those labels are :

	.  .  .  -   .  1  .  -   .  .  .
	.  0  .  -   2  .  3  -   .  5  .
        .  .  .  -   .  4  .  -   .  .  .

      * Connected component #i may exist if and only if its 
        associated 6-neighbor belongs to the background. 

      * Two connected components #i and #j are equivalent if both
	exists and if they are 18-adjacent and if their common
	6-neighbor belongs to the background.
	
    ---*/

  int label_neighbor[6] = {9,9,9,9,9,9};
  register int i, nb;
  register int a_label, b_label;

    /*--- initialization:
      
      if the 6-neighbor belongs to the background (its value is 0),
      its label is set to its possible value, else it is set to 
      some default value (here 9).

      trick #1 : during the initialization, we count the number of 
                 6-neighbors. If its number is 0 or 1, it's the
                 final result too.
    ---*/
    
    nb = 0;

    if ( GET_VAL( neighbors,  4 ) == 0 ) { label_neighbor[0] = 0;   nb++; } 

    if ( GET_VAL( neighbors, 10 ) == 0 ) { label_neighbor[1] = 1;   nb++; } 

    if ( GET_VAL( neighbors, 12 ) == 0 ) { label_neighbor[2] = 2;   nb++; } 

    if ( GET_VAL( neighbors, 14 ) == 0 ) { label_neighbor[3] = 3;   nb++; } 

    if ( GET_VAL( neighbors, 16 ) == 0 ) { label_neighbor[4] = 4;   nb++; } 

    if ( GET_VAL( neighbors, 22 ) == 0 ) { label_neighbor[5] = 5;   nb++; }

    /*--- trick #1 ---*/

    if ( nb <= 1 ) return( nb);

    /*--- Now, we will test the remaining 12 points of the 18-neighborhood.
       
      One of those points play a role if and only if its two 6-neighbors
      (which are 6-neighbors of the central point) belongs to the 
      background.

      We will test first the neighbors of #4, then the neighbors of #10,
      #12, #14, #16 and #22.

    ---*/

    /*--- points #1, #3, #5 and #7 :
          they are examinated if and only if #4 belongs to the background.

	  If one of those points and its other 6-neighbor both belong
	  to the background, the label of the other 6-neighbor is changed
	  to the label of #4 (which is 0).

	  At this point, no conflict between labels can occur.
    ----*/

    if ( GET_VAL( neighbors, 4 ) == 0 ) {

      if ( GET_VAL( neighbors, 1 ) == 0 )
	if ( GET_VAL( neighbors, 10 ) == 0 )
	  _SET_EQUIVALENCE( 0, 1 )
      
      if ( GET_VAL( neighbors, 3 ) == 0 )
	if ( GET_VAL( neighbors, 12 ) == 0 )
	  _SET_EQUIVALENCE( 0, 2 )
      
      if ( GET_VAL( neighbors, 5 ) == 0 )
	if ( GET_VAL( neighbors, 14 ) == 0 )
	  _SET_EQUIVALENCE( 0, 3 )
      
      if ( GET_VAL( neighbors, 7 ) == 0 )
	if ( GET_VAL( neighbors, 16 ) == 0 )
	  _SET_EQUIVALENCE( 0, 4 )
    }

    /*--- points #9, #11 and #19 :
          they are examinated if and only if #10 belongs to the background.

	  A conflict between labels can not occur for #9, but may
	  occur for #11 and #19. In this last case, we look
	  at equivalences.

    ---*/

    if ( GET_VAL( neighbors, 10 ) == 0 ) {

      if ( GET_VAL( neighbors, 9 ) == 0 ) 
	if ( GET_VAL( neighbors, 12 ) == 0 )
	  _SET_EQUIVALENCE( 1, 2 )
      
      if ( GET_VAL( neighbors, 11 ) == 0 )
	if ( GET_VAL( neighbors, 14 ) == 0 )
	  _SET_EQUIVALENCE( 1, 3 )

      if ( GET_VAL( neighbors, 19 ) == 0 )
	if ( GET_VAL( neighbors, 22 ) == 0 )
	  _SET_EQUIVALENCE( 1, 5 )
    }

    /*--- points #15 and #21 :
          they are examinated if and only if #12 belongs to the background.
    ---*/

    if ( GET_VAL( neighbors, 12 ) == 0 ) {
      
      if ( GET_VAL( neighbors, 15 ) == 0 )
	if ( GET_VAL( neighbors, 16 ) == 0 )
	  _SET_EQUIVALENCE( 2, 4 )

      if ( GET_VAL( neighbors, 21 ) == 0 )
	if ( GET_VAL( neighbors, 22 ) == 0 )
	  _SET_EQUIVALENCE( 2, 5 )
    }
    
    /*--- points #17 and #23 :
          they are examinated if and only if #14 belongs to the background.
    ---*/

    if ( GET_VAL( neighbors, 14 ) == 0 ) {

      if ( GET_VAL( neighbors, 17 ) == 0 )
	if ( GET_VAL( neighbors, 16 ) == 0 )
	  _SET_EQUIVALENCE( 3, 4 )

      if ( GET_VAL( neighbors, 23 ) == 0 )
	if ( GET_VAL( neighbors, 22 ) == 0 )
	  _SET_EQUIVALENCE( 3, 5 )
    }

    /*--- point #25 :
          it is examinated if and only if #16 belongs to the background.
    ---*/

    if ( GET_VAL( neighbors, 16 ) == 0 ) {

      if ( GET_VAL( neighbors, 25 ) == 0 )
	if ( GET_VAL( neighbors, 22 ) == 0 )
	  _SET_EQUIVALENCE( 4, 5 )
    }

    /*--- we count the connected components, which are characterized
          by label_neighbor[i] == i
    ---*/

    nb = 0;
    for ( i = 0; i < 6; i ++ )
	if ( label_neighbor[i] == i ) nb ++;
    
    return( nb );
}
