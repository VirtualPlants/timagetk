
#ifndef _vt_morpho_h_
#define _vt_morpho_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>

#include <vt_neighborhood.h>

#include <morpho.h>


/* Dilation binaire sur des images 3D.
 *
 * L'utilisation d'operations logiques (sur des entiers)
 * permet d'ameliorer les performances. Attention a ne pas 
 * l'utiliser sur des images en niveau de gris.
 * 
 * EXEMPLE: 
 * des images unsigned char binaires ne doivent 
 * contenir que les valeurs 0 et 255.
 */
extern int BinaryDilation( vt_image *theIm, /* input image */
			   vt_image *resIm, /* result image */
			   Neighborhood N, /* structuring element */
			   int iterations /* iterations number */ );

/* Erosion binaire sur des images 3D.
 *
 * L'utilisation d'operations logiques (sur des entiers)
 * permet d'ameliorer les performances. Attention a ne pas 
 * l'utiliser sur des images en niveau de gris.
 * 
 * EXEMPLE: 
 * des images unsigned char binaires ne doivent 
 * contenir que les valeurs 0 et 255.
 */
extern int BinaryErosion( vt_image *theIm,
			   vt_image *resIm,
			   Neighborhood N,
			   int iterations );

/* Dilation sur des images 3D.
 *
 * L'utilisation d'operations logiques (sur des entiers)
 * permet d'ameliorer les performances. Attention a ne pas 
 * l'utiliser sur des images en niveau de gris.
 * 
 * EXEMPLE: 
 * des images unsigned char binaires ne doivent 
 * contenir que les valeurs 0 et 255.
 */
extern int GreyLevelDilation( vt_image *theIm,
			       vt_image *resIm,
			       Neighborhood N,
			       int iterations );

/* Erosion sur des images 3D.
 *
 * L'utilisation d'operations logiques (sur des entiers)
 * permet d'ameliorer les performances. Attention a ne pas 
 * l'utiliser sur des images en niveau de gris.
 * 
 * EXEMPLE: 
 * des images unsigned char binaires ne doivent 
 * contenir que les valeurs 0 et 255.
 */
extern int GreyLevelErosion( vt_image *theIm,
			      vt_image *resIm,
			      Neighborhood N,
			      int iterations );
#ifdef __cplusplus
}
#endif

#endif
