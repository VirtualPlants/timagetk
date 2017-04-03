#include <vt_t06t26_simple.h>

/*---
  #6-neighbors est le nombre de 6-neighbors appartenant au fond ( 64 possibilites )

  #6-neighbors = 0 ( 1 possibilite )
     nb_t06 = 0;
     nb_t26 = 1;

  #6-neighbors = 1 ( 6 possibilites )
     nb_t06 = 1;
     nb_t26 = 1;

  #6-neighbors = 2 ( 15 possibilites )
     * en opposition ( 3 possibilites ) 
       nb_t06 = 2;
     * 18-voisins    ( 12 possibilites )
       nb_t06 = 1,2 (selon test )
       
       . . .   . O a   . . .
       . . .   . X O   . . .
       . . .   . . .   . . .
        
     nb_t26 = 1;

  #6-neighbors = 3 ( 20 possibilites )
     * les 3 dans un octant ( 8 possibilites )
       nb_t06 = 1,2,3 (selon test )
       si nb_t06 == 1, nb_t26 = 1,2 (selon test)
       sinon           nb_t26 = 1

       . a z   . O c   . . .
       . O b   . X O   . . .
       . . .   . . .   . . .

     * les 3 dans un plan ( 12 possibilites )
       nb_t06 = 1,2,3 (selon test )
       nb_t26 = 1;

       . . .   . O a   . . .
       . . .   . X O   . . .
       . . .   . O b   . . .

  #6-neighbors = 4 ( 15 possibilites )
     * les 4 dans un meme plan ( 3 possibilites ) 
       nb_t06 = 1,2,3,4 (selon test)
       si nb_t06 == 1, nb_t26 = 1,2 (selon test)
       sinon           nb_t26 = 1

       . . .   d O a   . . .   Les 4 points a testes sont
       . . .   O X O   . . .   dans un ordre particulier
       . . .   c O b   . . .

     * les 4 dans 2 octants    ( 12 possibilites )
       nb_t06 = 1,2,3,4 (selon test)
       si nb_t06 == 1,       nb_t26 = 1,2,3 (selon test)
       sinon si nb_t06 == 2, nb_t26 = 1,2 (selon test)
             sinon           nb_t26 = 1

       . a y   . O b   . . .   Les 5+2 points a testes sont
       . O c   . X O   . . .   dans un ordre particulier
       . d z   . O e   . . .

       
---*/

/*-------------------- statistics --------------------*/
#if defined(_STATISTICS_)
extern int TESTS_nb;
extern int EQUIV_nb;
#define GET_VAL(tab,index) ( ++TESTS_nb ? tab[index] : tab[index] )
#else
#define GET_VAL(tab,index) tab[index]
#endif
/*----------------------------------------------------*/

#define _FALSE_ 0
#define _TRUE_  1

#define CASE_0 goto END_FALSE
             
#define CASE_1 goto END_TRUE
             
#define CASE_2a goto END_FALSE

#define CASE_2b(tab,a) if ( GET_VAL(tab,a) == 0 ) goto END_TRUE; \
		       else                       goto END_FALSE

#define CASE_3a(tab,a,b,c,z)  if ( GET_VAL(tab,a) == 0 )  {                            \
			          if ( GET_VAL(tab,b) == 0 ) {                         \
			               if ( GET_VAL(tab,c) == 0 ) {                    \
                                          if ( GET_VAL(tab,z) == 0 ) goto END_TRUE;    \
                                          goto END_FALSE;                              \
                                      }                                                \
                                      goto END_TRUE;                                   \
                                  }                                                    \
                                  if ( GET_VAL(tab,c) == 0 ) goto END_TRUE;            \
                                  goto END_FALSE;                                      \
                              }                                                        \
                              if ( GET_VAL(tab,b) == 0 ) {                             \
                                  if ( GET_VAL(tab,c) == 0 ) goto END_TRUE;            \
                                  goto END_FALSE;                                      \
                              }                                                        \
                              goto END_FALSE

#define CASE_3b(tab,a,b)  if ( GET_VAL(tab,a) == 0 ) {                  \
			      if ( GET_VAL(tab,b) == 0 ) goto END_TRUE; \
			      goto END_FALSE;                           \
		          }                                             \
                          goto END_FALSE

#define CASE_4a(tab,a,b,c,d) if ( GET_VAL(tab,a) == 0 ) {                           \
                                 if ( GET_VAL(tab,b) == 0 ) {                       \
                                     if ( GET_VAL(tab,c) == 0 ) {                   \
                                         if ( GET_VAL(tab,d) == 0 ) goto END_FALSE; \
                                         goto END_TRUE ;                            \
			             }                                              \
                                     if ( GET_VAL(tab,d) == 0 ) goto END_TRUE;      \
                                     goto END_FALSE;                                \
			         }                                                  \
                                 if ( GET_VAL(tab,c) == 0 ) {                       \
                                     if ( GET_VAL(tab,d) == 0 ) goto END_TRUE;      \
                                     goto END_FALSE;                                \
			         }                                                  \
                                 goto END_FALSE;                                    \
			     }                                                      \
                             if ( GET_VAL(tab,b) == 0 ) {                           \
                                 if ( GET_VAL(tab,c) == 0 ) {                       \
                                     if ( GET_VAL(tab,d) == 0 ) goto END_TRUE;      \
                                     goto END_FALSE;                                \
			         }                                                  \
                                 goto END_FALSE;                                    \
			     }                                                      \
                             goto END_FALSE

