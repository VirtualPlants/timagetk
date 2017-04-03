/*************************************************************************
 * tsingle-axon.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2016, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun  7 mar 2016 22:28:25 CET
 *
 * ADDITIONS, CHANGES
 *
 */

#include <vt_common.h>

#include <connexe.h>
#include <topological-thickening.h>
#include <topological-thinning.h>

#include <topological-single-axon.h>




typedef struct local_par {
  vt_names names;

  char *valuesimage;

  typeSingleAxonParameters segpar;

  char *skeletonimage;
  typeThinningParameters thinpar;

  int type;
} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );






static char *usage = "[image-in] [image-out]\n\
 [-values-image|-values %s]\n\
 [-low-threshold|-lt|-sb %f] [-high-threshold|-ht|-sh %f]\n\
 [-skeleton %s]\n\
 [-step %d]\n\
 [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\n\
# from the seed image (consider as a binary image)\n\
# 1. removes all simple points below the high threshold,\n\
#    (somehow mimics the regression of axons branches)\n\
# 2. add simple points above the low threshold\n\
#    until a maximum number of points above the high threshold\n\
#    have been captured (somehow mimics the expansion of axon branches)\n\
 -seed-image|-seeds %s: seeds image for extraction = initial object\n\
 -high-threshold|-ht|-sh %f: defines points of interest/to be kept\n\
 -low-threshold|-lt|-sb %f: defines points that can be kept to connect\n\
  points of interest\n\
 -inv         # invert 'image-in'\n\
 -swap        # swap 'image-in' (if encoded on 2 bytes)\n\
 -v|-nv       # verbose | no verbose\n\
 -D]          #  debug\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imtmp, imskel;
  vt_image *imptr = (vt_image*)NULL;
  vt_image *imvalues;
  int theDim[3];
  int i, v;
  unsigned char *theBuf;

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );

  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );


  if ( par.valuesimage == (char*)NULL ) {
    VT_ErrorParse( "no values image\n", 0 );
  }
  imvalues = _VT_Inrimage( par.valuesimage );
  if ( imvalues == (vt_image*)NULL )
    VT_ErrorParse("unable to read values image\n", 0);

  if ( par.names.out[0] != '\0' ) {
    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) {
      VT_FreeImage( imvalues );
      VT_Free( (void**)&imvalues );
      VT_ErrorParse("unable to read input image\n", 0);
    }
    if ( image->type != UCHAR ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( imvalues );
      VT_Free( (void**)&imvalues );
      VT_ErrorParse("unable to deal with such input image type\n", 0);
    }
    if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_FreeImage( imvalues );
      VT_Free( (void**)&imvalues );
      VT_ErrorParse("error when copying image name\n", 0);
    }
    imptr = image;
  }
  else {
    VT_Image( &imtmp );
    VT_InitFromImage( &imtmp, imvalues, par.names.in, UCHAR );
    if ( VT_AllocImage( &imtmp ) != 1 ) {
      VT_FreeImage( imvalues );
      VT_Free( (void**)&imvalues );
      VT_ErrorParse( "unable to allocate auxiliary image \n", 0);
    }
    imptr = &imtmp;
  }


  theDim[0] = imvalues->dim.x;
  theDim[1] = imvalues->dim.y;
  theDim[2] = imvalues->dim.z;

  if ( treePropagation( imptr->buf, imvalues->buf, imvalues->type,
                       theDim, &(par.segpar) ) != 1 ) {
    VT_FreeImage( imptr );
    if ( par.names.out[0] != '\0' ) VT_Free( (void**)&imptr );
    VT_FreeImage( imvalues );
    VT_Free( (void**)&imvalues );
    VT_ErrorParse( "error in tree computation \n", 0);
  }

  if ( par.skeletonimage != (char*)NULL ) {

    VT_Image( &imskel );
    VT_InitFromImage( &imskel, imvalues, par.skeletonimage, UCHAR );
    if ( VT_AllocImage( &imskel ) != 1 ) {
      VT_FreeImage( imptr );
      if ( par.names.out[0] != '\0' ) VT_Free( (void**)&imptr );
      VT_FreeImage( imvalues );
      VT_Free( (void**)&imvalues );
      VT_ErrorParse( "unable to allocate skeleton image \n", 0);
    }
    v = theDim[0] * theDim[1] * theDim[2];
    (void)memcpy( imskel.buf, imptr->buf, v*sizeof(u8) );
    theBuf = (unsigned char *)imskel.buf;
    for ( i=0; i<v; i++ )
      if ( theBuf[i] > 0 ) theBuf[i] = _CANBECHANGED_VALUE_;

    if ( valueBasedThinning( imskel.buf, imvalues->buf, imvalues->type,
                             theDim, &(par.thinpar) ) != 1 ) {
      VT_FreeImage( &imskel );
      VT_FreeImage( imptr );
      if ( par.names.out[0] != '\0' ) VT_Free( (void**)&imptr );
      VT_FreeImage( imvalues );
      VT_Free( (void**)&imvalues );
      VT_ErrorParse( "error in skeleton computation \n", 0);
    }

    if ( VT_WriteInrimage( &imskel ) == -1 ) {
      VT_FreeImage( &imskel );
      VT_FreeImage( imptr );
      if ( par.names.out[0] != '\0' ) VT_Free( (void**)&imptr );
      VT_FreeImage( imvalues );
      VT_Free( (void**)&imvalues );
      VT_ErrorParse("unable to write output skeleton image\n", 0);
    }

    VT_FreeImage( &imskel );

  }



  VT_FreeImage( imvalues );
  VT_Free( (void**)&imvalues );





  if ( VT_WriteInrimage( imptr ) == -1 ) {
    VT_FreeImage( imptr );
    if ( par.names.out[0] != '\0' ) VT_Free( (void**)&imptr );
    VT_ErrorParse("unable to write output image\n", 0);
  }

  VT_FreeImage( imptr );
  if ( par.names.out[0] != '\0' ) VT_Free( (void**)&imptr );


  return( 1 );
}








