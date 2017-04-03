/*************************************************************************
 * transfo.c -
 *
 * $Id: transfo.c,v 1.7 2001/05/29 16:00:02 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Sep  6 21:52:28 MEST 2000
 *
 *
 * ADDITIONS, CHANGES
 * - Sat Oct  7 00:01:48 MEST 2000, G. Malandain
 *   Ajouts:
 *   - RotationMatrixFromQuaternion()
 *   - ComputeRigidTransfoBetweenListsOfPts()
 *   - TestComputeRigidTransfoBetweenListsOfPts() *
 */

#include <transfo.h>

static int _verbose_ = 1;









/* calcul la matrice de rotation a partir
   d'un vecteur de rotation r

   R = I + f(theta) X(r) + g(theta) [X(r) * X(r)]

   avec theta qui est l'angle de la rotation (norme du vecteur rotation), 
        g(theta) = (1 - cos(theta)) / (theta * theta)
        f(theta) = sin(theta) / theta
   et X(r) qui est la matrice du produit vectoriel par r.

   Si la rotation est donnee par un vecteur unitaire v et un angle
   theta, on a alors
   
   R = I + sin(theta) X(v) + (1 - cos(theta)) X(v)*X(v)

   D'ou on deduit que 
   trace(R) = 3 - 2*(1 - cos(theta)) = 1 + 2*cos(theta)


*/
void RotationMatrixFromRotationVector( double *mat,
				       double *rot )
{
  /* mat is an array of 9 double,
   * here mat describes the 3x3 rotation matrix,
   * not a matrix in homogeneous coordinates
   */
  double f, g, theta, t2;
  
  t2 = rot[0]*rot[0] + rot[1]*rot[1] + rot[2]*rot[2];
  theta = sqrt( t2 );
  
  if ( theta > 1e-8 ) {
    f = sin( theta ) / theta;
    g = ( 1.0 - cos( theta ) ) / ( t2 );
    
    mat[0] = 1.0 - g * (rot[1]*rot[1] + rot[2]*rot[2]);
    mat[4] = 1.0 - g * (rot[2]*rot[2] + rot[0]*rot[0]);
    mat[8] = 1.0 - g * (rot[0]*rot[0] + rot[1]*rot[1]);
    
    mat[3] = mat[1] = g * rot[0] * rot[1];
    mat[6] = mat[2] = g * rot[0] * rot[2];
    mat[7] = mat[5] = g * rot[2] * rot[1];
    
    mat[1] -= f * rot[2];

    mat[2] += f * rot[1];
    mat[5] -= f * rot[0];
    
    mat[3] += f * rot[2];
    mat[6] -= f * rot[1];
    mat[7] += f * rot[0];
  }
  else {
    mat[0] = mat[4] = mat[8] = 1.0;
    mat[1] = mat[2] = mat[3] = 0.0;
    mat[5] = mat[6] = mat[7] = 0.0;
  }
}



void RotationVectorFromRotationMatrix( double *rot,
                                       double *mat )
{
  double theta;
  double n[3][3], nrot;

  /* trace(R) = 3 - 2*(1 - cos(theta)) = 1 + 2*cos(theta)
   */
  theta = acos( (mat[0] + mat[4] + mat[8] - 1) / 2.0 );

  if ( theta > 0.0 ) {
      n[0][0] = 0.0;
      n[0][1] = (mat[1] - mat[3])/(2*sin(theta));
      n[0][2] = (mat[2] - mat[6])/(2*sin(theta));

      n[1][0] = (mat[3] - mat[1])/(2*sin(theta));
      n[1][1] = 0.0;
      n[1][2] = (mat[5] - mat[7])/(2*sin(theta));

      n[2][0] = (mat[6] - mat[2])/(2*sin(theta));
      n[2][1] = (mat[7] - mat[5])/(2*sin(theta));
      n[2][2] = 0.0;

      rot[0] = (n[2][1] - n[1][2])/2.0;
      rot[1] = (n[0][2] - n[2][0])/2.0;
      rot[2] = (n[1][0] - n[0][1])/2.0;

      nrot = sqrt( rot[0]*rot[0] + rot[1]*rot[1] + rot[2]*rot[2]);

      rot[0] *= theta / nrot;
      rot[1] *= theta / nrot;
      rot[2] *= theta / nrot;
  }
  else {
      rot[0] = rot[1] = rot[2] = 0.0;
  }
}






