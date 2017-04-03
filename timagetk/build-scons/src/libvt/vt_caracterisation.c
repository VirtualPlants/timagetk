
#include <vt_caracterisation.h>


#define _VT_BACK18 200
#define _VT_BACK06 100

/*------- Definition des fonctions statiques ----------*/

static int _VT_CaracterisationComplete( vt_image *im1, vt_image *im2, vt_caracterisation *par );
static int _VT_CCin5x5x5Neighborhood( int *vois );





void VT_InitCaracterisation( vt_caracterisation *par /* characterization parameters */ )
{
    par->bool_extended_junctions = 0;
    par->connexite = N26;
}





/* Caracterisation topologique d'une image sans a priori sur son contenu.

   L'image d'entree doit etre codee en unsigned char (type UCHAR)
   ainsi que l'image de sortie.

   Les points definissant l'objet sont ceux ayant une valeur non
   nulle.

   La caracterisation est realisee en comptant des nombres de composantes
   connexes dans des voisinages des points de l'objet.

   Si le flag bool_extended_junctions est a 1, les points
   frontieres (points simples) 26-voisins de points de jonctions
   entre surfaces
   sont consideres comme etant des points de jonctions.

   Dans l'image resultat, les points ont les valeurs suivantes :
   - VT_JUNCTION pour les points de jonction
   - VT_SIMPLE   pour les points simples (points frontieres)
   - VT_CC       pour les autres
 
*/

int VT_Caracterisation( vt_image *im1 /* image result */,
                  vt_image *im2 /* image to be topologically characterized */,
                  vt_caracterisation *par /* characterization parameters */ )
{  
    unsigned char *buf;
    register int ind, x, y, z;

    if ( VT_Test2Image( im1, im2, "VT_Caracterisation" ) == -1 ) return( -1 );
    if ( im2->type != (int)UCHAR ) {
      VT_Error("output image has bad type","VT_Caracterisation");
      return( -1 );
    }
    
    if ( _VT_CaracterisationComplete( im1, im2, par ) != 1 ) {
      VT_Error("first characterization has failed","VT_Caracterisation");
      return( -1 );
    }

    /* ATTENTION : il faut considerer les points de volumes comme points de jonction */

    buf = (unsigned char*)(im2->buf);
    ind = 0;

    /*--- seconde caracterisation (en trois classes) ---*/
    for ( z = 0; z < (int)im2->dim.z; z++ )
    for ( y = 0; y < (int)im2->dim.y; y++ )
    for ( x = 0; x < (int)im2->dim.x; x++ ) {

      switch ( buf[ind] ) {
      case VT_LABEL_10 : /* point isole      */
      case VT_LABEL_12 : /* point de courbe  */
      case VT_LABEL_21 : /* point de surface */
          buf[ind] = (unsigned char)VT_CC;
          break;
      case VT_LABEL_11 : /* point simple (frontiere) */
          buf[ind] = (unsigned char)VT_SIMPLE;
          break;
      case VT_LABEL_01 :  /* point interieur volume                               */
      case VT_LABEL_13 :  /* point jonction entre 3 courbes                       */
      case VT_LABEL_14 :  /* point jonction entre 4 courbes                       */
      case VT_LABEL_15 :  /* point jonction entre 5 courbes                       */
      case VT_LABEL_16 :  /* point jonction entre 6 courbes                       */
      case VT_LABEL_17 :  /* point jonction entre 7 courbes                       */
      case VT_LABEL_18 :  /* point jonction entre 8 courbes                       */
      case VT_LABEL_22 :  /* point jonction entre 1 surface et 1 courbe           */
      case VT_LABEL_23 :  /* point jonction entre 1 surface et 2 courbes          */
      case VT_LABEL_24 :  /* point jonction entre 1 surface et 3 courbes          */
      case VT_LABEL_25 :  /* point jonction entre 1 surface et 4 courbes          */
      case VT_LABEL_31 :  /* point jonction entre 3 surfaces                      */
      case VT_LABEL_32 :  /* point jonction entre 3 surfaces et 1 courbe          */
      case VT_LABEL_33 :  /* point jonction entre 3 surfaces et 2 courbes         */
      case VT_LABEL_41 :  /* point jonction entre 4 surfaces                      */
      case VT_LABEL_42 :  /* point jonction entre 4 surfaces et 1 courbe          */
      case VT_LABEL_51 :  /* point jonction entre 5 surfaces                      */
      case VT_LABEL_61 :  /* point jonction entre 6 surfaces                      */
      case VT_LABEL_11J : /* point frontiere voisin d'une jonction entre surfaces */
      case VT_LABEL_12J : /* point jonction entre plusieurs courbes               */
      case VT_LABEL_21J : /* point jonction entre plusieurs surfaces              */
      case VT_LABEL_22J : /* point jonction entre plusieurs surfaces              */
      case VT_LABEL_23J : /* point jonction entre plusieurs surfaces              */
      case VT_LABEL_24J : /* point jonction entre plusieurs surfaces              */
      case VT_LABEL_25J : /* point jonction entre plusieurs surfaces              */
          buf[ind] = (unsigned char)VT_JUNCTION;
          break;
      case 0 :  /* point du fond */
      default : /* point inconnu */
          buf[ind] = (unsigned char)0;
      }
      ind ++;
    }

    return( 1 );
}





/* Caracterisation topologique d'une image sans a priori sur son contenu.

   L'image d'entree doit etre codee en unsigned char (type UCHAR)
   ainsi que l'image de sortie.

   Les points definissant l'objet sont ceux ayant une valeur non
   nulle.

   La caracterisation est realisee en comptant des nombres de composantes
   connexes dans des voisinages des points de l'objet.

   Si le flag bool_extended_junctions est a 1, les points
   frontieres (points simples) 26-voisins de points de jonctions
   entre surfaces
   sont consideres comme etant des points de jonctions.

   Dans l'image resultat, l'eventail des valeurs est tres grand.
 
*/

