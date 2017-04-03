
#include <vt_amincir.h>





/* Amincissement : valeurs par defaut pour la structure de parametres.

   Le champ float seuil de la structure est mis a 0.5
   Le champ int type_method de la structure de parametre est mis a 0
   Le champ int type_option de la structure de parametre est mis a 0
   Le champ int bool_shrink de la structure de parametre est mis a 0
   Le champ vt_distance par_dist est initialise par VT_Distance
*/   

void VT_Amincir( vt_amincir *par /* parameters */ )
{
    par->seuil = 0.5;
    par->type_method = 0;
    par->type_option = 0;
    par->bool_shrink = 0;
    par->bool_end_surfaces = 1;
    par->bool_end_curves = 1;
    /*    par->min_end_distance = 0;*/
    VT_Distance( &(par->par_dist) );
    par->connexite = N26;
    par->epaisseur = N06;
}





/* Aminicissement d'une image.

   Les points dont la valeur est superieure ou egale au
   seuil (champ seuil de la structure) definissent l'objet a amincir.

   Le passage du seuil float en int se fait en prenant
   l'entier le plus proche :
   - pour les valeurs positives (int)(f+0.5)
   - pour les valeurs negatives (int)(f-0.5)

   L'objet aminci dans l'image resultat est defini par
   la valeur resultat de la fonction VT_Threshold.

   Le type de methode (champ type_method de la structure) peut etre :
   * VT_GONGBERTRAND (Gong, W.X. and Bertrand, G.
     "A simple parallel 3-D thinning algorithm",
     10th International Conference on Pattern Recognition,
     Atlantic City, june 17--21, 1990)
   * VT_GRG_PLANES : on prend la definition exacte des points simples,
     et la meme definition que Gong et Bertrand pour les 
     points d'arret.
   * VT_GRG_CURVES : on ne conserve que les points de courbe,
     ou de jonctions entre surfaces, avec condition d'arret.
   * VT_GRG_2D : amincissement d'image 2D (ou d'image 3D plan
     par plan), avec condition d'arret.

   L'implementation (champ type_option de la structure) peut etre :
   * VT_OLD : implementation historique
   * VT_BDD : implementation par les BDD (si disponible)
   * VT_DIST : tri par la distance (options dans le champ par-dist
     de la structure) avant amincissement

   Si le champ bool_shrink de la structure est a 1, les 
   conditions d'arret sont ignorees.

*/

int VT_Thinning( vt_image *im1 /* image result */,
		 vt_image *im2 /* image to be thinned */,
		 vt_amincir *par /* parameters */ )
{
    vt_image aux;
    
    if ( VT_Test2Image( im1, im2, "VT_Thinning" ) == -1 ) return( -1 );
    
    /*--- preparation de l'image intermediaire ---*/
    VT_InitImage( &aux, "", im2->dim.x, im2->dim.y, im2->dim.z, (int)UCHAR );
    
    if ( im1->type == (int)UCHAR ) {
	aux.buf = im1->buf;
    }
    else {
	if ( VT_AllocImage( &aux ) != 1 ) {
	    VT_Error("unable to allocate auxiliary image","VT_Thinning");
	    return( -1 );
	}
    }
    
    /*--- seuillage de l'image input ---*/
    if ( _VT_ThinThreshold( &aux, im2, par->seuil ) != 1 ) {
	VT_Error("unable to threshold input image","VT_Thinning");
	if ( im1->type != (int)UCHAR )
	    VT_FreeImage( &aux );
	return( -1 );
    }
    
    /*--- amincissement de l'image seuillee ---*/
    if ( VT_Intern_Thinning( im1, &aux, par ) != 1 ) {
	VT_Error("unable to thin thresholded image","VT_Thinning");
	if ( im1->type != (int)UCHAR )
	    VT_FreeImage( &aux );
	return( -1 );
    }
    
    /*--- liberation eventuelle de l'image intermediaire ---*/
    if ( im1->type != (int)UCHAR )
	VT_FreeImage( &aux );
    return( 1 );
}