#define CASE_4b(tab,a,b,c,d,e,y,z) if ( GET_VAL(tab,a) == 0 ) {                                      \
                                       if ( GET_VAL(tab,b) == 0 ) {                                  \
                                           if ( GET_VAL(tab,c) == 0 ) {                              \
                                               if ( GET_VAL(tab,d) == 0 ) {                          \
                                                   if ( GET_VAL(tab,e) == 0 ) {                      \
                                                       if ( GET_VAL(tab,y) == 0 ) {                  \
                                                           if ( GET_VAL(tab,z) == 0 ) goto END_TRUE; \
                                                           goto END_FALSE;                           \
			                               }                                             \
                                                       goto END_FALSE;                               \
                                                   }                                                 \
                                                   if ( GET_VAL(tab,y) == 0 ) goto END_TRUE;         \
                                                   goto END_FALSE;                                   \
                                               }                                                     \
                                               if ( GET_VAL(tab,e) == 0 ) {                          \
                                                   if ( GET_VAL(tab,y) == 0 ) goto END_TRUE;         \
                                                   goto END_FALSE;                                   \
			                       }                                                     \
                                               goto END_FALSE;                                       \
			                   }                                                         \
                                           if ( GET_VAL(tab,d) == 0 ) {                              \
                                               if ( GET_VAL(tab,e) == 0 ) goto END_FALSE;            \
                                               goto END_TRUE;                                        \
			                   }                                                         \
                                           if ( GET_VAL(tab,e) == 0 ) goto END_TRUE;                 \
                                           goto END_FALSE;                                           \
				       }                                                             \
                                       if ( GET_VAL(tab,c) == 0 ) {                                  \
                                           if ( GET_VAL(tab,d) == 0 ) {                              \
                                               if ( GET_VAL(tab,e) == 0 ) {                          \
                                                   if ( GET_VAL(tab,z) == 0 ) goto END_TRUE;         \
                                                   goto END_FALSE;                                   \
				               }                                                     \
                                               goto END_TRUE;                                        \
				           }                                                         \
                                           if ( GET_VAL(tab,e) == 0 ) goto END_TRUE;                 \
                                           goto END_FALSE;                                           \
				       }                                                             \
                                       if ( GET_VAL(tab,d) == 0 ) {                                  \
                                           if ( GET_VAL(tab,e) == 0 ) goto END_TRUE;                 \
                                           goto END_FALSE;                                           \
				       }                                                             \
                                       goto END_FALSE;                                               \
			           }                                                                 \
                                   if ( GET_VAL(tab,b) == 0 ) {                                      \
                                       if ( GET_VAL(tab,c) == 0 ) {                                  \
                                           if ( GET_VAL(tab,d) == 0 ) {                              \
                                               if ( GET_VAL(tab,e) == 0 ) {                          \
                                                   if ( GET_VAL(tab,z) == 0 ) goto END_TRUE;         \
                                                   goto END_FALSE;                                   \
				               }                                                     \
                                               goto END_TRUE;                                        \
				           }                                                         \
                                           if ( GET_VAL(tab,e) == 0 ) goto END_TRUE;                 \
                                           goto END_FALSE;                                           \
				       }                                                             \
                                       if ( GET_VAL(tab,d) == 0 ) {                                  \
                                           if ( GET_VAL(tab,e) == 0 ) goto END_TRUE;                 \
                                           goto END_FALSE;                                           \
				       }                                                             \
                                       goto END_FALSE;                                               \
				   }                                                                 \
                                   goto END_FALSE



#if defined(_STATISTICS_)

#define _ONE_T06_EQUIVALENCE( NEW_LABEL, OLD_CLASS ) { \
  EQUIV_nb++; \
  b_label = label_face[OLD_CLASS]; \
  label_face[b_label] = NEW_LABEL; \
  for ( i=b_label+1; i<6; i++ ) \
    if ( label_face[i] == b_label ) label_face[i] = NEW_LABEL; \
}
#define _TWO_T06_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  EQUIV_nb++; \
  label_face[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL+1; i<6; i++ ) \
    if ( label_face[i] == OLD_LABEL ) label_face[i] = NEW_LABEL; \
}

#define _ONE_T26_EQUIVALENCE( NEW_LABEL, OLD_CLASS ) { \
  EQUIV_nb++; \
  b_label = label_vertex[OLD_CLASS]; \
  label_vertex[b_label] = NEW_LABEL; \
  for ( i=b_label+1; i<8; i++ ) \
    if ( label_vertex[i] == b_label ) label_vertex[i] = NEW_LABEL; \
}
#define _TWO_T26_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  EQUIV_nb++; \
  label_vertex[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL+1; i<8; i++ ) \
    if ( label_vertex[i] == OLD_LABEL ) label_vertex[i] = NEW_LABEL; \
}


#else

#define _ONE_T06_EQUIVALENCE( NEW_LABEL, OLD_CLASS ) { \
  b_label = label_face[OLD_CLASS]; \
  label_face[b_label] = NEW_LABEL; \
  for ( i=b_label+1; i<6; i++ ) \
    if ( label_face[i] == b_label ) label_face[i] = NEW_LABEL; \
}
#define _TWO_T06_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  label_face[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL+1; i<6; i++ ) \
    if ( label_face[i] == OLD_LABEL ) label_face[i] = NEW_LABEL; \
}

#define _ONE_T26_EQUIVALENCE( NEW_LABEL, OLD_CLASS ) { \
  b_label = label_vertex[OLD_CLASS]; \
  label_vertex[b_label] = NEW_LABEL; \
  for ( i=b_label+1; i<8; i++ ) \
    if ( label_vertex[i] == b_label ) label_vertex[i] = NEW_LABEL; \
}
#define _TWO_T26_EQUIVALENCE( NEW_LABEL, OLD_LABEL ) { \
  label_vertex[OLD_LABEL] = NEW_LABEL; \
  for ( i=OLD_LABEL+1; i<8; i++ ) \
    if ( label_vertex[i] == OLD_LABEL ) label_vertex[i] = NEW_LABEL; \
}

#endif

#define _SET_T06_EQUIVALENCE( A_CLASS, B_CLASS ) { \
  a_label = label_face[A_CLASS]; \
  b_label = label_face[B_CLASS]; \
  if ( a_label < b_label ) \
    _TWO_T06_EQUIVALENCE( a_label, b_label ) \
  else if ( b_label <  a_label ) \
    _TWO_T06_EQUIVALENCE( b_label, a_label ) \
}

#define _SET_T26_EQUIVALENCE( A_CLASS, B_CLASS ) { \
  a_label = label_vertex[A_CLASS]; \
  b_label = label_vertex[B_CLASS]; \
  if ( a_label < b_label ) \
    _TWO_T26_EQUIVALENCE( a_label, b_label ) \
  else if ( b_label <  a_label ) \
    _TWO_T26_EQUIVALENCE( b_label, a_label ) \
}



#define _EXAMINATE_01 if ( GET_VAL( neighbors, 1 ) == 0 ) {           \
                          label_face[1] = label_face[0];              \
		      } else {                                        \
                          label_vertex[1] = label_vertex[0];          \
		          label_exists[0] = label_exists[1] = _TRUE_; \
		      }
#define _EXAMINATE_03_A if ( GET_VAL( neighbors, 3 ) == 0 ) {         \
                            label_face[2] = label_face[0];            \
		      } else {                                        \
			    label_vertex[2] = label_vertex[0];        \
		          label_exists[0] = label_exists[2] = _TRUE_; \
		      }

#define _EXAMINATE_03_B \
  if ( GET_VAL( neighbors, 3 ) == 0 ) {           \
    if ( label_face[0] < label_face[2] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[0], 2 ) }  \
  } else {                                        \
    if ( label_vertex[0] < label_vertex[2] ) {    \
      _ONE_T26_EQUIVALENCE( label_vertex[0], 2 ) }    \
    label_exists[0] = label_exists[2] = _TRUE_;   \
}

#define _EXAMINATE_05_A \
  if ( GET_VAL( neighbors, 5 ) == 0 ) {         \
    label_face[3] = label_face[0];              \
  } else {                                      \
    label_vertex[3] = label_vertex[1];          \
    label_exists[1] = label_exists[3] = _TRUE_; \
}

