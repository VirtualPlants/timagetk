/*************************************************************************
 * changevals.c -
 *
 * $Id: changevals.c,v 1.5 2006/04/14 08:37:38 greg Exp $
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
 *
 */
#include <math.h>
#include <vt_common.h>
#include <stdlib.h>

#include <time.h>

typedef enum {
  _VT_NONE = 0,
  _VT_LINEAR = 1,
  _VT_RANDOM = 2,
  _VT_GREY   = 3,
  _VT_NORMA  = 4,
  _VT_GAMMA  = 5
} typeInterpolation;

typedef enum {
  _NO_PROCESSING = 0,
  _VT_FICHIER = 1,
  _VT_VALUES = 2,
  _VT_INTERVAL = 3,
  _VT_TLINEAR  = 4
} typeTraitement;

#define TAB_LENGTH 131072 /* 2 * 65356 */

typedef struct local_par {
    vt_names names;
    typeTraitement type_traitement;
    typeInterpolation type_interpolation;
    int value_1;
    int value_2;
  double a;
  double b;
  double gamma;

  bufferType type;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void  VT_Parse( int argc, char *argv[], local_par *par );
static void  VT_ErrorParse( char *str, int l );
static void  VT_InitParam( local_par *par );
static void _VT_FillTable( int table[131072], int type_interpolation, 
			   double gamma,
			   int type_image );




static char *usage = "[image-in] [image-out] [-inv] [-swap]\n\
\t [[-changes %s | -modify %d %d] [-linear | -zero | -random | -grey | -gamma %lf]]\n\
\t [-interval %d %d [-norma]] [-a %lf -b %lf]  [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t image-in  : image d'entree (images sur 1 ou 2 octets)\n\
\t image-out : image de sortie (meme type que image-in)\n\
\t -changes %s : contient des lignes de la forme \" %d : %d\\n\"\n\
\t               soit \" old : new\" ce qui signifie que la valeur old\n\
\t               sera changee en la valeur new.\n\
\t -modify %d %d : change la valeur old en la valeur new.\n\
\t -linear     : interpolation lineaire entre les valeurs donnees\n\
\t               si les valeurs extremes ne sont pas precisees, elles\n\
\t               restent inchangees.\n\
\t -gamma %lf  : correction 'gamma' (x -> x^gamma) entre les valeurs donnees\n\
\t               gamma < 1 : favorise les petites valeurs\n\
\t               gamma = 1 : interpolation lineaire\n\
\t               gamma > 1 : favorise les grandes valeurs\n\
\t -zero       : les valeurs non specifiees sont mises a 0.\n\
\t -random     : les valeurs non specifiees sont choisies au hasard.\n\
\t -grey       : les valeurs non specifiees sont inchangees (defaut).\n\
\t -interval %d %d : les valeurs strictement au-dessous du seuil bas sont\n\
\t                   mises a la valeur minimale possible, les valeurs\n\
\t                   strictement au-dessus du seuil haut sont mises a\n\
\t                   a la valeur maximale possible, les autres restent\n\
\t                   inchangees.\n\
\t -norma        : normalisation des valeurs (avec -interval).\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s : short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n\
\n\
 $Revision: 1.5 $ $Date: 2006/04/14 08:37:38 $ $Author: greg $\n";




static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
    local_par par;
    vt_image *image, imres;
    int *table = (int*)NULL;
    int offset, min, max, val_min, val_max;
    register int i, v;
    double d;
    FILE *fopen(),*fin;
    int readLines, goodLines, ret, old, new;
    
    /*--- initialisation des parametres ---*/
    VT_InitParam( &par );
    /*--- lecture des parametres ---*/
    VT_Parse( argc, argv, &par );
    
    switch ( par.type_traitement ) {
    case _VT_FICHIER :
    case _VT_VALUES :
    case _VT_INTERVAL :
    case _VT_TLINEAR :
	break;
    default :
	VT_ErrorParse( "it seems there is nothing to do\n", 0 );
    }
    
    /*--- lecture de l'image d'entree ---*/
    image = _VT_Inrimage( par.names.in );
    if ( image == (vt_image*)NULL ) 
	VT_ErrorParse("unable to read input image\n", 0);
    
    /*--- operations eventuelles sur l'image d'entree ---*/
    if ( par.names.inv == 1 )  VT_InverseImage( image );
    if ( par.names.swap == 1 ) VT_SwapImage( image );
    
    /*--- copie du nom de l'image de sortie ---*/
    if ( VT_CopyName( image->name, par.names.out ) == 0 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to copy output image name into image header", 0 );
    }
    
    /*--- type de l'image ---*/
    min = max = offset = 0;
    switch ( image->type ) {
    case SCHAR :
	min = -128;
	max = 127;
	break;
    case UCHAR :
	min = 0;
	max = 255;
	break;
    case SSHORT :
	min = -32768;
	max = 32767;
	break;
    case USHORT :
	min = 0;
	max = 65535;
	break;
    default :
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to deal with such image type", 0 );
    }
    offset = min;

    /*--- allocation tableau ---*/
    table = (int*)VT_Malloc( (unsigned int)(TAB_LENGTH * sizeof(int)) );
    if ( table == (int*)NULL ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to allocate auxiliary array", 0 );
    }
    /*--- initialisation ---*/
    for ( i = 0; i < TAB_LENGTH; i ++ )
	table[i] = 0;
    

    switch ( par.type_traitement ) {

    case _VT_INTERVAL :
	if ( par.value_1 < par.value_2 ) val_min = par.value_1;
	else                             val_min = par.value_2;
	if ( par.value_1 > par.value_2 ) val_max = par.value_1;
	else                             val_max = par.value_2;
	/*--- mise a zero a l'exterieur de l'intervalle---*/
	for ( i = 0; i < (val_min - offset); i ++ )
	    table[i] = min;
	for ( i = (val_max + 1 - offset); i < TAB_LENGTH; i ++ )
	    table[i] = max;
	
	if ( (par.type_interpolation != _VT_NORMA) || (val_min == val_max) ) {
	    for ( i = (val_min - offset); i <= (val_max - offset); i ++ )
		table[i] = i + offset;
	} else {
	    /*---
	      val_min           -> min + 1
	      val_max           -> max - 1
	      val_max - val_min -> max - min - 2
	      X - val_min       -> Y - (min + 1)
	      
	      Y = (min + 1) + (X - val_min) * (max - min - 2) / (val_max - val_min)
	      ---*/
	    for ( i = (val_min - offset); i <= (val_max - offset); i ++ ) {
		d  = (double)(i + offset - val_min);
		d *= ((double)(max - min - 2)) / ((double)(val_max - val_min));
		d += (double)min + 1.0;
		if ( d >= 0.0 ) table[i] = (int)(d + 0.5);
		else            table[i] = (int)(d - 0.5);
	    }
	}
	break;

    case _VT_FICHIER :
	/*--- lecture du fichier ---*/
	fin = fopen( par.names.ext, "r" );
	readLines = goodLines = 0;
	while ( (ret = fscanf( fin, "%d : %d\n", &old, &new)) != EOF ) {
	  /*
	    if ( ret != 2 ) {
		VT_FreeImage( image );
		VT_Free( (void**)&image );
		VT_Free( (void**)&table );
		VT_ErrorParse("error while reading file of changes\n", 0);
	    }
	    */
	  readLines ++;
	  if ( ret == 2 ) {
	    table[old - offset]         = new;
	    table[65536 + old - offset] = 255;
	    goodLines ++;
	  }
	}
	fclose( fin );
	if ( _VT_DEBUG_ ) {
	  char text[256];
	  sprintf( text, "%d read lines, %d good lines", readLines, goodLines );
	  VT_Message( text, program );
	}
	/*--- on complete la table ---*/
	_VT_FillTable( table, par.type_interpolation, par.gamma, (int)image->type );

	break;
	
    case _VT_VALUES :

      table[ par.value_1 - offset] = par.value_2;
      table[65536 + par.value_1 - offset] = 255;
      _VT_FillTable( table, par.type_interpolation, par.gamma, (int)image->type );
      
      break;


   case _VT_TLINEAR :

     for ( i = min; i <= max; i ++ ) {

       d = par.a * i + par.b;
       if ( d > 0 ) {
	 v = (int)(d + 0.5);
       }
       else {
	 v = (int)(d - 0.5);
       }

       if ( v > max ) v = max;
       if ( v < min ) v = min;
       
       table[i - offset] = v;

     }
     break;
	
    default :
	VT_ErrorParse( "there is nothing to do\n", 0 );
    }
    
    if (  _VT_DEBUG_ ) {
      for ( i = min; i <= max; i ++ ) {
	fprintf( stderr, "%6d -> %6d\n", i, table[i-offset] );
      }
    }

    /*--- la table est construite : modification de l'image ---*/
    v = image->dim.x * image->dim.y * image->dim.z;
    switch ( image->type ) {
    default :
      VT_ErrorParse( "such image type is not handled\n", 0 );
    case SCHAR :
        { 
	s8 *buf;
	buf = (s8*)(image->buf);
	for ( i = 0; i < v; i++ ) {
	    ret = buf[i];
	    buf[i] = (s8)table[ret - offset];
	}}
	break;
    case UCHAR :
        { 
	u8 *buf;
	buf = (u8*)(image->buf);
	for ( i = 0; i < v; i++ ) {
	    ret = buf[i];
	    buf[i] = (u8)table[ret - offset];
	}}
	break;
    case SSHORT :
        { 
	s16 *buf;
	buf = (s16*)(image->buf);
	for ( i = 0; i < v; i++ ) {
	    ret = buf[i];
	    buf[i] = (s16)table[ret - offset];
	}}
	break;
    case USHORT :
        { 
	u16 *buf;
	buf = (u16*)(image->buf);
	for ( i = 0; i < v; i++ ) {
	    ret = buf[i];
	    buf[i] = (u16)table[ret - offset];
	}}
    }

    VT_Free( (void**)&table );

    /*--- ecriture de l'image resultat ---*/
    if ( par.type == TYPE_UNKNOWN ||  par.type == image->type ) {
      
      if ( VT_WriteInrimage( image ) == -1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to write output image\n", 0);
      }

    }
    else {

      VT_Image( &imres );
      VT_InitFromImage( &imres, image, par.names.out, par.type );
      if ( VT_AllocImage( &imres ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to allocate output image\n", 0);
      }
      if ( VT_CopyImage( image, &imres ) != 1 ) {
	VT_FreeImage( image );
	VT_FreeImage( &imres );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to copy input image\n", 0);
      }
      if ( VT_WriteInrimage( &imres ) == -1 ) {
	VT_FreeImage( image );
	VT_FreeImage( &imres );
	VT_Free( (void**)&image );
	VT_ErrorParse("unable to write output image\n", 0);
      }
      VT_FreeImage( &imres );

    }
    
    /*--- liberations memoires ---*/
    VT_FreeImage( image );
    VT_Free( (void**)&image );

    exit( 0 );
}




static void VT_Parse( int argc, char *argv[], local_par *par )
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
                else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
                    par->names.inv = 1;
                }
                else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
                    par->names.swap = 1;
                }
                else if ( strcmp ( argv[i], "-v" ) == 0 ) {
                    _VT_VERBOSE_ = 1;
                }
                else if ( strcmp ( argv[i], "-D" ) == 0 ) {
                    _VT_DEBUG_ = 1;
		}
		/*--- fichier ? ---*/
		else if ( strcmp ( argv[i], "-changes" ) == 0 ) {
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -changes...\n", 0 );
		    strncpy( par->names.ext, argv[i], STRINGLENGTH );
		    par->type_traitement = _VT_FICHIER;
                }
		/*--- valeurs ? ---*/
		else if ( strcmp ( argv[i], "-modify" ) == 0 ) {
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -modify...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->value_1) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -modify...\n", 0 );
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -modify...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->value_2) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -modify...\n", 0 );
		    par->type_traitement = _VT_VALUES;
		}
		/*--- intervalle ? ---*/
		else if ( strcmp ( argv[i], "-interval" ) == 0 ) {
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -interval...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->value_1) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -interval...\n", 0 );
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -interval...\n", 0 );
		    status = sscanf( argv[i],"%d",&(par->value_2) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -interval...\n", 0 );
		    par->type_traitement = _VT_INTERVAL;
		}
		/*--- lineaire ? ---*/
		else if ( strcmp ( argv[i], "-a" ) == 0 ) {
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -a...\n", 0 );
		    status = sscanf( argv[i],"%lf",&(par->a) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -a...\n", 0 );
		    par->type_traitement = _VT_TLINEAR;
		}
		else if ( strcmp ( argv[i], "-b" ) == 0 ) {
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -b...\n", 0 );
		    status = sscanf( argv[i],"%lf",&(par->b) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -b...\n", 0 );
		    par->type_traitement = _VT_TLINEAR;
		}
		/*--- type d'interpolation ---*/
		else if ( strcmp ( argv[i], "-zero" ) == 0 ) {
                    par->type_interpolation = _VT_NONE;
                }
		else if ( strcmp ( argv[i], "-linear" ) == 0 ) {
                    par->type_interpolation = _VT_LINEAR;
                }
		else if ( strcmp ( argv[i], "-gamma" ) == 0 ) {
		    i += 1;
                    if ( i >= argc)    VT_ErrorParse( "parsing -gamma...\n", 0 );
		    status = sscanf( argv[i],"%lf",&(par->gamma) );
		    if ( status <= 0 ) VT_ErrorParse( "parsing -gamma...\n", 0 );
                    par->type_interpolation = _VT_GAMMA;
                }
		else if ( strcmp ( argv[i], "-random" ) == 0 ) {
                    par->type_interpolation = _VT_RANDOM;
                }
		else if ( strcmp ( argv[i], "-grey" ) == 0 ) {
                    par->type_interpolation = _VT_GREY;
                }
		else if ( strcmp ( argv[i], "-norma" ) == 0 ) {
                    par->type_interpolation = _VT_NORMA;
                }
		/*--- lecture du type de l'image ---*/
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
		/*--- sinon ---*/
		else {
                    sprintf(text,"unknown option %s\n",argv[i]);
                    VT_ErrorParse(text, 0);
                }
            }
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
	
	/*--- noms des images ---*/
        if (nb == 0) {
	    strcpy( par->names.in,  "<" );  /* standart input */
                strcpy( par->names.out, ">" );  /* standart output */
        }
        if (nb == 1)
                strcpy( par->names.out, ">" );  /* standart output */
       
	/*--- type de l'image resultat ---*/
	if ( (o == 1) && (s == 1) && (r == 0) ) par->type = SCHAR;
	if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
	if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
	if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
	if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
	if ( (o == 0) && (s == 0) && (r == 1) )  par->type = FLOAT;
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
	par->type_traitement = _NO_PROCESSING;
	par->type_interpolation = _VT_GREY;
	par->value_1 = par->value_2 = 0;
	par->a = 1.0;
	par->b = 0.0;
	par->gamma = 1.0;
	par->type = TYPE_UNKNOWN;
}