/* Amincissement d'une image binaire deja preparee.

   La deuxieme image est l'image a amincir. Elle doit
   etre de type UCHAR (codee en unsigned char) et elle
   sera modifiee lors du processus.

   Les points appartenant a l'objet doivent etre differents de 0. 
   Seuls les points de valeur VT_DELETABLE seront candidats a
   l'effacement. On peut donc specifier des points qui ne
   sont pas effacables en leur attribuant la valeur
   VT_UNDELETABLE.

   L'objet aminci dans l'image resultat est defini par
   la valeur resultat de la fonction VT_Threshold.

   Le type de methode (champ type_method de la structure) peut etre :
   * VT_GONGBERTRAND (Gong, W.X. and Bertrand, G.
     "A simple parallel 3-D thinning algorithm",
     10th International Conference on Pattern Recognition,
     Atlantic City, june 17--21, 1990)
   * VT_GRG_PLANES : on prend la definition exacte des points simples,
     et la meme definition que Gong et Bertrand pour les 
     points d'arret.
   * VT_GRG_CURVES : on ne conserve que les points de courbe,
     ou de jonctions entre surfaces, avec condition d'arret.
   * VT_GRG_2D : amincissement d'image 2D (ou d'image 3D plan
     par plan), avec condition d'arret.

   L'implementation (champ type_option de la structure) peut etre :
   * VT_OLD : implementation historique
   * VT_BDD : implementation par les BDD (si disponible)
   * VT_DIST : tri par la distance (options dans le champ par-dist
     de la structure) avant amincissement

   Si le champ bool_shrink de la structure est a 1, les 
   conditions d'arret sont ignorees.

*/

int VT_Intern_Thinning( vt_image *im1 /* image result */,
		 vt_image *im2 /* image to be thinned */,
		 vt_amincir *par /* parameters */ )
{
    vt_amincir local_par;
    int ret = 0;

    if ( VT_Test2Image( im1, im2, "VT_Intern_Thinning" ) == -1 ) return( -1 );
    if ( im1->type != UCHAR ) {
	VT_Error( "incorrect image type", "VT_Intern_Thinning" );
	return( -1 );
    }
    
    /*--- determination des parametres ---*/
    local_par = *par;
    switch ( par->type_method ) {
    case VT_GONGBERTRAND :
    case VT_GRG_PLANES :
    case VT_GRG_CURVES :
    case VT_GRG_2D :
	break;
    default :
	if ( im1->dim.z == 1 ) local_par.type_method = VT_GRG_2D;
	else                   local_par.type_method = VT_GRG_PLANES;
    }

    switch ( par->type_option ) {
    case VT_BDD :
    case VT_DIST :
    case VT_OLD :
	break;
    default :
	local_par.type_option = VT_BDD;
    }

    switch ( par->bool_shrink ) {
    case 1 :
    case 0 :
	break;
    default :
	local_par.bool_shrink = 0;
    }

    /*--- les procedures d'amincissement ---*/
    switch ( local_par.type_method ) {
    case VT_GONGBERTRAND :
	switch ( local_par.type_option ) {
	case VT_BDD :
      ret = _VT_BDD_GONGBERTRAND( im2 );
	    break;
	case VT_DIST :
	case VT_OLD :    
	    ret = _VT_OLD_GONGBERTRAND( im2, &local_par );
	}
	break;    
    case VT_GRG_PLANES :
	switch ( local_par.type_option ) {
	case VT_BDD :
	    ret = _VT_BDD_GREG_PLANES( im2, &local_par );
	    break;
	case VT_DIST :
	case VT_OLD :    
	    ret = _VT_OLD_GREG_PLANES( im2, &local_par );
	}
	break;    
    case VT_GRG_CURVES :
	switch ( local_par.type_option ) {
	case VT_BDD :
	    ret = _VT_BDD_GREG_CURVES( im2, &local_par );
	    break;
	case VT_DIST :
	case VT_OLD :    
	    ret = _VT_OLD_GREG_CURVES( im2, &local_par );
	}
	break;    
    case VT_GRG_2D :
	switch ( local_par.type_option ) {
	case VT_DIST :
	    ret = _VT_DST_GREG_2D( im2, &local_par );
	    break;
	case VT_BDD :
	case VT_OLD :    
	    ret = _VT_OLD_GREG_2D( im2, &local_par );
	}
    }

    /*--- l'amincissement s'est-il bien passe ? ---*/
    if (ret != 1) {
	VT_Error( "unable to thin image", "VT_Intern_Thinning" );
	return( 0 );
    }

    /*--- on seuille ---*/
    return( VT_Threshold( im2, im1, (float)1.0 ) );
}
