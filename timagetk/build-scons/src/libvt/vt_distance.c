/*************************************************************************
 * vt_distance.c -
 *
 * $Id: vt_distance.c,v 1.2 2000/09/07 07:42:32 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 */



#include <vt_distance.h>

#define VT_MIN(A,B) ( (A) < (B) ? (A) : (B) )
#define VT_F2I( F ) ( (F) >= 0.0 ? ((int)((F)+0.5)) : ((int)((F)-0.5)) )

/*------ Static Functions ------*/

static int  _VT_IDistance( vt_image *resIm, vt_image *theIm, vt_distance *par );
static int  _VT_DCsg( u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_DLs(  u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_DCsd( u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_DCg(  u16 *vl, int i, int *offset, vt_distance *par );
#ifdef _UNUSED_
static int  _VT_DCe(  u16 *vl, int i, int *offset, vt_distance *par );
#endif
static int  _VT_DCd(  u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_DCig( u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_DLi(  u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_DCid( u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_RCid( u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_RLi(  u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_RCig( u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_RCd(  u16 *vl, int i, int *offset, vt_distance *par );
#ifdef _UNUSED_
static int  _VT_RCe(  u16 *vl, int i, int *offset, vt_distance *par );
#endif
static int  _VT_RCg(  u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_RCsd( u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_RLs(  u16 *vl, int i, int *offset, vt_distance *par );
static int  _VT_RCsg( u16 *vl, int i, int *offset, vt_distance *par );
static void _VT_ISeuil( vt_image *resIm, vt_image *theIm, int *offset, vt_distance *par );





/* Calcul de distance : valeurs par defaut pour la structure de parametres.

   Le champ float seuil de la structure est mis a 0.5
   Le champ int type de la structure de parametre est mis a VT_DIST_CHMFR
   Le champ int dim de la structure de parametre est mis a VT_3D
   Les 5 elements du champ int inc[5] sont mis a 1
*/   


void VT_Distance( vt_distance *par /* parameters */ )
{
	par->seuil = 0.5;
	par->type = VT_DIST_CHMFR;
	par->dim = VT_3D;
	par->inc[0] = par->inc[1] = par->inc[2] = par->inc[3] = par->inc[4] = 1;
	par->type_image_eucmap = SSHORT;
}





/* Calcul d'une image de distance.

   Le champ seuil de la structure de parametre definit le seuil pour
   binariser l'image. Le passage du seuil float en int se fait 
   en prenant l'entier le plus proche.

   Le champ type de la structure de parametre definit le type
   de distance :
   - VT_DIST_6 : 6-distance, les points partageant une face avec le 
     point central en sont a une distance de 1.
   - VT_DIST_10 : 10-distance, les points partageant une face ou une arete
     avec le point central dans le plan, ou une face dans un plan adjacent
     en sont a une distance de 1.
   - VT_DIST_18 : 18-distance, les points partageant une face ou une arete
     avec le point central en sont a une distance de 1.
   - VT_DIST_26 : 26-distance, les points partageant une face, une arete
     ou un sommet avec le point central en sont a une distance de 1.
   - VT_DIST_CHMFR : distance de chamfer. Coefficients (16, 21, 26)
     puis division par 16.
   - VT_DIST_CHMFR2 : distance de chamfer. Coefficients (16, 21, 26)
     sans division. La distance obtenue est donc 16 fois la "vraie"
     distance. 
   - VT_DIST_CHMFR3 : distance de chamfer. Coefficients (32, 42, 52)
     pour le calcul. Toutefois les points 6-voisins, 18-voisins et
     26-voisins de l'objet en sont a une distance de 16, 21 et 26.
     Pas de division ensuite. La distance obtenue est donc 32 fois 
     la "vraie" distance. 
   - VT_DIST_EUCLI : distance euclidienne (algorithme de Danielsson).
     On calcule d'abord les 3 images formant les vecteurs designant 
     les points les plus proches, puis on en prend la norme.
     Ce calcul est forcement 3D.
   - VT_DIST_EUCLI_2 : distance euclidienne au carre (algorithme de Danielsson).
     On calcule d'abord les 3 images formant les vecteurs designant 
     les points les plus proches, puis on en prend la norme au carre.
     Ce calcul est forcement 3D.
   - VT_DIST_4 : 4-distance = 6-distance dans le plan.
   - VT_DIST_8 : 8-distance = 18- ou 26-distance dans le plan.
   - VT_DIST_CHMFR_2D : distance de chamfer VT_DIST_CHMFR dans le plan.
   - VT_DIST_CHMFR2_2D : distance de chamfer VT_DIST_CHMFR2 dans le plan.
   - VT_DIST_CHMFR3_2D : distance de chamfer VT_DIST_CHMFR3 dans le plan.
   - VT_DIST_USER : increments specifies par l'utilisateur (ce calcul est
     forcement 3D).

   Le champ mode specifie la dimension du traitement. S'il est egal a
   VT_2D, le calcul se fera dans le plan : le type (dans le champ
   du meme nom) peut etre modifie, sinon on respectera le choix fait.

   Les 5 elements du champ inc[5] specifient les increments pour le calcul
   de la distance (facon chamfer) si le type est VT_DIST_USER, ils concernent
   - inc[0] : voisin par une face dans le plan
   - inc[1] : voisin par une arete dans le plan
   - inc[2] : voisin par une face dans le plan superieur ou inferieur
   - inc[3] : voisin par une arete dans le plan superieur ou inferieur
   - inc[4] : voisin par un sommet dans le plan superieur ou inferieur
   
RETURN
   Renvoie FALSE en cas d'erreur.
*/

int VT_Dist( vt_image *resIm /* result image */, 
	     vt_image *theIm /* input image */, 
	     vt_distance *par /* parameters */ )
{
    vt_image auxIm;
    register int i, x, y, z;
    u16 *vl;
  
  if ( VT_Test2Image( resIm, theIm, "VT_Dist" ) == -1 ) return( -1 );
  if ( ( theIm->dim.x < 3 ) || ( theIm->dim.y < 3 ) ) {
	  VT_Error( "images have bad dimensions", "VT_Dist" );
	  return( -1 );
  }
  
  switch ( par->type ) {
  case VT_DIST_6 :
  case VT_DIST_10 :
  case VT_DIST_18 :
  case VT_DIST_26 :
  case VT_DIST_CHMFR :
  case VT_DIST_CHMFR2 :
  case VT_DIST_CHMFR3 :
  case VT_DIST_4 :
  case VT_DIST_8 :
  case VT_DIST_CHMFR_2D :
  case VT_DIST_CHMFR2_2D :
  case VT_DIST_CHMFR3_2D :
  case VT_DIST_USER :
	  VT_InitImage( &auxIm, "", theIm->dim.x, theIm->dim.y, theIm->dim.z, (int)USHORT );
	  if ( (resIm->type != USHORT) && (resIm->type != SSHORT) ) {
		  if ( VT_AllocImage( &auxIm ) != 1 ) {
			  VT_Error("unable to allocate auxiliary image","VT_Dist");
			  return( -1 );
		  }
	  }
	  else
		  auxIm.buf = resIm->buf;
	  
	  /*--- calcul effectif dans l'image auxilliaire ---*/
	  if ( _VT_IDistance( &auxIm, theIm, par ) == 0 ) {
		  if ( (resIm->type != USHORT) && (resIm->type != SSHORT) )
			  VT_FreeImage( &auxIm );
		  return( 0 );
	  }
	  /*--- ecriture de l'image de sortie ---*/
	  if ( resIm->type != USHORT) {
		  vl = (u16 *)auxIm.buf;
		  i = 0;
		  switch (resIm->type) {
		  case UCHAR :
			  {
			  u8 *buf;
			  buf = (u8 *)(resIm->buf);
        for (z=0; z<(int)resIm->dim.z; z++)
        for (y=0; y<(int)resIm->dim.y; y++)
        for (x=0; x<(int)resIm->dim.x; x++) {
				  if ( *(vl+i) > 255 ) *(buf+i) = (u8)255;
				  else *(buf+i) = (u8)(*(vl+i));
				  i++;
			  }}
			  break;
		  case SSHORT :
			  {
			  s16 *buf;
			  buf = (s16 *)(resIm->buf);
        for (z=0; z<(int)resIm->dim.z; z++)
        for (y=0; y<(int)resIm->dim.y; y++)
        for (x=0; x<(int)resIm->dim.x; x++) {
				  if ( *(vl+i) > 32767 ) *(buf+i) = (s16)32767;
				  else *(buf+i) = (s16)(*(vl+i));
				  i++;
			  }}
			  break;
		  case SINT :
			  {
			  i32 *buf;
			  buf = (i32 *)(resIm->buf);
        for (z=0; z<(int)resIm->dim.z; z++)
        for (y=0; y<(int)resIm->dim.y; y++)
        for (x=0; x<(int)resIm->dim.x; x++) {
				  *(buf+i) = (i32)(*(vl+i));
				  i++;
			  }}
			  break;
		  case FLOAT :
			  {
			  r32 *buf;
			  buf = (r32 *)(resIm->buf);
        for (z=0; z<(int)resIm->dim.z; z++)
        for (y=0; y<(int)resIm->dim.y; y++)
        for (x=0; x<(int)resIm->dim.x; x++) {
				  *(buf+i) = (r32)(*(vl+i));
			  }}
			  break;
		  default :
			  VT_Error( "input image type unknown or not supported", "VT_Dist" );
			  if ( (resIm->type != USHORT) && (resIm->type != SSHORT) )
				  VT_FreeImage( &auxIm );
			  return( -1 );
		  }
	  }
	  if ( (resIm->type != USHORT) && (resIm->type != SSHORT) )
		  VT_FreeImage( &auxIm );
	  break;
  case VT_DIST_EUCLI :
  case VT_DIST_EUCLI_2 :
      return( VT_EucliDist( resIm, theIm, par ) );
  default :
	  VT_Error( "distance type unknown or unsupported", "VT_Dist" );
	  return( 0 );
  }
  return( 1 );
}





static int _VT_IDistance( vt_image *resIm, vt_image *theIm, vt_distance *par )
{
  int offset[27];
  register int i, x, y, z, test;
  int dimx, dimy, dimz, plan, local_dim;
  u16 *vl;
  vt_distance local_par;
  
  if ( VT_Test2Image( resIm, theIm, "_VT_IDistance" ) == -1 ) return( -1 );
  if ( resIm->type != USHORT ) {
	  VT_Error( "output image has a bad type", "_VT_IDistance" );
	  return( -1 );
  }
  if ( ( theIm->dim.x < 3 ) || ( theIm->dim.y < 3 ) ) {
	  VT_Error( "images have bad dimensions", "_VT_IDistance" );
	  return( -1 );
  }
  
  /*--- mise a jour des parametres locaux : resolution des conflits ---*/
  local_par = *par;
  if ( local_par.dim == VT_2D ) {
	  switch ( local_par.type ) {
	  case VT_DIST_6 :
		  local_par.type = VT_DIST_6_2D;
		  break;
	  case VT_DIST_10 :
		  local_par.type = VT_DIST_10_2D;
		  break;
	  case VT_DIST_18 :
		  local_par.type = VT_DIST_18_2D;
		  break;
	  case VT_DIST_26 :
		  local_par.type = VT_DIST_26_2D;
		  break;
	  case VT_DIST_CHMFR :
		  local_par.type = VT_DIST_CHMFR_2D;
		  break;
	  case VT_DIST_CHMFR2 :
		  local_par.type = VT_DIST_CHMFR2_2D;
		  break;
	  case VT_DIST_CHMFR3 :
		  local_par.type = VT_DIST_CHMFR3_2D;
	  }
  }
  switch ( local_par.type ) {
  case VT_DIST_4 :
  case VT_DIST_8 :
  case VT_DIST_CHMFR_2D :
  case VT_DIST_CHMFR2_2D :
  case VT_DIST_CHMFR3_2D :
	  local_par.dim = VT_2D;
  }
  /*--- specification des increments ---*/
  switch ( local_par.type ) {
  case VT_DIST_6 :
  case VT_DIST_10 :
  case VT_DIST_18 :
  case VT_DIST_26 :	  
  case VT_DIST_4 :
  case VT_DIST_8 :
	  local_par.inc[0] = 1;
	  local_par.inc[1] = 1;
	  local_par.inc[2] = 1;
	  local_par.inc[3] = 1;
	  local_par.inc[4] = 1;
	  break;
  case VT_DIST_CHMFR :
  case VT_DIST_CHMFR2 :
  case VT_DIST_CHMFR_2D :
  case VT_DIST_CHMFR2_2D :
	  local_par.inc[0] = 16;
	  local_par.inc[1] = 21;
	  local_par.inc[2] = 16;
	  local_par.inc[3] = 21;
	  local_par.inc[4] = 26;
	  break;
  case VT_DIST_CHMFR3 :
  case VT_DIST_CHMFR3_2D :
	  local_par.inc[0] = 32;
	  local_par.inc[1] = 42;
	  local_par.inc[2] = 32;
	  local_par.inc[3] = 42;
	  local_par.inc[4] = 52;
  }

  /*--- pre-calcul des offsets ---*/
  dimx = theIm->dim.x;
  dimy = theIm->dim.y;
  dimz = theIm->dim.z;
  plan = dimx*dimy;
  offset[0]  = - plan - dimx - 1;   /* point (-1, -1, -1) */
  offset[1]  = - plan - dimx;       /* point ( 0, -1, -1) */
  offset[2]  = - plan - dimx + 1;   /* point ( 1, -1, -1) */
  offset[3]  = - plan - 1;          /* point (-1,  0, -1) */
  offset[4]  = - plan;              /* point ( 0,  0, -1) */
  offset[5]  = - plan + 1;          /* point ( 1,  0, -1) */
  offset[6]  = - plan + dimx - 1;   /* point (-1,  1, -1) */
  offset[7]  = - plan + dimx;       /* point ( 0,  1, -1) */
  offset[8]  = - plan + dimx + 1;   /* point ( 1,  1, -1) */
  offset[9]  =        - dimx - 1;   /* point (-1, -1,  0) */
  offset[10] =        - dimx;       /* point ( 0, -1,  0) */
  offset[11] =        - dimx + 1;   /* point ( 1, -1,  0) */
  offset[12] =        - 1;          /* point (-1,  0,  0) */
  offset[13] =          0;          /* point ( 0,  0,  0) */
  offset[14] =          1;          /* point ( 1,  0,  0) */
  offset[15] =          dimx - 1;   /* point (-1,  1,  0) */
  offset[16] =          dimx;       /* point ( 0,  1,  0) */
  offset[17] =          dimx + 1;   /* point ( 1,  1,  0) */
  offset[18] =   plan - dimx - 1;   /* point (-1, -1,  1) */
  offset[19] =   plan - dimx;       /* point ( 0, -1,  1) */
  offset[20] =   plan - dimx + 1;   /* point ( 1, -1,  1) */
  offset[21] =   plan - 1;          /* point (-1,  0,  1) */
  offset[22] =   plan;              /* point ( 0,  0,  1) */
  offset[23] =   plan + 1;          /* point ( 1,  0,  1) */
  offset[24] =   plan + dimx - 1;   /* point (-1,  1,  1) */
  offset[25] =   plan + dimx;       /* point ( 0,  1,  1) */
  offset[26] =   plan + dimx + 1;   /* point ( 1,  1,  1) */

  /*--- dernieres affectations ---*/
  local_dim = local_par.dim;
  vl = (u16 *)(resIm->buf);
  
  /*--- preparation de l'image d'entree ---*/
  _VT_ISeuil( resIm, theIm, offset, &local_par );

  /*   boucle de la descente   */
  /*****************************/
  local_par.dim = VT_2D;
  i = 0;
  for (z=0; z<dimz; z++) {
	  /*--- coin superieur gauche ---*/
	  *(vl+i) = (u16)_VT_DCsg(vl, i, offset, &local_par);
	  i++;
	  /*--- ligne superieure ---*/
	  for (x=1; x<dimx-1; x++) {
		  *(vl+i) = (u16)_VT_DLs(vl, i, offset, &local_par);
		  i++;
	  }
	  /*--- coin superieur droit ---*/
	  *(vl+i) = (u16)_VT_DCsd(vl, i, offset, &local_par);
	  i++;
	  /*--- autres lignes ---*/
	  for (y=1; y<dimy-1; y++) {
		  /*--- autres lignes : colonne de gauche ---*/
		  *(vl+i) = (u16)_VT_DCg(vl, i, offset, &local_par);
		  i++;
		  /*--- autres lignes : centre ---*/
		  for (x=1; x<dimx-1; x++) {
			  /*--- *(vl+i) = (u16)_VT_DCe(vl, i, offset, &local_par); ---*/
			  test = *(vl+i);
			  switch ( local_par.type ) {
			  case VT_DIST_CHMFR :
			  case VT_DIST_CHMFR2 :
			  case VT_DIST_CHMFR3 :
			  case VT_DIST_CHMFR_2D :
			  case VT_DIST_CHMFR2_2D :
			  case VT_DIST_CHMFR3_2D :
			  case VT_DIST_USER :
			  case VT_DIST_26 :
			  case VT_DIST_18 :
			  case VT_DIST_10 :
			  case VT_DIST_8 :
				  test = VT_MIN( test, local_par.inc[1] + *(vl+i+offset[9]) );    /* (-1, -1,  0) */
				  test = VT_MIN( test, local_par.inc[1] + *(vl+i+offset[11]) );   /* ( 1, -1,  0) */
			  case VT_DIST_6 :
			  case VT_DIST_4 :
				  test = VT_MIN( test, local_par.inc[0] + *(vl+i+offset[10]) );   /* ( 0, -1,  0) */
				  test = VT_MIN( test, local_par.inc[0] + *(vl+i+offset[12]) );   /* (-1,  0,  0) */ 
			  }
			  if ( local_par.dim == VT_2D ) { *(vl+i) = test; i++; continue; }
			  switch ( local_par.type ) {
			  case VT_DIST_CHMFR :
			  case VT_DIST_CHMFR2 :
			  case VT_DIST_CHMFR3 :
			  case VT_DIST_USER :
			  case VT_DIST_26 :
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[0]) );    /* (-1, -1, -1) */
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[2]) );    /* ( 1, -1, -1) */
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[6]) );    /* (-1,  1, -1) */
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[8]) );    /* ( 1,  1, -1) */
			  case VT_DIST_18 :
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[1]) );    /* ( 0, -1, -1) */
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[3]) );    /* (-1,  0, -1) */
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[5]) );    /* ( 1,  0, -1) */
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[7]) );    /* ( 0,  1, -1) */
			  case VT_DIST_10 :
			  case VT_DIST_6 :
				  test = VT_MIN( test, local_par.inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
			  }
			  *(vl+i) = test; i++;
		  }
		  /*--- autres lignes : colonne de droite ---*/
		  *(vl+i) = (u16)_VT_DCd(vl, i, offset, &local_par);
		  i++;
	  }
	  /*--- coin inferieur gauche ---*/
	  *(vl+i) = (u16)_VT_DCig(vl, i, offset, &local_par);
	  i++;
	  
	  /*--- ligne inferieure ---*/
	  for (x=1; x<dimx-1; x++) {
		  *(vl+i) = (u16)_VT_DLi(vl, i, offset, &local_par);
		  i++;
	  }
	  /*--- coin inferieur droit ---*/
	  *(vl+i) = (u16)_VT_DCid(vl, i, offset, &local_par);
	  i++;
	  local_par.dim = local_dim;
  }
  
  /*   boucle de la remontee   */
  /*****************************/
  local_par.dim = VT_2D;
  i = dimz*plan-1;
  for (z=0; z<dimz; z++) {
	  /*--- coin inferieur droit ---*/
	  *(vl+i) = (u16)_VT_RCid(vl, i, offset, &local_par);
	  i--;
	  /*--- ligne inferieure ---*/
	  for (x=1; x<dimx-1; x++) {
		  *(vl+i) = (u16)_VT_RLi(vl, i, offset, &local_par);
		  i--;
	  }
	  /*--- coin inferieur gauche ---*/
	  *(vl+i) = (u16)_VT_RCig(vl, i, offset, &local_par);
	  i--;
	  /*--- autres lignes ---*/
	  for (y=1; y<dimy-1; y++) {
		  /*--- autres lignes : colonne droite ---*/
		  *(vl+i) = (u16)_VT_RCd(vl, i, offset, &local_par);
		  i--;
		  /*--- autres lignes : centre ---*/
		  for (x=1; x<dimx-1; x++) {
			  /*--- *(vl+i) = (u16)_VT_RCe(vl, i, offset, &local_par); ---*/
			  test = *(vl+i);
			  switch ( local_par.type ) {
			  case VT_DIST_CHMFR :
			  case VT_DIST_CHMFR2 :
			  case VT_DIST_CHMFR3 :
			  case VT_DIST_CHMFR_2D :
			  case VT_DIST_CHMFR2_2D :
			  case VT_DIST_CHMFR3_2D :
			  case VT_DIST_USER :
			  case VT_DIST_26 :
			  case VT_DIST_18 :
			  case VT_DIST_10 :
			  case VT_DIST_8 :
				  test = VT_MIN( test, local_par.inc[1] + *(vl+i+offset[15]) );   /* (-1,  1,  0) */
				  test = VT_MIN( test, local_par.inc[1] + *(vl+i+offset[17]) );   /* ( 1,  1,  0) */
			  case VT_DIST_6 :
			  case VT_DIST_4 :
				  test = VT_MIN( test, local_par.inc[0] + *(vl+i+offset[14]) );   /* ( 1,  0,  0) */ 
				  test = VT_MIN( test, local_par.inc[0] + *(vl+i+offset[16]) );   /* ( 0,  1,  0) */
			  }
			  if ( local_par.dim == VT_2D ) { *(vl+i) = test; i--; continue; }
			  switch ( local_par.type ) {
			  case VT_DIST_CHMFR :
			  case VT_DIST_CHMFR2 :
			  case VT_DIST_CHMFR3 :
			  case VT_DIST_USER :
			  case VT_DIST_26 :
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[18]) );   /* (-1, -1,  1) */
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[20]) );   /* ( 1, -1,  1) */
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[24]) );   /* (-1,  1,  1) */
				  test = VT_MIN( test, local_par.inc[4] + *(vl+i+offset[26]) );   /* ( 1,  1,  1) */
			  case VT_DIST_18 :
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[19]) );   /* ( 0, -1,  1) */
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[21]) );   /* (-1,  0,  1) */
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[23]) );   /* ( 1,  0,  1) */
				  test = VT_MIN( test, local_par.inc[3] + *(vl+i+offset[25]) );   /* ( 0,  1,  1) */
			  case VT_DIST_10 :
			  case VT_DIST_6 :
				  test = VT_MIN( test, local_par.inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
			  }
			  *(vl+i) = test; i--; 
		  }
		  /*--- autres lignes : colonne de gauche ---*/
		  *(vl+i) = (u16)_VT_RCg(vl, i, offset, &local_par);
		  i--;
	  }
	  /*--- coin superieur droit ---*/
	  *(vl+i) = (u16)_VT_RCsd(vl, i, offset, &local_par);
	  i--;
	  /*--- ligne superieure ---*/
	  for (x=1; x<dimx-1; x++) {
		  *(vl+i) = (u16)_VT_RLs(vl, i, offset, &local_par);
		  i--;
	  }
	  /*--- coin superieur gauche ---*/
	  *(vl+i) = (u16)_VT_RCsg(vl, i, offset, &local_par);
	  i--;
	  local_par.dim = local_dim;
  }
  
  if ( ( par->type == VT_DIST_CHMFR ) || ( par->type == VT_DIST_CHMFR_2D ) ) {
	  /*--- division par 16 ---*/
	  for (i=0; i<dimz*plan; i++)
		  *(vl+i) = *(vl+i) >> 4;
  }
  
  return( 1 );
}