/* calcul la matrice de rotation a partir
   d'un quaternion
*/

void RotationMatrixFromQuaternion( double *mat,
				   double *quat )
{
  double norme;
  int i;
  
  norme = quat[3]*quat[3] + quat[2]*quat[2] + quat[1]*quat[1] + quat[0]*quat[0];
  
  mat[0] = quat[0]*quat[0] + quat[1]*quat[1] - quat[2]*quat[2] - quat[3]*quat[3];
  mat[4] = quat[0]*quat[0] - quat[1]*quat[1] + quat[2]*quat[2] - quat[3]*quat[3];
  mat[8] = quat[0]*quat[0] - quat[1]*quat[1] - quat[2]*quat[2] + quat[3]*quat[3];
  mat[1] = (quat[1]*quat[2] - quat[0]*quat[3]) * 2.0;
  mat[2] = (quat[1]*quat[3] + quat[0]*quat[2]) * 2.0;
  mat[3] = (quat[1]*quat[2] + quat[0]*quat[3]) * 2.0;
  mat[5] = (quat[2]*quat[3] - quat[0]*quat[1]) * 2.0;
  mat[6] = (quat[1]*quat[3] - quat[0]*quat[2]) * 2.0;
  mat[7] = (quat[2]*quat[3] + quat[0]*quat[1]) * 2.0;
  
  /* le quaternion devrait theoriquement etre unitaire
     mais on normalise, au cas ou ...
  */

  for (i=0;i<9;i++)
    mat[i] /= norme;
}













/* Transformation du vecteur unitaire (x,y,z) en 2 angles (theta,phi)

   ( x )   ( cos theta sin phi )
   ( y ) = ( sin theta sin phi )
   ( z )   ( cos phi )
*/
void UnitVectorToSphericalAngles( const double *v, 
				  double *theta,
				  double *phi )
{
  double n;
  
  /* from % `man acos`
     
    ...
    The acos() and acosf() functions compute the principal value of the arc
    cosine of x in the interval [0,pi] radians. The value of x must be in the
    domain [-1,1].
    ...
    The atan2() and atan2f() functions compute the principal value of the arc
    tangent of y/x, in the interval [-pi,pi] radians.  The sign of atan2() and
    atan2f() is determined by the sign of y.  The value of atan2(y,x) is com-
    puted as follows where f is the number of fraction bits associated with the
    data type.
    _____________________________________________________
    Value of Input Arguments    Angle Returned
    _____________________________________________________
    x = 0 or y/x > 2**(f+1)     pi/2 * (sign y)
    x > 0 and y/x <= 2**(f+1)   atan(y/x)
    x < 0 and y/x <= 2**(f+1)   pi * (sign y) + atan(y/x)
    _____________________________________________________
    ...
  */

  n = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  *phi   = acos( v[2]/n );
  *theta = atan2( v[1]/n, v[0]/n );
}









/* Transformation de 2 angles (theta,phi) en vecteur unitaire (x,y,z)

   ( x )   ( cos theta sin phi )
   ( y ) = ( sin theta sin phi )
   ( z )   ( cos phi )


*/
void SphericalAnglesToUnitVector( const double theta,
				  const double phi,
				  double *v )
{
  v[0] = sin( phi ) * cos( theta );
  v[1] = sin( phi ) * sin( theta );
  v[2] = cos( phi );
}










/* Transformation de 2 angles (theta,phi) en repere orthonorme

   le vecteur lie au 2 angles est 
   ( x )   ( cos theta sin phi )
   ( y ) = ( sin theta sin phi )
   ( z )   ( cos phi )

   un vecteur orthonorme est
   (  sin theta )
   ( -cos theta )
   ( 0 )
   
   et le troisieme vecteur (par produit vectoriel) est alors
   ( cos theta cos phi )
   ( sin theta cos phi )
   ( -sin phi )

   Le repere est donc direct.

*/

void SphericalAnglesToUnitsVectors( const double theta,
				    const double phi,
				    double *v1, 
				    double *v2,
				    double *v3 )
{
  double sp, cp, ct, st;

  sp = sin( phi );
  cp = cos( phi );
  st = sin( theta );
  ct = cos( theta );
  
  v1[0] = sp * ct;
  v1[1] = sp * st;
  v1[2] = cp;
  
  v2[0] =  st;
  v2[1] = -ct;
  v2[2] = 0.0;
  
  v3[0] =  cp * ct;
  v3[1] =  cp * st;
  v3[2] = -sp;
}











