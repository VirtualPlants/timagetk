
#include <vt_t26noopt.h>

#define _FALSE_ 0
#define _TRUE_  1

/*-------------------- statistics --------------------*/
#if defined(_STATISTICS_)

extern int TESTS_nb;
extern int EQUIV_nb;
#define GET_VAL(tab,index) ( ++TESTS_nb ? tab[index] : tab[index] )
#define GET_OLDLABEL(index) EQUIV_nb++; old_label = label_vertex[ index ]

#define _TWO_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  EQUIV_nb++; \
  label_vertex[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL+1; i<8; i++ ) \
    if ( label_vertex[i] == OLD_LABEL ) label_vertex[i] = NEW_LABEL; \
}

#else

#define _TWO_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  label_vertex[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL+1; i<8; i++ ) \
    if ( label_vertex[i] == OLD_LABEL ) label_vertex[i] = NEW_LABEL; \
}

#define GET_VAL(tab,index) tab[index]
#define GET_OLDLABEL(index) old_label = label_vertex[ index ]

#endif
/*----------------------------------------------------*/


#define _T26_EQUIVALENCE( NEW_LABEL, OLD_CLASS ) { \
  GET_OLDLABEL(OLD_CLASS); \
  label_vertex[old_label] = NEW_LABEL; \
  for ( i=old_label+1; i<8; i++ ) \
    if ( label_vertex[i] == old_label ) label_vertex[i] = NEW_LABEL; \
}

#define _SET_EQUIVALENCE( A_CLASS, B_CLASS ) { \
  a_label = label_vertex[A_CLASS]; \
  b_label = label_vertex[B_CLASS]; \
  label_exists[A_CLASS] = label_exists[B_CLASS] = _TRUE_; \
  if ( a_label < b_label ) \
    _TWO_EQUIVALENCE( a_label, b_label ) \
  else if ( b_label <  a_label ) \
    _TWO_EQUIVALENCE( b_label, a_label ) \
  }






