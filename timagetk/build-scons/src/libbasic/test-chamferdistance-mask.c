/*************************************************************************
 * test-chamferdistance-mask.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2008
 *
 * DESCRIPTION: 
 *
 * 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Sat Nov  8 17:21:44 CET 2008
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <chamferdistance.h>

int main( int argc, char* argv[] )
{
  typeChamferMask theMask;
  typeChamferMask resMask;
  typeChamferMask resInitMask;

  initChamferMask( &theMask );
  initChamferMask( &resMask );
  initChamferMask( &resInitMask );
  
  fprintf( stdout, " _DISTANCE04_ \n" );
  buildPredefinedChamferMask( _DISTANCE04_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  freeChamferMask( &theMask );

  fprintf( stdout, " _DISTANCE08_ \n" );
  buildPredefinedChamferMask( _DISTANCE08_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  freeChamferMask( &theMask );
  
  fprintf( stdout, " _CHAMFER3x3_ \n" );
  buildPredefinedChamferMask( _CHAMFER3x3_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  buildShapeBasedInterpolationMasks(  &theMask, &resInitMask, &resMask );
  fprintf( stdout, " _CHAMFER3x3_ Initialization SBI mask\n" );
  printChamferMaskAsImage( stdout, &resInitMask );
  fprintf( stdout, " _CHAMFER3x3_ SBI mask\n" );
  printChamferMaskAsImage( stdout, &resMask );
  freeChamferMask( &theMask );
  freeChamferMask( &resMask );
  freeChamferMask( &resInitMask );

  fprintf( stdout, " _CHAMFER5x5_ \n" );
  buildPredefinedChamferMask( _CHAMFER5x5_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  buildShapeBasedInterpolationMasks(  &theMask, &resInitMask, &resMask );
  fprintf( stdout, " _CHAMFER5x5_ Initialization SBI mask\n" );
  printChamferMaskAsImage( stdout, &resInitMask );
  fprintf( stdout, " _CHAMFER5x5_ SBI mask\n" );
  printChamferMaskAsImage( stdout, &resMask );
  freeChamferMask( &theMask );
  freeChamferMask( &resMask );
  freeChamferMask( &resInitMask );
  
  


  fprintf( stdout, " _DISTANCE06_ \n" );
  buildPredefinedChamferMask( _DISTANCE06_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  freeChamferMask( &theMask );
  
  fprintf( stdout, " _DISTANCE18_ \n" );
  buildPredefinedChamferMask( _DISTANCE18_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  freeChamferMask( &theMask );
  
  fprintf( stdout, " _DISTANCE26_ \n" );
  buildPredefinedChamferMask( _DISTANCE26_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  freeChamferMask( &theMask );
  
  fprintf( stdout, " _CHAMFER3x3x3_ \n" );
  buildPredefinedChamferMask( _CHAMFER3x3x3_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  freeChamferMask( &theMask );

  fprintf( stdout, " _CHAMFER5x5x5_ \n" );
  buildPredefinedChamferMask( _CHAMFER5x5x5_, &theMask );
  printChamferMaskAsImage( stdout, &theMask );
  buildShapeBasedInterpolationMasks(  &theMask, &resInitMask, &resMask );
  fprintf( stdout, " _CHAMFER5x5x5_ Initialization SBI mask\n" );
  printChamferMaskAsImage( stdout, &resInitMask );
  fprintf( stdout, " _CHAMFER5x5x5_ SBI mask\n" );
  printChamferMaskAsImage( stdout, &resMask );
  freeChamferMask( &theMask );
  freeChamferMask( &resMask );
  freeChamferMask( &resInitMask );

  exit( 0 );
}
