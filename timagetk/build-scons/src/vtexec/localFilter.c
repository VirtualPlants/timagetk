/*************************************************************************
 * localFilter.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sun Feb 17 21:32:20 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 */

#include <sys/time.h> /* gettimeofday() */
#include <time.h> /* clock() */

#include <local-operation.h>

#include <vt_common.h>


typedef enum {
  _MAX_,
  _MEAN_,
  _MEDIAN_,
  _MIN_,
  _QUANTILE_,
  _ROBUST_MEAN_,
  _STDDEV_,
  _SUM_,
  _VAR_
} typeOperation;

typedef struct local_par {
  vt_names names;
  int type;
  vt_ipt window;
  typeOperation operation;

  double quantile; /* 0: min, 0.5: median, 1:max */
  double lts_fraction; /* samples to be kept */

  int print_time;

} local_par;




/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );
static double VT_GetTime();
static double VT_GetClock();




static char *usage = "[image-in] [image-out]\n\
 [-max|-min|-mean|-median|-quantile|-robust-mean|-sum|-var|-stddev]\n\
 [-window %d %d [%d]]\n\
 [-quantile-value|-q %lf]\n\
 [-lts-cut|-lts-fraction %lf]\n\
 [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
 -max         # \n\
 -min         # \n\
 -mean        # \n\
 -median      # \n\
 -quantile    # \n\
 -robust-mean # \n\
 -sum         # \n\
 -var         # \n\
 -stddev      # \n\
 -window %d %d [%d] # \n\
 -quantile-value|-q %lf] # quantile of the retained value\n\
   0:  minimum value, thus '-quantile -q 0'   <=> '-min'\n\
   0.5: median value, thus '-quantile -q 0.5' <=> '-median'\n\
   1:  maximum value, thus '-quantile -q 1'   <=> '-max'\n\
 -lts-cut|-lts-fraction %lf] # fraction of points to be kept for the\n\
   calculation of the robust mean (trimmed estimation)\n\
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
\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imres;
  double time_init, time_exit;
  double clock_init, clock_exit;
  int theDim[3], winDim[3];

  time_init = VT_GetTime();
  clock_init = VT_GetClock();


  /*--- initialisation des parametres ---*/
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
  
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imres );
  VT_InitFromImage( &imres, image, par.names.out, image->type );
  if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
  if ( VT_AllocImage( &imres ) != 1 ) {
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse("unable to allocate output image\n", 0);
  }
  


  theDim[0] = image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;
  winDim[0] = par.window.x;
  winDim[1] = par.window.y;
  winDim[2] = par.window.z;

  switch ( par.operation ) {
  default :
    VT_FreeImage( image );
    VT_FreeImage( &imres );
    VT_Free( (void**)&image );
    VT_ErrorParse("such operation not handled yet\n", 0);
  
  case _MAX_ :
    if ( maxFiltering( image->buf, image->type,
		       imres.buf, imres.type,
		       theDim, winDim ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during max filtering\n", 0);
    }
    break;

  case _MEAN_ :
    if ( meanFiltering( image->buf, image->type,
		       imres.buf, imres.type,
		       theDim, winDim ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during mean filtering\n", 0);
    }
    break; 

  case _MEDIAN_ :
    if ( medianFiltering( image->buf, image->type,
			  imres.buf, imres.type,
			  theDim, winDim ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during median filtering\n", 0);
    }
    break;
    
  case _MIN_ :
    if ( minFiltering( image->buf, image->type,
		       imres.buf, imres.type,
		       theDim, winDim ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during min filtering\n", 0);
    }
    break;

  case _QUANTILE_ :
    if ( quantileFiltering( image->buf, image->type,
			  imres.buf, imres.type,
			    theDim, winDim, par.quantile ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during quantile filtering\n", 0);
    }
    break;
    
  case _ROBUST_MEAN_ :
    if ( robustMeanFiltering( image->buf, image->type,
		       imres.buf, imres.type,
			      theDim, winDim, par.lts_fraction ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during robust mean filtering\n", 0);
    }
    break; 

  case _STDDEV_ :
    if ( stddevFiltering( image->buf, image->type,
		       imres.buf, imres.type,
		       theDim, winDim ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during stddev filtering\n", 0);
    }
    break;

  case _SUM_ :
    if ( sumFiltering( image->buf, image->type,
		       imres.buf, imres.type,
		       theDim, winDim ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during sum filtering\n", 0);
    }
    break;

  case _VAR_ :
    if ( varFiltering( image->buf, image->type,
		       imres.buf, imres.type,
		       theDim, winDim ) != 1 ) {
      VT_FreeImage( image );
      VT_FreeImage( &imres );
      VT_Free( (void**)&image );
      VT_ErrorParse( "error during var filtering\n", 0);
    }
    break;

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

  
  time_exit = VT_GetTime();
  clock_exit = VT_GetClock();

  if (  par.print_time ) {
    fprintf( stderr, "%s: elapsed time = %f\n", program, time_exit - time_init );
    fprintf( stderr, "%s: elapsed time = %f\n", program, clock_exit - clock_init );
  }

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
      else if ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) {
	VT_ErrorParse( "\n", 0 );
      }
      else if ( strcmp ( argv[i], "-help" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
	_VT_VERBOSE_ = 1;
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





      else if ( strcmp ( argv[i], "-max" ) == 0 ) {
	par->operation = _MAX_;
      }
      else if ( strcmp ( argv[i], "-min" ) == 0 ) {
	par->operation = _MIN_;
      }
      else if ( strcmp ( argv[i], "-median" ) == 0 ) {
	par->operation = _MEDIAN_;
      }
      else if ( strcmp ( argv[i], "-mean" ) == 0 ) {
	par->operation = _MEAN_;
      }
      else if ( strcmp ( argv[i], "-quantile" ) == 0 ) {
	par->operation = _QUANTILE_;
      }
      else if ( strcmp ( argv[i], "-robust-mean" ) == 0 
		|| strcmp ( argv[i], "-rmean" ) == 0 ) {
	par->operation = _ROBUST_MEAN_;
      }
      else if ( strcmp ( argv[i], "-sum" ) == 0 ) {
	par->operation = _SUM_;
      }
      else if ( strcmp ( argv[i], "-stddev" ) == 0 ) {
	par->operation = _STDDEV_;
      }
      else if ( strcmp ( argv[i], "-var" ) == 0 ) {
	par->operation = _VAR_;
      }

      else if ( strcmp ( argv[i], "-lts-fraction" ) == 0 
		|| strcmp ( argv[i], "-lts-cut" ) == 0) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -lts-fraction ...", 0 );
	status = sscanf( argv[i], "%lf", &(par->lts_fraction) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -lts-fraction ...", 0 );
      }
      
      else if ( strcmp ( argv[i], "-quantile-value" ) == 0 
		|| (strcmp ( argv[i], "-q" ) == 0  && argv[i][2] == '\0')  
		|| (strcmp ( argv[i], "-qv" ) == 0  && argv[i][3] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -quantile-value ...", 0 );
	status = sscanf( argv[i], "%lf", &(par->quantile) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -quantile-value ...", 0 );
      }


      else if ( strcmp (argv[i], "-window" ) == 0 
		|| (strcmp (argv[i], "-w") == 0  && argv[i][2] == '\0') ) {
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -window %d ...\n", 0  );
	status = sscanf( argv[i], "%d", &(par->window.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -window %d", 0 );
	i ++;
	if ( i >= argc)    VT_ErrorParse( "parsing -window %d %d", 0 );
	status = sscanf( argv[i], "%d", &(par->window.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -window %d %d", 0 );
	i ++;
	if ( i >= argc) par->window.z = 1;
	else {
	  status = sscanf( argv[i], "%d", &(par->window.z) );
	  if ( status <= 0 ) {
	    i--;
	    par->window.z = 1;
	  }
	}
      }


      else if ( (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
	par->print_time = 1;
      }
      else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')  
		|| (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
	par->print_time = 0;
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
  par->type = TYPE_UNKNOWN;
  par->window.x =  par->window.y = par->window.z = 3;
  par->operation = _MEAN_;

  par->quantile = 0.50;
  par->lts_fraction = 0.75;

  par->print_time = 1;
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
