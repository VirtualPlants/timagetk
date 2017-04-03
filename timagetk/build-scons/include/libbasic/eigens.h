/*************************************************************************
 * eigens.h -
 *
 * $Id: eigens.h,v 1.3 2001/07/12 13:29:40 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Thu Sep  7 11:04:44 MET DST 2000
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#ifndef _eigens_h_
#define _eigens_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef _LINUX_
long int random(void);
#endif


/* calcul les valeurs et vecteurs propres
   d'une matrice symetrique carree
   
   renvoie les valeurs propres dans le desordre
   les vecteurs propres sont les colonnes 
   de la matrice des vecteurs propres
   
   a la valeur propre eigenvalues[i] correspond le
   vecteur propre eigenvectors[j*dim+i], j=0...dim-1
*/
  
extern int _ComputeEigensOfSymetricSquareMatrix( const double *mat,
					  double *eigenvalues,
					  double *eigenvectors,
					  int dim );


/* tri des valeurs propres
   
   eigenvalues[0] <= eigenvalues[1] <= ... <= eigenvalues[dim-1]
*/
extern int _SortEigensInIncreasingOrder( double *eigenvalues,
				  double *eigenvectors,
				  int dim );
/* tri des valeurs propres
   
   |eigenvalues[0]| <= |eigenvalues[1]| <= ... <= |eigenvalues[dim-1]|
*/

extern int _SortEigensInAbsIncreasingOrder( double *eigenvalues,
				  double *eigenvectors,
				  int dim );

extern void _TestEigens( int matrixDim );


#ifdef __cplusplus
}
#endif

#endif