static int _VT_DCsg( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente ---*/
	/*--- coin superieur gauche ---*/
	int test;
	test = *(vl+i);
  
	if ( par->dim == VT_2D ) return( test );
	
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[8]) );    /* ( 1,  1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[5]) );    /* ( 1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[7]) );    /* ( 0,  1, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}
	
	return( test );
}





static int _VT_DLs( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente  ---*/
	/*--- ligne superieure       ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[12]) );   /* (-1,  0,  0) */ 
	}
	
	if ( par->dim == VT_2D ) return( test );
  
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[6]) );    /* (-1,  1, -1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[8]) );    /* ( 1,  1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[3]) );    /* (-1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[5]) );    /* ( 1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[7]) );    /* ( 0,  1, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}
  
	return( test );
}





static int _VT_DCsd( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente  ---*/
	/*--- cois superieur droit   ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[12]) );   /* (-1,  0,  0) */ 
	}
	
	if ( par->dim == VT_2D ) return( test );
  
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[6]) );    /* (-1,  1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[3]) );    /* (-1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[7]) );    /* ( 0,  1, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}
	
	return( test );
}





static int _VT_DCg( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente  ---*/
	/*--- colonne de gauche      ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[11]) );   /* ( 1, -1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[10]) );   /* ( 0, -1,  0) */
	}

	if ( par->dim == VT_2D ) return( test );
  
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[2]) );    /* ( 1, -1, -1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[8]) );    /* ( 1,  1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[1]) );    /* ( 0, -1, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[5]) );    /* ( 1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[7]) );    /* ( 0,  1, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}

	return( test );
}





#ifdef _UNUSED_
static int _VT_DCe( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente  ---*/
	/*--- autres lignes : centre ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[9]) );    /* (-1, -1,  0) */
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[11]) );   /* ( 1, -1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[10]) );   /* ( 0, -1,  0) */
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[12]) );   /* (-1,  0,  0) */ 
	}
	
	if ( par->dim == VT_2D ) return( test );
	
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[0]) );    /* (-1, -1, -1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[2]) );    /* ( 1, -1, -1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[6]) );    /* (-1,  1, -1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[8]) );    /* ( 1,  1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[1]) );    /* ( 0, -1, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[3]) );    /* (-1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[5]) );    /* ( 1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[7]) );    /* ( 0,  1, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}
	
	return( test );
}
#endif





static int _VT_DCd( u16 *vl, int i, int *offset, vt_distance *par )
{
 	/*--- boucle de la descente  ---*/
	/*--- colonne de droite      ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[9]) );    /* (-1, -1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[10]) );   /* ( 0, -1,  0) */
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[12]) );   /* (-1,  0,  0) */ 
	}
	
	if ( par->dim == VT_2D ) return( test );
  
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[0]) );    /* (-1, -1, -1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[6]) );    /* (-1,  1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[1]) );    /* ( 0, -1, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[3]) );    /* (-1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[7]) );    /* ( 0,  1, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}
  
	return( test );
}





static int _VT_DCig( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente  ---*/
	/*--- coin inferieur gauche  ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[11]) );   /* ( 1, -1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[10]) );   /* ( 0, -1,  0) */
	}
	
	if ( par->dim == VT_2D ) return( test );
	
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[2]) );    /* ( 1, -1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[1]) );    /* ( 0, -1, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[5]) );    /* ( 1,  0, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}

	return( test );
}





static int _VT_DLi( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente  ---*/
	/*--- ligne inferieure       ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[9]) );    /* (-1, -1,  0) */
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[11]) );   /* ( 1, -1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[10]) );   /* ( 0, -1,  0) */
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[12]) );   /* (-1,  0,  0) */ 
	}
	
	if ( par->dim == VT_2D ) return( test );
  
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[0]) );    /* (-1, -1, -1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[2]) );    /* ( 1, -1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[1]) );    /* ( 0, -1, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[3]) );    /* (-1,  0, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[5]) );    /* ( 1,  0, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}
  
	return( test );
}





static int _VT_DCid( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la descente  ---*/
	/*--- coin inferieur droit   ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
 	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[9]) );    /* (-1, -1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[10]) );   /* ( 0, -1,  0) */
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[12]) );   /* (-1,  0,  0) */ 
	}
	
	if ( par->dim == VT_2D ) return( test );
	  
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[0]) );    /* (-1, -1, -1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[1]) );    /* ( 0, -1, -1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[3]) );    /* (-1,  0, -1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[4]) );    /* ( 0,  0, -1) */
	}

	return( test );
}





