/*************************************************************************
 * chamfercoefficients.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2014, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Ven 17 jan 2014 11:23:42 CET
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#ifndef _CHAMFERCOEFFICIENTS_H_
#define _CHAMFERCOEFFICIENTS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>



/**************************************************
 * 
 * chamfer-common.c
 *
 **************************************************/



/* MISC: monitoring
 */

extern void _set_chamfer_debug( int d );
extern void _unset_chamfer_debug();

extern void _set_chamfer_trace( int t );
extern void _unset_chamfer_trace();

extern void _set_chamfer_verbose( int v );
extern void _unset_chamfer_verbose();





/* VECTORS
 */

typedef struct Vector {

  /* lattice coordinates */
  int x;
  int y;
  int z;

  /* real world coordinates */
  double vx;
  double vy;
  double vz;

  double vn;

  /* unit vector (v/|v|) coordinates */
  double ux;
  double uy;
  double uz;

  int index; /* for sorting */
  int w;     /* to generate mask */

} Vector;



typedef struct VectorList {

  Vector *allocatedVectors;
  int n_allocated_vectors;
  int n_vectors;

} VectorList;

extern void _init_vector_list( VectorList *vl );

extern void _free_vector_list( VectorList *vl );

extern void _print_2D_vector_list( FILE *f, VectorList *vl );

extern void _print_3D_vector_list( FILE *f, VectorList *vl );



/* CONES
 */

typedef enum {
  _UNSET_  = 0,
  _INSIDE_ = 1,
  _EDGE_IJ_ = 2,
  _EDGE_JK_ = 4,
  _EDGE_KI_ = 8
} LocalisationType;



typedef struct Cone {

  int iv1;
  int iv2;
  int iv3;

  LocalisationType maximumType;

  int convexity;

  double error;

  int flag; /* only required for weights computation */

} Cone;

typedef struct ConeList {

  Cone *allocatedCones;
  int n_allocated_cones;
  int n_cones;

} ConeList;


extern void _init_cone_list( ConeList *cl );

extern void _free_cone_list( ConeList *cl );

extern void _check_3D_mask_naturalconvexity( FILE *f, VectorList *vl, 
                           ConeList *cl, double *s );

extern void _check_3D_mask_fareyregularity( FILE *f, VectorList *vl, ConeList *cl );

extern void _print_cone_list( FILE *f, VectorList *vl, ConeList *cl );

extern int _old_compute_mask_vector_number( VectorList *vl );

extern int _compute_mask_vector_number( VectorList *vl,
                      ConeList *cl, double *voxel_size,
                      int anisotropy_y, int anisotropy_z );



/* WEIGHTS
 */

typedef struct WeightedVector {

  Vector *vec;

  Cone **c;
  int n_cones;

  int w;
  int f;
  int l;

  int flag;

} WeightedVector;
  

typedef struct WeightedVectorList {

  WeightedVector *allocatedVectors;
  int n_allocated_vectors;
  int n_vectors;

} WeightedVectorList;

extern void _init_weighted_vector_list( WeightedVectorList *wvl );
extern void _free_weighted_vector_list( WeightedVectorList *wvl );
extern int _init_weighted_vector_list_from_vector_list( WeightedVectorList *wvl,
                                VectorList *vl );


typedef struct {
  int *w;
  double error;
  double dmin;
  double dmax;
  double epsilon;
  double elapsed_time;
} Weights;

typedef struct {
  double error_optimal;
  int n_weights;
  int n_allocated_weights;
  Weights *allocatedWeights;
} WeightsList;

extern void _init_weights_list( WeightsList *rl );

extern void _free_weights_list( WeightsList *rl );


/**************************************************
 *
 * chamfer-build.c
 *
 **************************************************/

extern void _init_2D_mask( VectorList *vl, ConeList *cl, double *s );

extern void _symmetrise_2D_mask( VectorList *vl, ConeList *cl, double *s,
                     int anisotropy_y );

extern void _complete_2D_mask_in_quadrant( VectorList *vl, ConeList *cl, 
                        double *voxel_size, int anisotropy_y );

