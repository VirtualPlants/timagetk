/*************************************************************************
 * Ctable.c -
 *
 * $Id: Ctable.c,v 1.6 2002/12/11 12:04:03 greg Exp $
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
 * - Fri Mar 31 13:13:18 MET DST 2000, Gregoire Malandain
 *   ajout des tables de longueur 65536 pour les images sur 2 octets;
 */

#include <vt_common.h>
#include <stdlib.h>
#include <time.h>

typedef enum {
  _INRIMAGE_,
  _TCLMAP_,
  _MATLAB_
} OutputType;


typedef enum {
  _NULL_ = 0,
  _LINEAR_ = 1,
  _RANDOM_ = 2,
  _GREY_ = 3 
} InterpolationType;

typedef enum {
  DEF_NONE = 0,
  DEF_GORIS = 1,
  DEF_RANDOM = 2
} PredefinedType;


typedef struct local_par {
  vt_names names;
  InterpolationType type_interpolation;
  PredefinedType type_predefined;
  int black_first;
  int black_inter;
  int black_perio;
  int type;

  OutputType type_output;
  char TCLNAME[STRINGLENGTH];
  unsigned char overflow[3];

} local_par;

/*------- Definition des fonctions statiques ----------*/
static void  VT_Parse( int argc, char *argv[], local_par *par );
static void  VT_ErrorParse( char *str, int l );
static void  VT_InitParam( local_par *par );
static void _VT_PredefinedTable( unsigned char *table, int dim, int type_predefined );
static void _VT_FillTable( unsigned char *table, int dim, int type_interpolation );



static char *usage = "[fichier-in | -create fichier-in] [table-out | -tc table-out]\n\
\t [-linear | -zero | -random | -grey] [-black %d %d %d]\n\
\t [-inr |-tcl|-matlab] [-tclname %s] [-overflow %d %d %d]\n\
\t [-all_random | -all_goris] [-v] [-D] [-help]";


static char *detail = "\
\t fichier-in : contient des lignes de la forme \" %d : %d %d %d\\n\"\n\
\t              soit \" i : r g b\" ce qui signifie que les composantes\n\
\t              RGB de l'intensite i (entre 0 et 255) sont r, g et b.\n\
\t table-out : table couleur = image inrimage 256x3 sur un octet\n\
\t             premiere  ligne = composante rouge\n\
\t             deuxieme  ligne = composante verte\n\
\t             troisieme ligne = composante bleue\n\
\t -linear : interpolation lineaire entre les couleurs donnees (defaut)\n\
\t           si l'entree 0   n'existe pas c'est (  0,  0,  0)\n\
\t           si l'entree 255 n'existe pas c'est (255,255,255)\n\
\t -zero   : les entrees manquantes sont mises a (0,0,0)\n\
\t -random : les entrees manquantes sont choisies au hasard\n\
\t -grey   : les entrees manquantes sont mises au niveau de gris correspondant (i,i,i)\n\
\t -black %d %d %d : premier a noir, taille de l'intervalle a noir, periode\n\
\t -all_random : toute la table est choisie au hasard\n\
\t -all_goris  : coefficients utilises par Jerome Declerck a Stanford\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n\
 $Revision: 1.6 $ $Date: 2002/12/11 12:04:03 $ $Author: greg $\n";

static char program[STRINGLENGTH];

static int INIT_GORIS[8][4] = {
    {   0,   0,   0,   0 },
    {  30,   0,   0,  64 },
    {  80,   0, 150,  50 },
    { 135, 255, 255,   0 },
    { 160, 200,  50,  50 },
    { 210, 255,   0,   0 },
    { 254, 254, 254, 254 },
    { 255, 100,   0,   0 } 
};


