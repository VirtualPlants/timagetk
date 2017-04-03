
#include <vt_dst_amincir.h>

int _VT_DST_GREG_2D( vt_image *im /* image to be thinned */,
			    vt_amincir *par /* parameters */ )
{
    vt_vpt_amincir *liste;
    vt_image value;
    vt_distance dpar;
    int ln, lnb;
    int *in, *no, *ea, *so, *we;
    int tno[2], tea[2], tso[2], twe[2];
    int vois[3][3], offset[3][3];
    register unsigned char *b;
    unsigned short int *vb;
    register int i, j, l, ind, npt, dx, dxy;
    register int first, last, min;
    int cycle, nbdel, nbd;
    char message[256];

    if ( VT_Test1Image( im, "_VT_DST_GREG_2D" ) == -1 ) return( -1 );
    if ( (im->type != UCHAR) ) return( -1 );

    /*--- calcul des distances ---*/
    VT_InitImage( &value, "_vt_distance_image_", im->dim.x, im->dim.y, im->dim.z, (int)USHORT );
    if ( VT_AllocImage( &value ) != 1 ) {
	VT_Error("unable to allocate auxiliary image","_VT_DST_GREG_2D");
	return( -1 );
    }
    j = im->dim.x * im->dim.y * im->dim.z;
    b = (unsigned char*)(im->buf);
    vb = (unsigned short int*)(value.buf);
    for ( i = 0; i < j; i ++ )
	*vb++ = (unsigned short int)( ( *b++ > 0 ) ? 0 : 65535 );
    dpar = par->par_dist;
    dpar.seuil = (float)1.0;
    if ( VT_Dist( &value, &value, &dpar ) != 1 ) {
	VT_Error("unable to compute distance image","_VT_DST_GREG_2D");
	VT_FreeImage( &value );
	return( -1 );
    }

    /*--- calcul de la liste de points ---*/
    liste = _VT_ThinVPtList( im, &value, (int)VT_2D, &lnb );
    if ( (liste == (vt_vpt_amincir*)NULL) || (lnb <= 0) ) {
	VT_Error( "unable to compute list of points", "_VT_DST_GREG_2D" );
	VT_FreeImage( &value );
	return( 0 );
    }
    VT_FreeImage( &value );

    /*--- preparation ---*/
    b = (unsigned char*)(im->buf);
    tno[0] = 0;   tno[1] = 1;
    tea[0] = 1;   tea[1] = 0;
    tso[0] = 0;   tso[1] = -1;
    twe[0] = -1;  twe[1] = 0;
    no = tno;   ea = tea;
    so = tso;   we = twe;
    dx = im->dim.x;   dxy = im->dim.x * im->dim.y;
    offset[1][1] = 0;
    offset[1][0] = - dx;
    offset[1][2] = dx;
    offset[0][1] = - 1;
    offset[0][0] = offset[1][0] - 1;
    offset[0][2] = offset[1][2] - 1;
    offset[2][1] = 1;
    offset[2][0] = offset[1][0] + 1;
    offset[2][2] = offset[1][2] + 1;
    ln = lnb;
    
    first = 0;
    /*--- boucle generale ---*/
    do {
	/*--- boucle des 4 sous-cycles ---*/
	nbdel = 0;
	for (cycle = 0; cycle < 4; cycle++ ) {
	    min = liste[first].pt.v;
	    nbd = 0;
	    /*--- liste des points ---*/
	    for ( l = first; (l < ln) && (liste[l].pt.v == min); l++ ) {
		/*--- saisie du voisinage ---*/
		ind = liste[l].pt.z * dxy + liste[l].pt.y * dx + liste[l].pt.x;
		if ( liste[l].inside == 1 ) {
		    for ( i = 0; i < 3; i++ )
		    for ( j = 0; j < 3; j++ )
			vois[i][j] = (int)(b[ ind + offset[i][j] ]);
		} else {
		    for ( i = -1; i < 2; i++ )
		    for ( j = -1; j < 2; j++ ) {
			if ( (liste[l].pt.x+i >= 0) && (liste[l].pt.x+i < dx) &&
           (liste[l].pt.y+j >= 0) && (liste[l].pt.y+j < (int)im->dim.y) )
			    vois[1+i][1+j] = (int)(b[ ind + offset[1+i][1+j] ]);
			else
			    vois[1+i][1+j] = (int)0;
		    }
		}
		/*--- si la premiere condition n'est pas verifiee ---*/
		/*--- point + no appartient au fond               ---*/
		if ( vois[1+no[0]][1+no[1]] != 0 ) continue;

		/*--- si la seconde condition n'est pas verifiee ---*/
		/*--- point + so appartient a l'objet            ---*/
		if ( vois[1+so[0]][1+so[1]] == 0 ) continue;
		
		/*--- peut-on effacer le point ?            ---*/
		if ( vois[1+ea[0]][1+ea[1]] == 0 ) 
		    if ( vois[1+no[0]+ea[0]][1+no[1]+ea[1]] != 0 )
			continue;
		if ( vois[1+we[0]][1+we[1]] == 0 ) 
		    if ( vois[1+no[0]+we[0]][1+no[1]+we[1]] != 0 )
			continue;
		
		/*--- si on est arrive ici, c'est un point effacable ---*/
		if ( par->bool_shrink == 1 ) {
		    liste[l].status = VT_TOBEDELETED;
		    continue;
		}
		
		/*--- condition geometrique d'arret ---*/
		/*--- nombre de points dans le voisinage ---*/
		npt = 0;
		if ( vois[0][0] != 0 ) npt++;
		if ( vois[1][0] != 0 ) npt++;
		if ( vois[2][0] != 0 ) npt++;
		if ( vois[0][1] != 0 ) npt++;
		if ( vois[2][1] != 0 ) npt++;
		if ( vois[0][2] != 0 ) npt++;
		if ( vois[1][2] != 0 ) npt++;
		if ( vois[2][2] != 0 ) npt++;
		
		if ( npt == 1 ) {
		    liste[l].status = VT_UNDELETABLE;
		    continue;
		}
		
		liste[l].status = VT_TOBEDELETED;
	    }
	    /*--- fin du traitement de la liste ---*/
	    /*--- on modifie l'image et on rearrange la liste ---*/
	    last = l; /* le point apres le dernier point traite */
	    for ( l = last - 1; l >= first; l -- ) {
		switch ( liste[l].status ) {
		case VT_DELETABLE :
		    /*--- on garde le point dans la liste ---*/
		    liste[--last] = liste[l];
		    break;
		case VT_TOBEDELETED :
		    /*--- on efface le point ---*/
		    ind = liste[l].pt.z * dxy + liste[l].pt.y * dx + liste[l].pt.x;
		    b[ind] = (unsigned char)0;
		    nbd ++;
		    break;
		case VT_UNDELETABLE :
		    /*--- on ne fait rien ---*/
		    break;
		default :
		    /*--- on s'inquiete ---*/
		    VT_Error("point with unknown label","_VT_DST_GREG_2D");
		}
	    }
	    first = last;
	    nbdel += nbd;
	    
	    /*--- fin d'un sous-cycle : on prepare le suivant ---*/
	    switch ( cycle ) {
	    case 0 :
	    case 2 :
		for (i = 0; i < 2; i++ ) {
		    in = no; no = ea; ea = so; so = we; we = in;
		}
		break;
	    case 1 :
		for (i = 0; i < 1; i++ ) {
		    in = no; no = ea; ea = so; so = we; we = in;
		}
		break;
	    case 3 :
		for (i = 0; i < 3; i++ ) {
		    in = no; no = ea; ea = so; so = we; we = in;
		}
	    }
	    /*--- message ---*/
	    sprintf( message,"     subiteration # %d, deleted points = %d", cycle+1, nbd );
	    VT_Message( message, "_VT_DST_GREG_2D" );
	}
	/*--- message ---*/
	sprintf( message," iteration, deleted points = %d", nbdel );
	VT_Message( message, "_VT_DST_GREG_2D" );
    } while ( (nbdel > 0) && (first != ln) );
    
    
    /*--- si on n'est pas en mode shrinking ---*/
    if ( par->bool_shrink == 0 ) {
	VT_Free( (void**)&liste );
	return( 1 );
    }
    
    /*--- sinon on continue : boucle generale ---*/
    do {
	/*--- boucle des 4 sous-cycles ---*/
	nbdel = 0;
	for (cycle = 0; cycle < 4; cycle++ ) {
	    min = liste[first].pt.v;
	    nbd = 0;
	    /*--- liste des points ---*/
	    for ( l = first; (l < ln) && (liste[l].pt.v == min); l++ ) {
		/*--- saisie du voisinage ---*/
		ind = liste[l].pt.z * dxy + liste[l].pt.y * dx + liste[l].pt.x;
		if ( liste[l].inside == 1 ) {
		    for ( i = 0; i < 3; i++ )
		    for ( j = 0; j < 3; j++ )
			vois[i][j] = (int)(b[ ind + offset[i][j] ]);
		} else {
		    for ( i = -1; i < 2; i++ )
		    for ( j = -1; j < 2; j++ ) {
			if ( (liste[l].pt.x+i >= 0) && (liste[l].pt.x+i < dx) &&
           (liste[l].pt.y+j >= 0) && (liste[l].pt.y+j < (int)im->dim.y) )
			    vois[1+i][1+j] = (int)(b[ ind + offset[1+i][1+j] ]);
			else
			    vois[1+i][1+j] = (int)0;
		    }
		}
		
		for ( i = 0; i < 3; i++ )
		for ( j = 0; j < 3; j++ )
		    if ( vois[i][j] == VT_WILLBEDELETED ) vois[i][j] = 0;
		
		/*--- si la premiere condition n'est pas verifiee             ---*/
		/*--- le point ( no ) appartient au fond                      ---*/
		if ( ( vois[1+no[0]][1+no[1]] != 0 ) )
		    continue;

		/*--- si la seconde condition n'est pas verifiee                ---*/
		/*--- un des 3 points ( so, so-ea, so-we ) appartient a l'objet ---*/
		if ( ( vois[1+so[0]][1+so[1]] == 0 ) &&
		     ( vois[1+so[0]+ea[0]][1+so[1]+ea[1]] == 0 ) &&
		     ( vois[1+so[0]+we[0]][1+so[1]+we[1]] == 0 ) )
		    continue;
		
		/*--- peut-on effacer le point ?            ---*/
		if ( vois[1+ea[0]][1+ea[1]] == 0 ) 
		    if ( vois[1+no[0]+ea[0]][1+no[1]+ea[1]] != 0 )
			continue;
		if ( vois[1+we[0]][1+we[1]] == 0 ) 
		    if ( vois[1+no[0]+we[0]][1+no[1]+we[1]] != 0 )
			continue;
		if ( vois[1+so[0]][1+so[1]] == 0 ) 
		    if ( ( (vois[1+we[0]][1+we[1]] != 0) || 
			   (vois[1+so[0]+we[0]][1+so[1]+we[1]] != 0) ) &&
			 ( (vois[1+ea[0]][1+ea[1]] != 0) ||
			   (vois[1+so[0]+ea[0]][1+so[1]+ea[1]] != 0) ) )
			continue;
		
		/*--- si on est arrive ici, c'est un point effacable ---*/
		liste[l].status = VT_TOBEDELETED;
		b[ind] = VT_WILLBEDELETED;
	    }
	    
	    /*--- fin du traitement de la liste ---*/
	    /*--- on modifie l'image et on rearrange la liste ---*/
	    last = l; /* le point apres le dernier point traite */
	    for ( i = l = last - 1; l >= first; l -- ) {
		switch ( liste[l].status ) {
		case VT_DELETABLE :
		    /*--- on garde le point dans la liste ---*/
		    liste[i--] = liste[l];
		    break;
		case VT_TOBEDELETED :
		    /*--- on efface le point ---*/
		    ind = liste[l].pt.z * dxy + liste[l].pt.y * dx + liste[l].pt.x;
		    b[ind] = (unsigned char)0;
		    nbd ++;
		    break;
		case VT_UNDELETABLE :
		    /*--- on ne fait rien ---*/
		    break;
		default :
		    /*--- on s'inquiete ---*/
		    VT_Error("point with unknown label","_VT_DST_GREG_2D");
		}
	    }
	    first = last;
	    nbdel += nbd;
	    
	    /*--- fin d'un sous-cycle : on prepare le suivant ---*/
	    switch ( cycle ) {
	    case 0 :
	    case 2 :
		for (i = 0; i < 2; i++ ) {
		    in = no; no = ea; ea = so; so = we; we = in;
		}
		break;
	    case 1 :
		for (i = 0; i < 1; i++ ) {
		    in = no; no = ea; ea = so; so = we; we = in;
		}
		break;
	    case 3 :
		for (i = 0; i < 3; i++ ) {
		    in = no; no = ea; ea = so; so = we; we = in;
		}
	    }
	    /*--- message ---*/
	    sprintf( message,"     subiteration # %d, deleted points = %d", cycle+1, nbd );
	    VT_Message( message, "_VT_DST_GREG_2D" );
	}
	/*--- message ---*/
	sprintf( message," iteration, deleted points = %d", nbdel );
	VT_Message( message, "_VT_DST_GREG_2D" );
    } while ( (nbdel > 0) && (first != ln) );
    VT_Free( (void**)&liste );
    return( 1 );
}
