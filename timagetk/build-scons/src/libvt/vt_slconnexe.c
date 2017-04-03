#include <math.h>
#include <vt_slconnexe.h>


/* structure : liste de points

*/
typedef struct _vt_liste{
    int nb;
    int sz;
    int sz10;
    int sz100;
    int sz1000;
    vt_ipt **pt;
} _vt_liste;

/* structure : liste de points + valeur

*/
typedef struct _vt_vliste{
    int nb;
    int sz;
    int sz10;
    int sz100;
    int sz1000;
    vt_4vpt **pt;
} _vt_vliste;

/*-------Definition des fonctions statiques----------*/
static void _VT_InitNeighbors( vt_ipt *vois_06_18_26, vt_ipt *vois_04_08_10 );
static void _VT_InitList( _vt_liste *liste, int size );
static void _VT_FreeList( _vt_liste *liste );
static int  _VT_AddPtToList( _vt_liste *liste, int x, int y, int z );
static void _VT_InitVList( _vt_vliste *liste, int size );
static void _VT_FreeVList( _vt_vliste *liste );
static int  _VT_AddPtToVList( _vt_vliste *liste, int x, int y, int z, int v );


#define _VT_COMPUTE_LABEL(I,J,L,T) { (I) = 0;   (J) = (L); \
        while( (J) >= (T)->sz1000 ) { (I) += 1000;   (J) -= (T)->sz1000; } \
        while( (J) >= (T)->sz100 )  { (I) += 100;    (J) -= (T)->sz100;  } \
        while( (J) >= (T)->sz10 )   { (I) += 10;     (J) -= (T)->sz10;   } \
        while( (J) >= (T)->sz )     { (I) ++ ;       (J) -= (T)->sz;     } }

