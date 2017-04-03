/*************************************************************************
 * bal-lineartrsf-tools.c -
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
#include <math.h>
#include <time.h>


#include <bal-matrix.h>
#include <bal-lineartrsf-tools.h>

static int _verbose_ = 1;



static void RotationMatrixFromRotationVector( double *mat,
					      double *rot );


/************************************************************
 * 
 * random sampling utilities
 *
 ************************************************************/


static long int _random_seed_ = 0;
static int _seed_is_set_ = 0;

long int _GetRandomSeed()
{
  return( _random_seed_ );
}

void _SetRandomSeed( long int seed )
{
  srandom( seed );
  _random_seed_ = seed;
  _seed_is_set_ = 1;
}


/* returns a random value in the interval [0.0, 1.0).
 */
double _GetRandom( )
{
    if ( _seed_is_set_ == 0 ) {
        _SetRandomSeed( time(0) );
        _seed_is_set_ = 2;
    }
    return( (double)random()/(double)(RAND_MAX) );
}



/* returns a random value from a normal distribution of mean 0 and 
   standard deviation 1.

   The Box-Muller method uses two independent random numbers U and V 
   distributed uniformly on (0,1). Then the two random variables X and Y
   \begin{align} 
   & X = \sqrt{- 2 \ln U} \, \cos(2 \pi V) , \\ 
   & Y = \sqrt{- 2 \ln U} \, \sin(2 \pi V) . 
   \end{align} 
   will both have the standard normal distribution, and will be independent.
*/
double _GetBasicNormalRandom()
{
    register double u1,u2;

    u1 = _GetRandom();
    u2 = _GetRandom();
    return ( sqrt(-2.0*log(u1))*cos(2* 3.14159265358979323846 *u2));
}



/* returns a random value from a normal distribution of mean mu and 
   standard deviation stddev.
*/
double _GetNormalRandom( double mu, double stddev )
{
    return ( stddev * _GetBasicNormalRandom() + mu );
}





/************************************************************
 * 
 * random 3x3 matrices
 *
 ************************************************************/

void _IdentityMatrix( double *mat )
{
  mat[0] = 1.0;   mat[1] = 0.0;   mat[2] = 0.0;
  mat[3] = 0.0;   mat[4] = 1.0;   mat[5] = 0.0;
  mat[6] = 0.0;   mat[7] = 0.0;   mat[8] = 1.0;
}



void _Random2DTranslationVector( double *v,
				 double *translation_interval )
{
  v[0] = translation_interval[0] + (translation_interval[1]-translation_interval[0]) * _GetRandom();
  v[1] = translation_interval[0] + (translation_interval[1]-translation_interval[0]) * _GetRandom();
}



void _2DRotationMatrix( double *mat,
			double angle )
{

  _IdentityMatrix( mat );
  
  mat[0] = cos(angle);   mat[1] = -sin(angle);
  mat[3] = sin(angle);   mat[4] = cos(angle);
}



void _Random2DRotationMatrix( double *mat,
			      double *angle_interval )
{
  double a;

  /* pick a random angle 
   */
  a = angle_interval[0] + (angle_interval[1]-angle_interval[0]) * _GetRandom();
  
  _2DRotationMatrix( mat, a );
}



void _2DScaleMatrix( double *mat,
		     double sx, double sy )
{
  
  _IdentityMatrix( mat );
  
  mat[0] = sx;
  mat[4] = sy;
}



void _Random2DScaleMatrix( double *mat,
			   double *scale_interval )
{
  double sx, sy;

  sx = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();
  sy = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();

  _2DScaleMatrix( mat, sx, sy );
}



void _2DShearMatrix( double *mat,
			   double sxy, double syx )
{
  
  _IdentityMatrix( mat );
  
  mat[1] = sxy;
  mat[3] = syx;
}



void _Random2DShearMatrix( double *mat,
				       double *shear_interval )
{
  double sxy, syx;
  
  sxy = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  syx = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();

  _2DShearMatrix( mat, sxy, syx );
}



void _2DSimilitudeMatrix( double *mat,
			  double angle,
			  double scale )
{
  char *proc = "_2DSimilitudeMatrix";

  _IdentityMatrix( mat );
  _2DRotationMatrix( mat, angle );

  if ( _verbose_ )
    fprintf( stdout, "%s: global scale = %lf\n", proc, scale );

  mat[0] *= scale;   mat[1] *= scale;
  mat[3] *= scale;   mat[4] *= scale;
}



void _Random2DSimilitudeMatrix( double *mat,
				double *angle_interval,
				double *scale_interval )
{
  double a, s;
  
  a = angle_interval[0] + (angle_interval[1]-angle_interval[0]) * _GetRandom();
  s = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();

  _2DSimilitudeMatrix( mat, a, s );
}



void _2DAffineMatrix( double *mat,
			    double a, 
			    double scx, double scy,
			    double shxy, double shyx )
{
  double r[9], s[9], t[9];
  
  _IdentityMatrix( mat );
  _2DScaleMatrix( s, scx, scy );
  _2DRotationMatrix( r, a );
  E_DMMatMul ( s, r, t, 3 );
  _2DShearMatrix( s, shxy, shyx );
  E_DMMatMul ( t, s, mat, 3 );
}



void _Random2DAffineMatrix( double *mat,
			    double *angle_interval,
			    double *scale_interval,
			    double *shear_interval )
{
  double a, scx, scy, shxy, shyx;

  a = angle_interval[0] + (angle_interval[1]-angle_interval[0]) * _GetRandom();
  scx = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();
  scy = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();
  shxy = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  shyx = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();

  _2DAffineMatrix( mat, a, scx, scy, shxy, shyx );

}



