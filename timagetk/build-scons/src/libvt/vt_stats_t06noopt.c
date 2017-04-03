# 1 "vt_t06noopt.c"


# 1 "./vt_t06noopt.h"


























extern int Compute_NotOptT06( int neighbors[27] );









# 3 "vt_t06noopt.c"




extern int TESTS_nb;
extern int EQUIV_nb;












































int Compute_NotOptT06( int neighbors[27] )
{
    
    




























  int label_neighbor[6] = {9,9,9,9,9,9};
  register int i, nb;
  register int a_label, b_label;

    









    
    nb = 0;

    if ( ( ++TESTS_nb ? neighbors[ 4] : neighbors[ 4] ) == 0 ) { label_neighbor[0] = 0;   nb++; } 

    if ( ( ++TESTS_nb ? neighbors[ 10] : neighbors[ 10] ) == 0 ) { label_neighbor[1] = 1;   nb++; } 

    if ( ( ++TESTS_nb ? neighbors[ 12] : neighbors[ 12] ) == 0 ) { label_neighbor[2] = 2;   nb++; } 

    if ( ( ++TESTS_nb ? neighbors[ 14] : neighbors[ 14] ) == 0 ) { label_neighbor[3] = 3;   nb++; } 

    if ( ( ++TESTS_nb ? neighbors[ 16] : neighbors[ 16] ) == 0 ) { label_neighbor[4] = 4;   nb++; } 

    if ( ( ++TESTS_nb ? neighbors[ 22] : neighbors[ 22] ) == 0 ) { label_neighbor[5] = 5;   nb++; }

    

    if ( nb <= 1 ) return( nb);

    










    









    if ( ( ++TESTS_nb ? neighbors[ 4] : neighbors[ 4] ) == 0 ) {

      if ( ( ++TESTS_nb ? neighbors[ 1] : neighbors[ 1] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 10] : neighbors[ 10] ) == 0 )
	  { a_label = label_neighbor[ 0]; b_label = label_neighbor[ 1]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
      
      if ( ( ++TESTS_nb ? neighbors[ 3] : neighbors[ 3] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 12] : neighbors[ 12] ) == 0 )
	  { a_label = label_neighbor[ 0]; b_label = label_neighbor[ 2]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
      
      if ( ( ++TESTS_nb ? neighbors[ 5] : neighbors[ 5] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 14] : neighbors[ 14] ) == 0 )
	  { a_label = label_neighbor[ 0]; b_label = label_neighbor[ 3]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
      
      if ( ( ++TESTS_nb ? neighbors[ 7] : neighbors[ 7] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 16] : neighbors[ 16] ) == 0 )
	  { a_label = label_neighbor[ 0]; b_label = label_neighbor[ 4]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
    }

    








    if ( ( ++TESTS_nb ? neighbors[ 10] : neighbors[ 10] ) == 0 ) {

      if ( ( ++TESTS_nb ? neighbors[ 9] : neighbors[ 9] ) == 0 ) 
	if ( ( ++TESTS_nb ? neighbors[ 12] : neighbors[ 12] ) == 0 )
	  { a_label = label_neighbor[ 1]; b_label = label_neighbor[ 2]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
      
      if ( ( ++TESTS_nb ? neighbors[ 11] : neighbors[ 11] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 14] : neighbors[ 14] ) == 0 )
	  { a_label = label_neighbor[ 1]; b_label = label_neighbor[ 3]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }

      if ( ( ++TESTS_nb ? neighbors[ 19] : neighbors[ 19] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 22] : neighbors[ 22] ) == 0 )
	  { a_label = label_neighbor[ 1]; b_label = label_neighbor[ 5]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
    }

    



    if ( ( ++TESTS_nb ? neighbors[ 12] : neighbors[ 12] ) == 0 ) {
      
      if ( ( ++TESTS_nb ? neighbors[ 15] : neighbors[ 15] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 16] : neighbors[ 16] ) == 0 )
	  { a_label = label_neighbor[ 2]; b_label = label_neighbor[ 4]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }

      if ( ( ++TESTS_nb ? neighbors[ 21] : neighbors[ 21] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 22] : neighbors[ 22] ) == 0 )
	  { a_label = label_neighbor[ 2]; b_label = label_neighbor[ 5]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
    }
    
    



    if ( ( ++TESTS_nb ? neighbors[ 14] : neighbors[ 14] ) == 0 ) {

      if ( ( ++TESTS_nb ? neighbors[ 17] : neighbors[ 17] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 16] : neighbors[ 16] ) == 0 )
	  { a_label = label_neighbor[ 3]; b_label = label_neighbor[ 4]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }

      if ( ( ++TESTS_nb ? neighbors[ 23] : neighbors[ 23] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 22] : neighbors[ 22] ) == 0 )
	  { a_label = label_neighbor[ 3]; b_label = label_neighbor[ 5]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
    }

    



    if ( ( ++TESTS_nb ? neighbors[ 16] : neighbors[ 16] ) == 0 ) {

      if ( ( ++TESTS_nb ? neighbors[ 25] : neighbors[ 25] ) == 0 )
	if ( ( ++TESTS_nb ? neighbors[ 22] : neighbors[ 22] ) == 0 )
	  { a_label = label_neighbor[ 4]; b_label = label_neighbor[ 5]; if ( a_label < b_label ) { EQUIV_nb++; label_neighbor[b_label] = a_label; for ( i=b_label; i<6; i++ ) if ( label_neighbor[i] == b_label ) label_neighbor[i] = a_label; } else if ( b_label < a_label ) { EQUIV_nb++; label_neighbor[a_label] = b_label; for ( i=a_label; i<6; i++ ) if ( label_neighbor[i] == a_label ) label_neighbor[i] = b_label; } }
    }

    



    nb = 0;
    for ( i = 0; i < 6; i ++ )
	if ( label_neighbor[i] == i ) nb ++;
    
    return( nb );
}
