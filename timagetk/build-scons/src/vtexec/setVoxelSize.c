/*************************************************************************
 * setVoxelSize.c -
 *
 * $$
 *
 * Copyright (c) INRIA 2009
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

#include <vt_common.h>


/*------- Definition des fonctions statiques ----------*/
static void VT_ErrorParse( char *str, int l );





static char *usage = "[image] [%lf %lf %lf]\n";

static char *detail = "\
set voxel size\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  vt_image *image;
  double vx = 1.0;
  double vy = 1.0;
  double vz = 1.0;
  
  if ( argc <= 1 ) {
    VT_ErrorParse("\n", 1);
  }

  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( argv[1] );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  if ( argc < 3 || sscanf( argv[2],"%lf",&vx ) != 1 ) {
    VT_ErrorParse("unable to parse voxel size along X\n", 0);
    VT_FreeImage( image );
    VT_Free( (void**)&image );  
  }

  if ( argc < 4 || sscanf( argv[3],"%lf",&vy ) != 1 ) {
    VT_ErrorParse("unable to parse voxel size along Y\n", 0);
    VT_FreeImage( image );
    VT_Free( (void**)&image );  
  }
  
  if ( argc >= 5 ) {
    if(  sscanf( argv[4],"%lf",&vz ) != 1 ) {
      VT_ErrorParse("unable to parse voxel size along Z\n", 0);
      VT_FreeImage( image );
      VT_Free( (void**)&image );  
    }
    if ( argc > 6 )
      fprintf( stderr, "%s: too many args ...\n", argv[0] );
  }
  
  

  image->siz.x = vx;
  image->siz.y = vy;
  image->siz.z = vz;
  
  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( image ) == -1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( image );
  VT_Free( (void**)&image );
  return( 1 );
}







static void VT_ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage : %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",detail);
  (void)fprintf(stderr,"Erreur : %s",str);
  exit(0);
}






