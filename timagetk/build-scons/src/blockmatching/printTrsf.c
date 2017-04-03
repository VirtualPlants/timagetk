/*************************************************************************
 * printTrsf.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <bal-stddef.h>
#include <bal-transformation.h>









static char *program = NULL;

static char *usage = "%s ... %s\n";

static char *detail = "";



static int _verbose_ = 1;


static void _ErrorParse( char *str, int flag );
static char *_BaseName( char *p );





int main(int argc, char *argv[])
{
  int i;
  bal_transformation theTrsf;

  /***************************************************
   *
   * parsing parameters
   *
   ***************************************************/
  program = argv[0];
  
  
  /* no arguments
   */
  if ( argc == 1 ) _ErrorParse( NULL, 0 );

  
  /* displaying help is required
   */
  i = 1;
  while ( i < argc ) {
    if ( ( strcmp ( argv[i], "-help") == 0 ) 
	 || ( strcmp ( argv[i], "-h") == 0 && argv[i][2] == '\0' ) 
	 || ( strcmp ( argv[i], "--help") == 0 ) 
	 || ( strcmp ( argv[i], "--h") == 0 && argv[i][3] == '\0' ) ) {
      _ErrorParse( NULL, 1 );
    }
    i++;
  }



  /***************************************************
   *
   * 
   *
   ***************************************************/
  BAL_InitTransformation( &theTrsf );

  for ( i=1; i<argc; i++ ) {
    
    if ( BAL_ReadTransformation( &theTrsf, argv[i] ) != 1 ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to read '%s'\n", program, argv[i] );
      fprintf( stderr, "\n" );
    }
    
    fprintf( stdout, "***** Information about '%s'\n\n", argv[i] );
    BAL_PrintTransformation( stdout, &theTrsf, argv[i] );
    fprintf( stdout, "\n" );

    BAL_FreeTransformation( &theTrsf );
  }

  exit( 0 );
}




/***************************************************
 *
 * 
 *
 ***************************************************/





static void _ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage: %s %s\n",_BaseName(program), usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s\n",detail);
  if ( str != NULL ) (void)fprintf(stderr,"Error: %s\n",str);
  exit( -1 );
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