#define _EXAMINATE_05_B \
  if ( GET_VAL( neighbors, 5 ) == 0 ) {           \
    if ( label_face[0] < label_face[3] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[0], 3 ) }  \
  } else {                                        \
    if ( label_vertex[1] < label_vertex[3] ) {    \
      _ONE_T26_EQUIVALENCE( label_vertex[1], 3 ) }    \
    else { label_vertex[1] = label_vertex[3]; }   \
    label_exists[1] = label_exists[3] = _TRUE_;   \
  }

#define _EXAMINATE_05_C \
  if ( GET_VAL( neighbors, 5 ) == 0 ) {           \
    if ( label_face[0] < label_face[3] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[0], 3 ) }  \
  } else {                                        \
      if ( label_vertex[1] < label_vertex[3] ) {  \
        _ONE_T26_EQUIVALENCE( label_vertex[1], 3 ) }  \
      label_exists[1] = label_exists[3] = _TRUE_; \
  }

#define _EXAMINATE_07_A \
  if ( GET_VAL( neighbors, 7 ) == 0 ) {         \
    label_face[4] = label_face[0];              \
  } else {                                      \
    if ( label_vertex[2] < label_vertex[3] ) {  \
      _ONE_T26_EQUIVALENCE( label_vertex[2], 3 ) }  \
    else {                                      \
      label_vertex[2] = label_vertex[3]; }      \
    label_exists[2] = label_exists[3] = _TRUE_; \
  }

#define _EXAMINATE_07_B \
  if ( GET_VAL( neighbors, 7 ) == 0 ) {           \
    if ( label_face[0] < label_face[4] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[0], 4 ) }  \
  } else {                                        \
    if ( label_vertex[2] < label_vertex[3] ) {    \
      _ONE_T26_EQUIVALENCE( label_vertex[2], 3 ) }    \
    else                                     {    \
      label_vertex[2] = label_vertex[3]; }        \
    label_exists[2] = label_exists[3] = _TRUE_;   \
  }

#define _EXAMINATE_07_C \
  if ( GET_VAL( neighbors, 7 ) == 0 ) {           \
    if ( label_face[0] < label_face[4] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[0], 4 ) }  \
  } else {                                        \
    if ( label_vertex[2] < label_vertex[3] ) {    \
      _ONE_T26_EQUIVALENCE( label_vertex[2], 3 ) }    \
    label_exists[2] = label_exists[3] = _TRUE_;   \
  }

#define _EXAMINATE_07_D \
  if ( GET_VAL( neighbors, 7 ) == 0 ) {         \
    label_face[4] = label_face[0];              \
  } else {                                      \
    label_vertex[3] = label_vertex[2];          \
    label_exists[2] = label_exists[3] = _TRUE_; \
  }

#define _EXAMINATE_09_A \
  if ( GET_VAL( neighbors, 9 ) == 0 ) {            \
     if ( label_face[1] < label_face[2] ) {        \
       _ONE_T06_EQUIVALENCE( label_face[1], 2 ) }  \
     else { label_face[1] = label_face[2]; }       \
  } else {                                         \
    if ( label_vertex[0] < label_vertex[4] ) {     \
      _ONE_T26_EQUIVALENCE( label_vertex[0], 4 ) }     \
    label_exists[0] = label_exists[4] = _TRUE_;    \
  }

#define _EXAMINATE_09_B \
  if ( GET_VAL( neighbors, 9 ) == 0 ) {                                                 \
    if ( label_face[1] < label_face[2] ) { label_face[2] = label_face[1]; }             \
    else                                 { label_face[1] = label_face[2]; }             \
  } else {                                                                                \
    if ( label_vertex[0] < label_vertex[4] ) { _ONE_T26_EQUIVALENCE( label_vertex[0], 4 ) } \
    label_exists[0] = label_exists[4] = _TRUE_;                                         \
  }

#define _EXAMINATE_09_C if ( GET_VAL( neighbors, 9 ) == 0 ) {         \
                        label_face[2] = label_face[1];                \
		      } else {                                        \
                          label_vertex[4] = label_vertex[0];          \
		          label_exists[0] = label_exists[4] = _TRUE_; \
		      }
#define _EXAMINATE_09_D if ( GET_VAL( neighbors, 9 ) == 0 ) {                                     \
                          if ( label_face[1] < label_face[2] ) { label_face[2] = label_face[1]; } \
			  else                                 { label_face[1] = label_face[2]; } \
		      } else {                                                                    \
                          label_vertex[4] = label_vertex[0];                                      \
		          label_exists[0] = label_exists[4] = _TRUE_;                             \
		      }

#define _EXAMINATE_11_A \
  if ( GET_VAL( neighbors, 11 ) == 0 ) {                                                \
    if ( label_face[1] < label_face[3] ) { _ONE_T06_EQUIVALENCE( label_face[1], 3 ) }       \
    else                                 { label_face[1] = label_face[3]; }             \
  } else {                                                                                \
    if ( label_vertex[1] < label_vertex[5] ) { _ONE_T26_EQUIVALENCE( label_vertex[1], 5 ) } \
    else                                     { label_vertex[1] = label_vertex[5]; }     \
    label_exists[1] = label_exists[5] = _TRUE_;                                         \
  }

#define _EXAMINATE_11_B \
  if ( GET_VAL( neighbors, 11 ) == 0 ) {                                                \
    if ( label_face[1] < label_face[3] ) { label_face[3] = label_face[1]; }             \
    else                                 { label_face[1] = label_face[3]; }             \
  } else {                                                                                \
    if ( label_vertex[1] < label_vertex[5] ) { _ONE_T26_EQUIVALENCE( label_vertex[1], 5 ) } \
    else                                     { label_vertex[1] = label_vertex[5]; }     \
    label_exists[1] = label_exists[5] = _TRUE_;                                         \
  }

#define _EXAMINATE_11_C if ( GET_VAL( neighbors, 11 ) == 0 ) {        \
                          label_face[3] = label_face[1];              \
		      } else {                                        \
			  label_vertex[5] = label_vertex[1];          \
		          label_exists[1] = label_exists[5] = _TRUE_; \
		      }

#define _EXAMINATE_11_D \
if ( GET_VAL( neighbors, 11 ) == 0 ) {                                    \
                          if ( label_face[1] < label_face[3] ) { label_face[3] = label_face[1]; } \
			  else                                 { label_face[1] = label_face[3]; } \
		      } else {                                                                    \
			  label_vertex[5] = label_vertex[1];                                      \
		          label_exists[1] = label_exists[5] = _TRUE_;                             \
		      }

#define _EXAMINATE_11_E \
  if ( GET_VAL( neighbors, 11 ) == 0 ) {                                                \
    if ( label_face[1] < label_face[3] ) { _ONE_T06_EQUIVALENCE( label_face[1], 3 ) }       \
  } else {                                                                                \
    if ( label_vertex[1] < label_vertex[5] ) { _ONE_T26_EQUIVALENCE( label_vertex[1], 5 ) } \
    label_exists[1] = label_exists[5] = _TRUE_;                                         \
  }