static void _VT_FillTable( int table[131072], int type_interpolation, 
			   double gamma, int type_image )
{
    int min, max, offset;
    register int i;
    int first=0, nb, next;
    double x, val, mul;

    switch ( type_interpolation ) {
    case _VT_LINEAR :
    case _VT_GAMMA :
    case _VT_RANDOM :
    case _VT_GREY :
	break;
    default :
	return;
    }

    min = max = offset = 0;
    /*--- type de l'image ---*/
    switch ( type_image ) {
    case SCHAR :
	min = -128;
	max = 127;
	break;
    case UCHAR :
	min = 0;
	max = 255;
	break;
    case SSHORT :
	min = -32768;
	max = 32767;
	break;
    case USHORT :
	min = 0;
	max = 65535;
	break;
    default :
	    return;
    }
    offset = min;

    switch ( type_interpolation ) {
    case _VT_LINEAR :
    case _VT_GAMMA :
	/*--- si le premier point n'est pas specifie ---*/
	if ( table[ 65536 ] == 0 ) {
	    table[ 0 ] = min;
	    table[ 65536 ] = 255;
	}
	/*--- si le dernier point n'est pas specifie ---*/
	switch ( type_image ) {
	case SCHAR :
	case UCHAR :
	    if ( table[ 65536 + 255 ] == 0 ) {
		table[255] = max;
		table[ 65536 + 255 ] = 255;
	    }
	case SSHORT :
	case USHORT :
	    if ( table[ 131071 ] == 0 ) {
	    table[ 65535 ] = max;
	    table[ 131071 ] = 255;
	    }
	}
	    
        first = 0;
	while ( first < 65535 ) {
	    /*--- recherche du suivant ---*/
            nb = 0;
	    while ( table[65536 + first + (++nb)] == 0 );
	    next = first + nb;
	    /*--- interpolation ---*/
	    for ( i = 1; i < nb; i ++ ) {
	      /* valeur entre 0 et 1 */
	      x = (double)i / (double)nb;
	      /* correction gamma eventuelle */
	      if ( type_interpolation == _VT_GAMMA && gamma != 1.0 ) {
		x = exp( gamma * log( x ) );
	      }
	      /* interpolation */
	      val = (double)table[first] + (double)(table[next] - table[first]) * x;
	      if ( val >= 0.0 ) table[first + i] = (int)( val + 0.5 );
	      else              table[first + i] = (int)( val - 0.5 );
	      /* vieux code =
		val = (double)(nb-i) * (double)table[first] + (double)i * (double)table[next];
		if ( val >= 0.0 ) table[first + i] = (int)( ( val / (double)nb ) + 0.5 );
		else              table[first + i] = (int)( ( val / (double)nb ) - 0.5 );
	      */
	    }
	    /*--- suivant ---*/
	    first = next;
	}

	break;

    case _VT_RANDOM :

      (void)srandom(time(0));

	mul = ((double)(max - min)) / (double)2147483647.0;
	for ( i = 0; i < 65535 ; i ++ ) {
	    if ( table[ 65536 + i] != 0 ) continue;
	    val = (double)min + random() * mul;
	    if ( val >= 0.0 ) table[i] = (int)( val + 0.5 );
	    else              table[i] = (int)( val - 0.5 );
	}

	break;

    case _VT_GREY :
	for ( i = 0; i < 65535 ; i ++ ) {
	    if ( table[ 65536 + i] != 0 ) continue;
	    table[i] = i + offset;
	}
	
    }
}
