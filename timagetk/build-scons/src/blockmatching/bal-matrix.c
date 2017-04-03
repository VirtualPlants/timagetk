/*************************************************************************
 * bal-matrix.c -
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



#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>




#include <bal-matrix.h>

#define EPI_MAX(A,B) ( (A) > (B) ? (A) : (B) )
/* Boolean Values */
#define TRUE 1
#define FALSE 0


static int _verbose_ = 1;

int BAL_GetVerboseInBalMatrix( )
{
  return( _verbose_ );
}

void BAL_SetVerboseInBalMatrix( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalMatrix(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalMatrix(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}
















/*--------------------------------------------------
 *
 * MATRICES MANAGEMENT
 *
 --------------------------------------------------*/





void _init_mat( _MATRIX *mat )
{
  mat->m = (double*)NULL;
  mat->l = 0;
  mat->c = 0;
}


/* allocation matrix */
int _alloc_mat ( _MATRIX *mat, int l, int c )
{  
  
  mat->l = 0;
  mat->c = 0;

  mat->m =  (double *) calloc( (l*c), sizeof(double) );
  if ( mat->m == (double*)NULL ) return( -1 );

  mat->l = l;
  mat->c = c;
			  
  return ( 1 );
}

/* liberation matrix */
void _free_mat ( _MATRIX *mat )
{
  if ( mat->m != (double*)NULL )
    free ( mat->m );

  mat->m = NULL;
  mat->l = 0;
  mat->c = 0;
}



/* multiply two matrices:    m1 * m2 = res    */
void _mult_mat ( _MATRIX *m1, _MATRIX *m2, _MATRIX *res )
{
  int i,j,k;
  _MATRIX tmp;

  if ( res->l != m1->l 
       || res->c != m2->c 
       || m2->l != m1->c ) {
    if ( _verbose_ )
      fprintf( stderr, "_mult_mat: bad matrix dimensions\n" );
    return;
  }
  
  tmp.l =  m1->l;
  tmp.c =  m2->c;
  
  if ( m1 == res || m2 == res ) {
    if ( _alloc_mat ( &tmp, m1->l, m2->c ) != 1 ) {
      if ( _verbose_ )
	fprintf( stderr, 
		 "_mult_mat: unable to allocate temporary matrix\n" );
      return;
    }
  }
  else 
    tmp.m = res->m;
 
  for ( i=0; i<res->l; i++ )
  for ( j=0; j<res->c; j++ ) {
    tmp.m[j+i*res->c] = 0.0;
    for ( k=0; k<m1->c; k++ )
      tmp.m[j+i*res->c] += m1->m[k+i*m1->c] * m2->m[j+k*m2->c];
  }

  if ( m1 == res || m2 == res ) {
      for ( i=0; i<res->l; i++ )
      for ( j=0; j<res->c; j++ )
	res->m[j+i*res->c] = tmp.m[j+i*res->c];
      _free_mat( &tmp );
  }

}



/* substract two matrices:    m1 - m2  = res   */
void _sub_mat( _MATRIX *m1, _MATRIX *m2, _MATRIX *res )
{
  int i;

  if ( m1->l != m2->l || m1->c != m2->c 
       || m1->l != res->l || m1->c != res->c
       || m1->m == NULL || m2->m == NULL || res->m == NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "_sub_mat: bad matrix dimensions\n" );
  }

  for ( i = 0; i < m1->l*m1->c; i++ )
    res->m[i] = m1->m[i] - m2->m[i];
}

/* copy matrix */
void _copy_mat( _MATRIX *m, _MATRIX *res )
{
  int i;

  if ( m->l != res->l || m->c != res->c ) {
    if ( _verbose_ )
      fprintf( stderr, "_copy_mat: bad parameters\n" );
    return;
  }

  for ( i=0; i<m->l*m->c; i++ )
    res->m[i] = m->m[i]; 
  
} 



void _identity_mat( _MATRIX *mat )
{
  int i,j;

  for ( i=0; i<mat->l; i++ )
    for ( j=0; j<mat->c; j++ )
      mat->m[j+i*mat->c] = 0.0;
  
  for ( i=0; i<mat->c && i<mat->l; i++ ) 
    mat->m[i+i*mat->c] = 1.0;

}


/************************************************************
 *
 * Matrix I/O
 *
 ************************************************************/

/* affiche la matrice */
void _print_mat ( FILE *f, _MATRIX *m, char *desc )
{
  int i,j;

  if (desc != NULL) fprintf( f, "%s\n", desc ); 
  /* else fprintf( f, "\n"); */
  for(i=0;i<m->l;i++){
    for(j=0;j<m->c;j++) {
      fprintf(f,"%20.15f ",m->m[j+i*m->c]);
    /* fprintf(f,"%f\t",m->m[j+i*m->c]); */
    }
    fprintf(f, "\n");
  }
}



int _read_mat( char *name, _MATRIX *m )
{
  int r;
  FILE *f;
  char line[512];
  double f1, f2, f3, f4;
  
  if ( m->l != 4 || m->c != 4 || m->m == NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "_read_mat: matrix is not 4x4 or not allocated\n" );
    return( -1 );
  }

  if ((f = fopen (name, "r")) == NULL) {
    if ( _verbose_ )
      fprintf( stderr, "_read_mat: unable to open '%s' for reading\n", name );
    return( -1 );
  }
  
  /* get the first 4 values, ie the first row
   */

  if ( fgets(line, 512, f) == NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "_read_mat: an error occurs when reading '%s'\n", name );
    return( -1 );
  }
  
  while ( ( sscanf( line, "%lf %lf %lf %lf\n", &f1, &f2, &f3, &f4 ) < 4 ) && 
	  ( sscanf( line, "O8 %lf %lf %lf %lf\n", &f1, &f2, &f3, &f4 ) < 4 ) ) {
    if ( fgets(line, 512, f) == NULL ) {
      if ( _verbose_ )
	fprintf( stderr, "_read_mat: an error occurs when reading '%s'\n", name );
      return( -1 );
    }
  }

  m->m[0] = f1; m->m[1] = f2; m->m[2] = f3; m->m[3] = f4; 

  r = 1;
  
  /* get the 3 other rows
   */

  for ( r=1; r<4; r ++ ) {
    if ( fgets(line, 512, f) == NULL ) {
      if ( _verbose_ )
	fprintf( stderr, "_read_mat: an error occurs when reading '%s'\n", name );
      return( -1 );
    }
    if ( sscanf( line, "%lf %lf %lf %lf\n", &f1, &f2, &f3, &f4 ) != 4 ) {
      if ( _verbose_ )
	fprintf( stderr, "_read_mat: line for row #%d does not contain 4 values in '%s'\n", r+1, name );
      return( -1 );
    }
    m->m[r*4+0] = f1;
    m->m[r*4+1] = f2;
    m->m[r*4+2] = f3;
    m->m[r*4+3] = f4;
  }

  fclose( f );
  return( 1 );
}



