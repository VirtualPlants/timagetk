/*************************************************************************
 * sbinter.c -
 *
 * $$
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
 * - Wed Jul 12 19:11:52 MET DST 2000, G. Malandain
 *   calcul de la taille du voxel
 *
 */

#include <vt_common.h>
#include <vt_reech.h>
#include <vt_seuil.h>
#include <chamfercoefficients.h>
#include <chamferdistance.h>
#include <basictransform.h>

#define MODE_RESIZE  0
#define MODE_MATRICE 1
#define MODE_ISOTROPIC 3
#define MODE_ADD 4

#define INTR_LINEAR  0
#define INTR_CSPLINE 2

typedef enum {
  _REAL_,
  _VOXEL_
} enumMatrixType;

typedef struct local_par {
  vt_names names;
  vt_ipt newdim;

  double voxel_size_in[3];
  double voxel_size_out[3];
  double voxel_isotropic;

  char distanceimage[STRINGLENGTH];

  char matrice[STRINGLENGTH];
  enumMatrixType matrixType;
  int inverse;

  int addx;
  int addy;
  int addz;

  char matrice_out[STRINGLENGTH];
  char imagetemplate[STRINGLENGTH];


  int mode;
  int interp;

  double threshold;

  int xy;
  int xz;
  int yz;

  int borders;

  int enumerate;
  
  int chamfercoef_dim;
  int chamfercoef_size;
  int chamfercoef_max;

} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );
static void WriteMatrice( char *name, double *mat );
static int  ReadMatrice( char *name, double *mat );
static void MatriceFromRealToVoxels( double *mat, double *sizeIn, double *sizeOut );
static int  InverseMat4x4( double *matrice, double *inv );
static int _ChangeDistanceAtBordersAlongXY( vt_image *imdist );
static int _ComputeChamferWeights( typeChamferMask *theMaskXY, 
				   typeChamferMask *theMaskZX, 
				   typeChamferMask *theMaskYZ, 
				   double *original_voxel_size,
				   int size,
				   int dim,
				   int maxvalueofmincoefficient );



