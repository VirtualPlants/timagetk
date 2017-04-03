/*************************************************************************
 * minimum.c -
 *
 * $Id: axeMedianOfComponents.c,v 1.1 2006/04/14 08:37:11 greg Exp $
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

#include <vt_common.h>
#include <vt_distance.h>

#include <topological-operations-common.h>
#include <topological-thinning.h>




/*------- Definition des fonctions statiques ----------*/
static void VT_ErrorParse( char *str, int l );





static char *usage = "[image-in] [image-out]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -inv : inverse 'image-in'\n\
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
 $Revision: 1.1 $ $Date: 2006/04/14 08:37:11 $ $Author: greg $\n";

static char program[STRINGLENGTH];


typedef enum {
  INPUT_FILE,
  OUTPUT_FILE,
  OPTION
} enumTypeArg;



int main( int argc, char *argv[] )
{
  char text[STRINGLENGTH];
  enumTypeArg *type_arg = NULL;
  int i, j;
  int nimages=0;
  char * name_output =NULL;

  vt_image *image, imres, imdist;
  vt_image *dimage;
  int *npt;
  vt_distance dpar;
  unsigned char ***theRes;
  unsigned short ****allDist;
  int min, b, x,y,z;
  int theDim[3];

  typeThinningParameters p;

  initTypeThinningParameters( &p );

  if ( VT_CopyName( program, argv[0] ) != 1 )
    VT_Error("Error while copying program name", (char*)NULL);
  
  if ( argc == 1 ) VT_ErrorParse("\n", 0 ); 
  
  type_arg = (enumTypeArg *)malloc( argc*sizeof( enumTypeArg ) );
  for ( i=1; i<argc; i++ ) {
    if ( argv[i][0] == '-' ) {
      type_arg[i] = OPTION;
      if ( strcmp ( argv[i], "-help" ) == 0 ) {
        VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
        _VT_VERBOSE_ = 1;
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 ) {
        _VT_DEBUG_ = 1;
      }
      else {
        sprintf(text,"unknown option %s\n",argv[i]);
        VT_ErrorParse(text, 0);
      }
    }
    else {
      type_arg[i] = INPUT_FILE;
    }
  }
   
  for ( i=argc-1; i>=1; i-- )
    if ( type_arg[i] == INPUT_FILE ) {
      type_arg[i] = OUTPUT_FILE;
      name_output = argv[i];
      i = 0;
    }
  for ( i=1; i< argc; i++ ) {
    if ( type_arg[i] == INPUT_FILE ) nimages ++;
  }
  

  if ( 0 ) {
    printf( "INPUTS (%d) = ", nimages );
    for ( i=1; i< argc; i++ ) {
      if ( type_arg[i] == INPUT_FILE ) 
        printf( "%s ", argv[i] );
      if ( type_arg[i] == OUTPUT_FILE ) {
        printf( "\n" );
        printf( "OUTPUT = " );
        printf( "%s\n", argv[i] );
      }
    }
  }
 

  /* calcul des distances
   */
  VT_Distance( &dpar );
  dpar.type = VT_DIST_8;
  dimage = (vt_image*)malloc( nimages*sizeof(vt_image) );
  allDist = (unsigned short ****)malloc( nimages*sizeof(unsigned short ***) );
  npt = (int *)malloc( nimages*sizeof(int));
  for ( j=0; j<nimages; j++ ) {
    npt[j] = 0;
  }

  type_arg[0] = OPTION;

  /*--- on calcule une carte de distance par image d'entree
    ---*/

  for ( j=0, i=1; i< argc; i++ ) {

    if ( type_arg[i] != INPUT_FILE ) continue;

    image = _VT_Inrimage( argv[i] );
    if ( image == (vt_image*)NULL ) {
      sprintf( text, "unable to read '%s' (input %d/%d)\n", argv[i], j+1, nimages );
      VT_ErrorParse( text, 0 );
    }
  
    VT_Image( &(dimage[j]) );
    sprintf( text, "%s.dist", argv[i] );
    VT_InitFromImage( &(dimage[j]), image, text, USHORT );
    if ( VT_AllocImage( &(dimage[j]) ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      sprintf( text, "unable to allocate '%s' (input %d/%d)\n", argv[i], j+1, nimages );
      VT_ErrorParse( text, 0 );
    }

    allDist[j]=(unsigned short ***)dimage[j].array;

    if ( VT_Dist( &(dimage[j]), image, &dpar ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to compute distance\n", 0);
    }

    if ( 0 ) VT_WriteInrimage( &(dimage[j]) );

    VT_FreeImage( image );
    VT_Free( (void**)&image );

    j++;
  }

  /*--- initialisation de l'image distance resultat ---*/
  VT_Image( &imres );
  VT_InitFromImage( &imres, &(dimage[0]), name_output, UCHAR );
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_ErrorParse("unable to allocate resultat image\n", 0);
  }
  theRes=(unsigned char ***)imres.array;

  /* on met a 0 dans theRes les points qui sont a l'interieur 
     d'une structure et qui sont a une distance de plus de 1 
     des autres structures.
     On compte les tels points (tableau npt)
  */

  for ( z=0; z<(int)imres.dim.z; z++ )
  for ( y=0; y<(int)imres.dim.y; y++ )
  for ( x=0; x<(int)imres.dim.x; x++ ) {

    theRes[z][y][x] = _CANBECHANGED_VALUE_;

    if ( allDist[0][z][y][x] == 0 ) {
      min = allDist[1][z][y][x];
      for ( j=2; j<nimages; j++ ) 
        if  ( min > allDist[j][z][y][x] ) min = allDist[j][z][y][x];
      if ( min > 1 ) {
        theRes[z][y][x] = _BACKGROUND_VALUE_;
        npt[0] ++;
      }
      continue;
    }
    else {
      min = allDist[0][z][y][x];
      for ( b=0, j=1; b==0 && j<nimages;j++ ) {
        if ( allDist[j][z][y][x] == 0 ) {
          for ( i=j+1; i<nimages; i++ )
            if  ( min > allDist[i][z][y][x] ) min = allDist[i][z][y][x];
          if ( min > 1 ) {
            theRes[z][y][x] = _BACKGROUND_VALUE_;
            npt[j] ++;
          }
          b = 1;
        }
        else {
          if ( min > allDist[j][z][y][x] ) min = allDist[j][z][y][x];
        }
      }
    }

  }


  for ( j=0; j<nimages; j++ ) {
    if ( npt[j] == 0 ) {
      for ( b=0, i=0; i<argc; i++ )
        if ( type_arg[i] == INPUT_FILE ) {
          if ( b == j ) {
            printf( "ATTENTION: pas de points extraits de '%s'\n", argv[i] );
          }
          b++;
        }
    }
  }


  for ( j=0; j<nimages; j++ )
    VT_FreeImage( &(dimage[j]) );
  free ( dimage );
  free ( allDist );
  free ( npt );



  theDim[0] = imres.dim.x;
  theDim[1] = imres.dim.y;
  theDim[2] = imres.dim.z;
  

  p.cyclesBeforeEnding = 4; /* cyclesBeforeEnding */
  p.valueBeforeEnding = 1; /* distanceBeforeEnding */
  p.typeThickness = _26THICKNESS_;
  p.typeEndPoint = _NO_END_POINT_;


  if ( chamferBasedThinning( (unsigned char *)imres.buf,
                               theDim,
                               3,   /* chamfer 3x3x3 */
                               &p ) != 1 ) {
    VT_ErrorParse("unable to thin image\n", 0);
  }

  for ( z=0; z<(int)imres.dim.z; z++ ) {
    for ( x=0; x<(int)imres.dim.x; x++ ) {
      theRes[z][0][x] = theRes[z][imres.dim.y-1][x] = 0;
    }
    for ( y=0; y<(int)imres.dim.y; y++ ) {
       theRes[z][y][0] = theRes[z][y][imres.dim.x-1] = 0;
    }
  }

  if ( 0 ) VT_WriteInrimage( &imres );

  dpar.type = VT_DIST_CHMFR;

  VT_Image( &imdist );
  VT_InitFromImage( &imdist, &imres, name_output, UCHAR );
  if ( VT_AllocImage( &imdist ) != 1 ) {
    VT_ErrorParse("unable to allocate distance image\n", 0);
  }


  if ( VT_Dist( &imdist, &imres, &dpar ) != 1 ) {
    VT_ErrorParse("unable to compute distance\n", 0);
  }

  VT_WriteInrimage( &imdist);

  return( 0 );
  

}





static void VT_ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage : %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",detail);
  (void)fprintf(stderr,"Erreur : %s",str);
  exit(0);
}