int _write_mat( char *name, _MATRIX *m )
{
  int i;
  FILE *f;
  /* char *format = "%f %f %f %f\n";
  */
  char *format = "%12.8f %12.8f %12.8f %12.8f\n";
  
  if ( (strncmp( name, "stderr", 6 ) == 0 && name[6] == '\0') ) {
    for (i = 0; i < 4; i++) {
      fprintf( stderr, format,
	       m->m[i*4], m->m[1+i*4], m->m[2+i*4], m->m[3+i*4] );
    }
  }
  else if ( name == NULL || name[0] == '\0'
            || (name[0] == '>' && name[6] == '\0')
            || (strncmp( name, "stdout", 6 ) == 0 && name[6] == '\0') ) {
    for (i = 0; i < 4; i++) {
      fprintf( stdout, format,
	       m->m[i*4], m->m[1+i*4], m->m[2+i*4], m->m[3+i*4] );
    }
  }
  else {
    f = fopen( name, "w" );
    if ( f == NULL ) {
      if ( _verbose_ )
	fprintf( stderr, "_write_mat: unable to open '%s' for writing\n", name );
      return( -1 );
    }
    fprintf( f, "(\n" );
    fprintf( f, "O8\n" );
    for (i = 0; i < 4; i++) {
      fprintf( f, format,
	       m->m[i*4], m->m[1+i*4], m->m[2+i*4], m->m[3+i*4] );
    }
    fprintf( f, ")\n" );
    fclose( f );
  }
  return( 1 );
}













/* transpose a matrix */
_MATRIX transpose(_MATRIX m)
{
 int i,j,c;
 _MATRIX res;


 res.l=m.c;
 res.c=m.l;
 c=m.c;

 res.m = (double*) malloc((res.l * res.c)*sizeof(double));
  
 for(i=0;i<m.l;i++)
   for(j=0;j<c;j++)
     res.m[i+j*m.l]=m.m[j+i*c]; 
 
 return res;
}

/* determinant of a matrix */
double det(_MATRIX m)
{
  double res;
  int i,j;
  _MATRIX tmp;
  tmp.l = m.l - 1;
  tmp.c = m.c - 1;

  tmp.m = (double*) malloc((tmp.l * tmp.c)*sizeof(double));

  if(m.l == 0  || m.c == 0 ) 
    return 0;
  else if (m.l==1) 
    return m.m[0];
  else if(m.l==2) 
    return m.m[0]*m.m[3]-m.m[1]*m.m[2];
  else{
    for(i=0; i<m.l - 1; i++)
      for(j=0; j< m.c - 1; j++)
	tmp.m[i*tmp.c+j]=m.m[(i+1)*m.c+j+1];
    
    res=0;
    for(j=0;j<m.c - 1;j++){
      res+=det(tmp)*m.m[j]*(1-(j%2)*2);          
      for(i=1;i<m.l;i++) tmp.m[(i-1)*tmp.c+j]=m.m[j+i*m.c]; 
    }
    res+=det(tmp)*m.m[j]*(1-(j%2)*2);      
    free(tmp.m);
    return res;
  }
}

