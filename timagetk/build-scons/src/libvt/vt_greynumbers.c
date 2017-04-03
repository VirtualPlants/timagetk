#include <vt_t06t26.h>
#include <vt_t04t08.h>
#include <vt_greynumbers.h>





void Compute_allTs_2D( int neighbors[9],
		       int *tp,
		       int *tpp,
		       int *tm,
		       int *tmm )
{
  int myneighbors[9];
  int val=neighbors[4];
  int i;
  
  for ( i = 0; i<9; i++ ) {
    if ( neighbors[i] > val ) myneighbors[i] = 1;
    else myneighbors[i] = 0;
  }
  Compute_T04_and_T08( myneighbors, tm, tpp );

  for ( i = 0; i<9; i++ ) {
    if ( neighbors[i] >= val ) myneighbors[i] = 1;
    else myneighbors[i] = 0;
  }
  Compute_T04_and_T08( myneighbors, tmm, tp );

}





void Compute_allTs_3D( int neighbors[27],
		       int *tp,
		       int *tpp,
		       int *tm,
		       int *tmm )
{
  int myneighbors[27];
  int val=neighbors[13];
  int i;
  
  for ( i = 0; i<27; i++ ) {
    if ( neighbors[i] > val ) myneighbors[i] = 1;
    else myneighbors[i] = 0;
  }
  Compute_T06_and_T26( myneighbors, tm, tpp );

  for ( i = 0; i<27; i++ ) {
    if ( neighbors[i] >= val ) myneighbors[i] = 1;
    else myneighbors[i] = 0;
  }
  Compute_T06_and_T26( myneighbors, tmm, tp );

}
