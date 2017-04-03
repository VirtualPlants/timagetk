/*************************************************************************
 * bal-stddef.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Tue Mar 19 18:19:24 CET 2013
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

#include <bal-stddef.h>



void BAL_PrintTypeSimilarity( FILE *f, enumTypeSimilarity m, char *s )
{
  if ( s != (char*)NULL )
    fprintf( f, "%s", s );
  switch ( m ) {
  default : fprintf( f, "unknown\n" ); break;
  case _SAD_ : fprintf( f, "_SAD_\n" ); break;
  case _SSD_ : fprintf( f, "_SSD_\n" ); break;
  case _SQUARED_CC_ : fprintf( f, "_SQUARED_CC_\n" ); break;
  case _SQUARED_EXTCC_ : fprintf( f, "_SQUARED_EXTCC_\n" ); break;
  }
}



void BAL_PrintTypeTransformation( FILE *f, enumTypeTransfo m, char *s )
{
  if ( s != (char*)NULL )
    fprintf( f, "%s", s );
  switch ( m ) {
  default :
  case UNDEF_TRANSFORMATION : fprintf( f, "UNDEF_TRANSFORMATION\n" ); break;
  case TRANSLATION_2D : fprintf( f, "TRANSLATION_2D\n" ); break;
  case TRANSLATION_3D : fprintf( f, "TRANSLATION_3D\n" ); break;
  case TRANSLATION_SCALING_2D : fprintf( f, "TRANSLATION_SCALING_2D\n" ); break;
  case TRANSLATION_SCALING_3D : fprintf( f, "TRANSLATION_SCALING_3D\n" ); break;
  case RIGID_2D : fprintf( f, "RIGID_2D\n" ); break;
  case RIGID_3D : fprintf( f, "RIGID_3D\n" ); break;
  case SIMILITUDE_2D : fprintf( f, "SIMILITUDE_2D\n" ); break;
  case SIMILITUDE_3D : fprintf( f, "SIMILITUDE_3D\n" ); break;
  case AFFINE_2D : fprintf( f, "AFFINE_2D\n" ); break;
  case AFFINE_3D : fprintf( f, "AFFINE_3D\n" ); break;
  case VECTORFIELD_2D : fprintf( f, "VECTORFIELD_2D\n" ); break;
  case VECTORFIELD_3D : fprintf( f, "VECTORFIELD_3D\n" ); break;
  case SPLINE : fprintf( f, "SPLINE\n" ); break;
  }
}



void BAL_PrintIntegerPoint( FILE *f, bal_integerPoint *p, char *s )
{
  if ( s != (char*)NULL ) {
    fprintf( f, "%s.x = %d\n", s, p->x );
    fprintf( f, "%s.y = %d\n", s, p->y );
    fprintf( f, "%s.z = %d\n", s, p->z );
  }
  else {
    fprintf( f, "integerPoint.x = %d\n", p->x );
    fprintf( f, "integerPoint.y = %d\n", p->y );
    fprintf( f, "integerPoint.z = %d\n", p->z );
  }
}



void BAL_PrintDoublePoint( FILE *f, bal_doublePoint *p, char *s )
{
  if ( s != (char*)NULL )
    fprintf( f, "%s", s );
  else 
    fprintf( f, "doublePoint" );
   fprintf( f, "[%f %f %f]\n", p->x, p->y, p->z );
  /*
  if ( s != (char*)NULL ) {
    fprintf( f, "%s.x = %f\n", s, p->x );
    fprintf( f, "%s.y = %f\n", s, p->y );
    fprintf( f, "%s.z = %f\n", s, p->z );
  }
  else {
    fprintf( f, "doublePoint.x = %f\n", p->x );
    fprintf( f, "doublePoint.y = %f\n", p->y );
    fprintf( f, "doublePoint.z = %f\n", p->z );
  }
  */
}



void BAL_PrintTypeFieldPoint( FILE *f, bal_typeFieldPoint *p, char *s )
{
  if ( s != (char*)NULL )
    fprintf( f, "%s", s );

  fprintf( f, "%f %f %f\n", p->x, p->y, p->z );
}



void BAL_PrintIntensitySelection( FILE *f, bal_intensitySelection *p, char *s )
{  
  if ( s != (char*)NULL ) {
    fprintf( f, "%s.low_threshold  = %d\n", s, p->low_threshold );
    fprintf( f, "%s.high_threshold = %d\n", s, p->high_threshold);
    fprintf( f, "%s.max_removed_fraction = %f\n", s, p->max_removed_fraction );
  }
  else {
    fprintf( f, "intensitySelection.low_threshold  = %d\n", p->low_threshold );
    fprintf( f, "intensitySelection.high_threshold = %d\n", p->high_threshold );
    fprintf( f, "intensitySelection.max_removed_fraction = %f\n", p->max_removed_fraction );
  }
}