/* inverse a matrix */
_MATRIX inv_mat(_MATRIX mat)
{
  _MATRIX res,tmp;
  int i,j,k,ii,jj;
  double d;
  
  res.l=mat.l;
  res.c=mat.c;
  tmp.l=mat.l-1;
  tmp.c=mat.c-1;
  
  res.m = (double*) malloc((res.l * res.c)*sizeof(double));
  tmp.m = (double*) malloc((tmp.l * tmp.c)*sizeof(double));
  
  d=det(mat);
  
  if(d != 0){
    for(i=0;i<mat.l;i++){
      k=0;
      for(ii=0;ii<mat.l;ii++){
	if(ii!=i){
	  for(jj=0;jj<mat.c-1;jj++){
	    tmp.m[k*tmp.c+jj]=mat.m[ii*mat.c+jj+1];
	  }
	  k++;
	}
      }
      for(j=0;j<mat.c-1;j++){
	res.m[i*mat.c+j]=(det(tmp)/d)*(1-((i+j)%2)*2);
	k=0;
	for(ii=0;ii<mat.l;ii++){
	  if(ii!=i){
	    tmp.m[k*tmp.c+j]=mat.m[j+ii*mat.c]; 
	    k++;
	  }
	}
      }
      res.m[i*mat.c+j]=(det(tmp)/d)*(1-((i+j)%2)*2);
    }
  }
  free (tmp.m);
  return ( transpose(res) );
}




void Compose_Tmatrix ( _MATRIX T1, _MATRIX T2, _MATRIX T )
{

  T.m[0]  = T1.m[0]*T2.m[0] + T1.m[1]*T2.m[4] + T1.m[2]*T2.m[8];
  T.m[1]  = T1.m[0]*T2.m[1] + T1.m[1]*T2.m[5] + T1.m[2]*T2.m[9];
  T.m[2]  = T1.m[0]*T2.m[2] + T1.m[1]*T2.m[6] + T1.m[2]*T2.m[10];
  
  T.m[4]  = T1.m[4]*T2.m[0] + T1.m[5]*T2.m[4] + T1.m[6]*T2.m[8];
  T.m[5]  = T1.m[4]*T2.m[1] + T1.m[5]*T2.m[5] + T1.m[6]*T2.m[9];
  T.m[6]  = T1.m[4]*T2.m[2] + T1.m[5]*T2.m[6] + T1.m[6]*T2.m[10];
  
  T.m[8]  = T1.m[8]*T2.m[0] + T1.m[9]*T2.m[4] + T1.m[10]*T2.m[8];
  T.m[9]  = T1.m[8]*T2.m[1] + T1.m[9]*T2.m[5] + T1.m[10]*T2.m[9];
  T.m[10] = T1.m[8]*T2.m[2] + T1.m[9]*T2.m[6] + T1.m[10]*T2.m[10];
  
  T.m[3]  = T1.m[0]*T2.m[3] + T1.m[1]*T2.m[7] + T1.m[2]*T2.m[11] + T1.m[3];
  T.m[7]  = T1.m[4]*T2.m[3] + T1.m[5]*T2.m[7] + T1.m[6]*T2.m[11] + T1.m[7];
  T.m[11] = T1.m[8]*T2.m[3] + T1.m[9]*T2.m[7] + T1.m[10]*T2.m[11] + T1.m[11];
  
  T.m[12] = 0;
  T.m[13] = 0;
  T.m[14] = 0;
  T.m[15] = 1;
  
  return;
}





void Norms_Tmatrix (_MATRIX T, double * dr, double * dt)
{
  double aux;

  /* --- dr est defini comme dist(I, R) au sens des matrices. Il
     ne s'agit pas de la distance intrinseque sur SO3, i.e. la norme
     du vecteur rotation. Mais c'en est une approx du 2e ordre. 
     --- */
  aux = T.m[0] + T.m[5] + T.m[10];
  * dr = sqrt ( EPI_MAX (0, 6 - 2 * aux) );
  
  /* dt est la norme euclidienne du vecteur translation */
  * dt =  sqrt ( EPI_MAX (0,  T.m[3]*T.m[3] + T.m[7]*T.m[7] + T.m[11]*T.m[11] ) );
  
  return;
}


