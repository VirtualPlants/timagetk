/*************************************************************************
 * recline.h - tools for recursive filtering of 1D lines
 *
 * $Id: recline.h,v 1.5 2003/06/25 07:48:54 greg Exp $
 *
 * Copyright (c) INRIA 1998
 *
 * DESCRIPTION: 
 *
 * Recursive filtering of a line (a 1D array)
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * June, 9 1998
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */

/*
 * recursive filtering of a line (a 1D array)
 */

#ifndef _recline_h_
#define _recline_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <linearFiltering-common.h>



extern void setVerboseInRecLine( int v );
extern void incrementVerboseInRecLine(  );
extern void decrementVerboseInRecLine(  );



typedef struct RFcoefficientType {
  /*--- denominateur       ---*/
  double sd1;
  double sd2;
  double sd3;
  double sd4;
  /*--- numerateur positif ---*/
  double spm;
  double sp0;
  double sp1;
  double sp2;
  double sp3;
  /*--- numerateur negatif ---*/
  double snm;
  double sn0;
  double sn1;
  double sn2;
  double sn3;
  double sn4;
  /*--- type de filtre en cours ---*/
  filterType type_filter;
  derivativeOrder derivative;
  /*--- Triggs matrix,
    for Young/van Vliet filter correction ---*/
  double TriggsMat[9];
} RFcoefficientType;


extern void printRecursiveCoefficients( FILE *theFile,  RFcoefficientType *RFC, char *d );


/* Initialization of coefficients for recursive filtering.
 *
 * PARAMETERS:
 *
 * - the coefficient is the sigma's value in case of
 *   gaussian filtering, or the alpha's value in case
 *   of Deriche's filters.
 *
 * - the coefficient's value must be larger than 0.1
 *   in case of gaussian filtering, and in the
 *   [0.1,1.9] range in case of Deriche's filters.
 *
 * SEE:
 *
 * - filterType
 *
 * - derivativeOrder
 */
extern RFcoefficientType * InitRecursiveCoefficients( double x, /* coefficient's value */
                                       filterType filterType, /* filter's type */
                                       derivativeOrder derivative /* derivative's order */ );



/* 1D recursive filtering along a line.
 *
 * WARNING:
 * Coefficients should already be initialized.
 *
 * SEE:
 *
 * - filterType
 *
 * - derivativeOrder
 *
 * RETURN:
 *
 * - 0 in case of error
 *
 * - 1 if successful
 */
extern int RecursiveFilter1D( RFcoefficientType *RFC,
                              double *in, /* input line */
                              double *out, /* output line */
                              double *work1, /* first work array */
                              double *work2, /* second work array,
                                                could be out if out is different from in */
                              int dim /* lines' length */ );




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _recline_h_ */
