/*************************************************************************
 * linearFiltering-gradient.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Dec 26 19:04:23 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */



/* WARNING, this file is included from linearFiltering.h
 */



extern int gradientModulus2D( void *bufferIn,
                              bufferType typeIn,
                              void *bufferOut,
                              bufferType typeOut,
                              int *bufferDims,
                              int *borderLengths,
                              typeFilteringCoefficients *filter );

extern int gradientModulus3D( void *bufferIn,
                              bufferType typeIn,
                              void *bufferOut,
                              bufferType typeOut,
                              int *bufferDims,
                              int *borderLengths,
                              typeFilteringCoefficients *filter );

extern int gradientModulus( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *filter );
