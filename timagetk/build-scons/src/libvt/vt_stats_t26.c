# 1 "vt_t26.c"


# 1 "/u/cajal/0/greg/src//vt_t26noopt.h"
























extern int Compute_NotOptT26( int neighbors[27] );









# 3 "vt_t26.c"







extern int TESTS_nb;
extern int EQUIV_nb;















































int Compute_T26( int neighbors[27] )
{
    




































    int label_vertex[8], label_exists[8];
    register int i, min_label, nb, old_label;
    register int a_label, b_label;

    





    for ( i = 0; i < 8; i ++ ) {
	label_vertex[i] = i;
	label_exists[i] = 0;
    }

    













    if ( ( ++TESTS_nb ? neighbors[ 4] : neighbors[ 4] ) != 0 ) {
      label_vertex[1] = label_vertex[2] = label_vertex[3] = 0;
      label_exists[0] = label_exists[1] = label_exists[2] = label_exists[3] = 1;
	
      



      if ( ( ++TESTS_nb ? neighbors[ 10] : neighbors[ 10] ) != 0 ) {
	label_vertex[4] = label_vertex[5] = 0;
	label_exists[4] = label_exists[5] = 1;

      } else {

        if ( ( ++TESTS_nb ? neighbors[ 9] : neighbors[ 9] ) != 0 ) {
	  label_vertex[4] = 0;
	  label_exists[4] = 1;
	}
	if ( ( ++TESTS_nb ? neighbors[ 11] : neighbors[ 11] ) != 0 ) {
	  label_vertex[5] = 0;
	  label_exists[5] = 1;
	}
        if ( ( ++TESTS_nb ? neighbors[ 19] : neighbors[ 19] ) != 0 ) 
	    { a_label = label_vertex[ 4]; b_label = label_vertex[ 5]; label_exists[ 4] = label_exists[ 5] = 1; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } }
      }

    } else {

      


      if ( ( ++TESTS_nb ? neighbors[ 10] : neighbors[ 10] ) != 0 ) {
	label_vertex[1] = label_vertex[4] = label_vertex[5] = 0;
	label_exists[0] = label_exists[1] = label_exists[4] = label_exists[5] = 1;
	if ( ( ++TESTS_nb ? neighbors[ 3] : neighbors[ 3] ) != 0  ) {
	  label_vertex[2] = 0;
	  label_exists[2] = 1;
	}
	if ( ( ++TESTS_nb ? neighbors[ 5] : neighbors[ 5] ) != 0 ) {
	  label_vertex[3] = 0;
	  label_exists[3] = 1;
	}
	if ( ( ++TESTS_nb ? neighbors[ 7] : neighbors[ 7] ) != 0 )
	  { a_label = label_vertex[ 2]; b_label = label_vertex[ 3]; label_exists[ 2] = label_exists[ 3] = 1; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } }

      } else {

	if ( ( ++TESTS_nb ? neighbors[ 1] : neighbors[ 1] ) != 0 ) {
	  label_vertex[1] = 0;
	  label_exists[0] = label_exists[1] = 1;
	}
	if ( ( ++TESTS_nb ? neighbors[ 3] : neighbors[ 3] ) != 0  ) {
	  label_vertex[2] = 0;
	  label_exists[0] = label_exists[2] = 1;
	}
	if ( ( ++TESTS_nb ? neighbors[ 5] : neighbors[ 5] ) != 0 ) {
	  label_vertex[3] = label_vertex[1];
	  label_exists[1] = label_exists[3] = 1;
	}
	if ( ( ++TESTS_nb ? neighbors[ 7] : neighbors[ 7] ) != 0 )
	  { a_label = label_vertex[ 2]; b_label = label_vertex[ 3]; label_exists[ 2] = label_exists[ 3] = 1; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } }

	if ( ( ++TESTS_nb ? neighbors[ 9] : neighbors[ 9] ) != 0 ) {
	  label_vertex[4] = 0;
	  label_exists[0] = label_exists[4] = 1;
	}
	if ( ( ++TESTS_nb ? neighbors[ 11] : neighbors[ 11] ) != 0 ) {
	  label_vertex[5] = label_vertex[1];
	  label_exists[1] = label_exists[5] = 1;
      }
	if ( ( ++TESTS_nb ? neighbors[ 19] : neighbors[ 19] ) != 0 ) 
	  { a_label = label_vertex[ 4]; b_label = label_vertex[ 5]; label_exists[ 4] = label_exists[ 5] = 1; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } }
	    
      }
      
    }

    
    


    
    if ( ( ++TESTS_nb ? neighbors[ 12] : neighbors[ 12] ) != 0 ) {
      label_vertex[6] = 0;
      if ( label_vertex[2] != 0 ) { EQUIV_nb++; old_label = label_vertex[ 2 ]; label_vertex[old_label] = 0; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = 0; }
      if ( label_vertex[4] != 0 ) { EQUIV_nb++; old_label = label_vertex[ 4 ]; label_vertex[old_label] = 0; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = 0; }
      label_exists[0] = label_exists[2] = label_exists[4] = label_exists[6] = 1;
    } else {
      if ( ( ++TESTS_nb ? neighbors[ 15] : neighbors[ 15] ) != 0 ) {
	label_vertex[6] = label_vertex[2];
	label_exists[2] = label_exists[6] = 1;
      }
      if ( ( ++TESTS_nb ? neighbors[ 21] : neighbors[ 21] ) != 0 )
	{ a_label = label_vertex[ 4]; b_label = label_vertex[ 6]; label_exists[ 4] = label_exists[ 6] = 1; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } }
    }



    


    
    if ( ( ++TESTS_nb ? neighbors[ 14] : neighbors[ 14] ) != 0 ) {
	min_label = label_vertex[1];
	if ( label_vertex[3] < min_label ) min_label = label_vertex[3];
	if ( label_vertex[5] < min_label ) min_label = label_vertex[5];
	if ( label_vertex[7] > min_label ) { 
	  { EQUIV_nb++; old_label = label_vertex[ 7 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	} else {
	  min_label = label_vertex[7];
	}
	if ( label_vertex[1] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 1 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	if ( label_vertex[3] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 3 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	if ( label_vertex[5] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 5 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	label_exists[1] = label_exists[3] = label_exists[5] = label_exists[7] = 1;
    } else {
      if ( ( ++TESTS_nb ? neighbors[ 17] : neighbors[ 17] ) != 0 ) {
	label_vertex[7] = label_vertex[3];
	label_exists[3] = label_exists[7] = 1;
      }
      if ( ( ++TESTS_nb ? neighbors[ 23] : neighbors[ 23] ) != 0 )
           { a_label = label_vertex[ 5]; b_label = label_vertex[ 7]; label_exists[ 5] = label_exists[ 7] = 1; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } }
    }

    


    
    if ( ( ++TESTS_nb ? neighbors[ 16] : neighbors[ 16] ) != 0 ) {
	min_label = label_vertex[2];
	if ( label_vertex[3] < min_label ) min_label = label_vertex[3];
	if ( label_vertex[6] < min_label ) min_label = label_vertex[6];
	if ( label_vertex[7] > min_label ) { 
	  { EQUIV_nb++; old_label = label_vertex[ 7 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	} else {
	  min_label = label_vertex[7];
	}
	if ( label_vertex[2] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 2 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	if ( label_vertex[3] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 3 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	if ( label_vertex[6] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 6 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	label_exists[2] = label_exists[3] = label_exists[6] = label_exists[7] = 1;
    } else {
      if ( ( ++TESTS_nb ? neighbors[ 25] : neighbors[ 25] ) != 0 )
           { a_label = label_vertex[ 6]; b_label = label_vertex[ 7]; label_exists[ 6] = label_exists[ 7] = 1; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } }
    }

    



    if ( ( ++TESTS_nb ? neighbors[ 22] : neighbors[ 22] ) != 0 ) {
	min_label = label_vertex[4];
	if ( label_vertex[5] < min_label ) min_label = label_vertex[5];
	if ( label_vertex[6] < min_label ) min_label = label_vertex[6];
	if ( label_vertex[7] > min_label ) { 
	  { EQUIV_nb++; old_label = label_vertex[ 7 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	} else {
	  min_label = label_vertex[7];
	}
	if ( label_vertex[4] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 4 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	if ( label_vertex[5] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 5 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	if ( label_vertex[6] != min_label ) { EQUIV_nb++; old_label = label_vertex[ 6 ]; label_vertex[old_label] = min_label; for ( i=old_label+1; i<8; i++ ) if ( label_vertex[i] == old_label ) label_vertex[i] = min_label; }
	label_exists[4] = label_exists[5] = label_exists[6] = label_exists[7] = 1;
    } 

    












    nb = 0;

    if ( label_exists[0] == 1 ) {         nb ++; }
    else { if ( ( ++TESTS_nb ? neighbors[ 0] : neighbors[ 0] ) != 0 ) nb ++; }

    if ( label_vertex[1] == 1 ) {
	if ( label_exists[1] == 1 ) {         nb ++; }
	else { if ( ( ++TESTS_nb ? neighbors[ 2] : neighbors[ 2] ) != 0 ) nb ++; }
    }

    if ( label_vertex[2] == 2 ) {
	if ( label_exists[2] == 1 ) {         nb ++; }
	else { if ( ( ++TESTS_nb ? neighbors[ 6] : neighbors[ 6] ) != 0 ) nb ++; }
    }

    if ( label_vertex[3] == 3 ) {
	if ( label_exists[3] == 1 ) {         nb ++; }
	else { if ( ( ++TESTS_nb ? neighbors[ 8] : neighbors[ 8] ) != 0 ) nb ++; }
    }

    if ( label_vertex[4] == 4 ) {
	if ( label_exists[4] == 1 ) {         nb ++; }
	else { if ( ( ++TESTS_nb ? neighbors[ 18] : neighbors[ 18] ) != 0 ) nb ++; }
    }

    if ( label_vertex[5] == 5 ) {
	if ( label_exists[5] == 1 ) {         nb ++; }
	else { if ( ( ++TESTS_nb ? neighbors[ 20] : neighbors[ 20] ) != 0 ) nb ++; }
    }

    if ( label_vertex[6] == 6 ) {
	if ( label_exists[6] == 1 ) {         nb ++; }
	else { if ( ( ++TESTS_nb ? neighbors[ 24] : neighbors[ 24] ) != 0 ) nb ++; }
    }

    if ( label_vertex[7] == 7 ) {
	if ( label_exists[7] == 1 ) {         nb ++; }
	else { if ( ( ++TESTS_nb ? neighbors[ 26] : neighbors[ 26] ) != 0 ) nb ++; }
    }

    return( nb );
}