int Compute_NotOptT26( int neighbors[27] )
{
    /*--- the neighborhood is numbered as follows 

        0  1  2  -   9 10 11  -  18 19 20
        3  4  5  -  12 13 14  -  21 22 23
        6  7  8  -  15 16 17  -  24 25 26

      A point belongs to the background if its value is 0, else
      it belongs to the object. We consider here the connected
      components of the object.

      The maximal number of 26-connected components is obviously 8 
      (one per each vertex). We attribute one label per each 
      possible connected component.
	
      Those labels are :

	0  .  1  -   .  .  .  -   4  .  5
	.  .  .  -   .  .  .  -   .  .  .
        2  .  3  -   .  .  .  -   6  .  7
	
      * Connected component #i may exist if and only if one 
        among 8 points belongs to the object. Those points
	are respectively :
	for component #0, points  0  1  3  4  9 10 12 13
	for component #1, points  1  2  4  5 10 11 13 14
	for component #2, points  3  4  6  7 12 13 15 16
	for component #3, points  4  5  7  8 13 14 16 17
	for component #4, points              9 10 12 13 18 19 21 22
	for component #5, points             10 11 13 14 19 20 22 23
	for component #6, points             12 13 15 16 21 22 24 25
	for component #7, points             13 14 16 17 22 23 25 26
	
      *	If a object point belongs to more than one component, these
        components are equivalent.

      ---*/

    int label_vertex[8], label_exists[8];
    register int i, min_label, nb, old_label;
    register int a_label, b_label;

    /*--- initialization:

          Each label is set to its possible value, but we do not
	  known if each component will exist.
    ---*/

    for ( i = 0; i < 8; i ++ ) {
	label_vertex[i] = i;
	label_exists[i] = _FALSE_;
    }

    /*--- Let consider component #0 and its possible equivalences:

          we will examinate the possible points of #0 which
	  may generate an equivalence with an other component:
	  - 3 points (4, 10, 12) yield an equivalence between
	    four components (e.g. 4 yields an equivalence between
	    #0, #1, #2 and #3) 
	  - 3 points (1, 3, 9) yield an equivalence between
	    two components (e.g. 1 yields an equivalence between
	    #0 and #1)

	  At this point, no conflict between labels can occur.
    ---*/

    if ( GET_VAL( neighbors, 4 ) != 0 ) {
      min_label = label_vertex[0];
      if ( label_vertex[1] < min_label ) min_label = label_vertex[1];
      if ( label_vertex[2] < min_label ) min_label = label_vertex[2];
      if ( label_vertex[3] < min_label ) min_label = label_vertex[3];
      if ( label_vertex[0] != min_label ) _T26_EQUIVALENCE( min_label, 0 )
      if ( label_vertex[1] != min_label ) _T26_EQUIVALENCE( min_label, 1 )
      if ( label_vertex[2] != min_label ) _T26_EQUIVALENCE( min_label, 2 )
      if ( label_vertex[3] != min_label ) _T26_EQUIVALENCE( min_label, 3 )
      label_exists[0] = label_exists[1] = label_exists[2] = label_exists[3] = _TRUE_;
    } else {
      if ( GET_VAL( neighbors, 1 ) != 0 )
	  _SET_EQUIVALENCE( 0, 1 )
      if ( GET_VAL( neighbors, 3 ) != 0 )
	  _SET_EQUIVALENCE( 0, 2 )
      if ( GET_VAL( neighbors, 5 ) != 0 )
	  _SET_EQUIVALENCE( 1, 3 )
      if ( GET_VAL( neighbors, 7 ) != 0 )
	  _SET_EQUIVALENCE( 2, 3 )
    }

    if ( GET_VAL( neighbors, 10 ) != 0 ) {
      min_label = label_vertex[0];
      if ( label_vertex[1] < min_label ) min_label = label_vertex[1];
      if ( label_vertex[4] < min_label ) min_label = label_vertex[4];
      if ( label_vertex[5] < min_label ) min_label = label_vertex[5];
      if ( label_vertex[0] != min_label ) _T26_EQUIVALENCE( min_label, 0 )
      if ( label_vertex[1] != min_label ) _T26_EQUIVALENCE( min_label, 1 )
      if ( label_vertex[4] != min_label ) _T26_EQUIVALENCE( min_label, 4 )
      if ( label_vertex[5] != min_label ) _T26_EQUIVALENCE( min_label, 5 )
      label_exists[0] = label_exists[1] = label_exists[4] = label_exists[5] = _TRUE_;
    } else {
      if ( GET_VAL( neighbors, 9 ) != 0 )
	  _SET_EQUIVALENCE( 0, 4 )
      if ( GET_VAL( neighbors, 11 ) != 0 )
	  _SET_EQUIVALENCE( 1, 5 )
      if ( GET_VAL( neighbors, 19 ) != 0 ) 
	  _SET_EQUIVALENCE( 4, 5 )
    }

    if ( GET_VAL( neighbors, 12 ) != 0 ) {
      min_label = label_vertex[0];
      if ( label_vertex[2] < min_label ) min_label = label_vertex[2];
      if ( label_vertex[4] < min_label ) min_label = label_vertex[4];
      if ( label_vertex[6] < min_label ) min_label = label_vertex[6];
      if ( label_vertex[0] != min_label ) _T26_EQUIVALENCE( min_label, 0 )
      if ( label_vertex[2] != min_label ) _T26_EQUIVALENCE( min_label, 2 )
      if ( label_vertex[4] != min_label ) _T26_EQUIVALENCE( min_label, 4 )
      if ( label_vertex[6] != min_label ) _T26_EQUIVALENCE( min_label, 6 )
      label_exists[0] = label_exists[2] = label_exists[4] = label_exists[6] = _TRUE_;
    } else {
      if ( GET_VAL( neighbors, 15 ) != 0 )
	  _SET_EQUIVALENCE( 2, 6 )
      if ( GET_VAL( neighbors, 21 ) != 0 )
	  _SET_EQUIVALENCE( 4, 6 )
     
    }

    /*--- Let consider component #1 and its possible equivalences:

          we will examinate the possible points of #1 which
	  may generate an equivalence with an other component:
	  - 1 point (14) yields an equivalence between
	    four components 
	  - 2 points (5, 11) yield an equivalence between
	    two components
    ---*/

    if ( GET_VAL( neighbors, 14 ) != 0 ) {
	min_label = label_vertex[1];
	if ( label_vertex[3] < min_label ) min_label = label_vertex[3];
	if ( label_vertex[5] < min_label ) min_label = label_vertex[5];
	if ( label_vertex[7] < min_label ) min_label = label_vertex[7];
	if ( label_vertex[1] != min_label ) _T26_EQUIVALENCE( min_label, 1 )
	if ( label_vertex[3] != min_label ) _T26_EQUIVALENCE( min_label, 3 )
	if ( label_vertex[5] != min_label ) _T26_EQUIVALENCE( min_label, 5 )
	if ( label_vertex[7] != min_label ) _T26_EQUIVALENCE( min_label, 7 )
	label_exists[1] = label_exists[3] = label_exists[5] = label_exists[7] = _TRUE_;
    } else {
      if ( GET_VAL( neighbors, 17 ) != 0 )
           _SET_EQUIVALENCE( 3, 7 )
      if ( GET_VAL( neighbors, 23 ) != 0 )
           _SET_EQUIVALENCE( 5, 7 )
    }

    /*--- Let consider component #2 and its possible equivalences:

          we will examinate the possible points of #2 which
	  may generate an equivalence with an other component:
	  - 1 point (16) yields an equivalence between
	    four components 
	  - 2 points (7, 15) yield an equivalence between
	    two components
    ---*/

    if ( GET_VAL( neighbors, 16 ) != 0 ) {
	min_label = label_vertex[2];
	if ( label_vertex[3] < min_label ) min_label = label_vertex[3];
	if ( label_vertex[6] < min_label ) min_label = label_vertex[6];
	if ( label_vertex[7] < min_label ) min_label = label_vertex[7];
	if ( label_vertex[2] != min_label ) _T26_EQUIVALENCE( min_label, 2 )
	if ( label_vertex[3] != min_label ) _T26_EQUIVALENCE( min_label, 3 )
	if ( label_vertex[6] != min_label ) _T26_EQUIVALENCE( min_label, 6 )
	if ( label_vertex[7] != min_label ) _T26_EQUIVALENCE( min_label, 7 )
	label_exists[2] = label_exists[3] = label_exists[6] = label_exists[7] = _TRUE_;
    } else {
      if ( GET_VAL( neighbors, 25 ) != 0 )
           _SET_EQUIVALENCE( 6, 7 )
    }

    /*--- Let consider component #3 and its possible equivalences:

          we will examinate the possible points of #3 which
	  may generate an equivalence with an other component:
	  - 1 points (17) yields an equivalence between
	    two components
    ---*/

    /*--- Let consider component #4 and its possible equivalences:

          we will examinate the possible points of #4 which
	  may generate an equivalence with an other component:
	  - 1 point (22) yields an equivalence between
	    four components 
	  - 2 points (19, 21) yield an equivalence between
	    two components
    ---*/

    if ( GET_VAL( neighbors, 22 ) != 0 ) {
	min_label = label_vertex[4];
	if ( label_vertex[5] < min_label ) min_label = label_vertex[5];
	if ( label_vertex[6] < min_label ) min_label = label_vertex[6];
	if ( label_vertex[7] < min_label ) min_label = label_vertex[7];
	if ( label_vertex[4] != min_label ) _T26_EQUIVALENCE( min_label, 4 )
	if ( label_vertex[5] != min_label ) _T26_EQUIVALENCE( min_label, 5 )
	if ( label_vertex[6] != min_label ) _T26_EQUIVALENCE( min_label, 6 )
	if ( label_vertex[7] != min_label ) _T26_EQUIVALENCE( min_label, 7 )
	label_exists[4] = label_exists[5] = label_exists[6] = label_exists[7] = _TRUE_;
    } 

    /*--- At this point, we can count the connected components.

      This connected component #i exists if and only if :
      -     (label_vertex[i] == i)
      - AND (label_exists[i] == _TRUE_)
      
      We may have the following situation :
      (label_vertex[i] == i) AND (label_exists[i] == _FALSE_)
      It means that there was no equivalence for component #i.
      In this case, we have to check the associated vertex
      (one among the 8 points we do not have checked yet).
    ---*/

    nb = 0;

    if ( label_exists[0] == _TRUE_ ) {         nb ++; }
    else { if ( GET_VAL( neighbors, 0 ) != 0 ) nb ++; }

    if ( label_vertex[1] == 1 ) {
	if ( label_exists[1] == _TRUE_ ) {         nb ++; }
	else { if ( GET_VAL( neighbors, 2 ) != 0 ) nb ++; }
    }

    if ( label_vertex[2] == 2 ) {
	if ( label_exists[2] == _TRUE_ ) {         nb ++; }
	else { if ( GET_VAL( neighbors, 6 ) != 0 ) nb ++; }
    }

    if ( label_vertex[3] == 3 ) {
	if ( label_exists[3] == _TRUE_ ) {         nb ++; }
	else { if ( GET_VAL( neighbors, 8 ) != 0 ) nb ++; }
    }

    if ( label_vertex[4] == 4 ) {
	if ( label_exists[4] == _TRUE_ ) {         nb ++; }
	else { if ( GET_VAL( neighbors, 18 ) != 0 ) nb ++; }
    }

    if ( label_vertex[5] == 5 ) {
	if ( label_exists[5] == _TRUE_ ) {         nb ++; }
	else { if ( GET_VAL( neighbors, 20 ) != 0 ) nb ++; }
    }

    if ( label_vertex[6] == 6 ) {
	if ( label_exists[6] == _TRUE_ ) {         nb ++; }
	else { if ( GET_VAL( neighbors, 24 ) != 0 ) nb ++; }
    }

    if ( label_vertex[7] == 7 ) {
	if ( label_exists[7] == _TRUE_ ) {         nb ++; }
	else { if ( GET_VAL( neighbors, 26 ) != 0 ) nb ++; }
    }

    return( nb );
}
