
#include <vt_cc.h>

/*--- relations de 26-connexite dans le 26-voisinage ---*/
/*--- on ignore le point central                  ---*/
static int Relation26_hole[27][27] = {
        {  6,  1,  3,  9,              4, 10, 12,                                        },
        { 10,  0,  2,  4, 10,          3,  5,  9, 11,                                 12, 14 },
        {  6,  1,  5, 11,              4, 10, 14,                                        },
        { 10,  0,  4,  6, 12,          1,  7,  9, 15,                                 10, 16 },
        { 16,  1,  3,  5,  7,          0,  2,  6,  8, 10, 12, 14, 16,                  9, 11, 15, 17 },
        { 10,  2,  4,  8, 14,          1,  7, 11, 17,                                 10, 16 },
        {  6,  3,  7, 15,              4, 12, 16,                                        },
        { 10,  4,  6,  8, 16,          3,  5, 15, 17,                                 12, 14 },
        {  6,  5,  7, 17,              4, 14, 16,                                        },
        { 10,  0, 10, 12, 18,          1,  3, 19, 21,                                  4, 22 },
        { 16,  1,  9, 11, 19,          0,  2,  4, 12, 14, 18, 20, 22,                  3,  5, 21, 23 },
        { 10,  2, 10, 14, 20,          1,  5, 19, 23,                                  4, 22 },
        { 16,  3,  9, 15, 21,          0,  4,  6, 10, 16, 18, 22, 24,                  1,  7, 19, 25 },
        {  0  },
        { 16,  5, 11, 17, 23,          2,  4,  8, 10, 16, 20, 22, 26,                  1,  7, 19, 25 },
        { 10,  6, 12, 16, 24,          3,  7, 21, 25,                                  4, 22 },
        { 16,  7, 15, 17, 25,          4,  6,  8, 12, 14, 22, 24, 26,                  3,  5, 21, 23 },
        { 10,  8, 14, 16, 26,          5,  7, 23, 25,                                  4, 22 },
        {  6,  9, 19, 21,             10, 12, 22,                                        },
        { 10, 10, 18, 20, 22,          9, 11, 21, 23,                                 12, 14 },
        {  6, 11, 19, 23,             10, 14, 22,                                        },
        { 10, 12, 18, 22, 24,          9, 15, 19, 25,                                 10, 16 },
        { 16, 19, 21, 23, 25,         10, 12, 14, 16, 18, 20, 24, 26,                  9, 11, 15, 17 },
        { 10, 14, 20, 22, 26,         11, 17, 19, 25,                                 10, 16 },
        {  6, 15, 21, 25,             12, 16, 22,                                        },
        { 10, 16, 22, 24, 26,         15, 17, 21, 23,                                 12, 14 },
        {  6, 17, 23, 25,             14, 16, 22,                                        }
};