extern void _build_2D_mask( VectorList *vl, ConeList *cl, double *s, 
                int size /* 3, 5, ... etc. */
                );

extern void _build_2D_mask_wrt_error( FILE *fout, 
                          VectorList *vl, ConeList *cl,
                          double *s, int size, int n_vectors,
                          double error );



extern void _init_3D_mask( FILE *fout, VectorList *vl, ConeList *cl, double *s );

extern void _symmetrise_3D_mask( FILE *fout, VectorList *vl, ConeList *cl, double *voxel_size,
                     int anisotropy_y, int anisotropy_z );

extern void _complete_3D_mask_in_octant( VectorList *vl, ConeList *cl, 
                       double *voxel_size,
                      int anisotropy_y, int anisotropy_z );

extern void _correct_3D_mask_wrt_naturalconvexity( FILE *f, VectorList *vl, 
                          ConeList *cl, double *s );

extern void _build_3D_mask( FILE *fout, VectorList *vl, ConeList *cl, 
                double *s,
                int size, /* 3, 5, ... etc. */
                int correction /* if <> 0, correct wrt natural convexity */
                );
extern void _build_3D_mask_wrt_error( FILE *fout, VectorList *vl, ConeList *cl, 
                          double *s, int size, int n_vectors,
                          double mask_error, int correction );

extern void _build_3D_mask_3x3x5( FILE *fout, VectorList *vl, ConeList *cl, double *s );
extern void _build_3D_mask_3x5x5( FILE *fout, VectorList *vl, ConeList *cl, double *s );
extern void _build_3D_mask_5x5x5( FILE *fout, VectorList *vl, ConeList *cl, double *s );

extern void _build_3D_mask_3x3x5_2( FILE *fout, VectorList *vl, ConeList *cl, double *s );
extern void _build_3D_mask_5x5x5_2( FILE *fout, VectorList *vl, ConeList *cl, double *s );
extern void _build_3D_mask_3x3x5_3( FILE *fout, VectorList *vl, ConeList *cl, double *s );
extern void _build_3D_mask_5x5x5_3( FILE *fout, VectorList *vl, ConeList *cl, double *s );

extern void _build_3D_mask_7x7x7( FILE *fout, VectorList *vl, ConeList *cl, double *s );

extern int _symmetrise_vector_list( VectorList *nl, VectorList *vl,
                        int anisotropy_y, int anisotropy_z );

/**************************************************
 *
 * chamfer-weights.c
 *
 **************************************************/

extern int _compute_2D_weights( FILE *fout, WeightsList *wl, 
             VectorList *vl, ConeList *cl,
                    int f, int l );
extern int _compute_3D_weights( FILE *fout, WeightsList *wl, 
              VectorList *vl, ConeList *cl,
              double *s, int f, int l );

/**************************************************
 *
 * chamfer-results.c
 *
 **************************************************/

extern void _print_2D_mask_matlab( FILE *f, VectorList *vl, ConeList *cl );
extern void _print_3D_mask_matlab( FILE *f, VectorList *vl, ConeList *cl );

extern void _print_cone_information( FILE *fout, double optimal_err, 
                         double previous_err,
                         double err, double dmin,
                         double dmax, double epsilon,
                         double ct, double it );
extern void _print_mask_information( FILE *fout, double optimal_err, 
                   double previous_err,
                   double err, double dmin,
                   double dmax,
                   double lerr, double ldmin,
                   double ldmax, double lepsilon,
                         double ct, double it );

extern void _print_2D_weighted_vector_list( FILE *f, 
                          WeightedVectorList *wvl, int n );

extern void _print_3D_weighted_vector_list( FILE *f, 
                          WeightedVectorList *wvl, int n );

extern void _print_weights_list( FILE *f, WeightsList *wl,  VectorList *vl );

extern int *_build_buffer_mask( int *dims, VectorList *vl, ConeList *cl, 
                    double *voxel_size, int anisotropy_y, int anisotropy_z );

extern void _print_mask( FILE *f, int *buf, int *dims );

extern void _print_weights_list_in_latex( FILE *f, 
                        WeightsList *wl,  VectorList *vl );




#ifdef __cplusplus
}
#endif

#endif
