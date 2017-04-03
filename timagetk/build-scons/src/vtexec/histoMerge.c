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

static int _verbose_ = 1;


typedef struct local_par {
  vt_names names;
  char *description;
  int firstindex;
  int lastindex;
  int fbound;
  int lbound;
  int type;
} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );
static double VT_GetTime();
static double VT_GetClock();

static int _readHistogramList( typeHistogramList *hl, stringList *sl );




static char *usage = "[format-in] -f[irst] %d -l[ast] %d [file-out]\n\
 [-desc %s] [-fbound %d] [-lbound %d]\n";


static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n";


static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;

  typeHistogramList histoList;
  stringList histoFileList;

  typeHistogram histo;

  double time_init, time_exit;
  double clock_init, clock_exit;

  time_init = VT_GetTime();
  clock_init = VT_GetClock();


  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  


  initStringList( &histoFileList );
  initHistogramList( &histoList );

  if ( par.names.in != (char*)NULL && par.names.in[0] != '\0' ) {
    if ( buildStringListFromFormat( par.names.in, par.firstindex, par.lastindex, &histoFileList ) != 1 ) {
      VT_ErrorParse( "unable to build input histogram list\n", 0);
    }
  }
  else {
    VT_ErrorParse( "no input histogram format\n", 0);
  }

  if ( allocHistogramList( &histoList, histoFileList.n ) != 1 ) {
    freeStringList( &histoFileList );
    VT_ErrorParse( "unable to allocate histogram list\n", 0 );
  }

  if ( _readHistogramList( &histoList, &histoFileList ) != 1 ) {
    freeHistogramList( &histoList );
    freeStringList( &histoFileList );
    VT_ErrorParse( "unable to read histograms from list\n", 0 );
  }

  freeStringList( &histoFileList );



  if ( 0 ) {
    int i;
    for ( i=0; i<histoList.n; i++ ) {
      fprintf( stdout, "HISTOGRAM #%3d\n", i );
      fprintfHistogramHeaderInfo( stdout, &(histoList.data[i]), (char*)NULL );
    }
  }

  /* ...
     processing on list takes place here
  */
  initHistogram( &histo );
  if ( alloc2DHistogramFromHistogramList( &histo, &histoList ) != 1 ) {
    freeHistogramList( &histoList );
    VT_ErrorParse( "unable to convert histogram list into histogram\n", 0 );
  }




  if ( par.names.out != (char*)NULL && par.names.out[0] != '\0' ) {
    if ( VT_WriteHistogram( par.names.out, &histo, par.description ) != 1 ) {
      freeHistogram( &histo );
      freeHistogramList( &histoList );
      VT_ErrorParse("unable to write output image/histogram\n", 0);
    }
  }



  freeHistogram( &histo );
  freeHistogramList( &histoList );





  
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
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
	_VT_VERBOSE_ = 1;
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 ) {
	_VT_DEBUG_ = 1;
      }



      else if ( (strcmp ( argv[i], "-f" ) == 0 && argv[i][2] == '\0') 
		|| (strcmp ( argv[i], "-first" ) == 0 && argv[i][6] == '\0') ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -first ...\n", 0 );
	status = sscanf( argv[i], "%d", &(par->firstindex) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -first ...", 0 );
      }
      else if ( (strcmp ( argv[i], "-l" ) == 0 && argv[i][2] == '\0') 
		|| (strcmp ( argv[i], "-last" ) == 0 && argv[i][5] == '\0') ) {
	i += 1;
	if ( i >= argc) VT_ErrorParse( "parsing -last ...\n", 0 );
	status = sscanf( argv[i], "%d", &(par->lastindex) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -last ...", 0 );
      }



     else if ( strcmp ( argv[i], "-desc" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -desc...\n", 0 );
	par->description = argv[i];
      }
     else if ( strcmp ( argv[i], "-fbound" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -fbound...\n", 0 );
	status = sscanf( argv[i], "%d", &(par->fbound) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -fbound...\n", 0 );
      }
     else if ( strcmp ( argv[i], "-lbound" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -lbound...\n", 0 );
	status = sscanf( argv[i], "%d", &(par->lbound) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -lbound...\n", 0 );
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
  
  
  /*--- type de l'image resultat ---*/
  if ( (o == 1) && (s == 1) && (r == 0) ) par->type = SCHAR;
  if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
  if ( (o == 2) && (s == 1) && (r == 0) ) par->type = SSHORT;
  if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
  if ( (o == 4) && (s == 1) && (r == 0) ) par->type = SINT;
  if ( (o == 4) && (s == 0) && (r == 0) ) par->type = UINT;
  if ( (o == 0) && (s == 0) && (r == 1) ) par->type = FLOAT;
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
  par->description = (char*)NULL;
  par->firstindex = 0;
  par->lastindex = 0;
  par->fbound = -65535;
  par->lbound =  65535;
  par->type = TYPE_UNKNOWN;
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




/************************************************************
 *
 ************************************************************/

static int _readHistogramList( typeHistogramList *hl, stringList *sl )
{
  char *proc = "_readHistogramList";
  int i, j;
  
  for ( i=0; i<sl->n; i++ ) {
    if ( VT_ReadImageAsHistogram( sl->data[i], &(hl->data[i]) ) != 1 ) {
      for (j=0; j<i; j++ )
	freeHistogram( &(hl->data[j]) );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to read histogram #%d '%s'\n",
		 proc, i, sl->data[i] );
      return( -1 );
    }
  }

  return( 1 );
}
