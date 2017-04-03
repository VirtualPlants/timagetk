/*************************************************************************
 * transfo.h -
 *
 * $Id: transfo.h,v 1.5 2000/10/18 08:45:55 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Sep  6 21:52:46 MEST 2000
 *
 *
 * ADDITIONS, CHANGES
 * - Sat Oct  7 00:01:48 MEST 2000, G. Malandain
 *   Ajouts:
 *   - RotationMatrixFromQuaternion()
 *   - ComputeRigidTransfoBetweenListsOfPts()
 *   - TestComputeRigidTransfoBetweenListsOfPts()
 *
 */


#ifndef _transfo_h_
#define _transfo_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdio.h>
#include <eigens.h>
#include <stdlib.h>

/* mat is an array of 9 double,
 * here mat describes the 3x3 rotation matrix,
 * not a matrix in homogeneous coordinates
 */
extern void RotationMatrixFromRotationVector( double *mat,
					      double *rot );
extern void RotationVectorFromRotationMatrix( double *rot,
                                              double *mat );

extern void RotationMatrixFromQuaternion( double *mat,
					  double *quat );










/* Transformation du vecteur unitaire (x,y,z) en 2 angles (theta,phi)

   ( x )   ( cos theta sin phi )
   ( y ) = ( sin theta sin phi )
   ( z )   ( cos phi )
*/
extern void UnitVectorToSphericalAngles( const double *v, 
					 double *theta,
					 double *phi );

/* Transformation de 2 angles (theta,phi) en vecteur unitaire (x,y,z)

   ( x )   ( cos theta sin phi )
   ( y ) = ( sin theta sin phi )
   ( z )   ( cos phi )
*/
extern void SphericalAnglesToUnitVector( const double theta,
					 const double phi,
					 double *v );



/* Transformation de 2 angles (theta,phi) en un systeme orthonorme

   ( x )   ( cos theta sin phi )
   ( y ) = ( sin theta sin phi )
   ( z )   ( cos phi )

   (  sin theta )
   ( -cos theta )
   ( 0 )
   
   ( cos theta cos phi )
   ( sin theta cos phi )
   ( -sin phi )

*/
extern void SphericalAnglesToUnitsVectors( const double theta,
					   const double phi,
					   double *v1, 
					   double *v2,
					   double *v3 );




/* Calcule une transformation rigide (aux moindres carres)
   entre deux listes de points apparies
   
   le point d'indice #i dans une liste a pour coordonnees
   (X,Y,Z) = (liste[3*i], liste[3*i+1], liste[3*i+2])

   on renvoie une matrice en coordonnees homogenes,
   ie (X,Y,Z,1)^t, qui permet
   de passer de la liste 1 a la liste 2
   mat * list1[#i] = list2[#i]
   
   Une matrice en coordonnees homogenes est une matrice 4x4, indicee
   comme suit
   (  #0    #1    #2    #3   )
   (  #4    #5    #6    #7   )
   (  #8    #9   #10   #11   )
   ( #12=0 #13=0 #14=0 #15=1 )
   La sous-matrice 3x3 en haut a gauche est la matrice de rotation
   le vecteur 1x3 a droite est une translation.

   list1 est donc un pointeur sur un tableau de double de taille 3*nbPts
   list2 aussi
   mat est donc un pointeur sur un tableau de double de taille 16

*/

extern int ComputeRigidTransfoBetweenListsOfPts( double *list1, 
						 double *list2,
						 int nbPts,
						 double *mat );

extern double _TestComputeRigidTransfoBetweenListsOfPts( double *list1, 
							 double *list2,
							 int nbPts );

#ifdef __cplusplus
}
#endif

#endif
