/*************************************************************************
 * bal-point.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun 23 sep 2013 16:54:01 CEST
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

#include <bal-point.h>

static int _verbose_ = 1;



void BAL_SetVerboseInBalPoint( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalPoint(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalPoint(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}







/************************************************************
 *
 * MANAGEMENT (bal_typeFieldPointList)
 *
 ************************************************************/

void BAL_InitTypeFieldPointList( bal_typeFieldPointList *l )
{
  l->data = (bal_typeFieldPoint*)NULL;
  l->n_data = 0;
  l->n_allocated_data = 0;
  l->unit =  VOXEL_UNIT;
} 



void BAL_FreeTypeFieldPointList( bal_typeFieldPointList *l )
{
  if ( l->data != (bal_typeFieldPoint*)NULL ) {
    free( l->data );
  } 
  BAL_InitTypeFieldPointList( l );
} 



void BAL_PrintTypeFieldPointList( FILE *f, bal_typeFieldPointList *l )
{
  int i;
  
  for ( i=0; i<l->n_data; i++ )
    BAL_PrintTypeFieldPoint( f, &(l->data[i]), (char*)NULL );
}






static int _size_to_be_allocated_ = 100;

static int BAL_AddTypeFieldPointToTypeFieldPointList( bal_typeFieldPointList *l, 
						      bal_typeFieldPoint *c )
{
  char *proc = "BAL_AddTypeFieldPointToTypeFieldPointList";
  int s =  l->n_allocated_data;
  bal_typeFieldPoint *data;

  if ( l->n_data == l->n_allocated_data ) {
    s += _size_to_be_allocated_;
    data = (bal_typeFieldPoint*)malloc( s * sizeof(bal_typeFieldPoint) );
    if ( data == (bal_typeFieldPoint*)NULL ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
    } 
    if ( l->n_allocated_data > 0 ) {
      (void)memcpy( data, l->data, l->n_allocated_data*sizeof(bal_typeFieldPoint) );
      free( l->data );
    }
    l->n_allocated_data = s;
    l->data = data;
  }

  l->data[l->n_data] = *c;
  l->n_data ++;

  return( 1 );
} 





int BAL_ReadTypeFieldPointList( bal_typeFieldPointList *l, char *filename )
{
  char *proc = "BAL_ReadTypeFieldPointList";
  FILE *f;
  int i;
  float x, y, z;
  bal_typeFieldPoint c;

  f = fopen( filename, "r" );
  if ( f == (FILE*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to open '%s'\n", proc, filename );
    return( -1 );
  }

  i = 0;
  while ( fscanf( f, "%f %f %f", &x, &y, &z ) == 3 ) {
    c.x = x;
    c.y = y;
    c.z = z;
    if ( BAL_AddTypeFieldPointToTypeFieldPointList( l, &c ) != 1 ) {
      fclose( f );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to add measure #%d (=%f %f %f) to list\n", 
		 proc, i, x, y, z );
      return( -1 );
    }
    i ++;
  }

  if ( 0 && _verbose_ )
    fprintf( stderr, "%s: read %d values\n", proc, i );


  fclose( f );
  return( 1 );
}

