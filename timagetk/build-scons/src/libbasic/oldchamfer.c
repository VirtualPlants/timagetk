/*************************************************************************
 * chamfer.c - computation of chamfer distances
 *
 * $Id: chamfer.c,v 1.6 2001/03/19 17:11:42 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Feb, 8 2000
 *
 * ADDITIONS, CHANGES
 *
 * - Wed Feb 23 22:13:13 MET 2000, Gregoire Malandain
 *   add chamfer 5x5x5
 *
 *
 *
 *
 */


#include <chamfer.h>



static unsigned short int u16infinity = 65535;
static int _verbose_ = 0;


static enumDistance typeDistance = _MALANDAIN3x3x3_;

/* 3 coefficients for the 3x3(x3) chamfer distance
   [0] ->  6-neighbor  <=>  Sum |dx| = 1 (&& Max |dx| = 1)
   [1] -> 18-neighbor  <=>  Sum |dx| = 2  && Max |dx| = 1
   for the 3D case
   [2] -> 26-neighbor  <=>  Sum |dx| = 3  && Max |dx| = 1
*/
static int coeffChamfer3x3x3Borgefors[3] = {3, 4, 5};
static int coeffChamfer3x3x3Malandain[3] = {16, 21, 26 };
static int coeffChamfer3x3x3Distance06[3] = {1, 65535, 65535};
static int coeffChamfer3x3x3Distance18[3] = {1, 1, 65535};
static int coeffChamfer3x3x3Distance26[3] = {1, 1, 1};

/* 6 coefficients for the 5x5(x5) chamfer distance
   [0] -> Sum |dx| = 1  (&& Max |dx| = 1)  (&& Min |dx| = 0) <=> {1,0,0}
   [1] -> Sum |dx| = 2   && Max |dx| = 1   (&& Min |dx| = 0) <=> {1,1,0}
   [2] -> Sum |dx| = 3   && Max |dx| = 2   (&& Min |dx| = 0) <=> {2,1,0}
   for the 3D case 
   [3] -> Sum |dx| = 3   && Max |dx| = 1   (&& Min |dx| = 1) <=> {1,1,1}
   [4] -> Sum |dx| = 4   && Max |dx| = 2    && Min |dx| = 1  <=> {2,1,1}
   [5] -> Sum |dx| = 5   && Max |dx| = 2   (&& Min |dx| = 1) <=> {2,2,1}
*/
static int coeffChamfer5x5x5[6] = {13, 18, 29, 23, 32, 39};






typedef enum {
  _2D_,
  _3D_
} enumDimensionality;


static int _ComputeChamferDistance( void *inputBuf,
				    bufferType typeIn,
				    void *outputBuf,
				    bufferType typeOut,
				    const int *theDim,
				    float threshold,
				    enumDimensionality dim );
















int Compute2DChamfer3x3( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim )
{
  SetChamferCoefficientsToMalandain3x3x3();
  return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, 1.0, _2D_ ) );
}

int Compute3DChamfer3x3x3( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim )
{
  SetChamferCoefficientsToMalandain3x3x3();
  return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, 1.0, _3D_ ) );
}

int Compute2DChamfer5x5( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim )
{
  SetChamferCoefficientsToChamfer5x5x5();
  return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, 1.0, _2D_ ) );
}

int Compute3DChamfer5x5x5( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim )
{
  SetChamferCoefficientsToChamfer5x5x5();
  return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, 1.0, _3D_ ) );
}










int Compute2DChamfer3x3WithThreshold( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim,
					float threshold )
{
  SetChamferCoefficientsToMalandain3x3x3();
  return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, threshold, _2D_ ) );
}

int Compute3DChamfer3x3x3WithThreshold( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim,
					float threshold )
{
  SetChamferCoefficientsToMalandain3x3x3();
  return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, threshold, _3D_ ) );
}

int Compute2DChamfer5x5WithThreshold( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim,
					float threshold )
{
  SetChamferCoefficientsToChamfer5x5x5();
   return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, threshold, _2D_ ) );
}

int Compute3DChamfer5x5x5WithThreshold( void *inputBuf,
		       bufferType typeIn,
		       void *outputBuf,
		       bufferType typeOut,
		       const int *theDim,
					float threshold )
{
   SetChamferCoefficientsToChamfer5x5x5();
  return( _ComputeChamferDistance( inputBuf, typeIn, outputBuf, typeOut ,
			    theDim, threshold, _3D_ ) );
}










