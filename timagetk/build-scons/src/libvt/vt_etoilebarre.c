
#include <vt_etoilebarre.h>

#define _LOCAL_FALSE 0
#define _LOCAL_TRUE  1

#define _LOCAL_MIN(A,B) ( (A) < (B) ? (A) : (B) )

/*------- Definition des fonctions statiques ----------*/
static void _VT_CetoileEquivalence( int labels[8], int l, int i );
static void _VT_CbarreEquivalence( int labels[27], int l, int i );


/* Compte les 26-composantes connexes dans un 26-voisinage.

   Les points consideres sont ceux dont la valeur est 
   differente de 0. Le point central n'est pas compte
   (il est considere comme appartenant au fond).

RETURN
   Retourne le nombre de composantes connexes.

*/

int VT_ComputeCetoile( int voisins[27] )
{
    int label_octant[8];
    int filled_octant[8];
    register int i, min;
    int nb;

    for ( i = 0; i < 8; i ++ ) {
	label_octant[i] = i;
	filled_octant[i] = _LOCAL_FALSE;
    }

    /* la numerotation du voisinage est
        0  1  2  -   9 10 11  -  18 19 20
        3  4  5  -  12 13 14  -  21 22 23
        6  7  8  -  15 16 17  -  24 25 26

	octant #0 =  0  1  3  4  9 10 12 13
	octant #1 =  1  2  4  5 10 11 13 14
	octant #2 =  3  4  6  7 12 13 15 16
	octant #3 =  4  5  7  8 13 14 16 17
	octant #4 =              9 10 12 13 18 19 21 22
	octant #5 =             10 11 13 14 19 20 22 23
	octant #6 =             12 13 15 16 21 22 24 25
	octant #7 =             13 14 16 17 22 23 25 26
    */
    /* les points appartenant a 4 octants : 4 10 12 14 16 22
        4 : 0 1 2 3
       10 : 0 1 4 5
       12 : 0 2 4 6
       14 : 1 3 5 7
       16 : 2 3 6 7
       22 : 4 5 6 7
    */   
    /* les points appartenant a 2 octants : 1 3 5 7 9 11 15 17 19 21 23 25
        1 : 0 1
	3 : 0 2
	5 : 1 3
	7 : 2 3
	9 : 0 4
       11 : 1 5
       15 : 2 6
       17 : 3 7
       19 : 4 5
       21 : 4 6
       23 : 5 7
       25 : 6 7
    */   

    /*--- on propage deja l'octant 0 : pas de probleme d'equivalence ---*/
    if ( voisins[4] != 0 ) {
	label_octant[1]  = label_octant[2]  = label_octant[3]  = 0;
	filled_octant[0] = filled_octant[1] = filled_octant[2] = filled_octant[3] = _LOCAL_TRUE;
    } 
    if ( voisins[10] != 0 ) {
	label_octant[1]  = label_octant[4]  = label_octant[5]  = 0;
	filled_octant[0] = filled_octant[1] = filled_octant[4] = filled_octant[5] = _LOCAL_TRUE;
    } 
    if ( voisins[12] != 0 ) {
	label_octant[2]  = label_octant[4]  = label_octant[6]  = 0;
	filled_octant[0] = filled_octant[2] = filled_octant[4] = filled_octant[6] = _LOCAL_TRUE;
    } 
    if ( voisins[1] != 0 ) {
	label_octant[1]  = 0;
	filled_octant[0] = filled_octant[1] = _LOCAL_TRUE;
    }
    if ( voisins[3] != 0 ) {
	label_octant[2]  = 0;
	filled_octant[0] = filled_octant[2] = _LOCAL_TRUE;
    }
    if ( voisins[9] != 0 ) {
	label_octant[4]  = 0;
	filled_octant[0] = filled_octant[4] = _LOCAL_TRUE;
    }

    /*--- passons a l'octant 1 ---*/
    if ( voisins[14] != 0 ) {
	min = _LOCAL_MIN( label_octant[1], label_octant[3] );
	min = _LOCAL_MIN( label_octant[5], min );
	min = _LOCAL_MIN( label_octant[7], min );	
	if ( label_octant[1] != min ) _VT_CetoileEquivalence( label_octant, min, 1 );
	if ( label_octant[3] != min ) _VT_CetoileEquivalence( label_octant, min, 3 );
	if ( label_octant[5] != min ) _VT_CetoileEquivalence( label_octant, min, 5 );
	if ( label_octant[7] != min ) _VT_CetoileEquivalence( label_octant, min, 7 );
	filled_octant[1] = filled_octant[3] = filled_octant[5] = filled_octant[7] = _LOCAL_TRUE;
    } 
    if ( voisins[5] != 0 ) {
	min = _LOCAL_MIN( label_octant[1], label_octant[3] );
	if ( label_octant[1] != min ) _VT_CetoileEquivalence( label_octant, min, 1 );
	if ( label_octant[3] != min ) _VT_CetoileEquivalence( label_octant, min, 3 );
	filled_octant[1] = filled_octant[3] = _LOCAL_TRUE;
    }
    if ( voisins[11] != 0 ) {
	min = _LOCAL_MIN( label_octant[1], label_octant[5] );
	if ( label_octant[1] != min ) _VT_CetoileEquivalence( label_octant, min, 1 );
	if ( label_octant[5] != min ) _VT_CetoileEquivalence( label_octant, min, 5 );
	filled_octant[1] = filled_octant[5] = _LOCAL_TRUE;
    }

    /*--- passons a l'octant 2 ---*/
    if ( voisins[16] != 0 ) {
	min = _LOCAL_MIN( label_octant[2], label_octant[3] );
	min = _LOCAL_MIN( label_octant[6], min );
	min = _LOCAL_MIN( label_octant[7], min );	
	if ( label_octant[2] != min ) _VT_CetoileEquivalence( label_octant, min, 2 );
	if ( label_octant[3] != min ) _VT_CetoileEquivalence( label_octant, min, 3 );
	if ( label_octant[6] != min ) _VT_CetoileEquivalence( label_octant, min, 6 );
	if ( label_octant[7] != min ) _VT_CetoileEquivalence( label_octant, min, 7 );
	filled_octant[2] = filled_octant[3] = filled_octant[6] = filled_octant[7] = _LOCAL_TRUE;
    } 
    if ( voisins[7] != 0 ) {
	min = _LOCAL_MIN( label_octant[2], label_octant[3] );
	if ( label_octant[2] != min ) _VT_CetoileEquivalence( label_octant, min, 2 );
	if ( label_octant[3] != min ) _VT_CetoileEquivalence( label_octant, min, 3 );
	filled_octant[2] = filled_octant[3] = _LOCAL_TRUE;
    }
    if ( voisins[15] != 0 ) {
	min = _LOCAL_MIN( label_octant[2], label_octant[6] );
	if ( label_octant[2] != min ) _VT_CetoileEquivalence( label_octant, min, 2 );
	if ( label_octant[6] != min ) _VT_CetoileEquivalence( label_octant, min, 6 );
	filled_octant[2] = filled_octant[6] = _LOCAL_TRUE;
    }

    /*--- passons a l'octant 3 ---*/
    if ( voisins[17] != 0 ) {
	min = _LOCAL_MIN( label_octant[3], label_octant[7] );
	if ( label_octant[3] != min ) _VT_CetoileEquivalence( label_octant, min, 3 );
	if ( label_octant[7] != min ) _VT_CetoileEquivalence( label_octant, min, 7 );
	filled_octant[3] = filled_octant[7] = _LOCAL_TRUE;
    }

    /*--- passons a l'octant 4 ---*/
    if ( voisins[22] != 0 ) {
	min = _LOCAL_MIN( label_octant[4], label_octant[5] );
	min = _LOCAL_MIN( label_octant[6], min );
	min = _LOCAL_MIN( label_octant[7], min );	
	if ( label_octant[4] != min ) _VT_CetoileEquivalence( label_octant, min, 4 );
	if ( label_octant[5] != min ) _VT_CetoileEquivalence( label_octant, min, 5 );
	if ( label_octant[6] != min ) _VT_CetoileEquivalence( label_octant, min, 6 );
	if ( label_octant[7] != min ) _VT_CetoileEquivalence( label_octant, min, 7 );
	filled_octant[4] = filled_octant[5] = filled_octant[6] = filled_octant[7] = _LOCAL_TRUE;
    } 
    if ( voisins[19] != 0 ) {
	min = _LOCAL_MIN( label_octant[4], label_octant[5] );
	if ( label_octant[4] != min ) _VT_CetoileEquivalence( label_octant, min, 4 );
	if ( label_octant[5] != min ) _VT_CetoileEquivalence( label_octant, min, 5 );
	filled_octant[4] = filled_octant[5] = _LOCAL_TRUE;
    }
    if ( voisins[21] != 0 ) {
	min = _LOCAL_MIN( label_octant[4], label_octant[6] );
	if ( label_octant[4] != min ) _VT_CetoileEquivalence( label_octant, min, 4 );
	if ( label_octant[6] != min ) _VT_CetoileEquivalence( label_octant, min, 6 );
	filled_octant[4] = filled_octant[6] = _LOCAL_TRUE;
    }

    /*--- passons a l'octant 5 ---*/
    if ( voisins[23] != 0 ) {
	min = _LOCAL_MIN( label_octant[5], label_octant[7] );
	if ( label_octant[5] != min ) _VT_CetoileEquivalence( label_octant, min, 5 );
	if ( label_octant[7] != min ) _VT_CetoileEquivalence( label_octant, min, 7 );
	filled_octant[5] = filled_octant[7] = _LOCAL_TRUE;
    }

    /*--- passons a l'octant 6 ---*/
    if ( voisins[25] != 0 ) {
	min = _LOCAL_MIN( label_octant[6], label_octant[7] );
	if ( label_octant[6] != min ) _VT_CetoileEquivalence( label_octant, min, 6 );
	if ( label_octant[7] != min ) _VT_CetoileEquivalence( label_octant, min, 7 );
	filled_octant[6] = filled_octant[7] = _LOCAL_TRUE;
    }
    
    /*--- on peut compter ---*/
    nb = 0;

    if ( filled_octant[0] == _LOCAL_TRUE ) {
	nb ++;
    } else {
	if ( voisins[0] != 0 ) nb ++;
    }

    if ( label_octant[1] == 1 ) {
	if ( filled_octant[1] == _LOCAL_TRUE ) {
		nb ++;
	} else {
	    if ( voisins[2] != 0 ) nb ++;
	}
    }

    if ( label_octant[2] == 2 ) {
	if ( filled_octant[2] == _LOCAL_TRUE ) {
		nb ++;
	} else {
	    if ( voisins[6] != 0 ) nb ++;
	}
    }

    if ( label_octant[3] == 3 ) {
	if ( filled_octant[3] == _LOCAL_TRUE ) {
		nb ++;
	} else {
	    if ( voisins[8] != 0 ) nb ++;
	}
    }

    if ( label_octant[4] == 4 ) {
	if ( filled_octant[4] == _LOCAL_TRUE ) {
		nb ++;
	} else {
	    if ( voisins[18] != 0 ) nb ++;
	}
    }

    if ( label_octant[5] == 5 ) {
	if ( filled_octant[5] == _LOCAL_TRUE ) {
		nb ++;
	} else {
	    if ( voisins[20] != 0 ) nb ++;
	}
    }

    if ( label_octant[6] == 6 ) {
	if ( filled_octant[6] == _LOCAL_TRUE ) {
		nb ++;
	} else {
	    if ( voisins[24] != 0 ) nb ++;
	}
    }

    if ( label_octant[7] == 7 ) {
	if ( filled_octant[7] == _LOCAL_TRUE ) {
		nb ++;
	} else {
	    if ( voisins[26] != 0 ) nb ++;
	}
    }
    return( nb );
}