/*--- relations de 26-connexite dans le 26-voisinage ---*/
/*--- on compte le point central                     ---*/
static int Relation26_all[27][27] = {
        {  7,  1,  3,  9,              4, 10, 12,                                     13 },
        { 11,  0,  2,  4, 10,          3,  5,  9, 11, 13,                             12, 14 },
        {  7,  1,  5, 11,              4, 10, 14,                                     13 },
        { 11,  0,  4,  6, 12,          1,  7,  9, 13, 15,                             10, 16 },
        { 17,  1,  3,  5,  7, 13,      0,  2,  6,  8, 10, 12, 14, 16,                  9, 11, 15, 17 },
        { 11,  2,  4,  8, 14,          1,  7, 11, 13, 17,                             10, 16 },
        {  7,  3,  7, 15,              4, 12, 16,                                     13 },
        { 11,  4,  6,  8, 16,          3,  5, 13, 15, 17,                             12, 14 },
        {  7,  5,  7, 17,              4, 14, 16,                                     13 },
        { 11,  0, 10, 12, 18,          1,  3, 13, 19, 21,                              4, 22 },
        { 17,  1,  9, 11, 13, 19,      0,  2,  4, 12, 14, 18, 20, 22,                  3,  5, 21, 23 },
        { 11,  2, 10, 14, 20,          1,  5, 13, 19, 23,                              4, 22 },
        { 17,  3,  9, 13, 15, 21,      0,  4,  6, 10, 16, 18, 22, 24,                  1,  7, 19, 25 },
        { 26,  4, 10, 12, 14, 16, 22,  1,  3,  5,  7,  9, 11, 15, 17, 19, 21, 23, 25,  0,  2,  6,  8, 18, 20, 24, 26 },
        { 17,  5, 11, 13, 17, 23,      2,  4,  8, 10, 16, 20, 22, 26,                  1,  7, 19, 25 },
        { 11,  6, 12, 16, 24,          3,  7, 13, 21, 25,                              4, 22 },
        { 17,  7, 13, 15, 17, 25,      4,  6,  8, 12, 14, 22, 24, 26,                  3,  5, 21, 23 },
        { 11,  8, 14, 16, 26,          5,  7, 13, 23, 25,                              4, 22 },
        {  7,  9, 19, 21,             10, 12, 22,                                     13 },
        { 11, 10, 18, 20, 22,          9, 11, 13, 21, 23,                             12, 14 },
        {  7, 11, 19, 23,             10, 14, 22,                                     13 },
        { 11, 12, 18, 22, 24,          9, 13, 15, 19, 25,                             10, 16 },
        { 17, 13, 19, 21, 23, 25,     10, 12, 14, 16, 18, 20, 24, 26,                  9, 11, 15, 17 },
        { 11, 14, 20, 22, 26,         11, 13, 17, 19, 25,                             10, 16 },
        {  7, 15, 21, 25,             12, 16, 22,                                     13 },
        { 11, 16, 22, 24, 26,         13, 15, 17, 21, 23,                             12, 14 },
        {  7, 17, 23, 25,             14, 16, 22,                                     13 }
};

/*--- relations de 6-connexite dans le 18-voisinage ---*/
/*--- considere comme sous-ensemble du 26-voisinage ---*/
/*--- on ignore le point central                    ---*/
static int Relation06_hole[27][27] = {
        {  0 },
        {  2,  4, 10 },
        {  0 },
        {  2,  4, 12 },
        {  4,  1,  3,  5,  7 },
        {  2,  4, 14 },
        {  0 },
        {  2,  4, 16 },
        {  0 },
        {  2, 10, 12 },
        {  4,  1,  9, 11, 19 },
        {  2, 10, 14 },
        {  4,  3,  9, 15, 21 },
        {  0  },
        {  4,  5, 11, 17, 23 },
        {  2, 12, 16 },
        {  4,  7, 15, 17, 25 },
        {  2, 14, 16 },
        {  0 },
        {  2, 10, 22 },
        {  0 },
        {  2, 12, 22 },
        {  4, 19, 21, 23, 25 },
        {  2, 14, 22 },
        {  0 },
        {  2, 16, 22 },
        {  0 }
};

/*--- relations de 6-connexite dans le 18-voisinage ---*/
/*--- considere comme sous-ensemble du 26-voisinage ---*/
/*--- on compte le point central                    ---*/
static int Relation06_all[27][27] = {
        {  0 },
        {  2,  4, 10 },
        {  0 },
        {  2,  4, 12 },
        {  5,  1,  3,  5,  7, 13 },
        {  2,  4, 14 },
        {  0 },
        {  2,  4, 16 },
        {  0 },
        {  2, 10, 12 },
        {  5,  1,  9, 11, 13, 19 },
        {  2, 10, 14 },
        {  5,  3,  9, 13, 15, 21 },
        {  6,  4, 10, 12, 14, 16, 22  },
        {  5,  5, 11, 13, 17, 23 },
        {  2, 12, 16 },
        {  5,  7, 13, 15, 17, 25 },
        {  2, 14, 16 },
        {  0 },
        {  2, 10, 22 },
        {  0 },
        {  2, 12, 22 },
        {  5, 13, 19, 21, 23, 25 },
        {  2, 14, 22 },
        {  0 },
        {  2, 16, 22 },
        {  0 }
};

/*------- Definition des fonctions statiques ----------*/
static void _VT_MarqueVoisins_1( int v[27], int m[27], int R[27][27], int i, int nb );
static void _VT_MarqueVoisins_2( int v[27], int m[27], int R[27][27], int i, int nb );





/* Compte les 26-composantes connexes dans un 26-voisinage.

   Les points consideres sont ceux dont la valeur est 
   differente de 0. 

RETURN
   Retourne le nombre de composantes connexes.

*/