static int _ComputeChamferDistance( void *inputBuf,
				    bufferType typeIn,
				    void *outputBuf,
				    bufferType typeOut,
				    const int *theDim,
				    float threshold,
				    enumDimensionality dim )
{
  char *proc = "_ComputeChamferDistance";
  int theCoeff[6];
  int v = theDim[0]*theDim[1]*theDim[2];
  int i;
  unsigned short int *theBuf, *tmpBuf = (unsigned short int *)NULL;
  enumDimensionality d = dim;


  /* tests sur les dimensions
   */
  if ( theDim[2] == 1 ) d = _2D_ ;
  switch ( typeDistance ) {
  case _DISTANCE04_ :
  case _DISTANCE08_ :
    d = _2D_ ;
  default :
    if ( theDim[0] < 2 || theDim[1] < 2 ) {
      if ( _verbose_ ) {
	fprintf( stderr, "%s: unable to deal with such dimensions, X x Y = %d x %d\n",
		 proc, theDim[0], theDim[1] );
      }
      return( -1 );
    }
    break;
  case _CHAMFER5x5x5_ :
    if ( theDim[0] < 4 || theDim[1] < 4 ) {
      if ( _verbose_ ) {
	fprintf( stderr, "%s: unable to deal with such dimensions, X x Y = %d x %d\n",
		 proc, theDim[0], theDim[1] );
      }
      return( -1 );
    }
    break;
  }



  /* allocations
   */
  switch( typeOut ) {
  case USHORT :
  case SSHORT :
    tmpBuf = (unsigned short int *)outputBuf;
    break;
  default :
    tmpBuf = (unsigned short int *)malloc( v * sizeof(unsigned short int) );
    if ( tmpBuf == (unsigned short int *)NULL ) {
      if ( _verbose_)
	fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      return( -1 );
    }
  }


  /* initialisation
   */
  if ( InitializeChamferMap( inputBuf, typeIn, tmpBuf, theDim, threshold ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to initialize distance map\n", proc );
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    return( -1 );
  }

  

  /* choix des coefficients

     theCoeff[0] => increment pour une adjacence par face (3D)  / arete (2D)
     theCoeff[1] => increment pour une adjacence par arete (3D) / sommet (2D)
     theCoeff[2] => increment pour une adjacence par sommet (3D)
     
     
   */
  
  switch ( typeDistance ) {
  case _BORGEFORS3x3x3_ :
    theCoeff[0] = coeffChamfer3x3x3Borgefors[0];
    theCoeff[1] = coeffChamfer3x3x3Borgefors[1];
    theCoeff[2] = coeffChamfer3x3x3Borgefors[2];
    break;
  default :
    if ( _verbose_)
      fprintf( stderr, "%s: unknown distance type => switch to default\n", proc );
  case _MALANDAIN3x3x3_ :
    theCoeff[0] = coeffChamfer3x3x3Malandain[0];
    theCoeff[1] = coeffChamfer3x3x3Malandain[1];
    theCoeff[2] = coeffChamfer3x3x3Malandain[2];
    break;
  case _DISTANCE04_ :
  case _DISTANCE06_ :
    theCoeff[0] = coeffChamfer3x3x3Distance06[0];
    theCoeff[1] = coeffChamfer3x3x3Distance06[1];
    theCoeff[2] = coeffChamfer3x3x3Distance06[2];
    break;
  case _DISTANCE18_ :
    theCoeff[0] = coeffChamfer3x3x3Distance18[0];
    theCoeff[1] = coeffChamfer3x3x3Distance18[1];
    theCoeff[2] = coeffChamfer3x3x3Distance18[2];
    break;
  case _DISTANCE08_ :
  case _DISTANCE26_ :
    theCoeff[0] = coeffChamfer3x3x3Distance26[0];
    theCoeff[1] = coeffChamfer3x3x3Distance26[1];
    theCoeff[2] = coeffChamfer3x3x3Distance26[2];
    break;
  case _CHAMFER5x5x5_ :
    theCoeff[0] = coeffChamfer5x5x5[0];
    theCoeff[1] = coeffChamfer5x5x5[1];
    theCoeff[2] = coeffChamfer5x5x5[2];
    theCoeff[3] = coeffChamfer5x5x5[3];
    theCoeff[4] = coeffChamfer5x5x5[4];
    theCoeff[5] = coeffChamfer5x5x5[5];
  }


  /* calcul de la distance
   */
  
  switch ( typeDistance ) {
    
  default :
    
    switch ( d ) {
    case _2D_ :
      Compute2DChamfer3InUShortImage( tmpBuf, theDim, theCoeff );
      break;
    case _3D_ :
    default :
      Compute3DChamfer3InUShortImage( tmpBuf, theDim, theCoeff );
    }
    break;
    
  case _CHAMFER5x5x5_ :

    switch ( d ) {
    case _2D_ :
      Compute2DChamfer5InUShortImage( tmpBuf, theDim, theCoeff );
      break;
    case _3D_ :
    default :
      Compute3DChamfer5InUShortImage( tmpBuf, theDim, theCoeff );
    }

  }
    


  /* divers choix
     1. travailler directement sur la carte
        en sachant que les valeurs sont multipliees par theCoeff[0]
     2. division rapide (cas theCoeff[0]=16)
     3. division lente
  */
  theBuf = tmpBuf;

  switch ( typeDistance ) {
  case _BORGEFORS3x3x3_ :
    for ( i=0; i<v; i++, theBuf++ )
      *theBuf = (int)( (double)*theBuf / (double)3.0 + 0.5 );
    break;
  case _MALANDAIN3x3x3_ :
    for ( i=0; i<v; i++, theBuf++ )
      *theBuf = *theBuf >> 4;
    break;
  case _DISTANCE04_ :
  case _DISTANCE08_ :
  case _DISTANCE06_ :
  case _DISTANCE18_ :
  case _DISTANCE26_ :
  default :
    break;
  case _CHAMFER5x5x5_ :
    for ( i=0; i<v; i++, theBuf++ )
      *theBuf = (int)( (double)*theBuf / (double)13.0 + 0.5 );
    break;
  }
  

  if ( ConvertBuffer( (void*)tmpBuf, USHORT, outputBuf, typeOut, v ) != 1 ) {
    if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
    return( -1 );
  }

  if ( (typeOut != USHORT) && (typeOut != SSHORT) ) free( tmpBuf );
  return( 1 );

}













int InitializeChamferMap( void *inputBuf,
			  bufferType typeIn,
			  unsigned short int *outputBuf,
			  const int *theDim,
			  float threshold )
{
  char *proc="InitializeChamferMap";
  int i, v=theDim[0]*theDim[1]*theDim[2];
  int iThreshold = 0;
  unsigned short int *resBuf = outputBuf;

  /* iThreshold is the nearest integer to threshold
   */
  if ( threshold >= 0.0 ) iThreshold = (int)(threshold + 0.5);
  else                    iThreshold = (int)(threshold - 0.5);

  switch( typeIn ) {
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char *)inputBuf;
      for (i=0; i<v; i++, theBuf++, resBuf++) {
	if ( *theBuf < iThreshold ) {
	  *resBuf = u16infinity;
	}
	else *resBuf = 0;
      }
    }
    break;
  case SCHAR :
    {
      char *theBuf = (char *)inputBuf;
      for (i=0; i<v; i++, theBuf++, resBuf++) {
	if ( *theBuf < iThreshold ) {
	  *resBuf = u16infinity;
	}
	else *resBuf = 0;
      }
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)inputBuf;
      for (i=0; i<v; i++, theBuf++, resBuf++) {
	if ( *theBuf < iThreshold ) *resBuf = u16infinity;
	else *resBuf = 0;
      }
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)inputBuf;
      for (i=0; i<v; i++, theBuf++, resBuf++) {
	if ( *theBuf < iThreshold ) *resBuf = u16infinity;
	else *resBuf = 0;
      }
    }
    break;
  case FLOAT :
    {
      float *theBuf = (float *)inputBuf;
      for (i=0; i<v; i++, theBuf++, resBuf++) {
	if ( *theBuf < threshold ) *resBuf = u16infinity;
	else *resBuf = 0;
      }
    }
    break;
  default :
    if ( _verbose_ > 0 ) {
      fprintf( stderr, " %s: input buffer type not handled in switch\n", proc );
    }
    return( -1 );
  }
  return( 1 );
}
























