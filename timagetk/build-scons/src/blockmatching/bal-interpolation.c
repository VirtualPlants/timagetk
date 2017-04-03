/*************************************************************************
 * bal-interpolation.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2016, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Lun  4 avr 2016 22:09:07 CEST
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


#include <bal-interpolation.h>


static int _verbose_ = 1;



void BAL_SetVerboseInBalInterpolation( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalInterpolation(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalInterpolation(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





/************************************************************
 *
 ************************************************************/





static void _InitLabelList( typeLabelList *l )
{
  l->data = (int*)NULL;
  l->n_data = 0;
  l->n_allocated_data = 0;
}



static void _FreeLabelList( typeLabelList *l )
{
  if ( l->data != (int*)NULL )
    free( l->data );
  _InitLabelList( l );
}



static int _labels_to_be_allocated_ = 100;

static int _AddLabelToLabelList( typeLabelList *l, int n )
{
  char *proc = "_AddLabelToLabelList";
  int s =  l->n_allocated_data;
  int *data;

  if ( l->n_data == l->n_allocated_data ) {
    s += _labels_to_be_allocated_;
    data = (int*)malloc( s * sizeof(int) );
    if ( data == (int*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation error\n", proc );
      return( -1 );
    }
    if ( l->n_allocated_data > 0 ) {
      (void)memcpy( data, l->data, l->n_allocated_data*sizeof(int) );
      free( l->data );
    }
    l->n_allocated_data = s;
    l->data = data;
  }

  l->data[l->n_data] = n;
  l->n_data ++;

  return( 1 );
}





static void _InitCorrespondence( typeCorrespondence *c )
{
  _InitLabelList( &(c->labelA) );
  _InitLabelList( &(c->labelB) );
}



static void _FreeCorrespondence( typeCorrespondence *c )
{
  _FreeLabelList( &(c->labelA) );
  _FreeLabelList( &(c->labelB) );
}



static void _PrintCorrespondence( FILE *f, typeCorrespondence *c )
{
  int i;

  if ( c->labelA.n_data <= 0 && c->labelB.n_data )
      return;

  if ( c->labelA.n_data > 0 ) {
    for ( i=0; i<c->labelA.n_data; i++ )
      fprintf( f, "%d ", c->labelA.data[i] );
  }
  else {
    fprintf( f, " " );
  }

  fprintf( f, "-" );

  if ( c->labelB.n_data > 0 ) {
    for ( i=0; i<c->labelB.n_data; i++ )
      fprintf( f, " %d", c->labelB.data[i] );
    /* keep a trailing space to be striclty
     * equivalent to writing procedure of Gaal
     */
    fprintf( f, " " );
  }
  else {
    fprintf( f, " " );
  }

  fprintf( f, "\n" );
}



static int _correspondences_to_be_allocated_ = 10;

static int _AddCorrespondenceToCorrespondenceList( typeCorrespondenceList *l, typeCorrespondence *c)
{
  char *proc = "_AddCorrespondenceToCorrespondenceList";
  int s =  l->n_allocated_data;
  typeCorrespondence *data;
  typeCorrespondence *t;

  if ( l->n_data == l->n_allocated_data ) {

    s += _correspondences_to_be_allocated_;
    data = (typeCorrespondence*)malloc( s * sizeof(typeCorrespondence) );
    if ( data == (typeCorrespondence*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation error (array)\n", proc );
      return( -1 );
    }
    if ( l->n_allocated_data > 0 ) {
      (void)memcpy( data, l->data, l->n_allocated_data*sizeof(typeCorrespondence) );
      free( l->data );
    }
    l->n_allocated_data = s;
    l->data = data;
  }

  t = &(l->data[l->n_data]);
  _InitCorrespondence( t );

  if ( c->labelA.n_data > 0 ) {
      t->labelA.data = (int*)malloc( c->labelA.n_data * sizeof(int) );
      if ( t->labelA.data == (int*)NULL ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: allocation error (labelA) \n", proc );
          return( -1 );
      }
      (void)memcpy( t->labelA.data, c->labelA.data, c->labelA.n_data *sizeof(int) );
      t->labelA.n_data = c->labelA.n_data;
      t->labelA.n_allocated_data = c->labelA.n_data;
  }

  if ( c->labelB.n_data > 0 ) {
      t->labelB.data = (int*)malloc( c->labelB.n_data * sizeof(int) );
      if ( t->labelB.data == (int*)NULL ) {
          if ( _verbose_ )
            fprintf( stderr, "%s: allocation error (labelB) \n", proc );
          return( -1 );
      }
      (void)memcpy( t->labelB.data, c->labelB.data, c->labelB.n_data *sizeof(int) );
      t->labelB.n_data = c->labelB.n_data;
      t->labelB.n_allocated_data = c->labelB.n_data;
  }
  l->n_data ++;

  return( 1 );
}



void BAL_InitCorrespondenceList( typeCorrespondenceList *l )
{
    l->data = (typeCorrespondence*)NULL;
    l->n_data = 0;
    l->n_allocated_data = 0;
}



void BAL_FreeCorrespondenceList( typeCorrespondenceList *l )
{
    int c;
    if ( l->data != (typeCorrespondence*)NULL ) {
        for ( c=0; c<l->n_data; c++ ) {
            _FreeCorrespondence( &(l->data[c]) );
        }
        free( l->data );
    }
}



void BAL_PrintCorrespondenceList( FILE *f, typeCorrespondenceList *l )
{
  int c;
  for ( c=0; c<l->n_data; c++ ) {
      _PrintCorrespondence( f, &(l->data[c]) );
  }
}



int BAL_ReadCorrespondenceList( FILE *f, typeCorrespondenceList *l )
{
  char *proc = "BAL_ReadCorrespondenceList";
  typeCorrespondence c;
  typeLabelList *label;
  char line[512], *r;
  int n, stop;

  _InitCorrespondence( &c );

  while ( fgets( line, 512, f ) != (char*)NULL ) {

      c.labelA.n_data = 0;
      c.labelB.n_data = 0;
      label = &(c.labelA);

      r = line;

      /* comments
       */
      if ( *r == '#' || *r == '%' || (r[0] =='/' && r[0] =='/') )
          continue;

      for ( stop=0; stop==0; ) {
        switch( *r ) {
        default :
            _FreeCorrespondence( &c );
            if ( _verbose_ )
                fprintf( stderr, "%s: unexpected reading, line '%s'\n", proc, r );
            return( -1 );
            break;
        case ' ' :
        case '\t' :
            r++;
            break;
        case '\n' :
        case '\0' :
            stop = 1;
            break;
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' :
            if ( sscanf( r, "%d", &n ) != 1 ) {
                _FreeCorrespondence( &c );
                if ( _verbose_ )
                    fprintf( stderr, "%s: reading of '%s' failed\n", proc, r );
                return( -1 );
            }
            if ( _AddLabelToLabelList( label, n ) != 1 ) {
                _FreeCorrespondence( &c );
                if ( _verbose_ )
                    fprintf( stderr, "%s: can not add label %d from line '%s'\n", proc, n, line );
                return( -1 );
            }
            while ( *r >= '0' && *r <= '9' )
                r++;
            break;
        case '-' :
            if ( label == &(c.labelA) )
                label = &(c.labelB);
            else {
                _FreeCorrespondence( &c );
                if ( _verbose_ )
                    fprintf( stderr, "%s: '-' found twice in 'line '%s'\n", proc, line );
                return( -1 );
            }
            r++;
            break;
        }
      }


      if ( c.labelA.n_data > 0 || c.labelB.n_data > 0 ) {
          if ( _AddCorrespondenceToCorrespondenceList( l, &c ) != 1 ) {
              _FreeCorrespondence( &c );
              if ( _verbose_ )
                  fprintf( stderr, "%s: can not add correspondence to list\n", proc );
              return( -1 );
          }
      }

  }




  _FreeCorrespondence( &c );
  return( 1 );
}


