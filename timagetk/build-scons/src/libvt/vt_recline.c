#include <math.h>

#include <vt_recline.h>



/*--- denominateur       ---*/
static double sd1 = 0.0, sd2 = 0.0, sd3 = 0.0, sd4 = 0.0;
/*--- numerateur positif ---*/
static double sp0 = 0.0, sp1 = 0.0, sp2 = 0.0, sp3 = 0.0;
/*--- numerateur negatif ---*/
static double sn0 = 0.0, sn1 = 0.0, sn2 = 0.0, sn3 = 0.0, sn4 = 0.0;
/*--- type de filtre en cours ---*/
static int static_type_filter = 0;
static int static_derivative  = VT_NODERIVATIVE;





int VT_InitRecursiveCoefficients( double x, int type_filter, int derivative )
{
    double ex, k1, k2;
    double a0, a1, c0, c1, omega0, omega1, b0, b1;
    double cos0, sin0, cos1, sin1;
    double sumA=0.0, sumC=0.0, aux;

    sd1 = sd2 = sd3 = sd4 = 0.0;
    sp0 = sp1 = sp2 = sp3 = 0.0;
    sn0 = sn1 = sn2 = sn3 = sn4 = 0.0;

    static_type_filter = 0;
    static_derivative  = VT_NODERIVATIVE;

    ex = k1 = k2 = 0.0;
    a0 = a1 = c0 = c1 = 0.0;
    b0 = b1 = omega0 = omega1 = 0.0;

    /*--- Selon le type de filtrage (filtres de Deriche,
          ou approximation de la gaussienne), x designe
	  soit alpha, soit sigma                         ---*/
    
    switch ( type_filter ) {
    case VT_RECGAUSSIAN_MARTA :

	switch ( derivative ) {
	case VT_DERIVATIVE_0 :
	    a0 =  0.6570033214 / x;
	    a1 =  1.978946687  / x;
	    c0 = -0.2580640608 / x;
	    c1 = -0.2391206463 / x;
	    omega0 = 0.6512453378;
	    omega1 = 2.05339943;
	    b0 = 1.906154352;
	    b1 = 1.881305409;
	    break;
	case VT_DERIVATIVE_1 :
	    a0 = -0.1726729496 / x;
	    a1 = -2.003565572  / x;
	    c0 =  0.1726730777 / x;
	    c1 =  0.4440126835 / x;
	    b0 = 1.560644213;
	    b1 = 1.594202256;
	    omega0 = 0.6995461735;
	    omega1 = 2.144671764;
	    break;
	case VT_DERIVATIVE_2 :
	    a0 = -0.7241334169 / x;
	    a1 =  1.688628765  / x;
	    c0 =  0.3251949838 / x;
	    c1 = -0.7211796018 / x;
	    b0 = 1.294951143;
	    b1 = 1.427007123;
	    omega0 = 0.7789803775;
	    omega1 = 2.233566862;
	    break;
	case VT_DERIVATIVE_3 :
	    a0 =  1.285774106  / x;
	    a1 = -0.2896378408 / x;
	    c0 = -1.28577129   / x;
	    c1 =  0.26249833   / x;
	    b0 = 1.01162886;
	    b1 = 1.273344739;
	    omega0 = 0.9474270928;
	    omega1 = 2.337607006;
	    break;
	case VT_DERIVATIVE_1_CONTOURS :
	default :
	    VT_Error("improper type of derivative","VT_InitRecursiveCoefficients");
	    return( 0 );
	}
	 
	omega0 /= x;   sin0 = sin( omega0 );   cos0 = cos( omega0 ); 
	omega1 /= x;   sin1 = sin( omega1 );   cos1 = cos( omega1 ); 
	b0 /= x;
	b1 /= x;

	sp0  = a0 + c0;
	sp1  = exp( -b1 ) * (c1 * sin1 - (c0 + 2 * a0) * cos1);
	sp1 += exp( -b0 ) * (a1 * sin0 - (2 * c0 + a0) * cos0);
	sp2  = 2.0 * exp( -b0 - b1 ) 
	  * ((a0 + c0) * cos1 * cos0 - cos1 * a1 * sin0 - cos0 * c1 * sin1);
	sp2 += c0 * exp( -2.0 * b0 ) + a0 * exp( -2.0 * b1 );
	sp3  = exp( -b1 - 2.0 * b0 ) * (c1 * sin1 - c0 * cos1);
        sp3 += exp( -b0 - 2.0 * b1 ) * (a1 * sin0 - a0 * cos0);

	sd1  = -2.0 * exp( -b1 ) * cos1 - 2.0 * exp( -b0 ) * cos0;
	sd2  = 4.0 * cos1 * cos0 * exp( -b0 - b1 ) 
	  + exp( -2.0 * b1 ) + exp( -2.0 * b0 );
	sd3 = -2.0 * cos0 * exp( -b0 - 2.0 * b1 ) 
	  - 2.0 * cos1 * exp( -b1 - 2.0 * b0 );
	sd4 = exp( -2.0 * b0 - 2.0 * b1 );

	switch ( derivative ) {
	case VT_DERIVATIVE_0 :
	case VT_DERIVATIVE_2 :
	    sn1 = sp1 - sd1 * sp0;
	    sn2 = sp2 - sd2 * sp0;
	    sn3 = sp3 - sd3 * sp0;
	    sn4 = -sd4 * sp0;
	    break;
	case VT_DERIVATIVE_1 :
	case VT_DERIVATIVE_1_CONTOURS :
	case VT_DERIVATIVE_3 :
	    sn1 = - sp1 + sd1 * sp0;
	    sn2 = - sp2 + sd2 * sp0;
	    sn3 = - sp3 + sd3 * sp0;
	    sn4 = sd4 * sp0;
	}

	static_type_filter = type_filter;
	static_derivative  = derivative;
	break;

    case VT_RECGAUSSIAN_DERICHE :

	if ( x < 0.1 ) {
	    VT_Error("improper value of coefficient","VT_InitRecursiveCoefficients");
	    return( 0 );
	}

	switch ( derivative ) {
	case VT_DERIVATIVE_0 :
	    a0     =  1.68;
	    omega0 =  0.6318;
	    a1     =  3.735;
	    b0     =  1.783;
	    c0     = -0.6803;
	    omega1 =  1.997;
	    c1     = -0.2598;
	    b1     =  1.723;
	    break;
	case VT_DERIVATIVE_1 :
	case VT_DERIVATIVE_1_CONTOURS :
	    a0     =  -0.6472;
	    omega0 =  0.6719;
	    a1     =  -4.531;
	    b0     =  1.527;
	    c0     =  0.6494;
	    omega1 =  2.072;
	    c1     =  0.9557;
	    b1     =  1.516;
	    break;
	case VT_DERIVATIVE_2 :
	    a0     = -1.331;
	    omega0 =  0.748;
	    a1     =  3.661;
	    b0     =  1.24;
	    c0     =  0.3225;
	    omega1 =  2.166;
	    c1     = -1.738;
	    b1     =  1.314;
	    break;
	case VT_DERIVATIVE_3 :
	default :
	    VT_Error("improper type of derivative","VT_InitRecursiveCoefficients");
	    return( 0 );
	}
	 
	omega0 /= x;   sin0 = sin( omega0 );   cos0 = cos( omega0 ); 
	omega1 /= x;   sin1 = sin( omega1 );   cos1 = cos( omega1 ); 
	b0 /= x;
	b1 /= x;

	/*--- normalisation ---*/
	switch ( derivative ) {
	case VT_DERIVATIVE_0 :
	    sumA  = 2.0 * a1 * exp( b0 ) * cos0 * cos0 - a0 * sin0 * exp( 2.0 * b0 );
	    sumA += a0 * sin0 - 2.0 * a1 * exp( b0 );
	    sumA /= ( 2.0 * cos0 * exp( b0 ) - exp( 2.0 * b0 ) - 1 ) * sin0;
	    sumC  = 2.0 * c1 * exp( b1 ) * cos1 * cos1 - c0 * sin1 * exp( 2.0 * b1 );
	    sumC += c0 * sin1 - 2.0 * c1 * exp( b1 );
	    sumC /= ( 2.0 * cos1 * exp( b1 ) - exp( 2.0 * b1 ) - 1 ) * sin1;
	    /*---
	    sumA  = a0 * ( 1.0 - exp( 2.0 * b0 ) );
	    sumA /= ( 2.0 * cos0 * exp( b0 ) - exp( 2.0 * b0 ) - 1 );
	    sumC  = c0 * ( 1.0 - exp( 2.0 * b1 ) );
	    sumC /= ( 2.0 * cos1 * exp( b1 ) - exp( 2.0 * b1 ) - 1 );
	    ---*/
	    break;
	case VT_DERIVATIVE_1 :
	    aux   = exp( 4.0 * b0 ) - 4.0 * cos0 * exp( 3.0 * b0 );
	    aux  += 2.0 * exp( 2.0 * b0 ) + 4.0 * cos0 * cos0 * exp( 2.0 * b0 );
	    aux  += 1.0 - 4.0 * cos0 * exp( b0 );
	    sumA  = a0 * cos0 - a1 * sin0 + a1 * sin0 * exp( 2.0 * b0 );
	    sumA += a0 * cos0 * exp( 2.0 * b0 ) - 2.0 * a0 * exp( b0 );
	    sumA *= exp( b0 ) / aux;
	    aux   = exp( 4.0 * b1 ) - 4.0 * cos1 * exp( 3.0 * b1 );
	    aux  += 2.0 * exp( 2.0 * b1 ) + 4.0 * cos1 * cos1 * exp( 2.0 * b1 );
	    aux  += 1.0 - 4.0 * cos1 * exp( b1 );
	    sumC  = c0 * cos1 - c1 * sin1 + c1 * sin1 * exp( 2.0 * b1 );
	    sumC += c0 * cos1 * exp( 2.0 * b1 ) - 2.0 * c0 * exp( b1 );
	    sumC *= exp( b1 ) / aux;
	    /*---
	    aux   = 4.0 * cos0 * exp( 3.0 * b0 ) + 4.0 * cos0 * exp( b0 ) - exp( 4.0 * b0 );
	    aux  -= (4.0 * cos0 * cos0 * exp( 2.0 * b0 ) + 2.0 * exp( 2.0 * b0 ) + 1);
	    aux  *= sin0;
	    sumA  = 2.0 * a1 * exp( b0 );
	    sumA *= ( 1.0 + cos0 * cos0 * exp( 2.0 * b0 ) - exp( 2.0 * b0 ) - cos0 * cos0 );
	    sumA /= aux;
	    aux   = 4.0 * cos1 * exp( 3.0 * b1 ) + 4.0 * cos1 * exp( b1 ) - exp( 4.0 * b1 );
	    aux  -= (4.0 * cos1 * cos1 * exp( 2.0 * b1 ) + 2.0 * exp( 2.0 * b1 ) + 1);
	    aux  *= sin1;
	    sumC  = 2.0 * c1 * exp( b1 );
	    sumC *= ( 1.0 + cos1 * cos1 * exp( 2.0 * b1 ) - exp( 2.0 * b1 ) - cos1 * cos1 );
	    sumC /= aux;
	    ---*/
	    /*--- on multiplie les sommes par 2 car on n'a calcule que des demi-sommes 
	          et on change le signe car la somme doit etre egale a -1              ---*/
	    sumA *= (-2.0);
	    sumC *= (-2.0);
	    break;
	case VT_DERIVATIVE_1_CONTOURS :
	    /*--- la somme de 1 a l'infini est egale a 1 : cela introduit
	          un petit biais (reponse un rien superieur a la hauteur du step).
		  Avec une somme de 0 a l'infini, c'est pire                       ---*/
	    sumA  = a1 * exp( b0 ) - a1 * cos0 * cos0 * exp( b0 );
	    sumA += a0 * cos0 * sin0 * exp( b0 ) - a0 * sin0;
	    sumA /= sin0 * ( 2.0 * cos0 * exp( b0 ) - exp( 2.0 * b0 ) - 1 );
	    sumC  = c1 * exp( b1 ) - c1 * cos1 * cos1 * exp( b1 );
	    sumC += c0 * cos1 * sin1 * exp( b1 ) - c0 * sin1;
	    sumC /= sin1 * ( 2.0 * cos1 * exp( b1 ) - exp( 2.0 * b1 ) - 1 );
	    break;
	case VT_DERIVATIVE_2 :
	    aux   = 12.0 * cos0 * exp( 3.0 * b0 ) - 3.0 * exp( 2.0 * b0 );
	    aux  += 8.0 * cos0 * cos0 * cos0 * exp( 3.0 * b0 ) - 12.0 * cos0 * cos0 * exp( 4.0 * b0 );
	    aux  -= 3.0 * exp( 4.0 * b0 );
	    aux  += 6.0 * cos0 * exp( 5.0 * b0 ) -  exp( 6.0 * b0 ) + 6.0 * cos0 * exp( b0 );
	    aux  -= ( 1.0 + 12.0 * cos0 * cos0 * exp( 2.0 * b0 ) );
	    sumA  = 4.0 * a0 * sin0 * exp( 3.0 * b0 ) + a1 * cos0 * cos0 * exp( 4.0 * b0 );
	    sumA -= ( 4.0 * a0 * sin0 * exp( b0 ) + 6.0 * a1 * cos0 * cos0 * exp( 2.0 * b0 ) );
	    sumA += 2.0 * a1 * cos0 * cos0 * cos0 * exp( b0 ) - 2.0 * a1 * cos0 * exp( b0 );
	    sumA += 2.0 * a1 * cos0 * cos0 * cos0 * exp( 3.0 * b0 ) - 2.0 * a1 * cos0 * exp( 3.0 * b0 );
	    sumA += a1 * cos0 * cos0 - a1 * exp( 4.0 * b0 );
	    sumA += 2.0 * a0 * sin0 * cos0 * cos0 * exp( b0 ) - 2.0 * a0 * sin0 * cos0 * cos0 * exp( 3.0 * b0 );
	    sumA -= ( a0 * sin0 * cos0 * exp( 4.0 * b0 ) + a1 );
	    sumA += 6.0 * a1 * exp( 2.0 * b0 ) + a0 * cos0 * sin0;
	    sumA *= 2.0 * exp( b0 ) / ( aux * sin0 );
	    aux   = 12.0 * cos1 * exp( 3.0 * b1 ) - 3.0 * exp( 2.0 * b1 );
	    aux  += 8.0 * cos1 * cos1 * cos1 * exp( 3.0 * b1 ) - 12.0 * cos1 * cos1 * exp( 4.0 * b1 );
	    aux  -= 3.0 * exp( 4.0 * b1 );
	    aux  += 6.0 * cos1 * exp( 5.0 * b1 ) -  exp( 6.0 * b1 ) + 6.0 * cos1 * exp( b1 );
	    aux  -= ( 1.0 + 12.0 * cos1 * cos1 * exp( 2.0 * b1 ) );
	    sumC  = 4.0 * c0 * sin1 * exp( 3.0 * b1 ) + c1 * cos1 * cos1 * exp( 4.0 * b1 );
	    sumC -= ( 4.0 * c0 * sin1 * exp( b1 ) + 6.0 * c1 * cos1 * cos1 * exp( 2.0 * b1 ) );
	    sumC += 2.0 * c1 * cos1 * cos1 * cos1 * exp( b1 ) - 2.0 * c1 * cos1 * exp( b1 );
	    sumC += 2.0 * c1 * cos1 * cos1 * cos1 * exp( 3.0 * b1 ) - 2.0 * c1 * cos1 * exp( 3.0 * b1 );
	    sumC += c1 * cos1 * cos1 - c1 * exp( 4.0 * b1 );
	    sumC += 2.0 * c0 * sin1 * cos1 * cos1 * exp( b1 ) - 2.0 * c0 * sin1 * cos1 * cos1 * exp( 3.0 * b1 );
	    sumC -= ( c0 * sin1 * cos1 * exp( 4.0 * b1 ) + c1 );
	    sumC += 6.0 * c1 * exp( 2.0 * b1 ) + c0 * cos1 * sin1;
	    sumC *= 2.0 * exp( b1 ) / ( aux * sin1 );
	    /*---
	    aux   = 12.0 * cos0 * cos0 * exp( 4.0 * b0 ) + 12.0 * cos0 * cos0 * exp( 2.0 * b0 );
	    aux  -= ( 8.0 * cos0 * cos0 * cos0 * exp( 3.0 * b0 ) + 6.0 * cos0 * exp( 5.0 * b0 ) );
	    aux  -= ( 12.0 * cos0 * exp( 3.0 * b0 ) + 6.0 * cos0 * exp( b0 ) );
	    aux  += exp( 6.0 * b0 ) + 3.0 * exp( 4.0 * b0 ) + 3.0 * exp( 2.0 * b0 ) + 1.0;
	    sumA  = cos0 - 2.0 * cos0 * cos0 * exp( 3.0 * b0 ) - 4.0 * exp( b0 );
	    sumA += 2.0 * cos0 * cos0 * exp( b0 ) - cos0 * exp( 4.0 * b0 ) + 4.0 * exp( 3.0 * b0 );
	    sumA *= ( -2.0 * a0 * exp( b0 ) ) / aux;
	    aux   = 12.0 * cos1 * cos1 * exp( 4.0 * b1 ) + 12.0 * cos1 * cos1 * exp( 2.0 * b1 );
	    aux  -= ( 8.0 * cos1 * cos1 * cos1 * exp( 3.0 * b1 ) + 6.0 * cos1 * exp( 5.0 * b1 ) );
	    aux  -= ( 12.0 * cos1 * exp( 3.0 * b1 ) + 6.0 * cos1 * exp( b1 ) );
	    aux  += exp( 6.0 * b1 ) + 3.0 * exp( 4.0 * b1 ) + 3.0 * exp( 2.0 * b1 ) + 1.0;
	    sumC  = cos1 - 2.0 * cos1 * cos1 * exp( 3.0 * b1 ) - 4.0 * exp( b1 );
	    sumC += 2.0 * cos1 * cos1 * exp( b1 ) - cos1 * exp( 4.0 * b1 ) + 4.0 * exp( 3.0 * b1 );
	    sumC *= ( -2.0 * a1 * exp( b1 ) ) / aux;
	    ---*/
	    /*--- on divise les sommes par 2 (la somme doit etre egale a 2) ---*/
	    sumA /= 2;
	    sumC /= 2;
        }
	a0 /= ( sumA + sumC );
	a1 /= ( sumA + sumC );
	c0 /= ( sumA + sumC );
	c1 /= ( sumA + sumC );

	/*--- coefficients du calcul recursif ---*/
	sp0  = a0 + c0;
	sp1  = exp( -b1 ) * (c1 * sin1 - (c0 + 2 * a0) * cos1);
	sp1 += exp( -b0 ) * (a1 * sin0 - (2 * c0 + a0) * cos0);
	sp2  = 2.0 * exp( -b0 - b1 ) * ((a0 + c0) * cos1 * cos0 - cos1 * a1 * sin0 - cos0 * c1 * sin1);
	sp2 += c0 * exp( -2.0 * b0 ) + a0 * exp( -2.0 * b1 );
	sp3  = exp( -b1 - 2.0 * b0 ) * (c1 * sin1 - c0 * cos1);
        sp3 += exp( -b0 - 2.0 * b1 ) * (a1 * sin0 - a0 * cos0);

	sd1  = -2.0 * exp( -b1 ) * cos1 - 2.0 * exp( -b0 ) * cos0;
	sd2  = 4.0 * cos1 * cos0 * exp( -b0 - b1 ) + exp( -2.0 * b1 ) + exp( -2.0 * b0 );
	sd3 = -2.0 * cos0 * exp( -b0 - 2.0 * b1 ) - 2.0 * cos1 * exp( -b1 - 2.0 * b0 );
	sd4 = exp( -2.0 * b0 - 2.0 * b1 );

	switch ( derivative ) {
	case VT_DERIVATIVE_0 :
	case VT_DERIVATIVE_2 :
	    sn1 = sp1 - sd1 * sp0;
	    sn2 = sp2 - sd2 * sp0;
	    sn3 = sp3 - sd3 * sp0;
	    sn4 = -sd4 * sp0;
	    break;
	case VT_DERIVATIVE_1 :
	case VT_DERIVATIVE_1_CONTOURS :
	case VT_DERIVATIVE_3 :
	    sn1 = - sp1 + sd1 * sp0;
	    sn2 = - sp2 + sd2 * sp0;
	    sn3 = - sp3 + sd3 * sp0;
	    sn4 = sd4 * sp0;
	}

	static_type_filter = type_filter;
	static_derivative  = derivative;
	break;

    case VT_RECFILTERS_DERICHE :

	if ( (x < 0.1) || (x > 1.9) ) {
	    VT_Error("improper value of coefficient","VT_InitRecursiveCoefficients");
	    return( 0 );
	}

	ex = exp( (-x) );

	switch ( derivative ) {
	case VT_DERIVATIVE_0 :
	    sp0 = (1.0 - ex) * (1.0 - ex) / (1.0 + 2.0 * x * ex - ex * ex);
	    sp1 = sp0 * (x - 1.0) * ex;
	    sn1 = sp0 * (x + 1.0) * ex;
	    sn2 = (- sp0) * ex * ex;
	    sd1 = (- 2.0) * ex;
	    sd2 = ex * ex;
	    break;
	case VT_DERIVATIVE_1 :
	    sp1 = - (1.0 - ex) * (1.0 - ex) * (1.0 - ex) / (2.0 * (1.0 + ex));
	    sn1 = (- sp1);
	    sd1 = (- 2.0) * ex;
	    sd2 = ex * ex;	    
	    break;
	case VT_DERIVATIVE_1_CONTOURS :
	    sp1 = - (1.0 - ex) * (1.0 - ex);
	    sn1 = (- sp1);
	    sd1 = (- 2.0) * ex;
	    sd2 = ex * ex;	    
	    break;
	case VT_DERIVATIVE_2 :
	    k1 = (- 2.0) * (1.0 - ex) * (1.0 - ex) * (1.0 - ex);
	    k1 /= (1.0 + ex) * (1.0 + ex) * (1.0 + ex);
	    k2 = (1.0 - ex * ex) / (2.0 * ex);
	    sp0 = k1;
	    sp1 = (- k1) * (1.0 + k2) * ex;
	    sn1 = k1 * (1.0 - k2) * ex;
	    sn2 = (- k1) * ex * ex;
	    sd1 = (- 2.0) * ex;
	    sd2 = ex * ex;
	    break;
	case VT_DERIVATIVE_3 :
	    k1 = (1.0 + x) * ex + (x - 1.0);
	    k2 = (1.0 - ex) / k1;
	    k1 *= (1.0 - ex) * (1.0 - ex) * (1.0 - ex) * (1.0 - ex);
	    k1 /= 2.0 * x * x * ex * ex;
	    k1 /= ex + 1.0;
	    sp0 = k1 * x * (k2 + 1.0);
	    sp1 = (- k1) * x * (1.0 + k2 + k2*x) * ex;
	    sn0 = (- sp0);
	    sn1 = (- sp1);
	    sd1 = (- 2.0) * ex;
	    sd2 = ex * ex;
	    break;
	default :
	    VT_Error("improper type of derivative","VT_InitRecursiveCoefficients");
	    return( 0 );
	}
	static_type_filter = type_filter;
	static_derivative  = derivative;
	break;

    default :
	VT_Error("improper type of filter","VT_InitRecursiveCoefficients");
	return( 0 );
    }
	
    return( 1 );
}