void Compute2DChamfer3InUShortImage( unsigned short int *inputBuf,
				      const int *theDim,
				      int *theCoeff )
{
  int y, z;
  register int x;

  register int dimx = theDim[0];
  register int dimxMinus1 = theDim[0]-1;
  int dimxMinus2 = theDim[0]-2;
  register int dimxPlus1 = theDim[0]+1;

  int dimy = theDim[1];
  int dimyMinus2 = theDim[1]-2;

  int dimz = theDim[2];

  /* Aijk
     i = sum |dx|
     j = max |dx|
     k = min |dx| (calcule en 3D)
  */
  register int a110=theCoeff[0];
  register int a210=theCoeff[1];

  register int currentValue, updatedValue;

  register unsigned short int *theBuf = inputBuf;

 

  



  for ( z=0; z<dimz; z++ ) {

    theBuf = inputBuf;
    theBuf += z*dimx*dimy;


    /* ==============
       boucle FORWARD
       ==============
    */
    
    /* (x,y) = (0,0) 
     */
    theBuf ++;
    
    /* (x,y) = ([1,dimx-1],0)
     */
    for ( x=1; x<dimx; x++,  theBuf++ ) {
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf-1) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
    }
    
    /* (x,y) = ([0,dimx-1],[1,dimy-1])
     */
    
    for ( y=1; y<dimy; y++ ) {
      
      /* (x,y) = (0,y) 
       */
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf-dimx) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
      theBuf ++;
      
      /* (x,y) = ([1,dimx-2],y) 
       */
      for ( x=1; x<dimxMinus1; x++,  theBuf++ ) {
	updatedValue = *theBuf;
	currentValue = (int)*(theBuf-dimxPlus1) + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimx) + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxMinus1) + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-1) + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	*theBuf = updatedValue;
      }
      
      /* (x,y) = (dimx-1,y) 
       */
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf-dimxPlus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimx) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-1) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
      theBuf ++;
    }
    
    
    /* on a depasse le dernier point de l'image,
       => on revient un coup en arriere
    */
    theBuf --;
    
    
    /* ===============
       boucle BACKWARD
       ===============
     */
    
    /* (x,y) = (dimx-1,dimy-1) 
     */
    theBuf --;
    


    /* (x,y) = ([dimx-2,0],dimy-1)
     */
    for ( x=dimxMinus2; x>=0; x--,  theBuf-- ) {
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf+1) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
    }
    
    /* (x,y) = ([dimx-1,0],[dimy-2,0])
     */
    for ( y=dimyMinus2; y>=0; y-- ) {
      
      /* (x,y) = (dimx-1,y) 
       */
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf+dimx) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
      theBuf --;
      
      /* (x,y) = ([dimx-2,1],y)
       */
      for ( x=dimxMinus2; x>=1; x--,  theBuf-- ) {
	updatedValue = *theBuf;
	currentValue = (int)*(theBuf+dimxPlus1) + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimx) + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxMinus1) + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+1) + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	*theBuf = updatedValue;
      }
      
      /* (x,y) = (0,y)
       */
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf+dimxPlus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimx) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+1) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
      theBuf --;
    }

  }

}

