int _VT_26CCin27V( int voisins[27] /* neighborhood to be examinated */ )
{
	int marque[27];
	register int i, nbCC;

	/*--- initialisations ---*/
	for (i=0; i<27; i++) marque[i] = 0;
	nbCC = 0;

	/*--- c'est parti ---*/
	for (i=0; i<27; i++) {
		/*   si c'est un point non marque                   */
		/*   on commence une nouvelle composante connexes   */
		/****************************************************/
		if ( ( voisins[i] != 0 ) && ( marque[i] == 0 ) ) {
			nbCC ++;
			_VT_MarqueVoisins_1( voisins, marque, Relation26_all, i, nbCC );
		}
	}
	return( nbCC );
}





/* Compte les 26-composantes connexes dans un 26-voisinage.

   Les points consideres sont ceux dont la valeur est 
   differente de 0. Le point central n'est pas compte
   (il est considere comme appartenant au fond).

RETURN
   Retourne le nombre de composantes connexes.

*/

int _VT_26CCin26V( int voisins[27] /* neighborhood to be examinated */ )
{
	int marque[27];
	register int i, nbCC;

	/*--- initialisations ---*/
	for (i=0; i<27; i++) marque[i] = 0;
	/*--- on marque le point central ---*/
	marque[13] = 1;
	nbCC = 0;

	/*--- c'est parti ---*/
	for (i=0; i<27; i++) {
		/*   si c'est un point non marque                   */
		/*   on commence une nouvelle composante connexes   */
		/****************************************************/
		if ( ( voisins[i] != 0 ) && ( marque[i] == 0 ) ) {
			nbCC ++;
			_VT_MarqueVoisins_1( voisins, marque, Relation26_hole, i, nbCC );
		}
	}
	return(nbCC);
}





/* Verifie qu'il n'y a qu'une 26-composante connexe dans le 26-voisinage.

   Les points consideres sont ceux dont la valeur est 
   differente de 0. Le point central n'est pas compte
   (il est considere comme appartenant au fond).

RETURN
   Retourne 1 s'il n'y a qu'une composante connexe, 0 sinon.

*/

int _VT_26CCin26VisOne( int voisins[27] /* neighborhood to be examinated */ )
{
	int marque[27];
	register int i, nbCC;

	/*--- initialisations ---*/
	for (i=0; i<27; i++) marque[i] = 0;
	/*--- on marque le point central ---*/
	marque[13] = 1;
	nbCC = 0;

	/*--- c'est parti ---*/
	for (i=0; i<27; i++) {
		/*   si c'est un point non marque                   */
		/*   on commence une nouvelle composante connexes   */
		/****************************************************/
		if ( ( voisins[i] != 0 ) && ( marque[i] == 0 ) ) {
			nbCC ++;
			if ( nbCC > 1 ) return( 0 );
			_VT_MarqueVoisins_1( voisins, marque, Relation26_hole, i, nbCC );
		}
	}
	if ( nbCC < 1 ) return( 0 );
	return( 1 );
}





/* Compte les 6-composantes connexes dans un 18-voisinage.

   Les points consideres sont ceux dont la valeur est 
   egale a 0. 
   On ne compte que les 6-composantes qui sont 
   6-adjacentes au point central.

RETURN
   Retourne le nombre de composantes connexes.

*/

/*   les voisins sont inverses   */
/*   d'ou la negation            */
/*********************************/
int _VT_06CCin19V( int voisins[27] /* neighborhood to be examinated */ )
{
	int marque[27];
	register int ind, i, nbCC;

	/*--- initialisations ---*/
	for (i=0; i<27; i++) marque[i] = 0;
	nbCC = 0;
	ind = 4;

	/*--- c'est parti ---*/
	for (i=0; i<6; i++) {
		/*   si c'est un point non marque                   */
		/*   on commence une nouvelle composante connexes   */
		/****************************************************/
		if ( ( voisins[ind] == 0 ) && ( marque[ind] == 0 ) ) {
			nbCC ++;
			_VT_MarqueVoisins_2( voisins, marque, Relation06_all, ind, nbCC );
		}
		switch ( ind ) {
		case 4 :
			ind = 10; break;
		case 10 :
			ind = 12; break;
		case 12 :
			ind = 14; break;
		case 14 :
			ind = 16; break;
		case 16 :
			ind = 22; break;	
		case 22 :
			ind = 0;  break;
		}
	} 
	
	return( nbCC );
}





