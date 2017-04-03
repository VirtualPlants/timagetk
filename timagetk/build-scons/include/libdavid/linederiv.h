/*************************************************************************
 * linederiv.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 1998
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Thu Mar  9 17:03:02 MET 2000
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */



#ifndef _linederiv_h_
#define _linederiv_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <recline.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

extern int absoluteDerivative1D( double *input, double *output, int n, double sigma );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 
