# 1 "vt_t06t26_simple.c"

# 1 "/u/cajal/0/greg/src//vt_t06t26_simple.h"


























extern int Compute_Simple( int neighbors[27] );







# 2 "vt_t06t26_simple.c"



































































extern int TESTS_nb;
extern int EQUIV_nb;










             

             







































































































































































































































































































































































































































































































































































































































































int Compute_Simple( neighbors )
int neighbors[27]; 

{
    int label_vertex[8], label_exists[8];
    int label_face[6];
    register int i, nb_t26, nb_t06;
    register int a_label, b_label;

    nb_t06 = 0;
    nb_t26 = 0;

    if ( ( ++TESTS_nb ? neighbors[4] : neighbors[4] ) == 0 ) {
	
	if ( ( ++TESTS_nb ? neighbors[10] : neighbors[10] ) == 0 ) {
	    
	    if ( ( ++TESTS_nb ? neighbors[12] : neighbors[12] ) == 0 ) {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    

			    
			    for ( i = 0; i < 8; i ++ ) {
				label_vertex[i] = i;
				label_exists[i] = 0;
			    }
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
			    
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { label_face[1] = label_face[0]; } else { label_vertex[1] = label_vertex[0]; label_exists[0] = label_exists[1] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { label_face[2] = label_face[0]; } else { label_vertex[2] = label_vertex[0]; label_exists[0] = label_exists[2] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { label_face[3] = label_face[0]; } else { label_vertex[3] = label_vertex[1]; label_exists[1] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { label_face[4] = label_face[0]; } else { if ( label_vertex[2] < label_vertex[3] ) { { EQUIV_nb++; b_label = label_vertex[3]; label_vertex[b_label] = label_vertex[2]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[2]; } } else { label_vertex[2] = label_vertex[3]; } label_exists[2] = label_exists[3] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { label_face[5] = label_face[1] ; } else { label_vertex[5] = label_vertex[4] ; label_exists[4] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( label_face[2] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[2]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[2]; } } else { label_face[2] = label_face[5]; } } else { label_vertex[6] = label_vertex[4]; label_exists[4] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( label_face[3] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[3]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[3]; } } else { label_face[3] = label_face[5]; } } else { label_vertex[7] = label_vertex[5]; label_exists[5] = label_exists[7] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( label_face[4] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[4]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[4]; } } else { label_face[4] = label_face[5]; } } else { if ( label_vertex[6] < label_vertex[7] ) { { EQUIV_nb++; b_label = label_vertex[7]; label_vertex[b_label] = label_vertex[6]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[6]; } } else { label_vertex[6] = label_vertex[7]; } label_exists[6] = label_exists[7] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( label_face[1] < label_face[2] ) { { EQUIV_nb++; b_label = label_face[2]; label_face[b_label] = label_face[1]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[1]; } } else { label_face[1] = label_face[2]; } } else { if ( label_vertex[0] < label_vertex[4] ) { { EQUIV_nb++; b_label = label_vertex[4]; label_vertex[b_label] = label_vertex[0]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[0]; } } label_exists[0] = label_exists[4] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( label_face[1] < label_face[3] ) { { EQUIV_nb++; b_label = label_face[3]; label_face[b_label] = label_face[1]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[1]; } } else { label_face[1] = label_face[3]; } } else { if ( label_vertex[1] < label_vertex[5] ) { { EQUIV_nb++; b_label = label_vertex[5]; label_vertex[b_label] = label_vertex[1]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[1]; } } else { label_vertex[1] = label_vertex[5]; } label_exists[1] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { { a_label = label_face[2]; b_label = label_face[4]; if ( a_label < b_label ) { EQUIV_nb++; label_face[b_label] = a_label; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_face[a_label] = b_label; for ( i=a_label+1; i<6; i++ ) if ( label_face[i] == a_label ) label_face[i] = b_label; } } } else { { a_label = label_vertex[2]; b_label = label_vertex[6]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[2] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { { a_label = label_face[3]; b_label = label_face[4]; if ( a_label < b_label ) { EQUIV_nb++; label_face[b_label] = a_label; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_face[a_label] = b_label; for ( i=a_label+1; i<6; i++ ) if ( label_face[i] == a_label ) label_face[i] = b_label; } } } else { { a_label = label_vertex[3]; b_label = label_vertex[7]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[3] = label_exists[7] = 1; }

			    
			    for ( i = 0; i < 6; i ++ )
				if ( label_face[i] == i ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;

			    if ( label_exists[0] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) != 0 ) nb_t26 ++; }
			    if ( label_vertex[1] == 1 ) { if ( label_exists[1] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[2] == 2 ) { if ( label_exists[2] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[3] == 3 ) { if ( label_exists[3] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[4] == 4 ) { if ( label_exists[4] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[5] == 5 ) { if ( label_exists[5] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[6] == 6 ) { if ( label_exists[6] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[7] == 7 ) { if ( label_exists[7] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) != 0 ) nb_t26 ++; } }
			    if ( nb_t26 != 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    
			    
			    
			    label_vertex[0] = 0;   label_vertex[1] = 1;   label_vertex[2] = 2;   label_vertex[3] = 3;   
			    label_vertex[4] = 4;   label_vertex[5] = 4;   label_vertex[6] = 4;   label_vertex[7] = 4;   
			    label_exists[0] = label_exists[1] = label_exists[2] = label_exists[3] = 0;
			    label_exists[4] = label_exists[5] = label_exists[6] = label_exists[7] = 1;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;

			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { label_face[1] = label_face[0]; } else { label_vertex[1] = label_vertex[0]; label_exists[0] = label_exists[1] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { label_face[2] = label_face[0]; } else { label_vertex[2] = label_vertex[0]; label_exists[0] = label_exists[2] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { label_face[3] = label_face[0]; } else { label_vertex[3] = label_vertex[1]; label_exists[1] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { label_face[4] = label_face[0]; } else { if ( label_vertex[2] < label_vertex[3] ) { { EQUIV_nb++; b_label = label_vertex[3]; label_vertex[b_label] = label_vertex[2]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[2]; } } else { label_vertex[2] = label_vertex[3]; } label_exists[2] = label_exists[3] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( label_face[1] < label_face[2] ) { label_face[2] = label_face[1]; } else { label_face[1] = label_face[2]; } } else { if ( label_vertex[0] < label_vertex[4] ) { { EQUIV_nb++; b_label = label_vertex[4]; label_vertex[b_label] = label_vertex[0]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[0]; } } label_exists[0] = label_exists[4] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( label_face[1] < label_face[3] ) { label_face[3] = label_face[1]; } else { label_face[1] = label_face[3]; } } else { if ( label_vertex[1] < label_vertex[5] ) { { EQUIV_nb++; b_label = label_vertex[5]; label_vertex[b_label] = label_vertex[1]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[1]; } } else { label_vertex[1] = label_vertex[5]; } label_exists[1] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( label_face[4] < label_face[2] ) { { EQUIV_nb++; b_label = label_face[2]; label_face[b_label] = label_face[4]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[4]; } } else { label_face[4] = label_face[2]; } } else { { a_label = label_vertex[2]; b_label = label_vertex[6]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[2] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { { a_label = label_face[3]; b_label = label_face[4]; if ( a_label < b_label ) { EQUIV_nb++; label_face[b_label] = a_label; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_face[a_label] = b_label; for ( i=a_label+1; i<6; i++ ) if ( label_face[i] == a_label ) label_face[i] = b_label; } } } else { { a_label = label_vertex[3]; b_label = label_vertex[7]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[3] = label_exists[7] = 1; }

			    
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;

			    if ( label_exists[0] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) != 0 ) nb_t26 ++; }
			    if ( label_vertex[1] == 1 ) { if ( label_exists[1] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[2] == 2 ) { if ( label_exists[2] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[3] == 3 ) { if ( label_exists[3] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[4] == 4 ) nb_t26 ++;
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    

			    
			    label_vertex[0] = 0;   label_vertex[1] = 1;   label_vertex[4] = 4;   label_vertex[5] = 5;   
			    label_vertex[2] = 2;   label_vertex[3] = 2;   label_vertex[6] = 2;   label_vertex[7] = 2;   
			    label_exists[0] = label_exists[1] = label_exists[4] = label_exists[5] = 0;
			    label_exists[2] = label_exists[3] = label_exists[6] = label_exists[7] = 1;
			    for ( i = 0; i < 6; i ++ ) {
				label_face[i] = i;
			    }

			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { label_face[1] = label_face[0]; } else { label_vertex[1] = label_vertex[0]; label_exists[0] = label_exists[1] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { label_face[2] = label_face[1]; } else { label_vertex[4] = label_vertex[0]; label_exists[0] = label_exists[4] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { label_face[3] = label_face[1]; } else { label_vertex[5] = label_vertex[1]; label_exists[1] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { label_face[5] = label_face[1] ; } else { if ( label_vertex[4] < label_vertex[5] ) { { EQUIV_nb++; b_label = label_vertex[5]; label_vertex[b_label] = label_vertex[4]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[4]; } } else { label_vertex[4] = label_vertex[5]; } label_exists[4] = label_exists[5] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( label_face[0] < label_face[2] ) { { EQUIV_nb++; b_label = label_face[2]; label_face[b_label] = label_face[0]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[0]; } } } else { if ( label_vertex[0] < label_vertex[2] ) { { EQUIV_nb++; b_label = label_vertex[2]; label_vertex[b_label] = label_vertex[0]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[0]; } } label_exists[0] = label_exists[2] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( label_face[0] < label_face[3] ) { { EQUIV_nb++; b_label = label_face[3]; label_face[b_label] = label_face[0]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[0]; } } } else { if ( label_vertex[1] < label_vertex[3] ) { { EQUIV_nb++; b_label = label_vertex[3]; label_vertex[b_label] = label_vertex[1]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[1]; } } else { label_vertex[1] = label_vertex[3]; } label_exists[1] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( label_face[2] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[2]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[2]; } } else { label_face[2] = label_face[5]; } } else { { a_label = label_vertex[4]; b_label = label_vertex[6]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[4] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { { a_label = label_face[3]; b_label = label_face[5]; if ( a_label < b_label ) { EQUIV_nb++; label_face[b_label] = a_label; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_face[a_label] = b_label; for ( i=a_label+1; i<6; i++ ) if ( label_face[i] == a_label ) label_face[i] = b_label; } } } else { { a_label = label_vertex[5]; b_label = label_vertex[7]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[5] = label_exists[7] = 1; }

			    
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    if ( label_exists[0] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) != 0 ) nb_t26 ++; }
			    if ( label_vertex[1] == 1 ) { if ( label_exists[1] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[4] == 4 ) { if ( label_exists[4] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[5] == 5 ) { if ( label_exists[5] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[2] == 2 ) nb_t26 ++;
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    

			    
			    label_vertex[0] = 0;   label_vertex[2] = 2;   label_vertex[4] = 4;   label_vertex[6] = 6;   
			    label_vertex[1] = 1;   label_vertex[3] = 1;   label_vertex[5] = 1;   label_vertex[7] = 1;   
			    label_exists[0] = label_exists[2] = label_exists[4] = label_exists[6] = 0;
			    label_exists[1] = label_exists[3] = label_exists[5] = label_exists[7] = 1;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
 
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { label_face[2] = label_face[0]; } else { label_vertex[2] = label_vertex[0]; label_exists[0] = label_exists[2] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( label_face[1] < label_face[2] ) { label_face[2] = label_face[1]; } else { label_face[1] = label_face[2]; } } else { label_vertex[4] = label_vertex[0]; label_exists[0] = label_exists[4] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { label_face[4] = label_face[2]; } else { label_vertex[6] = label_vertex[2]; label_exists[2] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { label_face[5] = label_face[2]; } else { if ( label_vertex[4] < label_vertex[6] ) { { EQUIV_nb++; b_label = label_vertex[6]; label_vertex[b_label] = label_vertex[4]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[4]; } } else { label_vertex[4] = label_vertex[6]; } label_exists[4] = label_exists[6] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { label_face[1] = label_face[0]; } else { label_vertex[1] = label_vertex[0]; label_exists[0] = label_exists[1] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( label_face[0] < label_face[4] ) { { EQUIV_nb++; b_label = label_face[4]; label_face[b_label] = label_face[0]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[0]; } } } else { if ( label_vertex[2] < label_vertex[3] ) { { EQUIV_nb++; b_label = label_vertex[3]; label_vertex[b_label] = label_vertex[2]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[2]; } } else { label_vertex[2] = label_vertex[3]; } label_exists[2] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( label_face[1] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[1]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[1]; } } else { label_face[1] = label_face[5]; } } else { { a_label = label_vertex[4]; b_label = label_vertex[5]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[4] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { { a_label = label_face[4]; b_label = label_face[5]; if ( a_label < b_label ) { EQUIV_nb++; label_face[b_label] = a_label; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_face[a_label] = b_label; for ( i=a_label+1; i<6; i++ ) if ( label_face[i] == a_label ) label_face[i] = b_label; } } } else { { a_label = label_vertex[6]; b_label = label_vertex[7]; if ( a_label < b_label ) { EQUIV_nb++; label_vertex[b_label] = a_label; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_vertex[a_label] = b_label; for ( i=a_label+1; i<8; i++ ) if ( label_vertex[i] == a_label ) label_vertex[i] = b_label; } } label_exists[6] = label_exists[7] = 1; }

			    
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    if ( label_exists[0] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) != 0 ) nb_t26 ++; }
			    if ( label_vertex[2] == 2 ) { if ( label_exists[2] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[4] == 4 ) { if ( label_exists[4] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[6] == 6 ) { if ( label_exists[6] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[1] == 1 ) nb_t26 ++;
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[0] : neighbors[0] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    }
		}
	    } else {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    

			    
			    label_vertex[1] = 1;   label_vertex[3] = 3;   label_vertex[5] = 5;   label_vertex[7] = 7;   
			    label_vertex[0] = 0;   label_vertex[2] = 0;   label_vertex[4] = 0;   label_vertex[6] = 0;   
			    label_exists[1] = label_exists[3] = label_exists[5] = label_exists[7] = 0;
			    label_exists[0] = label_exists[2] = label_exists[4] = label_exists[6] = 1;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
   
			    
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { label_face[3] = label_face[0]; } else { label_vertex[3] = label_vertex[1]; label_exists[1] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( label_face[1] < label_face[3] ) { label_face[3] = label_face[1]; } else { label_face[1] = label_face[3]; } } else { label_vertex[5] = label_vertex[1]; label_exists[1] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { label_face[4] = label_face[3]; } else { label_vertex[7] = label_vertex[3]; label_exists[3] = label_exists[7] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { label_face[5] = label_face[3]; } else { if ( label_vertex[5] < label_vertex[7] ) { { EQUIV_nb++; b_label = label_vertex[7]; label_vertex[b_label] = label_vertex[5]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[5]; } } else { label_vertex[5] = label_vertex[7]; } label_exists[5] = label_exists[7] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { label_face[1] = label_face[0]; } else { label_vertex[1] = label_vertex[0]; label_exists[0] = label_exists[1] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( label_face[0] < label_face[4] ) { { EQUIV_nb++; b_label = label_face[4]; label_face[b_label] = label_face[0]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[0]; } } } else { if ( label_vertex[2] < label_vertex[3] ) { { EQUIV_nb++; b_label = label_vertex[3]; label_vertex[b_label] = label_vertex[2]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[2]; } } label_exists[2] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( label_face[1] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[1]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[1]; } } else { label_face[1] = label_face[5]; } } else { if ( label_vertex[4] < label_vertex[5] ) { { EQUIV_nb++; b_label = label_vertex[5]; label_vertex[b_label] = label_vertex[4]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[4]; } } label_exists[4] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { { a_label = label_face[4]; b_label = label_face[5]; if ( a_label < b_label ) { EQUIV_nb++; label_face[b_label] = a_label; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_face[a_label] = b_label; for ( i=a_label+1; i<6; i++ ) if ( label_face[i] == a_label ) label_face[i] = b_label; } } } else { if ( label_vertex[6] < label_vertex[7] ) { { EQUIV_nb++; b_label = label_vertex[7]; label_vertex[b_label] = label_vertex[6]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[6]; } } label_exists[6] = label_exists[7] = 1; }

			    
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    nb_t26 = 1;
                            if ( label_vertex[1] == 1 ) { if ( label_exists[1] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[3] == 3 ) { if ( label_exists[3] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[5] == 5 ) { if ( label_exists[5] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[7] == 7 ) { if ( label_exists[7] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) != 0 ) nb_t26 ++; } }
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[2] : neighbors[2] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) goto END_FALSE; goto END_TRUE ; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[1] : neighbors[1] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    }
		}
	    }
	} else {
	    
	    if ( ( ++TESTS_nb ? neighbors[12] : neighbors[12] ) == 0 ) {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    

			    
			    label_vertex[2] = 2;   label_vertex[3] = 3;   label_vertex[6] = 6;   label_vertex[7] = 7;   
			    label_vertex[0] = 0;   label_vertex[1] = 0;   label_vertex[4] = 0;   label_vertex[5] = 0;   
			    label_exists[2] = label_exists[3] = label_exists[6] = label_exists[7] = 0;
			    label_exists[0] = label_exists[1] = label_exists[4] = label_exists[5] = 1;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
 
			    
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { label_face[4] = label_face[0]; } else { label_vertex[3] = label_vertex[2]; label_exists[2] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( label_face[2] < label_face[4] ) { label_face[4] = label_face[2]; } else { label_face[2] = label_face[4]; } } else { label_vertex[6] = label_vertex[2]; label_exists[2] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( label_face[3] < label_face[4] ) { label_face[4] = label_face[3]; } else { label_face[3] = label_face[4]; } } else { label_vertex[7] = label_vertex[3]; label_exists[3] = label_exists[7] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { label_face[5] = label_face[4]; } else { if ( label_vertex[6] < label_vertex[7] ) { { EQUIV_nb++; b_label = label_vertex[7]; label_vertex[b_label] = label_vertex[6]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[6]; } } else { label_vertex[6] = label_vertex[7]; } label_exists[6] = label_exists[7] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { label_face[2] = label_face[0]; } else { label_vertex[2] = label_vertex[0]; label_exists[0] = label_exists[2] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( label_face[0] < label_face[3] ) { { EQUIV_nb++; b_label = label_face[3]; label_face[b_label] = label_face[0]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[0]; } } } else { if ( label_vertex[1] < label_vertex[3] ) { { EQUIV_nb++; b_label = label_vertex[3]; label_vertex[b_label] = label_vertex[1]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[1]; } } label_exists[1] = label_exists[3] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( label_face[2] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[2]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[2]; } } else { label_face[2] = label_face[5]; } } else { if ( label_vertex[4] < label_vertex[6] ) { { EQUIV_nb++; b_label = label_vertex[6]; label_vertex[b_label] = label_vertex[4]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[4]; } } else { label_vertex[4] = label_vertex[6]; } label_exists[4] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( label_face[3] < label_face[5] ) { { EQUIV_nb++; b_label = label_face[5]; label_face[b_label] = label_face[3]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[3]; } } else { label_face[3] = label_face[5]; } } else { if ( label_vertex[5] < label_vertex[7] ) { { EQUIV_nb++; b_label = label_vertex[7]; label_vertex[b_label] = label_vertex[5]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[5]; } } label_exists[5] = label_exists[7] = 1; }

			    
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    nb_t26 = 1;
			    if ( label_vertex[2] == 2 ) { if ( label_exists[2] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[3] == 3 ) { if ( label_exists[3] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[6] == 6 ) { if ( label_exists[6] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[7] == 7 ) { if ( label_exists[7] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) != 0 ) nb_t26 ++; } }
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_FALSE; goto END_TRUE ; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[6] : neighbors[6] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[3] : neighbors[3] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    }
		}
	    } else {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[8] : neighbors[8] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[5] : neighbors[5] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[7] : neighbors[7] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    goto END_FALSE;
			} else {
			    
			    goto END_TRUE;
			}
		    }
		}
	    }
	}
    } else {
	
	if ( ( ++TESTS_nb ? neighbors[10] : neighbors[10] ) == 0 ) {
	    
	    if ( ( ++TESTS_nb ? neighbors[12] : neighbors[12] ) == 0 ) {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    

			    
			    label_vertex[4] = 4;   label_vertex[5] = 5;   label_vertex[6] = 6;   label_vertex[7] = 7;   
			    label_vertex[0] = 0;   label_vertex[1] = 0;   label_vertex[2] = 0;   label_vertex[3] = 0;   
			    label_exists[4] = label_exists[5] = label_exists[6] = label_exists[7] = 0;	
			    label_exists[0] = label_exists[1] = label_exists[2] = label_exists[3] = 1;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
			    
			     
    			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { label_face[5] = label_face[1] ; } else { label_vertex[5] = label_vertex[4] ; label_exists[4] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( label_face[2] < label_face[5] ) { label_face[5] = label_face[2]; } else { label_face[2] = label_face[5]; } } else { label_vertex[6] = label_vertex[4]; label_exists[4] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( label_face[3] < label_face[5] ) { label_face[5] = label_face[3]; } else { label_face[3] = label_face[5]; } } else { label_vertex[7] = label_vertex[5]; label_exists[5] = label_exists[7] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( label_face[4] < label_face[5] ) { label_face[5] = label_face[4]; } else { label_face[4] = label_face[5]; } } else { if ( label_vertex[6] < label_vertex[7] ) { { EQUIV_nb++; b_label = label_vertex[7]; label_vertex[b_label] = label_vertex[6]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[6]; } } else { label_vertex[6] = label_vertex[7]; } label_exists[6] = label_exists[7] = 1; }

			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { label_face[2] = label_face[1]; } else { label_vertex[4] = label_vertex[0]; label_exists[0] = label_exists[4] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( label_face[1] < label_face[3] ) { { EQUIV_nb++; b_label = label_face[3]; label_face[b_label] = label_face[1]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[1]; } } } else { if ( label_vertex[1] < label_vertex[5] ) { { EQUIV_nb++; b_label = label_vertex[5]; label_vertex[b_label] = label_vertex[1]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[1]; } } label_exists[1] = label_exists[5] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( label_face[2] < label_face[4] ) { { EQUIV_nb++; b_label = label_face[4]; label_face[b_label] = label_face[2]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[2]; } } else { label_face[2] = label_face[4]; } } else { if ( label_vertex[2] < label_vertex[6] ) { { EQUIV_nb++; b_label = label_vertex[6]; label_vertex[b_label] = label_vertex[2]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[2]; } } label_exists[2] = label_exists[6] = 1; }
			    if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( label_face[3] < label_face[4] ) { { EQUIV_nb++; b_label = label_face[4]; label_face[b_label] = label_face[3]; for ( i=b_label+1; i<6; i++ ) if ( label_face[i] == b_label ) label_face[i] = label_face[3]; } } else { label_face[3] = label_face[4]; } } else { if ( label_vertex[3] < label_vertex[7] ) { { EQUIV_nb++; b_label = label_vertex[7]; label_vertex[b_label] = label_vertex[3]; for ( i=b_label+1; i<8; i++ ) if ( label_vertex[i] == b_label ) label_vertex[i] = label_vertex[3]; } } label_exists[3] = label_exists[7] = 1; }

			    
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    nb_t26 = 1;
			    if ( label_vertex[4] == 4 ) { if ( label_exists[4] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[5] == 5 ) { if ( label_exists[5] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[6] == 6 ) { if ( label_exists[6] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) != 0 ) nb_t26 ++; } }
			    if ( label_vertex[7] == 7 ) { if ( label_exists[7] == 1 ) { nb_t26 ++; } else { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) != 0 ) nb_t26 ++; } }
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_FALSE; goto END_TRUE ; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[18] : neighbors[18] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[9] : neighbors[9] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    }
		}
	    } else {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[20] : neighbors[20] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[11] : neighbors[11] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[19] : neighbors[19] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			} else {
			    
			    goto END_TRUE;
			}
		    }
		}
	    }
	} else {
	    
	    if ( ( ++TESTS_nb ? neighbors[12] : neighbors[12] ) == 0 ) {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_FALSE; goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    goto END_FALSE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[24] : neighbors[24] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[15] : neighbors[15] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[21] : neighbors[21] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			} else {
			    
			    goto END_TRUE;
			}
		    }
		}
	    } else {
		
		if ( ( ++TESTS_nb ? neighbors[14] : neighbors[14] ) == 0 ) {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[26] : neighbors[26] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_TRUE; } if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) { if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; goto END_FALSE; } goto END_FALSE;
			} else {
			    
			    if ( ( ++TESTS_nb ? neighbors[17] : neighbors[17] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[23] : neighbors[23] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			} else {
			    
			    goto END_TRUE;
			}
		    }
		} else {
		    
		    if ( ( ++TESTS_nb ? neighbors[16] : neighbors[16] ) == 0 ) {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    if ( ( ++TESTS_nb ? neighbors[25] : neighbors[25] ) == 0 ) goto END_TRUE; else goto END_FALSE;
			} else {
			    
			    goto END_TRUE;
			}
		    } else {
			
			if ( ( ++TESTS_nb ? neighbors[22] : neighbors[22] ) == 0 ) {
			    
			    goto END_TRUE;
			} else {
			    
			    goto END_FALSE;
			}
		    }
		}
	    }
	}
    }

   if ( (nb_t06 == 1) && (nb_t26 == 1) ) goto END_TRUE;
   goto END_FALSE;

END_FALSE :
   return( 0 );
END_TRUE :
   return( 1 );
}