static void _VT_CetoileEquivalence( int labels[8], int l, int i )
{
    register int old, k;
    old = labels[i];
    for ( k=0; k<8; k++ )
	if ( labels[k] == old ) labels[k] = l;
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

int VT_ComputeCbarre( int voisins[27] )
{
    int labels[27];
    int nb;
    register int i, min;

    /* initialisation des labels :
       les 6 plus petits labels sont pour les 6-voisins du point central */
    for ( i = 0; i<27; i++ ) labels[i] = 27+i;
    labels[ 4] = 0;
    labels[10] = 1;
    labels[12] = 2;
    labels[14] = 3;
    labels[16] = 4;
    labels[22] = 5;

    /* la numerotation du voisinage est
       0  1  2  -   9 10 11  -  18 19 20
       3  4  5  -  12 13 14  -  21 22 23
       6  7  8  -  15 16 17  -  24 25 26
       
       les relations de voisinages sont :
       static int Relation06_hole[27][27] = {
       {  0 },
       {  2,  4, 10 },
       {  0 },
       {  2,  4, 12 },
       {  4,  1,  3,  5,  7 },
       {  2,  4, 14 },
       {  0 },
       {  2,  4, 16 },
       {  0 },
       {  2, 10, 12 },
       {  4,  1,  9, 11, 19 },
       {  2, 10, 14 },
       {  4,  3,  9, 15, 21 },
       {  0  },
       {  4,  5, 11, 17, 23 },
       {  2, 12, 16 },
       {  4,  7, 15, 17, 25 },
       {  2, 14, 16 },
       {  0 },
       {  2, 10, 22 },
       {  0 },
       {  2, 12, 22 },
       {  4, 19, 21, 23, 25 },
       {  2, 14, 22 },
       {  0 },
       {  2, 16, 22 },
       {  0 }
       les relations a considerer sont donc
       1-4,   1-10,
       3-4,   3-12,
       4-1,   4-3,   4-5,   4-7,
       5-4,   5-14,
       7-4,   7-16,
       9-10,  9-12,
       10-1,  10-9,  10-11, 10-19,
       11-10, 11-14,
       12-3,  12-9,  12-15, 12-21,
       14-5,  14-11, 14-17, 14-23,
       15-12, 15-16,
       16-7,  16-15, 16-17, 16-25,
       17-14, 17-16,
       19-10, 19-22,
       21-12, 21-22,
       22-19, 22-21, 22-23, 22-25,
       23-14, 23-22,
       25-16, 25-22
       */

    /*--- on considere d'abord les points en relation avec 4 (label 0) ---*/
    if ( voisins[4] == 0 ) {
	if ( voisins[1] == 0 ) labels[1] = 0;
	if ( voisins[3] == 0 ) labels[3] = 0;
	if ( voisins[5] == 0 ) labels[5] = 0;
	if ( voisins[7] == 0 ) labels[7] = 0;
    }

    /*--- puis avec 22 (pas d'equivalence a faire) ---*/
    if ( voisins[22] == 0 ) {
	if ( voisins[19] == 0 ) labels[19] = 5;
	if ( voisins[21] == 0 ) labels[21] = 5;
	if ( voisins[23] == 0 ) labels[23] = 5;
	if ( voisins[25] == 0 ) labels[25] = 5;
    }


    /*--- point 10 ---*/
    if ( voisins[10] == 0 ) {
	min = _LOCAL_MIN( labels[1], labels[9] );
	min = _LOCAL_MIN( labels[10], min );
	min = _LOCAL_MIN( labels[11], min );
	min = _LOCAL_MIN( labels[19], min );
	if ( (voisins[ 1] == 0) && (labels[ 1] != min) ) _VT_CbarreEquivalence( labels, min,  1 );
	if ( (voisins[ 9] == 0) && (labels[ 9] != min) ) _VT_CbarreEquivalence( labels, min,  9 );
	if ( (voisins[10] == 0) && (labels[10] != min) ) _VT_CbarreEquivalence( labels, min, 10 );
	if ( (voisins[11] == 0) && (labels[11] != min) ) _VT_CbarreEquivalence( labels, min, 11 );
	if ( (voisins[19] == 0) && (labels[19] != min) ) _VT_CbarreEquivalence( labels, min, 19 );
    }


    /*--- point 12 ---*/
    if ( voisins[12] == 0 ) {
	min = _LOCAL_MIN( labels[3], labels[9] );
	min = _LOCAL_MIN( labels[12], min );
	min = _LOCAL_MIN( labels[15], min );
	min = _LOCAL_MIN( labels[21], min );
	if ( (voisins[ 3] == 0) && (labels[ 3] != min) ) _VT_CbarreEquivalence( labels, min,  3 );
	if ( (voisins[ 9] == 0) && (labels[ 9] != min) ) _VT_CbarreEquivalence( labels, min,  9 );
	if ( (voisins[12] == 0) && (labels[12] != min) ) _VT_CbarreEquivalence( labels, min, 12 );
	if ( (voisins[15] == 0) && (labels[15] != min) ) _VT_CbarreEquivalence( labels, min, 15 );
	if ( (voisins[21] == 0) && (labels[21] != min) ) _VT_CbarreEquivalence( labels, min, 21 );
    }


    /*--- point 14 ---*/
    if ( voisins[14] == 0 ) {
	min = _LOCAL_MIN( labels[5], labels[11] );
	min = _LOCAL_MIN( labels[14], min );
	min = _LOCAL_MIN( labels[17], min );
	min = _LOCAL_MIN( labels[23], min );
	if ( (voisins[ 5] == 0) && (labels[ 5] != min) ) _VT_CbarreEquivalence( labels, min,  5 );
	if ( (voisins[11] == 0) && (labels[11] != min) ) _VT_CbarreEquivalence( labels, min, 11 );
	if ( (voisins[14] == 0) && (labels[14] != min) ) _VT_CbarreEquivalence( labels, min, 14 );
	if ( (voisins[17] == 0) && (labels[17] != min) ) _VT_CbarreEquivalence( labels, min, 17 );
	if ( (voisins[23] == 0) && (labels[23] != min) ) _VT_CbarreEquivalence( labels, min, 23 );
    }


    /*--- point 16 ---*/
    if ( voisins[16] == 0 ) {
	min = _LOCAL_MIN( labels[7], labels[15] );
	min = _LOCAL_MIN( labels[16], min );
	min = _LOCAL_MIN( labels[17], min );
	min = _LOCAL_MIN( labels[25], min );
	if ( (voisins[ 7] == 0) && (labels[ 7] != min) ) _VT_CbarreEquivalence( labels, min,  7 );
	if ( (voisins[15] == 0) && (labels[15] != min) ) _VT_CbarreEquivalence( labels, min, 15 );
	if ( (voisins[16] == 0) && (labels[16] != min) ) _VT_CbarreEquivalence( labels, min, 16 );
	if ( (voisins[17] == 0) && (labels[17] != min) ) _VT_CbarreEquivalence( labels, min, 17 );
	if ( (voisins[25] == 0) && (labels[25] != min) ) _VT_CbarreEquivalence( labels, min, 25 );
    }


    /*--- on compte ---*/
    nb = 0;
    if ( (voisins[ 4] == 0) && (labels[ 4] == 0) ) nb++;
    if ( (voisins[10] == 0) && (labels[10] == 1) ) nb++;
    if ( (voisins[12] == 0) && (labels[12] == 2) ) nb++;
    if ( (voisins[14] == 0) && (labels[14] == 3) ) nb++;
    if ( (voisins[16] == 0) && (labels[16] == 4) ) nb++;
    if ( (voisins[22] == 0) && (labels[22] == 5) ) nb++;
    
    return( nb );
}





static void _VT_CbarreEquivalence( int labels[27], int l, int i )
{
    register int old;
    old = labels[i];

    if ( labels[ 1] == old ) labels[ 1] = l;
    if ( labels[ 3] == old ) labels[ 3] = l;
    if ( labels[ 4] == old ) labels[ 4] = l;
    if ( labels[ 5] == old ) labels[ 5] = l;
    if ( labels[ 7] == old ) labels[ 7] = l;

    if ( labels[ 9] == old ) labels[ 9] = l;
    if ( labels[10] == old ) labels[10] = l;
    if ( labels[11] == old ) labels[11] = l;
    if ( labels[12] == old ) labels[12] = l;
    if ( labels[14] == old ) labels[14] = l;
    if ( labels[15] == old ) labels[15] = l;
    if ( labels[16] == old ) labels[16] = l;
    if ( labels[17] == old ) labels[17] = l;

    if ( labels[19] == old ) labels[19] = l;
    if ( labels[21] == old ) labels[21] = l;
    if ( labels[22] == old ) labels[22] = l;
    if ( labels[23] == old ) labels[23] = l;
    if ( labels[25] == old ) labels[25] = l;
}
