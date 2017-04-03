#ifndef _vt_distfield_h_
#define _vt_distfield_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h> /* memcpy */
#include <vt_image.h>
#include <vt_inrimage.h>

/* Structure champ de distance.
 * 
 * Un champ de distance est compose de 3 buffers.
 * Il convient pour chaque point le vecteur pointant
 * vers le point le plus proche. Chacune des composantes
 * du vecteur est contenue dans un buffer,
 */
typedef struct {
  void *x; /* buffer contenant la premiere composante */
  void *y; /* buffer contenant la deuxieme composante */
  void *z; /* buffer contenant la troisieme composante */
  bufferType type; /* type de codage des buffers */
  int dimx; /* premiere dimension (selon X) */
  int dimy; /* deuxieme dimension (selon Y) */
  int dimz; /* troisieme dimension (selon Z) */
  int dimxy; /* dimx * dimy */
} distanceField;


/* Renvoie une structure distanceField a partir de 3 images.
 *
 * DESCRIPTION:
 * Chacune des 3 images contient une composante du vecteur 
 * pointant vers le point le plus proche.
 *
 * Cette procedure lit ces trois images, alloue la structure
 * et les buffers necessaires, et remplit ces buffers.
 *
 * Il faudra donc desallouer la memoire en utilisant
 * VT_FreeDistanceField();
 *
 * RETURN:
 * (distanceField *)NULL en cas d'erreur.
 */
extern distanceField *VT_FillDistanceField( char* imageX /* premiere composante */,
					    char* imageY /* deuxieme composante */,
					    char* imageZ /* troisieme composante */
					    );

/* Renvoie les trois composantes du champ de distance.
 *
 * DESCRIPTION:
 * le vecteur float *v (deja alloue) contiendra le vecteur
 * pointant vers le point le plus proche (v[0],v[1],v[2])
 * pour le point (x,y,z).
 */
   
extern void getDistanceFieldValues( float *v /* vecteur resultat */, 
				    int x /* premiere coordonnee */, 
				    int y /* deuxieme coordonnee */, 
				    int z /* troisieme coordonnee */,
				    distanceField *dfield /* champ de distance */
				    );

/* Desalloue une structure distanceField *.
 *
 * A appeler lorsque la strcuture doit etre
 * desallouee.
 */
extern void VT_FreeDistanceField( distanceField *dfield /* structure a desallouer */
				  );

/* Cree et alloue une structure distanceField.
 *
 * La structure et ses champs sont alloues 
 * mais pas remplis.
 *
 * RETURN:
 * (distanceField *)NULL en cas d'erreur.
 */
extern distanceField *VT_CreateDistanceField( int dimx /* premiere dimension */,
					      int dimy /* deuxieme dimension */,
					      int dimz /* troisieme dimension */,
					      bufferType type /* type de codage des buffers */
					      );

#ifdef __cplusplus
}
#endif

#endif /* _vt_distfield_h_ */
