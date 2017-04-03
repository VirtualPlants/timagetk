/*************************************************************************
 * histoStats.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 1999, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Ven 11 jul 2014 18:52:24 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#include <sys/time.h> /* gettimeofday() */
#include <time.h> /* clock() */

#include <histogram.h>
#include <string-tools.h>

#include <vt_common.h>
#include <vt_histo-tools.h>


static int _time_ = 0;
static int _clock_ = 0;



typedef struct local_par {
  vt_names names;
  char *description;
} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );
static double VT_GetTime();
static double VT_GetClock();





static char *usage = "[image-1] [image-2] [file-out]\n";


static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n";


static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image1, *image2;

  typeHistogram histo;
  typeHistogram hdiff;

  int theDim[3];

  double time_init, time_exit;
  double clock_init, clock_exit;

  time_init = VT_GetTime();
  clock_init = VT_GetClock();


  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  



  /*--- lecture de l'image d'entree ---*/
  image1 = _VT_Inrimage( par.names.in );
  if ( image1 == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image #1\n", 0);
  
  image2 = _VT_Inrimage( par.names.ext );
  if ( image2 == (vt_image*)NULL ) {
    VT_FreeImage( image1 );
    VT_Free( (void**)&image1 );
    VT_ErrorParse("unable to read input image #2\n", 0);
  }
  
  if ( image1->dim.v != 1 || image2->dim.v != 1 ) {
    VT_FreeImage( image2 );
    VT_Free( (void**)&image2 );
    VT_FreeImage( image1 );
    VT_Free( (void**)&image1 );
    VT_ErrorParse("unable to deal with vectorial images\n", 0 );
  }

  if ( image1->dim.x != image2->dim.x
       || image1->dim.y != image2->dim.y
       || image1->dim.z != image2->dim.z ) {
    VT_FreeImage( image2 );
    VT_Free( (void**)&image2 );
    VT_FreeImage( image1 );
    VT_Free( (void**)&image1 );
    VT_ErrorParse( "images do not have the same dimension\n", 0 );
  }

  theDim[0] = image1->dim.x;
  theDim[1] = image1->dim.y;
  theDim[2] = image1->dim.z;


  initHistogram( &histo );
 

  if ( alloc2DHistogramFromImages( &histo, 
				   image1->buf, image1->type, (void*)NULL, TYPE_UNKNOWN, (double*)NULL,
				   image2->buf, image2->type, (void*)NULL, TYPE_UNKNOWN, (double*)NULL,
				   theDim ) != 1 ) {
    freeHistogram( &histo );
    VT_FreeImage( image2 );
    VT_Free( (void**)&image2 );
    VT_FreeImage( image1 );
    VT_Free( (void**)&image1 );
    VT_ErrorParse( "unable to compute joint histogram\n", 0 );
  }

  VT_FreeImage( image2 );
  VT_Free( (void**)&image2 );
  VT_FreeImage( image1 );
  VT_Free( (void**)&image1 );
  
  if ( 0 ) {
    if ( par.names.out != (char*)NULL && par.names.out[0] != '\0' ) {
      if ( VT_WriteHistogram( par.names.out, &histo, par.description ) != 1 ) {
	freeHistogram( &histo );
	VT_ErrorParse("unable to write output image/histogram\n", 0 );
      }
    }
  }
  

  initHistogram( &hdiff );

  if ( alloc1DDifferenceHistogramFromJointHistogram( &hdiff, &histo ) != 1 ) {
    freeHistogram( &hdiff );
    freeHistogram( &histo );
    VT_ErrorParse( "unable to compute difference histogram\n", 0 );
  }

  if ( 1 ) {
    if ( par.names.out != (char*)NULL && par.names.out[0] != '\0' ) {
      if ( VT_WriteHistogram( par.names.out, &hdiff, par.description ) != 1 ) {
	freeHistogram( &histo );
	VT_ErrorParse("unable to write output image/histogram\n", 0 );
      }
    }
  }


  freeHistogram( &hdiff );
  freeHistogram( &histo );
  

  
  time_exit = VT_GetTime();
  clock_exit = VT_GetClock();

  if ( _time_ ) 
    fprintf( stderr, "%s: elapsed time = %f\n", program, time_exit - time_init );

  if ( _clock_ ) 
    fprintf( stderr, "%s: elapsed time = %f\n", program, clock_exit - clock_init );

  return( 1 );
}








static void VT_Parse( int argc, 
		      char *argv[], 
		      local_par *par )
{
  int i, nb;
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
      else if ( strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0' ) {
	_VT_VERBOSE_ = 1;
	incrementVerboseInHistogram();
      }
      else if ( strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0' ) {
	_VT_VERBOSE_ = 0;
	setVerboseInHistogram( 0 );
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0' ) {
	_VT_DEBUG_ = 1;
      }
      else if ( strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0' ) {
	_time_ = 1;
      }
      else if ( strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0' ) {
	_time_ = 0;
      }
      else if ( strcmp ( argv[i], "-clock" ) == 0 && argv[i][6] == '\0' ) {
	_clock_ = 1;
      }
      else if ( strcmp ( argv[i], "-noclock" ) == 0 && argv[i][8] == '\0' ) {
	_clock_ = 0;
      }


      


     else if ( strcmp ( argv[i], "-desc" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -desc...\n", 0 );
	par->description = argv[i];
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
	strncpy( par->names.ext, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else if ( nb == 2 ) {
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
  par->description = (char*)NULL;
}


static double VT_GetTime() 
{
  struct timeval tv;
  gettimeofday(&tv, (void *)0);
  return ( (double) tv.tv_sec + tv.tv_usec*1e-6 );
}

static double VT_GetClock() 
{
  return ( (double) clock() / (double)CLOCKS_PER_SEC );
}




