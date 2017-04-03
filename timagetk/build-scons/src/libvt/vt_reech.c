/*************************************************************************
 * vt_reech.c -
 *
 * $Id: vt_reech.c,v 1.5 2006/04/14 08:39:32 greg Exp $
 *
 * Copyright (c) INRIA 1999,2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <vt_reech.h>

static int _VERBOSE_ = 0;

/* Procedure de reechantillonnage "lineaire" 3D.

   La matrice 4x4 permet le passage des coordonnees
   de l'image resultat a l'image d'entree. On calcule
   ainsi pour chaque point de l'image resultat son
   correspondant en coordonnees reelles dans l'image 
   d'entree. On interpole alors lineairement l'intensite
   pour ce point.
*/

int Reech3DTriLin4x4( vt_image *theIm, /* input image */
                      vt_image *resIm, /* result image */
                      double *mat )
{
  char *proc="Reech3DTriLin4x4";
  int theDim[3], resDim[3];
  
  if (( theIm->dim.z == 1) && ( resIm->dim.z == 1))
    return( Reech2DTriLin4x4( theIm, resIm, mat ) );
  if ( theIm->type != resIm->type ) return( -1 );
  if ( theIm->buf == resIm->buf ) return( -1 );
  if (( theIm->dim.v != 1) || ( resIm->dim.v != 1)) return( -1 );

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;

  switch ( theIm->type ) {
    case UCHAR :
      Reech3DTriLin4x4_u8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
    case SCHAR :
      Reech3DTriLin4x4_s8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
  case USHORT :
      Reech3DTriLin4x4_u16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case SSHORT :
      Reech3DTriLin4x4_s16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case FLOAT :
      Reech3DTriLin4x4_r32( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}

int Reech3DTriLin4x4gb( vt_image *theIm, /* input image */
                        vt_image *resIm, /* result image */
                        double *mat,
                        float gain,
                        float bias )
{
  char *proc="Reech3DTriLin4x4gb";
  int theDim[3], resDim[3];
  
  if (( theIm->dim.z == 1) && ( resIm->dim.z == 1))
    return( Reech2DTriLin4x4( theIm, resIm, mat ) );
  if ( theIm->type != resIm->type ) return( -1 );
  if ( theIm->buf == resIm->buf ) return( -1 );
  if (( theIm->dim.v != 1) || ( resIm->dim.v != 1)) return( -1 );

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;

  switch ( theIm->type ) {
    case UCHAR :
      Reech3DTriLin4x4gb_u8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
    case SCHAR :
      Reech3DTriLin4x4gb_s8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  case USHORT :
      Reech3DTriLin4x4gb_u16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  case SSHORT :
      Reech3DTriLin4x4gb_s16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  case FLOAT :
      Reech3DTriLin4x4gb_r32( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}

/* Procedure de reechantillonnage "lineaire" 3D.

   La matrice 4x4 permet le passage des coordonnees
   de l'image resultat a l'image d'entree. On calcule
   ainsi pour chaque point de l'image resultat son
   correspondant en coordonnees reelles dans l'image 
   d'entree. On prend alors l'intensite du point le
   plus proche pour ce point.
*/

int Reech3DNearest4x4( vt_image *theIm, /* input image */
                      vt_image *resIm, /* result image */
                      double *mat )
{
  char *proc="Reech3DNearest4x4";
  int theDim[3], resDim[3];
  
  if (( theIm->dim.z == 1) && ( resIm->dim.z == 1))
    return( Reech2DNearest4x4( theIm, resIm, mat ) );
  if ( theIm->type != resIm->type ) return( -1 );
  if ( theIm->buf == resIm->buf ) return( -1 );
  if (( theIm->dim.v != 1) || ( resIm->dim.v != 1)) return( -1 );

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;

  switch ( theIm->type ) {
    case UCHAR :
      Reech3DNearest4x4_u8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
    case SCHAR :
      Reech3DNearest4x4_s8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
  case USHORT :
      Reech3DNearest4x4_u16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case SSHORT :
      Reech3DNearest4x4_s16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case FLOAT :
      Reech3DNearest4x4_r32( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}

/* Procedure de reechantillonnage "lineaire" 2D.

   La matrice 4x4 permet le passage des coordonnees
   de l'image resultat a l'image d'entree. On calcule
   ainsi pour chaque point de l'image resultat son
   correspondant en coordonnees reelles dans l'image 
   d'entree. On interpole alors lineairement l'intensite
   pour ce point.

   '2D' signifie que seule la partie de la matrice
   correspondant a une transformation 2D sera prise en
   compte.
*/

int Reech2DTriLin4x4( vt_image *theIm, /* input image */
                      vt_image *resIm, /* result image */
                      double *mat )
{
  char *proc="Reech2DTriLin4x4";
  int theDim[3], resDim[3];
  if ( theIm->type != resIm->type ) return( -1 );
  if ( theIm->buf == resIm->buf ) return( -1 );
  if (( theIm->dim.v != 1) || ( resIm->dim.v != 1)) return( -1 );

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;

  switch ( theIm->type ) {
    case UCHAR :
      Reech2DTriLin4x4_u8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
    case SCHAR :
      Reech2DTriLin4x4_s8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
  case USHORT :
      Reech2DTriLin4x4_u16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case SSHORT :
      Reech2DTriLin4x4_s16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case FLOAT :
      Reech2DTriLin4x4_r32( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}

int Reech2DTriLin4x4gb( vt_image *theIm, /* input image */
                        vt_image *resIm, /* result image */
                        double *mat,
                        float gain,
                        float bias )
{
  char *proc="Reech2DTriLin4x4gb";
  int theDim[3], resDim[3];
  if ( theIm->type != resIm->type ) return( -1 );
  if ( theIm->buf == resIm->buf ) return( -1 );
  if (( theIm->dim.v != 1) || ( resIm->dim.v != 1)) return( -1 );

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;

  switch ( theIm->type ) {
    case UCHAR :
      Reech2DTriLin4x4gb_u8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
    case SCHAR :
      Reech2DTriLin4x4gb_s8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  case USHORT :
      Reech2DTriLin4x4gb_u16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  case SSHORT :
      Reech2DTriLin4x4gb_s16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  case FLOAT :
      Reech2DTriLin4x4gb_r32( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat, gain, bias );
      break;
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}

/* Procedure de reechantillonnage "lineaire" 2D.

   La matrice 4x4 permet le passage des coordonnees
   de l'image resultat a l'image d'entree. On calcule
   ainsi pour chaque point de l'image resultat son
   correspondant en coordonnees reelles dans l'image 
   d'entree. On prend alors l'intensite du point le
   plus proche pour ce point.

   '2D' signifie que seule la partie de la matrice
   correspondant a une transformation 2D sera prise en
   compte.
*/

int Reech2DNearest4x4( vt_image *theIm, /* input image */
                      vt_image *resIm, /* result image */
                      double *mat )
{
  char *proc="Reech2DNearest4x4";
  int theDim[3], resDim[3];
  if ( theIm->type != resIm->type ) return( -1 );
  if ( theIm->buf == resIm->buf ) return( -1 );
  if (( theIm->dim.v != 1) || ( resIm->dim.v != 1)) return( -1 );

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;

  switch ( theIm->type ) {
    case UCHAR :
      Reech2DNearest4x4_u8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
    case SCHAR :
      Reech2DNearest4x4_s8( (void*)(theIm->buf), theDim,
                           (void*)(resIm->buf), resDim, mat );
      break;
  case USHORT :
      Reech2DNearest4x4_u16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case SSHORT :
      Reech2DNearest4x4_s16( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  case FLOAT :
      Reech2DNearest4x4_r32( (void*)(theIm->buf), theDim,
                            (void*)(resIm->buf), resDim, mat );
      break;
  default :
    VT_Error( "images type unknown or not supported", proc );
    return( -1 );
  }
  return( 1 );
}

/* Procedure de reechantillonnage "avec deformation" 3D.

   Les images de deformation contiennent, pour chaque point M
   de l'image resultat, les coordonnees d'un vecteur V pointant
   sur le point reel correspondant dans l'image d'entree.
   En fait, cette image d'entree peut avoir ete transformee
   par une matrice Mat^{-1} avant le calcul des deformations,
   donc le "vrai" point reel correspondant a M est donne par : 
                  M' = Mat * (M + v).
   L'intensite de M' est interpolee lineairement.
*/

int Reech3DTriLinDefBack( vt_image *theIm, /* input image */
                          vt_image *resIm, /* result image */
                          vt_image **theDef, /* deformation images */
                          double *theMatAfter, /* transformation matrix */
                          double *theMatBefore, /* transformation matrix */
                          double gain,
                          double bias )
{
  char *proc="Reech3DTriLinDefBack";
  int theDim[3], resDim[3], defDim[3];
  double mat[16];
  int z;

  if ( theIm->type != resIm->type ) return( -1 );
  if ( theIm->buf == resIm->buf ) return( -1 );

  if ( (theDef[0])->type != (theDef[1])->type ) {
    fprintf( stderr, "%s: deformation field components have different types\n", proc );
    return( -1 );
  }
  if ( (theDef[0])->dim.x != (theDef[1])->dim.x ) {
    fprintf( stderr, "%s: deformation field components have different X dimensions\n", proc );
    return( -1 );
  }
  if ( (theDef[0])->dim.y != (theDef[1])->dim.y ) {
    fprintf( stderr, "%s: deformation field components have different Y dimensions\n", proc );
    return( -1 );
  }  
  if ( (theDef[0])->dim.z != (theDef[1])->dim.z ) {
    fprintf( stderr, "%s: deformation field components have different Z dimensions\n", proc );
    return( -1 );
  }
  if ( theDef[2] != NULL ) {
    if ( (theDef[0])->type != (theDef[2])->type ) {
      fprintf( stderr, "%s: deformation field components have different types\n", proc );
      return( -1 );
    }
    if ( (theDef[0])->dim.x != (theDef[2])->dim.x ) {
      fprintf( stderr, "%s: deformation field components have different X dimensions\n", proc );
      return( -1 );
    }
    if ( (theDef[0])->dim.y != (theDef[2])->dim.y ) {
      fprintf( stderr, "%s: deformation field components have different Y dimensions\n", proc );
      return( -1 );
    }  
    if ( (theDef[0])->dim.z != (theDef[2])->dim.z ) {
      fprintf( stderr, "%s: deformation field components have different Z dimensions\n", proc );
      return( -1 );
    }
  }

  
  if ( theMatBefore == NULL ) {
    if ( resIm->dim.x != (theDef[0])->dim.x ) return( -1 );
    if ( resIm->dim.y != (theDef[0])->dim.y ) return( -1 );
    if ( resIm->dim.z != (theDef[0])->dim.z ) return( -1 );
  }


  if ( theMatAfter != (double*)NULL ) {
    for ( z = 0; z < 16; z ++ ) mat[z] = theMatAfter[z];
  } else {
    for ( z = 0; z < 16; z ++ ) mat[z] = 0.0;
    mat[0] = mat[5] = mat[10] = mat[15] = 1.0;
  }

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;
  
  defDim[0] = (theDef[0])->dim.x;
  defDim[1] = (theDef[0])->dim.y;
  defDim[2] = (theDef[0])->dim.z;
  
  

  switch ( (theDef[0])->type ) {
  default :
    fprintf( stderr, "%s: such deformation field type not handled yet", proc );
    return( -1 );
    
  case FLOAT :
    {
      r32 *bufDef[3] = {NULL, NULL, NULL};
      bufDef[0] = (r32*)(theDef[0]->buf);
      bufDef[1] = (r32*)(theDef[1]->buf);
      if ( theDef[2] != NULL )
        bufDef[2] = (r32*)(theDef[2]->buf);

      switch ( theIm->type ) {
      case UCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r32_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r32_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case SCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r32_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r32_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case USHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r32_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r32_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case SSHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r32_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r32_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case FLOAT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r32_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r32_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      default :
        VT_Error( "input image type unknown or not supported", proc );
        return( -1 );
      }
    }
    break;

  case DOUBLE :
    {
      r64 *bufDef[3] = {NULL, NULL, NULL};
      bufDef[0] = (r64*)(theDef[0]->buf);
      bufDef[1] = (r64*)(theDef[1]->buf);
      if ( theDef[2] != NULL )
        bufDef[2] = (r64*)(theDef[2]->buf);

      switch ( theIm->type ) {
      case UCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r64_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r64_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case SCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r64_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r64_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case USHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r64_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r64_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case SSHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r64_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r64_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      case FLOAT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorFieldgb_r64_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
          else
            Reech2DTriLinVectorFieldgb_r64_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore, gain, bias );
        }
        break;
      default :
        VT_Error( "input image type unknown or not supported", proc );
        return( -1 );
      }
    }
    break;

  }

  return( 1 );
}





/* Procedure de reechantillonnage "avec deformation" 3D.

   Les images de deformation contiennent, pour chaque point M
   de l'image resultat, les coordonnees d'un vecteur V pointant
   sur le point reel correspondant dans l'image d'entree.
   En fait, cette image d'entree peut avoir ete transformee
   par une matrice Mat^{-1} avant le calcul des deformations,
   donc le "vrai" point reel correspondant a M est donne par : 
                  M' = Mat * (M + v).
   On prend l'intensite du plus proche voisin de M'.
*/

int Reech3DNearestDefBack( vt_image *theIm, /* input image */
                          vt_image *resIm, /* result image */
                          vt_image **theDef, /* deformation images */
                          double *theMatAfter, /* transformation matrix */
                          double *theMatBefore /* transformation matrix */ )
{
  char *proc="Reech3DNearestDefBack";
  int theDim[3], resDim[3], defDim[3];
  double mat[16];
  int z;

  if ( theIm->type != resIm->type ) return( -1 );

  if ( (theDef[0])->type != (theDef[1])->type ) {
    fprintf( stderr, "%s: deformation field components have different types\n", proc );
    return( -1 );
  }
  if ( (theDef[0])->dim.x != (theDef[1])->dim.x ) {
    fprintf( stderr, "%s: deformation field components have different X dimensions\n", proc );
    return( -1 );
  }
  if ( (theDef[0])->dim.y != (theDef[1])->dim.y ) {
    fprintf( stderr, "%s: deformation field components have different Y dimensions\n", proc );
    return( -1 );
  }  
  if ( (theDef[0])->dim.z != (theDef[1])->dim.z ) {
    fprintf( stderr, "%s: deformation field components have different Z dimensions\n", proc );
    return( -1 );
  }
  if ( theDef[2] != NULL ) {
    if ( (theDef[0])->type != (theDef[2])->type ) {
      fprintf( stderr, "%s: deformation field components have different types\n", proc );
      return( -1 );
    }
    if ( (theDef[0])->dim.x != (theDef[2])->dim.x ) {
      fprintf( stderr, "%s: deformation field components have different X dimensions\n", proc );
      return( -1 );
    }
    if ( (theDef[0])->dim.y != (theDef[2])->dim.y ) {
      fprintf( stderr, "%s: deformation field components have different Y dimensions\n", proc );
      return( -1 );
    }  
    if ( (theDef[0])->dim.z != (theDef[2])->dim.z ) {
      fprintf( stderr, "%s: deformation field components have different Z dimensions\n", proc );
      return( -1 );
    }
  }



  if ( theMatBefore == NULL ) {
    if ( resIm->dim.x != (theDef[0])->dim.x ) return( -1 );
    if ( resIm->dim.y != (theDef[0])->dim.y ) return( -1 );
    if ( resIm->dim.z != (theDef[0])->dim.z ) return( -1 );
  }

  if ( theMatAfter != (double*)NULL ) {
    for ( z = 0; z < 16; z ++ ) mat[z] = theMatAfter[z];
  } else {
    for ( z = 0; z < 16; z ++ ) mat[z] = 0.0;
    mat[0] = mat[5] = mat[10] = mat[15] = 1.0;
  }

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;
  
  defDim[0] = (theDef[0])->dim.x;
  defDim[1] = (theDef[0])->dim.y;
  defDim[2] = (theDef[0])->dim.z;
  


  switch ( (theDef[0])->type ) {
  default :
    fprintf( stderr, "%s: such deformation field type not handled yet", proc );
    return( -1 );
    
  case FLOAT :
    {
      r32 *bufDef[3] = {NULL, NULL, NULL};
      bufDef[0] = (r32*)(theDef[0]->buf);
      bufDef[1] = (r32*)(theDef[1]->buf);
      if ( theDef[2] != NULL )
        bufDef[2] = (r32*)(theDef[2]->buf);

      switch ( theIm->type ) {
      case UCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r32_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r32_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case SCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r32_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r32_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case USHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r32_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r32_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case SSHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r32_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r32_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case FLOAT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r32_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r32_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      default :
        VT_Error( "input image type unknown or not supported", proc );
        return( -1 );
      }
    }
    break;

  case DOUBLE :
    {
      r64 *bufDef[3] = {NULL, NULL, NULL};
      bufDef[0] = (r64*)(theDef[0]->buf);
      bufDef[1] = (r64*)(theDef[1]->buf);
      if ( theDef[2] != NULL )
        bufDef[2] = (r64*)(theDef[2]->buf);

      switch ( theIm->type ) {
      case UCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r64_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r64_u8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case SCHAR :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r64_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r64_s8( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case USHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r64_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r64_u16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case SSHORT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r64_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r64_s16( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      case FLOAT :
        {
          if ( theDef[2] != NULL )
            Reech3DTriLinVectorField_r64_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
          else
            Reech2DTriLinVectorField_r64_r32( (void*)(theIm->buf), theDim,
                                             (void*)(resIm->buf), resDim,
                                             bufDef, defDim, mat, theMatBefore );
        }
        break;
      default :
        VT_Error( "input image type unknown or not supported", proc );
        return( -1 );
      }
    }
    break;

  }

  return( 1 );
}




/* Procedure de reechantillonnage 3D avec des splines cubiques

   La matrice 4x4 permet le passage des coordonnees
   de l'image resultat a l'image d'entree. On calcule
   ainsi pour chaque point de l'image resultat son
   correspondant en coordonnees reelles dans l'image 
   d'entree. 
*/

int Reech3DCSpline4x4( vt_image *theIm, /* input image */
                      vt_image *resIm, /* result image */
                      double *mat )
{
  char *proc="Reech3DCSpline4x4";
  int theDim[3], resDim[3];
  int derivative[3] = {0, 0, 0};
  
  if (( theIm->dim.v != 1) || ( resIm->dim.v != 1)) return( -1 );

  theDim[0] = theIm->dim.x;
  theDim[1] = theIm->dim.y;
  theDim[2] = theIm->dim.z;

  resDim[0] = resIm->dim.x;
  resDim[1] = resIm->dim.y;
  resDim[2] = resIm->dim.z;

  if ( ReechCSpline4x4( (void*)(theIm->buf), theIm->type, theDim,
                        (void*)(resIm->buf), resIm->type, resDim,
                        mat, derivative ) != 1 ) {
    if ( _VERBOSE_ )
      fprintf( stderr, "%s: error when resampling\n", proc );
    return( -1 );
  }

  return( 1 );
}








void Reech3D_verbose ( )
{
  _VERBOSE_ = 1;
  setVerboseInReech4x4( _VERBOSE_ );
}

void Reech3D_noverbose ( )
{
  _VERBOSE_ = 0;
  setVerboseInReech4x4( _VERBOSE_ );
}
