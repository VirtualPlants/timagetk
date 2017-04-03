
#include <vt_gb.h>





int _VT_GB_IsSimple( int vois[3][3][3], int *up, int *no, int *ea, int *bo, int *so, int *we )
{
    int i, j, npt;

    /*--- si la condition 2a n'est pas verifiee ---*/
    /*--- point + up appartient au fond         ---*/
    if ( vois[1+up[0]][1+up[1]][1+up[2]] != 0 ) return( 0 );
    
    /*--- si la condition 2b n'est pas verifiee ---*/
    /*--- point + bo appartient a l'objet       ---*/
    if ( vois[1+bo[0]][1+bo[1]][1+bo[2]] == 0 ) return( 0 );

    /*--- si la condition 2c n'est pas verifiee ---*/
    if ( vois[1+no[0]][1+no[1]][1+no[2]] == 0 )
        if ( vois[1+no[0]+up[0]][1+no[1]+up[1]][1+no[2]+up[2]] != 0 ) 
            return( 0 );
    if ( vois[1+ea[0]][1+ea[1]][1+ea[2]] == 0 )
        if ( vois[1+ea[0]+up[0]][1+ea[1]+up[1]][1+ea[2]+up[2]] != 0 ) 
            return( 0 );
    if ( vois[1+so[0]][1+so[1]][1+so[2]] == 0 )
        if ( vois[1+so[0]+up[0]][1+so[1]+up[1]][1+so[2]+up[2]] != 0 )
            return( 0 );
    if ( vois[1+we[0]][1+we[1]][1+we[2]] == 0 )
        if ( vois[1+we[0]+up[0]][1+we[1]+up[1]][1+we[2]+up[2]] != 0 )
            return( 0 );
    
    /*--- si la condition 2d n'est pas verifiee ---*/
    if ( ( vois[1+no[0]][1+no[1]][1+no[2]] == 0 ) && 
         ( vois[1+ea[0]][1+ea[1]][1+ea[2]] == 0 ) &&
         ( vois[1+no[0]+ea[0]][1+no[1]+ea[1]][1+no[2]+ea[2]] == 0 ) )
        if ( vois[1+up[0]+no[0]+ea[0]][1+up[1]+no[1]+ea[1]][1+up[2]+no[2]+ea[2]] != 0 )
            return( 0 );
    if ( ( vois[1+ea[0]][1+ea[1]][1+ea[2]] == 0 ) &&
         ( vois[1+so[0]][1+so[1]][1+so[2]] == 0 ) &&
         ( vois[1+ea[0]+so[0]][1+ea[1]+so[1]][1+ea[2]+so[2]] == 0 ) )
        if ( vois[1+up[0]+ea[0]+so[0]][1+up[1]+ea[1]+so[1]][1+up[2]+ea[2]+so[2]] != 0 )
            return( 0 );
    if ( ( vois[1+so[0]][1+so[1]][1+so[2]] == 0 ) &&
         ( vois[1+we[0]][1+we[1]][1+we[2]] == 0 ) &&
         ( vois[1+so[0]+we[0]][1+so[1]+we[1]][1+so[2]+we[2]] == 0 ) )
        if ( vois[1+up[0]+so[0]+we[0]][1+up[1]+so[1]+we[1]][1+up[2]+so[2]+we[2]] != 0 )
            return( 0 );
    if ( ( vois[1+we[0]][1+we[1]][1+we[2]] == 0 ) &&
         ( vois[1+no[0]][1+no[1]][1+no[2]] == 0 ) &&
         ( vois[1+we[0]+no[0]][1+we[1]+no[1]][1+we[2]+no[2]] == 0 ) )
        if ( vois[1+up[0]+we[0]+no[0]][1+up[1]+we[1]+no[1]][1+up[2]+we[2]+no[2]] != 0 )
            return( 0 );
    
    /*--- si on est arrive ici, c'est un point effacable ---*/
    
    /*--- on verifie maintenant que ce n'est pas un point bord de surface ---*/
    /*--- condition geometrique d'arret ---*/
    /*--- nombre de points dans le voisinage ---*/
    npt = 0;
    for ( i = 0; i < 3; i++ )
    for ( j = 0; j < 3; j++ ) {
        if ( vois[i][j][0] != 0 ) npt ++;
        if ( vois[i][j][2] != 0 ) npt ++;
    }
    for ( i = 0; i < 3; i++ ) {
        if ( vois[i][0][1] != 0 ) npt ++;
        if ( vois[i][2][1] != 0 ) npt ++;
    }
    if ( vois[0][1][1] != 0 ) npt ++;
    if ( vois[2][1][1] != 0 ) npt ++;
    
    if ( npt < 4 )  return( 0 );
    if ( npt >= 8 ) return( 1 );

    /*--- le nombre de points est entre 4 et 7 ---*/

    if ( ( vois[1+up[0]][1+up[1]][1+up[2]] != 0 ) && 
         ( vois[1+no[0]][1+no[1]][1+no[2]] != 0 ) &&
         ( vois[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) ) {
        return( 1 );
    }
    if ( ( vois[1+up[0]][1+up[1]][1+up[2]] != 0 ) &&
         ( vois[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) &&
         ( vois[1+so[0]][1+so[1]][1+so[2]] != 0 ) ) {
        return( 1 );
    }
    if ( ( vois[1+up[0]][1+up[1]][1+up[2]] != 0 ) &&
         ( vois[1+so[0]][1+so[1]][1+so[2]] != 0 ) &&
         ( vois[1+we[0]][1+we[1]][1+we[2]] != 0 ) ) {
        return( 1 );
    }
    if ( ( vois[1+up[0]][1+up[1]][1+up[2]] != 0 ) && 
         ( vois[1+we[0]][1+we[1]][1+we[2]] != 0 ) &&
         ( vois[1+no[0]][1+no[1]][1+no[2]] != 0 ) ) {
        return( 1 );
    }
    if ( ( vois[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
         ( vois[1+no[0]][1+no[1]][1+no[2]] != 0 ) &&
         ( vois[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) ) {
        return( 1 );
    }
    if ( ( vois[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
         ( vois[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) &&
         ( vois[1+so[0]][1+so[1]][1+so[2]] != 0 ) ) {
        return( 1 );
    }
    if ( ( vois[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
         ( vois[1+so[0]][1+so[1]][1+so[2]] != 0 ) &&
         ( vois[1+we[0]][1+we[1]][1+we[2]] != 0 ) ) {
        return( 1 );
    }
    if ( ( vois[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
         ( vois[1+we[0]][1+we[1]][1+we[2]] != 0 ) &&
         ( vois[1+no[0]][1+no[1]][1+no[2]] != 0 ) ) {
        return( 1 );
    }
    return( 0 );
}





int _VT_GB_IsSimple0( int *vois )
{
    int i, npt;

    /*--- si la condition 2a n'est pas verifiee ---*/
    /*--- point + up appartient au fond         ---*/
    if ( vois[21] != 0 ) return( 0 );
    
    /*--- si la condition 2b n'est pas verifiee ---*/
    /*--- point + bo appartient a l'objet       ---*/
    if ( vois[4] == 0 ) return( 0 );

    /*--- si la condition 2c n'est pas verifiee ---*/
    if ( vois[15] == 0 )
	if ( vois[24] != 0 ) 
	    return( 0 );
    if ( vois[13] == 0 )
	if ( vois[22] != 0 ) 
	    return( 0 );
    if ( vois[10] == 0 )
	if ( vois[18] != 0 )
	    return( 0 );
    if ( vois[12] == 0 )
	if ( vois[20] != 0 )
	    return( 0 );
    
    /*--- si la condition 2d n'est pas verifiee ---*/
    if ( ( vois[15] == 0 ) && 
	 ( vois[13] == 0 ) &&
	 ( vois[16] == 0 ) )
	if ( vois[25] != 0 )
	    return( 0 );
    if ( ( vois[13] == 0 ) &&
	 ( vois[10] == 0 ) &&
	 ( vois[11] == 0 ) )
	if ( vois[19] != 0 )
	    return( 0 );
    if ( ( vois[10] == 0 ) &&
	 ( vois[12] == 0 ) &&
	 ( vois[9] == 0 ) )
	if ( vois[17] != 0 )
	    return( 0 );
    if ( ( vois[12] == 0 ) &&
	 ( vois[15] == 0 ) &&
	 ( vois[14] == 0 ) )
	if ( vois[23] != 0 )
	    return( 0 );
    
    /*--- si on est arrive ici, c'est un point effacable ---*/
    
    /*--- on verifie maintenant que ce n'est pas un point bord de surface ---*/
    /*--- condition geometrique d'arret ---*/
    /*--- nombre de points dans le voisinage ---*/
    npt = 0;
    for ( i = 0; i < 26; i ++ )
	if ( vois[i] != 0 ) npt++;
    
    if ( npt < 4 )  return( 0 );
    if ( npt >= 8 ) return( 1 );

    /*--- le nombre de points est entre 4 et 7 ---*/

    if ( ( vois[21] != 0 ) && 
	 ( vois[15] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[21] != 0 ) &&
	 ( vois[13] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[21] != 0 ) &&
	 ( vois[10] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[21] != 0 ) && 
	 ( vois[12] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[4] != 0 ) &&
	 ( vois[15] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[4] != 0 ) &&
	 ( vois[13] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[4] != 0 ) &&
	 ( vois[10] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[4] != 0 ) &&
	 ( vois[12] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    return( 0 );
}





int _VT_GB_IsSimple1( int *vois )
{
    int i, npt;

    /*--- si la condition 2a n'est pas verifiee ---*/
    /*--- point + up appartient au fond         ---*/
    if ( vois[4] != 0 ) return( 0 );
    
    /*--- si la condition 2b n'est pas verifiee ---*/
    /*--- point + bo appartient a l'objet       ---*/
    if ( vois[21] == 0 ) return( 0 );

    /*--- si la condition 2c n'est pas verifiee ---*/
    if ( vois[10] == 0 )
	if ( vois[1] != 0 ) 
	    return( 0 );
    if ( vois[12] == 0 )
	if ( vois[3] != 0 ) 
	    return( 0 );
    if ( vois[15] == 0 )
	if ( vois[7] != 0 )
	    return( 0 );
    if ( vois[13] == 0 )
	if ( vois[5] != 0 )
	    return( 0 );
    
    /*--- si la condition 2d n'est pas verifiee ---*/
    if ( ( vois[10] == 0 ) && 
	 ( vois[12] == 0 ) &&
	 ( vois[9] == 0 ) )
	if ( vois[0] != 0 )
	    return( 0 );
    if ( ( vois[12] == 0 ) &&
	 ( vois[15] == 0 ) &&
	 ( vois[14] == 0 ) )
	if ( vois[6] != 0 )
	    return( 0 );
    if ( ( vois[15] == 0 ) &&
	 ( vois[13] == 0 ) &&
	 ( vois[16] == 0 ) )
	if ( vois[8] != 0 )
	    return( 0 );
    if ( ( vois[13] == 0 ) &&
	 ( vois[10] == 0 ) &&
	 ( vois[11] == 0 ) )
	if ( vois[2] != 0 )
	    return( 0 );
    
    /*--- si on est arrive ici, c'est un point effacable ---*/
    
    /*--- on verifie maintenant que ce n'est pas un point bord de surface ---*/
    /*--- condition geometrique d'arret ---*/
    /*--- nombre de points dans le voisinage ---*/
    npt = 0;
    for ( i = 0; i < 26; i ++ )
	if ( vois[i] != 0 ) npt++;

    if ( npt < 4 )  return( 0 );
    if ( npt >= 8 ) return( 1 );

    /*--- le nombre de points est entre 4 et 7 ---*/

    if ( ( vois[4] != 0 ) && 
	 ( vois[10] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[4] != 0 ) &&
	 ( vois[12] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[4] != 0 ) &&
	 ( vois[15] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[4] != 0 ) && 
	 ( vois[13] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[21] != 0 ) &&
	 ( vois[10] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[21] != 0 ) &&
	 ( vois[12] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[21] != 0 ) &&
	 ( vois[15] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[21] != 0 ) &&
	 ( vois[13] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    return( 0 );
}





int _VT_GB_IsSimple5( int *vois )
{
    int i, npt;

    /*--- si la condition 2a n'est pas verifiee ---*/
    /*--- point + up appartient au fond         ---*/
    if ( vois[12] != 0 ) return( 0 );
    
    /*--- si la condition 2b n'est pas verifiee ---*/
    /*--- point + bo appartient a l'objet       ---*/
    if ( vois[13] == 0 ) return( 0 );

    /*--- si la condition 2c n'est pas verifiee ---*/
    if ( vois[21] == 0 )
	if ( vois[20] != 0 ) 
	    return( 0 );
    if ( vois[15] == 0 )
	if ( vois[14] != 0 ) 
	    return( 0 );
    if ( vois[4] == 0 )
	if ( vois[3] != 0 )
	    return( 0 );
    if ( vois[10] == 0 )
	if ( vois[9] != 0 )
	    return( 0 );
    
    /*--- si la condition 2d n'est pas verifiee ---*/
    if ( ( vois[21] == 0 ) && 
	 ( vois[15] == 0 ) &&
	 ( vois[24] == 0 ) )
	if ( vois[23] != 0 )
	    return( 0 );
    if ( ( vois[15] == 0 ) &&
	 ( vois[4] == 0 ) &&
	 ( vois[7] == 0 ) )
	if ( vois[6] != 0 )
	    return( 0 );
    if ( ( vois[4] == 0 ) &&
	 ( vois[10] == 0 ) &&
	 ( vois[1] == 0 ) )
	if ( vois[0] != 0 )
	    return( 0 );
    if ( ( vois[10] == 0 ) &&
	 ( vois[21] == 0 ) &&
	 ( vois[18] == 0 ) )
	if ( vois[17] != 0 )
	    return( 0 );
    
    /*--- si on est arrive ici, c'est un point effacable ---*/
    
    /*--- on verifie maintenant que ce n'est pas un point bord de surface ---*/
    /*--- condition geometrique d'arret ---*/
    /*--- nombre de points dans le voisinage ---*/
    npt = 0;
    for ( i = 0; i < 26; i ++ )
	if ( vois[i] != 0 ) npt++;

    if ( npt < 4 )  return( 0 );
    if ( npt >= 8 ) return( 1 );

    /*--- le nombre de points est entre 4 et 7 ---*/

    if ( ( vois[12] != 0 ) && 
	 ( vois[21] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[12] != 0 ) &&
	 ( vois[15] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[12] != 0 ) &&
	 ( vois[4] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[12] != 0 ) && 
	 ( vois[10] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[13] != 0 ) &&
	 ( vois[21] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[13] != 0 ) &&
	 ( vois[15] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[13] != 0 ) &&
	 ( vois[4] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[13] != 0 ) &&
	 ( vois[10] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    return( 0 );
}





int _VT_GB_IsSimple4( int *vois )
{
    int i, npt;

    /*--- si la condition 2a n'est pas verifiee ---*/
    /*--- point + up appartient au fond         ---*/
    if ( vois[13] != 0 ) return( 0 );
    
    /*--- si la condition 2b n'est pas verifiee ---*/
    /*--- point + bo appartient a l'objet       ---*/
    if ( vois[12] == 0 ) return( 0 );

    /*--- si la condition 2c n'est pas verifiee ---*/
    if ( vois[4] == 0 )
	if ( vois[5] != 0 ) 
	    return( 0 );
    if ( vois[10] == 0 )
	if ( vois[11] != 0 ) 
	    return( 0 );
    if ( vois[21] == 0 )
	if ( vois[22] != 0 )
	    return( 0 );
    if ( vois[15] == 0 )
	if ( vois[16] != 0 )
	    return( 0 );
    
    /*--- si la condition 2d n'est pas verifiee ---*/
    if ( ( vois[4] == 0 ) && 
	 ( vois[10] == 0 ) &&
	 ( vois[1] == 0 ) )
	if ( vois[2] != 0 )
	    return( 0 );
    if ( ( vois[10] == 0 ) &&
	 ( vois[21] == 0 ) &&
	 ( vois[18] == 0 ) )
	if ( vois[19] != 0 )
	    return( 0 );
    if ( ( vois[21] == 0 ) &&
	 ( vois[15] == 0 ) &&
	 ( vois[24] == 0 ) )
	if ( vois[25] != 0 )
	    return( 0 );
    if ( ( vois[15] == 0 ) &&
	 ( vois[4] == 0 ) &&
	 ( vois[7] == 0 ) )
	if ( vois[8] != 0 )
	    return( 0 );
    
    /*--- si on est arrive ici, c'est un point effacable ---*/
    
    /*--- on verifie maintenant que ce n'est pas un point bord de surface ---*/
    /*--- condition geometrique d'arret ---*/
    /*--- nombre de points dans le voisinage ---*/
    npt = 0;
    for ( i = 0; i < 26; i ++ )
	if ( vois[i] != 0 ) npt++;
    
    if ( npt < 4 )  return( 0 );
    if ( npt >= 8 ) return( 1 );

    /*--- le nombre de points est entre 4 et 7 ---*/

    if ( ( vois[13] != 0 ) && 
	 ( vois[4] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[13] != 0 ) &&
	 ( vois[10] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[13] != 0 ) &&
	 ( vois[21] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[13] != 0 ) && 
	 ( vois[15] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[12] != 0 ) &&
	 ( vois[4] != 0 ) &&
	 ( vois[10] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[12] != 0 ) &&
	 ( vois[10] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[12] != 0 ) &&
	 ( vois[21] != 0 ) &&
	 ( vois[15] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[12] != 0 ) &&
	 ( vois[15] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    return( 0 );
}





int _VT_GB_IsSimple3( int *vois )
{
    int i, npt;

    /*--- si la condition 2a n'est pas verifiee ---*/
    /*--- point + up appartient au fond         ---*/
    if ( vois[10] != 0 ) return( 0 );
    
    /*--- si la condition 2b n'est pas verifiee ---*/
    /*--- point + bo appartient a l'objet       ---*/
    if ( vois[15] == 0 ) return( 0 );

    /*--- si la condition 2c n'est pas verifiee ---*/
    if ( vois[12] == 0 )
	if ( vois[9] != 0 ) 
	    return( 0 );
    if ( vois[21] == 0 )
	if ( vois[18] != 0 ) 
	    return( 0 );
    if ( vois[13] == 0 )
	if ( vois[11] != 0 )
	    return( 0 );
    if ( vois[4] == 0 )
	if ( vois[1] != 0 )
	    return( 0 );
    
    /*--- si la condition 2d n'est pas verifiee ---*/
    if ( ( vois[12] == 0 ) && 
	 ( vois[21] == 0 ) &&
	 ( vois[20] == 0 ) )
	if ( vois[17] != 0 )
	    return( 0 );
    if ( ( vois[21] == 0 ) &&
	 ( vois[13] == 0 ) &&
	 ( vois[22] == 0 ) )
	if ( vois[19] != 0 )
	    return( 0 );
    if ( ( vois[13] == 0 ) &&
	 ( vois[4] == 0 ) &&
	 ( vois[5] == 0 ) )
	if ( vois[2] != 0 )
	    return( 0 );
    if ( ( vois[4] == 0 ) &&
	 ( vois[12] == 0 ) &&
	 ( vois[3] == 0 ) )
	if ( vois[0] != 0 )
	    return( 0 );
    
    /*--- si on est arrive ici, c'est un point effacable ---*/
    
    /*--- on verifie maintenant que ce n'est pas un point bord de surface ---*/
    /*--- condition geometrique d'arret ---*/
    /*--- nombre de points dans le voisinage ---*/
    npt = 0;
    for ( i = 0; i < 26; i ++ )
	if ( vois[i] != 0 ) npt++;

    if ( npt < 4 )  return( 0 );
    if ( npt >= 8 ) return( 1 );

    /*--- le nombre de points est entre 4 et 7 ---*/

    if ( ( vois[10] != 0 ) && 
	 ( vois[12] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[10] != 0 ) &&
	 ( vois[21] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[10] != 0 ) &&
	 ( vois[13] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[10] != 0 ) && 
	 ( vois[4] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[15] != 0 ) &&
	 ( vois[12] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[15] != 0 ) &&
	 ( vois[21] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[15] != 0 ) &&
	 ( vois[13] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[15] != 0 ) &&
	 ( vois[4] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    return( 0 );
}





int _VT_GB_IsSimple2( int *vois )
{
    int i, npt;

    /*--- si la condition 2a n'est pas verifiee ---*/
    /*--- point + up appartient au fond         ---*/
    if ( vois[15] != 0 ) return( 0 );
    
    /*--- si la condition 2b n'est pas verifiee ---*/
    /*--- point + bo appartient a l'objet       ---*/
    if ( vois[10] == 0 ) return( 0 );

    /*--- si la condition 2c n'est pas verifiee ---*/
    if ( vois[13] == 0 )
	if ( vois[16] != 0 ) 
	    return( 0 );
    if ( vois[4] == 0 )
	if ( vois[7] != 0 ) 
	    return( 0 );
    if ( vois[12] == 0 )
	if ( vois[14] != 0 )
	    return( 0 );
    if ( vois[21] == 0 )
	if ( vois[24] != 0 )
	    return( 0 );
    
    /*--- si la condition 2d n'est pas verifiee ---*/
    if ( ( vois[13] == 0 ) && 
	 ( vois[4] == 0 ) &&
	 ( vois[5] == 0 ) )
	if ( vois[8] != 0 )
	    return( 0 );
    if ( ( vois[4] == 0 ) &&
	 ( vois[12] == 0 ) &&
	 ( vois[3] == 0 ) )
	if ( vois[6] != 0 )
	    return( 0 );
    if ( ( vois[12] == 0 ) &&
	 ( vois[21] == 0 ) &&
	 ( vois[20] == 0 ) )
	if ( vois[23] != 0 )
	    return( 0 );
    if ( ( vois[21] == 0 ) &&
	 ( vois[13] == 0 ) &&
	 ( vois[22] == 0 ) )
	if ( vois[25] != 0 )
	    return( 0 );
    
    /*--- si on est arrive ici, c'est un point effacable ---*/
    
    /*--- on verifie maintenant que ce n'est pas un point bord de surface ---*/
    /*--- condition geometrique d'arret ---*/
    /*--- nombre de points dans le voisinage ---*/
    npt = 0;
    for ( i = 0; i < 26; i ++ )
	if ( vois[i] != 0 ) npt++;
    
    if ( npt < 4 )  return( 0 );
    if ( npt >= 8 ) return( 1 );

    /*--- le nombre de points est entre 4 et 7 ---*/

    if ( ( vois[15] != 0 ) && 
	 ( vois[13] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[15] != 0 ) &&
	 ( vois[4] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[15] != 0 ) &&
	 ( vois[12] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[15] != 0 ) && 
	 ( vois[21] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[10] != 0 ) &&
	 ( vois[13] != 0 ) &&
	 ( vois[4] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[10] != 0 ) &&
	 ( vois[4] != 0 ) &&
	 ( vois[12] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[10] != 0 ) &&
	 ( vois[12] != 0 ) &&
	 ( vois[21] != 0 ) ) {
	return( 1 );
    }
    if ( ( vois[10] != 0 ) &&
	 ( vois[21] != 0 ) &&
	 ( vois[13] != 0 ) ) {
	return( 1 );
    }
    return( 0 );
}