int VT_CaracterisationComplete( vt_image *im1 /* image result */,
                  vt_image *im2 /* image to be topologically characterized */,
                  vt_caracterisation *par /* characterization parameters */ )
{  
    unsigned char *buf;
    register int ind, x, y, z;

    if ( VT_Test2Image( im1, im2, "VT_CaracterisationComplete" ) == -1 ) return( -1 );
    if ( im2->type != (int)UCHAR ) {
      VT_Error("output image has bad type","VT_CaracterisationComplete");
      return( -1 );
    }
    
    if ( _VT_CaracterisationComplete( im1, im2, par ) != 1 ) {
      VT_Error("first characterization has failed","VT_Caracterisation");
      return( -1 );
    }

    /* ATTENTION : il faut considerer les points de volumes comme points de jonction */

    buf = (unsigned char*)(im2->buf);
    ind = 0;

    /*--- seconde caracterisation ---*/
    for ( z = 0; z < (int)im2->dim.z; z++ )
    for ( y = 0; y < (int)im2->dim.y; y++ )
    for ( x = 0; x < (int)im2->dim.x; x++ ) {

      switch ( buf[ind] ) {
      case VT_LABEL_01 : buf[ind] = (unsigned char)1;   break;
      case VT_LABEL_10 : buf[ind] = (unsigned char)10;   break;
      case VT_LABEL_11 : buf[ind] = (unsigned char)11;   break;
      case VT_LABEL_12 : buf[ind] = (unsigned char)12;   break;
      case VT_LABEL_13 : buf[ind] = (unsigned char)13;   break;
      case VT_LABEL_14 : buf[ind] = (unsigned char)14;   break;
      case VT_LABEL_15 : buf[ind] = (unsigned char)15;   break;
      case VT_LABEL_16 : buf[ind] = (unsigned char)16;   break;
      case VT_LABEL_17 : buf[ind] = (unsigned char)17;   break;
      case VT_LABEL_18 : buf[ind] = (unsigned char)18;   break;
      case VT_LABEL_21 : buf[ind] = (unsigned char)21;   break;
      case VT_LABEL_22 : buf[ind] = (unsigned char)22;   break;
      case VT_LABEL_23 : buf[ind] = (unsigned char)23;   break;
      case VT_LABEL_24 : buf[ind] = (unsigned char)24;   break;
      case VT_LABEL_25 : buf[ind] = (unsigned char)25;   break;
      case VT_LABEL_31 : buf[ind] = (unsigned char)31;   break;
      case VT_LABEL_32 : buf[ind] = (unsigned char)32;   break;
      case VT_LABEL_33 : buf[ind] = (unsigned char)33;   break;
      case VT_LABEL_41 : buf[ind] = (unsigned char)41;   break;
      case VT_LABEL_42 : buf[ind] = (unsigned char)42;   break;
      case VT_LABEL_51 : buf[ind] = (unsigned char)51;   break;
      case VT_LABEL_61 : buf[ind] = (unsigned char)61;   break;
      case VT_LABEL_11J : buf[ind] = (unsigned char)111;   break;
      case VT_LABEL_12J : buf[ind] = (unsigned char)112;   break;
      case VT_LABEL_21J : buf[ind] = (unsigned char)121;   break;
      case VT_LABEL_22J : buf[ind] = (unsigned char)122;   break;
      case VT_LABEL_23J : buf[ind] = (unsigned char)123;   break;
      case VT_LABEL_24J : buf[ind] = (unsigned char)124;   break;
      case VT_LABEL_25J : buf[ind] = (unsigned char)125;   break;
      case 0 :  /* point du fond */
      default : /* point inconnu */
          buf[ind] = (unsigned char)0;
      }
      ind ++;
    }

    return( 1 );
}