void Norms_SimilTmatrix (_MATRIX T, double * dr, double * dt, double echelle)
{
  double aux;

  /* --- dr est defini comme dist(I, R) au sens des matrices. Il
     ne s'agit pas de la distance intrinseque sur SO3, i.e. la norme
     du vecteur rotation. Mais c'en est une approx du 2e ordre. 
     --- */
  aux = ( T.m[0] + T.m[5] + T.m[10] )/echelle;
  * dr = sqrt ( EPI_MAX (0, 6 - 2 * aux) );
  
  /* dt est la norme euclidienne du vecteur translation */
  * dt =  sqrt ( EPI_MAX (0,  T.m[3]*T.m[3] + T.m[7]*T.m[7] + T.m[11]*T.m[11] ) );
  
  return;
}



/* ----------------------------------------------------------------------

           Inversion d'une matrice de transformation rigide 4*4

	   On utilise la propriete :

	             R^t  | -R^t*u
	     T^-1 = - - - - - - - -
                    0 0 0 |   1


   ---------------------------------------------------------------------- */

void Inverse_RigidTmatrix (_MATRIX T, _MATRIX Tinv)
{
  /* --- Calcul de la rotation inverse = transposee de R --- */
  Tinv.m[0] = T.m[0];  Tinv.m[1] = T.m[4];  Tinv.m[2] = T.m[8];
  Tinv.m[4] = T.m[1];  Tinv.m[5] = T.m[5];  Tinv.m[6] = T.m[9];
  Tinv.m[8] = T.m[2];  Tinv.m[9] = T.m[6];  Tinv.m[10] = T.m[10];
    
  /* --- Calcul de la translation inverse = -R^t*u --- */
  Tinv.m[3]  = - ( T.m[0]*T.m[3] + T.m[4]*T.m[7] + T.m[8]*T.m[11] );
  Tinv.m[7]  = - ( T.m[1]*T.m[3] + T.m[5]*T.m[7] + T.m[9]*T.m[11] );
  Tinv.m[11] = - ( T.m[2]*T.m[3] + T.m[6]*T.m[7] + T.m[10]*T.m[11] );
    
  /* --- Complements... --- */
  Tinv.m[12] = 0;  Tinv.m[13] = 0;  Tinv.m[14] = 0;  Tinv.m[15] = 1;
  
  return;
}



/* ----------------------------------------------------------------------

           Inversion d'une matrice de similitude 4*4

	   On utilise la propriete :

	             1/s R^t  | -1/s R^t*u
	     T^-1 = - - - - - - - - - - - -
                      0 0 0   |   1


------------------------------------------------------------------------ */

void Inverse_SimilTmatrix (_MATRIX T, _MATRIX Tinv, double echelle)
{
  /* --- Calcul de la rotation inverse = 1/s transposee de R --- */
  Tinv.m[0] = T.m[0]/echelle;  Tinv.m[1] = T.m[4]/echelle;  Tinv.m[2] = T.m[8]/echelle;
  Tinv.m[4] = T.m[1]/echelle;  Tinv.m[5] = T.m[5]/echelle;  Tinv.m[6] = T.m[9]/echelle;
  Tinv.m[8] = T.m[2]/echelle;  Tinv.m[9] = T.m[6]/echelle;  Tinv.m[10] = T.m[10]/echelle;

  /* --- Calcul de la translation inverse = -1/s R^t*u --- */
  Tinv.m[3]  = - ( T.m[0]*T.m[3] + T.m[4]*T.m[7] + T.m[8]*T.m[11] )/echelle;
  Tinv.m[7]  = - ( T.m[1]*T.m[3] + T.m[5]*T.m[7] + T.m[9]*T.m[11] )/echelle;
  Tinv.m[11] = - ( T.m[2]*T.m[3] + T.m[6]*T.m[7] + T.m[10]*T.m[11] )/echelle;
  
  /* --- Complements... --- */
  Tinv.m[12] = 0;  Tinv.m[13] = 0;  Tinv.m[14] = 0;  Tinv.m[15] = 1;
  
  return;
}








/*--------------------------------------------------------------

           Code local

---------------------------------------------------------------*/





#define TINY 1e-12
int InverseMat4x4( double *matrice, double *inv )
{
  int i, j, k;
  int kmax, rang = 4;
  double c, max;
  double mat [16];
  
  for (i=0; i<16; i++ ) {
    mat[i] = matrice[i] ;
    inv[i] = 0.0;
  }
  inv[0] = inv[5] = inv[10] = inv[15] = 1.0;
  
  for ( j=0; j<4; j++ ) {
    if ( (mat[j*4+j] > (-TINY)) && (mat[j*4+j] < TINY) ) {
      /* recherche du plus grand element non nul sur la colonne j */
      kmax = j;
      max = 0.0;
      for (k=j+1; k<4; k++ ) {
	c = ( mat[k*4+j] > 0.0 ) ? mat[k*4+j] : (-mat[k*4+j]) ;
	if ( (c > TINY) && (c > max) ) { max = c; kmax = k; }
      }
      if ( kmax == j ) {
	/* la ligne est nulle */
	rang --;
      } else {
	/* sinon, on additionne */
	for ( i=0; i<4; i++ ) {
	  mat[j*4+i] += mat[kmax*4+i];
	  inv[j*4+i] += inv[kmax*4+i];
	}
      }
    }
    if ( (mat[j*4+j] < (-TINY)) || (mat[j*4+j] > TINY) ) {
      /* les autres lignes */
      for (k=0; k<4; k++) {
	if ( k != j ) {
	  c = mat[k*4 + j] / mat[j*4 + j];
	  for ( i=0; i<4; i++ ) {
	    mat[k*4 + i] -= c * mat[j*4 + i];
	    inv[k*4 + i] -= c * inv[j*4 + i];
	  }
	}
      }
      /* la ligne */
      c = mat[j*4 + j];
      for ( i=0; i<4; i++ ) {
	mat[j*4 + i] /= c;
	inv[j*4 + i] /= c;
      }
    }
  }

  return( rang );
}



