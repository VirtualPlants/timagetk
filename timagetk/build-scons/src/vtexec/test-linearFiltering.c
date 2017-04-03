/*************************************************************************
 * test-linearFilter.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Dec 10 16:30:01 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>


#include <vt_common.h>

#include <chunks.h>


#include <linearFiltering-tools.h>
#include <linearFiltering.h>


typedef struct local_par {
  vt_names names;
  
  int borderLengths[3];
  typeFilteringCoefficients filter[3];

  int print_time;

} local_par;





/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );
static void VT_PrintParam( FILE *theFile, local_par *par );
static double _GetTime();
static double _GetClock();





static char *usage = "[image-in] [image-out] [-x %d] [-y %d] [-z %d]\n\
 [-alpha %lf [%lf [%lf]]] [-sigma %lf [%lf [%lf]]] \n\
 [-border|cont %d [%d [%d]]]\n\
 [-gaussian-type|-type [deriche|fidrich|young-1995|young-2002|gabor-young-2002|convolution]]\n\
 [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
 si 'image-in' est '-', on prendra stdin\n\
 si 'image-out' est absent, on prendra stdout\n\
 si les deux sont absents, on prendra stdin et stdout\n\
 -x | -y | -z : ordre de derivation selon X, Y ou Z\n\
 -alpha | -a  : alpha pour le filtre recursif de Deriche\n\
 -sigma       : sigma pour l'approximation de la gaussienne\n\
 -cont        : points ajoutes aux bords\n\
 -inv : inverse 'image-in'\n\
 -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
 -v : mode verbose\n\
 -D : mode debug\n\
 options-de-type : -o 1    : unsigned char\n\
                   -o 2    : unsigned short int\n\
                   -o 2 -s : short int\n\
                   -o 4 -s : int\n\
                   -r      : float\n\
 si aucune de ces options n'est presente, on prend le type de 'image-in'\n";





static char program[STRINGLENGTH];

int main( int argc, char *argv[] )
{
  local_par par;
  vt_image image, imres, imtest;
  int theDims[3];
  typeFilteringCoefficients filter[3];

 vt_3m m;

  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;

  SetConvolution1DSigmaMultiplier( 4.5 );

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- image d'entree ---*/
  VT_InitImage( &image, "image-test-linearFiltering.hdr", 
		128, 128, 1, FLOAT );
  if ( VT_AllocImage( &image ) != 1 ) {
    VT_ErrorParse("unable to allocate input image\n", 0);
  }
  {
    int i, j, x, y;
    int ix[3] = { 9, image.dim.x/2, 118 };
    int iy[3] = { 9, image.dim.y/2, 118 };
    float *buf = (float*)image.buf;
    for ( i=0, y=0; y<image.dim.y; y++ )
    for ( x=0; x<image.dim.x; x++, i++ )
      buf[i] = 0.0;
    for ( j=0; j<3; j++ )
    for ( i=0; i<3; i++ )
      buf[ iy[j]*image.dim.x+ix[i] ] = 1.0;
  }


  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  VT_InitFromImage( &imres, &image, "imres-test-linearFiltering.hdr", FLOAT );
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( &image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
  
  /*--- initialisation de l'image test ---*/
  VT_Image( &imtest );
  VT_InitFromImage( &imtest, &image, "imtest-test-linearFiltering.hdr", FLOAT );
  if ( VT_AllocImage( &imtest ) != 1 ) {
    VT_FreeImage( &imres );
    VT_FreeImage( &image );
    VT_ErrorParse("unable to allocate test image\n", 0);
  }
  

  /*--- calcul ---*/
  theDims[0] = image.dim.x;
  theDims[1] = image.dim.y;
  theDims[2] = image.dim.z;


  filter[0] = par.filter[0];
  filter[1] = par.filter[1];
  filter[2] = par.filter[2];
  filter[0].type = GAUSSIAN_CONVOLUTION;
  filter[1].type = GAUSSIAN_CONVOLUTION;
  filter[2].type = GAUSSIAN_CONVOLUTION;

  if ( theDims[2] == 1 )
    par.filter[2].derivative = NODERIVATIVE;



  if ( separableLinearFiltering( image.buf, image.type, 
				 imres.buf, imres.type, theDims,
				 par.borderLengths, filter ) != 1 ) {
    VT_FreeImage( &imtest ); 
    VT_FreeImage( &imres ); 
    VT_FreeImage( &image );
    VT_ErrorParse("unable to filter input image\n", 0);
    exit( 1 );
  }
  
  if ( separableLinearFiltering( image.buf, image.type, 
				 imtest.buf, imtest.type, theDims,
				 par.borderLengths, par.filter ) != 1 ) {
    VT_FreeImage( &imtest ); 
    VT_FreeImage( &imres ); 
    VT_FreeImage( &image );
    VT_ErrorParse("unable to filter input image\n", 0);
    exit( 1 );
  }

  (void)VT_WriteInrimage( &imtest );

  {
    int x, y, i;
    float *testbuf = (float*)imtest.buf;
    float *resbuf = (float*)imres.buf;
    for ( i=0, y=0; y<imtest.dim.y; y++ )
    for ( x=0; x<imtest.dim.x; x++, i++ )
      testbuf[i] -= resbuf[i];
  }
  
  if ( VT_3m( &imtest, &m ) == -1 ) {
    VT_FreeImage( &imtest ); 
    VT_FreeImage( &imres ); 
    VT_FreeImage( &image );
    VT_ErrorParse("unable to compute minimum, mean and maximum\n", 0);
  }
  fprintf(stderr,"%17.6f %17.6f %17.6f\n", m.min, m.moy, m.max );
  fprintf(stderr,"                  %17.6f\n", m.ect );


  (void)VT_WriteInrimage( &imres );
  (void)VT_WriteInrimage( &image );

  /*--- ecriture de l'image resultat ---*/
  if ( 0 ) {
    if ( VT_WriteInrimage( &imres ) == -1 ) {
      VT_FreeImage( &imtest ); 
      VT_FreeImage( &image );
      VT_FreeImage( &imres );
      VT_ErrorParse("unable to write output image\n", 0);
    }
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( &imtest ); 
  VT_FreeImage( &image );
  VT_FreeImage( &imres );



  time_exit = _GetTime();
  clock_exit = _GetClock();

  if ( par.print_time ) { 
    fprintf( stderr, "%s: elapsed (real) time = %f\n", program, time_exit - time_init );
    fprintf( stderr, "\t       elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
    fprintf( stderr, "\t       ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
  }


  return( 1 );
}









static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, j, nb, status;
  int bool_contours = 0;
  int tmp;
  int maxchunks;
  char text[STRINGLENGTH];
  
  if ( VT_CopyName( program, argv[0] ) != 1 )
    VT_Error("Error while copying program name", (char*)NULL);
  if ( 0 && argc == 1 ) VT_ErrorParse("\n", 0 );
  
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
	if ( _VT_VERBOSE_ <= 0 ) _VT_VERBOSE_ = 1;
	else                     _VT_VERBOSE_ ++;
      }
      else if ( strcmp ( argv[i], "-nv" ) == 0 ) {
	_VT_VERBOSE_ = 0;
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

      /*--- ordres de derivation ---*/
      else if ( strcmp ( argv[i], "-x" ) == 0 && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
	status = sscanf( argv[i],"%d",&tmp );
	if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
	switch ( tmp ) {
	default :
	  par->filter[0].derivative = NODERIVATIVE;   break;
	case 0 :
	  par->filter[0].derivative = DERIVATIVE_0;   break;
	case 1 :
	  par->filter[0].derivative = DERIVATIVE_1;   break;
	case 2 :
	  par->filter[0].derivative = DERIVATIVE_2;   break;
	case 3 :
	  par->filter[0].derivative = DERIVATIVE_3;   break;
	}
      }

      else if ( strcmp ( argv[i], "-y" ) == 0 && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
	status = sscanf( argv[i],"%d",&tmp );
	if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
	switch ( tmp ) {
	default :
	  par->filter[1].derivative = NODERIVATIVE;   break;
	case 0 :
	  par->filter[1].derivative = DERIVATIVE_0;   break;
	case 1 :
	  par->filter[1].derivative = DERIVATIVE_1;   break;
	case 2 :
	  par->filter[1].derivative = DERIVATIVE_2;   break;
	case 3 :
	  par->filter[1].derivative = DERIVATIVE_3;   break;
	}
      }

      else if ( strcmp ( argv[i], "-z" ) == 0 && argv[i][2] == '\0' ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
	status = sscanf( argv[i],"%d",&tmp );
	if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
	switch ( tmp ) {
	default :
	  par->filter[2].derivative = NODERIVATIVE;   break;
	case 0 :
	  par->filter[2].derivative = DERIVATIVE_0;   break;
	case 1 :
	  par->filter[2].derivative = DERIVATIVE_1;   break;
	case 2 :
	  par->filter[2].derivative = DERIVATIVE_2;   break;
	case 3 :
	  par->filter[2].derivative = DERIVATIVE_3;   break;
	}
      }

      /*--- contours ? ---*/
      else if ( strcmp ( argv[i], "-edges" ) == 0 ) {
	bool_contours = 1;
      }

      /*--- alpha ---*/
      else if ( (strcmp ( argv[i], "-alpha" ) == 0) || 
		(strcmp ( argv[i], "-a" ) == 0 && argv[i][2] == '\0') ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing [-alpha|-a]...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->filter[0].coefficient) );
	if ( status <= 0 ) VT_ErrorParse( "parsing [-alpha|-a]...\n", 0 );
	i ++;
	if ( i >= argc) {
	  par->filter[1].coefficient = par->filter[0].coefficient;
	  par->filter[2].coefficient = par->filter[0].coefficient;
	}
	else {
	  status = sscanf( argv[i], "%lf", &(par->filter[1].coefficient) );
	  if ( status <= 0 ) {
	    i--;
	    par->filter[1].coefficient = par->filter[0].coefficient;
	    par->filter[2].coefficient = par->filter[0].coefficient;
	  }
	  else {
	    i ++;
	    if ( i >= argc) par->filter[2].coefficient = 0.0;
	    else {
	      status = sscanf( argv[i], "%lf", &(par->filter[2].coefficient) );
	      if ( status <= 0 ) {
		i--;
		par->filter[2].coefficient = 0;
	      }
	    }
	  }
	}
	par->filter[0].type = ALPHA_DERICHE;
	par->filter[1].type = ALPHA_DERICHE;
	par->filter[2].type = ALPHA_DERICHE;
      }
      
      /*--- sigma ---*/
      else if ( (strcmp ( argv[i], "-sigma" ) == 0) ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -sigma ...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->filter[0].coefficient) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -sigma ...\n", 0 );
	i ++;
	if ( i >= argc) {
	  par->filter[1].coefficient = par->filter[0].coefficient;
	  par->filter[2].coefficient = par->filter[0].coefficient;
	}
	else {
	  status = sscanf( argv[i], "%lf", &(par->filter[1].coefficient) );
	  if ( status <= 0 ) {
	    i--;
	    par->filter[1].coefficient = par->filter[0].coefficient;
	    par->filter[2].coefficient = par->filter[0].coefficient;
	  }
	  else {
	    i ++;
	    if ( i >= argc) par->filter[2].coefficient = 0.0;
	    else {
	      status = sscanf( argv[i], "%lf", &(par->filter[2].coefficient) );
	      if ( status <= 0 ) {
		i--;
		par->filter[2].coefficient = 0;
	      }
	    }
	  }
	}
	for ( j=0; j<3; j++ ) {
	  switch ( par->filter[j].type ) {
	  case GAUSSIAN_DERICHE :
	  case GAUSSIAN_FIDRICH :
	  case GAUSSIAN_YOUNG_1995 :
	  case GAUSSIAN_YOUNG_2002 :
	  case GABOR_YOUNG_2002 :
	  case GAUSSIAN_CONVOLUTION :
	    break;
	  default :
	    par->filter[j].type = GAUSSIAN_CONVOLUTION;
	    break;
	  }
	}
      }
      
      /*--- bordure ---*/
      else if ( strcmp ( argv[i], "-cont" ) == 0 
		|| strcmp ( argv[i], "-border" ) == 0) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -cont...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->borderLengths[0]) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -cont...\n", 0 );
	i ++;
	if ( i >= argc) {
	  par->borderLengths[2] = par->borderLengths[1] = par->borderLengths[0];
	}
	else {
	  status = sscanf( argv[i], "%d", &(par->borderLengths[1]) );
	  if ( status <= 0 ) {
	    i--;
	    par->borderLengths[2] = par->borderLengths[1] = par->borderLengths[0];
	  }
	  else {
	    i ++;
	    if ( i >= argc) par->borderLengths[2] = 0;
	    else {
	      status = sscanf( argv[i], "%d", &(par->borderLengths[2]) );
	      if ( status <= 0 ) {
		i--;
		par->borderLengths[2] = 0;
	      }
	    }
	  }
	}
      }


      /*--- filter type ---*/
      else if ( strcmp ( argv[i], "-type" ) == 0 
		|| strcmp ( argv[i], "-filter-type" ) == 0 ) {
	i++;
	if ( strcmp ( argv[i], "deriche" ) == 0 ) {
	  par->filter[0].type = GAUSSIAN_DERICHE;
	  par->filter[1].type = GAUSSIAN_DERICHE;
	  par->filter[2].type = GAUSSIAN_DERICHE;
	}
	else if ( strcmp ( argv[i], "fidrich" ) == 0 ) {
	  par->filter[0].type = GAUSSIAN_FIDRICH;
	  par->filter[1].type = GAUSSIAN_FIDRICH;
	  par->filter[2].type = GAUSSIAN_FIDRICH;
	}
	else if ( strcmp ( argv[i], "young-1995" ) == 0 ) {
	  par->filter[0].type = GAUSSIAN_YOUNG_1995;
	  par->filter[1].type = GAUSSIAN_YOUNG_1995;
	  par->filter[2].type = GAUSSIAN_YOUNG_1995;
	}
	else if ( strcmp ( argv[i], "young-2002" ) == 0 ) {
	  par->filter[0].type = GAUSSIAN_YOUNG_2002;
	  par->filter[1].type = GAUSSIAN_YOUNG_2002;
	  par->filter[2].type = GAUSSIAN_YOUNG_2002;
	}
	else if ( strcmp ( argv[i], "gabor-young-2002" ) == 0 ) {
	  par->filter[0].type = GABOR_YOUNG_2002;
	  par->filter[1].type = GABOR_YOUNG_2002;
	  par->filter[2].type = GABOR_YOUNG_2002;
	}
	else if ( strcmp ( argv[i], "convolution" ) == 0  ) {
	  par->filter[0].type = GAUSSIAN_CONVOLUTION;
	  par->filter[1].type = GAUSSIAN_CONVOLUTION;
	  par->filter[2].type = GAUSSIAN_CONVOLUTION;
	}
	else {
	  VT_ErrorParse( "parsing -gaussian-type...\n", 0 );
	}
      }



      /* parallelism
       */
      else if ( strcmp ( argv[i], "-parallel" ) == 0 ) {
	setParallelism( _DEFAULT_PARALLELISM_ );
      }
      
      else if ( strcmp ( argv[i], "-no-parallel" ) == 0 ) {
	setParallelism( _NO_PARALLELISM_ );
      }
      
      else if ( strcmp ( argv[i], "-parallelism-type" ) == 0 ||
		strcmp ( argv[i], "-parallel-type" ) == 0 ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "-parallelism-type", 0 );
	if ( strcmp ( argv[i], "default" ) == 0 ) {
        setParallelism( _DEFAULT_PARALLELISM_ );
	}
	else if ( strcmp ( argv[i], "none" ) == 0 ) {
	  setParallelism( _NO_PARALLELISM_ );
	}
	else if ( strcmp ( argv[i], "openmp" ) == 0 || strcmp ( argv[i], "omp" ) == 0 ) {
	  setParallelism( _OMP_PARALLELISM_ );
	}
	else if ( strcmp ( argv[i], "pthread" ) == 0 || strcmp ( argv[i], "thread" ) == 0 ) {
	  setParallelism( _PTHREAD_PARALLELISM_ );
	}
	else {
	  fprintf( stderr, "unknown parallelism type: '%s'\n", argv[i] );
	  VT_ErrorParse( "-parallelism-type", 0 );
	}
      }
      

      else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "-max-chunks", 0 );
	status = sscanf( argv[i], "%d", &maxchunks );
	if ( status <= 0 ) VT_ErrorParse( "-max-chunks", 0 );
	if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
      }
      
      else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 || 
	      ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][3] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "-parallel-scheduling", 0 );
	if ( strcmp ( argv[i], "default" ) == 0 ) {
	  setOmpScheduling( _DEFAULT_OMP_SCHEDULING_ );
	}
	else if ( strcmp ( argv[i], "static" ) == 0 ) {
	  setOmpScheduling( _STATIC_OMP_SCHEDULING_ );
	}
	else if ( strcmp ( argv[i], "dynamic-one" ) == 0 ) {
	  setOmpScheduling( _DYNAMIC_ONE_OMP_SCHEDULING_ );
	}
	else if ( strcmp ( argv[i], "dynamic" ) == 0 ) {
	  setOmpScheduling( _DYNAMIC_OMP_SCHEDULING_ );
	}
	else if ( strcmp ( argv[i], "guided" ) == 0 ) {
	  setOmpScheduling( _GUIDED_OMP_SCHEDULING_ );
	}
	else {
	  fprintf( stderr, "unknown omp scheduling type: '%s'\n", argv[i] );
	  VT_ErrorParse( "-omp-scheduling", 0 );
	}
      }



      else if ( (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
	par->print_time = 1;
      }
      else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0') ) {
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
    
    par->borderLengths[0] = 0;
    par->borderLengths[1] = 0;
    par->borderLengths[2] = 0;
    initFilteringCoefficients( &(par->filter[0]) );
    initFilteringCoefficients( &(par->filter[1]) );
    initFilteringCoefficients( &(par->filter[2]) );

    par->filter[0].derivative = DERIVATIVE_0;
    par->filter[1].derivative = DERIVATIVE_0;
    par->filter[2].derivative = DERIVATIVE_0;

    par->filter[0].type = GAUSSIAN_DERICHE;
    par->filter[1].type = GAUSSIAN_DERICHE;
    par->filter[2].type = GAUSSIAN_DERICHE;

    par->filter[0].coefficient = 3.0;
    par->filter[1].coefficient = 3.0;
    par->filter[2].coefficient = 3.0;

    par->print_time = 0;
}





static void VT_PrintParam( FILE *theFile, local_par *par )
{
  FILE *f = theFile;
  if ( theFile == (FILE*)NULL ) f = stderr;

  fprintf( f, "==================================================\n" );
  fprintf( f, "%s: parameters\n", program );

  VT_PrintNames( f, &(par->names) );
  
  fprintf( f, "- borders = [%d %d %d]\n",  par->borderLengths[0],
	   par->borderLengths[1], par->borderLengths[2] );
  
  printFilteringCoefficients( f, &(par->filter[0]), "filter along X" );
  printFilteringCoefficients( f, &(par->filter[1]), "filter along Y" );
  printFilteringCoefficients( f, &(par->filter[2]), "filter along Z" );
  
  fprintf( f, "- print time = %d\n",  par->print_time );

  fprintf( f, "==================================================\n" );
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
