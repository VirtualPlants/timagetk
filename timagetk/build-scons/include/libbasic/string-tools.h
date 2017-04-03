/*************************************************************************
 * string-tools.h -
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
 *
 *
 *
 */

#ifndef _string_tools_h_
#define _string_tools_h_

#ifdef __cplusplus
extern "C" {
#endif





#include <stdlib.h>
#include <stdio.h>



extern void setVerboseInStringTools( int v );
extern void incrementVerboseInStringTools(  );
extern void decrementVerboseInStringTools(  );



typedef struct {
  char **data;
  int n;
} stringList;

typedef struct {
  char **data;
  int n;
  char ***array;
  int ncols;
  int nrows;
} stringArray;



extern void initStringList( stringList *l );
extern void freeStringList( stringList *l );
extern void printStringList( FILE *f, stringList *l, char *d );
extern int addStringToList( char* s, stringList *l );
extern int buildStringListFromFile( char* name, stringList *l );
extern int buildStringListFromFormat( char* format, int first, int last, stringList *l );
extern int buildStringListFromArgv( int argc, char* argv[], int *is_a_file, stringList *l );



extern void initStringArray( stringArray *l );
extern void freeStringArray( stringArray *l );
extern void printStringArray( FILE *f, stringArray *l, char *d );
extern int allocStringArray( stringArray *l, int ncols, int nrows );
extern int buildStringArrayFromFormat( char* format, int firstcol, int lastcol, 
				       int firstrow, int lastrow, stringArray *l );





#ifdef __cplusplus
}
#endif


#endif