#define _EXAMINATE_15_A \
  if ( GET_VAL( neighbors, 15 ) == 0 ) {        \
    _SET_T06_EQUIVALENCE( 2, 4 )                \
  } else {                                      \
    _SET_T26_EQUIVALENCE( 2, 6 )                \
    label_exists[2] = label_exists[6] = _TRUE_; \
  }

#define _EXAMINATE_15_B \
  if ( GET_VAL( neighbors, 15 ) == 0 ) {          \
    if ( label_face[4] < label_face[2] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[4], 2 ) }  \
    else {                                        \
    label_face[4] = label_face[2]; }              \
  } else {                                        \
    _SET_T26_EQUIVALENCE( 2, 6 )                  \
    label_exists[2] = label_exists[6] = _TRUE_;   \
  }

#define _EXAMINATE_15_C if ( GET_VAL( neighbors, 15 ) == 0 ) {        \
                            label_face[4] = label_face[2];            \
		      } else {                                        \
			  label_vertex[6] = label_vertex[2];          \
		          label_exists[2] = label_exists[6] = _TRUE_; \
		      }
#define _EXAMINATE_15_D if ( GET_VAL( neighbors, 15 ) == 0 ) {                                    \
                          if ( label_face[2] < label_face[4] ) { label_face[4] = label_face[2]; } \
			  else                                 { label_face[2] = label_face[4]; } \
		      } else {                                                                    \
                          label_vertex[6] = label_vertex[2];                                      \
		          label_exists[2] = label_exists[6] = _TRUE_;                             \
		      }
#define _EXAMINATE_15_E \
  if ( GET_VAL( neighbors, 15 ) == 0 ) {                                                \
    if ( label_face[2] < label_face[4] ) { _ONE_T06_EQUIVALENCE( label_face[2], 4 ) }       \
    else                                 { label_face[2] = label_face[4]; }             \
  } else {                                                                                \
    if ( label_vertex[2] < label_vertex[6] ) { _ONE_T26_EQUIVALENCE( label_vertex[2], 6 ) } \
    label_exists[2] = label_exists[6] = _TRUE_;                                         \
  }

#define _EXAMINATE_17_A \
  if ( GET_VAL( neighbors, 17 ) == 0 ) {         \
    _SET_T06_EQUIVALENCE( 3, 4 )                 \
  } else {                                       \
    _SET_T26_EQUIVALENCE( 3, 7 )                 \
    label_exists[3] = label_exists[7] = _TRUE_;  \
  }

#define _EXAMINATE_17_B if ( GET_VAL( neighbors, 17 ) == 0 ) {        \
                          label_face[4] = label_face[3];              \
		      } else {                                        \
			  label_vertex[7] = label_vertex[3];          \
		          label_exists[3] = label_exists[7] = _TRUE_; \
		      }

#define _EXAMINATE_17_C if ( GET_VAL( neighbors, 17 ) == 0 ) {                                    \
                          if ( label_face[3] < label_face[4] ) { label_face[4] = label_face[3]; } \
			  else                                 { label_face[3] = label_face[4]; } \
		      } else {                                                                    \
			  label_vertex[7] = label_vertex[3];                                      \
		          label_exists[3] = label_exists[7] = _TRUE_;                             \
		      }

#define _EXAMINATE_17_D \
  if ( GET_VAL( neighbors, 17 ) == 0 ) {                                                \
    if ( label_face[3] < label_face[4] ) { _ONE_T06_EQUIVALENCE( label_face[3], 4 ) }       \
    else                                 { label_face[3] = label_face[4]; }             \
  } else {                                                                                \
    if ( label_vertex[3] < label_vertex[7] ) { _ONE_T26_EQUIVALENCE( label_vertex[3], 7 ) } \
    label_exists[3] = label_exists[7] = _TRUE_;                                         \
  }

#define _EXAMINATE_19_A if ( GET_VAL( neighbors, 19 ) == 0 ) {        \
                          label_face[5] = label_face[1] ;             \
		      } else {                                        \
			  label_vertex[5] = label_vertex[4] ;         \
		          label_exists[4] = label_exists[5] = _TRUE_; \
		      }

#define _EXAMINATE_19_B \
  if ( GET_VAL( neighbors, 19 ) == 0 ) {                                                \
    label_face[5] = label_face[1] ;                                                     \
  } else {                                                                                \
    if ( label_vertex[4] < label_vertex[5] ) { _ONE_T26_EQUIVALENCE( label_vertex[4], 5 ) } \
    else                                     { label_vertex[4] = label_vertex[5]; }     \
    label_exists[4] = label_exists[5] = _TRUE_;                                         \
  }

#define _EXAMINATE_19_C \
  if ( GET_VAL( neighbors, 19 ) == 0 ) {          \
    if ( label_face[1] < label_face[5] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[1], 5 ) }  \
    else  {                                       \
      label_face[1] = label_face[5]; }            \
  } else {                                        \
    _SET_T26_EQUIVALENCE( 4, 5 )                  \
    label_exists[4] = label_exists[5] = _TRUE_;   \
  }

#define _EXAMINATE_19_D \
  if ( GET_VAL( neighbors, 19 ) == 0 ) {                                                \
    if ( label_face[1] < label_face[5] ) { _ONE_T06_EQUIVALENCE( label_face[1], 5 ) }       \
    else                                 { label_face[1] = label_face[5];   }           \
  } else {                                                                                \
    if ( label_vertex[4] < label_vertex[5] ) { _ONE_T26_EQUIVALENCE( label_vertex[4], 5 ) } \
    label_exists[4] = label_exists[5] = _TRUE_;                                         \
  }

#define _EXAMINATE_21_A \
  if ( GET_VAL( neighbors, 21 ) == 0 ) {                                          \
    if ( label_face[2] < label_face[5] ) { _ONE_T06_EQUIVALENCE( label_face[2], 5 ) } \
    else                                 { label_face[2] = label_face[5]; }       \
  } else {                                                                          \
    label_vertex[6] = label_vertex[4];                                            \
    label_exists[4] = label_exists[6] = _TRUE_;                                   \
  }

#define _EXAMINATE_21_B \
  if ( GET_VAL( neighbors, 21 ) == 0 ) {          \
    if ( label_face[2] < label_face[5] ) {        \
      _ONE_T06_EQUIVALENCE( label_face[2], 5 ) }  \
    else {                                        \
      label_face[2] = label_face[5]; }            \
  } else {                                        \
    _SET_T26_EQUIVALENCE( 4, 6 )                  \
    label_exists[4] = label_exists[6] = _TRUE_;   \
  }

#define _EXAMINATE_21_C \
  if ( GET_VAL( neighbors, 21 ) == 0 ) {                                                \
    label_face[5] = label_face[2];                                                      \
  } else {                                                                                \
    if ( label_vertex[4] < label_vertex[6] ) { _ONE_T26_EQUIVALENCE( label_vertex[4], 6 ) } \
    else                                     { label_vertex[4] = label_vertex[6]; }     \
    label_exists[4] = label_exists[6] = _TRUE_;                                         \
  }