static int _VT_RCid( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- coin inferieur droit   ---*/
	int test;
	test = *(vl+i);
  
	if ( par->dim == VT_2D ) return( test );
	
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[18]) );   /* (-1, -1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[19]) );   /* ( 0, -1,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[21]) );   /* (-1,  0,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
	
	return( test );
}





static int _VT_RLi( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- ligne inferieure       ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[14]) );   /* ( 1,  0,  0) */ 
	}
	
	if ( par->dim == VT_2D ) return( test );
	
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[18]) );   /* (-1, -1,  1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[20]) );   /* ( 1, -1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[19]) );   /* ( 0, -1,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[21]) );   /* (-1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[23]) );   /* ( 1,  0,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
  
	return( test );
}





static int _VT_RCig( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- coin inferieur gauche  ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[14]) );   /* ( 1,  0,  0) */ 
	}

	if ( par->dim == VT_2D ) return( test );
  
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[20]) );   /* ( 1, -1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[19]) );   /* ( 0, -1,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[23]) );   /* ( 1,  0,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
	
	return( test );
}





static int _VT_RCd( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- colonne de droite      ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[15]) );   /* (-1,  1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[16]) );   /* ( 0,  1,  0) */
	}
	
	if ( par->dim == VT_2D ) return( test );

	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[18]) );   /* (-1, -1,  1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[24]) );   /* (-1,  1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[19]) );   /* ( 0, -1,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[21]) );   /* (-1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[25]) );   /* ( 0,  1,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}

	return( test );
}





#ifdef _UNUSED_
static int _VT_RCe( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- autres lignes : centre ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[15]) );   /* (-1,  1,  0) */
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[17]) );   /* ( 1,  1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[14]) );   /* ( 1,  0,  0) */ 
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[16]) );   /* ( 0,  1,  0) */
	}
	
	if ( par->dim == VT_2D ) return( test );
	
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[18]) );   /* (-1, -1,  1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[20]) );   /* ( 1, -1,  1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[24]) );   /* (-1,  1,  1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[26]) );   /* ( 1,  1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[19]) );   /* ( 0, -1,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[21]) );   /* (-1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[23]) );   /* ( 1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[25]) );   /* ( 0,  1,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
	
	return( test );
}
#endif





static int _VT_RCg( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- autres lignes : centre ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[17]) );   /* ( 1,  1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[14]) );   /* ( 1,  0,  0) */ 
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[16]) );   /* ( 0,  1,  0) */
	}
	
	if ( par->dim == VT_2D ) return( test );

	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[20]) );   /* ( 1, -1,  1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[26]) );   /* ( 1,  1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[19]) );   /* ( 0, -1,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[23]) );   /* ( 1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[25]) );   /* ( 0,  1,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
  
	return( test );
}





static int _VT_RCsd( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- coin superieur droit   ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[15]) );   /* (-1,  1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[16]) );   /* ( 0,  1,  0) */
	}
	
	if ( par->dim == VT_2D ) return( test );
	
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[24]) );   /* (-1,  1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[21]) );   /* (-1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[25]) );   /* ( 0,  1,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
  
	return( test );
}





static int _VT_RLs( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- ligne superieure       ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[15]) );   /* (-1,  1,  0) */
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[17]) );   /* ( 1,  1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[14]) );   /* ( 1,  0,  0) */ 
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[16]) );   /* ( 0,  1,  0) */
	}
	
	if ( par->dim == VT_2D ) return( test );

	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[24]) );   /* (-1,  1,  1) */
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[26]) );   /* ( 1,  1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[21]) );   /* (-1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[23]) );   /* ( 1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[25]) );   /* ( 0,  1,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
	
	return( test );
}





static int _VT_RCsg( u16 *vl, int i, int *offset, vt_distance *par )
{
	/*--- boucle de la remontee  ---*/
	/*--- coin superieur gauche  ---*/
	int test;
	test = *(vl+i);
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_CHMFR_2D :
	case VT_DIST_CHMFR2_2D :
	case VT_DIST_CHMFR3_2D :
	case VT_DIST_USER :
	case VT_DIST_26 :
	case VT_DIST_18 :
	case VT_DIST_10 :
	case VT_DIST_8 :
		test = VT_MIN( test, par->inc[1] + *(vl+i+offset[17]) );   /* ( 1,  1,  0) */
	case VT_DIST_6 :
	case VT_DIST_4 :
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[14]) );   /* ( 1,  0,  0) */ 
		test = VT_MIN( test, par->inc[0] + *(vl+i+offset[16]) );   /* ( 0,  1,  0) */
	}
	
	if ( par->dim == VT_2D ) return( test );
			
	switch ( par->type ) {
	case VT_DIST_CHMFR :
	case VT_DIST_CHMFR2 :
	case VT_DIST_CHMFR3 :
	case VT_DIST_USER :
	case VT_DIST_26 :
		test = VT_MIN( test, par->inc[4] + *(vl+i+offset[26]) );   /* ( 1,  1,  1) */
	case VT_DIST_18 :
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[23]) );   /* ( 1,  0,  1) */
		test = VT_MIN( test, par->inc[3] + *(vl+i+offset[25]) );   /* ( 0,  1,  1) */
	case VT_DIST_10 :
	case VT_DIST_6 :
		test = VT_MIN( test, par->inc[2] + *(vl+i+offset[22]) );   /* ( 0,  0,  1) */
	}
  
	return( test );
}





