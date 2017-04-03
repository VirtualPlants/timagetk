
#include <vt_ambdd.h>





/* Procedure d'aminicissement avec les BBDs.

   On suppose l'image deja etiquettee (les points
   de l'objet ont une valeur non nulle, ceux du
   fond une valeur nulle).

   La liste de point est de type vt_pt_amincir*.

   Options :
 
   1. shrinking

   2. deux conditions d'arret possibles : fin de 
   surface (condition de Gong et Bertrand et fin
   de courbe). 

   3. rien de particulier
*/

int _VT_BDD_THINNING( vt_image *im         /* image to be thinned */,
                  vt_pt_amincir *liste /* list of points which can be deleted */,
                  int *lnb             /* number of points which can be deleted */,
                  vt_amincir *par      /* parameters */ )
{
    vt_pt_amincir point;
    int ln;
    int epais, epais_max;
    int bool_continue=0, bool_shrinking=0;
    int *in, *up, *no, *ea, *bo, *so, *we;
    int tup[3], tno[3], tea[3], tbo[3], tso[3], twe[3];
    int v[3][3][3], w[3][3][3], voisins[26];
    register u8 ***pb;
    register int i, j, k, l;
    int nbPerm, cycle, nbd, nbdel, npt;
    char message[256], *name_proc="_VT_BDD_THINNING";

    /*--- tests ---*/
    if ( VT_Test1Image( im, name_proc ) == -1 ) return( -1 );
    if ( (im->type != UCHAR) ) return( -1 );
    if ( *lnb <= 0 ) return( 0 );
    
    /*--- epaisseurs ---*/
    epais_max = epais = N06;
    switch( par->epaisseur ) {
    case N26 :
      epais_max = N26;
      break;
   case N18 :
      epais_max = N18;
      break;
    case N06 :
    default :
      epais_max = N06;
    }
    if ( par->connexite == N06 )
      epais_max = N06;

    /*--- conditions d'arret ---*/
    if ( par->bool_shrink == 1 ) bool_shrinking = 1;
    if ( (par->bool_end_curves != 1) && (par->bool_end_surfaces != 1) )
      bool_shrinking = 1;

    /*--- preparation ---*/
    pb = (unsigned char***)(im->array);
    tup[0] = 0;   tup[1] = 0;   tup[2] = 1;
    tno[0] = 0;   tno[1] = 1;   tno[2] = 0;
    tea[0] = 1;   tea[1] = 0;   tea[2] = 0;
    tbo[0] = 0;   tbo[1] = 0;   tbo[2] = -1;
    tso[0] = 0;   tso[1] = -1;  tso[2] = 0;
    twe[0] = -1;  twe[1] = 0;   twe[2] = 0;
    up = tup;   no = tno;   ea = tea;
    bo = tbo;   so = tso;   we = twe;
    nbPerm = 3;

    /*--- nombre de points a effacer ---*/
    ln = *lnb;

    /*--- boucle generale ---*/
    do {

      /*--- boucle des 6 sous-cycles = 6 directions ---*/
      nbdel = 0;
      for (cycle = 0; cycle < 6; cycle++ ) {
          nbd = 0;

          /*--- liste des points ---*/
          for ( l = 0; l < ln; l++ ) {

            /*--- traitement d'un point : saisie du voisinage ---*/
            if ( liste[l].inside == 1 ) {
                for ( k = -1; k < 2; k++ )
                for ( j = -1; j < 2; j++ )
                for ( i = -1; i < 2; i++ )
                  v[1+i][1+j][1+k] = (int)(pb[liste[l].pt.z + k][liste[l].pt.y + j][liste[l].pt.x + i]);
            } else {
                for ( k = -1; k < 2; k++ )
                for ( j = -1; j < 2; j++ )
                for ( i = -1; i < 2; i++ ) {
                  if ( (liste[l].pt.x+i >= 0) && (liste[l].pt.x+i < (int)im->dim.x) &&
                       (liste[l].pt.y+j >= 0) && (liste[l].pt.y+j < (int)im->dim.y) &&
                       (liste[l].pt.z+k >= 0) && (liste[l].pt.z+k < (int)im->dim.z) )
                      v[1+i][1+j][1+k] = (int)(pb[liste[l].pt.z + k][liste[l].pt.y + j][liste[l].pt.x + i]);
                  else
                      v[1+i][1+j][1+k] = (int)0;
                }
            }

            /*--- traitement d'un point :               ---*/
            /*--- si la condition 2a n'est pas verifiee ---*/
            /*--- point + up appartient au fond         ---*/
            if ( v[1+up[0]][1+up[1]][1+up[2]] != 0 ) continue;

            /*--- traitement d'un point :               ---*/
            /*--- si la condition 2b n'est pas verifiee ---*/
            /*--- epaisseur = N06                                     ---*/
            /*----          point + bo appartient a l'objet                    ---*/
            /*--- epaisseur = N18                                     ---*/
            /*---           un des 5 points ( bo, bo-no, bo-ea, bo-so, bo-we ) ---*/
            /*---           appartient a l'objet                               ---*/
            /*--- epaisseur = N26                                     ---*/
            /*----          un des 9 points ( bo, bo-no, bo-ea, bo-so, bo-we ) ---*/
            /*---           et ( bo-no-ea, bo-ea-so, bo-so-we, bo-we-no )      ---*/
            /*---           appartient a l'objet                               ---*/
            bool_continue = 0;
            switch ( epais ) {
            case N26 :
                if ( ( v[1+bo[0]][1+bo[1]][1+bo[2]] == 0 ) &&
                   ( v[1+bo[0]+no[0]][1+bo[1]+no[1]][1+bo[2]+no[2]] == 0 ) &&
                   ( v[1+bo[0]+ea[0]][1+bo[1]+ea[1]][1+bo[2]+ea[2]] == 0 ) &&
                   ( v[1+bo[0]+so[0]][1+bo[1]+so[1]][1+bo[2]+so[2]] == 0 ) &&
                   ( v[1+bo[0]+we[0]][1+bo[1]+we[1]][1+bo[2]+we[2]] == 0 ) &&
                   ( v[1+bo[0]+no[0]+ea[0]][1+bo[1]+no[1]+ea[1]][1+bo[2]+no[2]+ea[2]] == 0 ) &&
                   ( v[1+bo[0]+ea[0]+so[0]][1+bo[1]+ea[1]+so[1]][1+bo[2]+ea[2]+so[2]] == 0 ) &&
                   ( v[1+bo[0]+so[0]+we[0]][1+bo[1]+so[1]+we[1]][1+bo[2]+so[2]+we[2]] == 0 ) &&
                   ( v[1+bo[0]+we[0]+no[0]][1+bo[1]+we[1]+no[1]][1+bo[2]+we[2]+no[2]] == 0 ) )
                  bool_continue = 1;
                break;
            case N18 :
                if ( ( v[1+bo[0]][1+bo[1]][1+bo[2]] == 0 ) &&
                   ( v[1+bo[0]+no[0]][1+bo[1]+no[1]][1+bo[2]+no[2]] == 0 ) &&
                   ( v[1+bo[0]+ea[0]][1+bo[1]+ea[1]][1+bo[2]+ea[2]] == 0 ) &&
                   ( v[1+bo[0]+so[0]][1+bo[1]+so[1]][1+bo[2]+so[2]] == 0 ) &&
                   ( v[1+bo[0]+we[0]][1+bo[1]+we[1]][1+bo[2]+we[2]] == 0 ) )
                  bool_continue = 1;
                break;
            case N06 :
            default :
                if ( v[1+bo[0]][1+bo[1]][1+bo[2]] == 0 )
                  bool_continue = 1;
            }
            if ( bool_continue == 1 ) continue;

            /*--- traitement d'un point :                  ---*/
            /*--- on change de representation de voisinage ---*/
            /*--- N06 : on "inverse" les points             ---*/
            /*--- N26 et defaut : on garde les memes points ---*/
            if ( par->connexite == N06 ) {
                voisins[ 0] = !v[0][0][0];  voisins[ 1] = !v[1][0][0];   voisins[ 2] = !v[2][0][0];
                voisins[ 3] = !v[0][1][0];  voisins[ 4] = !v[1][1][0];   voisins[ 5] = !v[2][1][0];
                voisins[ 6] = !v[0][2][0];  voisins[ 7] = !v[1][2][0];   voisins[ 8] = !v[2][2][0];
                voisins[ 9] = !v[0][0][1];  voisins[10] = !v[1][0][1];   voisins[11] = !v[2][0][1];
                voisins[12] = !v[0][1][1];                               voisins[13] = !v[2][1][1];
                voisins[14] = !v[0][2][1];  voisins[15] = !v[1][2][1];   voisins[16] = !v[2][2][1];
                voisins[17] = !v[0][0][2];  voisins[18] = !v[1][0][2];   voisins[19] = !v[2][0][2];
                voisins[20] = !v[0][1][2];  voisins[21] = !v[1][1][2];   voisins[22] = !v[2][1][2];
                voisins[23] = !v[0][2][2];  voisins[24] = !v[1][2][2];   voisins[25] = !v[2][2][2];
            } else {
                voisins[ 0] = v[0][0][0];  voisins[ 1] = v[1][0][0];   voisins[ 2] = v[2][0][0];
                voisins[ 3] = v[0][1][0];  voisins[ 4] = v[1][1][0];   voisins[ 5] = v[2][1][0];
                voisins[ 6] = v[0][2][0];  voisins[ 7] = v[1][2][0];   voisins[ 8] = v[2][2][0];
                voisins[ 9] = v[0][0][1];  voisins[10] = v[1][0][1];   voisins[11] = v[2][0][1];
                voisins[12] = v[0][1][1];                              voisins[13] = v[2][1][1];
                voisins[14] = v[0][2][1];  voisins[15] = v[1][2][1];   voisins[16] = v[2][2][1];
                voisins[17] = v[0][0][2];  voisins[18] = v[1][0][2];   voisins[19] = v[2][0][2];
                voisins[20] = v[0][1][2];  voisins[21] = v[1][1][2];   voisins[22] = v[2][1][2];
                voisins[23] = v[0][2][2];  voisins[24] = v[1][2][2];   voisins[25] = v[2][2][2];
            }

            /*--- traitement d'un point :                  ---*/
            /*--- ce n'est pas un point simple             ---*/
            if ( _VT_IsSimple( voisins ) != 1 ) continue;

            /*--- on efface les points qui vont disparaitre ---*/
            for ( i = 0; i < 3; i++ )
            for ( j = 0; j < 3; j++ )
            for ( k = 0; k < 3; k++ )
                w[i][j][k] = ( v[i][j][k] == VT_WILLBEDELETED ) ? 0 : v[i][j][k];

            /*--- on change de representation de voisinage ---*/
            if ( par->connexite == N06 ) {
                voisins[ 0] = !w[0][0][0];  voisins[ 1] = !w[1][0][0];   voisins[ 2] = !w[2][0][0];
                voisins[ 3] = !w[0][1][0];  voisins[ 4] = !w[1][1][0];   voisins[ 5] = !w[2][1][0];
                voisins[ 6] = !w[0][2][0];  voisins[ 7] = !w[1][2][0];   voisins[ 8] = !w[2][2][0];
                voisins[ 9] = !w[0][0][1];  voisins[10] = !w[1][0][1];   voisins[11] = !w[2][0][1];
                voisins[12] = !w[0][1][1];                               voisins[13] = !w[2][1][1];
                voisins[14] = !w[0][2][1];  voisins[15] = !w[1][2][1];   voisins[16] = !w[2][2][1];
                voisins[17] = !w[0][0][2];  voisins[18] = !w[1][0][2];   voisins[19] = !w[2][0][2];
                voisins[20] = !w[0][1][2];  voisins[21] = !w[1][1][2];   voisins[22] = !w[2][1][2];
                voisins[23] = !w[0][2][2];  voisins[24] = !w[1][2][2];   voisins[25] = !w[2][2][2];
            } else {
                voisins[ 0] = w[0][0][0];  voisins[ 1] = w[1][0][0];   voisins[ 2] = w[2][0][0];
                voisins[ 3] = w[0][1][0];  voisins[ 4] = w[1][1][0];   voisins[ 5] = w[2][1][0];
                voisins[ 6] = w[0][2][0];  voisins[ 7] = w[1][2][0];   voisins[ 8] = w[2][2][0];
                voisins[ 9] = w[0][0][1];  voisins[10] = w[1][0][1];   voisins[11] = w[2][0][1];
                voisins[12] = w[0][1][1];                              voisins[13] = w[2][1][1];
                voisins[14] = w[0][2][1];  voisins[15] = w[1][2][1];   voisins[16] = w[2][2][1];
                voisins[17] = w[0][0][2];  voisins[18] = w[1][0][2];   voisins[19] = w[2][0][2];
                voisins[20] = w[0][1][2];  voisins[21] = w[1][1][2];   voisins[22] = w[2][1][2];
                voisins[23] = w[0][2][2];  voisins[24] = w[1][2][2];   voisins[25] = w[2][2][2];
            }

            /*--- ce n'est pas un point simple                   ---*/
            if ( _VT_IsSimple( voisins ) != 1 ) continue;

            /*--- si on est arrive ici, c'est un point effacable ---*/
            /*--- 1. if shrinking                                ---*/
            if ( bool_shrinking == 1 ) {
                liste[l].status = VT_TOBEDELETED;
                pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                continue;
            }

            /*--- on compte les voisins ---*/
            npt = 0;
            for ( i = 0; i < 3; i++ )
            for ( j = 0; j < 3; j++ ) {
                if ( v[i][j][0] != 0 ) npt ++;
                if ( v[i][j][2] != 0 ) npt ++;
            }
            for ( i = 0; i < 3; i++ ) {
                if ( v[i][0][1] != 0 ) npt ++;
                if ( v[i][2][1] != 0 ) npt ++;
            }
            if ( v[0][1][1] != 0 ) npt ++;
            if ( v[2][1][1] != 0 ) npt ++;

            /*--- 2. si on conserve les fins de courbes (1 seul voisin) ---*/
            if ( (par->bool_end_curves == 1) && (npt == 1) ) {
                liste[l].status = VT_UNDELETABLE;
                continue;
            }

            /*--- si on ne conserve pas les bords de surfaces :
                  alors on peut effacer                         ---*/
            if ( par->bool_end_surfaces != 1 ) {
                liste[l].status = VT_TOBEDELETED;
                pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                continue;
            }

            /*--- 3. on conserve les bords de surfaces (condition Gong-Bertrand) ---*/

            /*--- Remarque :                                   ---*/
            /*--- Selon le voisinage sur lequel on fait porter ---*/
            /*--- cette condition (v ou w = v - pts effaces)   ---*/
            /*--- le resultat est different.                   ---*/
            /*--- En choisissant v, le resultat semble moins   ---*/
            /*--- "bruite" (moins de branches parasites).      ---*/

            if ( npt < 4 ) {
                liste[l].status = VT_DELETABLE;
                continue;
            }

            if ( npt >= 8 ) {
                liste[l].status = VT_TOBEDELETED;
                pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                continue;
            }

            if ( ( npt >= 4 ) && ( npt <= 7 ) ) {
                if ( ( v[1+up[0]][1+up[1]][1+up[2]] != 0 ) &&
                   ( v[1+no[0]][1+no[1]][1+no[2]] != 0 ) &&
                   ( v[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
                if ( ( v[1+up[0]][1+up[1]][1+up[2]] != 0 ) &&
                   ( v[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) &&
                   ( v[1+so[0]][1+so[1]][1+so[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
                if ( ( v[1+up[0]][1+up[1]][1+up[2]] != 0 ) &&
                   ( v[1+so[0]][1+so[1]][1+so[2]] != 0 ) &&
                   ( v[1+we[0]][1+we[1]][1+we[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
                if ( ( v[1+up[0]][1+up[1]][1+up[2]] != 0 ) &&
                   ( v[1+we[0]][1+we[1]][1+we[2]] != 0 ) &&
                   ( v[1+no[0]][1+no[1]][1+no[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
                if ( ( v[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
                   ( v[1+no[0]][1+no[1]][1+no[2]] != 0 ) &&
                   ( v[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
                if ( ( v[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
                   ( v[1+ea[0]][1+ea[1]][1+ea[2]] != 0 ) &&
                   ( v[1+so[0]][1+so[1]][1+so[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
                if ( ( v[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
                   ( v[1+so[0]][1+so[1]][1+so[2]] != 0 ) &&
                   ( v[1+we[0]][1+we[1]][1+we[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
                if ( ( v[1+bo[0]][1+bo[1]][1+bo[2]] != 0 ) &&
                   ( v[1+we[0]][1+we[1]][1+we[2]] != 0 ) &&
                   ( v[1+no[0]][1+no[1]][1+no[2]] != 0 ) ) {
                  liste[l].status = VT_TOBEDELETED;
                  pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = VT_WILLBEDELETED;
                  continue;
                }
            }
            liste[l].status = VT_DELETABLE;
          }
          /*--- fin du traitement de la liste ---*/

          /*--- on modifie l'image et on rearrange la liste ---*/
          i = 0;
          for ( l = 0; l < ln; l++ ) {
            switch ( liste[l].status ) {
            case VT_DELETABLE :
                /*--- on garde le point dans la liste :
                      on swappe les points pour tout
                    garder dans la liste              ---*/
                point    = liste[i];
                liste[i] = liste[l];
                liste[l] = point;
                i++;
                break;
            case VT_TOBEDELETED :
                /*--- on efface le point ---*/
                pb[ liste[l].pt.z ][ liste[l].pt.y ][ liste[l].pt.x ] = (u8)0;
                nbd ++;
                break;
            case VT_UNDELETABLE :
                /*--- on ne fait rien ---*/
                break;
            default :
                /*--- on s'inquiete ---*/
                VT_Error( "point with unknown label", name_proc );
            }
          }

          /*--- on repart pour un sous-sycle ---*/
          ln = i;
          nbdel += nbd;
          for ( i = 0; i < nbPerm; i++ ) {
            in = up;
            up = no;
            no = ea;
            ea = bo;
            bo = so;
            so = we;
            we = in;
          }
          if (nbPerm == 3) nbPerm = 4;
          else nbPerm = 3;
          if (cycle == 4) nbPerm = 1;

          /*--- message ---*/
          sprintf( message,"     subiteration # %d, deleted points = %d", cycle+1, nbd );
          VT_Message( message, name_proc );
      }
      /*--- fin d'un cycle = 6 sous-cycle ---*/

      /*--- message ---*/
      sprintf( message," iteration, deleted points = %d", nbdel );
      VT_Message( message, name_proc );
    
      /*--- fin ? :
            1. on teste s'il reste des points
            2. on teste si des points ont ete effaces :
               on repart alors d'une 6-epaisseur
              3. on regarde si on peut changer d'epaisseur ---*/ 
      if ( ln <= 0 ) {
          bool_continue = 0;
      } else if ( nbdel > 0 ) {
          bool_continue = 1;
          epais = N06;
      } else {
          switch ( epais_max ) {
          case N26 :
            if ( epais == N06 ) {
                epais = N18;
            } else if ( epais == N18 ) {
                epais = N26;
            } else {
                bool_continue = 0;
            }
            break;
          case N18 :
            if ( epais == N06 ) {
                epais = N18;
            } else {
                bool_continue = 0;
            }
            break;
          case N06 :
          default :
            bool_continue = 0;
          }
      }

    } while ( bool_continue == 1 );

    /*--- retours ---*/
    *lnb = ln;
    return( 1 );
}