#define _EXAMINATE_21_D \
  if ( GET_VAL( neighbors, 21 ) == 0 ) {                                                \
    if ( label_face[2] < label_face[5] ) { _ONE_T06_EQUIVALENCE( label_face[2], 5 ) }       \
    else                                 { label_face[2] = label_face[5]; }             \
  } else {                                                                                \
    if ( label_vertex[4] < label_vertex[6] ) { _ONE_T26_EQUIVALENCE( label_vertex[4], 6 ) } \
    else                                     { label_vertex[4] = label_vertex[6]; }     \
    label_exists[4] = label_exists[6] = _TRUE_;                                         \
  }

#define _EXAMINATE_21_E if ( GET_VAL( neighbors, 21 ) == 0 ) {                                    \
                          if ( label_face[2] < label_face[5] ) { label_face[5] = label_face[2]; } \
			  else                                 { label_face[2] = label_face[5]; } \
		      } else {                                                                    \
			  label_vertex[6] = label_vertex[4];                                      \
		          label_exists[4] = label_exists[6] = _TRUE_;                             \
		      }

#define _EXAMINATE_23_A \
  if ( GET_VAL( neighbors, 23 ) == 0 ) {                                          \
    if ( label_face[3] < label_face[5] ) { _ONE_T06_EQUIVALENCE( label_face[3], 5 ) } \
    else                                 { label_face[3] = label_face[5]; }       \
  } else {                                                                          \
    label_vertex[7] = label_vertex[5];                                            \
    label_exists[5] = label_exists[7] = _TRUE_;                                   \
  }

#define _EXAMINATE_23_B \
  if ( GET_VAL( neighbors, 23 ) == 0 ) {         \
    _SET_T06_EQUIVALENCE( 3, 5 )                 \
  } else {                                       \
    _SET_T26_EQUIVALENCE( 5, 7 )                 \
    label_exists[5] = label_exists[7] = _TRUE_;  \
  }

#define _EXAMINATE_23_C \
  if ( GET_VAL( neighbors, 23 ) == 0 ) {                                                \
    label_face[5] = label_face[3];                                                      \
  } else {                                                                                \
    if ( label_vertex[5] < label_vertex[7] ) { _ONE_T26_EQUIVALENCE( label_vertex[5], 7 ) } \
    else                                     { label_vertex[5] = label_vertex[7]; }     \
    label_exists[5] = label_exists[7] = _TRUE_;                                         \
  }

#define _EXAMINATE_23_D \
  if ( GET_VAL( neighbors, 23 ) == 0 ) {                                                \
    if ( label_face[3] < label_face[5] ) { _ONE_T06_EQUIVALENCE( label_face[3], 5 ) }       \
    else                                 { label_face[3] = label_face[5]; }             \
  } else {                                                                                \
    if ( label_vertex[5] < label_vertex[7] ) { _ONE_T26_EQUIVALENCE( label_vertex[5], 7 ) } \
    label_exists[5] = label_exists[7] = _TRUE_;                                         \
  }

#define _EXAMINATE_23_E if ( GET_VAL( neighbors, 23 ) == 0 ) {                                    \
                          if ( label_face[3] < label_face[5] ) { label_face[5] = label_face[3]; } \
			  else                                 { label_face[3] = label_face[5]; } \
		      } else {                                                                    \
			  label_vertex[7] = label_vertex[5];                                      \
		          label_exists[5] = label_exists[7] = _TRUE_;                             \
		      }

#define _EXAMINATE_25_A \
  if ( GET_VAL( neighbors, 25 ) == 0 ) {                                                \
    if ( label_face[4] < label_face[5] ) { _ONE_T06_EQUIVALENCE( label_face[4], 5 ) }       \
    else                                 { label_face[4] = label_face[5]; }             \
  } else {                                                                                \
    if ( label_vertex[6] < label_vertex[7] ) { _ONE_T26_EQUIVALENCE( label_vertex[6], 7 ) } \
    else                                     { label_vertex[6] = label_vertex[7]; }     \
    label_exists[6] = label_exists[7] = _TRUE_;                                         \
  }

#define _EXAMINATE_25_B \
  if ( GET_VAL( neighbors, 25 ) == 0 ) {         \
    _SET_T06_EQUIVALENCE( 4, 5 )                 \
  } else {                                       \
    _SET_T26_EQUIVALENCE( 6, 7 )                 \
    label_exists[6] = label_exists[7] = _TRUE_;  \
  }

#define _EXAMINATE_25_C \
  if ( GET_VAL( neighbors, 25 ) == 0 ) {        \
    _SET_T06_EQUIVALENCE( 4, 5 )                \
  } else {                                      \
    if ( label_vertex[6] < label_vertex[7] ) {  \
      _ONE_T26_EQUIVALENCE( label_vertex[6], 7 ) }  \
    label_exists[6] = label_exists[7] = _TRUE_; \
  }

#define _EXAMINATE_25_D \
  if ( GET_VAL( neighbors, 25 ) == 0 ) {                                                \
    label_face[5] = label_face[4];                                                      \
  } else {                                                                                \
    if ( label_vertex[6] < label_vertex[7] ) { _ONE_T26_EQUIVALENCE( label_vertex[6], 7 ) } \
    else                                     { label_vertex[6] = label_vertex[7]; }     \
    label_exists[6] = label_exists[7] = _TRUE_;                                         \
  }

#define _EXAMINATE_25_E \
  if ( GET_VAL( neighbors, 25 ) == 0 ) {                                                \
    if ( label_face[4] < label_face[5] ) { label_face[5] = label_face[4]; }             \
    else                                 { label_face[4] = label_face[5]; }             \
  } else {                                                                                \
    if ( label_vertex[6] < label_vertex[7] ) { _ONE_T26_EQUIVALENCE( label_vertex[6], 7 ) } \
    else                                     { label_vertex[6] = label_vertex[7]; }     \
    label_exists[6] = label_exists[7] = _TRUE_;                                         \
  }

#define _T26_COUNT_0 if ( label_exists[0] == _TRUE_ ) {         nb_t26 ++; } \
		     else { if ( GET_VAL( neighbors, 0 ) != 0 ) nb_t26 ++; }
#define _T26_COUNT_1 if ( label_vertex[1] == 1 ) {                               \
		         if ( label_exists[1] == _TRUE_ ) {         nb_t26 ++; } \
		         else { if ( GET_VAL( neighbors, 2 ) != 0 ) nb_t26 ++; } \
		     }
#define _T26_COUNT_2 if ( label_vertex[2] == 2 ) {                               \
		         if ( label_exists[2] == _TRUE_ ) {         nb_t26 ++; } \
		         else { if ( GET_VAL( neighbors, 6 ) != 0 ) nb_t26 ++; } \
		     }
#define _T26_COUNT_3 if ( label_vertex[3] == 3 ) {                               \
		         if ( label_exists[3] == _TRUE_ ) {         nb_t26 ++; } \
		         else { if ( GET_VAL( neighbors, 8 ) != 0 ) nb_t26 ++; } \
		     }