int VT_SlowConnexe( vt_image *image /* image where the connected components are to be searched */,
		    vt_connexe *par /* parameters */ )
{
    char *local_name = "VT_SlowConnexe";
    unsigned char *buf, ***pbuf;
    register int x, y, z, i, j, k, l, m;
    int flag, dx, dy, dz, size;
    int dx1, dy1, dz1, inside;
    _vt_liste liste1, liste2;
    _vt_liste *avant, *apres, *a_tmp;
    vt_ipt vois_06_18_26[26], vois_04_08_10[10], *vois;
    int local_connexite, local_dim, label, nb_voisins;

    if ( VT_Test1Image( image, local_name ) != 1 ) return( -1 );
    if ( image->type != UCHAR ) {
	VT_Error( "unable to deal with such image type", local_name );
	return( -1 );
    }

    /*--- dimensions ---*/
    dx = image->dim.x;
    dy = image->dim.y;
    dz = image->dim.z;
    dx1 = dx - 1;   dy1 = dy - 1;   dz1 = dz - 1;
    pbuf = (unsigned char***)(image->array);

    /*--- initialisations des voisinages ---*/
    _VT_InitNeighbors( vois_06_18_26, vois_04_08_10 );

    /*--- initialisation ---*/
    local_connexite = VT_CheckConnectivity( par->type_connexite, image->dim.z );

    switch( local_connexite ) {
    case N06 :
    case N10 :
    case N18 :
    case N26 :
	local_dim = VT_3D;
	break;
    case N04 :
    case N08 :
	local_dim = VT_2D;
	break;
    default :
	VT_Error( "unknown connectivity", local_name );
	return( -1 );
    }

    switch ( local_connexite ) {
    case N18 :
	nb_voisins = 18;   vois = vois_06_18_26;
	break;
    case N06 :
	nb_voisins = 6;    vois = vois_06_18_26;
	break;
    case N10 :
	nb_voisins = 10;   vois = vois_04_08_10;
	break;
    case N08 :
	nb_voisins = 8;    vois = vois_04_08_10;
	break;
    case N04 :
	nb_voisins = 4;    vois = vois_04_08_10;
	break;
    case N26 :
    default :
	nb_voisins = 26;   vois = vois_06_18_26;
    }

    /*--- initialisation :
          on met a 255 tout ce qui est au-dessus du seuil ---*/
    if ( par->threshold < 0.0 )        z = 0;
    else if ( par->threshold > 255.0 ) z = 255;
    else                               z = (int)(par->threshold + 0.5);
    y = dx * dy * dz;
    buf = (unsigned char*)(image->buf);
    for ( x = 0; x < y; x ++ ) {
	if ( *buf >= (unsigned char)z ) *buf++ = (unsigned char)255;
	else                            *buf++ = (unsigned char)0;
    }

    /*--- initialisation des listes ---*/
    size = (int)( sqrt( (double)(dx * dy * dz) ) + 1 );
    _VT_InitList( &liste1, size );
    _VT_InitList( &liste2, size );

    /*--- c'est parti ---*/
    label = 1;
    avant = &liste1;
    apres = &liste2;

    do {
	/*--- recherche d'un premier point ---*/
	flag = 0;
	size = 0;
	for ( z = 0; (z < dz) && (flag == 0); z ++ )
	for ( y = 0; (y < dy) && (flag == 0); y ++ )
        for ( x = 0; (x < dx) && (flag == 0); x ++ ) {
	    if ( pbuf[z][y][x] == (unsigned char)255 ) {
		if ( _VT_AddPtToList( avant, x, y, z ) != 1 ) {
		    _VT_FreeList( &liste1 );
		    _VT_FreeList( &liste2 );
		    VT_Error( "unable to add point to list", local_name );
		    return( -1 );
		}
		pbuf[z][y][x] = (unsigned char)label;
		size = 1;
		flag = 1;
	    }
	}
	/*--- a-t'on trouve un point ?
              si oui, on recherche la composante entiere ---*/
	if ( flag == 1 ) {
	    /*-- tant qu'il y a des points a traiter ---*/
	    while ( avant->nb > 0 ) {
		/*--- on traite les points de la liste ---*/
		for ( l = 0; l < avant->nb; l ++ ) {
		    _VT_COMPUTE_LABEL( i, j, l, avant )
		    x = avant->pt[i][j].x;
		    y = avant->pt[i][j].y;
		    z = avant->pt[i][j].z;
		    /*--- peut-on se passer des tests ? ---*/
		    inside = 0;
		    switch ( local_dim ) {
		    case VT_3D :
			if ( (z<=0) || (z>=dz1) ) break;
		    case VT_2D :
			if ( (x>0) && (x<dx1) && (y>0) && (y<dy1) ) inside = 1;
		    }
		    /*--- on traite les voisins ---*/
		    if ( inside == 1 ) {
			for ( m = 0; m < nb_voisins; m ++ ) {
			    i = vois[m].x;
			    j = vois[m].y;
			    k = vois[m].z;
			    if ( pbuf[z+k][y+j][x+i] == (unsigned char)255 ) {
				if ( _VT_AddPtToList( apres, x+i, y+j, z+k ) != 1 ) {
				    _VT_FreeList( &liste1 );
				    _VT_FreeList( &liste2 );
				    VT_Error( "unable to add point to list", local_name );
				    return( -1 );
				}
				pbuf[z+k][y+j][x+i] = (unsigned char)label;
				size ++;
			    }
			}
		    } else {
			for ( m = 0; m < nb_voisins; m ++ ) {
			    i = vois[m].x;
			    j = vois[m].y;
			    k = vois[m].z;
			    if ( (x+i >= 0) && (x+i < dx) &&
				 (y+j >= 0) && (y+j < dy) &&
				 (z+k >= 0) && (z+k < dz) ) {
				if ( pbuf[z+k][y+j][x+i] == (unsigned char)255 ) {
				    if ( _VT_AddPtToList( apres, x+i, y+j, z+k ) != 1 ) {
					_VT_FreeList( &liste1 );
					_VT_FreeList( &liste2 );
					VT_Error( "unable to add point to list", local_name );
					return( -1 );
				    }
				    pbuf[z+k][y+j][x+i] = (unsigned char)label;
				    size ++;
				}
			    }
			}
		    }
		    /*--- fin du traitement des voisins ---*/
		}

		/*-- on a traite les points de la premiere liste ---*/
		avant->nb = 0;
		/*--- on echange les listes ---*/
		a_tmp = avant;
		avant = apres;
		apres = a_tmp;
	    }

	    /*--- on a trouve une composante ---*/
	    if ( size < par->min_size ) {
		/*
		if ( par->verbose )
		    fprintf( stdout, "connected component of size %d will be deleted\n", size );
		*/
		y = dx * dy * dz;
		buf = (unsigned char*)(image->buf);
		for ( x = 0; x < y; x ++ ) {
		    if ( *buf == (unsigned char)label ) *buf = (unsigned char)0;
		    buf++;
		}
	    } else {
		if ( par->verbose )
		    fprintf( stdout, "connected component of size %d has label %d\n", size, label );
		label ++;
	    }
	}
    } while ( (label < 255) && (flag == 1) );

    _VT_FreeList( &liste1 );
    _VT_FreeList( &liste2 );
    
    if ( (label >= 255) ) {
	fprintf( stderr, "WARNING: 254 connected components are already be found. Remaining points\n" );
	fprintf( stderr, "         at 255 may form one or several other connected components.\n" );
    }

    /*--- fin ---*/
    return( 1 );
}