int InverseMat3x3( double *matrice, double *inv )
{
  int i, j, k;
  int kmax, rang = 3;
  double c, max;
  double mat [9];
  
  for (i=0; i<9; i++ ) {
    mat[i] = matrice[i] ;
    inv[i] = 0.0;
  }
  inv[0] = inv[4] = inv[8] = 1.0;
  
  for ( j=0; j<3; j++ ) {
    if ( (mat[j*3+j] > (-TINY)) && (mat[j*3+j] < TINY) ) {
      /* recherche du plus grand element non nul sur la colonne j */
      kmax = j;
      max = 0.0;
      for (k=j+1; k<3; k++ ) {
	c = ( mat[k*3+j] > 0.0 ) ? mat[k*3+j] : (-mat[k*3+j]) ;
	if ( (c > TINY) && (c > max) ) { max = c; kmax = k; }
      }
      if ( kmax == j ) {
	/* la ligne est nulle */
	rang --;
      } else {
	/* sinon, on additionne */
	for ( i=0; i<3; i++ ) {
	  mat[j*3+i] += mat[kmax*3+i];
	  inv[j*3+i] += inv[kmax*3+i];
	}
      }
    }
    if ( (mat[j*3+j] < (-TINY)) || (mat[j*3+j] > TINY) ) {
      /* les autres lignes */
      for (k=0; k<3; k++) {
	if ( k != j ) {
	  c = mat[k*3 + j] / mat[j*3 + j];
	  for ( i=0; i<3; i++ ) {
	    mat[k*3 + i] -= c * mat[j*3 + i];
	    inv[k*3 + i] -= c * inv[j*3 + i];
	  }
	}
      }
      /* la ligne */
      c = mat[j*3 + j];
      for ( i=0; i<3; i++ ) {
	mat[j*3 + i] /= c;
	inv[j*3 + i] /= c;
      }
    }
  }

  return( rang );
}



/*--------------------------------------------------------------

           Code extrait de epi_math2 et epi_quater (Gregoire)

---------------------------------------------------------------*/

#define DBLZERO 1e-12
#define EPI_MSIGN(a,b) ((b)<0 ? -fabs(a) : fabs(a))


void E_DMMatPrint( FILE *fi, double *a, int dim, char *str ) 
{
  FILE *f = stderr;
  int i, j;
  
  if ( fi != (FILE*)NULL ) f = fi;
    
  if ( str != (char*)NULL ) {
      fprintf( f, "%s\n", str );
  }

  for ( j=0; j<dim; j++ ) {
    for ( i=0; i<dim; i++ ) {
      fprintf( f, " %f", a[j*dim+i] );
    }
    fprintf( f, "\n" );
  }

}



/* Transposition d'une matrice carree.

   Les matrices sont codees en double. La
   matrice resultat doit etre differente de
   la matrice a transposer.
*/
void E_DMMatTrans( double *a /* (square) matrix to be transposed */, 
		  double *aT /* result matrix */, 
		  int dim    /* dimension of the square matrix */ )
{
  int i,j;

  for (i=0;i<dim;i++)
    for (j=0;j<dim;j++)
      aT[i*dim+j] = a[j*dim+i];
}


/* Multiplication de deux matrices carrees.

   Les matrices sont codees en doubles.
   La matrice resultat doit etre differente des
   deux matrices a multiplier.
*/
void E_DMMatMul( double *a /* first (square) matrix to be multiplied */, 
		double *b  /* second (square) matrix to be multiplied */, 
		double *ab /* result matrix = a * b */, 
		int dim    /* dimension of the matrix */ )
{
  int i,j,k,l;

  for (i=0;i<dim;i++)
    for (j=0;j<dim;j++) {
      l = i*dim+j;
      ab[l] = 0.0;
      for (k=0;k<dim;k++)
	ab[l] += a[i*dim+k]*b[k*dim+j];
    }
}


