/*************************************************************************
 * test-trsf-estimation.c -
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
#include <string.h>
#include <time.h>

#include <baladin.h>

static char *program = NULL;

static char *usage = "[-p|-points %d] [-seed|-init %d]\n\
 [-h|-help|--help]\n";

static char *detail = "";

static void _ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage: %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s\n",detail);
  if ( str != NULL ) (void)fprintf(stderr,"Error: %s\n",str);
  exit( -1 );
}





int main(int argc, char *argv[])
{
  int i, status;
  long int seedRandom = time(0);

  int _nbpoints_ = 100;
  enumTypeTransfo _transformation_ = RIGID_3D;

  /* parse args
   */
  program = argv[0];
  for ( i=1; i<argc; i++ ) {

    if ( argv[i][0] == '-' ) {
      if ( strcmp ( argv[i], "-help" ) == 0 ||
	   strcmp ( argv[i], "--help" ) == 0 ||
	   strcmp ( argv[i], "-h" ) == 0 ) {
	_ErrorParse( "help message\n", 1 );
      }
      
      /* number of points
       */
      else if ( (strcmp ( argv[i], "-p" ) == 0) ||
		(strcmp ( argv[i], "-points" ) == 0) ) {
	i += 1;
	if ( i >= argc)    _ErrorParse( "parsing -points...\n", 0 );
	status = sscanf( argv[i],"%d",&_nbpoints_ );
	if ( status <= 0 ) _ErrorParse( "parsing -points...\n", 0 );
      }
      
      /* seed for random distribution
       */
      else if ( (strcmp ( argv[i], "-init" ) == 0) ||
		(strcmp ( argv[i], "-seed" ) == 0) ) {
	i += 1;
	if ( i >= argc)    _ErrorParse( "parsing -seed...\n", 0 );
	status = sscanf( argv[i],"%ld",&seedRandom );
	if ( status <= 0 ) _ErrorParse( "parsing -seed...\n", 0 );
      }

      
      /* transformation
       */
      else if ( strcmp ( argv[i], "-transformation" ) == 0
		|| ( strcmp ( argv[i], "-tran" ) == 0 && argv[i][5] == '\0' ) ) {
	i ++;
	if ( i >= argc)    _ErrorParse( "-transformation", 0 );
	if ( (strcmp ( argv[i], "rigid" ) == 0 && argv[i][5] == '\0')
	     || (strcmp ( argv[i], "rigi" ) == 0 && argv[i][4] == '\0') ) {
	  _transformation_ = RIGID_3D;
	}
	else if ( strcmp ( argv[i], "similitude" ) == 0
		  || (strcmp ( argv[i], "simi" ) == 0 && argv[i][4] == '\0') ) {
	  _transformation_ = SIMILITUDE_3D;
	}
	else if ( strcmp ( argv[i], "affine" ) == 0
		  || (strcmp ( argv[i], "affi" ) == 0 && argv[i][4] == '\0') ) {
	  _transformation_ = AFFINE_3D;
	}
	else {
	  fprintf( stderr, "unknown transformation type: '%s'\n", argv[i] );
	  _ErrorParse( "parsing -transformation...\n", 0 );
	}
    }
 
      else {
	fprintf( stderr, "unknown option: '%s'\n", argv[i] );
      }
    }
    else {
      fprintf( stderr, "unknown option: '%s'\n", argv[i] );
    }
  }


  return( 0 );
}

