/*************************************************************************
 * temporal-matching.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun 30 sep 2013 22:09:46 CEST
 *
 * ADDITIONS, CHANGES
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <vt_temporal-matching.h>





static int _verbose_ = 1;



static char *program = NULL;



static char *usage = "-reference|-ref %s -floating|-flo %s -result|-res %s\n\
 [-reference-dt|-ref-dt|-rdt %f] [-floating-dt|-flo-dt|-fdt %f]\n\
 [-scilab %s]\n\
 [-debug|-no-debug]\n\
 [-h|-help|--h|--help]\n";

static char *detail = "\
";



typedef struct local_parameter {

  char *floating_measures;
  char *reference_measures;

  float floating_dt;
  float reference_dt;


  char *scilab_name;


  int print_time;

} local_parameter;



#define FILENAMELENGTH 128



static void _ErrorParse( char *str, int flag );
static void _Parse( int argc, char *argv[], local_parameter *p );
static void _InitParam( local_parameter *par );
static double _GetTime();
static double _GetClock();
static char *_BaseName( char *p );








int main( int argc, char *argv[] )
{
  local_parameter p;

  measureListType referenceList;
  measureListType floatingList;

  measureListType referenceMax;
  measureListType floatingMax;

  double time_init = _GetTime();
  double time_exit;
  double clock_init = _GetClock();
  double clock_exit;


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





  initmeasureList( &referenceList );
  initmeasureList( &floatingList );

  initmeasureList( &referenceMax );
  initmeasureList( &floatingMax );



  /* reading lists of measures
   */
  referenceList.dt = p.reference_dt;
  if ( readMeasureList( &referenceList, p.reference_measures ) != 1 ) {
    freeMeasureList( &floatingList );
    freeMeasureList( &referenceList );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when reading '%s'\n", program, p.reference_measures );
    return( -1 );
  }

  if ( 0 )
    fprintfMeasureList( stdout, &referenceList );
  
  floatingList.dt = p.floating_dt;
  if ( readMeasureList( &floatingList, p.floating_measures ) != 1 ) {
    freeMeasureList( &floatingList );
    freeMeasureList( &referenceList );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when reading '%s'\n", program, p.floating_measures );
    return( -1 );
  }






  if ( extractMeasureMaxima( &referenceMax, &referenceList ) != 1 ) {
    freeMeasureList( &floatingMax );
    freeMeasureList( &referenceMax );
    freeMeasureList( &floatingList );
    freeMeasureList( &referenceList );
        if ( _verbose_ )
      fprintf( stderr, "%s: error when computing reference maxima\n", program );
    return( -1 );
  }

  if ( 0 ) {
    fprintf( stdout, "Reference maxima\n" );
    fprintfMeasureList( stdout, &referenceMax );
  }

  if ( extractMeasureMaxima( &floatingMax, &floatingList ) != 1 ) {
    freeMeasureList( &floatingMax );
    freeMeasureList( &referenceMax );
    freeMeasureList( &floatingList );
    freeMeasureList( &referenceList );
        if ( _verbose_ )
      fprintf( stderr, "%s: error when computing floating maxima\n", program );
    return( -1 );
  }

  if ( 0 ) {
    fprintf( stdout, "Floating maxima\n" );
    fprintfMeasureList( stdout, &floatingMax );
  }



  








  if ( temporalRegistration( &referenceMax, &floatingMax, -20, 20, 1 ) != 1 ) {
    freeMeasureList( &floatingMax );
    freeMeasureList( &referenceMax );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when registering maxima\n", program );
    return( -1 );

  }



  
  if ( p.scilab_name != NULL ) {
    writeScilab( &referenceList, &floatingList, p.scilab_name );
  }





  freeMeasureList( &floatingList );
  freeMeasureList( &referenceList );
  freeMeasureList( &floatingMax );
  freeMeasureList( &referenceMax );




  /* end
   */
  time_exit = _GetTime();
  clock_exit = _GetClock();

  if ( p.print_time ) { 
    fprintf( stderr, "%s: elapsed (real) time = %f\n", _BaseName(program), time_exit - time_init );
    fprintf( stderr, "\t       elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
    fprintf( stderr, "\t       ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
  }

  return( 0 );
}













/************************************************************
 *
 *
 *
 ************************************************************/


static void _ErrorParse( char *str, int flag )
{
  (void)fprintf( stderr, "Usage: %s %s", _BaseName(program), usage );
  if ( flag > 0 ) (void)fprintf( stderr, "%s\n", detail );
  if ( str != NULL ) (void)fprintf( stderr, "Error: %s\n", str );
  exit( -1 );
}



/************************************************************
 *
 * reading parameters is done in two steps
 * 1. one looks for the transformation type
 *    -> this implies dedicated initialization
 * 2. the other parameters are read
 * 
 ************************************************************/

static void _Parse( int argc, char *argv[], local_parameter *p )
{
  int i;
  int status;

  program = argv[0];
  


  /* reading the  parameters
   */

  for ( i=1; i<argc; i++ ) {
    
    /* image file names 
     */
    if ( (strcmp ( argv[i], "-reference") == 0 && argv[i][10] == '\0')
	      || (strcmp ( argv[i], "-ref") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -reference", 0 );
      p->reference_measures = argv[i];
    }
    else if ( (strcmp ( argv[i], "-floating") == 0 && argv[i][9] == '\0')
	      || (strcmp ( argv[i], "-flo") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -floating", 0 );
      p->floating_measures = argv[i];
    }


    else if ( strcmp ( argv[i], "-scilab") == 0 ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -scilab", 0 );
      p->scilab_name = argv[i];
    }


    /*
     */
    else if ( strcmp ( argv[i], "-reference-dt") == 0
	      || (strcmp ( argv[i], "-ref-dt") == 0 && argv[i][7] == '\0')
	      || (strcmp ( argv[i], "-rdt") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -reference-dt", 0 );
      status = sscanf( argv[i],"%f",&p->reference_dt );
      if ( status <= 0 ) _ErrorParse( "parsing -reference-dt...\n", 0 );
    }
    else if ( strcmp ( argv[i], "-floating-dt") == 0
	      || (strcmp ( argv[i], "-flo-dt") == 0 && argv[i][7] == '\0')
	      || (strcmp ( argv[i], "-fdt") == 0 && argv[i][4] == '\0') ) {
      i++;
      if ( i >= argc) _ErrorParse( "parsing -floating-dt", 0 );
      status = sscanf( argv[i],"%f",&p->floating_dt );
      if ( status <= 0 ) _ErrorParse( "parsing -floating-dt...\n", 0 );
    }

    

    /*
     */

    else if ( (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
      p->print_time = 1;
    }
    else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')  
	      || (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
      p->print_time = 0;
    }

    else if ( ( strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0' )
	      || ( strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0' ) ) {
      _ErrorParse( NULL, 1 );
    }

    else if ( ( strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0' )
	      || ( strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0' ) ) {
      _ErrorParse( NULL, 1 );
    }
    
    else {
      fprintf(stderr,"unknown option: '%s'\n",argv[i]);
    }
  }	
}





static void _InitParam( local_parameter *p ) 
{

  p->floating_measures = NULL;
  p->reference_measures = NULL;

  p->floating_dt = 1.0;
  p->reference_dt = 1.0;

  p->scilab_name = NULL;

  p->print_time = 1;
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