/* Compte les 6-composantes connexes dans un 18-voisinage.

   Les points consideres sont ceux dont la valeur est 
   egale a 0. Le point central n'est pas compte
   (il est considere comme appartenant a l'objet).
   On ne compte que les 6-composantes qui sont 
   6-adjacentes au point central.

RETURN
   Retourne le nombre de composantes connexes.

*/

int _VT_06CCin18V( int voisins[27] /* neighborhood to be examinated */ )
{
	int marque[27];
	register int ind, i, nbCC;

	/*--- initialisations ---*/
	for (i=0; i<27; i++) marque[i] = 0;
	/*--- on marque le point central ---*/
	marque[13] = 1;
	nbCC = 0;
	ind = 4;

	/*--- c'est parti ---*/
	for (i=0; i<6; i++) {
		/*   si c'est un point non marque                   */
		/*   on commence une nouvelle composante connexes   */
		/****************************************************/
		if ( ( voisins[ind] == 0 ) && ( marque[ind] == 0 ) ) {
			nbCC ++;
			_VT_MarqueVoisins_2( voisins, marque, Relation06_hole, ind, nbCC );
		}
		switch ( ind ) {
		case 4 :
			ind = 10; break;
		case 10 :
			ind = 12; break;
		case 12 :
			ind = 14; break;
		case 14 :
			ind = 16; break;
		case 16 :
			ind = 22; break;	
		case 22 :
			ind = 0;  break;
		}
	} 
	
	return( nbCC );
}





/* Verifie qu'il n'y a qu'une 6-composante connexe dans le 18-voisinage.

   Les points consideres sont ceux dont la valeur est 
   egale a 0. Le point central n'est pas compte
   (il est considere comme appartenant a l'objet).

RETURN
   Retourne 1 s'il n'y a qu'une composante connexe, 0 sinon.

*/

int _VT_06CCin18VisOne( int voisins[27] /* neighborhood to be examinated */ )
{
	int marque[27];
	register int ind, i, nbCC;

	/*--- initialisations ---*/
	for (i=0; i<27; i++) marque[i] = 0;
	/*--- on marque le point central ---*/
	marque[13] = 1;
	nbCC = 0;
	ind = 4;

	/*--- c'est parti ---*/
	for (i=0; i<6; i++) {
		/*   si c'est un point non marque                   */
		/*   on commence une nouvelle composante connexes   */
		/****************************************************/
		if ( ( voisins[ind] == 0 ) && ( marque[ind] == 0 ) ) {
			nbCC ++;
			if ( nbCC > 1 ) return( 0 );
			_VT_MarqueVoisins_2( voisins, marque, Relation06_hole, ind, nbCC );
		}
		switch ( ind ) {
		case 4 :
			ind = 10; break;
		case 10 :
			ind = 12; break;
		case 12 :
			ind = 14; break;
		case 14 :
			ind = 16; break;
		case 16 :
			ind = 22; break;	
		case 22 :
			ind = 0;  break;
		}
	} 
	if ( nbCC < 1 ) return( 0 );
	return( 1 );
}





static void _VT_MarqueVoisins_1( int v[27], 
			      int m[27],
			      int R[27][27],
			      int i,
			      int nb )
{
	/*--- pour marquer les points dont la valeur est non-nulle ---*/
	/*------------------------------------------------------------*/
	register int j;

	m[i] = nb;
	for (j = 1; j <= R[i][0]; j++ )
		if ( ( v[R[i][j]] != 0 ) && ( m[R[i][j]] == 0 ) )
			_VT_MarqueVoisins_1( v, m, R, R[i][j], nb );
}



			

static void _VT_MarqueVoisins_2( int v[27], 
			      int m[27],
			      int R[27][27],
			      int i,
			      int nb )
{
	/*--- pour marquer les points dont la valeur est nulle ---*/
	/*--------------------------------------------------------*/
	register int j;

	m[i] = nb;
	for (j = 1; j <= R[i][0]; j++ )
		if ( ( v[R[i][j]] == 0 ) && ( m[R[i][j]] == 0 ) )
			_VT_MarqueVoisins_2( v, m, R, R[i][j], nb );
}
