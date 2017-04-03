/*************************************************************************
 * ccparameters.h - extraction de parametres sur des parties numerotees
 *
 * $Id: ccparameters.h,v 1.2 2001/04/03 10:27:21 greg Exp $
 *
 * DESCRIPTION: 
 *
 *
 *
 *
 *
 * AUTHOR:
 * Gregoire Malandain
 *
 * CREATION DATE:
 * Sun Feb 11 10:37:45 MET 2001
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 *
 */

#ifndef _ccparameters_h_
#define _ccparameters_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
/* #include <malloc.h> */
#include <math.h>
#include <string.h>

#ifdef _LINUX_
extern long int random();
extern void srandom(unsigned int seed);
#endif

#include <typedefs.h>

#define _CONFIGURATIONS_ 6

typedef enum {
  _INCONNU_ = 0,
  _ISOLE_ = 1,
  _UNIVOQUE_ = 2,
  _ZIGZAG_ = 3,
  _SPLIT_ = 4,
  _MERGE_ = 5
} enumConfiguration;

typedef enum {
  _VALIDE_,
  _INVALIDE_
} enumStatus;

#define _COMPONENTS_ 10

typedef struct {

  int ptmin[3];
  int ptmax[3];

  int volume;

  double dbary[3];
  int    ibary[3];

  int n_components;
  int i_component[_COMPONENTS_];

  enumConfiguration configuration;
  enumStatus status;

  int intersection[3];
  int iptmin[3];
  int iptmax[3];

  int is_printed;

} typeParameter;


extern typeParameter *ComputeParameterFromLabels( vt_image *theIm,
						  int *ncc );



typedef struct {
  int i_image;
  char *name;

  int n_labels;
  typeParameter *thePar;
} typeImageParameter;

void _print_same_components( FILE *f, enumConfiguration cfg,
			    typeImageParameter *par1,
			    typeImageParameter *par2 );

void _get_Statistics( int *stats, typeImageParameter *par );
void _print_Statistics( FILE *f, typeImageParameter *par );
void _print_UnknownComponents( FILE *f,
			       typeImageParameter *par1,
			       typeImageParameter *par2 );
void _print_AllComponents( FILE *f,
			       typeImageParameter *par1,
			       typeImageParameter *par2 );


void _NameComponents( vt_image *im1,
		      typeImageParameter *par1,
		      vt_image *im2,
		      typeImageParameter *par2 );

void _FollowsComponents( vt_image *im1,
			 typeImageParameter *par1,
			 vt_image *im2,
			 typeImageParameter *par2 );


void _printConnectedComponents( FILE *f, int label1,
				typeImageParameter *par1,
				int label2,
				typeImageParameter *par2,
				int spaces );


#ifdef __cplusplus
}
#endif

#endif /* _ccparameters_h_ */