static char *usage = "[image-in] [image-out]\n\
\t [-threshold|-t %lf] [-xy] [-xz] [-yz] [-borders|-no-borders]\n\
\t [-x %d] [-y %d] [-z %d] [-mat-real %s] [-mat-voxels %s] [-dims %s]\n\
\t [-addx %d] [-addy %d] [-addz %d] [-enumerate]\n\
\t [-linear | -cspline]\n\
\t [-iso %lf] [-omat|-out-matrice %s] [-distance %s]\n\
\t [-cc-dim %d] [-cc-size %d] [-cc-max %d]\n\
\t [-vin %f %f %f] [-vout %f %f %f]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\n\
\t [-x|-tx] %d : dimension along X of 'image-out'\n\
\t [-y|-ty] %d : dimension along Y of 'image-out'\n\
\t [-z|-tz] %d : dimension along Z of 'image-out'\n\
\t si une dimension n'est pas donnee, on prend celle de 'image-in'.\n\
\t si juste les dimensions sont donnees, on 'resize' l'image.\n\
\t    [-out-matrice %s] sert a ecrire la matrice utilisee\n\
\t -iso %f : taille de voxel isotropique\n\
\n\
\t -mat-real %s : matrix 4x4 from 'image-out' to 'image-in'.\n\
\t           M(in) = MAT * M(out).\n\
\t           (expressed in real world coordinate (not voxels)\n\
\t -mat-voxel %s : matrix 4x4 from 'image-out' to 'image-in'.\n\
\t           M(in) = MAT * M(out).\n\
\t           (expressed in voxel coordinates (not real world)\n\
\t -inv : inverse the matrix given by either -mat-real or -mat-voxel.\n\
\n\
\t -linear : interpolation bi- ou tri-lineaire de l'intensite.\n\
\t -cspline : utilisation de splines cubiques\n\
\t -vin %f %f %f : taille des voxels de l'image d'entree 'image-in'\n\
\t -vout %f %f %f : taille des voxels de l'image resultat 'image-out'\n\
\t -inv : inverse la matrice donnee par -mat.\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s : short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imsum, imres, *imtemplate;
  ImageType imsumType=FLOAT;

  vt_image imdist, imreech;
  vt_image image2, imdist2;

  vt_image minsum, imagecc;

  int cc, number_of_cc=1, minnotinitialized=0;
  void *inputbuf;
  bufferType inputtype;
  vt_3m mmm;
  
  vt_ipt resdim;
  double mat[16];
  int i;

  double vsize[3];

  int theDims[3], theDims2[3];  
  typeChamferMask theMaskXY;
  typeChamferMask theMaskZX;
  typeChamferMask theMaskYZ;


  if ( 0 ) fprintf( stderr, "UNDER DEVELOPMENT\n" );


  /*  0  1  2  3
      4  5  6  7
      8  9 10 11
     12 13 14 15 
     */
  for (i=0;i<16;i++) mat[i] = 0.0;
  mat[0] = mat[5] = mat[10] = mat[15] = 1.0;
  
  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  if ( _VT_VERBOSE_ != 0 ) Reech3D_verbose();
    
  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);

  vsize[0] = image->siz.x;
  vsize[1] = image->siz.y;
  vsize[2] = image->siz.z;


  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.swap == 1 ) VT_SwapImage( image );
  
  /*--- initialisation de l'image resultat ---*/

  resdim = par.newdim;
  if (resdim.x <= 0) resdim.x = image->dim.x;
  if (resdim.y <= 0) resdim.y = image->dim.y;
  if (resdim.z <= 0) resdim.z = image->dim.z;

  
  
  

  /* on determine la matrice */
  switch ( par.mode ) {
  case MODE_MATRICE :

    if ( par.matrice[0] != '\0' ) {

      if ( ReadMatrice( par.matrice, mat ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to read matrice\n", 0);
      }
      if ( par.inverse != 0 ) {
	double tmp[16];
	int rang;
	if ( (rang=InverseMat4x4( mat, tmp )) != 4 ) {
	  fprintf( stderr, "Warning: la matrice %s est de rang %d.\n",
		   par.matrice, rang );
	}
	for (i=0;i<16;i++) mat[i] = tmp[i];
      } 

    } else {
      for (i=0;i<16;i++) mat[i] = 0.0;
      mat[0] = mat[5] = mat[10] = mat[15] = 1.0;
    }
    
    if ( par.matrixType == _REAL_ )
      MatriceFromRealToVoxels( mat, par.voxel_size_in, par.voxel_size_out );


    break;

  case MODE_ISOTROPIC :
    
    /* Along one dimension, the field of view is oldim*oldsize,
       thus we have olddim*oldsize=newdim*newsize, and then
       newdim = olddim*oldsize / newsizec

       Moreover, the "integer" (or voxel) coordinates are at the voxel center,
       thus in voxel coordinates, the field of view goes from
       -0.5 to dim-1+0.5=dim-0.5.
       Then in real world coordinates, it goes from -size/2 to dim*size-size/2

       When resampling newimage from oldimage, we match the real world coordinates,
       it does that 
       (newx + newsize/2)/(newdim*newsize) = (oldx + oldsize/2)/(olddim*oldsize)
       then oldx = newx + oldsize/2 - newsize/2
    */
    {
      double newsize[3] = {par.voxel_isotropic, par.voxel_isotropic, par.voxel_isotropic};
      double oldsize[3] = {image->siz.x, image->siz.y, image->siz.z};
       
      mat[ 3] = newsize[0]/2 - oldsize[0]/2;
      mat[ 7] = newsize[1]/2 - oldsize[1]/2;
      mat[11] = newsize[2]/2 - oldsize[2]/2;
      
      MatriceFromRealToVoxels( mat, oldsize, newsize );
    }

    resdim.x = (int)( (image->dim.x * image->siz.x / par.voxel_isotropic) + 0.1 );
    resdim.y = (int)( (image->dim.y * image->siz.y / par.voxel_isotropic) + 0.1 );
    resdim.z = (int)( (image->dim.z * image->siz.z / par.voxel_isotropic) + 0.1 );

    if ( resdim.x == (int)image->dim.x ) par.yz = 0;
    if ( resdim.y == (int)image->dim.y ) par.xz = 0;
    if ( resdim.z == (int)image->dim.z ) par.xy = 0;
    
    break;

  case MODE_ADD :
    
    {
      double newsize[3];
      double oldsize[3] = {image->siz.x, image->siz.y, image->siz.z};
       
      newsize[0] = oldsize[0] / (par.addx + 1);
      newsize[1] = oldsize[1] / (par.addy + 1);
      newsize[2] = oldsize[2] / (par.addz + 1);

      /* no translation here
      mat[ 3] = newsize[0]/2 - oldsize[0]/2;
      mat[ 7] = newsize[1]/2 - oldsize[1]/2;
      mat[11] = newsize[2]/2 - oldsize[2]/2;
      */
      
      MatriceFromRealToVoxels( mat, oldsize, newsize );
    }

    resdim.x = (image->dim.x - 1)*(par.addx+1) + 1;
    resdim.y = (image->dim.y - 1)*(par.addy+1) + 1;
    resdim.z = (image->dim.z - 1)*(par.addz+1) + 1;

    if ( resdim.x == (int)image->dim.x ) par.yz = 0;
    if ( resdim.y == (int)image->dim.y ) par.xz = 0;
    if ( resdim.z == (int)image->dim.z ) par.xy = 0;
    
    break;

  case MODE_RESIZE :
  default :
    {
      double newsize[3];
      double oldsize[3] = {image->siz.x, image->siz.y, image->siz.z};
       
      newsize[0] = image->dim.x * image->siz.x / resdim.x;
      newsize[1] = image->dim.y * image->siz.y / resdim.y;
      newsize[2] = image->dim.z * image->siz.z / resdim.z;
      
      mat[ 3] = newsize[0]/2 - oldsize[0]/2;
      mat[ 7] = newsize[1]/2 - oldsize[1]/2;
      mat[11] = newsize[2]/2 - oldsize[2]/2;
      
      MatriceFromRealToVoxels( mat, oldsize, newsize );
    }

    if ( resdim.x == (int)image->dim.x ) par.yz = 0;
    if ( resdim.y == (int)image->dim.y ) par.xz = 0;
    if ( resdim.z == (int)image->dim.z ) par.xy = 0;
    
  }



  if ( par.xy + par.xz + par.yz == 1 )
    imsumType = SSHORT;

  VT_Image( &imsum );

  if ( par.imagetemplate[0] != '\0' ) {
    imtemplate = _VT_Inrimage( par.imagetemplate );
    if ( imtemplate == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read template image\n", 0);
    if ( par.distanceimage[0] != '\0' )
      VT_InitFromImage( &imsum, imtemplate, par.distanceimage, imsumType );
    else
      VT_InitFromImage( &imsum, imtemplate, "sumofdistances.inr", imsumType );
    VT_FreeImage( imtemplate );
    VT_Free( (void**)&imtemplate );
    resdim.x = imsum.dim.x;
    resdim.y = imsum.dim.y;
    resdim.z = imsum.dim.z;
  }
  else {
    if ( par.distanceimage[0] != '\0' )
      VT_InitImage( &imsum, par.distanceimage, resdim.x, resdim.y, resdim.z, imsumType );
    else
      VT_InitImage( &imsum, "sumofdistances.inr", resdim.x, resdim.y, resdim.z, imsumType );
    
    imsum.siz.x = (float)sqrt( mat[0]*image->siz.x * mat[0]*image->siz.x +
			mat[4]*image->siz.y * mat[4]*image->siz.y +
			mat[8]*image->siz.z * mat[8]*image->siz.z );
    imsum.siz.y = (float)sqrt( mat[1]*image->siz.x * mat[1]*image->siz.x +
			mat[5]*image->siz.y * mat[5]*image->siz.y +
			mat[9]*image->siz.z * mat[9]*image->siz.z );
    imsum.siz.z = (float)sqrt( mat[2]*image->siz.x * mat[2]*image->siz.x +
			mat[6]*image->siz.y * mat[6]*image->siz.y +
			mat[10]*image->siz.z * mat[10]*image->siz.z );
  }

  if ( VT_AllocImage( &imsum ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate sum of distances image\n", 0);
  }
  



  
  
  
  VT_Image( &imdist );
  VT_InitFromImage( &imdist, image, "distance.inr", SSHORT );
  
  if ( VT_AllocImage( &imdist ) != 1 ) {
    VT_FreeImage( &imsum );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate distance image\n", 0);
  }
  
  VT_Image( &imreech );
  VT_InitImage( &imreech, "distance_reech.inr", resdim.x, resdim.y, resdim.z, SSHORT );
  if ( VT_AllocImage( &imreech ) != 1 ) {
    VT_FreeImage( &imdist );
    VT_FreeImage( &imsum );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate distance reech image\n", 0);
  }
  



  initChamferMask( &theMaskXY );
  initChamferMask( &theMaskZX );
  initChamferMask( &theMaskYZ );

  switch ( par.chamfercoef_dim ) {

  default :

    if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMaskXY ) != 1 ) {
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "unable to build chamfer mask XY\n", 0 );
    }
    if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMaskZX ) != 1 ) {
      freeChamferMask( &theMaskXY );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "unable to build chamfer mask ZX\n", 0 );
    }
    if ( buildPredefinedChamferMask( _CHAMFER5x5_, &theMaskYZ ) != 1 ) {
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskXY );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "unable to build chamfer maskYZ\n", 0 );
    }
    break;

  case 2 :
    vsize[0] = vsize[1] = vsize[2] = 1.0;
  case 3 :
    if ( _ComputeChamferWeights( &theMaskXY, &theMaskZX, &theMaskYZ, 
				 vsize, par.chamfercoef_size, par.chamfercoef_dim,
				 par.chamfercoef_max ) != 1 ) {
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "unable to build chamfer masks\n", 0 );
    }
  }



  theDims[0] = image->dim.x;
  theDims[1] = image->dim.y;
  theDims[2] = image->dim.z;



  /* XY :
     imdist <- image : calcul de la distance
     imreech <- imdist : reechantillonage de la distance
     imsum <- imreech : sommation de la distance

     AUTRES
     image2 <- image : transposition
     imdist2 <- image2 : calcul de la distance
     imdist <- imdist2  : transposition
     imreech <- imdist : reechantillonage de la distance
     imsum <- imreech : sommation de la distance
  */

  VT_Image( &minsum );
  VT_Image( &imagecc );

  if ( par.enumerate > 0 ) {
    if ( VT_3m( image, (vt_image*)NULL, &mmm ) == 0 ) {
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error when min/max of image\n", 0);
    }
    number_of_cc = (int)(mmm.max+0.5);
    if ( number_of_cc <= 0 ) {
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "found no connected component\n", 0);
    }

    if ( par.distanceimage[0] != '\0' )
      VT_InitFromImage( &minsum, &imsum, par.distanceimage, imsum.type );
    else
      VT_InitFromImage( &minsum, &imsum, "minimumsumofdistances.inr", imsum.type );
    if ( VT_AllocImage( &minsum ) != 1 ) {
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "unable to allocate image of the minimum of distances\n", 0);
    }
    VT_ZeroImage( &minsum );
    
    VT_InitFromImage( &imagecc, image, "thresholdedimage.inr", UCHAR );
    if ( VT_AllocImage( &imagecc ) != 1 ) {
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &minsum );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "unable to allocate thresholded\n", 0);
    }
  }


  





  for ( cc=1; cc<=number_of_cc; cc++ ) {

    if ( par.enumerate > 0 ) {
      fprintf( stderr, "... processing component #%3d/%d\n", cc, number_of_cc );
      (void)VT_Threshold2( image, &imagecc, (float)cc, (float)cc );
      (void)VT_3m( &imagecc, (vt_image*)NULL, &mmm );
      if ( mmm.max < 1 ) {
	fprintf( stderr, "    empty component, skip it\n" );
	continue;
      }
    }

    


  VT_ZeroImage( &imsum );

  /* XY
   */
  if ( par.xy ) {

    fprintf( stderr, "... compute XY interpolation\n" );
    printChamferMaskAsImage( stderr, &theMaskXY );

    if ( par.enumerate > 0 ) {
      inputbuf = imagecc.buf;
      inputtype = imagecc.type;
    }
    else {
      inputbuf = image->buf;
      inputtype = image->type;
    }

    if ( _ComputePositiveNegativeChamferMap( inputbuf, inputtype,
					     imdist.buf, imdist.type,
					     theDims,
					     par.threshold, &theMaskXY, 0 ) < 0 ) {
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error when computing distance (XYZ)\n", 0);
    }

    if ( par.borders ) {
      if ( _ChangeDistanceAtBordersAlongXY( &imdist ) <= 0 ) {
	freeChamferMask( &theMaskXY );
	freeChamferMask( &theMaskZX );
	freeChamferMask( &theMaskYZ );
	VT_FreeImage( &imreech );
	VT_FreeImage( &imdist );
	VT_FreeImage( &imsum );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "error when modifying distances (XYZ)\n", 0);
      }
    }

    if ( 0 ) (void)VT_WriteInrimage( &imdist );

    switch ( par.interp ) { 
    case INTR_CSPLINE :
	if ( Reech3DCSpline4x4( &imdist, &imreech, mat ) != 1 ) {
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
      case INTR_LINEAR :
      default :
	if ( Reech3DTriLin4x4( &imdist, &imreech, mat ) != 1 ) {
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
    }


    

    switch ( imsum.type ) {

    case SSHORT :
      {
	s16 *sumBuf=(s16*)imsum.buf;

	switch ( imreech.type ) {
	case SSHORT :
	  {
	    s16 *theBuf = (s16*)imreech.buf;
	    for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	      sumBuf[i] += theBuf[i];
	  }
	  break;
	default :
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("type not handled in switch\n", 0);
	}
      }
      break;

    case FLOAT :
      {
	r32 *sumBuf=(r32*)imsum.buf;

	switch ( imreech.type ) {
	case SSHORT :
	  {
	    s16 *theBuf = (s16*)imreech.buf;
	    for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	      sumBuf[i] += theBuf[i];
	  }
	  break;
	default :
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("type not handled in switch\n", 0);
	}
      }
      break;

    default :
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("type not handled in switch\n", 0);

    }      

  } /* end of XY */





  /* ZX
     image is transposed into image2 whose dimensions are dimz,dimx,dimy
   */
  if ( par.xz ) {
    
    fprintf( stderr, "... compute ZX interpolation\n" );
    printChamferMaskAsImage( stderr, &theMaskZX );

    if ( par.enumerate > 0 ) {
      inputbuf = imagecc.buf;
      inputtype = imagecc.type;
    }
    else {
      inputbuf = image->buf;
      inputtype = image->type;
    }

    VT_Image( &image2 );
    VT_InitImage( &image2, "image2zxy.inr", image->dim.z, image->dim.x, image->dim.y, inputtype );
    if ( VT_AllocImage( &image2 ) != 1 ) {
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate image 2\n", 0);
    }
    
    VT_Image( &imdist2 );
    VT_InitImage( &imdist2, "distance2zxy.inr", image->dim.z, image->dim.x, image->dim.y, imdist.type );
    if ( VT_AllocImage( &imdist2 ) != 1 ) {
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate distance image 2 \n", 0);
    }

    if ( Transpose_ZXY( inputbuf, image2.buf, inputtype, theDims ) != 1 ) {
      VT_FreeImage( &imdist2 );
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to transpose input image (ZXY) \n", 0);
    }
    
    theDims2[0] = image2.dim.x;
    theDims2[1] = image2.dim.y;
    theDims2[2] = image2.dim.z;

    if ( _ComputePositiveNegativeChamferMap( image2.buf, image2.type,
					     imdist2.buf, imdist2.type,
					     theDims2,
					     par.threshold, &theMaskZX, 0 ) < 0 ) {
      VT_FreeImage( &imdist2 );
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error when computing distance (ZXY)\n", 0);
    }
    
    if ( par.borders ) {
      if ( _ChangeDistanceAtBordersAlongXY( &imdist2 ) <= 0 ) {
	VT_FreeImage( &imdist2 );
	VT_FreeImage( &image2 );
	freeChamferMask( &theMaskXY );
	freeChamferMask( &theMaskZX );
	freeChamferMask( &theMaskYZ );
	VT_FreeImage( &imreech );
	VT_FreeImage( &imdist );
	VT_FreeImage( &imsum );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "error when modifying distances (ZXY)\n", 0);
      }
    }

    if ( Transpose_YZX( imdist2.buf, imdist.buf, imdist2.type, theDims2 ) != 1 ) {
      VT_FreeImage( &imdist2 );
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to transpose distance image (ZXY) \n", 0);
    }

    VT_FreeImage( &imdist2 );
    VT_FreeImage( &image2 );
    
    if ( 0 ) (void)VT_WriteInrimage( &imdist );

    switch ( par.interp ) { 
    case INTR_CSPLINE :
	if ( Reech3DCSpline4x4( &imdist, &imreech, mat ) != 1 ) {
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
      case INTR_LINEAR :
      default :
	if ( Reech3DTriLin4x4( &imdist, &imreech, mat ) != 1 ) {
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
    }



    switch ( imsum.type ) {

    case SSHORT :
      {
	s16 *sumBuf=(s16*)imsum.buf;

	switch ( imreech.type ) {
	case SSHORT :
	  {
	    s16 *theBuf = (s16*)imreech.buf;
	    for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	      sumBuf[i] += theBuf[i];
	  }
	  break;
	default :
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("type not handled in switch\n", 0);
	}
      }
      break;

    case FLOAT :
      {
	r32 *sumBuf=(r32*)imsum.buf;

	switch ( imreech.type ) {
	case SSHORT :
	  {
	    s16 *theBuf = (s16*)imreech.buf;
	    for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	      sumBuf[i] += theBuf[i];
	  }
	  break;
	default :
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("type not handled in switch\n", 0);
	}
      }
      break;

    default :
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("type not handled in switch\n", 0);

    }      


  } /* end of ZX */
 





  /* YZ
     image is transposed into image2 whose dimensions are dimy,dimz,dimyx
   */
  if ( par.yz ) {

    fprintf( stderr, "... compute YZ interpolation\n" );
    printChamferMaskAsImage( stderr, &theMaskYZ );

    if ( par.enumerate > 0 ) {
      inputbuf = imagecc.buf;
      inputtype = imagecc.type;
    }
    else {
      inputbuf = image->buf;
      inputtype = image->type;
    }

    VT_Image( &image2 );
    VT_InitImage( &image2, "image2yzx.inr", image->dim.y, image->dim.z, image->dim.x, inputtype );
    if ( VT_AllocImage( &image2 ) != 1 ) {
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate image 2\n", 0);
    }
    
    VT_Image( &imdist2 );
    VT_InitImage( &imdist2, "distance2yzx.inr", image->dim.y, image->dim.z, image->dim.x, imdist.type );
    if ( VT_AllocImage( &imdist2 ) != 1 ) {
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate distance image 2 \n", 0);
    }

    if ( Transpose_YZX( inputbuf, image2.buf, inputtype, theDims ) != 1 ) {
      VT_FreeImage( &imdist2 );
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to transpose input image (ZXY) \n", 0);
    }
    
    theDims2[0] = image2.dim.x;
    theDims2[1] = image2.dim.y;
    theDims2[2] = image2.dim.z;

    if ( _ComputePositiveNegativeChamferMap( image2.buf, image2.type,
					     imdist2.buf, imdist2.type,
					     theDims2,
					     par.threshold, &theMaskYZ, 0 ) < 0 ) {
      VT_FreeImage( &imdist2 );
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error when computing distance (YZX)\n", 0);
    }
    
    if ( par.borders ) {
      if ( _ChangeDistanceAtBordersAlongXY( &imdist2 ) <= 0 ) {
	VT_FreeImage( &imdist2 );
	VT_FreeImage( &image2 );
	freeChamferMask( &theMaskXY );
	freeChamferMask( &theMaskZX );
	freeChamferMask( &theMaskYZ );
	VT_FreeImage( &imreech );
	VT_FreeImage( &imdist );
	VT_FreeImage( &imsum );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "error when modifying distances (YZX)\n", 0);
      }
    }

    if ( Transpose_ZXY( imdist2.buf, imdist.buf, imdist2.type, theDims2 ) != 1 ) {
      VT_FreeImage( &imdist2 );
      VT_FreeImage( &image2 );
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to transpose distance image (YZX) \n", 0);
    }

    VT_FreeImage( &imdist2 );
    VT_FreeImage( &image2 );
    
    if ( 0 ) (void)VT_WriteInrimage( &imdist );

    switch ( par.interp ) { 
    case INTR_CSPLINE :
	if ( Reech3DCSpline4x4( &imdist, &imreech, mat ) != 1 ) {
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
      case INTR_LINEAR :
      default :
	if ( Reech3DTriLin4x4( &imdist, &imreech, mat ) != 1 ) {
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
    }

    
    
    switch ( imsum.type ) {

    case SSHORT :
      {
	s16 *sumBuf=(s16*)imsum.buf;

	switch ( imreech.type ) {
	case SSHORT :
	  {
	    s16 *theBuf = (s16*)imreech.buf;
	    for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	      sumBuf[i] += theBuf[i];
	  }
	  break;
	default :
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("type not handled in switch\n", 0);
	}
      }
      break;

    case FLOAT :
      {
	r32 *sumBuf=(r32*)imsum.buf;

	switch ( imreech.type ) {
	case SSHORT :
	  {
	    s16 *theBuf = (s16*)imreech.buf;
	    for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	      sumBuf[i] += theBuf[i];
	  }
	  break;
	default :
	  freeChamferMask( &theMaskXY );
	  freeChamferMask( &theMaskZX );
	  freeChamferMask( &theMaskYZ );
	  VT_FreeImage( &imreech );
	  VT_FreeImage( &imdist );
	  VT_FreeImage( &imsum );
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("type not handled in switch\n", 0);
	}
      }
      break;

    default :
      freeChamferMask( &theMaskXY );
      freeChamferMask( &theMaskZX );
      freeChamferMask( &theMaskYZ );
      VT_FreeImage( &imreech );
      VT_FreeImage( &imdist );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("type not handled in switch\n", 0);

    }      

  } /* end of YZ */


  if ( par.enumerate > 0 ) {
    if ( minnotinitialized == 0 ) {
      (void)VT_CopyImage( &imsum, &minsum );
      minnotinitialized = 1;
    }
    else {
      switch ( imsum.type ) {
      case SSHORT :
	{
	  s16 *minbuf = (s16*)minsum.buf;
	  s16 *sumbuf = (s16*)imsum.buf;
	  for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	    if ( minbuf[i] > sumbuf[i] ) minbuf[i] = sumbuf[i];
	}
	break;
      case FLOAT :
	{
	  r32 *minbuf = (r32*)minsum.buf;
	  r32 *sumbuf = (r32*)imsum.buf;
	  for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	    if ( minbuf[i] > sumbuf[i] ) minbuf[i] = sumbuf[i];
	}
	break;
      default :
	VT_ErrorParse("type not handled in switch\n", 0);
      }
    }
  }



  } /* end of for ( cc=1; cc<=number_of_cc; cc++ ) */




  /* some memory can be made free
   */
  VT_FreeImage( &imreech );
  VT_FreeImage( &imdist );
  freeChamferMask( &theMaskXY );
  freeChamferMask( &theMaskZX );
  freeChamferMask( &theMaskYZ );
  if ( par.enumerate > 0 ) {
    VT_FreeImage( &imagecc );
  }



  

  VT_Image( &imres );
  VT_InitFromImage( &imres, &imsum, par.names.out, UCHAR );
  imres.siz.x = imsum.siz.x;
  imres.siz.y = imsum.siz.y;
  imres.siz.z = imsum.siz.z;

  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( &imsum );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }


  



  
  /* threshold of the sum of distances image
   */
  if ( par.enumerate > 0 )
    inputbuf = minsum.buf;
  else
    inputbuf = imsum.buf;
  {
    u8 *resBuf = (u8*)imres.buf;
    switch ( imsum.type ) {
    case SSHORT :
      {
	s16 *sumBuf=(s16*)inputbuf;
	for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	  resBuf[i] = (sumBuf[i] < 0) ? 255 : 0; 
      }
      break;
    case FLOAT :
      {
	r32 *sumBuf = (r32*)inputbuf;
	for ( i=0; i<resdim.x*resdim.y*resdim.z; i++ )
	  resBuf[i] = (sumBuf[i] < 0.0) ? 255 : 0; 
      }
      break;
    default :
      VT_FreeImage( &imres );
      VT_FreeImage( &imsum );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("type not handled in switch\n", 0);
    }
  }




  /* writing
   */

  if ( par.matrice_out[0] != '\0' ) WriteMatrice( par.matrice_out, mat );

  
  /*--- ecriture de l'image resultat ---*/
  if ( par.distanceimage[0] != '\0' ) {
    if ( par.enumerate > 0 ) {
      if ( VT_WriteInrimage( &minsum ) == -1 ) {
	VT_FreeImage( &imres );
	VT_FreeImage( &imsum );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to write output image\n", 0);
      }
    }
    else {
      if ( VT_WriteInrimage( &imsum ) == -1 ) {
	VT_FreeImage( &imres );
	VT_FreeImage( &imsum );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to write output image\n", 0);
      }
    }
  }

  if ( par.enumerate > 0 ) {
    VT_FreeImage( &minsum );
  }

  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( &imres );
    VT_FreeImage( &imsum );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( &imres );
  VT_FreeImage( &imsum );
  VT_FreeImage( image );
  VT_Free( (void**)&image );

  return( 1 );
}




















static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  char text[STRINGLENGTH];
  
  if ( VT_CopyName( program, argv[0] ) != 1 )
    VT_Error("Error while copying program name", (char*)NULL);
  if ( argc == 1 ) VT_ErrorParse("\n", 0 );
  
  /*--- lecture des parametres ---*/
  i = 1; nb = 0;
  while ( i < argc ) {
    if ( argv[i][0] == '-' ) {
      if ( argv[i][1] == '\0' ) {
	if ( nb == 0 ) {
	  /*--- standart input ---*/
	  strcpy( par->names.in, "<" );
	  nb += 1;
	}
      }
      /*--- arguments generaux ---*/
      else if ( strcmp ( argv[i], "-help" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "--help" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-?" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 ) {
	if ( _VT_DEBUG_ <= 0 )
	  _VT_DEBUG_ = 1;
	else 
	  _VT_DEBUG_ ++;
      }
      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }

      /*--- ---*/
      else if ( strcmp ( argv[i], "-distance" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -distance...\n", 0 );
	strncpy( par->distanceimage, argv[i], STRINGLENGTH ); 
      }


      else if ( strcmp ( argv[i], "-enumerate" ) == 0 ) {
	par->enumerate = 1;
      }
      /*--- matrice ---*/
      else if ( strcmp ( argv[i], "-mat-real" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -mat-real...\n", 0 );
	strncpy( par->matrice, argv[i], STRINGLENGTH ); 
	par->mode = MODE_MATRICE;
	par->matrixType = _REAL_;
      }

      else if ( strcmp ( argv[i], "-mat-voxel" ) == 0 || strcmp ( argv[i], "-mat-voxels" ) == 0) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -mat-voxel...\n", 0 );
	strncpy( par->matrice, argv[i], STRINGLENGTH ); 
	par->mode = MODE_MATRICE;
	par->matrixType = _VOXEL_;
      }

      else if ( strcmp ( argv[i], "-omat" ) == 0 ||
		strcmp ( argv[i], "-out-matrice" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -out-matrice...\n", 0 );
	strncpy( par->matrice_out, argv[i], STRINGLENGTH );  
      }
      
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->inverse = 1;
      }

      else if ( strcmp ( argv[i], "-dims" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -dims...\n", 0 );
	strncpy( par->imagetemplate, argv[i], STRINGLENGTH );  
      }


      /*--- type d'interpolation ---*/
      else if ( strcmp ( argv[i], "-linear" ) == 0 ) {
	par->interp = INTR_LINEAR;
      }
      else if ( strcmp ( argv[i], "-cspline" ) == 0 ) {
	par->interp = INTR_CSPLINE;
      }

      /*--- ---*/
      else if ( strcmp ( argv[i], "-addx" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -addx...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->addx) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -addx...\n", 0 );
	par->mode = MODE_ADD;
      }
      else if ( strcmp ( argv[i], "-addy" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -addy...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->addy) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -addy...\n", 0 );
	par->mode = MODE_ADD;
      }
      else if ( strcmp ( argv[i], "-addz" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -addz...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->addz) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -addz...\n", 0 );
	par->mode = MODE_ADD;
      }


      else if ( strcmp ( argv[i], "-xy" ) == 0 ) {
	if ( par->xy == 1 && par->xz == 1 && par->yz == 1 )
	  par->xy = par->xz = par->yz = 0;
	par->xy = 1;
      }
      else if ( strcmp ( argv[i], "-xz" ) == 0 ) {
	if ( par->xy == 1 && par->xz == 1 && par->yz == 1 )
	  par->xy = par->xz = par->yz = 0;
	par->xz = 1;
      }
      else if ( strcmp ( argv[i], "-yz" ) == 0 ) {
	if ( par->xy == 1 && par->xz == 1 && par->yz == 1 )
	  par->xy = par->xz = par->yz = 0;
	par->yz = 1;
      }
   
      else if ( strcmp ( argv[i], "-borders" ) == 0 ) {
	par->borders = 1;
      }
      else if ( strcmp ( argv[i], "-no-borders" ) == 0 ) {
	par->borders = 0;
      }

      else if ( strcmp ( argv[i], "-cc-dim" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -cc-dim...\n", 0 );
	status = sscanf( argv[i],"%d", &(par->chamfercoef_dim) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -cc-dim...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-cc-size" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -cc-size...\n", 0 );
	status = sscanf( argv[i],"%d", &(par->chamfercoef_size) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -cc-size...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-cc-max" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -cc-max...\n", 0 );
	status = sscanf( argv[i],"%d", &(par->chamfercoef_max) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -cc-max...\n", 0 );
      }

      /*--- deformation, bias et gain ---*/      
      else if ( strcmp ( argv[i], "-iso" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -iso...\n", 0 );
	status = sscanf( argv[i],"%lf", &(par->voxel_isotropic) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -iso...\n", 0 );
	par->mode = MODE_ISOTROPIC;
      }

      /*--- taille des voxels ---*/
      else if ( strcmp ( argv[i], "-vin" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vin...\n", 0 );
	status = sscanf( argv[i],"%lf", &(par->voxel_size_in[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vin...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vin...\n", 0 );
	status = sscanf( argv[i],"%lf", &(par->voxel_size_in[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vin...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vin...\n", 0 );
	status = sscanf( argv[i],"%lf", &(par->voxel_size_in[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vin...\n", 0 );
	if ( (par->voxel_size_in[0] <= 0.0) || (par->voxel_size_in[1] <= 0.0) 
	     || (par->voxel_size_in[2] <= 0.0) )
	  VT_ErrorParse( "parsing -vin... bad arguments\n", 0 );
      }

      else if ( strcmp ( argv[i], "-vout" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vout...\n", 0 );
	status = sscanf( argv[i],"%lf", &(par->voxel_size_out[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vout...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vout...\n", 0 );
	status = sscanf( argv[i],"%lf", &(par->voxel_size_out[1]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vout...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -vout...\n", 0 );
	status = sscanf( argv[i],"%lf", &(par->voxel_size_out[2]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -vout...\n", 0 );
	if ( (par->voxel_size_out[0] <= 0.0) || (par->voxel_size_out[1] <= 0.0) 
	     || (par->voxel_size_out[2] <= 0.0) )
	  VT_ErrorParse( "parsing -vout... bad arguments\n", 0 );
      }

      /*--- dimension de l'image de sortie ---*/
      else if ( strcmp ( argv[i], "-tx" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -tx...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->newdim.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -tx...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-ty" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -ty...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->newdim.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -ty...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-tz" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -tz...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->newdim.z) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -tz...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-x" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->newdim.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-y" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->newdim.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-z" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->newdim.z) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
      }

      else if ( (strcmp ( argv[i], "-threshold" ) == 0) || 
		(strcmp ( argv[i], "-th" ) == 0) ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -threshold...\n", 0 );
	status = sscanf( argv[i], "%lf", &par->threshold );
	if ( status <= 0 ) VT_ErrorParse( "parsing -threshold...\n", 0 );
      }


      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
	if ( _VT_VERBOSE_ <= 0 )
	  _VT_VERBOSE_ = 1;
	else
	  _VT_VERBOSE_ ++;
	_set_chamfer_verbose( _VT_VERBOSE_ );
      }



      /*--- option inconnue ---*/
      else {
	sprintf(text,"unknown option %s\n",argv[i]);
	VT_ErrorParse(text, 0);
      }
    }
    /*--- saisie des noms d'images ---*/
    else if ( argv[i][0] != 0 ) {
      if ( nb == 0 ) { 
	strncpy( par->names.in, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else if ( nb == 1 ) {
	strncpy( par->names.out, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else if ( nb == 2 ) {
	strncpy( par->names.ext, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else 
	VT_ErrorParse("too much file names when parsing\n", 0 );
    }
    i += 1;
  }
  
  /*--- s'il n'y a pas assez de noms ... ---*/
  if (nb == 0) {
    strcpy( par->names.in,  "<" );  /* standart input */
    strcpy( par->names.out, ">" );  /* standart output */
  }
  if (nb == 1)
    strcpy( par->names.out, ">" );  /* standart output */
  
  /*--- compatibilite avec reech3D ---*/
  if (nb == 3) {
    strncpy( par->matrice, par->names.out, STRINGLENGTH );
    strncpy( par->names.out, par->names.ext, STRINGLENGTH );
    par->mode = MODE_MATRICE;
  }
}



static void VT_ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage : %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",detail);
  (void)fprintf(stderr,"Erreur : %s",str);
  exit(0);
}



static void VT_InitParam( local_par *par )
{
  VT_Names( &(par->names) );
  par->voxel_size_in[0]  = par->voxel_size_in[1]  = par->voxel_size_in[2]  = 1.0;
  par->voxel_size_out[0] = par->voxel_size_out[1] = par->voxel_size_out[2] = 1.0;
  par->voxel_isotropic = 1.0;
  par->newdim.x = par->newdim.y = par->newdim.z = 0;

  par->distanceimage[0] ='\0';

  par->matrice[0] ='\0';
  par->matrice_out[0] ='\0';
  par->imagetemplate[0] ='\0';

  par->inverse = 0;
  par->mode = MODE_RESIZE;
  par->interp = INTR_LINEAR;

  par->addx = 0;
  par->addy = 0;
  par->addz = 0;

  par->threshold = 1.0;
  par->xy = 1;
  par->xz = 1;
  par->yz = 1;

  par->borders = 0;
  par->enumerate = 0;

  par->chamfercoef_dim = 0;
  par->chamfercoef_size = 5;
  par->chamfercoef_max = 10;

}





static void WriteMatrice( char *name, double *mat )
{
  FILE *fopen(), *fp;
  
  fp = fopen( name, "w" );
  if ( fp == NULL ) {
    fprintf( stderr, "unable to open '%s' for writing\n", name );
    return;
  }
  fprintf( fp, "(\nO8\n" );
  fprintf( fp, "%f %f %f %f\n", mat[ 0], mat[ 1], mat[ 2], mat[ 3] );
  fprintf( fp, "%f %f %f %f\n", mat[ 4], mat[ 5], mat[ 6], mat[ 7] );
  fprintf( fp, "%f %f %f %f\n", mat[ 8], mat[ 9], mat[10], mat[11] );
  fprintf( fp, "%f %f %f %f\n", mat[12], mat[13], mat[14], mat[15] );
  fprintf( fp, ")\n" );
  fclose( fp );
}




static int ReadMatrice( char *name, double *mat )
{
  char *proc = "ReadMatrice";
  FILE *fopen(), *fp;
  char text[STRINGLENGTH];
  int nbelts = 0;
  int i, status;
  
  /* lecture de 4 double par ligne
     On prevoit le cas ou la ligne commence par "O8 xxxxx ...
     */

  fp = fopen( name, "r" );
  if ( fp == NULL ) {
     if ( _VT_DEBUG_ ||  _VT_VERBOSE_ )
       fprintf( stderr, "%s: error when opening file '%s'\n", proc, name );
    return( 0 );
  }
  
  i = 0;
  while ( (nbelts < 16) && (fgets( text, STRINGLENGTH, fp ) != NULL) ) {
    if ( 0 && _VT_DEBUG_ ) {
      fprintf( stderr, "parsing line #%02d: '%s'\n", i, text );
    }
    if ( (text[0] == 'O') && (text[1] == '8') ) {
      status = sscanf( &(text[2]), "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    } else {
      status = sscanf( text, "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    }
    if ( status == 4 ) nbelts += 4;
    i ++;
  }
  fclose( fp );

  if ( _VT_DEBUG_ == 1 ) {
    fprintf( stderr, " lecture de la matrice %s\n", name );
    fprintf( stderr, " %d elements lus\n", nbelts );
    fprintf( stderr,"   %f %f %f %f\n", mat[0], mat[1], mat[2], mat[3] );
    fprintf( stderr,"   %f %f %f %f\n", mat[4], mat[5], mat[6], mat[7] );
    fprintf( stderr,"   %f %f %f %f\n", mat[8], mat[9], mat[10], mat[11] );
    fprintf( stderr,"   %f %f %f %f\n", mat[12], mat[13], mat[14], mat[15] );
  }
  if ( nbelts == 16 ) return ( 1 );
  return( 0 );
}




static void MatriceFromRealToVoxels( double *mat, double *sizeIn, double *sizeOut )
{
  int i, j;

  /* la matrice MAT va a priori de l'image-out vers l'image-in
     la nouvelle matrice va donc etre composee par
     MAT' = H^(-1)(in) * MAT * H(out)
     ou H(in) et H(out) sont les matrices diagonales 
     contenant les tailles des voxels.
   */
  
  for ( i=0; i<4; i++ )
    for ( j=0; j<3; j++)
      mat[4*i + j] *= sizeOut[j];
  
  for ( j=0; j<3; j++)
    for ( i=0; i<4; i++ ) 
      mat[4*j + i] /= sizeIn[j];


  if ( _VT_DEBUG_ == 1 ) {
    fprintf( stderr, " Matrice transformee\n"  );
    fprintf( stderr,"   %f %f %f %f\n", mat[0], mat[1], mat[2], mat[3] );
    fprintf( stderr,"   %f %f %f %f\n", mat[4], mat[5], mat[6], mat[7] );
    fprintf( stderr,"   %f %f %f %f\n", mat[8], mat[9], mat[10], mat[11] );
    fprintf( stderr,"   %f %f %f %f\n", mat[12], mat[13], mat[14], mat[15] );
  }

}









#define TINY 1e-12


static int InverseMat4x4( double *matrice, double *inv )
{
  register int i, j, k;
  int kmax, rang = 4;
  register double c, max;
  double mat [16];
  
  for (i=0; i<16; i++ ) {
    mat[i] = matrice[i] ;
    inv[i] = 0.0;
  }
  inv[0] = inv[5] = inv[10] = inv[15] = 1.0;
  
  for ( j=0; j<4; j++ ) {
    if ( (mat[j*4+j] > (-TINY)) && (mat[j*4+j] < TINY) ) {
      /* recherche du plus grand element non nul sur la colonne j */
      kmax = j;
      max = 0.0;
      for (k=j+1; k<4; k++ ) {
	c = ( mat[k*4+j] > 0.0 ) ? mat[k*4+j] : (-mat[k*4+j]) ;
	if ( (c > TINY) && (c > max) ) { max = c; kmax = k; }
      }
      if ( kmax == j ) {
	/* la ligne est nulle */
	rang --;
      } else {
	/* sinon, on additionne */
	for ( i=0; i<4; i++ ) {
	  mat[j*4+i] += mat[kmax*4+i];
	  inv[j*4+i] += inv[kmax*4+i];
	}
      }
    }
    if ( (mat[j*4+j] < (-TINY)) || (mat[j*4+j] > TINY) ) {
      /* les autres lignes */
      for (k=0; k<4; k++) {
	if ( k != j ) {
	  c = mat[k*4 + j] / mat[j*4 + j];
	  for ( i=0; i<4; i++ ) {
	    mat[k*4 + i] -= c * mat[j*4 + i];
	    inv[k*4 + i] -= c * inv[j*4 + i];
	  }
	}
      }
      /* la ligne */
      c = mat[j*4 + j];
      for ( i=0; i<4; i++ ) {
	mat[j*4 + i] /= c;
	inv[j*4 + i] /= c;
      }
    }
  }

  return( rang );
}










static int _ChangeDistanceAtBordersAlongXY( vt_image *imdist )
{
  char *proc = "_ChangeDistanceAtBordersAlongXY";
  int x, y, z;
  int *min = (int*)NULL;
  int *max = (int*)NULL;
  int *val = (int*)NULL;

  min = (int *)malloc( imdist->dim.z * 3 * sizeof(int) );
  if ( min == (int*)NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }
  max = min + imdist->dim.z;
  val = min + 2*imdist->dim.z;

  switch( imdist->type ) {

  case SSHORT :
    {
      s16 ***theBuf = (s16***)(imdist->array);
      for ( z=0; z<(int)imdist->dim.z; z ++ ) {
	min[z] = max[z] = theBuf[z][0][0];
  for ( y=0; y<(int)imdist->dim.y; y ++ )
        for ( x=0; x<(int)imdist->dim.x; x ++ ) {
	  if ( min[z] > theBuf[z][y][x] ) min[z] = theBuf[z][y][x];
	  if ( max[z] < theBuf[z][y][x] ) max[z] = theBuf[z][y][x];
	}
      }
    }
    break;

  default :
    free( min );
    fprintf( stderr, "%s: type not handled in switch\n", proc );
    return( -1 );
  }


  /* zero-crossings should be approximately in the middle between 2 slices
     (negative distances are inside the object)
   */
  for ( z=0; z<(int)imdist->dim.z; z ++ ) {
    if ( min[z] == max[z] ) {
      val[z] = min[z];
      if ((z>0 && min[z-1] != max[z-1]) && (z<(int)imdist->dim.z-1 && min[z+1] != max[z+1])) {
	val[z] = (-min[z-1] -min[z+1])/2;
      }
      else if (z>0 && min[z-1] != max[z-1]) {
	val[z] = (-min[z-1]);
      }
      else if (z<(int)imdist->dim.z-1 && min[z+1] != max[z+1]) {
	val[z] = (-min[z+1]);
      }
    }
  }


  if ( 0 ) {
    for ( z=0; z<(int)imdist->dim.z; z ++ ) {
      fprintf( stdout, "slice %3d: min=%6d max=%6d", z, min[z], max[z] );
      if ( min[z] == max[z] )
	fprintf( stdout, " -> val=%6d", val[z] );
      fprintf( stdout, "\n" );
    }
  }

  switch( imdist->type ) {

  case SSHORT :
    {
      s16 ***theBuf = (s16***)(imdist->array);
      for ( z=0; z<(int)imdist->dim.z; z ++ ) {
	if ( min[z] == max[z] && val[z] != min[z] ) {
    for ( y=0; y<(int)imdist->dim.y; y ++ )
    for ( x=0; x<(int)imdist->dim.x; x ++ ) {
	    theBuf[z][y][x] = val[z];
	  }
	}
      }
    }
    break;

  default :
    free( min );
    fprintf( stderr, "%s: type not handled in switch\n", proc );
    return( -1 );
  }


  free( min );
  return( 1 );
}



static int _ComputeChamferWeights( typeChamferMask *theMaskXY, 
				   typeChamferMask *theMaskZX, 
				   typeChamferMask *theMaskYZ, 
				   double *original_voxel_size,
				   int size,
				   int dim,
				   int maxvalueofmincoefficient )
{
  char *proc = "_ComputeChamferWeights";
  VectorList vl;
  ConeList cl;
  WeightsList wl;

  int anisotropy_y = 0;
  int anisotropy_z = 0;
  double voxel_size[3];

  int minvalueofmincoefficient = 1;
  
  int mask_size = size;
  
  int x, y, z, i, n;
  int dims[3], c[3];
  int *btxt = NULL;



  /* inspired from test-gener-mask.c
   */

  _init_vector_list( &vl );
  _init_cone_list( &cl );


  /* anisotropies
   */
  voxel_size[0] = original_voxel_size[0];
  voxel_size[1] = original_voxel_size[1];
  voxel_size[2] = original_voxel_size[2];

  voxel_size[0] = 1.0;
  voxel_size[1] /= original_voxel_size[0];
  if ( voxel_size[1] < 0.99 || voxel_size[1] > 1.01 )
    anisotropy_y = 1;
  if ( dim == 3 ) {
    voxel_size[2] /= original_voxel_size[0];
    if ( voxel_size[2] < 0.99 || voxel_size[2] > 1.01 )
      anisotropy_z = 1;
  }

  /* Mask Initialization
   */
  switch( dim ) {
  case 2 :
  default :
    _init_2D_mask( &vl, &cl, voxel_size );
    break;
  case 3 :
    _init_3D_mask( stdout, &vl, &cl, voxel_size ); 
    break;
  }

  switch( dim ) {
  case 2 :
  default :
    _build_2D_mask( &vl, &cl, voxel_size, mask_size );
    break;
  case 3 :
    _build_3D_mask( stdout, &vl, &cl, voxel_size, mask_size, (int)0 );
    break;
  }
  
  switch( dim ) {
  case 2 :
  default :
    _symmetrise_2D_mask( &vl, &cl, voxel_size, anisotropy_y );
    break;
  case 3 :
    _symmetrise_3D_mask( stdout, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );
    break;
  }

  
  /* Print Stuff
   */
  switch( dim ) {
  case 2 :
  default :
     if ( _VT_VERBOSE_ )
       _print_2D_vector_list( stdout, &vl );
    break;
  case 3 :
     if ( _VT_VERBOSE_ )
       _print_3D_vector_list( stdout, &vl );
    _check_3D_mask_naturalconvexity( stdout, &vl, &cl, voxel_size );
    break;
  }
  
  switch( dim ) {
  case 2 :
  default :
    break;
  case 3 :
    _check_3D_mask_fareyregularity( stdout, &vl, &cl );
    break;
  }



  /* compute weights
   */

  _init_weights_list( &wl );
  wl.error_optimal = cl.allocatedCones[0].error;
  
  switch( dim ) {
  case 2 :
  default :
    (void) _compute_2D_weights( stdout, &wl, &vl, &cl, minvalueofmincoefficient, maxvalueofmincoefficient );
    break;
  case 3 :
    (void) _compute_3D_weights( stdout, &wl, &vl, &cl, voxel_size, minvalueofmincoefficient, maxvalueofmincoefficient );
    break;
  }

  if ( wl.n_weights > 0 ) {

    if ( _VT_VERBOSE_ >= 1 )
      _print_weights_list( stdout, &wl, &vl );

    /* masks extraction
     */    

    for ( i=0; i<vl.n_vectors; i++ )
      vl.allocatedVectors[i].w = wl.allocatedWeights[wl.n_weights - 1].w[i];
    btxt = _build_buffer_mask( dims, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );

    c[0] = (dims[0]-1)/2;
    c[1] = (dims[1]-1)/2;
    c[2] = (dims[2]-1)/2;

    if ( dims[2] == 1 ) {
      
      if ( allocChamferMask( theMaskXY, dims[0] * dims[1] ) <= 0 ) {
	fprintf( stderr, "%s: error in allocation\n", proc );
	_free_cone_list( &cl );
	_free_vector_list( &vl );
	_free_weights_list( &wl );
	return( -1 );
      }
      if ( allocChamferMask( theMaskYZ, dims[0] * dims[1] ) <= 0 ) {
	fprintf( stderr, "%s: error in allocation\n", proc );
	freeChamferMask( theMaskXY );
	_free_cone_list( &cl );
	_free_vector_list( &vl );
	_free_weights_list( &wl );
	return( -1 );
      }
      if ( allocChamferMask( theMaskZX, dims[0] * dims[1] ) <= 0 ) {
	fprintf( stderr, "%s: error in allocation\n", proc );
	freeChamferMask( theMaskXY );
	freeChamferMask( theMaskYZ );
	_free_cone_list( &cl );
	_free_vector_list( &vl );
	_free_weights_list( &wl );
	return( -1 );
      }
      
      n = 0;
      for ( y=0; y<dims[1]; y++ )
      for ( x=0; x<dims[0]; x++ ) {
	if ( btxt[y*dims[0] + x] > 0 ) {
	  theMaskXY->list[n].x = theMaskYZ->list[n].x = theMaskZX->list[n].x = x - c[0];
	  theMaskXY->list[n].y = theMaskYZ->list[n].y = theMaskZX->list[n].y = y - c[1];
	  theMaskXY->list[n].z = theMaskYZ->list[n].z = theMaskZX->list[n].z = 0;
	  theMaskXY->list[n].inc = theMaskYZ->list[n].inc = theMaskZX->list[n].inc = btxt[y*dims[0] + x];
	  n++;
	}
      }
      theMaskXY->nb = theMaskYZ->nb = theMaskZX->nb = n;
      
    }
    else {
      
      if ( allocChamferMask( theMaskXY, dims[0] * dims[1] ) <= 0 ) {
	fprintf( stderr, "%s: error in allocation\n", proc );
	_free_cone_list( &cl );
	_free_vector_list( &vl );
	_free_weights_list( &wl );
	return( -1 );
      }
      if ( allocChamferMask( theMaskYZ, dims[1] * dims[2] ) <= 0 ) {
	fprintf( stderr, "%s: error in allocation\n", proc );
	freeChamferMask( theMaskXY );
	_free_cone_list( &cl );
	_free_vector_list( &vl );
	_free_weights_list( &wl );
	return( -1 );
      }
      if ( allocChamferMask( theMaskZX, dims[2] * dims[0] ) <= 0 ) {
	fprintf( stderr, "%s: error in allocation\n", proc );
	freeChamferMask( theMaskXY );
	freeChamferMask( theMaskYZ );
	_free_cone_list( &cl );
	_free_vector_list( &vl );
	_free_weights_list( &wl );
	return( -1 );
      }
      
      n = 0;
      for ( y=0; y<dims[1]; y++ )
      for ( x=0; x<dims[0]; x++ ) {
	if ( btxt[c[2]*dims[0]*dims[1]+y*dims[0] + x] > 0 ) {
	  theMaskXY->list[n].x = x - c[0];
	  theMaskXY->list[n].y = y - c[1];
	  theMaskXY->list[n].z = 0;
	  theMaskXY->list[n].inc = btxt[c[2]*dims[0]*dims[1]+y*dims[0] + x];
	  n++;
	}
      }
      theMaskXY->nb = n;

      n = 0;
      for ( y=0; y<dims[1]; y++ )
      for ( z=0; z<dims[2]; z++ ) {
	if ( btxt[z*dims[0]*dims[1]+y*dims[0] + c[0]] > 0 ) {
	  theMaskYZ->list[n].x = y - c[1];
	  theMaskYZ->list[n].y = z - c[2];
	  theMaskYZ->list[n].z = 0;
	  theMaskYZ->list[n].inc = btxt[z*dims[0]*dims[1]+y*dims[0] + c[0]];
	  n++;
	}
      }
      theMaskYZ->nb = n;

      n = 0;
      for ( z=0; z<dims[2]; z++ )
      for ( x=0; x<dims[0]; x++ ) {
	if ( btxt[z*dims[0]*dims[1]+c[1]*dims[0] + x] > 0 ) {
	  theMaskZX->list[n].x = z - c[2];
	  theMaskZX->list[n].y = x - c[0];
	  theMaskZX->list[n].z = 0;
	  theMaskZX->list[n].inc = btxt[z*dims[0]*dims[1]+c[1]*dims[0] + x];
	  n++;
	}
      }
      theMaskZX->nb = n;


    }

    if ( btxt != NULL ) free( btxt );

  }

  _free_cone_list( &cl );
  _free_vector_list( &vl );
  _free_weights_list( &wl );

  return( 1 );

}