static void VT_Parse( int argc, 
                      char *argv[],
                      local_par *par )
{
  int i, nb, status;
  int o=0, s=0, r=0;
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
      else if ( (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0')
                || strcmp ( argv[i], "-verbose" ) == 0 ) {
        _VT_VERBOSE_ = 1;
        incrementVerboseInTopologicalThickening();
        incrementVerboseInTopologicalThinning();
        incrementVerboseInTopologicalSingleAxon();
        incrementVerboseInConnexe();
      }
      else if ( (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0')
                || strcmp ( argv[i], "-noverbose" ) == 0 ) {
        _VT_VERBOSE_ = 0;
        setVerboseInTopologicalThickening( 0 );
        setVerboseInTopologicalThinning( 0 );
        setVerboseInTopologicalSingleAxon( 0 );
        setVerboseInConnexe( 0 );
      }
      else if ( (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0')
                || strcmp ( argv[i], "-debug" ) == 0 ) {
        _VT_DEBUG_ = 1;
        incrementDebugInTopologicalThinning();
        incrementDebugInTopologicalSingleAxon();
      }
      else if ( strcmp ( argv[i], "-nodebug" ) == 0 ) {
        _VT_DEBUG_ = 0;
        setDebugInTopologicalThinning( 0 );
        setDebugInTopologicalSingleAxon( 0 );
      }
      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
        par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
        par->names.swap = 1;
      }



      /* ]
       */
      else if ( strcmp ( argv[i], "-values-image" ) == 0 ||
                strcmp ( argv[i], "-values" ) == 0 ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -values-image...\n", 0 );
        par->valuesimage = argv[i];
      }

      else if ( (strcmp ( argv[i], "-low-threshold" ) == 0) ||
                (strcmp ( argv[i], "-lt" ) == 0 && argv[i][3] == '\0') ||
                (strcmp ( argv[i], "-sb" ) == 0 && argv[i][3] == '\0') ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -low-threshold...\n", 0 );
        status = sscanf( argv[i], "%d", &par->segpar.low_threshold );
        if ( status <= 0 ) VT_ErrorParse( "parsing -low-threshold...\n", 0 );
      }
      else if ( (strcmp ( argv[i], "-high-threshold" ) == 0) ||
                (strcmp ( argv[i], "-ht" ) == 0 && argv[i][3] == '\0') ||
                (strcmp ( argv[i], "-sh" ) == 0 && argv[i][3] == '\0') ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -high-threshold...\n", 0 );
        status = sscanf( argv[i], "%d", &par->segpar.high_threshold );
        if ( status <= 0 ) VT_ErrorParse( "parsing -high-threshold...\n", 0 );
      }

      else if ( (strcmp ( argv[i], "-step" ) == 0 && argv[i][5] == '\0') ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -step...\n", 0 );
        status = sscanf( argv[i], "%d", &par->segpar.stepToStop );
        if ( status <= 0 ) VT_ErrorParse( "parsing -step...\n", 0 );
      }

      else if ( strcmp ( argv[i], "-skeleton-image" ) == 0 ||
                strcmp ( argv[i], "-skeleton" ) == 0 ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -skeleton-image...\n", 0 );
        par->skeletonimage = argv[i];
      }


      /*
      else if ( (strcmp ( argv[i], "-threshold" ) == 0) || 
                (strcmp ( argv[i], "-t" ) == 0 && argv[i][2] == '\0') ||
                (strcmp ( argv[i], "-sb" ) == 0 && argv[i][3] == '\0') ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -threshold...\n", 0 );
        status = sscanf( argv[i], "%d", &par->threshold );
        if ( status <= 0 ) VT_ErrorParse( "parsing -threshold...\n", 0 );
      }
      */







      /*--- lecture du type de l'image de sortie ---*/
      else if ( strcmp ( argv[i], "-r" ) == 0 && argv[i][3] == '\0' ) {
        r = 1;
      }
      else if ( strcmp ( argv[i], "-s" ) == 0 && argv[i][3] == '\0' ) {
        s = 1;
      }
      else if ( strcmp ( argv[i], "-o" ) == 0 && argv[i][3] == '\0' ) {
        i += 1;
        if ( i >= argc)    VT_ErrorParse( "parsing -o...\n", 0 );
        status = sscanf( argv[i],"%d",&o );
        if ( status <= 0 ) VT_ErrorParse( "parsing -o...\n", 0 );
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

  par->valuesimage = (char*)NULL;
  initTypeSingleAxonParameters( &(par->segpar) );

  par->skeletonimage = (char*)NULL;
  initTypeThinningParameters( &(par->thinpar) );
  par->thinpar.typeEndPoint = _PURE_CURVE_;

  par->type = TYPE_UNKNOWN;
}