int main( int argc, char *argv[] )
{
	local_par par;
	vt_image imres;
	int dim=256;
	unsigned char *buf, *table = (unsigned char*)NULL;
	int i, j, r, g, b, ret;
	FILE *fopen(),*fin;
	
	/*--- initialisation des parametres ---*/
	VT_InitParam( &par );
	/*--- lecture des parametres ---*/
	VT_Parse( argc, argv, &par );

	if ( par.type == USHORT ) dim *= dim;


	/*--- initialisation ---*/
	table = (unsigned char *)malloc( dim*4*sizeof( unsigned char ));
	if ( table == (unsigned char*)NULL ) {
	  VT_ErrorParse("unable to allocate table\n", 0);
	}
	for ( i = 0; i < 4*dim; i++ ) table[i] = (unsigned char)0;

	if ( par.type_predefined != DEF_NONE ) {
	    /*--- erreur ---*/
	    if ( (par.names.out[0] == '\0') || (par.names.out[0] == '>') ) {
	      if ( (par.names.in[0] == '\0') || (par.names.in[0] == '<') ) {
		  free( table );
		  VT_ErrorParse("bad output image\n", 0);
	      }
	      else
		strncpy( par.names.out,par.names.in, STRINGLENGTH );  
	    }
	    /*--- on remplit ---*/
	    _VT_PredefinedTable( table, dim, par.type_predefined );
	} else {
	    /*--- erreur ---*/
	    if ( (par.names.in[0] == '\0') || (par.names.in[0] == '<') ) {
	      free( table );
	      VT_ErrorParse("bad input image\n", 0);
	    }
	    if ( (par.names.out[0] == '\0') || (par.names.out[0] == '>') ) {
	      free( table );
	      VT_ErrorParse("bad output image\n", 0);
	    }
	    /*--- fichier entree ---*/
	    fin = fopen( par.names.in, "r" );
	    while ( (ret = fscanf( fin, "%d : %d %d %d\n", &i, &r, &g, &b)) != EOF ) {
		if ( ret != 4 ) VT_ErrorParse("error in reading input file\n", 0);
		table[i]       = (unsigned char)r;
		table[dim + i] = (unsigned char)g;
		table[2*dim + i] = (unsigned char)b;
		table[3*dim + i] = (unsigned char)255;
	    }
	    fclose( fin );
	    /*--- on complete ---*/
	    _VT_FillTable( table, dim, par.type_interpolation );
	}

	if ( par.black_perio > 0 ) {
	  for (i = par.black_first; i < dim; i += par.black_perio ) {
	    if ( i < 0 ) continue;
	    for ( j = i; j < (i + par.black_inter); j ++ )
	      table[j] = table[dim + j] = table[2*dim + j] = 0;
	  }
	}

	/*--- initialisation de l'image resultat ---*/
        VT_Image( &imres );
        VT_InitImage( &imres, par.names.out, (int)dim, (int)3, (int)1, (int)UCHAR );
        if ( VT_AllocImage( &imres ) != 1 ) {
	  free( table );
	  VT_ErrorParse("unable to allocate output image\n", 0);
        }
	
	buf = (unsigned char*)(imres.buf);
	for ( i = 0; i < 3*dim; i ++ ) {
	    buf[i] = table[i];
	}

	free( table );
	/*--- ecriture de l'image resultat ---*/
	switch( par.type_output ) {
	default :
	case _INRIMAGE_ :
	  if ( VT_WriteInrimage( &imres ) == -1 ) {
	    VT_FreeImage( &imres );
	    VT_ErrorParse("unable to write output image\n", 0);
	  }
	  break;
	case _MATLAB_ :
	  fin = fopen( par.names.out, "w" );
	  fprintf( fin, "%%----------------------------\n");
	  fprintf( fin, "function mytable= myColorMap\n");
	  fprintf( fin, "%%----------------------------\n");
	  fprintf( fin, "\n" );
	  fprintf( fin, "mytable = [   %6.4g %6.4g %6.4g ;...\n",
		   (double)buf[0*dim +0]/(double)(dim-1), 
		   (double)buf[1*dim +0]/(double)(dim-1), 
		   (double)buf[2*dim +0]/(double)(dim-1) );
	  for ( i=0; i<dim-1; i++ )
	    fprintf( fin, "              %6.4g %6.4g %6.4g ;...\n",
		     (double)buf[0*dim +i]/(double)(dim-1), 
		     (double)buf[1*dim +i]/(double)(dim-1), 
		     (double)buf[2*dim +i]/(double)(dim-1) );
	    fprintf( fin, "              %6.4g %6.4g %6.4g ];\n",
		     (double)buf[0*dim +dim-1]/(double)(dim-1), 
		     (double)buf[1*dim +dim-1]/(double)(dim-1), 
		     (double)buf[2*dim +dim-1]/(double)(dim-1) );
	  
	  fclose( fin);
	  break;
	case _TCLMAP_ :
	  fin = fopen( par.names.out, "w" );
	  fprintf( fin, "\n" );
	  fprintf( fin, "unsigned char\n" );
	  fprintf( fin, "TclInrimageBackground::%sColorMapRed[257]={",
		   par.TCLNAME );
	  for (i=0;i<dim;i++ ) fprintf( fin, "%d,", buf[i] );
	  fprintf( fin, "%d};\n\n", par.overflow[0] );
	  fprintf( fin, "TclInrimageBackground::%sColorMapGreen[257]={",
		   par.TCLNAME );
	  for (i=0;i<dim;i++ ) fprintf( fin, "%d,", buf[dim+i] );
	  fprintf( fin, "%d};\n\n", par.overflow[1] );
	  fprintf( fin, "TclInrimageBackground::%sColorMapBlue[257]={",
		   par.TCLNAME );
	  for (i=0;i<dim;i++ ) fprintf( fin, "%d,", buf[2*dim+i] );
	  fprintf( fin, "%d};\n\n", par.overflow[2] );
	  fclose( fin);
	}
		
	/*--- liberations memoires ---*/
        VT_FreeImage( &imres );
	return( 1 );
}





