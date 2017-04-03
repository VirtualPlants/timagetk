/*************************************************************************
 * parcellisation.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2008
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

#include <time.h>
#include <sys/time.h>

#include <chamferdistance-mask.h>
#include <chamferdistance.h>
#include <chunks.h>
#include <parcelling.h>

#include <vt_common.h>



typedef struct local_par {
  vt_names names;
  char seedfile[STRINGLENGTH];
  char seedout[STRINGLENGTH];
  char seedimg[STRINGLENGTH];
  int nparcels;
  int type;

  /* chamfer related options
   */
  /* calculated mask
   */
  int chamfercoef_dim;
  int chamfercoef_size;
  int chamfercoef_max;

  /* pre-calculated mask
   */
  enumDistance predefineddistance;

  /* misc
   */
  int printmask;
  int print_time;


} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );


static char *_BaseName( char *p );
static double _GetTime();
static double _GetClock();


static char *usage = "[image-in] [image-out]\n\
 [-dist %s] [-iterations|-i %d]\n\
 [-force] [-no-force]\n\
 [-seeds %s] [-sout %s] [-wi %s]\n\
 [-parcels|-p %d] [-random-seed|-rs %ld]\n\
 [-partial-update] [-no-partial-update]\n\
 [-chamfer-mask-size |-cm-size %d]\n\
 [-chamfer-mask-max |-cm-max %d]\n\
 [-predefined-chamfer-mask|-pcm 4|8|3x3|5x5|6|18|26|3x3x3|5x5x5]\n\
 [-connectivity|-con|-c %d]\n\
 [-print-mask]\n\
 [-inv] [-swap] [-v] [-nv] [-D] [-help] [options-de-type]\n\
 [-print-time|-time] [-notime]";