void Compute3DChamfer3InUShortImage( unsigned short int *inputBuf,
				     const int *theDim,
				     int *theCoeff )
{
  int y, z;
  register int x;

  int dimx = theDim[0];
  int dimxMinus1 = theDim[0]-1;
  int dimxMinus2 = theDim[0]-2;
  int dimxPlus1 = theDim[0]+1;

  int dimy = theDim[1];
  int dimyMinus1 = theDim[1]-1;
  int dimyMinus2 = theDim[1]-2;

  int dimz = theDim[2];

  int dimxy = theDim[0]*theDim[1];
  int dimxyMinusDxMinus1 = dimxy - dimx - 1;
  int dimxyMinusDx       = dimxy - dimx;
  int dimxyMinusDxPlus1  = dimxy - dimx + 1;
  int dimxyMinus1        = dimxy - 1;
  int dimxyPlus1         = dimxy + 1;
  int dimxyPlusDxMinus1  = dimxy + dimx - 1;
  int dimxyPlusDx        = dimxy + dimx;
  int dimxyPlusDxPlus1   = dimxy + dimx + 1;

  /* Aijk
     i = sum |dx|
     j = max |dx|
     k = min |dx| (calcule en 3D)
  */
  register int a110=theCoeff[0];
  register int a210=theCoeff[1];
  register int a311=theCoeff[2];

  register int currentValue, updatedValue;

  register unsigned short int *theBuf = inputBuf;

 






  /* ==============
     boucle FORWARD
     ==============
  */


  /* (x,y,z) = (0,0,0) 
   */
  theBuf ++;
  
  /* (x,y,z) = ([1,dimx-1],0,0)
   */
  for ( x=1; x<dimx; x++,  theBuf++ ) {
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf-1) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    *theBuf = updatedValue;
  }
    
  /* (x,y,z) = ([0,dimx-1],[1,dimy-1],0)
   */

  for ( y=1; y<dimy; y++ ) {
      
    /* (x,y,z) = (0,y,0) 
     */
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf-dimx) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxMinus1) + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    *theBuf = updatedValue;
    theBuf ++;
      
    /* (x,y,z) = ([1,dimx-2],y,0) 
     */
    for ( x=1; x<dimxMinus1; x++,  theBuf++ ) {
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf-dimxPlus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimx) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-1) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
    }
    
    /* (x,y,z) = (dimx-1,y,0) 
     */
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf-dimxPlus1) + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimx) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-1) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    *theBuf = updatedValue;
    theBuf ++;
  }


  
  
  /* (x,y,z) = ([0,dimx-1],[0,dimy-1],[1,dimz-1])
   */

  for ( z=1; z<dimz; z++ ) {
    /* (x,y,z) = (0,0,z)
     */
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf-dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyMinus1)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyMinusDx)       + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyMinusDxMinus1) + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    *theBuf = updatedValue;
    theBuf ++;

    /* (x,y,z) = ([1,dimx-2],0,z)
     */
    for ( x=1; x<dimxMinus1; x++, theBuf++ ) {
      updatedValue = *theBuf;
      currentValue = (int)*(theBuf-dimxyPlus1)         + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinus1)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinusDxPlus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinusDx)       + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinusDxMinus1) + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-1) + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      *theBuf = updatedValue;
    }

    /* (x,y,z) = (dimx-1,0,z) 
     */
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf-dimxyPlus1)         + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyMinusDxPlus1)  + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyMinusDx)       + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-1) + a110;
    *theBuf = updatedValue;
    theBuf ++;



    
    /* (x,y,z) = (x,[1,dimy-2],z)
     */
    for ( y=1; y<dimyMinus1; y++ ) {

      /* (x,y,z) = (0,[1,dimy-2],z)
       */
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf-dimxyPlusDx)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyPlusDxMinus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinus1)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinusDx)       + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinusDxMinus1) + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;

      currentValue = (int)*(theBuf-dimx)       + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      *theBuf = updatedValue;
      theBuf ++;

      /* (x,y,z) = ([1,dimx-2],[1,dimy-2],z)
       */
      for ( x=1; x<dimxMinus1; x++, theBuf++ ) {
	updatedValue = *theBuf;

	currentValue = (int)*(theBuf-dimxyPlusDxPlus1)   + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxyPlusDx)        + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxyPlusDxMinus1)  + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxyPlus1)         + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxy)              + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxyMinus1)        + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxyMinusDxPlus1)  + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxyMinusDx)       + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxyMinusDxMinus1) + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	
	currentValue = (int)*(theBuf-dimxPlus1)  + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimx)       + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-dimxMinus1) + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf-1)          + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;

	*theBuf = updatedValue;
      }
      
      /* (x,y,z) = (dimx-1,[1,dimy-2],z)
       */
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf-dimxyPlusDxPlus1)   + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyPlusDx)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyPlus1)         + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinusDxPlus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinusDx)       + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      currentValue = (int)*(theBuf-dimxPlus1)  + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimx)       + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-1)          + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      *theBuf = updatedValue;
      theBuf ++;

    }

    /* (x,y,z) = (0,dimy-1,z) 
     */
    updatedValue = *theBuf;

    currentValue = (int)*(theBuf-dimxyPlusDx)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyPlusDxMinus1)  + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyMinus1)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
	
    currentValue = (int)*(theBuf-dimx)       + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxMinus1) + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;

    *theBuf = updatedValue;
    theBuf ++;

    /* (x,y,z) = ([1,dimx-2],dimy-1,z)
     */
    for ( x=1; x<dimxMinus1; x++, theBuf++ ) {
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf-dimxyPlusDxPlus1)   + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyPlusDx)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyPlusDxMinus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyPlus1)         + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxyMinus1)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      currentValue = (int)*(theBuf-dimxPlus1)  + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimx)       + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf-1)          + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      *theBuf = updatedValue;
    }

    /* (x,y,z) = (dimx-1,dimy-1,z) 
     */
    updatedValue = *theBuf;

    currentValue = (int)*(theBuf-dimxyPlusDxPlus1)   + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyPlusDx)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxyPlus1)         + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
	
    currentValue = (int)*(theBuf-dimxPlus1)  + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-dimx)       + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf-1)          + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;

    *theBuf = updatedValue;
    theBuf ++;
  }




  /* on a depasse le dernier point de l'image,
     => on revient un coup en arriere
  */
  theBuf --;


  /* ===============
     boucle BACKWARD
     ===============
  */



  


  /* (x,y,z) = (dimx-1,dimy-1,dimz-1) 
   */
  theBuf --;
  
  /* (x,y,z) = ([dimx-2,0],dimy-1,dimz-1)
   */
  for ( x=dimxMinus2; x>=0; x--,  theBuf-- ) {
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf+1) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    *theBuf = updatedValue;
  }
    
  /* (x,y,z) = ([dimx-1,0],[dimy-2,0],dimz-1)
   */

  for ( y=dimyMinus2; y>=0; y-- ) {
      
    /* (x,y,z) = (dimx-1,y,dimz-1) 
     */
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf+dimx) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxMinus1) + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    *theBuf = updatedValue;
    theBuf --;
    
    /* (x,y,z) = ([dimx-2,1],y,dimz-1)
     */
    for ( x=dimxMinus2; x>=1; x--,  theBuf-- ) {
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf+dimxPlus1)  + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimx)       + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+1)          + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;

      *theBuf = updatedValue;
    }
      
    /* (x,y,z) = (0,y,dimz-1)
     */
    updatedValue = *theBuf;
    currentValue = (int)*(theBuf+dimxPlus1) + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimx) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+1) + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    *theBuf = updatedValue;
    theBuf --;
  }
  


  /* (x,y,z) = ([dimx-1,0],[dimy-1,0],[dimz-2,0])
   */

  for ( z=dimz-2; z>=0; z-- ) {
    /* (x,y,z) = (dimx-1,dimy-1,0) 
     */
    updatedValue = *theBuf;

    currentValue = (int)*(theBuf+dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyMinus1)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyMinusDx)       + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyMinusDxMinus1) + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
      
    *theBuf = updatedValue;
    theBuf --;
     
    /* (x,y,z) = ([dimx-2,1],dimy-1,z)
     */
    for ( x=dimxMinus2; x>=1; x--,  theBuf-- ) {
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf+dimxyPlus1)         + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinus1)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinusDxPlus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinusDx)       + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinusDxMinus1) + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      currentValue = (int)*(theBuf+1)          + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;

      *theBuf = updatedValue;
    }

    /* (x,y,z) = (0,dimy-1,z)
     */
    updatedValue = *theBuf;

    currentValue = (int)*(theBuf+dimxyPlus1)         + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyMinusDxPlus1)  + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyMinusDx)       + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    
    currentValue = (int)*(theBuf+1)          + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;

    *theBuf = updatedValue;
    theBuf --;



    
    /* (x,y,z) = (x,[dimy-2,1],z)
     */
    for ( y=dimyMinus2; y>=1; y-- ) {

      /* (x,y,z) = (dimx-1,y,z)
       */
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf+dimxyPlusDx)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyPlusDxMinus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinus1)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinusDx)       + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinusDxMinus1) + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
	
      currentValue = (int)*(theBuf+dimx)       + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;

      *theBuf = updatedValue;
      theBuf --;

      /* (x,y,z) = ([dimx-2,1],[dimy-2,1],z)
       */
      for ( x=dimxMinus2; x>=1; x--, theBuf-- ) {
	updatedValue = *theBuf;

	currentValue = (int)*(theBuf+dimxyPlusDxPlus1)   + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxyPlusDx)        + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxyPlusDxMinus1)  + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxyPlus1)         + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxy)              + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxyMinus1)        + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxyMinusDxPlus1)  + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxyMinusDx)       + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxyMinusDxMinus1) + a311;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	
	currentValue = (int)*(theBuf+dimxPlus1)  + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimx)       + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+dimxMinus1) + a210;
	if ( updatedValue > currentValue ) updatedValue = currentValue;
	currentValue = (int)*(theBuf+1)          + a110;
	if ( updatedValue > currentValue ) updatedValue = currentValue;

	*theBuf = updatedValue;
      }

      /* (x,y,z) = (dimx-1,[1,dimy-2],z)
       */
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf+dimxyPlusDxPlus1)   + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyPlusDx)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyPlus1)         + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinusDxPlus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinusDx)       + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      currentValue = (int)*(theBuf+dimxPlus1)  + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimx)       + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+1)          + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      *theBuf = updatedValue;
      theBuf --;

    }

    /* (x,y,z) = (dimx-1,0,z) 
     */
    updatedValue = *theBuf;

    currentValue = (int)*(theBuf+dimxyPlusDx)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyPlusDxMinus1)  + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyMinus1)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
      
    currentValue = (int)*(theBuf+dimx)       + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxMinus1) + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;

    *theBuf = updatedValue;
    theBuf --;

    /* (x,y,z) = ([dimx-2,1],0,z)
     */
    for ( x=dimxMinus2; x>=1; x--, theBuf-- ) {
      updatedValue = *theBuf;

      currentValue = (int)*(theBuf+dimxyPlusDxPlus1)   + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyPlusDx)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyPlusDxMinus1)  + a311;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyPlus1)         + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxy)              + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxyMinus1)        + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      currentValue = (int)*(theBuf+dimxPlus1)  + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimx)       + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+dimxMinus1) + a210;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      currentValue = (int)*(theBuf+1)          + a110;
      if ( updatedValue > currentValue ) updatedValue = currentValue;
      
      *theBuf = updatedValue;
    }

    /* (x,y,z) = (0,0,z) 
     */
    updatedValue = *theBuf;

    currentValue = (int)*(theBuf+dimxyPlusDxPlus1)   + a311;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyPlusDx)        + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxyPlus1)         + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimxy)              + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    
    currentValue = (int)*(theBuf+dimxPlus1)  + a210;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+dimx)       + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    currentValue = (int)*(theBuf+1)          + a110;
    if ( updatedValue > currentValue ) updatedValue = currentValue;
    
    *theBuf = updatedValue;
    theBuf --;

  }

}