int VT_SlowConnexeWithThresholds( vt_image *image /* image where the connected components are to be searched */,
				  vt_connexe *par /* parameters */,
				  vt_ipt *seuils /* thresholds */,
				  vt_ipt *first_point /* first point */ )
{
    char *local_name = "VT_SlowConnexeWithThresholds";
    unsigned char *buf, ***pbuf;
    register int x, y, z, v, i, j, k, l, m;
    int dx, dy, dz, size;
    int dx1, dy1, dz1, dx2, dy2, dz2, inside1, inside2;
    int threshold, delta1, delta2;
    _vt_vliste liste1, liste2;
    _vt_vliste *avant, *apres, *a_tmp;
    vt_ipt vois_06_18_26[26], vois_04_08_10[10], *vois;
    int local_connexite, local_dim, nb_voisins;

    if ( VT_Test1Image( image, local_name ) != 1 ) return( -1 );
    if ( image->type != UCHAR ) {
	VT_Error( "unable to deal with such image type", local_name );
	return( -1 );
    }

    /*--- seuils ---*/
    threshold = seuils->x;
    delta1    = seuils->y;
    delta2    = seuils->z;

    /*--- dimensions ---*/
    dx = image->dim.x;
    dy = image->dim.y;
    dz = image->dim.z;
    dx1 = dx - 1;   dy1 = dy - 1;   dz1 = dz - 1;
    dx2 = dx - 2;   dy2 = dy - 2;   dz2 = dz - 2;
    pbuf = (unsigned char***)(image->array);

    /*--- initialisations des voisinages ---*/
    _VT_InitNeighbors( vois_06_18_26, vois_04_08_10 );

    /*--- initialisation de la connexite ---*/
    local_connexite = VT_CheckConnectivity( par->type_connexite, image->dim.z );

    switch( local_connexite ) {
    case N06 :
    case N10 :
    case N18 :
    case N26 :
	local_dim = VT_3D;
	break;
    case N04 :
    case N08 :
	local_dim = VT_2D;
	break;
    default :
	VT_Error( "unknown connectivity", local_name );
	return( -1 );
    }

    switch ( local_connexite ) {
    case N18 :
	nb_voisins = 18;   vois = vois_06_18_26;
	break;
    case N06 :
	nb_voisins = 6;    vois = vois_06_18_26;
	break;
    case N10 :
	nb_voisins = 10;   vois = vois_04_08_10;
	break;
    case N08 :
	nb_voisins = 8;    vois = vois_04_08_10;
	break;
    case N04 :
	nb_voisins = 4;    vois = vois_04_08_10;
	break;
    case N26 :
    default :
	nb_voisins = 26;   vois = vois_06_18_26;
    }

    /*--- initialisation :
          on met a 250 tout ce qui est au-dessus de 250 (c'est Luc qui l'a dit) ---*/
    y = dx * dy * dz;
    buf = (unsigned char*)(image->buf);
    for ( x = 0; x < y; x ++ ) {
	if ( *buf >= (unsigned char)250 ) *buf++ = (unsigned char)250;
    }

    /*--- initialisation des listes ---*/
    size = (int)( sqrt( (double)(dx * dy * dz) ) + 1 );
    _VT_InitVList( &liste1, size );
    _VT_InitVList( &liste2, size );

    /*--- c'est parti ---*/
    avant = &liste1;
    apres = &liste2;

    /*--- le premier point est donne ---*/
    x = first_point->x;
    y = first_point->y;
    z = first_point->z;
    if ( (int)pbuf[z][y][x] > threshold ) {
	_VT_FreeVList( &liste1 );
	_VT_FreeVList( &liste2 );
	VT_Error( "value of first point greater than threshold", local_name );
	return( -1 );
    }

    if ( _VT_AddPtToVList( avant, x, y, z, (int)pbuf[z][y][x] ) != 1 ) {
	_VT_FreeVList( &liste1 );
	_VT_FreeVList( &liste2 );
	VT_Error( "unable to add point to list", local_name );
	return( -1 );
    }
    pbuf[z][y][x] = (unsigned char)255;
    size = 1;

    /*-- tant qu'il y a des points a traiter ---*/
    while ( avant->nb > 0 ) {
	/*--- on traite les points de la liste ---*/
	for ( l = 0; l < avant->nb; l ++ ) {
	    _VT_COMPUTE_LABEL( i, j, l, avant )
	    x = avant->pt[i][j].x;
	    y = avant->pt[i][j].y;
	    z = avant->pt[i][j].z;
	    v = avant->pt[i][j].v;
	    /*--- peut-on se passer des tests ? ---*/
	    inside1 = inside2 = 0;
	    switch ( local_dim ) {
	    case VT_3D :
		if ( (z<=0) || (z>=dz1) ) break;
	    case VT_2D :
		if ( (x>0) && (x<dx1) && (y>0) && (y<dy1) ) inside1 = 1;
	    }
	    if ( inside1 == 1 ) {
		switch ( local_dim ) {
		case VT_3D :
		    if ( (z<=1) || (z>=dz2) ) break;
		case VT_2D :
		    if ( (x>1) && (x<dx2) && (y>1) && (y<dy2) ) inside2 = 1;
		}
	    }
	    /*--- on traite les voisins ---*/
	    if ( inside2 == 1 ) {
		for ( m = 0; m < nb_voisins; m ++ ) {
		    i = vois[m].x;   j = vois[m].y;   k = vois[m].z;
		    /*--- le point voisin de 1 est-il correct ? ---*/
		    if ( ((int)pbuf[z+k][y+j][x+i] > threshold) ||
			 (((int)pbuf[z+k][y+j][x+i] - v) > delta1) ||
			 ((v - (int)pbuf[z+k][y+j][x+i]) > delta1) ) continue;
		    /*--- si oui, peut-on comparer avec un voisin de 2 ? ---*/
		    if ( (int)pbuf[z+k+k][y+j+j][x+i+i] < 255 ) {
			if ( (((int)pbuf[z+k+k][y+j+j][x+i+i] - v) <= delta2) &&
			     ((v - (int)pbuf[z+k+k][y+j+j][x+i+i]) <= delta2) ) {
			    /*--- OK ---*/
			    if ( _VT_AddPtToVList( apres, x+i, y+j, z+k, (int)pbuf[z+k][y+j][x+i] ) != 1 ) {
				_VT_FreeVList( &liste1 );
				_VT_FreeVList( &liste2 );
				VT_Error( "unable to add point to list", local_name );
				return( -1 );
			    }
			    pbuf[z+k][y+j][x+i] = (unsigned char)255;
			    size ++;
			}
		    } else {
			/*--- on ne peut pas comparer, mais OK quand meme ---*/
			if ( _VT_AddPtToVList( apres, x+i, y+j, z+k, (int)pbuf[z+k][y+j][x+i] ) != 1 ) {
			    _VT_FreeVList( &liste1 );
			    _VT_FreeVList( &liste2 );
			    VT_Error( "unable to add point to list", local_name );
			    return( -1 );
			}
			pbuf[z+k][y+j][x+i] = (unsigned char)255;
			size ++;
		    }
		}
		/*--- fin du traitement des voisins ---*/
	    } else if ( inside1 == 1 ) {
		for ( m = 0; m < nb_voisins; m ++ ) {
		    i = vois[m].x;   j = vois[m].y;   k = vois[m].z;
		    /*--- le point voisin de 1 est-il correct ? ---*/
		    if ( ((int)pbuf[z+k][y+j][x+i] > threshold) ||
			 (((int)pbuf[z+k][y+j][x+i] - v) > delta1) ||
			 ((v - (int)pbuf[z+k][y+j][x+i]) > delta1) ) continue;
		    /*--- si oui, peut-on comparer avec un voisin de 2 ? ---*/
		    if ( (x+i+i >= 0) && (x+i+i < dx) &&
			 (y+j+j >= 0) && (y+j+j < dy) &&
			 (z+k+k >= 0) && (z+k+k < dz) &&
			 ((int)pbuf[z+k+k][y+j+j][x+i+i] < 255) ) {
			if ( (((int)pbuf[z+k+k][y+j+j][x+i+i] - v) <= delta2) &&
			     ((v - (int)pbuf[z+k+k][y+j+j][x+i+i]) <= delta2) ) {
			    /*--- OK ---*/
			    if ( _VT_AddPtToVList( apres, x+i, y+j, z+k, (int)pbuf[z+k][y+j][x+i] ) != 1 ) {
				_VT_FreeVList( &liste1 );
				_VT_FreeVList( &liste2 );
				VT_Error( "unable to add point to list", local_name );
				return( -1 );
			    }
			    pbuf[z+k][y+j][x+i] = (unsigned char)255;
			    size ++;
			}
		    } else {
			/*--- on ne peut pas comparer, mais OK quand meme ---*/
			if ( _VT_AddPtToVList( apres, x+i, y+j, z+k, (int)pbuf[z+k][y+j][x+i] ) != 1 ) {
			    _VT_FreeVList( &liste1 );
			    _VT_FreeVList( &liste2 );
			    VT_Error( "unable to add point to list", local_name );
			    return( -1 );
			}
			pbuf[z+k][y+j][x+i] = (unsigned char)255;
			size ++;
		    }
		}
		/*--- fin du traitement des voisins ---*/
	    } else {
		for ( m = 0; m < nb_voisins; m ++ ) {
		    i = vois[m].x;   j = vois[m].y;   k = vois[m].z;
		    /*--- le point voisin de 1 est-il dans l'image ? ---*/
		    if ( (x+i < 0) || (x+i >= dx) ||
			 (y+j < 0) || (y+j >= dy) ||
			 (z+k < 0) || (z+k >= dz) ) continue;
		    /*--- si oui, est-il correct ? ---*/
		    if ( ((int)pbuf[z+k][y+j][x+i] > threshold) ||
			 (((int)pbuf[z+k][y+j][x+i] - v) > delta1) ||
			 ((v - (int)pbuf[z+k][y+j][x+i]) > delta1) ) continue;
		    /*--- si oui, peut-on comparer avec un voisin de 2 ? ---*/
		    if ( (x+i+i >= 0) && (x+i+i < dx) &&
			 (y+j+j >= 0) && (y+j+j < dy) &&
			 (z+k+k >= 0) && (z+k+k < dz) &&
			 ((int)pbuf[z+k+k][y+j+j][x+i+i] < 255) ) {
			if ( (((int)pbuf[z+k+k][y+j+j][x+i+i] - v) <= delta2) &&
			     ((v - (int)pbuf[z+k+k][y+j+j][x+i+i]) <= delta2) ) {
			    /*--- OK ---*/
			    if ( _VT_AddPtToVList( apres, x+i, y+j, z+k, (int)pbuf[z+k][y+j][x+i] ) != 1 ) {
				_VT_FreeVList( &liste1 );
				_VT_FreeVList( &liste2 );
				VT_Error( "unable to add point to list", local_name );
				return( -1 );
			    }
			    pbuf[z+k][y+j][x+i] = (unsigned char)255;
			    size ++;
			}
		    } else {
			/*--- on ne peut pas comparer, mais OK quand meme ---*/
			if ( _VT_AddPtToVList( apres, x+i, y+j, z+k, (int)pbuf[z+k][y+j][x+i] ) != 1 ) {
			    _VT_FreeVList( &liste1 );
			    _VT_FreeVList( &liste2 );
			    VT_Error( "unable to add point to list", local_name );
			    return( -1 );
			}
			pbuf[z+k][y+j][x+i] = (unsigned char)255;
			size ++;
		    }
		}
		/*--- fin du traitement des voisins ---*/
	    }
	}
	
	/*-- on a traite les points de la premiere liste ---*/
	avant->nb = 0;
	/*--- on echange les listes ---*/
	a_tmp = avant;
	avant = apres;
	apres = a_tmp;
    }

    if ( par->verbose )
	fprintf( stdout, "connected component of size %d has label 255\n", size );
    
    _VT_FreeVList( &liste1 );
    _VT_FreeVList( &liste2 );
    
    /*--- fin ---*/
    return( 1 );
}