static char *detail = "\
 -parcels|-p %d: number of parcels\n\
 -dist %s: output distance image\n\
 -iterations|-i %d: maximal number of iterations\n\
 -force: force iterative computation of parcel center\n\
    default: if the barycenter is in the parcel, it is considered\n\
    as the geodesic barycenter\n\
 -no-force:\n\
 -seeds: list of seed points\n\
         each line is of the form 'X Y Z'\n\
 -sout %s: write list of seeds computed in %s\n\
 -random-seed %ld: specify random seed\n\
 -wi %s: seeds written in binary image file %s\n\
 [-partial-update]: do not consider unchanging parcels that are\n\
     not adjacent to a changing parcel\n\
 [-no-partial-update]: recompute all parcels at each iteration\n\
 [-chamfer-mask-size | -cm-size %d] # size of the chamfer mask\n\
               # 3 (default), 5, 7, ...\n\
 [-chamfer-mask-max | -cm-max  %d]  # maximum value of the minimum\n\
               # value of the chamfer mask. The larger this value, \n\
               # the longer the computation of the mask\n\
               # a rule of thumb is to specify the size of the mask\n\
 [-predefined-chamfer-mask | -pcm ... ] \n\
               # 4 or 8 : 2D adjacencies\n\
               # 6, 10, 18, or 26 : 3D adjacencies\n\
               # 3x3 : (3,4) mask\n\
               # 5x5 : (5,7,11) mask \n\
               # 3x3x3 : (3,4,5) mask \n\
               # 5x5x5 : (5,7,11,9,12,15) mask \n\
 [-connectivity|-con %d] # id. -predefined-chamfer-mask %d\n\
 [-chamfer %d] # id. -predefined-chamfer-mask %dx%dx%d\n\
 [-print-mask] # \n\
 si 'image-in' est '-', on prendra stdin\n\
 si 'image-out' est absent, on prendra stdout\n\
 si les deux sont absents, on prendra stdin et stdout\n\
 -inv : inverse 'image-in'\n\
 -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
 -v : mode verbose\n\
 -D : mode debug\n\
 options-de-type : -o 1    : unsigned char\n\
                   -o 2    : unsigned short int\n\
                   -o 2 -s : short int\n\
                   -o 4 -s : int\n\
                   -r      : float\n\
 si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.5 $ $Date: 2000/08/16 16:31:56 $ $Author: greg $\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image;
  vt_image imres;
  vt_image imdist;
  int theDim[3];

  FILE *fopen(),*fseeds;
  int n;
  int nallocated = 10000;
  int **theSeeds = NULL;
  int *seeds = NULL;
  int ret;
  int inSeeds = 0;

  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;

  typeChamferMask theMask;
  double voxel_size[3];

  /*--- initialisation des parametres ---*/
  _VT_VERBOSE_ = 1;
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );

  theDim[0] = image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;



  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  if ( par.nparcels < 256 && par.seedfile[0] == '\0')
    VT_InitFromImage( &imres, image, par.names.out, UCHAR );
  else 
    VT_InitFromImage( &imres, image, par.names.out, USHORT );

  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
  
  
  VT_Image( &imdist );
  if ( par.names.ext[0] != '\0' ) {
    VT_InitFromImage( &imdist, image, par.names.ext, USHORT );
    if ( VT_AllocImage( &imdist ) != 1 ) {
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to allocate distance image\n", 0);
    }
  }

  if ( par.seedfile[0] != '\0' ) {
    inSeeds = 1;
    fseeds = fopen( par.seedfile, "r" );
    if ( fseeds == NULL ) {
      if ( par.names.ext[0] != '\0' ) 
         VT_FreeImage( &imdist );
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("error when opening seeds file\n", 0);
    }
    
    seeds = (int*)malloc( 3*nallocated * sizeof( int ) );
    theSeeds = (int**)malloc( nallocated * sizeof( int* ) );
    if ( seeds == NULL || theSeeds == NULL ) {
      if ( seeds != NULL ) free( seeds );
      if ( theSeeds != NULL ) free( theSeeds );
      if ( par.names.ext[0] != '\0' ) 
         VT_FreeImage( &imdist );
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("error when allocating seeds arrays\n", 0);
    }

    for ( n=0;n<nallocated;n++ ) {
      theSeeds[n] = &(seeds[3*n]);
    }
    
    n = 0;
    while ( (ret = fscanf( fseeds, "%d %d %d\n", &theSeeds[n][0], &theSeeds[n][1], &theSeeds[n][2]) ) != EOF ) {
      if ( ret != 3 ) {
         free( seeds );
         free( theSeeds );
         if ( par.names.ext[0] != '\0' )
           VT_FreeImage( &imdist );
         VT_FreeImage( &imres );
         VT_FreeImage( image );
         VT_Free( (void**)&image );
         VT_ErrorParse("error in reading seeds file\n", 0);
      }
      n++;
    }
    fclose( fseeds );
    
    fprintf( stderr, "%s: read %d seeds in file '%s'\n", program, n, par.seedfile );
    
    if ( n == 0 ) {
      free( seeds );
      free( theSeeds );
      if ( par.names.ext[0] != '\0' ) 
         VT_FreeImage( &imdist );
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("empty seeds file ?\n", 0);
    }
    par.nparcels = n;
  }
  else {
    if( par.seedimg[0] != '\0' || par.seedout[0] != '\0') {
      seeds = (int*)malloc( 3*par.nparcels * sizeof( int ) );
      theSeeds = (int**)malloc( par.nparcels * sizeof( int* ) );
      if ( seeds == NULL || theSeeds == NULL ) {
        if ( seeds != NULL ) free( seeds );
        if ( theSeeds != NULL ) free( theSeeds );
        if ( par.names.ext[0] != '\0' )
          VT_FreeImage( &imdist );
        VT_FreeImage( &imres );
        VT_FreeImage( image );
        VT_Free( (void**)&image );
        VT_ErrorParse("error when allocating seeds arrays\n", 0);
      }

      for ( n=0;n<par.nparcels;n++ ) {
        theSeeds[n] = &(seeds[3*n]);
      }
    }
  }

  /* distance increments
   */
  if ( image->dim.z >= 2 && par.chamfercoef_dim == 0 ) {
    par.chamfercoef_dim = 3;
  }
  else if ( image->dim.z == 1 ) {
    par.chamfercoef_dim = 2;
  }

  initChamferMask( &theMask );

  voxel_size[0] = 1.0;
  voxel_size[1] = image->siz.y / image->siz.x;
  voxel_size[2] = image->siz.z / image->siz.x;

  if ( buildChamferMask( voxel_size, par.chamfercoef_dim, par.chamfercoef_size,
                            par.chamfercoef_max, par.predefineddistance, &theMask ) != 1 ) {
    freeChamferMask( &theMask );
    VT_FreeImage( &imdist );
    VT_FreeImage( &imres );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    if ( seeds != NULL ) free( seeds );
    if ( theSeeds != NULL ) free( theSeeds );
    VT_ErrorParse( "error when computing masks\n", 0);
  }

  if ( par.printmask ) {
    sortChamferMask( &theMask );
    printChamferMaskAsList( stdout, &theMask );
    printChamferMaskAsImage( stdout, &theMask );
  }

  if ( 1 ) {
    fprintf( stderr, "%s: random is %ld\n", _BaseName(argv[0]), getRandomSeedInParcelling() );
  }

  if ( par.names.ext[0] != '\0' ) {
    if ( parcelling( image->buf, image->type, 
                       theSeeds, par.nparcels,
                       imres.buf, imres.type,
                       imdist.buf, imdist.type,
                       theDim, inSeeds, &theMask  ) != 1 ) {
        freeChamferMask( &theMask );
        VT_FreeImage( &imdist );
        VT_FreeImage( &imres );
        VT_FreeImage( image );
        VT_Free( (void**)&image );
        if ( seeds != NULL ) free( seeds );
        if ( theSeeds != NULL ) free( theSeeds );
        VT_ErrorParse("error when processing\n", 0);
    }
  }
  else {
    if ( parcelling( image->buf, image->type, 
                     theSeeds, par.nparcels,
                       imres.buf, imres.type,
                       NULL, TYPE_UNKNOWN,
                       theDim, inSeeds, &theMask ) != 1 ) {
      freeChamferMask( &theMask );
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      if ( seeds != NULL ) free( seeds );
      if ( theSeeds != NULL ) free( theSeeds );
      VT_ErrorParse("error when processing\n", 0);
    }
  }

  freeChamferMask( &theMask );





  /*--- ecriture de l'image resultat ---*/


  /* faudrait ecrire les points 
   */

  if ( (seeds != NULL) || ( theSeeds != NULL )) {
    if ( par.seedout[0] != '\0') {
      fseeds = fopen( par.seedout, "w" );

      if ( fseeds == NULL ) {
        if ( par.names.ext[0] != '\0' )
          VT_FreeImage( &imdist );
        VT_FreeImage( &imres );
        VT_FreeImage( image );
        VT_Free( (void**)&image );
        free( seeds );
        free( theSeeds );
        VT_ErrorParse("error when writing seeds output file\n", 0);
      }

      for ( n=0; n<par.nparcels; n++ ) {
        fprintf( fseeds, "%d %d %d\n", theSeeds[n][0], theSeeds[n][1], theSeeds[n][2] );
      }
    }
    else {
        /* there is no need to write result seeds on stdout
         */
        if ( 0 ) {
            for ( n=0; n<par.nparcels; n++ ) {
              fprintf( stdout, "#%03d: %d %d %d\n", n, theSeeds[n][0], theSeeds[n][1], theSeeds[n][2] );
            }
      }
    }
    free( seeds );
    free( theSeeds );
  }

  if ( par.names.ext[0] != '\0' ) {
    if ( VT_WriteInrimage( &imdist ) == -1 ) {
      VT_FreeImage( &imdist );
      VT_FreeImage( &imres );
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to write output distance image\n", 0);
    }
    VT_FreeImage( &imdist );
  }

  if ( VT_WriteInrimage( &imres ) == -1 ) {
    VT_FreeImage( &imres );
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( &imres );
  VT_FreeImage( image );
  VT_Free( (void**)&image );

  time_exit = _GetTime();
  clock_exit = _GetClock();

  if ( par.print_time ) {
    fprintf( stderr, "%s: elapsed (real) time = %f\n", _BaseName( argv[0] ), time_exit - time_init );
    fprintf( stderr, "\t       elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
    fprintf( stderr, "\t       ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
  }

  return( 1 );
}








static void VT_Parse( int argc, 
                        char *argv[],
                        local_par *par )
{
  int i, nb, status;
  int o=0, s=0, r=0;
  int connectivity = 0;
  int chamfer = 0;
  int iterations;
  char text[STRINGLENGTH];
  long int randomseed;

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
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
         _VT_VERBOSE_ ++ ;
         incrementVerboseInParcelling();
      }
      else if ( strcmp ( argv[i], "-nv" ) == 0 ) {
         _VT_VERBOSE_ = 0;
         setVerboseInParcelling( 0 );
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 ) {
         _VT_DEBUG_ = 1;
      }

      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
         par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
         par->names.swap = 1;
      }





      /*---  ---*/

      else if ( strcmp ( argv[i], "-dist" ) == 0 ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -dist...\n", 0 );
         strncpy( par->names.ext, argv[i], STRINGLENGTH );
      }
      
      
      
      else if ( strcmp ( argv[i], "-seeds" ) == 0 ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -seeds...\n", 0 );
        strncpy( par->seedfile, argv[i], STRINGLENGTH );
      }

      else if ( strcmp ( argv[i], "-sout" ) == 0 ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -sout...\n", 0 );
        strncpy( par->seedout, argv[i], STRINGLENGTH );
      }

      else if ( strcmp ( argv[i], "-wi" ) == 0 ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -wi...\n", 0 );
        strncpy( par->seedimg, argv[i], STRINGLENGTH );
      }

      
      
      else if ( strcmp ( argv[i], "-parcels" ) == 0 || strcmp ( argv[i], "-p" ) == 0 ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -parcels...\n", 0 );
         status = sscanf( argv[i],"%d",&(par->nparcels) );
         if ( status <= 0 ) VT_ErrorParse( "parsing -parcels...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-random-seed" ) == 0 || strcmp ( argv[i], "-rs" ) == 0 ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -random-seed...\n", 0 );
         status = sscanf( argv[i],"%ld",&randomseed );
         if ( status <= 0 ) VT_ErrorParse( "parsing -random-seed...\n", 0 );
         setRandomSeedInParcelling( randomseed );
      }

      else if ( strcmp ( argv[i], "-iterations" ) == 0 || strcmp ( argv[i], "-i" ) == 0 ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -iterations...\n", 0 );
         status = sscanf( argv[i],"%d",&iterations );
         if ( status <= 0 ) VT_ErrorParse( "parsing -iterations...\n", 0 );
         parcelling_setNumberOfIterations( iterations );
      }

      else if ( strcmp ( argv[i], "-force" ) == 0 ) {
         parcelling_ForceExactCenterCalculation();
      }
      else if ( strcmp ( argv[i], "-no-force" ) == 0 ) {
         parcelling_DoNotForceExactCenterCalculation();
      }

      else if ( strcmp ( argv[i], "-partial-update" ) == 0 ) {
         parcelling_setPartialUpdate( 1 );
      }
      else if ( strcmp ( argv[i], "-no-partial-update" ) == 0 ) {
         parcelling_setPartialUpdate( 0 );
      }



      /* chamfer related options
       */
      else if ( (strcmp ( argv[i], "-chamfer-mask-size" ) == 0) ||
                  (strcmp ( argv[i], "-cm-size" ) == 0) ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -chamfer-mask-size...\n", 0 );
         status = sscanf( argv[i],"%d", &(par->chamfercoef_size) );
         if ( status <= 0 ) VT_ErrorParse( "parsing -chamfer-mask-size...\n", 0 );
      }
      else if ( (strcmp ( argv[i], "-chamfer-mask-max" ) == 0) ||
                  (strcmp ( argv[i], "-cm-max" ) == 0) ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -chamfer-mask-max...\n", 0 );
         status = sscanf( argv[i],"%d", &(par->chamfercoef_max) );
         if ( status <= 0 ) VT_ErrorParse( "parsing -chamfer-mask-max...\n", 0 );
      }

      /* predefined masks
       */

      else if ( (strcmp ( argv[i], "-predefined-chamfer-mask" ) == 0) ||
                  (strcmp ( argv[i], "-pcm" ) == 0 && argv[i][4] == '\0') ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -predefined-chamfer-mask...\n", 0 );
         if ( strcmp( argv[i], "4" ) == 0 && argv[i][1] == '\0' ) {
           par->predefineddistance = _DISTANCE04_;
         }
         else if ( strcmp( argv[i], "8" ) == 0 && argv[i][1] == '\0' ) {
           par->predefineddistance = _DISTANCE08_;
         }
         else if ( strcmp( argv[i], "6" ) == 0 && argv[i][1] == '\0' ) {
           par->predefineddistance = _DISTANCE06_;
         }
         else if ( strcmp( argv[i], "10" ) == 0 && argv[i][2] == '\0' ) {
           par->predefineddistance = _DISTANCE10_;
         }
         else if ( strcmp( argv[i], "18" ) == 0 && argv[i][2] == '\0' ) {
           par->predefineddistance = _DISTANCE18_;
         }
         else if ( strcmp( argv[i], "26" ) == 0 && argv[i][2] == '\0' ) {
           par->predefineddistance = _DISTANCE26_;
         }
         else if ( strcmp( argv[i], "3x3" ) == 0 && argv[i][3] == '\0' ) {
           par->predefineddistance = _CHAMFER3x3_;
         }
         else if ( strcmp( argv[i], "5x5" ) == 0 && argv[i][3] == '\0' ) {
           par->predefineddistance = _CHAMFER5x5_;
         }
         else if ( strcmp( argv[i], "3x3x3" ) == 0 && argv[i][5] == '\0' ) {
           par->predefineddistance = _CHAMFER3x3x3_;
         }
         else if ( strcmp( argv[i], "5x5x5" ) == 0 && argv[i][5] == '\0' ) {
           par->predefineddistance = _CHAMFER5x5x5_;
         }
         else {
           VT_ErrorParse( "parsing -predefined-chamfer-mask: unknown distance type ...\n", 0 );
         }
      }

      else if ( strcmp ( argv[i], "-connectivity" ) == 0 ||
                  (strcmp ( argv[i], "-c" ) == 0  && argv[i][2] == '\0') ||
                  (strcmp ( argv[i], "-con" ) == 0 && argv[i][4] == '\0')  ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -connectivity...\n", 0 );
         status = sscanf( argv[i],"%d",&connectivity );
         if ( status <= 0 ) VT_ErrorParse( "parsing -connectivity...\n", 0 );
         switch ( connectivity ) {
         case  4 : par->predefineddistance = _DISTANCE04_; break;
         case  8 : par->predefineddistance = _DISTANCE08_; break;
         case  6 : par->predefineddistance = _DISTANCE06_; break;
         case 10 : par->predefineddistance = _DISTANCE10_; break;
         case 18 : par->predefineddistance = _DISTANCE18_; break;
         case 26 : par->predefineddistance = _DISTANCE26_; break;
         default :
           VT_ErrorParse( "parsing -connectivity: connectivity is not handled...\n", 0 );
         }
      }

      else if ( strcmp ( argv[i], "-chamfer" ) == 0 && argv[i][8] == '\0' ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -chamfer...\n", 0 );
         status = sscanf( argv[i],"%d", &chamfer );
         if ( status <= 0 ) VT_ErrorParse( "parsing -chamfer...\n", 0 );
         switch ( chamfer ) {
         case  3 : par->predefineddistance = _CHAMFER3x3x3_; break;
         case  5 : par->predefineddistance = _CHAMFER5x5x5_; break;
         default :
           VT_ErrorParse( "parsing -chamfer: mask is not handled...\n", 0 );
         }
      }

      else if ( strcmp ( argv[i], "-print-mask" ) == 0 ) {
         par->printmask = 1;
      }



      /*--- lecture du type de l'image de sortie ---*/
      else if ( strcmp ( argv[i], "-r" ) == 0 ) {
         r = 1;
      }
      else if ( strcmp ( argv[i], "-s" ) == 0 ) {
         s = 1;
      }
      else if ( strcmp ( argv[i], "-o" ) == 0 ) {
         i += 1;
         if ( i >= argc)    VT_ErrorParse( "parsing -o...\n", 0 );
         status = sscanf( argv[i],"%d",&o );
         if ( status <= 0 ) VT_ErrorParse( "parsing -o...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-parallel" ) == 0 ) {
         setParallelism( _DEFAULT_PARALLELISM_ );
      }

      else if ( strcmp ( argv[i], "-no-parallel" ) == 0 ) {
         setParallelism( _NO_PARALLELISM_ );
      }

      else if ( strcmp ( argv[i], "-print-time" ) == 0
                 || (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
         par->print_time = 1;
      }
      else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')
                  || (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
         par->print_time = 0;
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
  
  /*--- type de l'image resultat ---*/
  if ( (o == 1) && (s == 1) && (r == 0) )  par->type = SCHAR;
  if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
  if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
  if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
  if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
  if ( (o == 0) && (s == 0) && (r == 1) )  par->type = FLOAT;
  /* if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program); */
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
  par->nparcels = 100;
  par->type = TYPE_UNKNOWN;
  par->seedfile[0] = '\0';
  par->seedout[0] = '\0';
  par->seedimg[0] = '\0';

  par->chamfercoef_dim = 0;
  par->chamfercoef_size = 3;
  par->chamfercoef_max = 3;

  par->predefineddistance = _UNDEFINED_DISTANCE_;

  par->printmask = 0;

  par->print_time = 1;

}





static char *_BaseName( char *p )
{
  int l;
  if ( p == (char*)NULL ) return( (char*)NULL );
  l = strlen( p ) - 1;
  while ( l >= 0 && p[l] != '/' ) l--;
  if ( l < 0 ) l = 0;
  if ( p[l] == '/' ) l++;
  return( &(p[l]) );
}



static double _GetTime()
{
  struct timeval tv;
  gettimeofday(&tv, (void *)0);
  return ( (double) tv.tv_sec + tv.tv_usec*1e-6 );
}



static double _GetClock()
{
  return ( (double) clock() / (double)CLOCKS_PER_SEC );
}
