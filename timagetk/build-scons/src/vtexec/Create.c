/*************************************************************************
 * Create.c -
 *
 * $Id: Create.c,v 1.3 2001/04/13 18:12:12 greg Exp $
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

typedef struct local_par {
  vt_names names;
  vt_4vpt dim;
  vt_fpt voxel;
  vt_fpt offset;
    int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-out]\n\
[-x %d] [-y %d] [-z %d] [-v %d] [-template %s]\n\
[-vs %f %f %f] [-tx %f] [-ty %f] [[-tz %f] options-de-type]\n\
\t [-verbose|-w] [-D] [-help]";

static char *detail = "\
\t si 'image-out' est absent, on prendra stdout\n\
\t -x %d      : taille de l'image selon X (1 par defaut)\n\
\t -y %d      : taille de l'image selon Y (1 par defaut)\n\
\t -z %d      : taille de l'image selon Z (1 par defaut)\n\
\t -v %d      : taille de l'image selon V (1 par defaut)\n\
\t -vs %f %f %f : voxel size\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 1 -s :   signed char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s :   signed short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type 'unsigned char'\n\
\n\
 $Revision: 1.3 $ $Date: 2001/04/13 18:12:12 $ $Author: greg $\n";

static char program[STRINGLENGTH];

int main( int argc, char *argv[] )
{
	local_par par;
        vt_image imres;
        vt_image *imtemplate;

	char header[257];
	int fd ,nb;

	/*--- initialisation des parametres ---*/
	VT_InitParam( &par );
	/*--- lecture des parametres ---*/
	VT_Parse( argc, argv, &par );

        if ( par.names.ext[0] != '\0' ) {
          imtemplate = _VT_Inrimage( par.names.ext );
          if ( imtemplate == (vt_image*)NULL ) {
            VT_ErrorParse("unable to read template image\n", 0);
          }

          VT_InitFromImage( &imres, imtemplate, par.names.out, imtemplate->type );

          VT_FreeImage( imtemplate );
          VT_Free( (void**)&imtemplate );

          if ( par.dim.v > 0 ) imres.dim.v = par.dim.v;
          if ( par.dim.x > 0 ) imres.dim.x = par.dim.x;
          if ( par.dim.y > 0 ) imres.dim.y = par.dim.y;
          if ( par.dim.z > 0 ) imres.dim.z = par.dim.z;

          if ( par.voxel.x > 0.0 ) imres.siz.x = par.voxel.x;
          if ( par.voxel.y > 0.0 ) imres.siz.y = par.voxel.y;
          if ( par.voxel.z > 0.0 ) imres.siz.z = par.voxel.z;

          if ( par.type != TYPE_UNKNOWN ) imres.type = par.type;
        }
        else {
            if ( par.dim.x < 0 && par.dim.y < 0 ) {
              VT_ErrorParse( "negative X and Y dimensions\n", 0);
            }

            if ( par.dim.v < 0 ) par.dim.v = 1;
            if ( par.dim.x < 0 ) par.dim.x = 1;
            if ( par.dim.y < 0 ) par.dim.y = 1;
            if ( par.dim.z < 0 ) par.dim.z = 1;

            if ( par.voxel.x < 0.0 ) par.voxel.x = 1.0;
            if ( par.voxel.y < 0.0 ) par.voxel.y = 1.0;
            if ( par.voxel.z < 0.0 ) par.voxel.z = 1.0;

            if ( par.type == TYPE_UNKNOWN ) par.type = UCHAR;

            VT_InitVImage( &imres, par.names.out, par.dim.v,
                           par.dim.x, par.dim.y, par.dim.z,
                           par.type );

            imres.siz.x = par.voxel.x;
            imres.siz.y = par.voxel.y;
            imres.siz.z = par.voxel.z;
        }


        VT_SetImageOffset( &imres, par.offset.x, par.offset.y, par.offset.z );

	/*--- creation de l'header ---*/
        VT_FillInrimHeader( header, &imres );

	/*--- opening the file ---*/
        fd = VT_WOpen( imres.name );
	if ( fd == -1 ) {
	    VT_ErrorParse("Unable to open file for writing", 0);
	}
	/*--- writing the header ---*/
	nb = VT_Write( fd, header, 256 );
	if ( nb == -1 ) {
	    VT_Close( fd );
	    VT_ErrorParse("error when writing the header", 0);
	}
	if ( nb < 256 ) {
	    VT_Close( fd );
	    VT_ErrorParse("not enough space left for writing the header", 0);
	}

	VT_Close( fd );
	
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
		VT_ErrorParse("parsing - \n", 1);
	    }
	    /*--- options generales ---*/
            else if ( strcmp ( argv[i], "--help" ) == 0
                      || ( strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0' )
                      || ( strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0' )
                      || ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) ) {
		VT_ErrorParse("\n", 1);
	    }
            else if ( strcmp ( argv[i], "-verbose" ) == 0 || strcmp ( argv[i], "-w" ) == 0 ) {
                if ( _VT_VERBOSE_ <= 0 )
                  _VT_VERBOSE_ = 1;
                else
                  _VT_VERBOSE_ ++;
	    }
	    else if ( strcmp ( argv[i], "-D" ) == 0 ) {
                if ( _VT_DEBUG_  <= 0 )
                  _VT_DEBUG_ = 1;
                else
                  _VT_DEBUG_ ++;
	    }

	    /*--- dimension de l'image ---*/
            else if ( strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0' ) {
              i ++;
              if ( i >= argc)    VT_ErrorParse( "parsing -dim %d\n", 0 );
              status = sscanf( argv[i], "%d", &(par->dim.x) );
              if ( status <= 0 ) VT_ErrorParse( "parsing -dim %d\n", 0 );
              i ++;
              if ( i >= argc)    VT_ErrorParse( "parsing -dim %d %d\n", 0 );
              status = sscanf( argv[i], "%d", &(par->dim.y) );
              if ( status <= 0 ) VT_ErrorParse( "parsing -dim %d %d\n", 0 );
              i ++;
              if ( i < argc ) {
                status = sscanf( argv[i], "%d", &(par->dim.z) );
                if ( status <= 0 ) {
                  i--;
                }
              }
            }
	    else if ( strcmp ( argv[i], "-x" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->dim.x) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-y" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->dim.y) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-z" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -z...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->dim.z) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -z...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-tx" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -tx...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->offset.x) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -tx...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-ty" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -ty...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->offset.y) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -ty...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-tz" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -tz...\n", 0 );
		status = sscanf( argv[i],"%f",&(par->offset.z) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -tz...\n", 0 );
	    }
	    else if ( strcmp ( argv[i], "-v" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -v...\n", 0 );
		status = sscanf( argv[i],"%d",&(par->dim.v) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -v...\n", 0 );
	    }

	    else if ( strcmp ( argv[i], "-vs" ) == 0 ) {
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -vs..\n", 0 );
		status = sscanf( argv[i],"%f",&(par->voxel.x) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -vs...\n", 0 );
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -vs..\n", 0 );
		status = sscanf( argv[i],"%f",&(par->voxel.y) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -vs...\n", 0 );
		i += 1;
		if ( i >= argc)    VT_ErrorParse( "parsing -vs..\n", 0 );
		status = sscanf( argv[i],"%f",&(par->voxel.z) );
		if ( status <= 0 ) VT_ErrorParse( "parsing -vs...\n", 0 );
	    }

            /* template
             */

            else if ( strcmp ( argv[i], "-template") == 0
                 || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
                 || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
              i++;
              if ( i >= argc) VT_ErrorParse( "parsing -template\n", 0 );
              strncpy( par->names.ext, argv[i], STRINGLENGTH );
            }

	    /*--- lecture du type de l'image ---*/
	    else if ( strcmp ( argv[i], "-f" ) == 0 ) {
	      /*--- rien : fixed ---*/
	    }
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
	    else {
		sprintf(text,"unknown option %s\n",argv[i]);
		VT_ErrorParse(text, 0);
	    }
	}
	else if ( argv[i][0] != 0 ) {
	    if ( nb == 0 ) { 
		strncpy( par->names.out, argv[i], STRINGLENGTH );  
		nb += 1;
	    }
	    else 
		VT_ErrorParse("too much file names when parsing\n", 0 );
	}
	i += 1;
    }
    if (nb == 0) {
	strcpy( par->names.out, ">" );  /* standart output */
    }
    
    /*--- type de l'image resultat ---*/
    if ( (o == 1) && (s == 1) && (r == 0) )  par->type = SCHAR;
    if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
    if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
    if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
    if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
    if ( ((o == 0) || (o == 4)) && (r == 1) )  par->type = FLOAT;
    if ( (o == 8) && (r == 1) )  par->type = DOUBLE;
    if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program);
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
        par->dim.x = par->dim.y = par->dim.z = par->dim.v = -1;
        par->voxel.x = par->voxel.y = par->voxel.z = -1.0;
	par->offset.x = par->offset.y = par->offset.z = 0.0;
	par->type = UCHAR;
}
