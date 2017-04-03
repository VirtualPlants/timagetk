
#ifndef _vt_neighborhood_h_
#define _vt_neighborhood_h_

/* Voisinages usuels.
 * - N04 = 4,  4-voisinage, voisins qui partagent une arete en 2D
 * - N06 = 6,  6-voisinage, voisins qui partagent une face en 3D 
 * - N08 = 8,  8-voisinage, voisins qui partagent une arete ou un sommet en 2D 
 * - N10 = 10,  10-voisinage = union du 6-voisinage et ou 8-voisinage 
 * - N18 = 18,  18-voisinage, voisins qui partagent une face ou une arete en 3D 
 * - N26 = 26  26-voisinage, voisins qui partagent une face, une arete ou un sommet en 3D 
 */
typedef enum  {
  C_04 = 4,
  C_08 = 8,
  C_06 = 6,
  C_10 = 10,
  C_18 = 18,
  C_26 = 26,
  N04 = 4, /* 4-voisinage, voisins qui partagent une arete en 2D */
  N06 = 6, /* 6-voisinage, voisins qui partagent une face en 3D */
  N08 = 8, /* 8-voisinage, voisins qui partagent une arete ou un sommet en 2D */
  N10 = 10, /* 10-voisinage = union du 6-voisinage et ou 8-voisinage */
  N18 = 18, /* 18-voisinage, voisins qui partagent une face ou une arete en 3D */
  N26 = 26 /* 26-voisinage, voisins qui partagent une face, une arete ou un sommet en 3D */
} Neighborhood;

#endif