static void _VT_ISeuil( vt_image *resIm, vt_image *theIm, int *offset, vt_distance *par )
{
    register int is, x, y, z;
    size_t i, v;
    int dx1, dy1, dz1;
    register float fs;
    u16 *vl;
    
    fs = par->seuil;
    is = VT_F2I( fs );
    v = theIm->dim.x * theIm->dim.y * theIm->dim.z;
    vl = (u16*)(resIm->buf);
    
    switch ( theIm->type ) {
    case UCHAR :
        {
	u8 *buf;
	buf = (u8 *)(theIm->buf);
	for (i=0; i<v; i++) 
	    *vl++ = ( *buf++ < is ) ? VT_UINFINI : 0;
	}
	break;
    case USHORT :
        {
	u16 *buf;
	buf = (u16 *)(theIm->buf);
	for (i=0; i<v; i++) 
	    *vl++ = ( *buf++ < is ) ? VT_UINFINI : 0;
	}
	break;
    case SSHORT :
        {
	s16 *buf;
	buf = (s16 *)(theIm->buf);
	for (i=0; i<v; i++) 
	    *vl++ = ( *buf++ < is ) ? VT_UINFINI : 0;
	}
	break;
    case SINT :
        {
	i32 *buf;
	buf = (i32 *)(theIm->buf);
	for (i=0; i<v; i++) 
	    *vl++ = ( *buf++ < is ) ? VT_UINFINI : 0;
	}
	break;
    case FLOAT :
        {
	r32 *buf;
	buf = (r32 *)(theIm->buf);
	for (i=0; i<v; i++) 
	    *vl++ = ( *buf++ < fs ) ? VT_UINFINI : 0;
	}
    default :
      return;
    }
    
    if ( (par->type != VT_DIST_CHMFR3_2D) && (par->type != VT_DIST_CHMFR3) ) 
	return;
    
    /*--- post-seuillage : les points immediatement voisins de l'objet
          ont des valeurs particulieres                                 ---*/

    dx1 = theIm->dim.x - 1;
    dy1 = theIm->dim.y - 1;
    dz1 = theIm->dim.z - 1;
    i = 0;
    for (z = 0; z < (int)theIm->dim.z; z ++ )
    for (y = 0; y < (int)theIm->dim.y; y ++ )
    for (x = 0; x < (int)theIm->dim.x; x ++ ) {
	if ( *(vl + i) == 0 ) continue;
	/*--- 6-voisins ---*/
	if ( x > 0 ) {
	    if ( *(vl + i + offset[12]) == 0 ) *(vl + i) = 16;
	    continue;
	}
	if ( x < dx1 ) {
	    if ( *(vl + i + offset[14]) == 0 ) *(vl + i) = 16;
	    continue;
	}
	if ( y > 0 ) {
	    if ( *(vl + i + offset[10]) == 0 ) *(vl + i) = 16;
	    continue;
	}
	if ( y < dy1 ) {
	    if ( *(vl + i + offset[16]) == 0 ) *(vl + i) = 16;
	    continue;
	}
	if ( par->type == VT_DIST_CHMFR3 ) {
	    if ( z > 0 ) {
		if ( *(vl + i + offset[4]) == 0 ) *(vl + i) = 16;
		continue;
	    }
	    if ( z < dz1 ) {
		if ( *(vl + i + offset[22]) == 0 ) *(vl + i) = 16;
		continue;
	    }
	}
	/*--- 18-voisins ---*/
	if ( (x > 0) && (y > 0) ) {
	    if ( *(vl + i + offset[9]) == 0 ) *(vl + i) = 21;
	    continue;
	}
	if ( (x > 0) && (y < dy1) ) {
	    if ( *(vl + i + offset[15]) == 0 ) *(vl + i) = 21;
	    continue;
	}
	if ( (x < dx1) && (y > 0) ) {
	    if ( *(vl + i + offset[11]) == 0 ) *(vl + i) = 21;
	    continue;
	}
	if ( (x < dx1) && (y < dy1) ) {
	    if ( *(vl + i + offset[17]) == 0 ) *(vl + i) = 21;
	    continue;
	}
	if ( par->type == VT_DIST_CHMFR3 ) {
	    if ( (x > 0) && (z > 0) ) {
		if ( *(vl + i + offset[3]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	    if ( (x > 0) && (z < dz1) ) {
		if ( *(vl + i + offset[21]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	    if ( (x < dx1) && (z > 0) ) {
		if ( *(vl + i + offset[5]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	    if ( (x < dx1) && (z < dz1) ) {
		if ( *(vl + i + offset[23]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	    if ( (z > 0) && (y > 0) ) {
		if ( *(vl + i + offset[1]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	    if ( (z > 0) && (y < dy1) ) {
		if ( *(vl + i + offset[7]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	    if ( (z < dz1) && (y > 0) ) {
		if ( *(vl + i + offset[19]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	    if ( (z < dz1) && (y < dy1) ) {
		if ( *(vl + i + offset[25]) == 0 ) *(vl + i) = 21;
		continue;
	    }
	}
	/*--- 26-voisins ---*/
	if ( par->type == VT_DIST_CHMFR3_2D ) continue;
	if ( (x > 0) && (y > 0) && (z > 0) ) {
	    if ( *(vl + i + offset[0]) == 0 ) *(vl + i) = 26;
	    continue;
	}
	if ( (x < dx1) && (y > 0) && (z > 0) ) {
	    if ( *(vl + i + offset[2]) == 0 ) *(vl + i) = 26;
	    continue;
	}
	if ( (x > 0) && (y < dy1) && (z > 0) ) {
	    if ( *(vl + i + offset[6]) == 0 ) *(vl + i) = 26;
	    continue;
	}
	if ( (x < dx1) && (y < dy1) && (z > 0) ) {
	    if ( *(vl + i + offset[8]) == 0 ) *(vl + i) = 26;
	    continue;
	}
	if ( (x > 0) && (y > 0) && (z < dz1) ) {
	    if ( *(vl + i + offset[18]) == 0 ) *(vl + i) = 26;
	    continue;
	}
	if ( (x < dx1) && (y > 0) && (z < dz1) ) {
	    if ( *(vl + i + offset[20]) == 0 ) *(vl + i) = 26;
	    continue;
	}
	if ( (x > 0) && (y < dy1) && (z < dz1) ) {
	    if ( *(vl + i + offset[24]) == 0 ) *(vl + i) = 26;
	    continue;
	}
	if ( (x < dx1) && (y < dy1) && (z < dz1) ) {
	    if ( *(vl + i + offset[26]) == 0 ) *(vl + i) = 26;
	    continue;
	}
    }
}