/* Multiplication d'une matrice carree par un vecteur.

   La matrice et les vecteurs sont codes en double.
   Il ne faut que le vecteur resultat soit le meme
   que le vecteur a multiplier, sinon le resultat sera
   nul (et surement faux).
*/
void E_DMMatMulVect( double *a /* (square) matrix */, 
		   double *b   /* vector to be multiplied to the matrix */, 
		   double *c   /* result vector */, 
		   int dim     /* dimension of the vectors and the matrix */ )
{
  int i,k;

  for (i=0;i<dim;i++) {
    c[i] = 0.0;
    for (k=0;k<dim;k++)
      c[i] += a[i*dim+k]*b[k];
  }
}

static void E_DMSwapI( int *pts, int l, int r )
{
  /*--------------------------------------------------
    procedures de tri
    --------------------------------------------------*/
  int t;
  t = pts[l];
  pts[l] = pts[r];
  pts[r] = t;
}

static void E_DMSwapD( double *pts, int l, int r )
{
  /*--------------------------------------------------
    procedures de tri
    --------------------------------------------------*/
  double t;
  t = pts[l];
  pts[l] = pts[r];
  pts[r] = t;
}

static void E_DMTriPointsDbl( double *nbPts, int *perm, int left, int right )
{
  /****************************************************************/
  /*   procedure de tri qui est une reecriture de qsort           */
  /*   avec un terme conservant la permutation realisant le tri   */
  /*   le resultat du tri est dans l'ordre croissant              */
  /****************************************************************/
  int i,last;
	
  if (left>=right)                             /* do nothing if array contains */
    return;                                    /* fewer than two elements      */
  E_DMSwapD(nbPts,left,(left+right)/2);        /* move partition elem to v[0]  */
  E_DMSwapI(perm,left,(left+right)/2);
  last = left;
  for (i=left+1;i<=right;i++)                  /* partition                    */
    if ( nbPts[i] < nbPts[left] ) {
      E_DMSwapD(nbPts,++last,i);
      E_DMSwapI(perm,last,i);
    }
  E_DMSwapD(nbPts,left,last);                  /* restore partition elem       */
  E_DMSwapI(perm,left,last);
  E_DMTriPointsDbl(nbPts,perm,left,last-1);
  E_DMTriPointsDbl(nbPts,perm,last+1,right);
  return;
}

static void E_DMEchCKandCL( double *m, int k, int l, int dim )
{
  /*--------------------------------------------------
    matrice double
    echange la colonne k avec la colonne l
    --------------------------------------------------*/
  int i;
  double t;

  for (i=0; i<dim; i++) {
    t = *(m + k + i*dim);
    *(m + k + i*dim) = *(m + l + i*dim);
    *(m + l + i*dim) = t;
  }
}

static int E_DMVVpropMatTriDiag( double *td, double *tr, int dim )
{
  int i, l, k, iter, m;
  double b, c, dd, f, g, p, r, s;
  
  for (l=0; l<dim; l++) {
    iter = 0;
    do {
      for (m=l; m<dim-1; m++) {
	dd = fabs( td[dim+m] ) + fabs( td[dim+m+1] );
	if ((double)(fabs(td[m]) + dd) == dd)
	  break;
      }
      if (m != l) {
	if (iter++ == 30) {
	  fprintf(stderr, "E_DMVVpropMatTriDiag : Too many iterations\n");
	  exit( -1 );
	}
	g = (td[dim+l+1] - td[dim+l]) / (2.0*td[l]);
	r = sqrt((g*g)+1.0);
	g = td[dim+m] - td[dim+l] + td[l]/(g+EPI_MSIGN(r,g));
	s = c =1.0;
	p = 0.0;
	for (i=m-1; i>=l; i--) {
	  f = s*td[i];
	  b = c*td[i];
	  if (fabs(f) >= fabs(g)) {
	    c = g/f;
	    r = sqrt((c*c)+1.0);
	    td[i+1] = f*r;
	    c *= (s=1.0/r);
	  }
	  else {
	    s = f/g;
	    r = sqrt((s*s)+1.0);
	    td[i+1] = g*r;
	    s *= (c=1.0/r);
	  }
	  g = td[dim+i+1] - p;
	  r = (td[dim+i]-g)*s + 2.0*c*b;
	  p = s*r;
	  td[dim+i+1] =g+p;
	  g = c*r-b;
	  /* Next loop can be omitted if eigenvectors not wanted */
	  for (k=0; k<dim; k++) {
	    f = tr[k*dim+i+1];
	    tr[k*dim+i+1] = s*tr[k*dim+i]+c*f;
	    tr[k*dim+i] = c*tr[k*dim+i]-s*f;
	  }
	}
	td[dim+l] = td[dim+l]-p;
	td[l] = g;
	td[m] = 0.0;
      }
    } while (m != l);
  }
  return( TRUE );
}

