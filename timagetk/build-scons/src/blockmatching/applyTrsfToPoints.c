/*************************************************************************
 * applyTrsfToPoints.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Feb 11 15:22:38 CET 2013
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#include <bal-applyTrsfToPoints.h>



/* usages:
   - image_to_be_resampled resampled_image [-trsf %s] [geometry options]
*/


typedef struct {
  char *thefile_name;
  char *resfile_name;

  char *real_transformation_name;
} local_parameter;





static char *program = NULL;

static char *usage = "%s %s [-transformation |-trsf %s]\n";

static char *detail = "\
\n";


static int _verbose_ = 1;
static int _debug_ = 0;


static void _ErrorParse( char *str, int flag );
static void _Parse( int argc, char *argv[], local_parameter *p );
static void _InitParam( local_parameter *p );
static char *_BaseName( char *p );




int main(int argc, char *argv[])
{
  local_parameter p;

  /***************************************************
   *
   * parsing parameters
   *
   ***************************************************/
  program = argv[0];
  
  
  /* no arguments
   */
  if ( argc == 1 ) _ErrorParse( NULL, 0 );

  /* parsing parameters 
   */
  _InitParam( &p );
  _Parse( argc, argv, &p );

  

  /***************************************************
   *
   * 
   *
   ***************************************************/
  if (applyTrsfToPoints(
	p.thefile_name,
	p.resfile_name,
	p.real_transformation_name,
       _verbose_,
       _debug_))
   {
    fprintf( stderr, "%s: Failure.\n",program);
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



static void _Parse( int argc, char *argv[], local_parameter *p )
{
  int i;

  program = argv[0];

  for ( i=1; i<argc; i++ ) {

    if ( argv[i][0] == '-' ) {

      if ( strcmp ( argv[i], "-transformation") == 0
	   || (strcmp ( argv[i], "-trsf") == 0 && argv[i][5] == '\0') ) {
	i++;
	if ( i >= argc) _ErrorParse( "parsing -transformation", 0 );
	p->real_transformation_name = argv[i];
      }
      
      else if ( strcmp ( argv[i], "--help" ) == 0 
		|| ( strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0' )
		|| ( strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0' )
		|| ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) ) {
	_ErrorParse( NULL, 1 );
      }

      else if ( ( strcmp ( argv[i], "-d" ) == 0 && argv[i][2] == '\0' ) 
		|| ( strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0' )) {
	if ( _debug_ <= 0 )  _debug_ = 1;
	else _debug_ ++;
      }

      else {
	fprintf(stderr,"unknown option: '%s'\n",argv[i]);
      }

    }
    /* ( argv[i][0] == '-' )
     */
    else {
      if ( p->thefile_name == NULL ) {
	p->thefile_name = argv[i];
      }
      else if ( p->resfile_name == NULL ) {
	p->resfile_name = argv[i];
      }
      else {
	fprintf(stderr,"too many file names: '%s'\n",argv[i]);
      }
    }

  }
  
}



static void _InitParam( local_parameter *p )
{
  p->thefile_name = NULL;
  p->resfile_name = NULL;

  p->real_transformation_name = NULL;
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
