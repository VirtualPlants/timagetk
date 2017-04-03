/*************************************************************************
 * reech3d.c -
 *
 * $Id: reech3d.c,v 1.10 2006/04/14 08:37:00 greg Exp $
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vt_common.h>
#include <vt_reech.h>

#define MODE_RESIZE  0
#define MODE_MATRICE 1
#define MODE_DEFBACK 2
#define MODE_ISOTROPIC 3

#define INTR_LINEAR  0
#define INTR_NEAREST 1
#define INTR_CSPLINE 2

typedef struct local_par {
  vt_names names;
  vt_ipt newdim;
  double voxel_size_in[3];
  double voxel_size_out[3];
  double voxel_isotropic;
  char matrice[STRINGLENGTH];
  char matrice_before[STRINGLENGTH];
  char matrice_out[STRINGLENGTH];
  char imagetemplate[STRINGLENGTH];
  int inverse;
  int inverse_before;
  int mode;
  int interp;
  char deform[STRINGLENGTH];
  int bias_gain;
  double bias;
  double gain;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );
static void WriteMatrice( char *name, double *mat );
static int  ReadMatrice( char *name, double *mat );
static void MatriceFromRealToVoxels( double *mat, double *sizeIn, double *sizeOut );
static int  InverseMat4x4( double *matrice, double *inv );



static char *usage = "[image-in] [image-out]\n\
\t [-x %d] [-y %d] [-z %d] [-mat %s] [-mat-bef %s] \n\
\t [-def %s] [-gain %f] [-bias %f]] [-dims %s]\n\
\t [-nearest | -linear | -cspline]\n\
\t [-iso %lf] [-omat|-out-matrice %s]\n\
\t [-vin %f %f %f] [-vout %f %f %f]\n\
\t [-inv] [-inv-bef] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t [-x|-tx] %d : dimension selon X de 'image-out'\n\
\t [-y|-ty] %d : dimension selon Y de 'image-out'\n\
\t [-z|-tz] %d : dimension selon Z de 'image-out'\n\
\t si une dimension n'est pas donnee, on prend celle de 'image-in'.\n\
\t si juste les dimensions sont donnees, on 'resize' l'image.\n\
\t -mat %s : matrice 4x4 de passage de 'image-out' a 'image-in'.\n\
\t           M(in) = MAT * M(out).\n\
\t -mat-bef %s: autre matrice 4x4, dans le cas du champ de deformation,\n\
\t           qui sera appliquee AVANT le champ de deformation\n\
\t -def %s : champ de deformation de vecteurs v. \n\
\t        Il est applique avant la matrice passee par -mat: M(in) = MAT * ( M(out) +v )\n\
\t        mais apres la matrice passee par -mat-bef: M(in) = MAT * ( MATB*M(out) +v )\n\
\t        les images sont %s.[x,y,x].\n\
\t -gain %f, -bias %f : l'intensite est transformee par i*g+b.\n\
\t                      (seulement avec -def et -linear).\n\
\t -nearest : on prend l'intensite du voisin le plus proche.\n\
\t -linear : interpolation bi- ou tri-lineaire de l'intensite.\n\
\t -cspline : utilisation de splines cubiques\n\
\t -vin %f %f %f : taille des voxels de l'image d'entree 'image-in'\n\
\t -vout %f %f %f : taille des voxels de l'image resultat 'image-out'\n\
\t -inv : inverse la matrice donnee par -mat.\n\
\t -inv-bef : inverse la matrice donnee par -mat-bef.\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s : short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.10 $ $Date: 2006/04/14 08:37:00 $ $Author: greg $\n";

static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imres, *imvectdef, *imdef[3], *imtemplate;
  vt_ipt resdim;
  double mat[16];
  double *mat_b=NULL, mat_bef[16];
  int x, y, z, i;
  struct stat filestat;

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
  case MODE_DEFBACK :

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
    
    MatriceFromRealToVoxels( mat, par.voxel_size_in, par.voxel_size_out );

    if ( par.matrice_before[0] != '\0' ) {

      if ( ReadMatrice( par.matrice_before, mat_bef ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to read *before* matrice\n", 0);
      }
      if ( par.inverse_before != 0 ) {
	double tmp[16];
	int rang;
	if ( (rang=InverseMat4x4( mat_bef, tmp )) != 4 ) {
	  fprintf( stderr, "Warning: la matrice %s est de rang %d.\n",
		   par.matrice, rang );
	}
	for (i=0;i<16;i++) mat_bef[i] = tmp[i];
      } 
      mat_b = mat_bef;

    } else {
      for (i=0;i<16;i++) mat_bef[i] = 0.0;
      mat_bef[0] = mat_bef[5] = mat_bef[10] = mat_bef[15] = 1.0;
    }
    


    break;

  case MODE_ISOTROPIC :
    
    /* Along one dimension, the field of view is oldim*oldsize,
       thus we have olddim*oldsize=newdim*newsize, and then
       newdim = olddim*oldsize / newsize

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

  }



  VT_Image( &imres );

  if ( par.imagetemplate[0] != '\0' ) {
    imtemplate = _VT_Inrimage( par.imagetemplate );
    if ( imtemplate == (vt_image*)NULL ) 
      VT_ErrorParse("unable to read template image\n", 0);
    VT_InitFromImage( &imres, imtemplate, par.names.out, image->type );
    VT_FreeImage( imtemplate );
    VT_Free( (void**)&imtemplate );
    resdim.x = imres.dim.x;
    resdim.y = imres.dim.y;
    resdim.z = imres.dim.z;
  }
  else {
    VT_InitImage( &imres, par.names.out, resdim.x, resdim.y, resdim.z, image->type );
    imres.siz.x = (float)sqrt( mat[0]*image->siz.x * mat[0]*image->siz.x +
			mat[4]*image->siz.y * mat[4]*image->siz.y +
			mat[8]*image->siz.z * mat[8]*image->siz.z );
    imres.siz.y = (float)sqrt( mat[1]*image->siz.x * mat[1]*image->siz.x +
			mat[5]*image->siz.y * mat[5]*image->siz.y +
			mat[9]*image->siz.z * mat[9]*image->siz.z );
    imres.siz.z = (float)sqrt( mat[2]*image->siz.x * mat[2]*image->siz.x +
			mat[6]*image->siz.y * mat[6]*image->siz.y +
			mat[10]*image->siz.z * mat[10]*image->siz.z );
  }
   
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
 







  /* on calcule la transformation */
  switch ( par.mode ) {
  case MODE_DEFBACK :
    
    imdef[0] = imdef[1] = imdef[2] = (vt_image *)NULL;

    /* check whether the par.deform image exists
       if yes, this should be a vectorial image */
    
    fprintf( stderr, "stat of %s is %d\n", par.deform, stat( par.deform, &filestat ) );

    if ( stat( par.deform, &filestat ) == 0 ) {

      imvectdef = _VT_Inrimage( par.deform );

      if ( imvectdef == (vt_image*)NULL ) {
	VT_FreeImage( image );
	VT_FreeImage( &imres );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to read deformation image\n", 0);
      }

      if ( imvectdef->dim.v != 3 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_FreeImage( &imres );
	VT_FreeImage( imvectdef );
	VT_Free( (void**)&imvectdef );
	VT_ErrorParse("deformation image should be vectorial\n", 0);
      }

      imdef[0] = (vt_image*)VT_Malloc( (unsigned)sizeof(vt_image) );
      imdef[1] = (vt_image*)VT_Malloc( (unsigned)sizeof(vt_image) );
      imdef[2] = (vt_image*)VT_Malloc( (unsigned)sizeof(vt_image) );
      VT_InitFromImage( imdef[0], imvectdef, "deformation.x", imvectdef->type );
      VT_InitFromImage( imdef[1], imvectdef, "deformation.y", imvectdef->type );
      VT_InitFromImage( imdef[2], imvectdef, "deformation.z", imvectdef->type );
      imdef[0]->dim.v = 1;
      imdef[1]->dim.v = 1;
      imdef[2]->dim.v = 1;

      if ( VT_AllocImage( imdef[0] ) <= 0 ) {
	VT_FreeImage( imvectdef );
	VT_Free( (void**)&imvectdef );
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate X deformation image\n", 0);
      }
      if ( VT_AllocImage( imdef[1] ) <= 0 ) {
	VT_FreeImage( imdef[0] );
	VT_Free( (void**)imdef[0] );
	VT_FreeImage( imvectdef );
	VT_Free( (void**)&imvectdef );
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate Y deformation image\n", 0);
      }
      if ( VT_AllocImage( imdef[2] ) <= 0 ) {
	VT_FreeImage( imdef[1] );
	VT_FreeImage( imdef[0] );
	VT_Free( (void**)imdef[1] );
	VT_Free( (void**)imdef[0] );
	VT_FreeImage( imvectdef );
	VT_Free( (void**)&imvectdef );
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate Z deformation image\n", 0);
      }

      /* copy deformation field
       */
      switch ( imvectdef->type ) {
      default :
	VT_FreeImage( imdef[2] );
	VT_FreeImage( imdef[1] );
	VT_FreeImage( imdef[0] );
	VT_Free( (void**)imdef[2] );
	VT_Free( (void**)imdef[1] );
	VT_Free( (void**)imdef[0] );
	VT_FreeImage( imvectdef );
	VT_Free( (void**)&imvectdef );
	VT_FreeImage( &imres );
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("such deformation image type not handled yet\n", 0);
      case FLOAT :
	{
	  r32 ***theBuf = (r32***)imvectdef->array;
	  r32 ***xBuf = (r32***)imdef[0]->array;
	  r32 ***yBuf = (r32***)imdef[1]->array;
	  r32 ***zBuf = (r32***)imdef[2]->array;
    for ( z=0; z<(int)imvectdef->dim.z; z++ )
    for ( y=0; y<(int)imvectdef->dim.y; y++ )
    for ( x=0; x<(int)imvectdef->dim.x; x++ ) {
	    /* we divide by voxel size to get voxel based transformation
	     */
	    if ( 0 ) {
	      xBuf[z][y][x] = theBuf[z][y][3*x]   / imvectdef->siz.x;
	      yBuf[z][y][x] = theBuf[z][y][3*x+1] / imvectdef->siz.y;
	      zBuf[z][y][x] = theBuf[z][y][3*x+2] / imvectdef->siz.z;
	    }
	    else {
	      xBuf[z][y][x] = theBuf[z][y][3*x];
	      yBuf[z][y][x] = theBuf[z][y][3*x+1];
	      zBuf[z][y][x] = theBuf[z][y][3*x+2];
	    }
	  }
	}
	break;
      case DOUBLE :
	{
	  r64 ***theBuf = (r64***)imvectdef->array;
	  r64 ***xBuf = (r64***)imdef[0]->array;
	  r64 ***yBuf = (r64***)imdef[1]->array;
	  r64 ***zBuf = (r64***)imdef[2]->array;
    for ( z=0; z<(int)imvectdef->dim.z; z++ )
    for ( y=0; y<(int)imvectdef->dim.y; y++ )
    for ( x=0; x<(int)imvectdef->dim.x; x++ ) {
	    /* we divide by voxel size to get voxel based transformation
	     */
	    if ( 0 ) {
	      xBuf[z][y][x] = theBuf[z][y][3*x]   / imvectdef->siz.x;
	      yBuf[z][y][x] = theBuf[z][y][3*x+1] / imvectdef->siz.y;
	      zBuf[z][y][x] = theBuf[z][y][3*x+2] / imvectdef->siz.z;
	    }
	    else {
	      xBuf[z][y][x] = theBuf[z][y][3*x];
	      yBuf[z][y][x] = theBuf[z][y][3*x+1];
	      zBuf[z][y][x] = theBuf[z][y][3*x+2];
	    }
	  }
	}
	break;
      }

      VT_FreeImage( imvectdef );
      VT_Free( (void**)&imvectdef );
    
    }
    else {
      char name[STRINGLENGTH];
      strcat ( strcpy( name, par.deform ), ".x" );
      imdef[0] = _VT_Inrimage( name );
      if ( imdef[0] == (vt_image*)NULL ) {
	VT_FreeImage( image );
	VT_FreeImage( &imres );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to read X deformation image\n", 0);
      }
      strcat ( strcpy( name, par.deform ), ".y" );
      imdef[1] = _VT_Inrimage( name );
      if ( imdef[1] == (vt_image*)NULL ) {
	VT_FreeImage( image );
	VT_FreeImage( &imres );
	VT_Free( (void**)&image );
	VT_FreeImage( imdef[0] );
	VT_Free( (void**)&imdef[0] );
	VT_ErrorParse("unable to read Y deformation image\n", 0);
      }
      strcat ( strcpy( name, par.deform ), ".z" );
      imdef[2] = _VT_Inrimage( name );
      if ( imdef[2] == (vt_image*)NULL ) {
	VT_FreeImage( image );
	VT_FreeImage( &imres );
	VT_Free( (void**)&image );
	VT_FreeImage( imdef[0] );
	VT_Free( (void**)&imdef[0] );
	VT_FreeImage( imdef[1] );
	VT_Free( (void**)&imdef[1] );
	VT_ErrorParse("unable to read Z deformation image\n", 0);
      }
    }
  
    switch ( par.interp ) {
      case INTR_NEAREST :
	if ( Reech3DNearestDefBack( image, &imres, imdef,
				   mat, mat_b ) != 1 ) {
	  VT_FreeImage( image );
	  VT_FreeImage( &imres );
	  VT_Free( (void**)&image );
	  VT_FreeImage( imdef[0] );
	  VT_Free( (void**)&imdef[0] );
	  VT_FreeImage( imdef[1] );
	  VT_Free( (void**)&imdef[1] );
	  VT_FreeImage( imdef[2] );
	  VT_Free( (void**)&imdef[2] );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
    default :
      fprintf( stderr, "... switch to linear interpolation\n" );
    case INTR_LINEAR :
      if ( Reech3DTriLinDefBack( image, &imres, imdef,
				   mat, mat_b, par.gain, par.bias ) != 1 ) {
	  VT_FreeImage( image );
	  VT_FreeImage( &imres );
	  VT_Free( (void**)&image );
	  VT_FreeImage( imdef[0] );
	  VT_Free( (void**)&imdef[0] );
	  VT_FreeImage( imdef[1] );
	  VT_Free( (void**)&imdef[1] );
	  VT_FreeImage( imdef[2] );
	  VT_Free( (void**)&imdef[2] );
	  VT_ErrorParse("unable to compute result\n", 0);
      }
    }
    VT_FreeImage( imdef[0] );
    VT_Free( (void**)&imdef[0] );
    VT_FreeImage( imdef[1] );
    VT_Free( (void**)&imdef[1] );
    VT_FreeImage( imdef[2] );
    VT_Free( (void**)&imdef[2] );
    break;

  case MODE_MATRICE :
  case MODE_ISOTROPIC :
  case MODE_RESIZE :
  default :
    if ( imres.dim.z == 1 && image->dim.z == 1 ) {
      switch ( par.interp ) {
      case INTR_NEAREST :
	if ( Reech2DNearest4x4( image, &imres, mat ) != 1 ) {
	  VT_FreeImage( image );
	  VT_FreeImage( &imres );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
      case INTR_CSPLINE :
	if ( Reech3DCSpline4x4( image, &imres, mat ) != 1 ) {
	  VT_FreeImage( image );
	  VT_FreeImage( &imres );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
      default :
      case INTR_LINEAR :
	if ( par.bias_gain != 0 ) {
	  if ( Reech2DTriLin4x4gb( image, &imres, mat, (float)par.gain, (float)par.bias ) != 1 ) {
	    VT_FreeImage( image );
	    VT_FreeImage( &imres );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute result\n", 0);
	  }
	} else {
	  if ( Reech2DTriLin4x4( image, &imres, mat ) != 1 ) {
	    VT_FreeImage( image );
	    VT_FreeImage( &imres );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute result\n", 0);
	  }
	}
      }
    } else {
      switch ( par.interp ) {
      case INTR_NEAREST :
	if ( Reech3DNearest4x4( image, &imres, mat ) != 1 ) {
	  VT_FreeImage( image );
	  VT_FreeImage( &imres );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
      case INTR_CSPLINE :
	if ( Reech3DCSpline4x4( image, &imres, mat ) != 1 ) {
	  VT_FreeImage( image );
	  VT_FreeImage( &imres );
	  VT_Free( (void**)&image );
	  VT_ErrorParse("unable to compute result\n", 0);
	}
	break;
      case INTR_LINEAR :
      default :
	if ( par.bias_gain != 0 ) {
	  if ( Reech3DTriLin4x4gb( image, &imres, mat, (float)par.gain, (float)par.bias ) != 1 ) {
	    VT_FreeImage( image );
	    VT_FreeImage( &imres );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute result\n", 0);
	  }
	} else {
	  if ( Reech3DTriLin4x4( image, &imres, mat ) != 1 ) {
	    VT_FreeImage( image );
	    VT_FreeImage( &imres );
	    VT_Free( (void**)&image );
	    VT_ErrorParse("unable to compute result\n", 0);
	  }
	}
      }
    }

    if ( par.matrice_out[0] != '\0' ) WriteMatrice( par.matrice_out, mat );

  }
  
  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( image );
    VT_FreeImage( &imres );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( image );
  VT_FreeImage( &imres );
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
	_VT_DEBUG_ = 1;
      }
      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }
      /*--- matrice ---*/
      else if ( strcmp ( argv[i], "-mat" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -mat...\n", 0 );
	strncpy( par->matrice, argv[i], STRINGLENGTH );  
	if ( par->mode != MODE_DEFBACK ) par->mode = MODE_MATRICE;
      }

       else if ( strcmp ( argv[i], "-mat-bef" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -mat-bef...\n", 0 );
	strncpy( par->matrice_before, argv[i], STRINGLENGTH );  
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
      else if ( strcmp ( argv[i], "-inv-bef" ) == 0 ) {
	par->inverse_before = 1;
      }


      else if ( strcmp ( argv[i], "-dims" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -dims...\n", 0 );
	strncpy( par->imagetemplate, argv[i], STRINGLENGTH );  
      }



      /*--- type d'interpolation ---*/
      else if ( strcmp ( argv[i], "-near" ) == 0 ) {
	par->interp = INTR_NEAREST;
      }
      else if ( strcmp ( argv[i], "-nearest" ) == 0 ) {
	par->interp = INTR_NEAREST;
      }
      else if ( strcmp ( argv[i], "-linear" ) == 0 ) {
	par->interp = INTR_LINEAR;
      }
      else if ( strcmp ( argv[i], "-cspline" ) == 0 ) {
	par->interp = INTR_CSPLINE;
      }

      /*--- deformation, bias et gain ---*/
      else if ( strcmp ( argv[i], "-def" ) == 0 ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -def...\n", 0 );
	strncpy( par->deform, argv[i], STRINGLENGTH );  
	par->mode = MODE_DEFBACK;
      }
      else if ( strcmp ( argv[i], "-bias" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -bias...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->bias) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -bias...\n", 0 );
	par->bias_gain = 1;
      }
      else if ( strcmp ( argv[i], "-gain" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -gain...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->gain) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -gain...\n", 0 );
	par->bias_gain = 1;
      }
      
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


      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
	_VT_VERBOSE_ = 1;
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
  par->matrice[0] ='\0';
  par->matrice_before[0] ='\0';
  par->matrice_out[0] ='\0';
  par->imagetemplate[0] ='\0';
  par->inverse = 0;
  par->inverse_before = 0;
  par->mode = MODE_RESIZE;
  par->interp = INTR_LINEAR;
  par->bias_gain = 0;
  par->bias = 0.0;
  par->gain = 1.0;
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
  FILE *fopen(), *fp;
  char text[STRINGLENGTH];
  int i, nbelts = 0;
  int status;
  
  /* lecture de 4 double par ligne
     On prevoit le cas ou la ligne commence par "O8 xxxxx ...
     */

  fp = fopen( name, "r" );
  if ( fp == NULL ) return( 0 );
  
  while ( (nbelts < 16) && (fgets( text, STRINGLENGTH, fp ) != NULL) ) {
    if ( (text[0] == 'O') && (text[1] == '8') ) {
      status = sscanf( &(text[2]), "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    } else {
      status = sscanf( text, "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    }
    if ( _VT_DEBUG_ ) {
      fprintf( stderr, "read %d elements:", status );
      for (i=0; i<status; i++ )
	fprintf( stderr, " %lf", mat[nbelts+i] );
      fprintf( stderr, "\n" );
    }
    if ( status == 4 ) nbelts += 4;
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