static void _VT_InitNeighbors( vt_ipt *vois_06_18_26, vt_ipt *vois_04_08_10 )
{
    /*--- 06-voisins ---*/
    vois_06_18_26[ 0].x =  0;   vois_06_18_26[ 0].y =  0;   vois_06_18_26[ 0].z = -1;
    vois_06_18_26[ 1].x =  0;   vois_06_18_26[ 1].y =  0;   vois_06_18_26[ 1].z =  1;
    vois_06_18_26[ 2].x =  0;   vois_06_18_26[ 2].y = -1;   vois_06_18_26[ 2].z =  0;
    vois_06_18_26[ 3].x =  0;   vois_06_18_26[ 3].y =  1;   vois_06_18_26[ 3].z =  0;
    vois_06_18_26[ 4].x = -1;   vois_06_18_26[ 4].y =  0;   vois_06_18_26[ 4].z =  0;
    vois_06_18_26[ 5].x =  1;   vois_06_18_26[ 5].y =  0;   vois_06_18_26[ 5].z =  0;
    /*--- 18-voisins ---*/
    vois_06_18_26[ 6].x =  0;   vois_06_18_26[ 6].y = -1;   vois_06_18_26[ 6].z = -1;
    vois_06_18_26[ 7].x = -1;   vois_06_18_26[ 7].y =  0;   vois_06_18_26[ 7].z = -1;
    vois_06_18_26[ 8].x =  0;   vois_06_18_26[ 8].y =  1;   vois_06_18_26[ 8].z = -1;
    vois_06_18_26[ 9].x =  1;   vois_06_18_26[ 9].y =  0;   vois_06_18_26[ 9].z = -1;
    vois_06_18_26[10].x =  0;   vois_06_18_26[10].y = -1;   vois_06_18_26[10].z =  1;
    vois_06_18_26[11].x = -1;   vois_06_18_26[11].y =  0;   vois_06_18_26[11].z =  1;
    vois_06_18_26[12].x =  0;   vois_06_18_26[12].y =  1;   vois_06_18_26[12].z =  1;
    vois_06_18_26[13].x =  1;   vois_06_18_26[13].y =  0;   vois_06_18_26[13].z =  1;
    vois_06_18_26[14].x = -1;   vois_06_18_26[14].y = -1;   vois_06_18_26[14].z =  0;
    vois_06_18_26[15].x =  1;   vois_06_18_26[15].y = -1;   vois_06_18_26[15].z =  0;
    vois_06_18_26[16].x = -1;   vois_06_18_26[16].y =  1;   vois_06_18_26[16].z =  0;
    vois_06_18_26[17].x =  1;   vois_06_18_26[17].y =  1;   vois_06_18_26[17].z =  0;
    /*--- 26 voisins ---*/
    vois_06_18_26[18].x = -1;   vois_06_18_26[18].y = -1;   vois_06_18_26[18].z = -1;
    vois_06_18_26[19].x =  1;   vois_06_18_26[19].y = -1;   vois_06_18_26[19].z = -1;
    vois_06_18_26[20].x = -1;   vois_06_18_26[20].y =  1;   vois_06_18_26[20].z = -1;
    vois_06_18_26[21].x =  1;   vois_06_18_26[21].y =  1;   vois_06_18_26[21].z = -1;
    vois_06_18_26[22].x = -1;   vois_06_18_26[22].y = -1;   vois_06_18_26[22].z =  1;
    vois_06_18_26[23].x =  1;   vois_06_18_26[23].y = -1;   vois_06_18_26[23].z =  1;
    vois_06_18_26[24].x = -1;   vois_06_18_26[24].y =  1;   vois_06_18_26[24].z =  1;
    vois_06_18_26[25].x =  1;   vois_06_18_26[25].y =  1;   vois_06_18_26[25].z =  1;

    /*--- 04-voisins ---*/
    vois_04_08_10[ 0].x =  0;   vois_04_08_10[ 0].y = -1;   vois_04_08_10[ 0].z =  0;
    vois_04_08_10[ 1].x =  0;   vois_04_08_10[ 1].y =  1;   vois_04_08_10[ 1].z =  0;
    vois_04_08_10[ 2].x = -1;   vois_04_08_10[ 2].y =  0;   vois_04_08_10[ 2].z =  0;
    vois_04_08_10[ 3].x =  1;   vois_04_08_10[ 3].y =  0;   vois_04_08_10[ 3].z =  0;
    /*--- 08-voisins ---*/
    vois_04_08_10[ 4].x = -1;   vois_04_08_10[ 4].y = -1;   vois_04_08_10[ 4].z =  0;
    vois_04_08_10[ 5].x =  1;   vois_04_08_10[ 5].y = -1;   vois_04_08_10[ 5].z =  0;
    vois_04_08_10[ 6].x = -1;   vois_04_08_10[ 6].y =  1;   vois_04_08_10[ 6].z =  0;
    vois_04_08_10[ 7].x =  1;   vois_04_08_10[ 7].y =  1;   vois_04_08_10[ 7].z =  0;
    /*--- 10 voisins ---*/
    vois_04_08_10[ 8].x =  0;   vois_04_08_10[ 8].y =  0;   vois_04_08_10[ 8].z = -1;
    vois_04_08_10[ 9].x =  0;   vois_04_08_10[ 9].y =  0;   vois_04_08_10[ 9].z =  1;
}





