/****************************************************
 * issimple3D.h - determine si un point 3D est simple
 *
 * $Id: issimple3D.h,v 1.2 2000/08/08 14:48:27 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Feb, 24 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */


#ifndef _issimple_THREED_h_
#define _issimple_THREED_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Determine si un point est simple ou pas.

DESCRIPTION:
   - On considere la 26-connexite pour l'objet
   c'est-a-dire les points de valeurs non nulle,
   et la 6-connexite pour le fond,
   c'est-a-dire les points de valeurs nulles.

   - Le voisinage est numerote comme suit
        0  1  2  -   9 10 11  -  18 19 20
        3  4  5  -  12 13 14  -  21 22 23
        6  7  8  -  15 16 17  -  24 25 26

RETURN:
   - 1 si le point est simple

   - 0 sinon

*/
#if defined(_STATISTICS_)
  extern int IsA3DPointSimple( int neighbors[27], int *TESTS_nb, int *EQUIV_nb );
#else
  extern int IsA3DPointSimple( int neighbors[27] );
#endif

#ifdef __cplusplus
}
#endif

#endif