static int _VT_CaracterisationComplete( vt_image *im1 /* image result */,
                  vt_image *im2 /* image to be topologically characterized */,
                  vt_caracterisation *par /* characterization parameters */ )
{  
    vt_image imaux;
    unsigned char *buf1, *buf2, *buf;
    register int ind, i, j, k, x, y, z;
    int in;
    int offset[3][3][3], off[3][3][3], OFFSET[5][5][5];
    int v[3][3][3], voisins[27], VOISINS[125], BACKGROUND[125];
    int dx, dy, dz, dxy, dx1, dy1, dz1, dx2, dy2, dz2;
    int C_etoile, C_barre, nb, nb_voisins, distance, flag;
    int nb_points[2][7][9];

    /*--- ---*/
    switch ( par->connexite ) {
    case N18 :
      nb_voisins = 18;
      break;
    case N26 :
      nb_voisins = 26;
      break;
    default :
      VT_Error("bad connectivity in parameters","_VT_CaracterisationComplete");
      return( -1 );
    }

    /*--- initialisation ---*/
    for (i=0; i<7; i++)
    for (j=0; j<9; j++)
      nb_points[0][i][j] = nb_points[1][i][j] = 0;

    /*--- tests ---*/
    if ( VT_Test2Image( im1, im2, "_VT_CaracterisationComplete" ) == -1 ) return( -1 );
    if ( im1->type != (int)UCHAR ) {
      VT_Error("input image has bad type","_VT_CaracterisationComplete");
      return( -1 );
    }
    if ( im2->type != (int)UCHAR ) {
      VT_Error("output image has bad type","_VT_CaracterisationComplete");
      return( -1 );
    }
    
    /*--- initialisation de l'image auxilliaire ---*/
    VT_Image( &imaux );
    VT_InitImage( &imaux, (char*)NULL, im2->dim.x, im2->dim.y, im2->dim.z, (int)UCHAR );
    if ( VT_AllocImage( &imaux ) != 1 ) {
      VT_Error("unable to allocate auxiliary image","_VT_CaracterisationComplete");
      return( -1 );
    }

    /*--- preparation ---*/
    buf1 = (unsigned char*)(im1->buf);
    buf2 = (unsigned char*)(im2->buf);
    buf  = (unsigned char*)(imaux.buf);
    dx = im2->dim.x;   dx1 = dx - 1;   dx2 = dx - 2;
    dy = im2->dim.y;   dy1 = dy - 1;   dy2 = dy - 2;
    dz = im2->dim.z;   dz1 = dz - 1;   dz2 = dz - 2;
    dxy = im2->dim.x * im2->dim.y;
    /*--- petit tableau d'offset ---*/
    for ( i = 0; i < 3; i++ )
      offset[i][1][1] = off[i][1][1] = i - 1;
    for ( i = 0; i < 3; i++ ) {
      offset[i][0][1] = offset[i][1][1] - dx;
      offset[i][2][1] = offset[i][1][1] + dx;
      off[i][0][1]    = off[i][1][1]    - 5;
      off[i][2][1]    = off[i][1][1]    + 5;
    }
    for ( i = 0; i < 3; i++ )
    for ( j = 0; j < 3; j++ ) {
      offset[i][j][0] = offset[i][j][1] - dxy;
      offset[i][j][2] = offset[i][j][1] + dxy;
      off[i][j][0]    = off[i][j][1]    - 25;
      off[i][j][2]    = off[i][j][1]    + 25;
    }
    /*--- grand tableau d'offset ---*/
    for ( i = 0; i < 5; i++ )
      OFFSET[i][2][2] = i - 2;
    for ( j = 1; j < 3; j++ ) 
    for ( i = 0; i < 5; i++ ) {
      OFFSET[i][2-j][2] = OFFSET[i][2][2] - j * dx;
      OFFSET[i][2+j][2] = OFFSET[i][2][2] + j * dx;
    }
    for ( k = 1; k < 3; k++ ) 
    for ( j = 0; j < 5; j++ ) 
    for ( i = 0; i < 5; i++ ) {
      OFFSET[i][j][2-k] = OFFSET[i][j][2] - k * dxy;
      OFFSET[i][j][2+k] = OFFSET[i][j][2] + k * dxy;
    }

    ind = 0;
    /*--- premiere caracterisation (complete) ---*/
    for ( z = 0; z < (int)im2->dim.z; z++ )
    for ( y = 0; y < (int)im2->dim.y; y++ )
    for ( x = 0; x < (int)im2->dim.x; x++ ) {
      if ( buf2[ind] <= 0 ) {
          buf[ind] = (unsigned char)0;
          ind ++;
          continue;
      }

      /*--- on separe les cas "bord de l'image" des autres ---*/
      /*--- a l'interieur de l'image, on ne teste pas      ---*/
      if ( ( x > 0 ) && ( x < dx1 ) &&
           ( y > 0 ) && ( y < dy1 ) &&
           ( z > 0 ) && ( z < dz1 ) ) {
          for ( i = 0; i < 3; i++ )
            for ( j = 0; j < 3; j++ ) 
          for ( k = 0; k < 3; k++ ) {
            v[i][j][k] = (int)(buf2[ ind + offset[i][j][k] ]);
          }
      } else {
          for ( i = -1; i < 2; i++ )
            for ( j = -1; j < 2; j++ ) 
            for ( k = -1; k < 2; k++ ) {
            if ( (x+i >= 0) && (x+i < dx) &&
                 (y+j >= 0) && (y+j < dy) &&
                 (z+k >= 0) && (z+k < dz) ) {
                v[1+i][1+j][1+k] = (int)(buf2[ ind + offset[1+i][1+j][1+k] ]);
            } else {
                v[1+i][1+j][1+k] = (int)0;
            }
          }
      }

      /*--- on change de representation de voisinage ---*/
      voisins[ 0] = v[0][0][0];  voisins[ 1] = v[1][0][0];   voisins[ 2] = v[2][0][0];
      voisins[ 3] = v[0][1][0];  voisins[ 4] = v[1][1][0];   voisins[ 5] = v[2][1][0];
      voisins[ 6] = v[0][2][0];  voisins[ 7] = v[1][2][0];   voisins[ 8] = v[2][2][0];
      voisins[ 9] = v[0][0][1];  voisins[10] = v[1][0][1];   voisins[11] = v[2][0][1];
      voisins[12] = v[0][1][1];  voisins[13] = v[1][1][1];   voisins[14] = v[2][1][1];
      voisins[15] = v[0][2][1];  voisins[16] = v[1][2][1];   voisins[17] = v[2][2][1];
      voisins[18] = v[0][0][2];  voisins[19] = v[1][0][2];   voisins[20] = v[2][0][2];
      voisins[21] = v[0][1][2];  voisins[22] = v[1][1][2];   voisins[23] = v[2][1][2];
      voisins[24] = v[0][2][2];  voisins[25] = v[1][2][2];   voisins[26] = v[2][2][2];

      /*--- on compte des composantes connexes ---*/
      /*
      C_etoile = _VT_26CCin26V( voisins );
      C_barre  = _VT_06CCin18V( voisins );
      */
      /*--- on utilise des procedures plus adaptees et plus rapides ---*/
      C_etoile = VT_ComputeCetoile( voisins );
      C_barre  = VT_ComputeCbarre( voisins );

      i = C_barre * 10 + C_etoile;

      /*--- on compte ---*/
      nb_points[0][C_barre][C_etoile] ++;

      /*--- on donne une valeur ---*/
      switch ( i ) {
      case 1 :
          buf[ind] = (unsigned char)VT_LABEL_01;   break;
      case 10 :
          buf[ind] = (unsigned char)VT_LABEL_10;   break;
      case 11 :
          buf[ind] = (unsigned char)VT_LABEL_11;   break;
      case 12 :
          buf[ind] = (unsigned char)VT_LABEL_12;   break;
      case 13 :
          buf[ind] = (unsigned char)VT_LABEL_13;   break;
      case 14 :
          buf[ind] = (unsigned char)VT_LABEL_14;   break;
      case 15 :
          buf[ind] = (unsigned char)VT_LABEL_15;   break;
      case 16 :
          buf[ind] = (unsigned char)VT_LABEL_16;   break;
      case 17 :
          buf[ind] = (unsigned char)VT_LABEL_17;   break;
      case 18 :
          buf[ind] = (unsigned char)VT_LABEL_18;   break;
      case 21 :
          buf[ind] = (unsigned char)VT_LABEL_21;   break;
      case 22 :
          buf[ind] = (unsigned char)VT_LABEL_22;   break;
      case 23 :
          buf[ind] = (unsigned char)VT_LABEL_23;   break;
      case 24 :
          buf[ind] = (unsigned char)VT_LABEL_24;   break;
      case 25 :
          buf[ind] = (unsigned char)VT_LABEL_25;   break;
      case 31 :
          buf[ind] = (unsigned char)VT_LABEL_31;   break;
      case 32 :
          buf[ind] = (unsigned char)VT_LABEL_32;   break;
      case 33 :
          buf[ind] = (unsigned char)VT_LABEL_33;   break;
      case 41 :
          buf[ind] = (unsigned char)VT_LABEL_41;   break;
      case 42 :
          buf[ind] = (unsigned char)VT_LABEL_42;   break;
      case 51 :
          buf[ind] = (unsigned char)VT_LABEL_51;   break;
      case 61 :
          buf[ind] = (unsigned char)VT_LABEL_61;   break;
      default :
          buf[ind] = (unsigned char)0;
      }
      ind ++;
    }

    /*--- seconde caracterisation :
          on recupere les jonctions manquees qui seront labellisees 
        VT_LABEL_12J pour les points de courbe
        VT_LABEL_21J pour les points de surface                   ---*/

    /*--- on precalcule les indices des 27 voisins dans le voisinage 5x5x5 :
          dans l'ordre, le point central, les 6-voisins, les 18-voisins
          et les 26-voisins                                                  ---*/
    voisins[ 0] = 62;
    voisins[ 1] = 37;   voisins[ 2] = 57;   voisins[ 3] = 61;
    voisins[ 4] = 63;   voisins[ 5] = 67;   voisins[ 6] = 87;
    voisins[ 7] = 32;   voisins[ 8] = 36;   voisins[ 9] = 38;
    voisins[10] = 42;   voisins[11] = 56;   voisins[12] = 58;
    voisins[13] = 66;   voisins[14] = 68;   voisins[15] = 82;
    voisins[16] = 86;   voisins[17] = 88;   voisins[18] = 92;
    voisins[19] = 31;   voisins[20] = 33;   voisins[21] = 41;   voisins[22] = 43;
    voisins[23] = 81;   voisins[24] = 83;   voisins[25] = 91;   voisins[26] = 93;

    /*--- on re-parcourt l'image pour la seconde caracterisation ---*/
    ind = 0;
    for ( z = 0; z < (int)im2->dim.z; z++ )
    for ( y = 0; y < (int)im2->dim.y; y++ )
    for ( x = 0; x < (int)im2->dim.x; x++ ) {
      switch ( buf[ind] ) {
      case VT_LABEL_12 : /*--- point de courbe ---*/
          nb = 0;
          if ( ( x > 0 ) && ( x < dx1 ) &&
             ( y > 0 ) && ( y < dy1 ) &&
             ( z > 0 ) && ( z < dz1 ) ) {
            for ( i = 0; i < 3; i++ )
                for ( j = 0; j < 3; j++ ) 
            for ( k = 0; k < 3; k++ ) {
                if ( buf[ind + offset[i][j][k]] > 0 ) nb ++;
            }
          } else {
            for ( i = -1; i < 2; i++ )
                for ( j = -1; j < 2; j++ ) 
            for ( k = -1; k < 2; k++ ) {
                if ( (x+i >= 0) && (x+i < dx) &&
                   (y+j >= 0) && (y+j < dy) &&
                   (z+k >= 0) && (z+k < dz) ) {
                  if ( buf[ind + offset[1+i][1+j][1+k]] > 0 ) nb ++;
                }
            }
          }
          /*--- on a le nombre TOTAL de points dans le voisinage :
                le point central en fait partie                    ---*/
          if ( nb > 3 ) {
            buf[ind] = (unsigned char)VT_LABEL_12J;
            nb_points[0][1][2] --;
            nb_points[1][1][2] ++;
          }
          break;

      case VT_LABEL_21 : /*--- point de surface ---*/
      case VT_LABEL_22 :
      case VT_LABEL_23 :
      case VT_LABEL_24 :
      case VT_LABEL_25 :
          in = 0;
          /*--- 1. on initialise le fond ---*/
          for ( i = 0; i < 125; i++ ) BACKGROUND[i] = 0;

          /*--- 2. on recupere un grand voisinage 5x5x5 ---*/
          if ( ( x > 1 ) && ( x < dx2 ) &&
             ( y > 1 ) && ( y < dy2 ) &&
             ( z > 1 ) && ( z < dz2 ) ) {
            for ( k = 0; k < 5; k++ )
              for ( j = 0; j < 5; j++ )
              for ( i = 0; i < 5; i++ ) {
                VOISINS[in] = (int)buf[ind + OFFSET[i][j][k]];
                in ++;
            }
          }
          else {
            for ( k = -2; k < 3; k++ )
            for ( j = -2; j < 3; j++ )
              for ( i = -2; i < 3; i++ ) {
                if ( (x+i >= 0) && (x+i < dx) &&
                   (y+j >= 0) && (y+j < dy) &&
                   (z+k >= 0) && (z+k < dz) ) {
                  VOISINS[in] = (int)buf[ind + OFFSET[2+i][2+j][2+k]];
                }
                else {
                  VOISINS[in] = 0;
                }
                in ++;
            }
          }

          /*--- 3. dans ce voisinage, il faut garder les 6-composantes connexes
                du fond servant a la definition Cbarre == 2. Ce sont donc
              les points du fond dans les 18-voisinages des points de surface
              (Cbarre == 2) appartenant a une 6-composante connexe
              6-adjacente a ce point de surface. ATTENTION, ce ne sont donc
              pas tous les points du fond dans le 18-voisinage.               ---*/

          for ( i = 0; i <= nb_voisins; i++ ) {
            in = voisins[i];
            /*---  3.1. si c'est un point de surface */
            switch ( VOISINS[in] ) {
            case VT_LABEL_21 :
            case VT_LABEL_22 :
            case VT_LABEL_23 :
            case VT_LABEL_24 :
            case VT_LABEL_25 :
            case VT_LABEL_21J :
            case VT_LABEL_22J :
            case VT_LABEL_23J :
            case VT_LABEL_24J :
            case VT_LABEL_25J :

                /*--- 3.1 on recupere les 6-voisins ---*/
                if ( VOISINS[in + off[0][1][1]] == 0 ) BACKGROUND[in + off[0][1][1]] = _VT_BACK06;
                if ( VOISINS[in + off[2][1][1]] == 0 ) BACKGROUND[in + off[2][1][1]] = _VT_BACK06;
                if ( VOISINS[in + off[1][0][1]] == 0 ) BACKGROUND[in + off[1][0][1]] = _VT_BACK06;
                if ( VOISINS[in + off[1][2][1]] == 0 ) BACKGROUND[in + off[1][2][1]] = _VT_BACK06;
                if ( VOISINS[in + off[1][1][0]] == 0 ) BACKGROUND[in + off[1][1][0]] = _VT_BACK06;
                if ( VOISINS[in + off[1][1][2]] == 0 ) BACKGROUND[in + off[1][1][2]] = _VT_BACK06;

                /*--- 3.2 on recupere les 18-voisins ---*/
                if ( (VOISINS[in + off[0][0][1]] == 0) && (BACKGROUND[in + off[0][0][1]] != _VT_BACK06) )
                  BACKGROUND[in + off[0][0][1]] = _VT_BACK18;
                if ( (VOISINS[in + off[2][0][1]] == 0) && (BACKGROUND[in + off[2][0][1]] != _VT_BACK06) )
                  BACKGROUND[in + off[2][0][1]] = _VT_BACK18;
                if ( (VOISINS[in + off[0][2][1]] == 0) && (BACKGROUND[in + off[0][2][1]] != _VT_BACK06) )
                  BACKGROUND[in + off[0][2][1]] = _VT_BACK18;
                if ( (VOISINS[in + off[2][2][1]] == 0) && (BACKGROUND[in + off[2][2][1]] != _VT_BACK06) )
                  BACKGROUND[in + off[2][2][1]] = _VT_BACK18;

                if ( (VOISINS[in + off[1][0][0]] == 0) && (BACKGROUND[in + off[1][0][0]] != _VT_BACK06) )
                  BACKGROUND[in + off[1][0][0]] = _VT_BACK18;
                if ( (VOISINS[in + off[1][2][0]] == 0) && (BACKGROUND[in + off[1][2][0]] != _VT_BACK06) )
                  BACKGROUND[in + off[1][2][0]] = _VT_BACK18;
                if ( (VOISINS[in + off[1][0][2]] == 0) && (BACKGROUND[in + off[1][0][2]] != _VT_BACK06) )
                  BACKGROUND[in + off[1][0][2]] = _VT_BACK18;
                if ( (VOISINS[in + off[1][2][2]] == 0) && (BACKGROUND[in + off[1][2][2]] != _VT_BACK06) )
                  BACKGROUND[in + off[1][2][2]] = _VT_BACK18;

                if ( (VOISINS[in + off[0][1][0]] == 0) && (BACKGROUND[in + off[0][1][0]] != _VT_BACK06) )
                  BACKGROUND[in + off[0][1][0]] = _VT_BACK18;
                if ( (VOISINS[in + off[0][1][2]] == 0) && (BACKGROUND[in + off[0][1][2]] != _VT_BACK06) )
                  BACKGROUND[in + off[0][1][2]] = _VT_BACK18;
                if ( (VOISINS[in + off[2][1][0]] == 0) && (BACKGROUND[in + off[2][1][0]] != _VT_BACK06) )
                  BACKGROUND[in + off[2][1][0]] = _VT_BACK18;
                if ( (VOISINS[in + off[2][1][2]] == 0) && (BACKGROUND[in + off[2][1][2]] != _VT_BACK06) )
                  BACKGROUND[in + off[2][1][2]] = _VT_BACK18;
            }
          }
          /*--- 4. Plutot que de propager les labels _VT_BACK06 parmi les _VT_BACK18,
                     de mettre a 0 tout ce qui n'est pas _VT_BACK06,
                     et de compter les composantes _VT_BACK06,
                     il vaut mieux mettre a 0 tout ce qui n'est pas _VT_BACK06 ou _VT_BACK18,
                     et compter les composantes (_VT_BACK06 ou _VT_BACK18) reconstruites
                     a partir de _VT_BACK06.                                                  ---*/

          for ( i = 0; i < 125; i++ )
            if ( (BACKGROUND[i] != _VT_BACK06) && (BACKGROUND[i] != _VT_BACK18) ) BACKGROUND[i] = 0;

            /*--- fin du traitement des 27 points du centre ---*/

          /*--- maintenant, on compte les 6-composantes connexes de ce voisinage :
                  c'est le resultat de la reconstruction des points (_VT_BACK06 ou _VT_BACK18)
              a partir des points _VT_BACK06.                                              ---*/
          nb = _VT_CCin5x5x5Neighborhood( BACKGROUND );

          if ( nb > 2 ) {
            switch ( buf[ind] ) {
            case VT_LABEL_21 :
                buf[ind] = VT_LABEL_21J;
                nb_points[0][2][1] --;
                nb_points[1][2][1] ++;
                break;
            case VT_LABEL_22 :
                buf[ind] = VT_LABEL_22J;
                nb_points[0][2][2] --;
                nb_points[1][2][2] ++;
                break;
            case VT_LABEL_23 :
                buf[ind] = VT_LABEL_23J;
                nb_points[0][2][3] --;
                nb_points[1][2][3] ++;
                break;
            case VT_LABEL_24 :
                buf[ind] = VT_LABEL_24J;
                nb_points[0][2][4] --;
                nb_points[1][2][4] ++;
                break;
            case VT_LABEL_25 :
                buf[ind] = VT_LABEL_25J;
                nb_points[0][2][5] --;
                nb_points[1][2][5] ++;
                break;
            }
          }

          /*--- fin du traitement des points de surfaces ---*/
          break;

          /*--- Une remarque :
            Ici on collecte les 18-voisins des 9 points de surface du centre
            du voisinage 5x5x5, on marque leurs 6-voisins et on reconstruit
            les 6 composantes connexes parmi les 18-voisins a partir des
            6-voisins.
            Cette approche donne des resultats differents (moins de "nouvelles"
            jonctions sur un exemple) que si on trouve d'abord les 2 6-composantes
            du point de surface, qu'on collecte toutes les 6-composantes, et
            que l'on compte les 6-composantes du resultat de la collection.
            La premiere approche est plus rapide                                   ----*/

      }
      ind++;
    }

    if ( par->bool_extended_junctions == 1 ) {
      /*--- on rearrange les indices des voisins ---*/
      voisins[ 0] = offset[1][1][0];   voisins[ 1] = offset[1][0][1];
      voisins[ 2] = offset[0][1][1];   voisins[ 3] = offset[2][1][1];
      voisins[ 4] = offset[1][2][1];   voisins[ 5] = offset[1][1][2];
      voisins[ 6] = offset[1][0][0];   voisins[ 7] = offset[0][1][0];   voisins[ 8] = offset[2][1][0];
      voisins[ 9] = offset[1][2][0];   voisins[10] = offset[0][0][1];   voisins[11] = offset[2][0][1];
      voisins[12] = offset[0][2][1];   voisins[13] = offset[2][2][1];   voisins[14] = offset[1][0][2];
      voisins[15] = offset[0][1][2];   voisins[16] = offset[2][1][2];   voisins[17] = offset[1][2][1];
      voisins[18] = offset[0][0][0];   voisins[19] = offset[2][0][0];
      voisins[20] = offset[0][2][0];   voisins[21] = offset[2][2][0];
      voisins[22] = offset[0][0][2];   voisins[23] = offset[2][0][2];
      voisins[24] = offset[0][2][2];   voisins[25] = offset[2][2][2];

      /*--- les points frontieres voisins de points d'une
            jonction entre surfaces ont un nouveau label : 111 ---*/
      ind = 0;
      for ( z = 0; z < (int)im2->dim.z; z++ )
        for ( y = 0; y < (int)im2->dim.y; y++ )
        for ( x = 0; x < (int)im2->dim.x; x++ ) {
          if ( buf[ind] != (unsigned char)VT_LABEL_11 ) {
            ind ++;
            continue;
          }
          flag = 0;
          /*--- on traite le voisinage ---*/
          if ( ( x > 0 ) && ( x < dx1 ) &&
             ( y > 0 ) && ( y < dy1 ) &&
             ( z > 0 ) && ( z < dz1 ) ) {
            for ( i = 0; (i < nb_voisins) && (flag == 0); i++ ) {
                switch( buf[ ind + voisins[i] ] ) {
                case VT_LABEL_01 :
                case VT_LABEL_31 :
                case VT_LABEL_32 :
                case VT_LABEL_33 :
                case VT_LABEL_41 :
                case VT_LABEL_42 :
                case VT_LABEL_51 :
                case VT_LABEL_61 :
                case VT_LABEL_12J :
                case VT_LABEL_21J :
                case VT_LABEL_22J :
                case VT_LABEL_23J :
                case VT_LABEL_24J :
                case VT_LABEL_25J :
                  flag = 1;
                }
            }
          } else {
            for ( i = -1; i < 2; i++ )
                for ( j = -1; j < 2; j++ ) 
            for ( k = -1; k < 2; k++ ) {
                if ( (x+i >= 0) && (x+i < dx) &&
                   (y+j >= 0) && (y+j < dy) &&
                   (z+k >= 0) && (z+k < dz) ) {
                  distance = 0;
                  if ( i != 0 ) distance ++;
                  if ( j != 0 ) distance ++;
                  if ( k != 0 ) distance ++;
                  /*--- si la connexite est 26, ou si c'est un 18-voisin ---*/
                  if ( (par->connexite == N26) || (distance <= 2) ) {
                      switch( buf[ind + offset[1+i][1+j][1+k]] ) {
                      case VT_LABEL_01 :
                      case VT_LABEL_31 :
                      case VT_LABEL_32 :
                      case VT_LABEL_33 :
                      case VT_LABEL_41 :
                      case VT_LABEL_42 :
                      case VT_LABEL_51 :
                      case VT_LABEL_61 :
                      case VT_LABEL_12J :
                      case VT_LABEL_21J :
                      case VT_LABEL_22J :
                      case VT_LABEL_23J :
                      case VT_LABEL_24J :
                      case VT_LABEL_25J :
                        flag = 1;
                      }
                  }
                }
            }}
          /*--- on a traite le voisinage ---*/
          if ( flag == 1 ) {
            buf[ind] = (unsigned char)VT_LABEL_11J;
            nb_points[0][1][1] --;
            nb_points[1][1][1] ++;
          }
          ind ++;
      }
    }

    /*--- copie de l'image intermediaire ---*/
    for ( z = 0; z < (int)im2->dim.z; z++ )
    for ( y = 0; y < (int)im2->dim.y; y++ )
    for ( x = 0; x < (int)im2->dim.x; x++ ) {
      *buf1++ = *buf++;
    }

    VT_FreeImage( &imaux );


    /*--- faut-il ecrire ? ---*/
    if ( _VT_VERBOSE_ == 1 ) {

      if ( nb_points[0][1][1] > 0 )
          fprintf( stderr, " nombre de points simples (frontiere)                                   = %d\n", nb_points[0][1][1] );

      nb_points[0][0][0] = 0;
      nb_points[0][0][0] = nb_points[0][1][0] + nb_points[0][1][2] + nb_points[0][2][1];

      if ( nb_points[0][0][0] > 0 ) {
          fprintf( stderr, " nombre de points appartenant a une composante                          = %d\n", nb_points[0][0][0] );
          if ( nb_points[0][1][0] > 0 )
            fprintf( stderr, "                                               (nombre de points isoles = %d)\n", nb_points[0][1][0] );
          if ( nb_points[0][1][2] > 0 )
            fprintf( stderr, "                                            (nombre de points de courbe = %d)\n", nb_points[0][1][2] );
          if ( nb_points[0][2][1] > 0 )
            fprintf( stderr, "                                           (nombre de points de surface = %d)\n", nb_points[0][2][1] );
      }

      nb_points[0][0][0] = 0;
      nb_points[0][0][0] += nb_points[0][0][1];
      for (i=3; i<9; i++ ) nb_points[0][0][0] += nb_points[0][1][i];
      for (i=2; i<6; i++ ) nb_points[0][0][0] += nb_points[0][2][i];
      for (i=1; i<4; i++ ) nb_points[0][0][0] += nb_points[0][3][i];
      for (i=1; i<3; i++ ) nb_points[0][0][0] += nb_points[0][4][i];
      nb_points[0][0][0] += nb_points[0][5][1] + nb_points[0][6][1];

      nb_points[0][0][0] += nb_points[1][1][1];
      nb_points[0][0][0] += nb_points[1][1][2];
      for (i=1; i<6; i++ ) nb_points[0][0][0] += nb_points[1][2][i];

      if ( nb_points[0][0][0] > 0 ) {
          fprintf( stderr, " nombre de points appartenant a une jonction                            = %d\n", nb_points[0][0][0] );
          if ( nb_points[0][0][1] > 0 )
            fprintf( stderr, "                                         (points interieurs a un volume = %d)\n", nb_points[0][0][1] );
          if ( nb_points[0][1][3] > 0 )
            fprintf( stderr, "                                       (points jonction entre 3 courbes = %d)\n", nb_points[0][1][3] );
          if ( nb_points[0][1][4] > 0 )
            fprintf( stderr, "                                       (points jonction entre 4 courbes = %d)\n", nb_points[0][1][4] );
          if ( nb_points[0][1][5] > 0 )
            fprintf( stderr, "                                       (points jonction entre 5 courbes = %d)\n", nb_points[0][1][5] );
          if ( nb_points[0][1][6] > 0 )
            fprintf( stderr, "                                       (points jonction entre 6 courbes = %d)\n", nb_points[0][1][6] );
          if ( nb_points[0][1][7] > 0 )
            fprintf( stderr, "                                       (points jonction entre 7 courbes = %d)\n", nb_points[0][1][7] );
          if ( nb_points[0][1][8] > 0 )
            fprintf( stderr, "                                       (points jonction entre 8 courbes = %d)\n", nb_points[0][1][8] );
          if ( nb_points[0][2][2] > 0 )
            fprintf( stderr, "                           (points jonction entre 1 surface et 1 courbe = %d)\n", nb_points[0][2][2] );
          if ( nb_points[0][2][3] > 0 )
            fprintf( stderr, "                          (points jonction entre 1 surface et 2 courbes = %d)\n", nb_points[0][2][3] );
          if ( nb_points[0][2][4] > 0 )
            fprintf( stderr, "                          (points jonction entre 1 surface et 3 courbes = %d)\n", nb_points[0][2][4] );
          if ( nb_points[0][2][5] > 0 )
            fprintf( stderr, "                          (points jonction entre 1 surface et 4 courbes = %d)\n", nb_points[0][2][5] );
          if ( nb_points[0][3][1] > 0 )
            fprintf( stderr, "                                      (points jonction entre 3 surfaces = %d)\n", nb_points[0][3][1] );
          if ( nb_points[0][3][2] > 0 )
            fprintf( stderr, "                          (points jonction entre 3 surfaces et 1 courbe = %d)\n", nb_points[0][3][2] );
          if ( nb_points[0][3][3] > 0 )
            fprintf( stderr, "                         (points jonction entre 3 surfaces et 2 courbes = %d)\n", nb_points[0][3][3] );
          if ( nb_points[0][4][1] > 0 )
            fprintf( stderr, "                                      (points jonction entre 4 surfaces = %d)\n", nb_points[0][4][1] );
          if ( nb_points[0][4][2] > 0 )
            fprintf( stderr, "                          (points jonction entre 4 surfaces et 1 courbe = %d)\n", nb_points[0][4][2] );
          if ( nb_points[0][5][1] > 0 )
            fprintf( stderr, "                                      (points jonction entre 5 surfaces = %d)\n", nb_points[0][5][1] );
          if ( nb_points[0][6][1] > 0 )
            fprintf( stderr, "                                      (points jonction entre 6 surfaces = %d)\n", nb_points[0][6][1] );
          if ( nb_points[1][1][1] > 0 )
            fprintf( stderr, "                       (points frontiere voisins d'un point de jonction = %d)\n", nb_points[1][1][1] );
          if ( nb_points[1][1][2] > 0 )
            fprintf( stderr, "                              (points de courbe, jonction entre courbes = %d)\n", nb_points[1][1][2] );
          if ( nb_points[1][2][1] > 0 )
            fprintf( stderr, "                            (points de surface, jonction entre surfaces = %d)\n", nb_points[1][2][1] );
          if ( nb_points[1][2][2] > 0 )
            fprintf( stderr, "  (points jonction entre 1 surface et 1 courbe, jonction entre surfaces = %d)\n", nb_points[1][2][2] );
          if ( nb_points[1][2][3] > 0 )
            fprintf( stderr, " (points jonction entre 1 surface et 2 courbes, jonction entre surfaces = %d)\n", nb_points[1][2][3] );
          if ( nb_points[1][2][4] > 0 )
            fprintf( stderr, " (points jonction entre 1 surface et 3 courbes, jonction entre surfaces = %d)\n", nb_points[1][2][4] );
          if ( nb_points[1][2][5] > 0 )
            fprintf( stderr, " (points jonction entre 1 surface et 4 courbes, jonction entre surfaces = %d)\n", nb_points[1][2][5] );
      }

    }

    return( 1 );
}





