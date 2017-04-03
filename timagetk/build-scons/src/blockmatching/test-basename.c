/*************************************************************************
 * test-basename.c -
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *__BaseName( char *p )
{
  int l;
  if ( p == (char*)NULL ) return( (char*)NULL );
  l = strlen( p ) - 1;
  while ( l >= 0 && p[l] != '/' )
    l--;
  if ( l < 0 ) l = 0;
  if ( p[l] == '/' ) l++;
  fprintf( stderr, "l=%d\n", l );
  return( &(p[l]) );
}


int main (int argc, char *argv[] ) 
{
  printf( "basename = '%s'\n", __BaseName( argv[0] ) );
}