/* Calcule une transformation rigide (aux moindres carres)
   entre deux listes de points apparies
   
   le point d'indice #i dans une liste a pour coordonnees
   (X,Y,Z) = (liste[3*i], liste[3*i+1], liste[3*i+2])

   on renvoie une matrice en coordonnees homogenes qui permet
   de passer de la liste 1 a la liste 2
   mat * list1[#i] = list2[#i]
*/


int ComputeRigidTransfoBetweenListsOfPts( double *list1, 
					  double *list2,
					  int nbPts,
					  double *mat )
{
  char *proc = "ComputeRigidTransfoBetweenListsOfPts";
  int i, n;

  double b1[3], b2[3];
  double x1, y1, z1, x2, y2, z2;
  double sum[16], a[16];
  double valp[4], vecp[16];
  double quat[4], rot[9];

  
  /* calcul des barycentres
   */
  for ( i=0; i<3; i++ ) {
    b1[i] = list1[i];
    b2[i] = list2[i];
  }
  
  for ( n=1; n<nbPts; n++ ) {
    for ( i=0; i<3; i++ ) {
      b1[i] += list1[3*n+i];
      b2[i] += list2[3*n+i];
    }
  }
  
  for ( i=0; i<3; i++ ) {
    b1[i] /= (double)nbPts;
    b2[i] /= (double)nbPts;
  }



  /* construction de la matrice 4x4
     pour la recherche du quaternion
     on calcule sum_i (a^T * a)
  */
  for (i=0;i<16;i++) sum[i] = 0.0;

  for ( n=0; n<nbPts; n++ ) {

    x1 = list1[3*n]   - b1[0];
    y1 = list1[3*n+1] - b1[1];
    z1 = list1[3*n+2] - b1[2];

    x2 = list2[3*n]   - b2[0];
    y2 = list2[3*n+1] - b2[1];
    z2 = list2[3*n+2] - b2[2];
    
    a[0] = a[5] = a[10] = a[15] = 0.0;
    a[1] = x1 - x2; 
    a[2] = y1 - y2;
    a[3] = z1 - z2;
    a[6] = -(z1 + z2);
    a[7] = y1 + y2;
    a[11] = -(x1 + x2);
    a[4] = -a[1];
    a[8] = -a[2];
    a[9] = -a[6];
    a[12] = -a[3];
    a[13] = -a[7];
    a[14] = -a[11];
    
    sum[ 0] += a[ 0]*a[ 0] + a[ 4]*a[ 4] + a[ 8]*a[ 8] + a[12]*a[12];
    sum[ 1] += a[ 1]*a[ 0] + a[ 5]*a[ 4] + a[ 9]*a[ 8] + a[13]*a[12];
    sum[ 2] += a[ 2]*a[ 0] + a[ 6]*a[ 4] + a[10]*a[ 8] + a[14]*a[12];
    sum[ 3] += a[ 3]*a[ 0] + a[ 7]*a[ 4] + a[11]*a[ 8] + a[15]*a[12];

    sum[ 4] += a[ 0]*a[ 1] + a[ 4]*a[ 5] + a[ 8]*a[ 9] + a[12]*a[13];
    sum[ 5] += a[ 1]*a[ 1] + a[ 5]*a[ 5] + a[ 9]*a[ 9] + a[13]*a[13];
    sum[ 6] += a[ 2]*a[ 1] + a[ 6]*a[ 5] + a[10]*a[ 9] + a[14]*a[13];
    sum[ 7] += a[ 3]*a[ 1] + a[ 7]*a[ 5] + a[11]*a[ 9] + a[15]*a[13];

    sum[ 8] += a[ 0]*a[ 2] + a[ 4]*a[ 6] + a[ 8]*a[10] + a[12]*a[14];
    sum[ 9] += a[ 1]*a[ 2] + a[ 5]*a[ 6] + a[ 9]*a[10] + a[13]*a[14];
    sum[10] += a[ 2]*a[ 2] + a[ 6]*a[ 6] + a[10]*a[10] + a[14]*a[14];
    sum[11] += a[ 3]*a[ 2] + a[ 7]*a[ 6] + a[11]*a[10] + a[15]*a[14];

    sum[12] += a[ 0]*a[ 3] + a[ 4]*a[ 7] + a[ 8]*a[11] + a[12]*a[15];
    sum[13] += a[ 1]*a[ 3] + a[ 5]*a[ 7] + a[ 9]*a[11] + a[13]*a[15];
    sum[14] += a[ 2]*a[ 3] + a[ 6]*a[ 7] + a[10]*a[11] + a[14]*a[15];
    sum[15] += a[ 3]*a[ 3] + a[ 7]*a[ 7] + a[11]*a[11] + a[15]*a[15];

  }


  if ( _ComputeEigensOfSymetricSquareMatrix( sum, valp, vecp, 4 ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute eigen values and vectors\n", proc );
    return( 0 );
  }





  if ( _SortEigensInIncreasingOrder( valp, vecp, 4 ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to sort eigen values and vectors\n", proc );
    return( 0 );
  }
  


  /* le quaternion realisant la rotation de plus petite somme
     de carres est celui associe a la plus petite valeur propre
     le residu est la valeur propre associee (valp[0])
  */
  for (i=0;i<4;i++) quat[i] = vecp[4*i];
  
  
  /* on passe du quaternion a la matrice de rotation
   */
  RotationMatrixFromQuaternion( rot, quat );
  
  /* reste a calculer la translation a partir
     des barycentres
     elle est egale a b2 - R * b1
     ainsi b1 se transforme en b2
  */
  mat[ 0] = rot[0];
  mat[ 1] = rot[1];
  mat[ 2] = rot[2];
  mat[ 3] = b2[0] - rot[0]*b1[0] - rot[1]*b1[1] - rot[2]*b1[2];

  mat[ 4] = rot[3];
  mat[ 5] = rot[4];
  mat[ 6] = rot[5];
  mat[ 7] = b2[1] - rot[3]*b1[0] - rot[4]*b1[1] - rot[5]*b1[2];

  mat[ 8] = rot[6];
  mat[ 9] = rot[7];
  mat[10] = rot[8];
  mat[11] = b2[2] - rot[6]*b1[0] - rot[7]*b1[1] - rot[8]*b1[2];

  mat[12] = mat[13] = mat[14] = 0.0;
  mat[15] = 1.0;
  
  return( 1 );
}

  






  





#include <time.h>

#include <stdlib.h>

/* programme de test
   
   - tire au hasard la liste 1
                    une transformation
   - calcule la liste 2
   - estime la transformation pour passer de la liste 1 a la liste 2
   - renvoie la somme des carres des differences
*/

double _TestComputeRigidTransfoBetweenListsOfPts( double *list1, 
						  double *list2,
						  int nbPts )
{
  char *proc = "_TestComputeRigidTransfoBetweenListsOfPts";
  double max = 2147483647; /* (2^31)-1 */

  double angleMaxDegrees = 30.0;
  double translationMax  = 30.0;
  double angle;
  double vec[3], rot[9], b1[3], b2[3], trs[3], mat[16], m[16];
  double norme, d, error, e;

  int n, i;
  

  /* on tire des points de coordonnees entre -1 et 1 
   */
  b1[0] = b1[1] = b1[2] = 0.0;
  for ( n=0; n<nbPts; n++ ) 
    for ( i=0; i<3; i++ ) {
      list1[3*n+i] = 2.0*random() / max - 1.0;
      b1[i] += list1[3*n+i];
    }
  for ( i=0; i<3; i++ ) b1[i] /= nbPts;
  


  /* on tire une transformation au hasard
     1. un vecteur unitaire 
     2. un angle dans [-angleMax, angleMax]
     3. une translation dans [-translationMax, translationMax]
        la translation transforme le barycentre:
	b2 = b1 + trs = M*b1 = (R*b1 + T)
	d'ou la translation de la matrice en coordonnees homogenes
	T = b1 + trs - R*b1
  */

  do {
    vec[0] = 2.0*random() / max - 1.0;
    vec[1] = 2.0*random() / max - 1.0;
    vec[2] = 2.0*random() / max - 1.0;
    norme = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
  } while ( norme > 1 || norme < 0.0000001 );
  norme = sqrt( norme );
  vec[0] /= norme;
  vec[1] /= norme;
  vec[2] /= norme;

  angle = (2.0*random()/max - 1.0)*angleMaxDegrees*(3.1415927 / 180.0);
  vec[0] *= angle;
  vec[1] *= angle;
  vec[2] *= angle;

  do {
    trs[0] = (2.0*random() / max - 1.0) * translationMax;
    trs[1] = (2.0*random() / max - 1.0) * translationMax;
    trs[2] = (2.0*random() / max - 1.0) * translationMax;
    norme = trs[0]*trs[0] + trs[1]*trs[1] + trs[2]*trs[2];
  } while ( norme > translationMax*translationMax );
  


  RotationMatrixFromRotationVector( rot, vec );


  m[ 0] = rot[0];
  m[ 1] = rot[1];
  m[ 2] = rot[2];
  m[ 3] = b1[0] + trs[0] - rot[0]*b1[0] - rot[1]*b1[1] - rot[2]*b1[2];

  m[ 4] = rot[3];
  m[ 5] = rot[4];
  m[ 6] = rot[5];
  m[ 7] = b1[1] + trs[1] - rot[3]*b1[0] - rot[4]*b1[1] - rot[5]*b1[2];

  m[ 8] = rot[6];
  m[ 9] = rot[7];
  m[10] = rot[8];
  m[11] = b1[2] + trs[2] - rot[6]*b1[0] - rot[7]*b1[1] - rot[8]*b1[2];

  m[12] = m[13] = m[14] = 0.0;
  m[15] = 1.0;

  
  /* on peut donc calculer la liste 2
   */
  for ( n=0; n<nbPts; n++ ) {
    list2[3*n  ] = m[ 0]*list1[3*n] + m[ 1]*list1[3*n+1] + m[ 2]*list1[3*n+2] + m[ 3];
    list2[3*n+1] = m[ 4]*list1[3*n] + m[ 5]*list1[3*n+1] + m[ 6]*list1[3*n+2] + m[ 7];
    list2[3*n+2] = m[ 8]*list1[3*n] + m[ 9]*list1[3*n+1] + m[10]*list1[3*n+2] + m[11];
  }


  /* on peut ajouter un bruit uniforme 
   */
  if ( 0 ) {
    for ( n=0; n<3*nbPts; n++ )
      list2[n] += (2.0*random()/max - 1.0)*0.01;
  }


  if ( ComputeRigidTransfoBetweenListsOfPts( list1, list2, nbPts, mat ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute transformation\n", proc );
    return( -1000000 );
  }


  
  error = 0;
  for ( i=0; i<3; i++ ) b2[i] = 0.0;

  for ( n=0; n<nbPts; n++ ) {
    b2[0] += mat[ 0]*list1[3*n] + mat[ 1]*list1[3*n+1] + mat[ 2]*list1[3*n+2] + mat[ 3];
    b2[1] += mat[ 4]*list1[3*n] + mat[ 5]*list1[3*n+1] + mat[ 6]*list1[3*n+2] + mat[ 7];
    b2[2] += mat[ 8]*list1[3*n] + mat[ 9]*list1[3*n+1] + mat[10]*list1[3*n+2] + mat[11];
    d = mat[ 0]*list1[3*n] + mat[ 1]*list1[3*n+1] + mat[ 2]*list1[3*n+2] + mat[ 3] - list2[3*n];
    error += d*d;
    d = mat[ 4]*list1[3*n] + mat[ 5]*list1[3*n+1] + mat[ 6]*list1[3*n+2] + mat[ 7] - list2[3*n+1];
    error += d*d;
    d = mat[ 8]*list1[3*n] + mat[ 9]*list1[3*n+1] + mat[10]*list1[3*n+2] + mat[11] - list2[3*n+2];
    error += d*d;
  }

  for ( i=0; i<3; i++ ) b2[i] /= nbPts;
  
  e = 0.0;
  for ( i=0; i<3; i++ ) {
    d = (b2[i] - b1[i] - trs[i]);
    e += d*d;
  }
  fprintf( stderr, "    - erreur en translation      = %f\n", sqrt(e) );
  
  e = acos ( (mat[0]+mat[5]+mat[10]-1.0) / 2.0 );
  fprintf( stderr, "    - erreur d'angle de rotation = %f degres\n", fabs(fabs(angle)-fabs(e)) * 180.0/3.1415927 );


  return( error );
}
