/*************************************************************************
 * string-tools.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sam  8 jui 2013 15:28:41 CEST
 *
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* for STRINGLENGTH defintion
 */
#include <typedefs.h>

#include <string-tools.h>





static int _verbose_ = 1;

void setVerboseInStringTools( int v )
{
  _verbose_ = v;
}

void incrementVerboseInStringTools(  )
{
  _verbose_ ++;
}

void decrementVerboseInStringTools(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





/**************************************************
 *
 * string list
 *
 **************************************************/






void initStringList( stringList *l )
{
  l->data = (char**)NULL;
  l->n = 0;
}





void freeStringList( stringList *l )
{
  int i;
  for ( i=0; i<l->n; i++ ) {
    if ( l->data[i] != (char*)NULL ) free( l->data[i] );
    l->data[i] = (char*)NULL;
  }
  if ( l->data != (char**)NULL ) free( l->data );
  initStringList( l );
}





void printStringList( FILE *f, stringList *l, char *d )
{
  int i;

  if ( d != (char*)NULL )
    fprintf( f, "%s\n", d );
  
  if ( l->data == (char**)NULL || l->n == 0 ) {
    fprintf( f, "NULL list\n" );
    return;
  }
  
  for ( i=0; i<l->n; i++ ) {
    fprintf( f, "  #%3d: ", i );
    if ( l->data[i] != (char*)NULL ) 
      fprintf( f, "%s\n", l->data[i] );
    else
      fprintf( f, "NULL\n" );
  }
}





int addStringToList( char* s, stringList *l )
{
  char *proc = "addStringToList";
  char **t;
  int i;

  if ( s == (char*)NULL ) return( 1 );

  /* (re-)allocate the data list
   */

  t = (char**)malloc( (l->n + 1) * sizeof(char*) );
  if ( t == (char**)NULL ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate data\n", proc );
    return( -1 );
  }
  
  if ( l->data != (char**)NULL && l->n > 0 ) {
    for ( i=0; i<l->n; i++ )
      t[i] = l->data[i];
    free( l->data );
  }

  l->data = t;
  l->n++;
  t[l->n-1] = (char*)NULL;
  
  /* allocate the string
   */

  t[l->n-1] = (char*)malloc( (strlen(s)+1) * sizeof(char) );
  if ( t[l->n-1] == (char*)NULL ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate string #%d\n", proc, l->n - 1 );
    return( -1 );
  }
  (void)strncpy( t[l->n-1], s, strlen(s)+1 );
  return( 1 );
}





int buildStringListFromFile( char* name, stringList *l )
{
  char *proc = "buildStringListFromFile";
  FILE *f;
  char imname[STRINGLENGTH];

  f = fopen( name, "r" );
  if ( f == NULL ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: error when opening %s\n", proc, name);
    return( -1 );
  }

  while ( fgets( imname, STRINGLENGTH, f ) != NULL ) {
    /* comments
     */
    if ( imname[0] == '#' 
         ||  imname[0] == '\%' ) continue;
    /* empty string
     */
    if ( imname[0] == '\n' 
         || imname[0] == '0' ) continue;

    if ( imname[strlen(imname)-1] == '\n' )  imname[strlen(imname)-1] = '\0';
    if  ( addStringToList( imname, l ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to add string #%d '%s'\n", proc, l->n, imname);

        freeStringList( l );
        return( -1 );
    }
  }

  fclose( f );
  return( 1 );
}





int buildStringListFromFormat( char* format, int first, int last, stringList *l )
{
  char *proc = "buildStringListFromFormat";
  char imname[STRINGLENGTH];
  int i;
  int inc;

  inc = ( first <= last ) ? 1 : -1;

  for ( i=first; i<=last; i += inc ) {
    sprintf( imname, format, i );
    if  ( addStringToList( imname, l ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to add string #%d '%s'\n", proc, i, imname);
        freeStringList( l );
        return( -1 );
    }
  }
  
  return( 1 );
}



int buildStringListFromArgv( int argc, char* argv[], int *is_a_file, stringList *l )
{
  char *proc = "buildStringListFromArgv";
  int i;

  for ( i=0; i<argc; i++ ) {
    if ( is_a_file[i] == 0 ) continue;
    if  ( addStringToList( argv[i], l ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: unable to add string #%d '%s'\n", proc, i, argv[i]);
        freeStringList( l );
        return( -1 );
    }
  }
  
  return( 1 );
}










/**************************************************
 *
 * string array
 *
 **************************************************/






void initStringArray( stringArray *l )
{
  l->data = (char**)NULL;
  l->n = 0;
  l->array = (char***)NULL;
  l->ncols = 0;
  l->nrows = 0;
}





void freeStringArray( stringArray *l )
{
  int i;
  for ( i=0; i<l->n; i++ ) {
    if ( l->data[i] != (char*)NULL ) free( l->data[i] );
    l->data[i] = (char*)NULL;
  }
  if ( l->data != (char**)NULL ) free( l->data );
  if ( l->array != (char***)NULL ) free( l->array );
  initStringArray( l );
}





void printStringArray( FILE *f, stringArray *l, char *d )
{
  int i, j;

  if ( d != (char*)NULL )
    fprintf( f, "%s\n", d );
  
  if ( l->data == (char**)NULL || l->n == 0 ) {
    fprintf( f, "NULL list\n" );
    return;
  }
  
  for ( j=0; j<l->nrows; j++ ) {
    for ( i=0; i<l->ncols; i++ ) {
      fprintf( f, "  str[%3d][%3d]: ", j, i );
      if ( l->array[j][i] != (char*)NULL ) 
        fprintf( f, "%s\n", l->array[j][i] );
      else
        fprintf( f, "NULL\n" );
    }
  }
}





int allocStringArray( stringArray *l, int ncols, int nrows )
{
  char *proc = "allocStringArray";
  int i, j, n;

  if ( ncols <= 0 || nrows <= 0 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: negative dimensions\n", proc );
      return( -1 );
  }

  /* allocate data
   */
  n = ncols*nrows;
  l->data = (char**)malloc( n * sizeof( char* ) );
  if ( l->data == (char**)NULL ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate data\n", proc );
      return( -1 );
  }
  for ( i=0; i<n; i++ ) 
    l->data[i] = (char*)NULL;
  l->n = n;

  /* allocate array
   */
  l->array = (char***)malloc( nrows * sizeof( char** ) );
  if ( l->array == (char***)NULL ) {
    free( l->data );
    l->n = 0;
    if ( _verbose_ )
         fprintf( stderr, "%s: unable to allocate array\n", proc );
    return( -1 );
  }
  
  /* build array
   */
  for ( j=0, i=0 ; j<nrows; j++, i+=ncols )
    l->array[j] = &(l->data[i]);
  l->nrows = nrows;
  l->ncols = ncols;
  
  return( 1 );
}






int buildStringArrayFromFormat( char* format, int firstcol, int lastcol, 
                                int firstrow, int lastrow, stringArray *l )
{
  char *proc = "buildStringArrayFromFormat";
  char imname[STRINGLENGTH];
  int nrows, ncols;
  int i, j;

  ncols = lastcol-firstcol+1;
  nrows = lastrow-firstrow+1;

  if ( allocStringArray( l, ncols, nrows ) != 1 ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate string array\n", proc );
      return( -1 );
  }

  for ( j=firstrow; j<=lastrow; j++ ) {
    for ( i=firstcol; i<=lastcol; i++ ) {

      sprintf( imname, format, i, j );

      l->array[j-firstrow][i-firstcol] = (char*)malloc( (strlen(imname)+1) * sizeof(char) );
      if ( l->array[j-firstrow][i-firstcol] == (char*)NULL ) {
          if ( _verbose_ )
              fprintf( stderr, "%s: unable to allocate string[%d][%d]\n", proc, j, i );
          return( -1 );
      }
      (void)strncpy( l->array[j-firstrow][i-firstcol], imname, strlen(imname)+1 );
      if ( 0 )
          fprintf( stderr, "%s: add %s at [%d][%d]\n", proc, imname, j-firstrow, i-firstcol );
    }
  }
  
  return( 1 );
}
