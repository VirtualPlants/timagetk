/*************************************************************************
 * zpar.c -
 *
 * $$
 *
 * Copyright (c) INRIA 2009
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


/*------- Definition des fonctions statiques ----------*/
static void VT_ErrorParse( char *str, int l );





static char *usage = "image #1 [image #2] ...\n";

static char *detail = "\
gives some information on images\n";

static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  vt_image *image;
  int i;
  
  if ( argc <= 1 ) { 
    VT_ErrorParse("\n", 1);
  }

  
  for ( i=1; i<argc; i++ ) {
    
    fprintf( stdout, "%s: ", argv[i] );
    image = _VT_Inrimage( argv[i] );

    if ( image == (vt_image*)NULL ) {
      fprintf( stdout, "unable to read\n");
      continue;
    }
    if ( image->dim.v > 1 )
      fprintf( stdout, "-v %lu ", image->dim.v );

    fprintf( stdout, "-x %lu ", image->dim.x );
    fprintf( stdout, "-y %lu ", image->dim.y );
    fprintf( stdout, "-z %lu ", image->dim.z );
    
    switch ( image->type ) {
    case SCHAR :
      printf( "-f -o 1 -s ");
	break;
    case UCHAR :
	printf( "-f -o 1 ");
	break;
    case SSHORT :
	printf( "-f -o 2 -s ");
	break;
    case USHORT :
	printf( "-f -o 2 ");
	break;
    case SINT :
	printf( "-f -o 4 -s ");
	break;
    case UINT :
	printf( "-f -o 4 ");
	break;
    case ULINT :
	printf( "-r -o 8 ");
	break;
    case FLOAT :
	printf( "-r -o 4 ");
	break;
    case DOUBLE :
	printf( "-r -o 8 ");
	break;
    case TYPE_UNKNOWN :
    default :
      printf( "type-unknown ");
      ;
    }
    
    fprintf( stdout, "-vx %f ", image->siz.x );
    fprintf( stdout, "-vy %f ", image->siz.y );
    fprintf( stdout, "-vz %f ", image->siz.z );

    fprintf( stdout, "\n" );
    
    VT_FreeImage( image );
    VT_Free( (void**)&image );

  }
  
  return( 1 );
}









static void VT_ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage : %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",detail);
  (void)fprintf(stderr,"Erreur : %s",str);
  exit(0);
}