#define _T26_COUNT_4 if ( label_vertex[4] == 4 ) {                                \
		         if ( label_exists[4] == _TRUE_ ) {          nb_t26 ++; } \
		         else { if ( GET_VAL( neighbors, 18 ) != 0 ) nb_t26 ++; } \
		     }
#define _T26_COUNT_5 if ( label_vertex[5] == 5 ) {                                \
		         if ( label_exists[5] == _TRUE_ ) {          nb_t26 ++; } \
		         else { if ( GET_VAL( neighbors, 20 ) != 0 ) nb_t26 ++; } \
		     }
#define _T26_COUNT_6 if ( label_vertex[6] == 6 ) {                                \
		         if ( label_exists[6] == _TRUE_ ) {          nb_t26 ++; } \
		         else { if ( GET_VAL( neighbors, 24 ) != 0 ) nb_t26 ++; } \
		     }
#define _T26_COUNT_7 if ( label_vertex[7] == 7 ) {                                \
		         if ( label_exists[7] == _TRUE_ ) {          nb_t26 ++; } \
		         else { if ( GET_VAL( neighbors, 26 ) != 0 ) nb_t26 ++; } \
		     }





int Compute_Simple( int neighbors[27] )
{
    int label_vertex[8], label_exists[8];
    int label_face[6];
    register int i, nb_t26, nb_t06;
    register int a_label, b_label;

    nb_t06 = 0;
    nb_t26 = 0;

    if ( GET_VAL(neighbors,4) == 0 ) {
	/*  6-neighbors :  0 . . . . . */
	if ( GET_VAL(neighbors,10) == 0 ) {
	    /*  6-neighbors :  0 0 . . . . */
	    if ( GET_VAL(neighbors,12) == 0 ) {
		/*  6-neighbors :  0 0 0 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  0 0 0 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 0 0 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 0 0 0 0 */

			    /*--- initialization ---*/
			    for ( i = 0; i < 8; i ++ ) {
				label_vertex[i] = i;
				label_exists[i] = _FALSE_;
			    }
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
			    
			    /*--- all 18-neighbors ---*/
			    _EXAMINATE_01
			    _EXAMINATE_03_A
			    _EXAMINATE_05_A
			    _EXAMINATE_07_A

			    _EXAMINATE_19_A
			    _EXAMINATE_21_A
			    _EXAMINATE_23_A
			    _EXAMINATE_25_A

			    _EXAMINATE_09_A
			    _EXAMINATE_11_A
			    _EXAMINATE_15_A
			    _EXAMINATE_17_A

			    /*--- we count the connected components ---*/
			    for ( i = 0; i < 6; i ++ )
				if ( label_face[i] == i ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;

			    _T26_COUNT_0
			    _T26_COUNT_1
			    _T26_COUNT_2
			    _T26_COUNT_3
			    _T26_COUNT_4
			    _T26_COUNT_5
			    _T26_COUNT_6
			    _T26_COUNT_7
			    if ( nb_t26 != 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    /*  6-neighbors :  0 0 0 0 0 1 */
			    
			    /*--- initialization ---*/
			    label_vertex[0] = 0;   label_vertex[1] = 1;   label_vertex[2] = 2;   label_vertex[3] = 3;   
			    label_vertex[4] = 4;   label_vertex[5] = 4;   label_vertex[6] = 4;   label_vertex[7] = 4;   
			    label_exists[0] = label_exists[1] = label_exists[2] = label_exists[3] = _FALSE_;
			    label_exists[4] = label_exists[5] = label_exists[6] = label_exists[7] = _TRUE_;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;

			    /*--- some 18-neighbors ---*/
			    _EXAMINATE_01
			    _EXAMINATE_03_A
			    _EXAMINATE_05_A
			    _EXAMINATE_07_A

			    _EXAMINATE_09_B
			    _EXAMINATE_11_B
			    _EXAMINATE_15_B
			    _EXAMINATE_17_A

			    /*--- we count the connected components ---*/
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;

			    _T26_COUNT_0
			    _T26_COUNT_1
			    _T26_COUNT_2
			    _T26_COUNT_3
			    if ( label_vertex[4] == 4 ) nb_t26 ++;
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			}
		    } else {
			/*  6-neighbors :  0 0 0 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 0 0 1 0 */

			    /*--- initialization ---*/
			    label_vertex[0] = 0;   label_vertex[1] = 1;   label_vertex[4] = 4;   label_vertex[5] = 5;   
			    label_vertex[2] = 2;   label_vertex[3] = 2;   label_vertex[6] = 2;   label_vertex[7] = 2;   
			    label_exists[0] = label_exists[1] = label_exists[4] = label_exists[5] = _FALSE_;
			    label_exists[2] = label_exists[3] = label_exists[6] = label_exists[7] = _TRUE_;
			    for ( i = 0; i < 6; i ++ ) {
				label_face[i] = i;
			    }

			    /*--- some 18-neighbors ---*/
			    _EXAMINATE_01
			    _EXAMINATE_09_C
			    _EXAMINATE_11_C
			    _EXAMINATE_19_B

			    _EXAMINATE_03_B
			    _EXAMINATE_05_B
			    _EXAMINATE_21_B
			    _EXAMINATE_23_B

			    /*--- we count the connected components ---*/
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    _T26_COUNT_0
			    _T26_COUNT_1
			    _T26_COUNT_4
			    _T26_COUNT_5
			    if ( label_vertex[2] == 2 ) nb_t26 ++;
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    /*  6-neighbors :  0 0 0 0 1 1 */
			    CASE_4b(neighbors, 3, 9, 1, 5, 11, 0, 2 );
			}
		    }
		} else {
		    /*  6-neighbors :  0 0 0 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 0 0 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 0 1 0 0 */

			    /*--- initialization ---*/
			    label_vertex[0] = 0;   label_vertex[2] = 2;   label_vertex[4] = 4;   label_vertex[6] = 6;   
			    label_vertex[1] = 1;   label_vertex[3] = 1;   label_vertex[5] = 1;   label_vertex[7] = 1;   
			    label_exists[0] = label_exists[2] = label_exists[4] = label_exists[6] = _FALSE_;
			    label_exists[1] = label_exists[3] = label_exists[5] = label_exists[7] = _TRUE_;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
 
			    /*--- some 18-neighbors ---*/
			    _EXAMINATE_03_A
			    _EXAMINATE_09_D
			    _EXAMINATE_15_C
			    _EXAMINATE_21_C

			    _EXAMINATE_01
			    _EXAMINATE_07_B
			    _EXAMINATE_19_C
			    _EXAMINATE_25_B

			    /*--- we count the connected components ---*/
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    _T26_COUNT_0
			    _T26_COUNT_2
			    _T26_COUNT_4
			    _T26_COUNT_6
			    if ( label_vertex[1] == 1 ) nb_t26 ++;
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    /*  6-neighbors :  0 0 0 1 0 1 */
			    CASE_4b(neighbors, 1, 9, 3, 7, 15, 0, 6 );
			}
		    } else {
			/*  6-neighbors :  0 0 0 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 0 1 1 0 */
			    CASE_4b(neighbors, 1, 3, 9, 19, 21, 0, 18 );
			} else {
			    /*  6-neighbors :  0 0 0 1 1 1 */
			    CASE_3a( neighbors, 1, 3, 9, 0 );
			}
		    }
		}
	    } else {
		/*  6-neighbors :  0 0 1 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  0 0 1 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 0 1 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 1 0 0 0 */

			    /*--- initialization ---*/
			    label_vertex[1] = 1;   label_vertex[3] = 3;   label_vertex[5] = 5;   label_vertex[7] = 7;   
			    label_vertex[0] = 0;   label_vertex[2] = 0;   label_vertex[4] = 0;   label_vertex[6] = 0;   
			    label_exists[1] = label_exists[3] = label_exists[5] = label_exists[7] = _FALSE_;
			    label_exists[0] = label_exists[2] = label_exists[4] = label_exists[6] = _TRUE_;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
   
			    /*--- some 18-neighbors ---*/
			    _EXAMINATE_05_A
			    _EXAMINATE_11_D
			    _EXAMINATE_17_B
			    _EXAMINATE_23_C

			    _EXAMINATE_01
			    _EXAMINATE_07_C
			    _EXAMINATE_19_D
			    _EXAMINATE_25_C

			    /*--- we count the connected components ---*/
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    nb_t26 = 1;
                            _T26_COUNT_1
			    _T26_COUNT_3
			    _T26_COUNT_5
			    _T26_COUNT_7
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    /*  6-neighbors :  0 0 1 0 0 1 */
			    CASE_4b(neighbors, 1, 11, 5, 7, 17, 2, 8 );
			}
		    } else {
			/*  6-neighbors :  0 0 1 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 1 0 1 0 */
			    CASE_4b(neighbors, 5, 1, 11, 23, 19, 2, 20 );
			} else {
			    /*  6-neighbors :  0 0 1 0 1 1 */
			    CASE_3a( neighbors, 1, 5, 11, 2 );
			}
		    }
		} else {
		    /*  6-neighbors :  0 0 1 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 0 1 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 1 1 0 0 */
			    CASE_4a( neighbors, 1, 19, 25, 7 );
			} else {
			    /*  6-neighbors :  0 0 1 1 0 1 */
			    CASE_3b( neighbors, 1, 7 );
			}
		    } else {
			/*  6-neighbors :  0 0 1 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 0 1 1 1 0 */
			    CASE_3b( neighbors, 1, 19 );
			} else {
			    /*  6-neighbors :  0 0 1 1 1 1 */
			    CASE_2b( neighbors, 1 );
			}
		    }
		}
	    }
	} else {
	    /*  6-neighbors :  0 1 . . . . */
	    if ( GET_VAL(neighbors,12) == 0 ) {
		/*  6-neighbors :  0 1 0 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  0 1 0 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 1 0 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 0 0 0 0 */

			    /*--- initialization ---*/
			    label_vertex[2] = 2;   label_vertex[3] = 3;   label_vertex[6] = 6;   label_vertex[7] = 7;   
			    label_vertex[0] = 0;   label_vertex[1] = 0;   label_vertex[4] = 0;   label_vertex[5] = 0;   
			    label_exists[2] = label_exists[3] = label_exists[6] = label_exists[7] = _FALSE_;
			    label_exists[0] = label_exists[1] = label_exists[4] = label_exists[5] = _TRUE_;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
 
			    /*--- some 18-neighbors ---*/
			    _EXAMINATE_07_D
			    _EXAMINATE_15_D
			    _EXAMINATE_17_C
			    _EXAMINATE_25_D

			    _EXAMINATE_03_A
			    _EXAMINATE_05_C
			    _EXAMINATE_21_D
			    _EXAMINATE_23_D

			    /*--- we count the connected components ---*/
			    if ( label_face[0] == 0 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    nb_t26 = 1;
			    _T26_COUNT_2
			    _T26_COUNT_3
			    _T26_COUNT_6
			    _T26_COUNT_7
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    /*  6-neighbors :  0 1 0 0 0 1 */
			    CASE_4b(neighbors, 5, 17, 7, 3, 15, 8, 6 );
			}
		    } else {
			/*  6-neighbors :  0 1 0 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 0 0 1 0 */
			    CASE_4a( neighbors, 3, 5, 23 ,21 );
			} else {
			    /*  6-neighbors :  0 1 0 0 1 1 */
			    CASE_3b( neighbors, 3, 5 );
			}
		    }
		} else {
		    /*  6-neighbors :  0 1 0 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 1 0 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 0 1 0 0 */
			    CASE_4b(neighbors, 3, 7, 15, 21, 25, 6, 24 );
			} else {
			    /*  6-neighbors :  0 1 0 1 0 1 */
			    CASE_3a( neighbors, 3, 7, 15, 6 );
			}
		    } else {
			/*  6-neighbors :  0 1 0 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 0 1 1 0 */
			    CASE_3b( neighbors, 3, 21 );
			} else {
			    /*  6-neighbors :  0 1 0 1 1 1 */
			    CASE_2b( neighbors, 3 );
			}
		    }
		}
	    } else {
		/*  6-neighbors :  0 1 1 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  0 1 1 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 1 1 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 1 0 0 0 */
			    CASE_4b(neighbors, 7, 5, 17, 25, 23, 8, 26 );
			} else {
			    /*  6-neighbors :  0 1 1 0 0 1 */
			    CASE_3a( neighbors, 5, 7, 17, 8 );
			}
		    } else {
			/*  6-neighbors :  0 1 1 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 1 0 1 0 */
			    CASE_3b( neighbors, 5, 23 );
			} else {
			    /*  6-neighbors :  0 1 1 0 1 1 */
			    CASE_2b( neighbors, 5 );
			}
		    }
		} else {
		    /*  6-neighbors :  0 1 1 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  0 1 1 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 1 1 0 0 */
			    CASE_3b( neighbors, 7, 25 );
			} else {
			    /*  6-neighbors :  0 1 1 1 0 1 */
			    CASE_2b( neighbors, 7 );
			}
		    } else {
			/*  6-neighbors :  0 1 1 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  0 1 1 1 1 0 */
			    CASE_2a;
			} else {
			    /*  6-neighbors :  0 1 1 1 1 1 */
			    CASE_1;
			}
		    }
		}
	    }
	}
    } else {
	/*  6-neighbors :  1 . . . . . */
	if ( GET_VAL(neighbors,10) == 0 ) {
	    /*  6-neighbors :  1 0 . . . . */
	    if ( GET_VAL(neighbors,12) == 0 ) {
		/*  6-neighbors :  1 0 0 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  1 0 0 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 0 0 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 0 0 0 0 */

			    /*--- initialization ---*/
			    label_vertex[4] = 4;   label_vertex[5] = 5;   label_vertex[6] = 6;   label_vertex[7] = 7;   
			    label_vertex[0] = 0;   label_vertex[1] = 0;   label_vertex[2] = 0;   label_vertex[3] = 0;   
			    label_exists[4] = label_exists[5] = label_exists[6] = label_exists[7] = _FALSE_;	
			    label_exists[0] = label_exists[1] = label_exists[2] = label_exists[3] = _TRUE_;
			    for ( i = 0; i < 6; i ++ ) label_face[i] = i;
			    
			    /*--- some 18-neighbors ---*/ 
    			    _EXAMINATE_19_A
			    _EXAMINATE_21_E
			    _EXAMINATE_23_E
			    _EXAMINATE_25_E

			    _EXAMINATE_09_C
			    _EXAMINATE_11_E
			    _EXAMINATE_15_E
			    _EXAMINATE_17_D

			    /*--- we count the connected components ---*/
			    if ( label_face[1] == 1 ) nb_t06 ++;
			    if ( label_face[2] == 2 ) nb_t06 ++;
			    if ( label_face[3] == 3 ) nb_t06 ++;
			    if ( label_face[4] == 4 ) nb_t06 ++;
			    if ( label_face[5] == 5 ) nb_t06 ++;
			    if ( nb_t06 > 1 ) goto END_FALSE;
			    
			    nb_t26 = 1;
			    _T26_COUNT_4
			    _T26_COUNT_5
			    _T26_COUNT_6
			    _T26_COUNT_7
			    if ( nb_t26 > 1 ) goto END_FALSE;
                            goto END_TRUE;

			} else {
			    /*  6-neighbors :  1 0 0 0 0 1 */
			    CASE_4a( neighbors, 9, 11, 15, 17 );
			}
		    } else {
			/*  6-neighbors :  1 0 0 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 0 0 1 0 */
			    CASE_4b(neighbors, 21, 9, 19, 23, 11, 18, 20 );
			} else {
			    /*  6-neighbors :  1 0 0 0 1 1 */
			    CASE_3b( neighbors, 9, 11 );
			}
		    }
		} else {
		    /*  6-neighbors :  1 0 0 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 0 0 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 0 1 0 0 */
			    CASE_4b(neighbors, 19, 9, 21, 25, 15, 18, 24 );
			} else {
			    /*  6-neighbors :  1 0 0 1 0 1 */
			    CASE_3b( neighbors, 9, 15 );
			}
		    } else {
			/*  6-neighbors :  1 0 0 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 0 1 1 0 */
			    CASE_3a( neighbors, 9, 19, 21, 18 );
			} else {
			    /*  6-neighbors :  1 0 0 1 1 1 */
			    CASE_2b( neighbors, 9 );
			}
		    }
		}
	    } else {
		/*  6-neighbors :  1 0 1 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  1 0 1 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 0 1 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 1 0 0 0 */
			    CASE_4b(neighbors, 19, 11, 23, 25, 17, 20, 26 );
			} else {
			    /*  6-neighbors :  1 0 1 0 0 1 */
			    CASE_3b( neighbors, 11, 17 );
			}
		    } else {
			/*  6-neighbors :  1 0 1 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 1 0 1 0 */
			    CASE_3a( neighbors, 11, 19, 23, 20 );
			} else {
			    /*  6-neighbors :  1 0 1 0 1 1 */
			    CASE_2b( neighbors, 11 );
			}
		    }
		} else {
		    /*  6-neighbors :  1 0 1 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 0 1 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 1 1 0 0 */
			    CASE_3b( neighbors, 19, 25 );
			} else {
			    /*  6-neighbors :  1 0 1 1 0 1 */
			    CASE_2a;
			}
		    } else {
			/*  6-neighbors :  1 0 1 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 0 1 1 1 0 */
			    CASE_2b( neighbors, 19 );
			} else {
			    /*  6-neighbors :  1 0 1 1 1 1 */
			    CASE_1;
			}
		    }
		}
	    }
	} else {
	    /*  6-neighbors :  1 1 . . . . */
	    if ( GET_VAL(neighbors,12) == 0 ) {
		/*  6-neighbors :  1 1 0 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  1 1 0 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 1 0 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 0 0 0 0 */
			    CASE_4b(neighbors, 23, 17, 25, 21, 15, 26, 24 );
			} else {
			    /*  6-neighbors :  1 1 0 0 0 1 */
			    CASE_3b( neighbors, 15, 17 );
			}
		    } else {
			/*  6-neighbors :  1 1 0 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 0 0 1 0 */
			    CASE_3b( neighbors, 21, 23 );
			} else {
			    /*  6-neighbors :  1 1 0 0 1 1 */
			    CASE_2a;
			}
		    }
		} else {
		    /*  6-neighbors :  1 1 0 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 1 0 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 0 1 0 0 */
			    CASE_3a( neighbors, 15, 21, 25, 24 );
			} else {
			    /*  6-neighbors :  1 1 0 1 0 1 */
			    CASE_2b( neighbors, 15 );
			}
		    } else {
			/*  6-neighbors :  1 1 0 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 0 1 1 0 */
			    CASE_2b( neighbors, 21 );
			} else {
			    /*  6-neighbors :  1 1 0 1 1 1 */
			    CASE_1;
			}
		    }
		}
	    } else {
		/*  6-neighbors :  1 1 1 . . . */
		if ( GET_VAL(neighbors,14) == 0 ) {
		    /*  6-neighbors :  1 1 1 0 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 1 1 0 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 1 0 0 0 */
			    CASE_3a( neighbors, 17, 23, 25, 26 );
			} else {
			    /*  6-neighbors :  1 1 1 0 0 1 */
			    CASE_2b( neighbors, 17 );
			}
		    } else {
			/*  6-neighbors :  1 1 1 0 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 1 0 1 0 */
			    CASE_2b( neighbors, 23 );
			} else {
			    /*  6-neighbors :  1 1 1 0 1 1 */
			    CASE_1;
			}
		    }
		} else {
		    /*  6-neighbors :  1 1 1 1 . . */
		    if ( GET_VAL(neighbors,16) == 0 ) {
			/*  6-neighbors :  1 1 1 1 0 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 1 1 0 0 */
			    CASE_2b( neighbors, 25 );
			} else {
			    /*  6-neighbors :  1 1 1 1 0 1 */
			    CASE_1;
			}
		    } else {
			/*  6-neighbors :  1 1 1 1 1 . */
			if ( GET_VAL(neighbors,22) == 0 ) {
			    /*  6-neighbors :  1 1 1 1 1 0 */
			    CASE_1;
			} else {
			    /*  6-neighbors : 1 1 1 1 1 1 */
			    CASE_0;
			}
		    }
		}
	    }
	}
    }

   if ( (nb_t06 == 1) && (nb_t26 == 1) ) goto END_TRUE;
   goto END_FALSE;

END_FALSE :
   return( _FALSE_ );
END_TRUE :
   return( _TRUE_ );
}