static int E_DMTriDiagonal( double *tr, double *td, int dim )
{  
  /*   tr : matrice originale symetrique                                  */
  /*        au retour, elle contiendra la matrice de passage              */
  /*   td : matrice tridiagonale : td[0-(dim-2)] elements non diagonaux   */
  /*                               td[n-(2dim-1)] elements diagonaux      */
  /*   dim : dimension de la matrice                                      */
  /************************************************************************/
	
  int i, j, k, l;
  double scale, hh, h, g, f;

  for (i=dim-1; i>=1 ; i--) {
    l = i-1;
    h = scale = 0.0;

    if ( l > 0 ) {
      
      for ( k=0; k<=l; k++ ) {
	scale += fabs(tr[i*dim+k]);
      }
      if ( scale == 0.0 ) {
	td[i] = tr[i*dim+l];

      }
      else {

	for (k=0; k<=l; k++) {
	  tr[i*dim+k] /= scale;
	  h += tr[i*dim+k]*tr[i*dim+k];
	}
	f = tr[i*dim+l];
	g = f>0 ? -sqrt(h) : sqrt(h);
	td[i] = scale*g;
	h -= f*g;
	tr[i*dim+l] = f-g;
	f = 0.0;

	for (j=0; j<=l; j++) {
	  /* Next statement can be omitted if eigenvectors not wanted */
	  tr[j*dim+i] = tr[i*dim+j]/h;
	  g = 0.0;
	  for (k=0 ;k<=j; k++)
	    g += tr[j*dim+k]*tr[i*dim+k];
	  for (k=j+1; k<=l; k++)
	    g += tr[k*dim+j]*tr[i*dim+k];
	  td[j] = g/h;
	  f += td[j]*tr[i*dim+j];
	}
	hh = f/(h+h);

	for (j=0; j<=l; j++) {
	  f = tr[i*dim+j];
	  td[j] = g = td[j]-hh*f;
	  for (k=0; k<=j; k++) {
	    tr[j*dim+k] -= (f*td[k]+g*tr[i*dim+k]);
	  }
	}

      }
    }
    else {
      td[i] = tr[i*dim+l];
    }
    td[dim+i] = h;
  } /* for (i=dim-1; i>=1 ; i--) */

  /* Next statement can be omitted if eigenvectors not wanted */
  td[dim] = 0.0;
  td[0] = 0.0;

  /* Contents of this loop can be omitted if eigenvectors not
     wanted except for statement d[i]=tr[i*dim+i]; */
  for (i=0;i<dim;i++) {
    l = i-1;
    if (td[dim+i]) {
      for (j=0; j<=l; j++) {
	g = 0.0;
	for (k=0; k<=l; k++)
	  g += tr[i*dim+k]*tr[k*dim+j];
	for (k=0; k<=l; k++)
	  tr[k*dim+j] -= g*tr[k*dim+i];
      }
    }
    td[dim+i] = tr[i*dim+i];
    tr[i*dim+i] = 1.0;
    for (j=0; j<=l; j++)
      tr[j*dim+i] = tr[i*dim+j]=0.0;
  }
  for (i=1;i<dim;i++)
    td[i-1] = td[i];
  td[dim-1] = 0.0;
  
  return( TRUE );
}

static int E_DMVVpropMatSym( double *m, double *val, double *vec, int dim )
{
  /*--------------------------------------------------
    matrice double
    calcul des valeurs et vecteurs propres d'une
    matrice symetrique
    --------------------------------------------------*/
  int i,j;
  double *td=NULL;
	
  /*   allocations memoires   */
  /****************************/
  td = (double*)malloc( (unsigned)(2*dim*sizeof(double)) );       
  /* matrice tri-diagonale */
	
  /*   transformation de la matrice symetrique en tri-diagonale   */
  /*   et calcul des valeurs et vecteurs propres                  */
  /****************************************************************/
  for (i=0; i<dim; i++)
    for (j=0; j<dim; j++)
      vec[i*dim+j] = m[i*dim+j];

  E_DMTriDiagonal( vec, td, dim );
  E_DMVVpropMatTriDiag( td, vec, dim);
	
  for (i=0; i<dim; i++)
    val[i] = td[dim+i];
	
  free((void *)td);
  return( TRUE );
}

