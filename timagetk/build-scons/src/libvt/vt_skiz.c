/*************************************************************************
 * skiz.c -
 *
 * $Id: vt_skiz.c,v 1.4 2001/04/09 10:24:47 greg Exp $
 *
 * DESCRIPTION: 
 *
 *
 *
 *
 *
 * AUTHOR:
 * Gregoire Malandain
 *
 * CREATION DATE:
 * ???
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 * - Mon Mar 19 19:16:47 MET 2001
 *   L'image de distance est passee en parametre.
 *
 *
 */

#include <string.h>
#include <vt_skiz.h>


#define EXIT_ON_FAILURE 0
#define EXIT_ON_SUCCESS 1
static int _VERBOSE_ = 1;
static int _DILATION_RADIUS_ = 0;

/*
 * pour ecrire l'image de distance
 */ 
static typeBoolean writeDistanceImage = False;
static char distanceImageName[ STRINGLENGTH ];

typedef struct {
  int x;
  int y;
  int z;
  int i;
} typeIncrement;

static int VT_ConstrainedSkizInDistanceMapWithChamferInUShort( vt_image *imLabels, 
							       vt_image *imMask,
							       vt_image *imDist,
							       vt_image *imOutput,
							       int chamferSize,
							       int *inc );


int VT_ConstrainedSkizWithChamfer( vt_image *imLabels, 
				   vt_image *imMask,
				   vt_image *imOutput,
				   int chamferSize,
				   int *inc )
{
  char *proc="VT_ConstrainedSkizWithChamfer";
  vt_image imDist;
  
  switch( imLabels->type ) {
  case UCHAR :
  case USHORT :
    break;
  default :
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, "%s: such type not handled yet.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }

  /* allocation of distance image
   */
  VT_InitImage( &imDist, distanceImageName, 
		imLabels->dim.x, imLabels->dim.y, imLabels->dim.z, USHORT );
  if ( VT_AllocImage( &imDist ) == 0 ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, "%s: unable to allocate auxiliary image.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }
  
  switch( imLabels->type ) {
  case UCHAR :
    return( VT_ConstrainedSkizInDistanceMapWithChamfer( imLabels, imMask, 
							&imDist, imOutput,
							chamferSize, inc ) );
  case USHORT :
    return( VT_ConstrainedSkizInDistanceMapWithChamferInUShort( imLabels, imMask, 
								&imDist, imOutput,
								chamferSize, inc ) );
  default :
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, "%s: such type not handled yet.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }

  if ( writeDistanceImage == True ) {
    VT_WriteInrimage( &imDist );
  }

  VT_FreeImage( &imDist );
  return( EXIT_ON_SUCCESS );
}







int VT_ConstrainedSkizInDistanceMapWithChamfer( vt_image *imLabels, 
						vt_image *imMask,
						vt_image *imDist,
						vt_image *imOutput,
						int chamferSize,
						int *increments )
{
  char *proc="VT_ConstrainedSkizInDistanceMapWithChamfer";
  int dimx, dimy, dimz;
  int bord;
  int dimxMinusOne, dimyMinusOne, dimzMinusOne;
  int dimxMinusBord, dimyMinusBord, dimzMinusBord;
  register int x, y, z;
  register int xn, yn, zn;
  u8 ***arrayLabels;
  u8 ***arrayMask;
  u8 ***arrayOutput;
  int distance_infinie;
  u16 ***arrayDist;
  register int distance_courante, d, l;
  register int nb_changes;

  int inc[12];

  /* 
   * 29 est un maximum pour une distance de chamfer 5x5x5
   *
   *  . c . c .   f e c e f   . f . f .
   *  c b a b c   e d b d e   f e c e f
   *  . a X       c b a       . c .
   *     z=0         z=1         z=2
   */
  typeIncrement forw[49], back[49]; 
  int n, nbIncrements=0;
  int nbIterations = 0;
  /*
   * pour gerer le rayon
   */
  int maxRadius = _DILATION_RADIUS_ * inc[0];
  typeBoolean radiusHasToBeChanged = False;
  typeBoolean processingIsDone = False;
  /*
   * pour verifier qu'un 6-voisin a le meme label
   */ 
  int labelsAdmissibles[6];
  int nbLabels;
  typeBoolean foundLabel;
  register int label;



  if ( (imLabels->type == USHORT) && (imOutput->type == USHORT) ) {
    return( VT_ConstrainedSkizInDistanceMapWithChamferInUShort( imLabels, 
						   imMask, imDist,
						   imOutput,
						   chamferSize,
						   increments ) );
  }



  /*
   * Tests
   */
  if ( (imLabels->dim.x != imMask->dim.x) || (imLabels->dim.x != imOutput->dim.x) ||
       (imLabels->dim.y != imMask->dim.y) || (imLabels->dim.y != imOutput->dim.y) ||
       (imLabels->dim.z != imMask->dim.z) || (imLabels->dim.z != imOutput->dim.z) ||
       (imLabels->dim.x != imDist->dim.x) ||
       (imLabels->dim.y != imDist->dim.y) ||
       (imLabels->dim.z != imDist->dim.z) ) {
    if ( _VERBOSE_ != 0 ) {
	fprintf( stderr, "%s: images have different dimensions.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }
  if ( (imLabels->type != UCHAR) || (imMask->type != UCHAR) ||
       (imOutput->type != UCHAR) ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, "%s: images should be of unsigned char type.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }
  if ( imDist->type != USHORT ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, "%s: distance image should be of unsigned short int type.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }




  /*
   * valeur max autorisee 
   * c'est l'infini pour le calcul
   * cela depend du codage utilise
   */
  distance_infinie = 65535;

  /*
   * initialisation des images 
   */
  arrayLabels = (u8 ***)(imLabels->array);
  arrayMask = (u8 ***)(imMask->array);
  arrayOutput = (u8 ***)(imOutput->array);
  arrayDist = (u16 ***)(imDist->array);
  dimx = imLabels->dim.x;
  dimy = imLabels->dim.y;
  dimz = imLabels->dim.z;
  
  /*
   * Les etiquettes finales sont deja les etiquettes
   * initiales, sauf en dehors du mask : c'est 0.
   * La distance est a l'infini, sauf en dehors du mask
   * et sur les etiquettes initiales ou elle est a 0.
   * Ainsi on ne remettra a jour que les points dans le 
   * masque qui ne sont pas des etiquettes initiales.
   */
  for ( z = 0; z < dimz; z ++ )
  for ( y = 0; y < dimy; y ++ )
  for ( x = 0; x < dimx; x ++ ) {
    arrayOutput[z][y][x] = arrayLabels[z][y][x];
    arrayDist[z][y][x] = distance_infinie;
    if ( arrayLabels[z][y][x] > 0 ) arrayDist[z][y][x] = 0;
    if ( arrayMask[z][y][x] == 0 ) {
      arrayDist[z][y][x] = 0;
      arrayOutput[z][y][x] = 0;
    }
  }

  /*
   * bordure
   */
  switch ( chamferSize ) {
  case 3 :
  default :
    bord = 1;
    break;
  case 5 :
    bord = 2;
  }
  dimxMinusBord = dimx - bord;
  dimyMinusBord = dimy - bord;
  dimzMinusBord = dimz - bord;
  dimxMinusOne = dimx - 1;
  dimyMinusOne = dimy - 1;
  dimzMinusOne = dimz - 1;


  if ( increments == NULL ) {
    switch ( chamferSize ) {
    case 3 :
    default :
      inc[0] = 16; /* les 4-voisins dans le plan (dx=1 ou dy=1) */
      inc[1] = 21; /* les 4-voisins dans le plan (dx=1 et dy=1) */
      
      inc[2] = 16; /* dz=1 et dx=0 et dy=0 */
      inc[3] = 21; /* dz=1 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[4] = 26; /* dz=1 et (dx=1 et dy=1) */
      break;
    case 5 :
      inc[0] = 13; /* dz=0 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[1] = 18; /* dz=0 et (dx=1 et dy=1) */
      inc[2] = 29; /* dz=0 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */

      inc[3] = 13; /* dz=1 et dx=0 et dy=0 */
      inc[4] = 18; /* dz=1 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[5] = 23; /* dz=1 et (dx=1 et dy=1) */
      inc[6] = 29; /* dz=1 et ( (dx=0 et dy=2) ou (dx=2 et dy=0) ) */
      inc[7] = 32; /* dz=1 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */
      inc[8] = 39; /* dz=1 et (dx=2 et dy=2) */

      inc[9] = 29; /* dz=2 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[10] = 32; /* dz=2 et (dx=1 et dy=1) */
      inc[11] = 39; /* dz=2 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */
    }
  } else {
    (void)memcpy( inc, increments, 12*sizeof(int) );
  }


  switch ( chamferSize ) {
  case 3 :
  default :
    /*
     * les increments 2D
     * forward et backward
     */
    nbIncrements = 4;
    
    forw[0].x = -1;   forw[0].y =  0;   forw[0].z =  0;   forw[0].i = inc[0];
    forw[1].x =  0;   forw[1].y = -1;   forw[1].z =  0;   forw[1].i = inc[0];
    forw[2].x = -1;   forw[2].y = -1;   forw[2].z =  0;   forw[2].i = inc[1];
    forw[3].x =  1;   forw[3].y = -1;   forw[3].z =  0;   forw[3].i = inc[1];
    
    back[0].x =  1;   back[0].y =  0;   back[0].z =  0;   back[0].i = inc[0];
    back[1].x =  0;   back[1].y =  1;   back[1].z =  0;   back[1].i = inc[0];
    back[2].x = -1;   back[2].y =  1;   back[2].z =  0;   back[2].i = inc[1];
    back[3].x =  1;   back[3].y =  1;   back[3].z =  0;   back[3].i = inc[1];
    
    if ( dimz > 1 ) {
      /*
       * les increments 3D
       * forward et backward
       */
      nbIncrements = 13;
      
      forw[ 4].x =  0;   forw[ 4].y =  0;   forw[ 4].z = -1;   forw[ 4].i = inc[2];
      forw[ 5].x = -1;   forw[ 5].y =  0;   forw[ 5].z = -1;   forw[ 5].i = inc[3];
      forw[ 6].x =  1;   forw[ 6].y =  0;   forw[ 6].z = -1;   forw[ 6].i = inc[3];
      forw[ 7].x =  0;   forw[ 7].y = -1;   forw[ 7].z = -1;   forw[ 7].i = inc[3];
      forw[ 8].x =  0;   forw[ 8].y =  1;   forw[ 8].z = -1;   forw[ 8].i = inc[3];
      forw[ 9].x = -1;   forw[ 9].y = -1;   forw[ 9].z = -1;   forw[ 9].i = inc[4];
      forw[10].x = -1;   forw[10].y =  1;   forw[10].z = -1;   forw[10].i = inc[4];
      forw[11].x =  1;   forw[11].y = -1;   forw[11].z = -1;   forw[11].i = inc[4];
      forw[12].x =  1;   forw[12].y =  1;   forw[12].z = -1;   forw[12].i = inc[4];
      
      back[ 4].x =  0;   back[ 4].y =  0;   back[ 4].z =  1;   back[ 4].i = inc[2];
      back[ 5].x = -1;   back[ 5].y =  0;   back[ 5].z =  1;   back[ 5].i = inc[3];
      back[ 6].x =  1;   back[ 6].y =  0;   back[ 6].z =  1;   back[ 6].i = inc[3];
      back[ 7].x =  0;   back[ 7].y = -1;   back[ 7].z =  1;   back[ 7].i = inc[3];
      back[ 8].x =  0;   back[ 8].y =  1;   back[ 8].z =  1;   back[ 8].i = inc[3];
      back[ 9].x = -1;   back[ 9].y = -1;   back[ 9].z =  1;   back[ 9].i = inc[4];
      back[10].x = -1;   back[10].y =  1;   back[10].z =  1;   back[10].i = inc[4];
      back[11].x =  1;   back[11].y = -1;   back[11].z =  1;   back[11].i = inc[4];
      back[12].x =  1;   back[12].y =  1;   back[12].z =  1;   back[12].i = inc[4];
    }

    break;

  case 5 :
    /*
     * les increments 2D
     * forward et backward
     */
    nbIncrements = 8;
    
    forw[0].x = -1;   forw[0].y =  0;   forw[0].z =  0;   forw[0].i = inc[0];
    forw[1].x =  0;   forw[1].y = -1;   forw[1].z =  0;   forw[1].i = inc[0];
    forw[2].x = -1;   forw[2].y = -1;   forw[2].z =  0;   forw[2].i = inc[1];
    forw[3].x =  1;   forw[3].y = -1;   forw[3].z =  0;   forw[3].i = inc[1];
    forw[4].x = -2;   forw[4].y = -1;   forw[4].z =  0;   forw[4].i = inc[2];
    forw[5].x = -1;   forw[5].y = -2;   forw[5].z =  0;   forw[5].i = inc[2];
    forw[6].x =  1;   forw[6].y = -2;   forw[6].z =  0;   forw[6].i = inc[2];
    forw[7].x =  2;   forw[7].y = -1;   forw[7].z =  0;   forw[7].i = inc[2];

    back[0].x =  1;   back[0].y =  0;   back[0].z =  0;   back[0].i = inc[0];
    back[1].x =  0;   back[1].y =  1;   back[1].z =  0;   back[1].i = inc[0];
    back[2].x = -1;   back[2].y =  1;   back[2].z =  0;   back[2].i = inc[1];
    back[3].x =  1;   back[3].y =  1;   back[3].z =  0;   back[3].i = inc[1];
    back[4].x = -2;   back[4].y =  1;   back[4].z =  0;   back[4].i = inc[2];
    back[5].x = -1;   back[5].y =  2;   back[5].z =  0;   back[5].i = inc[2];
    back[6].x =  1;   back[6].y =  2;   back[6].z =  0;   back[6].i = inc[2];
    back[7].x =  2;   back[7].y =  1;   back[7].z =  0;   back[7].i = inc[2];
    
    if ( dimz > 1 ) {
      /*
       * les increments 3D
       * forward et backward
       */
      nbIncrements = 49;
      
      forw[ 8].x =  0; forw[ 8].y =  0;   forw[ 8].z = -1;   forw[ 8].i = inc[3];
      forw[ 9].x = -1; forw[ 9].y =  0;   forw[ 9].z = -1;   forw[ 9].i = inc[4];
      forw[10].x =  1; forw[10].y =  0;   forw[10].z = -1;   forw[10].i = inc[4];
      forw[11].x =  0; forw[11].y = -1;   forw[11].z = -1;   forw[11].i = inc[4];
      forw[12].x =  0; forw[12].y =  1;   forw[12].z = -1;   forw[12].i = inc[4];
      forw[13].x = -2; forw[13].y =  0;   forw[13].z = -1;   forw[13].i = inc[6];
      forw[14].x =  2; forw[14].y =  0;   forw[14].z = -1;   forw[14].i = inc[6];
      forw[15].x =  0; forw[15].y = -2;   forw[15].z = -1;   forw[15].i = inc[6];
      forw[16].x =  0; forw[16].y =  2;   forw[16].z = -1;   forw[16].i = inc[6];
      forw[17].x = -1; forw[17].y =  1;   forw[17].z = -1;   forw[17].i = inc[5];
      forw[18].x =  1; forw[18].y =  1;   forw[18].z = -1;   forw[18].i = inc[5];
      forw[19].x = -1; forw[19].y = -1;   forw[19].z = -1;   forw[19].i = inc[5];
      forw[20].x =  1; forw[20].y = -1;   forw[20].z = -1;   forw[20].i = inc[5];
      forw[21].x = -2; forw[21].y =  1;   forw[21].z = -1;   forw[21].i = inc[7];
      forw[22].x =  2; forw[22].y =  1;   forw[22].z = -1;   forw[22].i = inc[7];
      forw[23].x = -2; forw[23].y = -1;   forw[23].z = -1;   forw[23].i = inc[7];
      forw[24].x =  2; forw[24].y = -1;   forw[24].z = -1;   forw[24].i = inc[7];
      forw[25].x = -1; forw[25].y =  2;   forw[25].z = -1;   forw[25].i = inc[7];
      forw[26].x =  1; forw[26].y =  2;   forw[26].z = -1;   forw[26].i = inc[7];
      forw[27].x = -1; forw[27].y = -2;   forw[27].z = -1;   forw[27].i = inc[7];
      forw[28].x =  1; forw[28].y = -2;   forw[28].z = -1;   forw[28].i = inc[7];
      forw[29].x = -2; forw[29].y =  2;   forw[29].z = -1;   forw[29].i = inc[8];
      forw[30].x =  2; forw[30].y =  2;   forw[30].z = -1;   forw[30].i = inc[8];
      forw[31].x = -2; forw[31].y = -2;   forw[31].z = -1;   forw[31].i = inc[8];
      forw[32].x =  2; forw[32].y = -2;   forw[32].z = -1;   forw[32].i = inc[8];
      
      forw[33].x = -1; forw[33].y =  0;   forw[33].z = -2;   forw[33].i = inc[9];
      forw[34].x =  1; forw[34].y =  0;   forw[34].z = -2;   forw[34].i = inc[9];
      forw[35].x =  0; forw[35].y = -1;   forw[35].z = -2;   forw[35].i = inc[9];
      forw[36].x =  0; forw[36].y =  1;   forw[36].z = -2;   forw[36].i = inc[9];
      forw[37].x = -1; forw[37].y =  1;   forw[37].z = -2;   forw[37].i = inc[10];
      forw[38].x =  1; forw[38].y =  1;   forw[38].z = -2;   forw[38].i = inc[10];
      forw[39].x = -1; forw[39].y = -1;   forw[39].z = -2;   forw[39].i = inc[10];
      forw[40].x =  1; forw[40].y = -1;   forw[40].z = -2;   forw[40].i = inc[10];
      forw[41].x = -2; forw[41].y =  1;   forw[41].z = -2;   forw[41].i = inc[11];
      forw[42].x =  2; forw[42].y =  1;   forw[42].z = -2;   forw[42].i = inc[11];
      forw[43].x = -2; forw[43].y = -1;   forw[43].z = -2;   forw[43].i = inc[11];
      forw[44].x =  2; forw[44].y = -1;   forw[44].z = -2;   forw[44].i = inc[11];
      forw[45].x = -1; forw[45].y =  2;   forw[45].z = -2;   forw[45].i = inc[11];
      forw[46].x =  1; forw[46].y =  2;   forw[46].z = -2;   forw[46].i = inc[11];
      forw[47].x = -1; forw[47].y = -2;   forw[47].z = -2;   forw[47].i = inc[11];
      forw[48].x =  1; forw[48].y = -2;   forw[48].z = -2;   forw[48].i = inc[11];

      back[ 8].x =  0; back[ 8].y =  0;   back[ 8].z =  1;   back[ 8].i = inc[3];
      back[ 9].x = -1; back[ 9].y =  0;   back[ 9].z =  1;   back[ 9].i = inc[4];
      back[10].x =  1; back[10].y =  0;   back[10].z =  1;   back[10].i = inc[4];
      back[11].x =  0; back[11].y = -1;   back[11].z =  1;   back[11].i = inc[4];
      back[12].x =  0; back[12].y =  1;   back[12].z =  1;   back[12].i = inc[4];
      back[13].x = -2; back[13].y =  0;   back[13].z =  1;   back[13].i = inc[6];
      back[14].x =  2; back[14].y =  0;   back[14].z =  1;   back[14].i = inc[6];
      back[15].x =  0; back[15].y = -2;   back[15].z =  1;   back[15].i = inc[6];
      back[16].x =  0; back[16].y =  2;   back[16].z =  1;   back[16].i = inc[6];
      back[17].x = -1; back[17].y =  1;   back[17].z =  1;   back[17].i = inc[5];
      back[18].x =  1; back[18].y =  1;   back[18].z =  1;   back[18].i = inc[5];
      back[19].x = -1; back[19].y = -1;   back[19].z =  1;   back[19].i = inc[5];
      back[20].x =  1; back[20].y = -1;   back[20].z =  1;   back[20].i = inc[5];
      back[21].x = -2; back[21].y =  1;   back[21].z =  1;   back[21].i = inc[7];
      back[22].x =  2; back[22].y =  1;   back[22].z =  1;   back[22].i = inc[7];
      back[23].x = -2; back[23].y = -1;   back[23].z =  1;   back[23].i = inc[7];
      back[24].x =  2; back[24].y = -1;   back[24].z =  1;   back[24].i = inc[7];
      back[25].x = -1; back[25].y =  2;   back[25].z =  1;   back[25].i = inc[7];
      back[26].x =  1; back[26].y =  2;   back[26].z =  1;   back[26].i = inc[7];
      back[27].x = -1; back[27].y = -2;   back[27].z =  1;   back[27].i = inc[7];
      back[28].x =  1; back[28].y = -2;   back[28].z =  1;   back[28].i = inc[7];
      back[29].x = -2; back[29].y =  2;   back[29].z =  1;   back[29].i = inc[8];
      back[30].x =  2; back[30].y =  2;   back[30].z =  1;   back[30].i = inc[8];
      back[31].x = -2; back[31].y = -2;   back[31].z =  1;   back[31].i = inc[8];
      back[32].x =  2; back[32].y = -2;   back[32].z =  1;   back[32].i = inc[8];
      
      back[33].x = -1; back[33].y =  0;   back[33].z =  2;   back[33].i = inc[9];
      back[34].x =  1; back[34].y =  0;   back[34].z =  2;   back[34].i = inc[9];
      back[35].x =  0; back[35].y = -1;   back[35].z =  2;   back[35].i = inc[9];
      back[36].x =  0; back[36].y =  1;   back[36].z =  2;   back[36].i = inc[9];
      back[37].x = -1; back[37].y =  1;   back[37].z =  2;   back[37].i = inc[10];
      back[38].x =  1; back[38].y =  1;   back[38].z =  2;   back[38].i = inc[10];
      back[39].x = -1; back[39].y = -1;   back[39].z =  2;   back[39].i = inc[10];
      back[40].x =  1; back[40].y = -1;   back[40].z =  2;   back[40].i = inc[10];
      back[41].x = -2; back[41].y =  1;   back[41].z =  2;   back[41].i = inc[11];
      back[42].x =  2; back[42].y =  1;   back[42].z =  2;   back[42].i = inc[11];
      back[43].x = -2; back[43].y = -1;   back[43].z =  2;   back[43].i = inc[11];
      back[44].x =  2; back[44].y = -1;   back[44].z =  2;   back[44].i = inc[11];
      back[45].x = -1; back[45].y =  2;   back[45].z =  2;   back[45].i = inc[11];
      back[46].x =  1; back[46].y =  2;   back[46].z =  2;   back[46].i = inc[11];
      back[47].x = -1; back[47].y = -2;   back[47].z =  2;   back[47].i = inc[11];
      back[48].x =  1; back[48].y = -2;   back[48].z =  2;   back[48].i = inc[11];

    }
  }

  /*
   * cas 2D
   */
  if ( dimz == 1 ) {
    z = 0;
    /* 
     * boucle sur l'augmentation eventuelle du rayon
     */
    do { 

      /*
       * boucle sur le nombre de points ayant evolues
       */
      do {
	nb_changes = 0;
	/*
	 * 1er parcours : sens video
	 */
	for ( y = 0; y < dimy; y ++ )
	for ( x = 0; x < dimx; x ++ ) {
	  
	  /*
	   * distance nulle => point du fond ou point etiquette
	   */
	  if ( arrayDist[z][y][x] == 0 ) continue;
	  
	  /*
	   * on regarde si un 6-voisin a deja une etiquette
	   * un point ne pourra avoir qu'une etiquette
	   * d'un de ses 6-voisins
	   */
	  nbLabels = 0;
	  if ( y > 0 )
	    if (arrayOutput[z][y-1][x] > 0) 
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	  if ( x > 0 )
	    if (arrayOutput[z][y][x-1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	  if ( x < dimxMinusOne ) 
	    if (arrayOutput[z][y][x+1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	  if ( y < dimyMinusOne ) 
	    if (arrayOutput[z][y+1][x] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	  if ( nbLabels == 0 ) continue;
	  
	  /*
	   * on enleve les doublons
	   */
	  n = 1;
	  for ( label=1; label<nbLabels; label++  ) {
	    foundLabel = False;
	    for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	      if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
		foundLabel = True;
	    if ( foundLabel == False )
	      labelsAdmissibles[n++] = labelsAdmissibles[label];
	  }
	  nbLabels = n;
	  
	  /*
	   * On traite le point courant
	   */
	  distance_courante = arrayDist[z][y][x];
	  l = 0;
	  
	  if ( (x >= bord) && (x < dimxMinusBord) &&
	       (y >= bord) && (y < dimyMinusBord) ) {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + forw[n].x;
	      yn = y + forw[n].y;
	      /*
	       * on traite un voisin, s'il a un label non nul
	       */
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      /*
	       * si le label est celui d'un des 6-voisins
	       */
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      /*
	       * si la distance est bien inferieure a celle d'avant
	       */
	      d = arrayDist[z][ yn ][ xn ] + forw[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  } else {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + forw[n].x;
	      if ( (xn < 0) || (xn >= dimx) ) continue;
	      yn = y + forw[n].y;
	      if ( (yn < 0) || (yn >= dimy) ) continue;
	      
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      /*
	       * si le label est celui d'un des 6-voisins
	       */
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      /*
	       * si la distance est bien inferieure a celle d'avant
	       */
	      d = arrayDist[z][ yn ][ xn ] + forw[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  }
	  
	  /*
	   * a-t-on trouve un voisin interessant
	   */
	  if ( distance_courante >= arrayDist[z][y][x] ) continue;
	  if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	  arrayDist[z][y][x] = distance_courante;
	  arrayOutput[z][y][x] = l;
	  nb_changes ++;
	}

	/*
	 * 2nd parcours : sens video inverse
	 */
	for ( y = dimy-1; y >= 0; y -- )
        for ( x = dimx-1; x >= 0; x -- ) {
	

	  if ( arrayDist[z][y][x] == 0 ) continue;
	
	  nbLabels = 0;
	  if ( y > 0 )
	    if (arrayOutput[z][y-1][x] > 0) 
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	  if ( x > 0 )
	    if (arrayOutput[z][y][x-1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	  if ( x < dimxMinusOne ) 
	    if (arrayOutput[z][y][x+1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	  if ( y < dimyMinusOne ) 
	    if (arrayOutput[z][y+1][x] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	  if ( nbLabels == 0 ) continue;
	  
	  n = 1;
	  for ( label=1; label<nbLabels; label++  ) {
	    foundLabel = False;
	    for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	      if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
		foundLabel = True;
	    if ( foundLabel == False )
	      labelsAdmissibles[n++] = labelsAdmissibles[label];
	  }
	  nbLabels = n;
	  
	  distance_courante = arrayDist[z][y][x];
	  l = 0;
	  
	  if ( (x >= bord) && (x < dimxMinusBord) &&
	       (y >= bord) && (y < dimyMinusBord) ) {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + back[n].x;
	      yn = y + back[n].y;
	      
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      
	      d = arrayDist[z][ yn ][ xn ] + back[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  } else {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + back[n].x;
	      if ( (xn < 0) || (xn >= dimx) ) continue;
	      yn = y + back[n].y;
	      if ( (yn < 0) || (yn >= dimy) ) continue;
	      
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      
	      d = arrayDist[z][ yn ][ xn ] + back[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  }
	  
	  /*
	   * a-t-on trouve un voisin interessant
	   */
	  if ( distance_courante >= arrayDist[z][y][x] ) continue;
	  if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	  arrayDist[z][y][x] = distance_courante;
	  arrayOutput[z][y][x] = l;
	  nb_changes ++;
	  
	}
      
	nbIterations ++;
	if ( nb_changes > 0 ) {
	  if ( _VERBOSE_ != 0 ) {
	    fprintf( stderr, " %s: iteration #%3d, changes = %d\n", 
		     proc, nbIterations, nb_changes );
	  }
	  if ( _DILATION_RADIUS_ > 0 ) radiusHasToBeChanged = True;
	}
	
      } while ( nb_changes > 0 );
      
      if ( radiusHasToBeChanged == True ) {
	maxRadius += _DILATION_RADIUS_ * inc[0];
	radiusHasToBeChanged = False;
	if ( _VERBOSE_ != 0 ) {
	  fprintf( stderr, " %s: radius value is now %d\n", 
		   proc, maxRadius );
	}
      } else {
	processingIsDone = True;
      }
    } while ( processingIsDone == False );
    


    /* distance image normalisation
     */
    for ( z = 0; z < dimz; z ++ )
    for ( y = 0; y < dimy; y ++ )
    for ( x = 0; x < dimx; x ++ ) {
      arrayDist[z][y][x] = (int)( (double)arrayDist[z][y][x] / (double)inc[0] + 0.5 );
    }

    return( EXIT_ON_SUCCESS );
  }
  


  /*******************************
   * cas 3D
   *******************************/
  

  
  /* 
   * boucle sur l'augmentation eventuelle du rayon
   */
  do { 
    

    /*
     * boucle sur le nombre de points ayant evolues
     */
    do {
      nb_changes = 0;
      /*
       * 1er parcours : sens video
       */
      for ( z = 0; z < dimz; z ++ )
      for ( y = 0; y < dimy; y ++ )
      for ( x = 0; x < dimx; x ++ ) {
	
	/*
	 * distance nulle => point du fond ou point etiquette
	 */
	if ( arrayDist[z][y][x] == 0 ) continue;
	
	/*
	 * on regarde si un 6-voisin a deja une etiquette
	 * un point ne pourra avoir qu'une etiquette
	 * d'un de ses 6-voisins
	 */
	nbLabels = 0;
	if ( z > 0 )
	  if (arrayOutput[z-1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z-1][y][x];
	if ( y > 0 )
	  if (arrayOutput[z][y-1][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	if ( x > 0 )
	  if (arrayOutput[z][y][x-1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	if ( x < dimxMinusOne ) 
	  if (arrayOutput[z][y][x+1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	if ( y < dimyMinusOne ) 
	  if (arrayOutput[z][y+1][x] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	if ( z < dimzMinusOne ) 
	  if (arrayOutput[z+1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z+1][y][x];
	if ( nbLabels == 0 ) continue;
	
	/*
	 * on enleve les doublons
	 */
	n = 1;
	for ( label=1; label<nbLabels; label++  ) {
	  foundLabel = False;
	  for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	    if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
	      foundLabel = True;
	  if ( foundLabel == False )
	    labelsAdmissibles[n++] = labelsAdmissibles[label];
	}
	nbLabels = n;
	
	/*
	 * On traite le point courant
	 */
	distance_courante = arrayDist[z][y][x];
	l = 0;
	
	if ( (x >= bord) && (x < dimxMinusBord) &&
	     (y >= bord) && (y < dimyMinusBord) &&
	     (z >= bord) && (z < dimzMinusBord) ) {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + forw[n].x;
	    yn = y + forw[n].y;
	    zn = z + forw[n].z;
	    /*
	     * on traite un voisin, s'il a un label non nul
	     */
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	} else {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + forw[n].x;
	    if ( (xn < 0) || (xn >= dimx) ) continue;
	    yn = y + forw[n].y;
	    if ( (yn < 0) || (yn >= dimy) ) continue;
	    zn = z + forw[n].z;
	    if ( (zn < 0) || (zn >= dimz) ) continue;
	    
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	}
	
	/*
	 * a-t-on trouve un voisin interessant
	 */
	if ( distance_courante >= arrayDist[z][y][x] ) continue;
	if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	arrayDist[z][y][x] = distance_courante;
	arrayOutput[z][y][x] = l;
	nb_changes ++;
      }
      
      /*
       * 2nd parcours : sens video inverse
       */
      for ( z = dimz-1; z >= 0; z -- )
      for ( y = dimy-1; y >= 0; y -- )
      for ( x = dimx-1; x >= 0; x -- ) {
	
	if ( arrayDist[z][y][x] == 0 ) continue;
	
	nbLabels = 0;
	if ( z > 0 )
	  if (arrayOutput[z-1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z-1][y][x];
	if ( y > 0 )
	  if (arrayOutput[z][y-1][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	if ( x > 0 )
	  if (arrayOutput[z][y][x-1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	if ( x < dimxMinusOne ) 
	  if (arrayOutput[z][y][x+1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	if ( y < dimyMinusOne ) 
	  if (arrayOutput[z][y+1][x] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	if ( z < dimzMinusOne ) 
	  if (arrayOutput[z+1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z+1][y][x];
	if ( nbLabels == 0 ) continue;
	
	n = 1;
	for ( label=1; label<nbLabels; label++  ) {
	  foundLabel = False;
	  for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	  if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
	    foundLabel = True;
	  if ( foundLabel == False )
	    labelsAdmissibles[n++] = labelsAdmissibles[label];
	}
	nbLabels = n;
	
	distance_courante = arrayDist[z][y][x];
	l = 0;
	
	if ( (x >= bord) && (x < dimxMinusBord) &&
	     (y >= bord) && (y < dimyMinusBord) &&
	     (z >= bord) && (z < dimzMinusBord) ) {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + back[n].x;
	    yn = y + back[n].y;
	    zn = z + back[n].z;
	    
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	} else {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + back[n].x;
	    if ( (xn < 0) || (xn >= dimx) ) continue;
	    yn = y + back[n].y;
	    if ( (yn < 0) || (yn >= dimy) ) continue;
	    zn = z + back[n].z;
	    if ( (zn < 0) || (zn >= dimz) ) continue;
	    
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	}
	
	/*
	 * a-t-on trouve un voisin interessant
	 */
	if ( distance_courante >= arrayDist[z][y][x] ) continue;
	if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	arrayDist[z][y][x] = distance_courante;
	arrayOutput[z][y][x] = l;
	nb_changes ++;
	
      }
      
      nbIterations ++;
      if ( nb_changes > 0 ) {
	if ( _VERBOSE_ != 0 ) {
	  fprintf( stderr, " %s: iteration #%3d, changes = %d\n", 
		   proc, nbIterations, nb_changes );
	}
	if ( _DILATION_RADIUS_ > 0 ) radiusHasToBeChanged = True;
      }
    } while ( nb_changes > 0 );
    
    if ( radiusHasToBeChanged == True ) {
      maxRadius += _DILATION_RADIUS_ * inc[0];
      radiusHasToBeChanged = False;
      if ( _VERBOSE_ != 0 ) {
	fprintf( stderr, " %s: radius value is now %d\n", 
		 proc, maxRadius );
      }
    } else {
      processingIsDone = True;
    }
  } while ( processingIsDone == False );



  

  /* distance image normalisation
   */
  for ( z = 0; z < dimz; z ++ )
  for ( y = 0; y < dimy; y ++ )
  for ( x = 0; x < dimx; x ++ ) {
    arrayDist[z][y][x] = (int)( (double)arrayDist[z][y][x] / (double)inc[0] + 0.5 );
  }
  
  
  return( EXIT_ON_SUCCESS );
  
}











static int VT_ConstrainedSkizInDistanceMapWithChamferInUShort( vt_image *imLabels, 
							       vt_image *imMask,
							       vt_image *imDist,
							       vt_image *imOutput,
							       int chamferSize,
							       int *increments )
{
  char *proc="VT_ConstrainedSkizInDistanceMapWithChamferInUShort";
  int dimx, dimy, dimz;
  int bord;
  int dimxMinusOne, dimyMinusOne, dimzMinusOne;
  int dimxMinusBord, dimyMinusBord, dimzMinusBord;
  register int x, y, z;
  register int xn, yn, zn;
  u16 ***arrayLabels;
  u8 ***arrayMask;
  u16 ***arrayOutput;
  int distance_infinie;
  u16 ***arrayDist;
  register int distance_courante, d, l;
  register int nb_changes;

  int inc[12];

  /* 
   * 29 est un maximum pour une distance de chamfer 5x5x5
   *
   *  . c . c .   f e c e f   . f . f .
   *  c b a b c   e d b d e   f e c e f
   *  . a X       c b a       . c .
   *     z=0         z=1         z=2
   */
  typeIncrement forw[49], back[49]; 
  int n, nbIncrements=0;
  int nbIterations = 0;
  /*
   * pour gerer le rayon
   */
  int maxRadius = _DILATION_RADIUS_ * inc[0];
  typeBoolean radiusHasToBeChanged = False;
  typeBoolean processingIsDone = False;
  /*
   * pour verifier qu'un 6-voisin a le meme label
   */ 
  int labelsAdmissibles[6];
  int nbLabels;
  typeBoolean foundLabel;
  register int label;

  /*
   * Tests
   */
  if ( (imLabels->dim.x != imMask->dim.x) || (imLabels->dim.x != imOutput->dim.x) ||
       (imLabels->dim.y != imMask->dim.y) || (imLabels->dim.y != imOutput->dim.y) ||
       (imLabels->dim.z != imMask->dim.z) || (imLabels->dim.z != imOutput->dim.z) ||
       (imLabels->dim.x != imDist->dim.x) ||
       (imLabels->dim.y != imDist->dim.y) ||
       (imLabels->dim.z != imDist->dim.z) ) {
    if ( _VERBOSE_ != 0 ) {
	fprintf( stderr, "%s: images have different dimensions.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }
  if ( (imLabels->type != USHORT) || (imMask->type != UCHAR) ||
       (imOutput->type != USHORT) ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, "%s: images should be of unsigned short type.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }
  if ( imDist->type != USHORT ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, "%s: distance image should be of unsigned short int type.\n", proc );
    }
    return( EXIT_ON_FAILURE );
  }




  /*
   * valeur max autorisee 
   * c'est l'infini pour le calcul
   * cela depend du codage utilise
   */
  distance_infinie = 65535;

  /*
   * initialisation des images 
   */
  arrayLabels = (u16 ***)(imLabels->array);
  arrayMask = (u8 ***)(imMask->array);
  arrayOutput = (u16 ***)(imOutput->array);
  arrayDist = (u16 ***)(imDist->array);
  dimx = imLabels->dim.x;
  dimy = imLabels->dim.y;
  dimz = imLabels->dim.z;
  
  /*
   * Les etiquettes finales sont deja les etiquettes
   * initiales, sauf en dehors du mask : c'est 0.
   * La distance est a l'infini, sauf en dehors du mask
   * et sur les etiquettes initiales ou elle est a 0.
   * Ainsi on ne remettra a jour que les points dans le 
   * masque qui ne sont pas des etiquettes initiales.
   */
  for ( z = 0; z < dimz; z ++ )
  for ( y = 0; y < dimy; y ++ )
  for ( x = 0; x < dimx; x ++ ) {
    arrayOutput[z][y][x] = arrayLabels[z][y][x];
    arrayDist[z][y][x] = distance_infinie;
    if ( arrayLabels[z][y][x] > 0 ) arrayDist[z][y][x] = 0;
    if ( arrayMask[z][y][x] == 0 ) {
      arrayDist[z][y][x] = 0;
      arrayOutput[z][y][x] = 0;
    }
  }

  /*
   * bordure
   */
  switch ( chamferSize ) {
  case 3 :
  default :
    bord = 1;
    break;
  case 5 :
    bord = 2;
  }
  dimxMinusBord = dimx - bord;
  dimyMinusBord = dimy - bord;
  dimzMinusBord = dimz - bord;
  dimxMinusOne = dimx - 1;
  dimyMinusOne = dimy - 1;
  dimzMinusOne = dimz - 1;


  if ( increments == NULL ) {
    switch ( chamferSize ) {
    case 3 :
    default :
      inc[0] = 16; /* les 4-voisins dans le plan (dx=1 ou dy=1) */
      inc[1] = 21; /* les 4-voisins dans le plan (dx=1 et dy=1) */
      
      inc[2] = 16; /* dz=1 et dx=0 et dy=0 */
      inc[3] = 21; /* dz=1 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[4] = 26; /* dz=1 et (dx=1 et dy=1) */
      break;
    case 5 :
      inc[0] = 13; /* dz=0 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[1] = 18; /* dz=0 et (dx=1 et dy=1) */
      inc[2] = 29; /* dz=0 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */

      inc[3] = 13; /* dz=1 et dx=0 et dy=0 */
      inc[4] = 18; /* dz=1 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[5] = 23; /* dz=1 et (dx=1 et dy=1) */
      inc[6] = 29; /* dz=1 et ( (dx=0 et dy=2) ou (dx=2 et dy=0) ) */
      inc[7] = 32; /* dz=1 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */
      inc[8] = 39; /* dz=1 et (dx=2 et dy=2) */

      inc[9] = 29; /* dz=2 et ( (dx=0 et dy=1) ou (dx=1 et dy=0) ) */
      inc[10] = 32; /* dz=2 et (dx=1 et dy=1) */
      inc[11] = 39; /* dz=2 et ( (dx=1 et dy=2) ou (dx=2 et dy=1) ) */
    }
  } else {
    (void)memcpy( inc, increments, 12*sizeof(int) );
  }


  switch ( chamferSize ) {
  case 3 :
  default :
    /*
     * les increments 2D
     * forward et backward
     */
    nbIncrements = 4;
    
    forw[0].x = -1;   forw[0].y =  0;   forw[0].z =  0;   forw[0].i = inc[0];
    forw[1].x =  0;   forw[1].y = -1;   forw[1].z =  0;   forw[1].i = inc[0];
    forw[2].x = -1;   forw[2].y = -1;   forw[2].z =  0;   forw[2].i = inc[1];
    forw[3].x =  1;   forw[3].y = -1;   forw[3].z =  0;   forw[3].i = inc[1];
    
    back[0].x =  1;   back[0].y =  0;   back[0].z =  0;   back[0].i = inc[0];
    back[1].x =  0;   back[1].y =  1;   back[1].z =  0;   back[1].i = inc[0];
    back[2].x = -1;   back[2].y =  1;   back[2].z =  0;   back[2].i = inc[1];
    back[3].x =  1;   back[3].y =  1;   back[3].z =  0;   back[3].i = inc[1];
    
    if ( dimz > 1 ) {
      /*
       * les increments 3D
       * forward et backward
       */
      nbIncrements = 13;
      
      forw[ 4].x =  0;   forw[ 4].y =  0;   forw[ 4].z = -1;   forw[ 4].i = inc[2];
      forw[ 5].x = -1;   forw[ 5].y =  0;   forw[ 5].z = -1;   forw[ 5].i = inc[3];
      forw[ 6].x =  1;   forw[ 6].y =  0;   forw[ 6].z = -1;   forw[ 6].i = inc[3];
      forw[ 7].x =  0;   forw[ 7].y = -1;   forw[ 7].z = -1;   forw[ 7].i = inc[3];
      forw[ 8].x =  0;   forw[ 8].y =  1;   forw[ 8].z = -1;   forw[ 8].i = inc[3];
      forw[ 9].x = -1;   forw[ 9].y = -1;   forw[ 9].z = -1;   forw[ 9].i = inc[4];
      forw[10].x = -1;   forw[10].y =  1;   forw[10].z = -1;   forw[10].i = inc[4];
      forw[11].x =  1;   forw[11].y = -1;   forw[11].z = -1;   forw[11].i = inc[4];
      forw[12].x =  1;   forw[12].y =  1;   forw[12].z = -1;   forw[12].i = inc[4];
      
      back[ 4].x =  0;   back[ 4].y =  0;   back[ 4].z =  1;   back[ 4].i = inc[2];
      back[ 5].x = -1;   back[ 5].y =  0;   back[ 5].z =  1;   back[ 5].i = inc[3];
      back[ 6].x =  1;   back[ 6].y =  0;   back[ 6].z =  1;   back[ 6].i = inc[3];
      back[ 7].x =  0;   back[ 7].y = -1;   back[ 7].z =  1;   back[ 7].i = inc[3];
      back[ 8].x =  0;   back[ 8].y =  1;   back[ 8].z =  1;   back[ 8].i = inc[3];
      back[ 9].x = -1;   back[ 9].y = -1;   back[ 9].z =  1;   back[ 9].i = inc[4];
      back[10].x = -1;   back[10].y =  1;   back[10].z =  1;   back[10].i = inc[4];
      back[11].x =  1;   back[11].y = -1;   back[11].z =  1;   back[11].i = inc[4];
      back[12].x =  1;   back[12].y =  1;   back[12].z =  1;   back[12].i = inc[4];
    }

    break;

  case 5 :
    /*
     * les increments 2D
     * forward et backward
     */
    nbIncrements = 8;
    
    forw[0].x = -1;   forw[0].y =  0;   forw[0].z =  0;   forw[0].i = inc[0];
    forw[1].x =  0;   forw[1].y = -1;   forw[1].z =  0;   forw[1].i = inc[0];
    forw[2].x = -1;   forw[2].y = -1;   forw[2].z =  0;   forw[2].i = inc[1];
    forw[3].x =  1;   forw[3].y = -1;   forw[3].z =  0;   forw[3].i = inc[1];
    forw[4].x = -2;   forw[4].y = -1;   forw[4].z =  0;   forw[4].i = inc[2];
    forw[5].x = -1;   forw[5].y = -2;   forw[5].z =  0;   forw[5].i = inc[2];
    forw[6].x =  1;   forw[6].y = -2;   forw[6].z =  0;   forw[6].i = inc[2];
    forw[7].x =  2;   forw[7].y = -1;   forw[7].z =  0;   forw[7].i = inc[2];

    back[0].x =  1;   back[0].y =  0;   back[0].z =  0;   back[0].i = inc[0];
    back[1].x =  0;   back[1].y =  1;   back[1].z =  0;   back[1].i = inc[0];
    back[2].x = -1;   back[2].y =  1;   back[2].z =  0;   back[2].i = inc[1];
    back[3].x =  1;   back[3].y =  1;   back[3].z =  0;   back[3].i = inc[1];
    back[4].x = -2;   back[4].y =  1;   back[4].z =  0;   back[4].i = inc[2];
    back[5].x = -1;   back[5].y =  2;   back[5].z =  0;   back[5].i = inc[2];
    back[6].x =  1;   back[6].y =  2;   back[6].z =  0;   back[6].i = inc[2];
    back[7].x =  2;   back[7].y =  1;   back[7].z =  0;   back[7].i = inc[2];
    
    if ( dimz > 1 ) {
      /*
       * les increments 3D
       * forward et backward
       */
      nbIncrements = 49;
      
      forw[ 8].x =  0; forw[ 8].y =  0;   forw[ 8].z = -1;   forw[ 8].i = inc[3];
      forw[ 9].x = -1; forw[ 9].y =  0;   forw[ 9].z = -1;   forw[ 9].i = inc[4];
      forw[10].x =  1; forw[10].y =  0;   forw[10].z = -1;   forw[10].i = inc[4];
      forw[11].x =  0; forw[11].y = -1;   forw[11].z = -1;   forw[11].i = inc[4];
      forw[12].x =  0; forw[12].y =  1;   forw[12].z = -1;   forw[12].i = inc[4];
      forw[13].x = -2; forw[13].y =  0;   forw[13].z = -1;   forw[13].i = inc[6];
      forw[14].x =  2; forw[14].y =  0;   forw[14].z = -1;   forw[14].i = inc[6];
      forw[15].x =  0; forw[15].y = -2;   forw[15].z = -1;   forw[15].i = inc[6];
      forw[16].x =  0; forw[16].y =  2;   forw[16].z = -1;   forw[16].i = inc[6];
      forw[17].x = -1; forw[17].y =  1;   forw[17].z = -1;   forw[17].i = inc[5];
      forw[18].x =  1; forw[18].y =  1;   forw[18].z = -1;   forw[18].i = inc[5];
      forw[19].x = -1; forw[19].y = -1;   forw[19].z = -1;   forw[19].i = inc[5];
      forw[20].x =  1; forw[20].y = -1;   forw[20].z = -1;   forw[20].i = inc[5];
      forw[21].x = -2; forw[21].y =  1;   forw[21].z = -1;   forw[21].i = inc[7];
      forw[22].x =  2; forw[22].y =  1;   forw[22].z = -1;   forw[22].i = inc[7];
      forw[23].x = -2; forw[23].y = -1;   forw[23].z = -1;   forw[23].i = inc[7];
      forw[24].x =  2; forw[24].y = -1;   forw[24].z = -1;   forw[24].i = inc[7];
      forw[25].x = -1; forw[25].y =  2;   forw[25].z = -1;   forw[25].i = inc[7];
      forw[26].x =  1; forw[26].y =  2;   forw[26].z = -1;   forw[26].i = inc[7];
      forw[27].x = -1; forw[27].y = -2;   forw[27].z = -1;   forw[27].i = inc[7];
      forw[28].x =  1; forw[28].y = -2;   forw[28].z = -1;   forw[28].i = inc[7];
      forw[29].x = -2; forw[29].y =  2;   forw[29].z = -1;   forw[29].i = inc[8];
      forw[30].x =  2; forw[30].y =  2;   forw[30].z = -1;   forw[30].i = inc[8];
      forw[31].x = -2; forw[31].y = -2;   forw[31].z = -1;   forw[31].i = inc[8];
      forw[32].x =  2; forw[32].y = -2;   forw[32].z = -1;   forw[32].i = inc[8];
      
      forw[33].x = -1; forw[33].y =  0;   forw[33].z = -2;   forw[33].i = inc[9];
      forw[34].x =  1; forw[34].y =  0;   forw[34].z = -2;   forw[34].i = inc[9];
      forw[35].x =  0; forw[35].y = -1;   forw[35].z = -2;   forw[35].i = inc[9];
      forw[36].x =  0; forw[36].y =  1;   forw[36].z = -2;   forw[36].i = inc[9];
      forw[37].x = -1; forw[37].y =  1;   forw[37].z = -2;   forw[37].i = inc[10];
      forw[38].x =  1; forw[38].y =  1;   forw[38].z = -2;   forw[38].i = inc[10];
      forw[39].x = -1; forw[39].y = -1;   forw[39].z = -2;   forw[39].i = inc[10];
      forw[40].x =  1; forw[40].y = -1;   forw[40].z = -2;   forw[40].i = inc[10];
      forw[41].x = -2; forw[41].y =  1;   forw[41].z = -2;   forw[41].i = inc[11];
      forw[42].x =  2; forw[42].y =  1;   forw[42].z = -2;   forw[42].i = inc[11];
      forw[43].x = -2; forw[43].y = -1;   forw[43].z = -2;   forw[43].i = inc[11];
      forw[44].x =  2; forw[44].y = -1;   forw[44].z = -2;   forw[44].i = inc[11];
      forw[45].x = -1; forw[45].y =  2;   forw[45].z = -2;   forw[45].i = inc[11];
      forw[46].x =  1; forw[46].y =  2;   forw[46].z = -2;   forw[46].i = inc[11];
      forw[47].x = -1; forw[47].y = -2;   forw[47].z = -2;   forw[47].i = inc[11];
      forw[48].x =  1; forw[48].y = -2;   forw[48].z = -2;   forw[48].i = inc[11];

      back[ 8].x =  0; back[ 8].y =  0;   back[ 8].z =  1;   back[ 8].i = inc[3];
      back[ 9].x = -1; back[ 9].y =  0;   back[ 9].z =  1;   back[ 9].i = inc[4];
      back[10].x =  1; back[10].y =  0;   back[10].z =  1;   back[10].i = inc[4];
      back[11].x =  0; back[11].y = -1;   back[11].z =  1;   back[11].i = inc[4];
      back[12].x =  0; back[12].y =  1;   back[12].z =  1;   back[12].i = inc[4];
      back[13].x = -2; back[13].y =  0;   back[13].z =  1;   back[13].i = inc[6];
      back[14].x =  2; back[14].y =  0;   back[14].z =  1;   back[14].i = inc[6];
      back[15].x =  0; back[15].y = -2;   back[15].z =  1;   back[15].i = inc[6];
      back[16].x =  0; back[16].y =  2;   back[16].z =  1;   back[16].i = inc[6];
      back[17].x = -1; back[17].y =  1;   back[17].z =  1;   back[17].i = inc[5];
      back[18].x =  1; back[18].y =  1;   back[18].z =  1;   back[18].i = inc[5];
      back[19].x = -1; back[19].y = -1;   back[19].z =  1;   back[19].i = inc[5];
      back[20].x =  1; back[20].y = -1;   back[20].z =  1;   back[20].i = inc[5];
      back[21].x = -2; back[21].y =  1;   back[21].z =  1;   back[21].i = inc[7];
      back[22].x =  2; back[22].y =  1;   back[22].z =  1;   back[22].i = inc[7];
      back[23].x = -2; back[23].y = -1;   back[23].z =  1;   back[23].i = inc[7];
      back[24].x =  2; back[24].y = -1;   back[24].z =  1;   back[24].i = inc[7];
      back[25].x = -1; back[25].y =  2;   back[25].z =  1;   back[25].i = inc[7];
      back[26].x =  1; back[26].y =  2;   back[26].z =  1;   back[26].i = inc[7];
      back[27].x = -1; back[27].y = -2;   back[27].z =  1;   back[27].i = inc[7];
      back[28].x =  1; back[28].y = -2;   back[28].z =  1;   back[28].i = inc[7];
      back[29].x = -2; back[29].y =  2;   back[29].z =  1;   back[29].i = inc[8];
      back[30].x =  2; back[30].y =  2;   back[30].z =  1;   back[30].i = inc[8];
      back[31].x = -2; back[31].y = -2;   back[31].z =  1;   back[31].i = inc[8];
      back[32].x =  2; back[32].y = -2;   back[32].z =  1;   back[32].i = inc[8];
      
      back[33].x = -1; back[33].y =  0;   back[33].z =  2;   back[33].i = inc[9];
      back[34].x =  1; back[34].y =  0;   back[34].z =  2;   back[34].i = inc[9];
      back[35].x =  0; back[35].y = -1;   back[35].z =  2;   back[35].i = inc[9];
      back[36].x =  0; back[36].y =  1;   back[36].z =  2;   back[36].i = inc[9];
      back[37].x = -1; back[37].y =  1;   back[37].z =  2;   back[37].i = inc[10];
      back[38].x =  1; back[38].y =  1;   back[38].z =  2;   back[38].i = inc[10];
      back[39].x = -1; back[39].y = -1;   back[39].z =  2;   back[39].i = inc[10];
      back[40].x =  1; back[40].y = -1;   back[40].z =  2;   back[40].i = inc[10];
      back[41].x = -2; back[41].y =  1;   back[41].z =  2;   back[41].i = inc[11];
      back[42].x =  2; back[42].y =  1;   back[42].z =  2;   back[42].i = inc[11];
      back[43].x = -2; back[43].y = -1;   back[43].z =  2;   back[43].i = inc[11];
      back[44].x =  2; back[44].y = -1;   back[44].z =  2;   back[44].i = inc[11];
      back[45].x = -1; back[45].y =  2;   back[45].z =  2;   back[45].i = inc[11];
      back[46].x =  1; back[46].y =  2;   back[46].z =  2;   back[46].i = inc[11];
      back[47].x = -1; back[47].y = -2;   back[47].z =  2;   back[47].i = inc[11];
      back[48].x =  1; back[48].y = -2;   back[48].z =  2;   back[48].i = inc[11];

    }
  }

  /*
   * cas 2D
   */
  if ( dimz == 1 ) {
    z = 0;
    /* 
     * boucle sur l'augmentation eventuelle du rayon
     */
    do { 

      /*
       * boucle sur le nombre de points ayant evolues
       */
      do {
	nb_changes = 0;
	/*
	 * 1er parcours : sens video
	 */
	for ( y = 0; y < dimy; y ++ )
	for ( x = 0; x < dimx; x ++ ) {
	  
	  /*
	   * distance nulle => point du fond ou point etiquette
	   */
	  if ( arrayDist[z][y][x] == 0 ) continue;
	  
	  /*
	   * on regarde si un 6-voisin a deja une etiquette
	   * un point ne pourra avoir qu'une etiquette
	   * d'un de ses 6-voisins
	   */
	  nbLabels = 0;
	  if ( y > 0 )
	    if (arrayOutput[z][y-1][x] > 0) 
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	  if ( x > 0 )
	    if (arrayOutput[z][y][x-1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	  if ( x < dimxMinusOne ) 
	    if (arrayOutput[z][y][x+1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	  if ( y < dimyMinusOne ) 
	    if (arrayOutput[z][y+1][x] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	  if ( nbLabels == 0 ) continue;
	  
	  /*
	   * on enleve les doublons
	   */
	  n = 1;
	  for ( label=1; label<nbLabels; label++  ) {
	    foundLabel = False;
	    for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	      if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
		foundLabel = True;
	    if ( foundLabel == False )
	      labelsAdmissibles[n++] = labelsAdmissibles[label];
	  }
	  nbLabels = n;
	  
	  /*
	   * On traite le point courant
	   */
	  distance_courante = arrayDist[z][y][x];
	  l = 0;
	  
	  if ( (x >= bord) && (x < dimxMinusBord) &&
	       (y >= bord) && (y < dimyMinusBord) ) {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + forw[n].x;
	      yn = y + forw[n].y;
	      /*
	       * on traite un voisin, s'il a un label non nul
	       */
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      /*
	       * si le label est celui d'un des 6-voisins
	       */
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      /*
	       * si la distance est bien inferieure a celle d'avant
	       */
	      d = arrayDist[z][ yn ][ xn ] + forw[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  } else {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + forw[n].x;
	      if ( (xn < 0) || (xn >= dimx) ) continue;
	      yn = y + forw[n].y;
	      if ( (yn < 0) || (yn >= dimy) ) continue;
	      
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      /*
	       * si le label est celui d'un des 6-voisins
	       */
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      /*
	       * si la distance est bien inferieure a celle d'avant
	       */
	      d = arrayDist[z][ yn ][ xn ] + forw[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  }
	  
	  /*
	   * a-t-on trouve un voisin interessant
	   */
	  if ( distance_courante >= arrayDist[z][y][x] ) continue;
	  if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	  arrayDist[z][y][x] = distance_courante;
	  arrayOutput[z][y][x] = l;
	  nb_changes ++;
	}

	/*
	 * 2nd parcours : sens video inverse
	 */
	for ( y = dimy-1; y >= 0; y -- )
        for ( x = dimx-1; x >= 0; x -- ) {
	

	  if ( arrayDist[z][y][x] == 0 ) continue;
	
	  nbLabels = 0;
	  if ( y > 0 )
	    if (arrayOutput[z][y-1][x] > 0) 
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	  if ( x > 0 )
	    if (arrayOutput[z][y][x-1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	  if ( x < dimxMinusOne ) 
	    if (arrayOutput[z][y][x+1] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	  if ( y < dimyMinusOne ) 
	    if (arrayOutput[z][y+1][x] > 0)
	      labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	  if ( nbLabels == 0 ) continue;
	  
	  n = 1;
	  for ( label=1; label<nbLabels; label++  ) {
	    foundLabel = False;
	    for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	      if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
		foundLabel = True;
	    if ( foundLabel == False )
	      labelsAdmissibles[n++] = labelsAdmissibles[label];
	  }
	  nbLabels = n;
	  
	  distance_courante = arrayDist[z][y][x];
	  l = 0;
	  
	  if ( (x >= bord) && (x < dimxMinusBord) &&
	       (y >= bord) && (y < dimyMinusBord) ) {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + back[n].x;
	      yn = y + back[n].y;
	      
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      
	      d = arrayDist[z][ yn ][ xn ] + back[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  } else {
	    
	    for ( n=0; n<nbIncrements; n++ ) {
	      xn = x + back[n].x;
	      if ( (xn < 0) || (xn >= dimx) ) continue;
	      yn = y + back[n].y;
	      if ( (yn < 0) || (yn >= dimy) ) continue;
	      
	      if ( arrayOutput[z][ yn ][ xn ] == 0 ) continue;
	      
	      foundLabel = False;
	      for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
		if ( arrayOutput[z][ yn ][ xn ] == labelsAdmissibles[label] )
		  foundLabel = True;
	      if ( foundLabel == False ) continue;
	      
	      d = arrayDist[z][ yn ][ xn ] + back[n].i;
	      if ( d >= distance_courante ) continue;
	      l = arrayOutput[z][ yn ][ xn ];
	      distance_courante = d;
	    }
	    
	  }
	  
	  /*
	   * a-t-on trouve un voisin interessant
	   */
	  if ( distance_courante >= arrayDist[z][y][x] ) continue;
	  if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	  arrayDist[z][y][x] = distance_courante;
	  arrayOutput[z][y][x] = l;
	  nb_changes ++;
	  
	}
      
	nbIterations ++;
	if ( nb_changes > 0 ) {
	  if ( _VERBOSE_ != 0 ) {
	    fprintf( stderr, " %s: iteration #%3d, changes = %d\n", 
		     proc, nbIterations, nb_changes );
	  }
	  if ( _DILATION_RADIUS_ > 0 ) radiusHasToBeChanged = True;
	}
	
      } while ( nb_changes > 0 );
      
      if ( radiusHasToBeChanged == True ) {
	maxRadius += _DILATION_RADIUS_ * inc[0];
	radiusHasToBeChanged = False;
	if ( _VERBOSE_ != 0 ) {
	  fprintf( stderr, " %s: radius value is now %d\n", 
		   proc, maxRadius );
	}
      } else {
	processingIsDone = True;
      }
    } while ( processingIsDone == False );
    


    
    /* distance image normalisation
     */
    for ( z = 0; z < dimz; z ++ )
    for ( y = 0; y < dimy; y ++ )
    for ( x = 0; x < dimx; x ++ ) {
      arrayDist[z][y][x] = (int)( (double)arrayDist[z][y][x] / (double)inc[0] + 0.5 );
    }

    return( EXIT_ON_SUCCESS );
  }



  /*******************************
   * cas 3D
   *******************************/
  

  
  /* 
   * boucle sur l'augmentation eventuelle du rayon
   */
  do { 
    

    /*
     * boucle sur le nombre de points ayant evolues
     */
    do {
      nb_changes = 0;
      /*
       * 1er parcours : sens video
       */
      for ( z = 0; z < dimz; z ++ )
      for ( y = 0; y < dimy; y ++ )
      for ( x = 0; x < dimx; x ++ ) {
	
	/*
	 * distance nulle => point du fond ou point etiquette
	 */
	if ( arrayDist[z][y][x] == 0 ) continue;
	
	/*
	 * on regarde si un 6-voisin a deja une etiquette
	 * un point ne pourra avoir qu'une etiquette
	 * d'un de ses 6-voisins
	 */
	nbLabels = 0;
	if ( z > 0 )
	  if (arrayOutput[z-1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z-1][y][x];
	if ( y > 0 )
	  if (arrayOutput[z][y-1][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	if ( x > 0 )
	  if (arrayOutput[z][y][x-1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	if ( x < dimxMinusOne ) 
	  if (arrayOutput[z][y][x+1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	if ( y < dimyMinusOne ) 
	  if (arrayOutput[z][y+1][x] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	if ( z < dimzMinusOne ) 
	  if (arrayOutput[z+1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z+1][y][x];
	if ( nbLabels == 0 ) continue;
	
	/*
	 * on enleve les doublons
	 */
	n = 1;
	for ( label=1; label<nbLabels; label++  ) {
	  foundLabel = False;
	  for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	    if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
	      foundLabel = True;
	  if ( foundLabel == False )
	    labelsAdmissibles[n++] = labelsAdmissibles[label];
	}
	nbLabels = n;
	
	/*
	 * On traite le point courant
	 */
	distance_courante = arrayDist[z][y][x];
	l = 0;
	
	if ( (x >= bord) && (x < dimxMinusBord) &&
	     (y >= bord) && (y < dimyMinusBord) &&
	     (z >= bord) && (z < dimzMinusBord) ) {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + forw[n].x;
	    yn = y + forw[n].y;
	    zn = z + forw[n].z;
	    /*
	     * on traite un voisin, s'il a un label non nul
	     */
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	} else {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + forw[n].x;
	    if ( (xn < 0) || (xn >= dimx) ) continue;
	    yn = y + forw[n].y;
	    if ( (yn < 0) || (yn >= dimy) ) continue;
	    zn = z + forw[n].z;
	    if ( (zn < 0) || (zn >= dimz) ) continue;
	    
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	}
	
	/*
	 * a-t-on trouve un voisin interessant
	 */
	if ( distance_courante >= arrayDist[z][y][x] ) continue;
	if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	arrayDist[z][y][x] = distance_courante;
	arrayOutput[z][y][x] = l;
	nb_changes ++;
      }
      
      /*
       * 2nd parcours : sens video inverse
       */
      for ( z = dimz-1; z >= 0; z -- )
      for ( y = dimy-1; y >= 0; y -- )
      for ( x = dimx-1; x >= 0; x -- ) {
	
	if ( arrayDist[z][y][x] == 0 ) continue;
	
	nbLabels = 0;
	if ( z > 0 )
	  if (arrayOutput[z-1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z-1][y][x];
	if ( y > 0 )
	  if (arrayOutput[z][y-1][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y-1][x];
	if ( x > 0 )
	  if (arrayOutput[z][y][x-1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x-1];
	if ( x < dimxMinusOne ) 
	  if (arrayOutput[z][y][x+1] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y][x+1];
	if ( y < dimyMinusOne ) 
	  if (arrayOutput[z][y+1][x] > 0)
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z][y+1][x];
	if ( z < dimzMinusOne ) 
	  if (arrayOutput[z+1][y][x] > 0) 
	    labelsAdmissibles[ nbLabels++ ] = arrayOutput[z+1][y][x];
	if ( nbLabels == 0 ) continue;
	
	n = 1;
	for ( label=1; label<nbLabels; label++  ) {
	  foundLabel = False;
	  for ( l = 0; (l<n) && (foundLabel==False); l ++ )
	  if ( labelsAdmissibles[label] == labelsAdmissibles[l] )
	    foundLabel = True;
	  if ( foundLabel == False )
	    labelsAdmissibles[n++] = labelsAdmissibles[label];
	}
	nbLabels = n;
	
	distance_courante = arrayDist[z][y][x];
	l = 0;
	
	if ( (x >= bord) && (x < dimxMinusBord) &&
	     (y >= bord) && (y < dimyMinusBord) &&
	     (z >= bord) && (z < dimzMinusBord) ) {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + back[n].x;
	    yn = y + back[n].y;
	    zn = z + back[n].z;
	    
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	} else {
	  
	  for ( n=0; n<nbIncrements; n++ ) {
	    xn = x + back[n].x;
	    if ( (xn < 0) || (xn >= dimx) ) continue;
	    yn = y + back[n].y;
	    if ( (yn < 0) || (yn >= dimy) ) continue;
	    zn = z + back[n].z;
	    if ( (zn < 0) || (zn >= dimz) ) continue;
	    
	    if ( arrayOutput[ zn ][ yn ][ xn ] == 0 ) continue;
	    
	    foundLabel = False;
	    for ( label=0; (label<nbLabels) && (foundLabel==False); label++  )
	      if ( arrayOutput[ zn ][ yn ][ xn ] == labelsAdmissibles[label] )
		foundLabel = True;
	    if ( foundLabel == False ) continue;
	    
	    d = arrayDist[ zn ][ yn ][ xn ] + forw[n].i;
	    if ( d >= distance_courante ) continue;
	    l = arrayOutput[ zn ][ yn ][ xn ];
	    distance_courante = d;
	  }
	  
	}
	
	/*
	 * a-t-on trouve un voisin interessant
	 */
	if ( distance_courante >= arrayDist[z][y][x] ) continue;
	if ( (_DILATION_RADIUS_ > 0) && ( distance_courante > maxRadius ) ) continue;
	arrayDist[z][y][x] = distance_courante;
	arrayOutput[z][y][x] = l;
	nb_changes ++;
	
      }
      
      nbIterations ++;
      if ( nb_changes > 0 ) {
	if ( _VERBOSE_ != 0 ) {
	  fprintf( stderr, " %s: iteration #%3d, changes = %d\n", 
		   proc, nbIterations, nb_changes );
	}
	if ( _DILATION_RADIUS_ > 0 ) radiusHasToBeChanged = True;
      }
    } while ( nb_changes > 0 );
    
    if ( radiusHasToBeChanged == True ) {
      maxRadius += _DILATION_RADIUS_ * inc[0];
      radiusHasToBeChanged = False;
      if ( _VERBOSE_ != 0 ) {
	fprintf( stderr, " %s: radius value is now %d\n", 
		 proc, maxRadius );
      }
    } else {
      processingIsDone = True;
    }
  } while ( processingIsDone == False );





  /* distance image normalisation
   */
  for ( z = 0; z < dimz; z ++ )
  for ( y = 0; y < dimy; y ++ )
  for ( x = 0; x < dimx; x ++ ) {
    arrayDist[z][y][x] = (int)( (double)arrayDist[z][y][x] / (double)inc[0] + 0.5 );
  }

  return( EXIT_ON_SUCCESS );
  
}

























void VT_SetRadiusOfConstrainedSkiz( int radius ) 
{
  if ( radius < 0 ) return;
  _DILATION_RADIUS_ = radius;
}

void VT_SetSkizDistanceImageName( char *name ) 
{
  char *proc="VT_SetDistanceImageName";
  if ( name == (char*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: NULL string.", proc );
      fprintf( stderr, " Distance image will not be written.\n" );
      return;
    }
  }
  
  if ( strlen( name ) >= STRINGLENGTH ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: string too long.", proc );
      fprintf( stderr, " Distance image will not be written.\n" );
      return;
    }
  }
  if ( strncpy( distanceImageName, name, STRINGLENGTH ) == (char*)NULL ) {
    if ( _VERBOSE_ != 0 ) {
      fprintf( stderr, " %s: error while copying string.", proc );
      fprintf( stderr, " Distance image will not be written.\n" );
      return;
    }
  }
  
  writeDistanceImage = True;
}

void VT_DontWriteSkizDistanceImage( )
{
  writeDistanceImage = False;
}
