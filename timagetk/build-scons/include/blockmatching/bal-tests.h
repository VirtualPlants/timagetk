/*************************************************************************
 * bal-tests.h -
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



#ifndef BAL_TESTS_H
#define BAL_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bal-image.h>

extern void BAL_TestPyramidLevel( int n );
extern void BAL_TestPyramidImage( bal_image *theIm, 
			   int pyramid_gaussian_filtering,
			   int test_inverse_trsf ); 

#ifdef __cplusplus
}
#endif

#endif