typedef struct {
  int exists;
  int o;
  int i;
} typeIncrement;






/* 
 *
 *  . c . c .   f e c e f   . f . f .
 *  c b a b c   e d b d e   f e c e f
 *  . a X       c b a b c   . c . c .
 *              e d b d e   f e c e f
 *              f e c e f   . f . f .
 *
 *     z=0         z=1         z=2
 *
 *     29 .. 29      39 32 29 32 39   .. 39 .. 39 ..
 *  29 18 13 18 29   32 23 18 23 32   39 32 29 32 39
 *  .. 13 XX         29 18 13 18 29   .. 29 .. 29 ..
 *                   32 23 18 23 32   39 32 29 32 39
 *                   39 32 29 32 39   .. 39 .. 39 ..
 */

void Compute2DChamfer5InUShortImage( unsigned short int *inputBuf,
				     const int *theDim,
				     int *theCoeff )
{
  int y, z, i, j;
  register int x;
  int jmin, jmax, imin, imax;
  int _INSIDE_;

  int dimx = theDim[0];
  int dimxMinus1 = theDim[0]-1;
  int dimxMinus2 = theDim[0]-2;
  int dimxMinus3 = theDim[0]-3;
  int dimxPlus1 = theDim[0]+1;
  int dimxPlus2 = theDim[0]+2;
  int dimxTimes2Minus1 = 2*theDim[0]-1;
  int dimxTimes2Plus1 = 2*theDim[0]+1;

  int dimy = theDim[1];
  int dimyMinus3 = theDim[1]-3;
  int dimyPlus1 = theDim[1]+1;

  int dimz = theDim[2];
  
  /* Aijk
     i = sum |dx|
     j = max |dx|
     k = min |dx| (calcule en 3D)
  */
  register int a110=theCoeff[0];
  register int a210=theCoeff[1];
  register int a320=theCoeff[2];

  typeIncrement theNeigh[5][5];

  register int currentValue, updatedValue;

  register unsigned short int *theBuf = inputBuf;

  
  /* initialisation des increments
   */
  {
    int max, sum;
    for ( j=0; j<5; j++ )
    for ( i=0; i<5; i++ ) {
      
      theNeigh[j][i].exists = theNeigh[j][i].i = 0;
      theNeigh[j][i].o = (j-2)*dimx + (i-2);

      sum = abs(j-2) + abs(i-2);
      max = abs(j-2);
      if ( max < abs(i-2) ) max = abs(i-2);

      switch ( sum ) {
      case 1 :
	theNeigh[j][i].exists = 1;
	theNeigh[j][i].i = theCoeff[0];
	break;
      case 2 :
	if ( max == 1 ) {
	  theNeigh[j][i].exists = 1;
	  theNeigh[j][i].i = theCoeff[1];
	}
	break;
      case 3 :
	if ( max == 2 ) {
	  theNeigh[j][i].exists = 1;
	  theNeigh[j][i].i = theCoeff[2];
	}
	break;
      }
    }
  }
  

  if ( 0 ) {

    i = j = 0;
    for ( y=0; y<5; y++ )
      for ( x=0; x<5; x++ ) {
	if ( theNeigh[y][x].exists == 1 ) {
	  if ( (y<2) || ((y==2) && (x<2)) ) 
	    /* forwNeigh[i++] = theNeigh[y][x];
	     */
	    i++;
	  if ( (y>2) || ((y==2) && (x>2)) ) 
	    /* backNeigh[j++] = theNeigh[y][x];
	     */
	    j++;
	}
      }
    

    if ( 1 ) {
      printf( "\n #points pour la boucle forward = %d\n", i );
      for ( y=0; y<5; y++ ) {
	for ( x=0; x<5; x++ ) {
	  if (theNeigh[y][x].exists == 1)
	    printf( " %2d ", theNeigh[y][x].i );
	  else
	    printf( " .. " );
	}
	printf( "\n" );
      }
    }
  } /* if ( 0 ) */

  for ( z=0; z<dimz; z++ ) {

    theBuf = inputBuf;
    theBuf += z*dimx*dimy;
    

    /* ==============
       boucle FORWARD
       ==============
    */
    
    for ( y=0; y<dimy; y++ ) {
      /* tests des voisins

	 for ( j=jmin; j<=jmax; j++ )
	 for ( i=imin; i<=imax; i++ )
	    if ( (j<2) || ((j==2) && (i<2)) )

	 soit

	 for ( j=jmin; j<2; j++ )
	 for ( i=imin; i<=imax; i++ )
	    on traite (i,j)
	 for ( i=imin; i<2; i++ )
	    on traite (i,2)
      */
      if ( y < 2 ) {
	jmin = 2-y;
	_INSIDE_ = 0;
      } else {
	jmin = 0;
	_INSIDE_ = 1;
      }

      for ( x=0; x<dimx; x++, theBuf++ ) {
		
	updatedValue = *theBuf;

	if ( (_INSIDE_ == 0) || (x < 2) || (x > dimxMinus3) ) {

	  imin = 0;
	  imax = 4;

	  if ( x < 2 ) {
	    imin = 2-x;
	  } else if ( x > dimxMinus3 ) {
	    imax = dimxPlus1-x;
	  }

	  for ( j=jmin; j<2; j++ ) 
	  for ( i=imin; i<=imax; i++ ) {
	    if ( theNeigh[j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[j][i].o) + theNeigh[j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  j = 2;
	  for ( i=imin; i<2; i++ ) {
	    if ( theNeigh[j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[j][i].o) + theNeigh[j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  
	} else {
	  
	  currentValue = (int)*(theBuf-dimxTimes2Plus1) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf-dimxTimes2Minus1) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf-dimxPlus2) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf-dimxPlus1) + a210;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf-dimx) + a110;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf-dimxMinus1) + a210;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf-dimxMinus2) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf-1) + a110;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;

	}

	*theBuf = updatedValue;
      }
    }

    
    /* on a depasse le dernier point de l'image,
       => on revient un coup en arriere
    */
    theBuf --;
    
    
    /* ===============
       boucle BACKWARD
       ===============
     */
    for ( y=dimy-1; y>=0; y-- ) {

      if ( y > dimyMinus3 ) {
	jmax = dimyPlus1-y;
	_INSIDE_ = 0;
      } else {
	jmax = 4;
	_INSIDE_ = 1;
      }
      
      for ( x=dimx-1; x>=0; x--, theBuf-- ) {

	updatedValue = *theBuf;

	if ( (_INSIDE_ == 0) || (x < 2) || (x > dimxMinus3) ) {

	  imin = 0;
	  imax = 4;

	  if ( x < 2 ) {
	    imin = 2-x;
	  } else if ( x > dimxMinus3 ) {
	    imax = dimxPlus1-x;
	  }

	  for ( j=3; j<=jmax; j++ ) 
	  for ( i=imin; i<=imax; i++ ) {
	    if ( theNeigh[j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[j][i].o) + theNeigh[j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  j = 2;
	  for ( i=3; i<=imax; i++ ) {
	    if ( theNeigh[j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[j][i].o) + theNeigh[j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  
	} else {

	  currentValue = (int)*(theBuf+dimxTimes2Plus1) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf+dimxTimes2Minus1) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf+dimxPlus2) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf+dimxPlus1) + a210;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf+dimx) + a110;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf+dimxMinus1) + a210;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf+dimxMinus2) + a320;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;
	  currentValue = (int)*(theBuf+1) + a110;
	  if ( updatedValue > currentValue ) updatedValue = currentValue;

	}

	*theBuf = updatedValue;

      }
    }

  }

}























