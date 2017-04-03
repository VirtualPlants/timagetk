
#ifndef _vt_reech_h_
#define _vt_reech_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h> /* memcpy */

#include <vt_common.h>

#include <convert.h>
#include <typedefs.h>

#include <reech4x4.h>
#include <reech-def.h>
#include <cspline.h>

/* Fonction de reechantillonnage d'une image 3D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes.
 *
 * COMMENTS:
 * Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en utilisant une interpolation
 * trilineaire.
 *
 * ERRORS:
 * L'image resultat doit etre de meme type que l'image d'entree.
 */ 
extern int Reech3DTriLin4x4( vt_image *theIm, /* input image */
			     vt_image *resIm, /* result image */
			     double *mat /* transformation matrix */ );

/* Fonction de reechantillonnage d'une image 3D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes. L'intensite calculee est ainsi 
 * multipliee par un gain, un biais est ajoute a ce resultat.
 *
 * COMMENTS:
 * Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en utilisant une interpolation
 * trilineaire.
 *
 * ERRORS:
 * L'image resultat doit etre de meme type que l'image d'entree.
 */ 
extern int Reech3DTriLin4x4gb( vt_image *theIm, /* input image */
			       vt_image *resIm, /* result image */
			       double *mat, /* transformation matrix */
			       float gain,
			       float bias );

/* Fonction de reechantillonnage d'une image 3D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes.
 *
 * COMMENTS:
 * Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en prenant la valeur du point
 * le plus proche.
 *
 * ERRORS:
 * L'image resultat doit etre de meme type que l'image d'entree.
 */ 
extern int Reech3DNearest4x4( vt_image *theIm, /* input image */
			      vt_image *resIm, /* result image */
			      double *mat );

/* Fonction de reechantillonnage d'une image 2D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes.
 *
 * COMMENTS:
 * - Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en utilisant une interpolation
 * trilineaire.
 * - On applique une transformation 2D a chacune des coupes
 * de l'image 3D.
 *
 * ERRORS:
 * L'image resultat doit etre de meme type que l'image d'entree.
 */ 
extern int Reech2DTriLin4x4( vt_image *theIm, /* input image */
			     vt_image *resIm, /* result image */
			     double *mat /* transformation matrix */ );

/* Fonction de reechantillonnage d'une image 2D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes. L'intensite calculee est ainsi 
 * multipliee par un gain, un biais est ajoute a ce resultat.
 *
 * COMMENTS:
 * - Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en utilisant une interpolation
 * trilineaire.
 * - On applique une transformation 2D a chacune des coupes
 * de l'image 3D.
 *
 * ERRORS:
 * L'image resultat doit etre de meme type que l'image d'entree.
 */ 
extern int Reech2DTriLin4x4gb( vt_image *theIm, /* input image */
			       vt_image *resIm, /* result image */
			       double *mat, /* transformation matrix */
			       float gain,
			       float bias );

/* Fonction de reechantillonnage d'une image 2D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes.
 *
 * COMMENTS:
 * -Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en prenant la valeur du point
 * le plus proche.
 * - On applique une transformation 2D a chacune des coupes
 * de l'image 3D.
 *
 * ERRORS:
 * L'image resultat doit etre de meme type que l'image d'entree.
 */ 
extern int Reech2DNearest4x4( vt_image *theIm, /* input image */
			      vt_image *resIm, /* result image */
			      double *mat  /* transformation matrix */);

/* Fonction de reechantillonnage d'une image 3D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes et/ou un champ dense de deformations.
 *
 * COMMENTS:
 * - Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en utilisant une interpolation
 * trilineaire.
 * - On applique d'abord le champ de deformation, puis la matrice 
 * si elle est non NULL. Enfin la valeur interpolee
 * est modifiee par les valeurs du gain (multiplicatif)
 * et du biais (additif).
 *
 * WARNING: 
 * - Le champ de deformation, ainsi que la matrice, permettent 
 * le passage de l'image resultat VERS l'image d'entree.
 *
 * ERREURS:
 * - Les images de deformations, si elles existent, doivent etre 
 * de type float, et de meme dimensions que l'image resultat.
 */ 
extern int Reech3DTriLinDefBack( vt_image *theIm, /* input image */
				 vt_image *resIm, /* result image */
				 vt_image **theDef, /* deformation images */
				 double *mat_aft, /* transformation matrix */
				 double *mat_bef, /* transformation matrix */
				 double gain,
				 double bias );

/* Fonction de reechantillonnage d'une image 3D.
 *
 * Le reechantillonnage est fait avec une matrice de passage
 * en coordonnees homogenes et/ou un champ dense de deformations.
 *
 * COMMENTS:
 * - Cette fonction reechantillonne l'image d'entree (theIm)
 * dans l'image de sortie en utilisant la valeur du plus
 * proche voisin.
 * - On applique d'abord le champ de deformation, puis la matrice 
 * si elle est non NULL. 
 *
 * WARNING: 
 * - Le champ de deformation, ainsi que la matrice, permettent 
 * le passage de l'image resultat VERS l'image d'entree.
 *
 * ERREURS:
 * - Les images de deformations, si elles existent, doivent etre 
 * de type float, et de meme dimensions que l'image resultat.
 */ 
extern int Reech3DNearestDefBack( vt_image *theIm, /* input image */
				  vt_image *resIm, /* result image */
				  vt_image **theDef, /* deformation images */
				  double *mat_aft, /* transformation matrix */
				  double *mat_bef /* transformation matrix */ );

/* Procedure de reechantillonnage 3D avec des splines cubiques

   La matrice 4x4 permet le passage des coordonnees
   de l'image resultat a l'image d'entree. On calcule
   ainsi pour chaque point de l'image resultat son
   correspondant en coordonnees reelles dans l'image 
   d'entree. 
*/

extern int Reech3DCSpline4x4( vt_image *theIm, /* input image */
		      vt_image *resIm, /* result image */
		      double *mat );


/* Active l'ecriture de diverses informations
 *
 * Cette fonction active l'ecriture de diverses informations
 * lors de l'appel aux fonctions de reechantillonnage.
 */
extern void Reech3D_verbose ( );

/* Desactive l'ecriture de diverses informations
 *
 * Fonction inverse de celle de Reech3D_verbose ( );
 */
extern void Reech3D_noverbose ( );

#ifdef __cplusplus
}
#endif

#endif