int VT_RecFilterOnLine( double *in, double *out, double *work1, double *work2, int dim )
{
    register double rp0, rp1, rp2, rp3;
    register double rd1, rd2, rd3, rd4;
    register double rn0, rn1, rn2, rn3, rn4;
    register int i, j, k;
    
    if ( static_type_filter == 0 ) {
	VT_Error("unknown type of recursive filter","VT_RecFilterOnLine");
	return( 0 );
    }
    if ( static_derivative == VT_NODERIVATIVE ) {
	VT_Error("unknown type of derivative","VT_RecFilterOnLine");
	return( 0 );
    }

    rd1 = rd2 = rd3 = rd4 = 0.0;
    rp0 = rp1 = rp2 = rp3 = 0.0;
    rn0 = rn1 = rn2 = rn3 = rn4 = 0.0;

    switch( static_type_filter ) {
    case VT_RECGAUSSIAN_DERICHE :
    case VT_RECGAUSSIAN_MARTA :
	/*--- filtrage generique d'ordre 4 ---*/
	rp0 = sp0;   rp1 = sp1;   rp2 = sp2;   rp3 = sp3;
	rd1 = sd1;   rd2 = sd2;   rd3 = sd3;   rd4 = sd4;
	rn1 = sn1;   rn2 = sn2;   rn3 = sn3;   rn4 = sn4;

        /*--- calcul de y+ ---*/
	*(work1)   = rp0 * *(in);
	*(work1+1) = rp0 * *(in+1) + rp1 * *(in) -
		     rd1 * *(work1);   
	*(work1+2) = rp0 * *(in+2) + rp1 * *(in+1) + rp2 * *(in) -
	             rd1 * *(work1+1) - rd2 * *(work1);
	*(work1+3) = rp0 * *(in+3) + rp1 * *(in+2) + rp2 * *(in+1) + rp3 * *(in) -
		     rd1 * *(work1+2) - rd2 * *(work1+1) - rd3 * *(work1);
	for (i=4; i<dim; i++)
            *(work1+i) = rp0 * *(in+i) + rp1 * *(in+i-1) +
			 rp2 * *(in+i-2) + rp3 * *(in+i-3) -
			 rd1 * *(work1+i-1) - rd2 * *(work1+i-2) -
			 rd3 * *(work1+i-3) - rd4 * *(work1+i-4);
        /*--- calcul de y- ---*/
        *(work2+dim-1) = 0;
	*(work2+dim-2) = rn1 * *(in+dim-1);
	*(work2+dim-3) = rn1 * *(in+dim-2) + rn2 * *(in+dim-1) - rd1 * *(work2+dim-2);
	*(work2+dim-4) = rn1 * *(in+dim-3) + rn2 * *(in+dim-2) + rn3 * *(in+dim-1) -
		         rd1 * *(work2+dim-3) - rd2 * *(work2+dim-2);
	for (i=dim-5; i>=0; i--)
	    *(work2+i) = rn1 * *(in+i+1) + rn2 * *(in+i+2) + rn3 * *(in+i+3) +
                         rn4 * *(in+i+4) - rd1 * *(work2+i+1) - rd2 * *(work2+i+2) -
			 rd3 * *(work2+i+3) - rd4 * *(work2+i+4);
	/*--- calcul final ---*/
	for (i=0 ; i<dim ; i++)
		*(out+i) = *(work1+i) + *(work2+i);

	break;
    case VT_RECFILTERS_DERICHE :
	switch( static_derivative ) {
	case VT_DERIVATIVE_0 :
	    rp0 = sp0;   rp1 = sp1;
	    rd1 = sd1;   rd2 = sd2;
	    rn1 = sn1;   rn2 = sn2;

	    /*--- calcul de y+ ---*/
	    *(work1)   = rp0 * *(in);
	    *(work1+1) = rp0 * *(in+1) + rp1 * *(in) - rd1 * *(work1);     
	    for (i=2, j=1, k=0;  i<dim; i++, j++, k++)
		*(work1+i) = rp0 * *(in+i) + rp1 * *(in+j)
		    - rd1 * *(work1+j) - rd2 * *(work1+k);

	    /*--- calcul de y- ---*/
	    *(work2+dim-1) = 0.0;
	    *(work2+dim-2) = rn1 * *(in+dim-1);
	    for (i=dim-3, j=dim-2, k=dim-1; i>=0; i--, j--, k--)
		*(work2+i) = rn1 * *(in+j) + rn2 * *(in+k)
		    - rd1 * *(work2+j) - rd2 * *(work2+k);

	    /*--- calcul final ---*/
	    for (i=0 ; i<dim ; i++)
		*(out+i) = *(work1+i) + *(work2+i);

	    break;

	case VT_DERIVATIVE_1 :
	case VT_DERIVATIVE_1_CONTOURS :
	    rp1 = sp1;
	    rn1 = sn1;
	    rd1 = sd1;   rd2 = sd2;

	    /*--- calcul de y+ ---*/
	    *(work1)   = 0.0;
	    *(work1+1) = rp1 * *(in);     
	    for (i=2, j=1, k=0;  i<dim; i++, j++, k++)
		*(work1+i) = rp1 * *(in+j)
		    - rd1 * *(work1+j) - rd2 * *(work1+k);
	    
	    /*--- calcul de y- ---*/
	    *(work2+dim-1) = 0.0;
	    *(work2+dim-2) = rn1 * *(in+dim-1);
	    for (i=dim-3, j=dim-2, k=dim-1; i>=0; i--, j--, k--)
		*(work2+i) = rn1 * *(in+j)
		    - rd1 * *(work2+j) - rd2 * *(work2+k);

	    /*--- calcul final ---*/
	    for (i=0 ; i<dim ; i++)
		*(out+i) = *(work1+i) + *(work2+i);

	    break;

	case VT_DERIVATIVE_2 :
	    rp0 = sp0;   rp1 = sp1;
	    rd1 = sd1;   rd2 = sd2;
	    rn1 = sn1;   rn2 = sn2;

	    /*--- calcul de y+ ---*/
	    *(work1)   = rp0 * *(in);
	    *(work1+1) = rp0 * *(in+1) + rp1 * *(in) - rd1 * *(work1);     
	    for (i=2, j=1, k=0;  i<dim; i++, j++, k++)
		*(work1+i) = rp0 * *(in+i) + rp1 * *(in+j)
		    - rd1 * *(work1+j) - rd2 * *(work1+k);

	    /*--- calcul de y- ---*/
	    *(work2+dim-1) = 0.0;
	    *(work2+dim-2) = rn1 * *(in+dim-1);
	    for (i=dim-3, j=dim-2, k=dim-1; i>=0; i--, j--, k--)
		*(work2+i) = rn1 * *(in+j) + rn2 * *(in+k)
		    - rd1 * *(work2+j) - rd2 * *(work2+k);

	    /*--- calcul final ---*/
	    for (i=0 ; i<dim ; i++)
		*(out+i) = *(work1+i) + *(work2+i);

	    break;

	case VT_DERIVATIVE_3 :
	    rp0 = sp0;   rp1 = sp1;
	    rd1 = sd1;   rd2 = sd2;
	    rn0 = sn0;   rn1 = sn1;

	    /*--- calcul de y+ ---*/
	    *(work1)   = rp0 * *(in);
	    *(work1+1) = rp0 * *(in+1) + rp1 * *(in) - rd1 * *(work1);     
	    for (i=2, j=1, k=0;  i<dim; i++, j++, k++)
		*(work1+i) = rp0 * *(in+i) + rp1 * *(in+j)
		    - rd1 * *(work1+j) - rd2 * *(work1+k);

	    /*--- calcul de y- ---*/
	    *(work2+dim-1) = rn0 * *(in+dim-1);
	    *(work2+dim-2) = rn0 * *(in+dim-2) + rn1 * *(in+dim-1) - rd1 * *(work2+dim-1);
	    for (i=dim-3, j=dim-2, k=dim-1; i>=0; i--, j--, k--)
		*(work2+i) = rn0 * *(in+i) + rn1 * *(in+j)
		    - rd1 * *(work2+j) - rd2 * *(work2+k);

	    /*--- calcul final ---*/
	    for (i=0 ; i<dim ; i++)
		*(out+i) = *(work1+i) + *(work2+i);
	}
	break;
    default :
	VT_Error("unknown type of recursive filter","VT_RecFilterOnLine");
	return( 0 );
 
   }
    return( 1 );
}