static void VT_Parse( int argc, char *argv[], local_par *par )
{
	int i, status;
	int bool_in = 0, bool_out = 0;
	char text[STRINGLENGTH];
	int tmp, o=1;

	if ( VT_CopyName( program, argv[0] ) != 1 )
		VT_Error("Error while copying program name", (char*)NULL);
	if ( argc == 1 ) VT_ErrorParse("\n", 0 );

	/*--- lecture des parametres ---*/
	i = 1;
	while ( i < argc ) {
	    if ( argv[i][0] == '-' ) {
		if ( argv[i][1] == '\0' ) {
		    if ( bool_in == 0 ) {
			/*--- standart input ---*/
			strcpy( par->names.in, "<" );
			bool_in = 1;
		    }
		    else VT_ErrorParse( "parsing - ...\n", 0 );
		}
		/*--- options generales ---*/
		else if ( strcmp ( argv[i], "-help" ) == 0 ) {
		    VT_ErrorParse("\n", 1);
		}
		else if ( strcmp ( argv[i], "-v" ) == 0 ) {
		    _VT_VERBOSE_ = 1;
		}
		else if ( strcmp ( argv[i], "-D" ) == 0 ) {
		    _VT_DEBUG_ = 1;
		}
		/*--- fichiers ---*/
		else if ( strcmp ( argv[i], "-tc" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -tc...\n", 0 );
		    if ( bool_out == 0 ) {
			strncpy( par->names.out, argv[i], STRINGLENGTH );
			bool_out = 1;
		    } else             VT_ErrorParse( "parsing -tc...\n", 0 );
		}
		else if ( strcmp ( argv[i], "-create" ) == 0 ) {
		    i += 1;
		    if ( i >= argc)    VT_ErrorParse( "parsing -create...\n", 0 );
		    if ( bool_in == 0 ) {
			strncpy( par->names.in, argv[i], STRINGLENGTH );
			bool_in = 1;
		    } else             VT_ErrorParse( "parsing -create...\n", 0 );
		}




		else if ( strcmp ( argv[i], "-inr" ) == 0 ) {
		    par->type_output = _INRIMAGE_;
		}
		else if ( strcmp ( argv[i], "-matlab" ) == 0 ) {
		  par->type_output = _MATLAB_;
		}
		else if ( strcmp ( argv[i], "-tcl" ) == 0 ) {
		  par->type_output = _TCLMAP_;
		}
		else if ( strcmp ( argv[i], "-tclname" ) == 0 ) {
		  i += 1;
		  if ( i >= argc)    VT_ErrorParse( "parsing -tclname...\n", 0 ); 
		  strncpy( par->TCLNAME, argv[i], STRINGLENGTH );
		}
		else if ( strcmp ( argv[i], "-overflow" ) == 0 ) {
		  if ( i < argc - 3 ) {
		    i += 1;
		    status = sscanf( argv[i],"%d", &tmp );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -overflow ...\n", 0  );
		    par->overflow[0] = tmp;
		    i += 1;
		    status = sscanf( argv[i],"%d", &tmp );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -overflow ...\n", 0  );
		    i += 1;
		    par->overflow[1] = tmp;
		    status = sscanf( argv[i],"%d", &tmp );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -overflow ...\n", 0  );
		    par->overflow[2] = tmp;
		    
		  } else 
		    VT_ErrorParse( "parsing -black ...\n", 0  );
		}

		/*--- type d'interpolation ---*/
		else if ( strcmp ( argv[i], "-linear" ) == 0 ) {
		    par->type_interpolation = _LINEAR_;
		}
		else if ( strcmp ( argv[i], "-random" ) == 0 ) {
		    par->type_interpolation = _RANDOM_;
		}
		else if ( strcmp ( argv[i], "-grey" ) == 0 ) {
		    par->type_interpolation = _GREY_;
		}
		else if ( strcmp ( argv[i], "-zero" ) == 0 ) {
		    par->type_interpolation = _NULL_;
		}
		/* */
		else if ( strcmp ( argv[i], "-black" ) == 0 ) {
		  if ( i < argc - 3 ) {
		    i += 1;
		    status = sscanf( argv[i],"%d", &(par->black_first) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -black ...\n", 0  );
		    i += 1;
		    status = sscanf( argv[i],"%d", &(par->black_inter) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -black ...\n", 0  );
		    i += 1;
		    status = sscanf( argv[i],"%d", &(par->black_perio) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -black ...\n", 0  );
		  } else 
		    VT_ErrorParse( "parsing -black ...\n", 0  );
		}
		/*--- tables predefinies ---*/
		else if ( strcmp ( argv[i], "-all_random" ) == 0 ) {
		    par->type_predefined = DEF_RANDOM;
		}
		else if ( strcmp ( argv[i], "-all_goris" ) == 0 ) {
		    par->type_predefined = DEF_GORIS;
		}

/*--- lecture du type de l'image de sortie ---*/
else if ( strcmp ( argv[i], "-o" ) == 0 ) {
i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -o...\n", 0 );
	status = sscanf( argv[i],"%d",&o );
	if ( status <= 0 ) VT_ErrorParse( "parsing -o...\n", 0 );
      }
		/*--- erreur ---*/
		else {
		    sprintf(text,"unknown option %s\n",argv[i]);
		    VT_ErrorParse(text, 0);
		}
	    }
	    else if ( argv[i][0] != 0 ) {
		if ( bool_in == 0 ) {
		    strncpy( par->names.in, argv[i], STRINGLENGTH );  
		    bool_in = 1;
		} else if ( bool_out == 0 ) {
		    strncpy( par->names.out, argv[i], STRINGLENGTH );  
		    bool_out = 1;
		} else
		    VT_ErrorParse("too much file names when parsing\n", 0 );
	    }
	    i += 1;
	}

	if ( bool_in == 0 )
	    strcpy( par->names.in, "<" );  /* standart input */
	if ( bool_out == 0 )
	    strcpy( par->names.out, ">" );  /* standart output */
	
	switch( o ) {
	case 2 : par->type = USHORT; break;
	default :
	case 1 : par->type = UCHAR; break;	  
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
  par->type_interpolation = _LINEAR_;
  par->type_predefined = DEF_NONE;
  par->black_first = par->black_inter = par->black_perio = 0;

  par->type = TYPE_UNKNOWN;

  par->type_output = _INRIMAGE_;
  sprintf(par->TCLNAME, "PET" );
  par->overflow[0] = par->overflow[1] = par->overflow[2] = 255;
}





static void _VT_PredefinedTable( unsigned char *table, int dim, 
				 int type_predefined )
{
    int i;
    unsigned char *red, *green, *blue, *mark;
    
    red   = table;
    green = table + dim;
    blue  = table + 2*dim; 
    mark  = table + 3*dim;
    for ( i = 0; i < 4*dim; i++ ) table[i] = (unsigned char)0;

    switch ( type_predefined ) {
    case DEF_GORIS :
	for ( i = 0 ; i < 8; i ++ ) {
	    red[ INIT_GORIS[i][0] ]   = INIT_GORIS[i][1];
	    green[ INIT_GORIS[i][0] ] = INIT_GORIS[i][2];
	    blue[ INIT_GORIS[i][0] ]  = INIT_GORIS[i][3];
	    mark[ INIT_GORIS[i][0] ]  = (unsigned char)255;
	}
	_VT_FillTable( table, dim, (int)_LINEAR_ );
	break;
    case DEF_RANDOM :
    default :
	_VT_FillTable( table, dim, (int)_RANDOM_ );
    }
}





static void _VT_FillTable( unsigned char *table, int dim,
			   int type_interpolation )
{
    unsigned char *red, *green, *blue, *mark;
    double val, max, inv;
    int i, first=0, nb, next;

    red   = table;
    green = table + dim;
    blue  = table + 2*dim; 
    mark  = table + 3*dim;

    switch ( type_interpolation ) {
    case _NULL_ :
	for ( i = 0; i < dim; i ++ ) {
	    if ( mark[i] != (unsigned char)0 ) continue;
	    red[i] = green[i] = blue[i] = (unsigned char)0;
	}
	break;
    case _RANDOM_ :

        (void)srandom(time(0));

	max = (double)2147483647.0; /* 2^31 - 1 : valeur max atteinte */
	inv = (double)255.0 / (double)2147483647.0;

	for ( i = 0; i < dim; i ++ ) {
	    if ( mark[i] != (unsigned char)0 ) continue;
	    do {
	      red[i]   = (unsigned char)( (int)( random() * inv + 0.5 ) );
	      green[i] = (unsigned char)( (int)( random() * inv + 0.5 ) );
	      blue[i]  = (unsigned char)( (int)( random() * inv + 0.5 ) );
	      if ( 0 ) fprintf( stderr, "#%3d = %3d %3d %3d\n", i, red[i], green[i], blue[i] );
	    } while ( ((-100 < red[i] - green[i]) && (red[i] - green[i] < 100)
		       && (-100 < red[i] - blue[i]) && (red[i] - blue[i] < 100) 
		       && (-100 < blue[i] - green[i]) && (blue[i] - green[i] < 100))
		      || ( red[i] < 200 && blue[i] < 200 && green[i] < 200 ) );
	    if ( 0 ) fprintf( stderr, "#%3d = %3d %3d %3d\n", i, red[i], green[i], blue[i] );
	}
	break;
    case _GREY_ :
	/*--- si le premier point n'est pas specifie ---*/
	if ( mark[0] == (unsigned char)0 ) {
	    red[0] = green[0] = blue[0] = (unsigned char)0;
	    mark[0] = (unsigned char)255;
	}
	/*--- si le dernier point n'est pas specifie ---*/
	if ( mark[dim-1] == (unsigned char)0 ) {
	    red[dim-1] = green[dim-1] = blue[dim-1] = (unsigned char)255;
	    mark[dim-1] = (unsigned char)255;
	}
	first = 0;
	while ( first < dim-1 ) {
	    /*--- recherche du suivant ---*/
	    nb = 0;
	    while ( mark[first + (++nb)] == (unsigned char)0 );
	    next = first + nb;
	    /*--- niveau de gris---*/
	    for ( i = 1; i < nb; i ++ ) {
		red[first + i]   = (unsigned char)(first+i);
		green[first + i] = (unsigned char)(first+i);
		blue[first + i]  = (unsigned char)(first+i);
	    }
	    /*--- suivant ---*/
	    first = next;
	}
	break;
    case _LINEAR_ :
    default :
	/*--- si le premier point n'est pas specifie ---*/
	if ( mark[0] == (unsigned char)0 ) {
	    red[0] = green[0] = blue[0] = (unsigned char)0;
	    mark[0] = (unsigned char)255;
	}
	/*--- si le dernier point n'est pas specifie ---*/
	if ( mark[dim-1] == (unsigned char)0 ) {
	    red[dim-1] = green[dim-1] = blue[dim-1] = (unsigned char)255;
	    mark[dim-1] = (unsigned char)255;
	}
	first = 0;
	while ( first < dim-1 ) {
	    /*--- recherche du suivant ---*/
	    nb = 0;
	    while ( mark[first + (++nb)] == (unsigned char)0 );
	    next = first + nb;
	    /*--- interpolation ---*/
	    for ( i = 1; i < nb; i ++ ) {
		val = (double)(nb-i) * (double)red[first] + (double)i * (double)red[next];
		red[first + i] = (unsigned char)( (int)( val / (double)(nb) + 0.5 ) );
		val = (double)(nb-i) * (double)green[first] + (double)i * (double)green[next];
		green[first + i] = (unsigned char)( (int)( val / (double)(nb) + 0.5 ) );
		val = (double)(nb-i) * (double)blue[first] + (double)i * (double)blue[next];
		blue[first + i] = (unsigned char)( (int)( val / (double)(nb) + 0.5 ) );
	    }
	    /*--- suivant ---*/
	    first = next;
	}
    }
}