void Compute3DChamfer5InUShortImage( unsigned short int *inputBuf,
				     const int *theDim,
				     int *theCoeff )
{
  int y, z, i, j, k;
  register int x;
  int kmin, kmax, jmin, jmax, imin, imax;
  int _INSIDE_;

  int dimx = theDim[0];
  int dimxMinus1 = theDim[0]-1;
  int dimxMinus3 = theDim[0]-3;
  int dimxPlus1 = theDim[0]+1;

  int dimy = theDim[1];
  int dimyMinus1 = theDim[1]-1;
  int dimyMinus3 = theDim[1]-3;
  int dimyPlus1 = theDim[1]+1;

  int dimz = theDim[2];
  
  int dimxy = theDim[0]*theDim[1];


  typeIncrement theNeigh[5][5][5];
  typeIncrement forwNeigh[49];
  typeIncrement backNeigh[49];

  register int currentValue, updatedValue;

  register unsigned short int *theBuf = inputBuf;
  

  
  /* initialisation des increments
   */
  {
    int min, max, sum;
    for ( k=0; k<5; k++ )
    for ( j=0; j<5; j++ )
    for ( i=0; i<5; i++ ) {
      
      theNeigh[k][j][i].exists = theNeigh[k][j][i].i = 0;
      theNeigh[k][j][i].o = (k-2)*dimxy + (j-2)*dimx + (i-2);

      sum = abs(k-2) + abs(j-2) + abs(i-2);
      max = abs(k-2);
      if ( max < abs(j-2) ) max = abs(j-2);
      if ( max < abs(i-2) ) max = abs(i-2);
      min = abs(k-2);
      if ( min > abs(j-2) ) min = abs(j-2);
      if ( min > abs(i-2) ) min = abs(i-2);

      switch ( sum ) {
      case 1 :
	theNeigh[k][j][i].exists = 1;
	theNeigh[k][j][i].i = theCoeff[0];
	break;
      case 2 :
	if ( max == 1 ) {
	  theNeigh[k][j][i].exists = 1;
	  theNeigh[k][j][i].i = theCoeff[1];
	}
	break;
      case 3 :
	if ( max == 2 ) {
	  theNeigh[k][j][i].exists = 1;
	  theNeigh[k][j][i].i = theCoeff[2];
	}
	if ( max == 1 ) {
	  theNeigh[k][j][i].exists = 1;
	  theNeigh[k][j][i].i = theCoeff[3];
	}
	break;
      case 4 :
	if ( max == 2 && min == 1) {
	  theNeigh[k][j][i].exists = 1;
	  theNeigh[k][j][i].i = theCoeff[4];
	}
	break;
      case 5 :
	if ( max == 2 ) {
	  theNeigh[k][j][i].exists = 1;
	  theNeigh[k][j][i].i = theCoeff[5];
	}
	break;
      }
    }
  }
  

  
  i = j = 0;
  for ( z=0; z<5; z++ )
  for ( y=0; y<5; y++ )
  for ( x=0; x<5; x++ ) {
    if ( theNeigh[z][y][x].exists == 1 ) {
      if ( (z<2) || ((z==2) && (y<2)) || ((z==2) && (y==2) && (x<2)) ) 
	forwNeigh[i++] = theNeigh[z][y][x];
      /* i++; */
      if ( (z>2) || ((z==2) && (y>2)) || ((z==2) && (y==2) && (x>2)) ) 
	backNeigh[j++] = theNeigh[z][y][x];
      /* j++; */
    }
  }
  

  if ( 0 ) {
    printf( "\n #points pour la boucle forward = %d\n", i );
    for ( z=0; z<5; z++ ) {
      for ( y=0; y<5; y++ ) {
	for ( x=0; x<5; x++ ) {
	  if (theNeigh[z][y][x].exists == 1)
	    printf( " %2d ", theNeigh[z][y][x].i );
	  else
	    printf( " .. " );
	}
	printf( "\n" );
      }
      printf( "\n" );
    }
  } /* if ( 0 ) */

  

  /* ==============
     boucle FORWARD
     ==============
  */

  for ( z=0; z<dimz; z++ ) {

    if ( z < 2 ) {
      kmin = 2-z;
      _INSIDE_ = 0;
    } else {
      kmin = 0;
      _INSIDE_ = 1;
    }

    for ( y=0; y<dimy; y++ ) {

      jmin = 0;
      jmax = 4;

      if ( y < 2 ) {
	jmin = 2-y;
	_INSIDE_ = 0;
      } else if ( y > dimyMinus3 ) {
	jmax = dimyPlus1-y;
	_INSIDE_ = 0;
      }

      for ( x=0; x<dimx; x++, theBuf++ ) {
		
	updatedValue = *theBuf;

	if ( (_INSIDE_ == 0) || (x < 2) || (x > dimxMinus3) ) {

	  imin = 0;
	  imax = 4;

	  if ( x < 2 ) {
	    imin = 2-x;
	  } else if ( x > dimxMinus3 ) {
	    imax = dimxPlus1-x;
	  }

	  for ( k=kmin; k<2; k++ ) 
	  for ( j=jmin; j<=jmax; j++ ) 
	  for ( i=imin; i<=imax; i++ ) {
	    if ( theNeigh[k][j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[k][j][i].o) + theNeigh[k][j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  k = 2;
	  for ( j=jmin; j<2; j++ ) 
	  for ( i=imin; i<=imax; i++ ) {
	    if ( theNeigh[k][j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[k][j][i].o) + theNeigh[k][j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  j = 2;
	  for ( i=imin; i<2; i++ ) {
	    if ( theNeigh[k][j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[k][j][i].o) + theNeigh[k][j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  
	} else {
	  
	  for ( i=0; i<49; i++ ) {
	    currentValue = (int)*(theBuf+forwNeigh[i].o) + forwNeigh[i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }

	}

	*theBuf = updatedValue;
      }
    }
  }




  /* on a depasse le dernier point de l'image,
     => on revient un coup en arriere
  */
  theBuf --;
    
    
  /* ===============
     boucle BACKWARD
     ===============
  */

  for ( z=dimz-1; z>=0; z-- ) {

    if ( z > dimz-3 ) {
      kmax = 1+dimz-z;
      _INSIDE_ = 0;
    } else {
      kmax = 4;
      _INSIDE_ = 1;
    }
   
    for ( y=dimyMinus1; y>=0; y-- ) {

      jmin = 0;
      jmax = 4;

      if ( y < 2 ) {
	jmin = 2-y;
	_INSIDE_ = 0;
      } else if ( y > dimyMinus3 ) {
	jmax = dimyPlus1-y;
	_INSIDE_ = 0;
      }

      for ( x=dimxMinus1; x>=0; x--, theBuf-- ) {

	updatedValue = *theBuf;

	if ( (_INSIDE_ == 0) || (x < 2) || (x > dimxMinus3) ) {

	  imin = 0;
	  imax = 4;

	  if ( x < 2 ) {
	    imin = 2-x;
	  } else if ( x > dimxMinus3 ) {
	    imax = dimxPlus1-x;
	  }

	  for ( k=3; k<=kmax; k++ ) 
	  for ( j=jmin; j<=jmax; j++ ) 
	  for ( i=imin; i<=imax; i++ ) {
	    if ( theNeigh[k][j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[k][j][i].o) + theNeigh[k][j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  k = 2;
	  for ( j=3; j<=jmax; j++ ) 
	  for ( i=imin; i<=imax; i++ ) {
	    if ( theNeigh[k][j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[k][j][i].o) + theNeigh[k][j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  j = 2;
	  for ( i=3; i<=imax; i++ ) {
	    if ( theNeigh[k][j][i].exists != 1 ) continue;
	    currentValue = (int)*(theBuf+theNeigh[k][j][i].o) + theNeigh[k][j][i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }
	  
	} else {

	  for ( i=0; i<49; i++ ) {
	    currentValue = (int)*(theBuf+backNeigh[i].o) + backNeigh[i].i;
	    if ( updatedValue > currentValue ) updatedValue = currentValue;
	  }

	}

	*theBuf = updatedValue;

      }
    }

  }

}


















void Chamfer_verbose()
{
  if ( _verbose_ <= 0 ) _verbose_ = 1;
  else _verbose_ ++;
}

void Chamfer_noverbose()
{
  _verbose_ = 0;
}


void SetChamferCoefficientsToBorgefors3x3x3()
{
  typeDistance = _BORGEFORS3x3x3_;
}

void SetChamferCoefficientsToMalandain3x3x3()
{
  typeDistance = _MALANDAIN3x3x3_;
}

void SetChamferCoefficientsToDistance04()
{
  typeDistance = _DISTANCE04_;
}

void SetChamferCoefficientsToDistance08()
{
  typeDistance = _DISTANCE08_;
}

void SetChamferCoefficientsToDistance06()
{
  typeDistance = _DISTANCE06_;
}

void SetChamferCoefficientsToDistance18()
{
  typeDistance = _DISTANCE18_;
}

void SetChamferCoefficientsToDistance26()
{
  typeDistance = _DISTANCE26_;
}

void SetChamferCoefficientsToChamfer5x5x5()
{
  typeDistance = _CHAMFER5x5x5_;
}