static void _VT_InitList( _vt_liste *liste, int size )
{
    liste->nb     = 0;
    liste->sz     = size;
    liste->sz10   = 10   * size;
    liste->sz100  = 100  * size;
    liste->sz1000 = 1000 * size;
    liste->pt = (vt_ipt**)NULL;
}





static void _VT_FreeList( _vt_liste *liste )
{
    int i;


    if ( liste->pt != (vt_ipt**)NULL ) {
	for ( i = 0; i < liste->sz; i ++ )
	    if ( liste->pt[i] != (vt_ipt*)NULL )
		VT_Free( (void**)&(liste->pt[i]) );
	VT_Free( (void**)&(liste->pt) );
    }
    liste->nb     = 0;
    liste->sz     = 0;
    liste->sz10   = 0;
    liste->sz100  = 0;
    liste->sz1000 = 0;
}





static int _VT_AddPtToList( _vt_liste *liste, int x, int y, int z )
{
    int i, j;
    
    /*--- premiere allocation ---*/
    if ( liste->pt == (vt_ipt**)NULL ) {
	liste->pt = (vt_ipt**)VT_Malloc( (unsigned int)(liste->sz * sizeof( vt_ipt* )) );
	if ( liste->pt == (vt_ipt**)NULL ) {
	    VT_Error( "allocation of main list failed", "_VT_AddPtToList" );
	    return( -1 );
	}
	for ( i = 0; i < liste->sz; i ++ )
	    liste->pt[i] = (vt_ipt*)NULL;
    }
    _VT_COMPUTE_LABEL( i, j, liste->nb, liste )
    /*--- autre allocation ? ---*/
    if ( (j == 0) && (liste->pt[i] == (vt_ipt*)NULL) ) {
	liste->pt[i] = (vt_ipt*)VT_Malloc( (unsigned int)(liste->sz * sizeof( vt_ipt )) );
	if ( liste->pt[i] == (vt_ipt*)NULL ) {
	    VT_Error( "allocation of second list failed", "_VT_AddPtToList" );
	    return( -1 );
	}
    }
    /*--- on ajoute le point ---*/
    liste->pt[i][j].x = x;
    liste->pt[i][j].y = y;
    liste->pt[i][j].z = z;
    liste->nb ++;
    return( 1 );
}