/* Calcul des valeurs et vecteurs propres d'une matrice carree symetrique.

   Matrices et vecteurs sont codes en double.
   Apres calcul (s'il est possible), les valeurs propres sont 
   ordonnees par ordre croissant et rangees dans le vecteur
   val. 
   Leurs vecteurs propres correspondants sont ranges dans 
   le meme ordre dans la matrice vec (chaque colonne
   correspond a un vecteur propre) et ont ete normalises.

RETURN
   Retourne 0 en cas d'erreur ou de calcul impossible.
*/
int E_DMVVpropresMatSym( double *m  /* (square) symmetric matrix whose eigenvectors and values are to be computed */, 
			double *val /* computed eigenvalues (it is a vector) */, 
			double *vec /* computed eigenvectors (it is a matrix whose columns are the eigenvectors */, 
			int dim     /* dimension of vecors and matrix */ )
{
  int i, j;
  int ret, *perm=NULL, *perm2=NULL;
  double norme;
	
  /*   algorithme de calcul proprement dit   */
  /*******************************************/
  if ( !(ret = E_DMVVpropMatSym( m, val, vec, dim )) )
    return( 0 );
	
  /*   on ordonne les valeurs propres   */
  /**************************************/
  perm = (int *)malloc( (unsigned)(dim*sizeof(int)) );
  perm2 = (int*)malloc( (unsigned)(dim*sizeof(int)) );
	
  for (i=0; i<dim; i++)
    perm[i] = perm2[i] = i;
	
  /*--- tri des valeurs propres ---*/
  E_DMTriPointsDbl( val, perm, 0, dim-1 );
	
  /*--- rearrangements des vecteurs propres ---*/
  for (i=0; i<dim; i++) {
    for (j=i; ( (j<dim) && (perm2[j]!=perm[i]) ); j++) ;
    if ( j!=i ) {
      E_DMSwapI(perm2,i,j);
      E_DMEchCKandCL(vec,i,j,dim);
    }
  }
	
  /*--- normalisation des vecteurs propres ---*/
  for (i=0; i<dim; i++) {
    norme = 0.0;
    for (j=0; j<dim; j++)
      norme += vec[j*dim+i]*vec[j*dim+i];
    if ( norme < DBLZERO )
      continue;
    norme = sqrt(norme);
    for (j=0; j<dim; j++)
      vec[j*dim+i] /= norme;
  }

  free((void *) perm );
  free((void *) perm2 );
  return( ret );
}







/* Calcul d'une matrice de rotation a partir d'un quaternion.

   Le titre dit tout.

BUGS
   Aucune verification sur la validite du quaternion.
*/

void E_MatRotFromQuat( double qr[4] /* input quaternion */, 
		       double rot[9] /* result rotation matrix */ )
{
  /*   calcul de la matrice de rotation a partir du quaternion   */
  /***************************************************************/
  double norme;
  int i;
  
  norme = qr[3]*qr[3];
  norme += qr[2]*qr[2];
  norme += qr[1]*qr[1];
  norme += qr[0]*qr[0];
  
  rot[0] = qr[0]*qr[0] + qr[1]*qr[1] - qr[2]*qr[2] - qr[3]*qr[3];
  rot[4] = qr[0]*qr[0] - qr[1]*qr[1] + qr[2]*qr[2] - qr[3]*qr[3];
  rot[8] = qr[0]*qr[0] - qr[1]*qr[1] - qr[2]*qr[2] + qr[3]*qr[3];
  rot[1] = (qr[1]*qr[2] - qr[0]*qr[3]) * 2.0;
  rot[2] = (qr[1]*qr[3] + qr[0]*qr[2]) * 2.0;
  rot[3] = (qr[1]*qr[2] + qr[0]*qr[3]) * 2.0;
  rot[5] = (qr[2]*qr[3] - qr[0]*qr[1]) * 2.0;
  rot[6] = (qr[1]*qr[3] - qr[0]*qr[2]) * 2.0;
  rot[7] = (qr[2]*qr[3] + qr[0]*qr[1]) * 2.0;
  
  for (i=0;i<9;i++)
    rot[i] /= norme;
}


/* Calcul d'une matrice de rotation a partir d'un vecteur rotation

*/

void E_MatRotFromRVec( double v[3] /* input rotation vector */, 
		       double rot[9] /* result rotation matrix */ )
{
  /*   calcul de la matrice de rotation a partir du quaternion   */
  /***************************************************************/
  double theta, n[3];
  double c, s;
  int i;

  for (i=0;i<9;i++) rot[i] = 0.0;
  rot[0] = rot[4] = rot[8] = 1.0;

  theta = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  if ( theta < 0.000001 ) return;

  c = cos( theta );
  s = sin( theta );
  for (i=0;i<3;i++)  n[i] = v[i] / theta;
    
  rot[0] = 1.0 - (1.0 - c) * (n[1]*n[1] + n[2]*n[2]); 

  rot[1] = - s * n[2] + (1.0 - c) * n[0]*n[1];
  rot[2] =   s * n[1] + (1.0 - c) * n[0]*n[2];
  rot[3] =   s * n[2] + (1.0 - c) * n[0]*n[1];

  rot[4] = 1.0 - (1.0 - c) * (n[0]*n[0] + n[2]*n[2]); 

  rot[5] = - s * n[0] + (1.0 - c) * n[1]*n[2];
  rot[6] = - s * n[1] + (1.0 - c) * n[0]*n[2];
  rot[7] =   s * n[0] + (1.0 - c) * n[1]*n[2];

  rot[8] = 1.0 - (1.0 - c) * (n[0]*n[0] + n[1]*n[1]); 

}

