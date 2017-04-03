/*************************************************************************
 * bal-matrix.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#ifndef BAL_MATRIX_H
#define BAL_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif





#include <bal-behavior.h>

/* On definit une matrice comme un tableau de taille ligne x colonne */
typedef struct {
  int l,c;
  double *m;
} _MATRIX;


extern int BAL_GetVerboseInBalMatrix( );
extern void BAL_SetVerboseInBalMatrix( int v );
extern void BAL_IncrementVerboseInBalMatrix(  );
extern void BAL_DecrementVerboseInBalMatrix(  );


/*--------------------------------------------------
 *
 * some procedures on transformation matrices
 *
 --------------------------------------------------*/


void _init_mat( _MATRIX *mat );
/* liberation matrix */
void _free_mat (_MATRIX *m);
int _alloc_mat (_MATRIX *m, int l, int c );

/* multiply two matrices:    m1 * m2 = res    */
void _mult_mat ( _MATRIX *m1, _MATRIX *m2, _MATRIX *res );
/* substract two matrices:    m1 - m2  = res   */
void _sub_mat( _MATRIX *m1, _MATRIX *m2, _MATRIX *res );

/* copy matrix */
void _copy_mat( _MATRIX *m, _MATRIX *res );

void _identity_mat( _MATRIX *mat );

/* matrix I/O
 */
extern void _print_mat ( FILE *f, _MATRIX *m, char *desc );
extern int _read_mat( char *name, _MATRIX *m );
extern int _write_mat( char *name, _MATRIX *m );






/* substract two matrix */
_MATRIX sub_mat(_MATRIX m1, _MATRIX m2);

/* transpose matrix */
_MATRIX transpose(_MATRIX m);

/* det matrix */
double det(_MATRIX mat);

/* inverse matrix */
_MATRIX inv_mat(_MATRIX m);




void Compose_Tmatrix ( _MATRIX T1, _MATRIX T2, _MATRIX T );


void Norms_Tmatrix ( _MATRIX T, double * dr, double * dt );

void Norms_SimilTmatrix ( _MATRIX T, double * dr, double * dt, double echelle );

void Inverse_RigidTmatrix ( _MATRIX T, _MATRIX Tinv);

void Inverse_SimilTmatrix ( _MATRIX T, _MATRIX Tinv, double echelle);


int InverseMat4x4( double *matrice, double *inv );
int InverseMat3x3( double *matrice, double *inv );

void E_DMMatPrint( FILE *fi, double *a, int dim, char *str );

void E_DMMatTrans( double *a, double *aT, int dim );

void E_DMMatMul( double *a, double *b, double *ab, int dim );

void E_DMMatMulVect( double *a, double *b, double *c, int dim );

int E_DMVVpropresMatSym( double *m, double *val, double *vec, int dim );

void E_MatRotFromQuat( double qr[4], double rot[9] );
void E_MatRotFromRVec( double rv[3], double rot[9] );


#ifdef __cplusplus
}
#endif

#endif