static void _VT_InitVList( _vt_vliste *liste, int size )
{
    liste->nb     = 0;
    liste->sz     = size;
    liste->sz10   = 10   * size;
    liste->sz100  = 100  * size;
    liste->sz1000 = 1000 * size;
    liste->pt = (vt_4vpt**)NULL;
}





static void _VT_FreeVList( _vt_vliste *liste )
{
    int i;


    if ( liste->pt != (vt_4vpt**)NULL ) {
	for ( i = 0; i < liste->sz; i ++ )
	    if ( liste->pt[i] != (vt_4vpt*)NULL )
		VT_Free( (void**)&(liste->pt[i]) );
	VT_Free( (void**)&(liste->pt) );
    }
    liste->nb     = 0;
    liste->sz     = 0;
    liste->sz10   = 0;
    liste->sz100  = 0;
    liste->sz1000 = 0;
}





static int _VT_AddPtToVList( _vt_vliste *liste, int x, int y, int z, int v )
{
    int i, j;
    
    /*--- premiere allocation ---*/
    if ( liste->pt == (vt_4vpt**)NULL ) {
	liste->pt = (vt_4vpt**)VT_Malloc( (unsigned int)(liste->sz * sizeof( vt_4vpt* )) );
	if ( liste->pt == (vt_4vpt**)NULL ) {
	    VT_Error( "allocation of main list failed", "_VT_AddPtToVList" );
	    return( -1 );
	}
	for ( i = 0; i < liste->sz; i ++ )
	    liste->pt[i] = (vt_4vpt*)NULL;
    }
    _VT_COMPUTE_LABEL( i, j, liste->nb, liste )
    /*--- autre allocation ? ---*/
    if ( (j == 0) && (liste->pt[i] == (vt_4vpt*)NULL) ) {
	liste->pt[i] = (vt_4vpt*)VT_Malloc( (unsigned int)(liste->sz * sizeof( vt_4vpt )) );
	if ( liste->pt[i] == (vt_4vpt*)NULL ) {
	    VT_Error( "allocation of second list failed", "_VT_AddPtToVList" );
	    return( -1 );
	}
    }
    /*--- on ajoute le point ---*/
    liste->pt[i][j].x = x;
    liste->pt[i][j].y = y;
    liste->pt[i][j].z = z;
    liste->pt[i][j].v = v;
    liste->nb ++;
    return( 1 );
}