/* Caracterisation topologique d'une image contenant a priori des courbes.

   L'image d'entree doit etre codee en unsigned char (type UCHAR)
   ainsi que l'image de sortie.

   Les points definissant l'objet sont ceux ayant une valeur non
   nulle.

   La caracterisation est realisee en comptant le nombre de voisins
   de chaque point.

   Dans l'image resultat, les points ont les valeurs suivantes :
   - VT_JUNCTION pour les points de jonction
   - VT_SIMPLE   pour les points simples (points frontieres)
   - VT_CC       pour les autres
 
*/

int VT_CaracterisationCurves( vt_image *im1 /* image result */,
                        vt_image *im2 /* image to be topologically characterized */ )
{
    unsigned char *buf1, *buf2;
    register int nb, ind, i, j, k, x, y, z;
    int offset[3][3][3];
    int dx, dy, dz, dxy, dx1, dy1, dz1;
    int nb_points[27];
    
    if ( VT_Test2Image( im1, im2, "VT_CaracterisationCurves" ) == -1 ) return( -1 );
    if ( im1->type != (int)UCHAR ) {
      VT_Error("input image has bad type","VT_CaracterisationCurves" );
      return( -1 );
    }
    if ( im2->type != (int)UCHAR ) {
      VT_Error("output image has bad type","VT_CaracterisationCurves" );
      return( -1 );
    }

    /*--- initialisation ---*/
    for ( i = 0; i < 27; i++ ) nb_points[i] = 0;

    /*--- preparation ---*/
    buf1 = (unsigned char*)(im1->buf);
    buf2 = (unsigned char*)(im2->buf);
    dx = im2->dim.x;   dx1 = dx - 1;
    dy = im2->dim.y;   dy1 = dy - 1;
    dz = im2->dim.z;   dz1 = dz - 1;
    dxy = im2->dim.x * im2->dim.y;
    offset[1][1][1] = 0;
    offset[1][0][1] = - dx;
    offset[1][2][1] = dx;
    offset[0][1][1] = - 1;
    offset[0][0][1] = offset[1][0][1] - 1;
    offset[0][2][1] = offset[1][2][1] - 1;
    offset[2][1][1] = 1;
    offset[2][0][1] = offset[1][0][1] + 1;
    offset[2][2][1] = offset[1][2][1] + 1;
    for ( i = 0; i < 3; i++ )
    for ( j = 0; j < 3; j++ ) {
      offset[i][j][0] = offset[i][j][1] - dxy;
      offset[i][j][2] = offset[i][j][1] + dxy;
    }
    ind = 0;

    /*----------------------*/
    /*--- === CAS 2D === ---*/
    /*----------------------*/
    if ( im2->dim.z == 1 ) {
      for ( y = 0; y < (int)im2->dim.y; y++ )
        for ( x = 0; x < (int)im2->dim.x; x++ ) {
          if ( buf2[ind] <= 0 ) {
            buf1[ind] = (unsigned char)0;
            ind ++;
            continue;
          }

          nb = 0;
          /*--- a l'interieur de l'image, on ne teste pas ---*/
          if ( ( x > 0 ) && ( x < dx1 ) &&
             ( y > 0 ) && ( y < dy1 ) ) {
            for ( i = 0; i < 3; i++ )
                for ( j = 0; j < 3; j++ ) {
                if ( buf2[ind + offset[i][j][1]] > 0 ) nb ++;
            }
          } else {
            /*--- au bord de l'image, on teste ---*/
            for ( i = -1; i < 2; i++ )
                for ( j = -1; j < 2; j++ ) {
                if ( (x+i >= 0) && (x+i < dx) &&
                   (y+j >= 0) && (y+j < dy) ) {
                  if ( buf2[ind + offset[1+i][1+j][1]] > 0 ) nb ++;
                }
            }
          }

          /*--- on compte ---*/
          nb_points[nb] ++;

          /*--- on a le nombre TOTAL de points dans le voisinage :
                le point central en fait partie                    ---*/
          switch ( nb ) {
          case 1 :
          case 3 :
            buf1[ind] = (unsigned char)VT_CC;
            break;
          case 2 :
            buf1[ind] = (unsigned char)VT_SIMPLE;
            break;
          default :
            buf1[ind] = (unsigned char)VT_JUNCTION;
          }
          ind ++;
      }
    }
    /*----------------------*/
    /*--- === CAS 3D === ---*/
    /*----------------------*/
    else {
      for ( z = 0; z < (int)im2->dim.z; z++ )
      for ( y = 0; y < (int)im2->dim.y; y++ )
        for ( x = 0; x < (int)im2->dim.x; x++ ) {
          if ( buf2[ind] <= 0 ) {
            buf1[ind] = (unsigned char)0;
            ind ++;
            continue;
          }

          nb = 0;
          /*--- a l'interieur de l'image, on ne teste pas ---*/
          if ( ( x > 0 ) && ( x < dx1 ) &&
             ( y > 0 ) && ( y < dy1 ) &&
             ( z > 0 ) && ( z < dz1 ) ) {
            for ( i = 0; i < 3; i++ )
                for ( j = 0; j < 3; j++ ) 
            for ( k = 0; k < 3; k++ ) {
                if ( buf2[ind + offset[i][j][k]] > 0 ) nb ++;
            }
          } else {
          /*--- au bord de l'image, on teste ---*/
            for ( i = -1; i < 2; i++ )
                for ( j = -1; j < 2; j++ ) 
            for ( k = -1; k < 2; k++ ) {
                if ( (x+i >= 0) && (x+i < dx) &&
                   (y+j >= 0) && (y+j < dy) &&
                   (z+k >= 0) && (z+k < dz) ) {
                  if ( buf2[ind + offset[1+i][1+j][1+k]] > 0 ) nb ++;
                }
            }
          }

          /*--- on compte ---*/
          nb_points[nb] ++;

          /*--- on a le nombre TOTAL de points dans le voisinage :
                le point central en fait partie                    ---*/
          switch ( nb ) {
          case 1 :
          case 3 :
            buf1[ind] = (unsigned char)VT_CC;
            break;
          case 2 :
            buf1[ind] = (unsigned char)VT_SIMPLE;
            break;
          default :
            buf1[ind] = (unsigned char)VT_JUNCTION;
          }
          ind ++;
      }
    }

    /*--- faut-il ecrire ? ---*/
    if ( _VT_VERBOSE_ == 1 ) {
      if ( nb_points[0] > 0 )
          fprintf( stderr, "ERREUR : %d points ont -1 voisins.\n", nb_points[0] );
      if ( nb_points[2] > 0 )
          fprintf( stderr, "nombre de points simples (frontiere)          = %d\n", nb_points[2] );
      if ( (nb_points[1]+nb_points[3]) > 0 ) {
          fprintf( stderr, "nombre de points appartenant a une composante = %d\n", (nb_points[1]+nb_points[3]) );
          if ( nb_points[1] > 0 )
            fprintf( stderr, "                     (nombre de points isoles = %d)\n", nb_points[1] );
          if ( nb_points[3] > 0 )
            fprintf( stderr, "              (nombre de points a  2 voisins  = %d)\n", nb_points[3] );
      }
      nb_points[0] = 0;
      for ( i = 4; i < 27; i++ ) nb_points[0] += nb_points[i];
      if ( nb_points[0] > 0 ) {
          fprintf( stderr, "nombre de points appartenant a une jonction   = %d\n", nb_points[0] );
          for ( i = 4; i < 27; i++ ) {
            if ( nb_points[i] > 0 )
                fprintf( stderr, "              (nombre de points a %2d voisins  = %d)\n", i, nb_points[i] );
          }
      }
    }

    return( 1 );
}