void _Random3DTranslationVector( double *v,
				 double *translation_interval )
{
  v[0] = translation_interval[0] + (translation_interval[1]-translation_interval[0]) * _GetRandom();
  v[1] = translation_interval[0] + (translation_interval[1]-translation_interval[0]) * _GetRandom();
  v[2] = translation_interval[0] + (translation_interval[1]-translation_interval[0]) * _GetRandom();
}



void _3DRotationMatrix( double *mat,
			double *v, double angle )
{
  double n;

  _IdentityMatrix( mat );

  n = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  
  v[0] *= angle/n;
  v[1] *= angle/n;
  v[2] *= angle/n;
  
  RotationMatrixFromRotationVector( mat, v );
}



void _Random3DRotationMatrix( double *mat,
			      double *angle_interval )
{
  double a, n, v[3];

  /* pick a random vector
   */
  do { 
    v[0] = (2.0 * _GetRandom() - 1.0 );
    v[1] = (2.0 * _GetRandom() - 1.0 );
    v[2] = (2.0 * _GetRandom() - 1.0 );
    n = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  } while ( n < 0.0001 );
  
  /* pick a random angle 
   */
  a = angle_interval[0] + (angle_interval[1]-angle_interval[0]) * _GetRandom();
  
  _3DRotationMatrix( mat, v, a );
}



void _3DScaleMatrix( double *mat,
		     double sx, double sy, double sz )
{
  
  _IdentityMatrix( mat );
  
  mat[0] = sx;
  mat[4] = sy;
  mat[8] = sz;
}



void _Random3DScaleMatrix( double *mat,
			   double *scale_interval )
{
  double sx, sy, sz;
  
  sx = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();
  sy = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();
  sz = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();

  _3DScaleMatrix( mat, sx, sy, sz );
}



void _3DShearMatrix( double *mat,
		     double sxy, double sxz,
		     double syx, double syz,
		     double szx, double szy )
{
  
  _IdentityMatrix( mat );
  
  mat[1] = sxy;
  mat[2] = sxz;
  mat[3] = syx;
  mat[5] = syz;
  mat[6] = szx;
  mat[7] = szy;
}



void _Random3DShearMatrix( double *mat,
			   double *shear_interval )
{
  double sxy, sxz;
  double syx, syz;
  double szx, szy;
  
  sxy = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  sxz = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  syx = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  syz = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  szx = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  szy = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();

  _3DShearMatrix( mat, sxy, sxz, syx, syz, szx, szy );
}



void _3DSimilitudeMatrix( double *mat,
			  double *v, double angle,
			  double scale )
{
  char *proc = "_3DSimilitudeMatrix";
  
  _3DRotationMatrix( mat, v, angle );

  if ( _verbose_ )
    fprintf( stdout, "%s: global scale = %lf\n", proc, scale );

  mat[0] *= scale;   mat[1] *= scale;   mat[2] *= scale;
  mat[3] *= scale;   mat[4] *= scale;   mat[5] *= scale;
  mat[6] *= scale;   mat[7] *= scale;   mat[8] *= scale;
}



void _Random3DSimilitudeMatrix( double *mat,
				double *angle_interval,
				double *scale_interval )
{
  double a, n, v[3];
  double s;  

  /* pick a random vector
   */
  do { 
    v[0] = (2.0 * _GetRandom() - 1.0 );
    v[1] = (2.0 * _GetRandom() - 1.0 );
    v[2] = (2.0 * _GetRandom() - 1.0 );
    n = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  } while ( n < 0.0001 );
  
  /* pick a random angle 
   */
  a = angle_interval[0] + (angle_interval[1]-angle_interval[0]) * _GetRandom();

  /* pick a random scale
   */
  s = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();

  _3DSimilitudeMatrix( mat, v, a, s );
}



void _3DAffineMatrix( double *mat,
		      double *v, double angle,
		      double scx, double scy, double scz,
		      double shxy, double shxz,
		      double shyx, double shyz,
		      double shzx, double shzy )
{
  double r[9], s[9], t[9];
  
  _3DScaleMatrix( s, scx, scy, scz );
  _3DRotationMatrix( r, v, angle );
  E_DMMatMul ( s, r, t, 3 );
  _3DShearMatrix( s, shxy, shxz, shyx, shyz, shzx, shzy );
  E_DMMatMul ( t, s, mat, 3 );
}



void _Random3DAffineMatrix( double *mat,
			    double *angle_interval,
			    double *scale_interval,
			    double *shear_interval )
{
  double a, n, v[3];
  double scx, scy, scz;
  double shxy, shxz;
  double shyx, shyz;
  double shzx, shzy;

  /* pick a random vector
   */
  do { 
    v[0] = (2.0 * _GetRandom() - 1.0 );
    v[1] = (2.0 * _GetRandom() - 1.0 );
    v[2] = (2.0 * _GetRandom() - 1.0 );
    n = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  } while ( n < 0.0001 );
  
  /* pick a random angle 
   */
  a = angle_interval[0] + (angle_interval[1]-angle_interval[0]) * _GetRandom();

  scx = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();
  scy = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();
  scz = scale_interval[0] + (scale_interval[1]-scale_interval[0]) * _GetRandom();

  shxy = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  shxz = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  shyx = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  shyz = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  shzx = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();
  shzy = shear_interval[0] + (shear_interval[1]-shear_interval[0]) * _GetRandom();

  _3DAffineMatrix( mat, v, a, scx, scy, scz,
		   shxy, shxz, shyx, shyz, shzx, shzy );
}












/************************************************************
 * 
 * 
 *
 ************************************************************/

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
static void RotationMatrixFromRotationVector( double *mat,
					      double *rot )
{
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