static int _VT_CCin5x5x5Neighborhood( int *vois )
{
    register int x, y, z, nb;
    int ind, label, flag;

    label = ind = nb = 0;
    flag = 0;
    do {
      /*--- recherche d'un premier point ---*/
      ind = 0;
      flag = 0;
      for ( z = 0; (z < 5) && (flag == 0); z ++ )
      for ( y = 0; (y < 5) && (flag == 0); y ++ )
        for ( x = 0; (x < 5) && (flag == 0); x ++ ) {
          if ( vois[ind] == _VT_BACK06 ) {
            label ++;
            vois[ind] = label;
            flag = 1;
          }
          ind ++;
      }
      /*--- propagation du label ---*/
      if ( flag == 1 ) {
          do {
            nb = 0;
            ind = 0;
            for ( z = 0; z < 5; z ++ )
            for ( y = 0; y < 5; y ++ )
            for ( x = 0; x < 5; x ++ ) {
                if ( vois[ind] == label ) {
                  if ( x > 0 ) {
                      if ( (vois[ind-1] == _VT_BACK06) || (vois[ind-1] == _VT_BACK18) ) { vois[ind-1] = label;   nb++; }
                  }
                  if ( x < 4 ) {
                      if ( (vois[ind+1] == _VT_BACK06) || (vois[ind+1] == _VT_BACK18) ) { vois[ind+1] = label;   nb++; }
                  }
                  if ( y > 0 ) {
                      if ( (vois[ind-5] == _VT_BACK06) || (vois[ind-5] == _VT_BACK18) ) { vois[ind-5] = label;   nb++; }
                  }
                  if ( y < 4 ) {
                      if ( (vois[ind+5] == _VT_BACK06) || (vois[ind+5] == _VT_BACK18) ) { vois[ind+5] = label;   nb++; }
                  }
                  if ( z > 0 ) {
                      if ( (vois[ind-25] == _VT_BACK06) || (vois[ind-25] == _VT_BACK18) ) { vois[ind-25] = label;   nb++; }
                  }
                  if ( z < 4 ) {
                      if ( (vois[ind+25] == _VT_BACK06) || (vois[ind+25] == _VT_BACK18) ) { vois[ind+25] = label;   nb++; }
                  }
                }
                ind ++;
            }
          } while ( nb > 0 );
      }
    } while ( flag == 1 );

    return( label);
}
