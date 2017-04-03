/*
 * vt_connexe.c - connected components computation.
 *
 * DESCRIPTION:
 *
 * AUTHOR:        Gregoire Malandain
 * CREATION DATE: April 1997
 * COPYRIGHT:     Gregoire Malandain
 * RELEASE:       1.0
 *
 * ADDITIONS, CHANGES:
 *
 * - Mon Oct 11 18:47:54 MET DST 1999
 *   * Ajout d'un champ a la structure composante (_my_cc),
 *     'inside', qui sert a la gestion du depassement maximum
 *     de labels.
 *   * reecriture de la gestion du nombre max de labels
 *     et prise en compte des composantes entierement
 *     parcourues.
 *   * ajout du flag binaire
 *   * donne le meme label aux composantes validees dans le
 *     mode binaire
 *
 *
 * - Fri Oct  8 15:09:21 MET DST 1999 (G. Malandain)
 *   Gestion du depassement du nombre max de labels 
 *   Creation d'une nouvelle table et renumerotation 
 *   partielle de l'image.
 *
 *
 */


#include <vt_connexe.h>


typedef struct _my_cc{
  int label;
  int lowValuePoints;
  int highValuePoints;
  char inside;
} _my_cc;


static int _Main_Computation( vt_image *theIm, vt_connexe *par );
static int _Local_Computation( vt_image *theIm, vt_connexe *par, _my_cc **cc );
static void _Sort_CC( _my_cc *tab, int left, int right );
#ifdef _UNUSED_
static _my_cc * VT_RelabelLabelsImage( vt_image *theIm, _my_cc *cc,
                                  int *used_labels,
                                  int xlim, int ylim, int zlim );
#endif
static _my_cc * _LabelsOverflowManagement( vt_image *theIm, _my_cc *cc,
                                           int *used_labels,
                                           int x, int y, int z,
                                           int connexite,
                                           int minCcSize );
static void _SetOutputToBinary();
static void _UnsetOutputToBinary();


/*
 * predefined low and high value.
 * They must verify _hig_value_ > _low_value_.
 *
 * If there is a single threshold (connected components extraction)
 * all points will have the label _hig_value_, which means that the validity
 * of a connected components will depend only on its size.
 *
 * If there are two thresholds (hysteresis thresholding).
 * points between the two thresholds are labeled _low_value_
 * while points above the high threshold are labeled _hig_value_.
 * A connected component is valid if it contains at least one
 * _hig_value point.
 *
 * If there is a seed point, 
 * all points will have the label _low_value_ and the seed point
 * will be labeled _hig_value_.
 */
static u16 _low_value_ = (u16)100;
static u16 _hig_value_ = (u16)200;

/*
 * binary value (single value in case of binary output)
 */
static u16 _bin_value_ = (u16)65535;


static int _binary_output_ = 0;


/*
 * Thresholding one buffer image of given type with one single threshold.
 */
#define _ONE_THRESHOLD_( TYPE, VALUE, OUTPUT_VAL ) {  \
    TYPE *theBuf = (TYPE*)(theIm->buf);               \
    for ( i = 0; i < v ; i++ , theBuf++, auxBuf++ ) { \
      if ( *theBuf < VALUE ) { *auxBuf = (u16)0; continue; } \
      *auxBuf = OUTPUT_VAL;                           \
    }                                                 \
} 
/*
 * Thresholding one image with one single threshold,
 */
#define _IMAGE_THRESHOLD_( THRES, OUTPUT_VAL ) {       \
  dth = (double)(THRES);   ith = VT_F2I( dth );        \
  v = resIm->dim.x * resIm->dim.y * resIm->dim.z;      \
  switch ( theIm->type ) {                             \
  case UCHAR :                                      \
    _ONE_THRESHOLD_( u8, ith, OUTPUT_VAL )  \
    break;                                             \
  case SCHAR :                                      \
    _ONE_THRESHOLD_( s8, ith, OUTPUT_VAL )  \
    break;                                             \
  case USHORT :                                     \
    _ONE_THRESHOLD_( u16, ith, OUTPUT_VAL ) \
    break;                                             \
  case SSHORT :                                     \
    _ONE_THRESHOLD_( s16, ith, OUTPUT_VAL ) \
    break;                                             \
  case SINT :                                        \
    _ONE_THRESHOLD_( i32, ith, OUTPUT_VAL )    \
    break;                                             \
  case FLOAT :                                      \
    _ONE_THRESHOLD_( r32, dth, OUTPUT_VAL )  \
    break;                                             \
  default :                                            \
    VT_Error( "input image type unknown or not supported", proc ); \
    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );       \
    return( -1 );                                \
  }                                                    \
}

/*
 * Common first part of procedures 
 *
 * Tests of images
 * Initialization of auxiliary image
 */
#define _FIRST_PART_ {                                                              \
  if ( VT_Test2Image( resIm, theIm, proc ) == -1 ) return( -1 );        \
  VT_InitFromImage( &auxIm, resIm, "connected.components.volume", (int)USHORT ); \
  if ( resIm->type == USHORT ) {                                                 \
    auxIm.buf = resIm->buf;   auxIm.array = resIm->array;               \
  } else {                                                                          \
    if ( VT_AllocImage( &auxIm ) != 1 ) {                                     \
      VT_Error( "allocation failed for auxiliary image", proc );                    \
      return( -1 );                                                           \
    }                                                                               \
    auxIsAllocated = 1;                                                       \
  }                                                                                 \
  auxBuf = (u16*)auxIm.buf;                                              \
}

/*
 * Main computation
 */
#define _MAIN_COMPUTATION_ {                                  \
  if ( _Main_Computation( &auxIm, &local_par ) != 1 ) { \
    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );  \
    return( -1 );                                       \
  }                                                           \
}

/*
 * Auxiliary image to output image
 * VT_CopyImage() could be optimized.
 */
#define _AUXILIARY_2_OUTPUT {                                                         \
  if ( auxIsAllocated == 1 ) {                                                  \
    switch ( par->type_output ) {                                                     \
    case VT_BINAIRE :                                                                 \
      if ( VT_Threshold( &auxIm, resIm, (float)1.0 ) != 1 ) {                   \
        VT_Error( "unable to copy the auxiliary image into the result image", proc ); \
        VT_FreeImage( &auxIm );                                                       \
        return( -1 );                                                           \
      }                                                                               \
      break;                                                                          \
    case VT_GREY :                                                                    \
    default :                                                                         \
      if ( VT_CopyImage( &auxIm, resIm ) != 1 ) {                               \
        VT_Error( "unable to copy the auxiliary image into the result image", proc ); \
        VT_FreeImage( &auxIm );                                                       \
        return( -1 );                                                           \
      }                                                                               \
    }                                                                                 \
    VT_FreeImage( &auxIm );                                                           \
  }                                                                                   \
}

/*
 * Connected component extraction.
 *
 * We binarize the input image into an auxiliary image with the given threshold.
 * All the non-zero points in the auxiliary image are labeled _hig_value_,
 * thus we make sure to keep all the connected components.
 */
int VT_ConnectedComponents( vt_image *theIm /* input image */, 
                            vt_image *resIm /* output image */,
                            float threshold /* threshold to binarize */,
                            vt_connexe *par /* parameters structure */ )
{
  char *proc="VT_ConnectedComponents";
  vt_connexe local_par = *par;  
  vt_image auxIm;
  register int i, v, ith;
  register double dth;
  int auxIsAllocated = 0;
  u16 *auxBuf;

  /*
   * common first part
   */
  _FIRST_PART_

  /*  
   * Thresholding with output value = _hig_value
   */
  _IMAGE_THRESHOLD_( threshold, _hig_value_ )
  
  /*  
   * Main computation
   */
  _MAIN_COMPUTATION_

  /*
   * Auxiliary image to output image
   */
  _AUXILIARY_2_OUTPUT 

  return( 1 );
}





/*
 * Hysteresis thresholding.
 *
 * We ternarize (?) the input image into an auxiliary image with the given thresholds.
 * All the points with value lower than low are set to 0,
 * points with value >= low and value < high are set to _low_value_
 * points with value > high are set to _hig_value_
 */

int VT_Hysteresis( vt_image *theIm /* input image */, 
                   vt_image *resIm /* output image */,
                   float low /* low threshold */,
                   float high /* high threshold */,
                   vt_connexe *par /* parameters structure */ )
{
  char *proc="VT_Hysteresis";
  vt_connexe local_par = *par;  
  vt_image auxIm;
  register int i, v, isb, ish;
  register double dsb, dsh;
  int auxIsAllocated = 0;
  u16 *auxBuf;

  /*
   * common first part
   */
  _FIRST_PART_

  /*
   * thresholds of the the input image
   */
  dsb = (double)low;   isb = VT_F2I( dsb );
  dsh =  (double)high; ish = VT_F2I( dsh );
  v = resIm->dim.x * resIm->dim.y * resIm->dim.z;

#define _TWO_THRESHOLDS_( TYPE, LOW, HIGH ) {         \
    TYPE *theBuf = (TYPE*)(theIm->buf);               \
    for ( i = 0; i < v ; i++ , theBuf++, auxBuf++ ) { \
      if ( *theBuf < LOW )  { *auxBuf = (u16)0; continue; } \
      if ( *theBuf < HIGH ) { *auxBuf = _low_value_; continue; }       \
      *auxBuf = _hig_value_;                          \
    }                                                 \
}

  switch ( theIm->type ) {
  case UCHAR :
    _TWO_THRESHOLDS_( u8, isb, ish )
    break;
  case SCHAR :
    _TWO_THRESHOLDS_( s8, isb, ish )
    break;
  case USHORT :
    _TWO_THRESHOLDS_( u16, isb, ish )
    break;
  case SSHORT :
    _TWO_THRESHOLDS_( s16, isb, ish )
    break;
  case SINT :
    _TWO_THRESHOLDS_( i32, isb, ish )
    break;
  case FLOAT :
    _TWO_THRESHOLDS_( r32, dsb, dsh )
    break;
  default :
    VT_Error( "input image type unknown or not supported", proc );
    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
    return( -1 );
  }
  
  /*  
   * Main computation
   */
  _MAIN_COMPUTATION_

  /*
   * Auxiliary image to output image
   */
  _AUXILIARY_2_OUTPUT 

  return( 1 );
}





/*
 * Connected component extraction (containing a seed)
 *
 * We binarize the input image into an auxiliary image with the given threshold.
 * All the non-zero points in the auxiliary image are labeled _low_value_,
 * the seed point is labeled _hig_value_.
 */

int VT_ConnectedComponentWithOneSeed( vt_image *theIm /* input image */, 
                                 vt_image *resIm /* output image */,
                                 float threshold /* threshold to binarize */,
                                 vt_ipt *seed /* seed (3D point) */,
                                 vt_connexe *par /* parameters structure */ )
{
  char *proc="VT_ConnectedComponentWithOneSeed";
  vt_connexe local_par = *par;  
  vt_image auxIm;
  register int i, v, ith;
  register double dth;
  int auxIsAllocated = 0;
  u16 *auxBuf;

  if ( (seed->x < 0) || (seed->x >= (int)theIm->dim.x) ||
       (seed->y < 0) || (seed->y >= (int)theIm->dim.y) ||
       (seed->z < 0) || (seed->z >= (int)theIm->dim.z) ) {
    VT_Error( "Bad seed point (not in the image)", proc );
    return( -2 );
  }

  /*
   * common first part
   */
  _FIRST_PART_

  /*  
   * Thresholding with output value = _low_value_
   */
  _IMAGE_THRESHOLD_( threshold, _low_value_ )
  
  /*
   * The seed point will have the _hig_value_ label
   * if the seed point is not in the binary image => error
   */
  i = seed->z * resIm->dim.x * resIm->dim.y + seed->y * resIm->dim.x + seed->x;
  auxBuf = (u16*)auxIm.buf; 
  if ( auxBuf[i] < _low_value_ ) {
    VT_Error( "Bad seed point (not in the binary image)", proc );
    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
    return( -2 );
  }
  auxBuf[i] = _hig_value_;
  local_par.min_size = 1;
  local_par.max_nbcc = 0;
  local_par.type_output = VT_BINAIRE;

  /*  
   * Main computation
   */
  _MAIN_COMPUTATION_

  /*
   * Auxiliary image to output image
   */
  _AUXILIARY_2_OUTPUT 

  return( 1 );
}











/*
 * Connected component extraction (containing seeds)
 *
 * We binarize the input image into an auxiliary image with the given threshold.
 * All the non-zero points in the auxiliary image are labeled _low_value_,
 * the seed points are labeled _hig_value_.
 */

int VT_ConnectedComponentsWithSeeds( vt_image *theIm /* input image */, 
                                 vt_image *resIm /* output image */,
                                 float threshold /* threshold to binarize */,
                                 vt_image *seeds /* seed (3D point) */,
                                 vt_connexe *par /* parameters structure */ )
{
  char *proc="VT_ConnectedComponentsWithSeeds";
  vt_connexe local_par = *par;  
  vt_image auxIm;
  register int i, v, ith;
  register double dth;
  int auxIsAllocated = 0;
  u16 *auxBuf;
  int SomethingIsInside;
  u8 *seedBuf;

  if ( VT_Test2Image( seeds, theIm, proc ) == -1 ) return( -1 );
  if ( seeds->type != UCHAR ) {
    VT_Error( "seeds image type should be unsigned char", proc );
    return( -1 );
  }
  /*
   * common first part
   */
  _FIRST_PART_

  /*  
   * Thresholding with output value = _low_value_
   */
  _IMAGE_THRESHOLD_( threshold, _low_value_ )
  
  /*
   * The seed points will have the _hig_value_ label
   * they have a non zero value.
   * if a seed point is not in the binary image => nothing
   */
  auxBuf = (u16*)auxIm.buf; 
  seedBuf = (u8*)seeds->buf;
  SomethingIsInside = 0;

  v = seeds->dim.z * seeds->dim.y * seeds->dim.x;
  for ( i = 0; i < v ; i ++ ) {
    if ( seedBuf[i] > (u8)0 ) {
      if ( auxBuf[i] < _low_value_ ) continue;
      auxBuf[i] = _hig_value_;
      SomethingIsInside = 1;
    }
  }
  
  if ( SomethingIsInside == 0 ) {
    VT_Error( "No seed points inside the input image", proc );
    if ( auxIsAllocated == 1 ) VT_FreeImage( &auxIm );
    return( -2 );
  }
  local_par.min_size = 1;
  /* local_par.max_nbcc = 0; */

  /*  
   * Main computation
   */
  _MAIN_COMPUTATION_

  /*
   * Auxiliary image to output image
   */
  _AUXILIARY_2_OUTPUT 

  return( 1 );
}






/*
 * Here is the global point.
 *
 * Allocation of the equivalence table.
 * What is needed from the parameter structure par:
 * - what is already needed by _Local_Computation
 * - the type of computation (3D or slice by slice): par->dim
 */

static int _Main_Computation( vt_image *theIm /* pre-processed image */, 
                              vt_connexe *par /* parameters structure */ )
{
  _my_cc *components = (_my_cc *)NULL;
  u16 *theBuf;
  int i, v;
  char *proc="_Main_Computation";

  /*
   * Equivalence table initialization
   * 
   * We allocate the largest equivalence table which may be needed
   * for a labeling from 1 to 65535 (image of type unsigned short).
   * 
   * Dynamic allocation is possible if needed.
   */
  
  components = (_my_cc *)VT_Malloc( (unsigned int)((int)_EQUIVALENCE_ARRAY_SIZE_ * sizeof(_my_cc)) );
  if ( components == (_my_cc *)NULL ) {
    VT_Error( "allocation failed for equivalence array", proc );
    return( -1 );
  }
  
  if ( par->type_output == VT_BINAIRE ) _SetOutputToBinary();

  if ( par->dim == VT_2D ) {
    /*
     * 2D case
     */
    vt_image sliceIm;
    int z;

    VT_Image( &sliceIm );
    VT_InitImage( &sliceIm, "connected.components.slice", theIm->dim.x, theIm->dim.y, (int)1, (int)USHORT );

    v = theIm->dim.x * theIm->dim.y;
    for ( z = 0; z < (int)theIm->dim.z; z ++ ) {
      /*
       * "copy" of a 3D slice into the 2D image
       */
      theBuf = (u16*)(theIm->buf);
      theBuf += z * v;
      sliceIm.buf = (void *)(theBuf);
      /*
       * computation
       */
      if ( _Local_Computation( &sliceIm, par, &components ) != 1 ) {
        VT_Error( "computation of 2D connected components failed", proc );
        VT_Free( (void**)(&components) );
        _UnsetOutputToBinary();
        return( -1 );
      }
      /*
       * relabeling of input image
       */
      theBuf = (u16*)(sliceIm.buf);
      for ( i = 0; i < v; i ++, theBuf++ )
        *theBuf = (u16)( components[ (int)(*theBuf) ].label );
    }
  } else {
    /*
     * 3D case
     */
    if ( _Local_Computation( theIm, par, &components ) != 1 ) {
      VT_Error( "computation of 3D connected components failed", proc );
      VT_Free( (void**)(&components) );
      _UnsetOutputToBinary();
      return( -1 );
    }

    /*
     * relabeling of input image
     */
    v =  theIm->dim.x * theIm->dim.y * theIm->dim.z;
    theBuf = (u16*)(theIm->buf);
    for ( i = 0; i < v; i ++, theBuf++ )
      *theBuf = (u16)( components[ (int)(*theBuf) ].label );
  }

  VT_Free( (void**)(&components) );
  _UnsetOutputToBinary();
  return( 1 );
}





/*
 * Here is the point
 *
 * What is needed from the parameter structure par:
 * - the connectivity (par->type_connexite)
 * - the minimum size of a connected component (par->min_size)
 * - the maximum number of connected components (par->max_nbcc)
 * - the type of output (par->type_output)
 */
static int _Local_Computation( vt_image *theIm /* pre-processed image */, 
                               vt_connexe *par /* parameters structure */,
                               _my_cc **theCc /* equivalence table */ )
{
  char *proc="_Local_Computation";
  int i, j, k, x, y, z, _INSIDEY_;
  u16 current_value, *theBuf = (u16 *)(theIm->buf);
  _my_cc *cc = *theCc;
  int dimx, dimy, dimx1, dimy1, dimxy;
  int array_offset[3][3][2], offsets[13], nb_offsets;
  int local_connexite, used_labels = 0;
  register int current_label;
  int nb_neighbors, label_neighbors[13];
  int valid_labels = 0;

  _my_cc *tmpCc = (_my_cc *)NULL;
  int tmpUsed = 0;
  
  /*
   * equivalence table initialization
  for ( i = 0; i < (int)_EQUIVALENCE_ARRAY_SIZE_; i ++ ) {
    cc[i].label = cc[i].lowValuePoints = 0;
    cc[i].highValuePoints = 0;
  }
   */
  cc[0].label = 0;

  /*
   * sizes
   */
  dimx = theIm->dim.x;   dimx1 = dimx - 1;
  dimy = theIm->dim.y;   dimy1 = dimy - 1;
  dimxy = dimx * dimy;

  /*
   * offsets initialization
   * we duplicate the offsets :
   * 1. we use an 1D array (offsets[]) when we are sure that the neighbors of the current point
   *    are all inside the image.
   * 2. we use an 3D array (array_offset[][][]) when we have to check the neighbors
   */
#define _INIT_CONNECTIVITY( CONNECTIVITY ) {                                                       \
  for ( i = 0; i < 3; i ++ )                                                                       \
  for ( j = 0; j < 3; j ++ )                                                                       \
  for ( k = 0; k < 2; k ++ )                                                                       \
    array_offset[i][j][k] = 0;                                                                     \
  for ( i = 0; i < 13; i ++ )                                                                      \
    offsets[i] = 0;                                                                                \
  nb_offsets = 0;                                                                                  \
  switch ( CONNECTIVITY ) {                                                                        \
  case N08 :                                                                              \
    array_offset[0][0][1] = offsets[ nb_offsets++ ] =         - dimx - 1; /* point (-1, -1,  0) */ \
    array_offset[2][0][1] = offsets[ nb_offsets++ ] =         - dimx + 1; /* point ( 1, -1,  0) */ \
  case N04 :                                                                              \
    array_offset[1][0][1] = offsets[ nb_offsets++ ] =         - dimx;     /* point ( 0, -1,  0) */ \
    array_offset[0][1][1] = offsets[ nb_offsets++ ] =                - 1; /* point (-1,  0,  0) */ \
    break;                                                                                         \
  case N26 :                                                                              \
    array_offset[0][0][0] = offsets[ nb_offsets++ ] = - dimxy - dimx - 1; /* point (-1, -1, -1) */ \
    array_offset[2][0][0] = offsets[ nb_offsets++ ] = - dimxy - dimx + 1; /* point ( 1, -1, -1) */ \
    array_offset[0][2][0] = offsets[ nb_offsets++ ] = - dimxy + dimx - 1; /* point (-1,  1, -1) */ \
    array_offset[2][2][0] = offsets[ nb_offsets++ ] = - dimxy + dimx + 1; /* point ( 1,  1, -1) */ \
  case N18 :                                                                              \
    array_offset[1][0][0] = offsets[ nb_offsets++ ] = - dimxy - dimx;     /* point ( 0, -1, -1) */ \
    array_offset[0][1][0] = offsets[ nb_offsets++ ] = - dimxy        - 1; /* point (-1,  0, -1) */ \
    array_offset[2][1][0] = offsets[ nb_offsets++ ] = - dimxy        + 1; /* point ( 1,  0, -1) */ \
    array_offset[1][2][0] = offsets[ nb_offsets++ ] = - dimxy + dimx;     /* point ( 0, -1, -1) */ \
  case N10 :                                                                              \
    array_offset[0][0][1] = offsets[ nb_offsets++ ] =         - dimx - 1; /* point (-1, -1,  0) */ \
    array_offset[2][0][1] = offsets[ nb_offsets++ ] =         - dimx + 1; /* point ( 1, -1,  0) */ \
  case N06 :                                                                              \
    array_offset[1][1][0] = offsets[ nb_offsets++ ] = - dimxy;            /* point ( 0,  0, -1) */ \
    array_offset[1][0][1] = offsets[ nb_offsets++ ] =         - dimx;     /* point ( 0, -1,  0) */ \
    array_offset[0][1][1] = offsets[ nb_offsets++ ] =                - 1; /* point (-1,  0,  0) */ \
  }                                                                                                \
}

  /*
   * Computation of a point's label : current_label
   * If this point has some neighbors (i.e. nb_neighbors > 0) then
   *   1. Its label is the minimum value of all the neighbors labels
   *   2. We report the equivalences between all the labels
   * If not (i.e. nb_neighbors == 0) then
   *   we use a new label (potential case of error : label overflow)
   */
#define _LABEL_COMPUTATION {                                                             \
      if ( nb_neighbors > 0 ) {                                                          \
        /*                                                                               \
         * If there are some neighbors,                                                  \
         * the equivalent label for all of them is the minimum                           \
         * of all labels.                                                                \
         */                                                                              \
        current_label = label_neighbors[0];                                              \
        for ( i = 1; i < nb_neighbors; i ++ )                                            \
          if ( label_neighbors[i] < current_label ) current_label = label_neighbors[i];  \
        /*                                                                               \
         * we have to report the equivalences inside the equivalence table               \
         */                                                                              \
        for ( i = 0; i < nb_neighbors; i ++ ) {                                          \
          if ( label_neighbors[i] != current_label ) {                                   \
            k = label_neighbors[i];                                                      \
            /*                                                                           \
             * si la classe n'a pas deja ete traitee                                     \
             */                                                                         \
            if ( cc[ k ].label != current_label ) {                                     \
              cc[ current_label ].lowValuePoints += cc[ k ].lowValuePoints;     \
              cc[ k ].lowValuePoints = 0;    \
               cc[ current_label ].highValuePoints += cc[ k ].highValuePoints;   \
              cc[ k ].highValuePoints = 0;   \
              cc[ k ].label = current_label;                               \
              for ( j = k+1; j <= used_labels; j ++ )                      \
                if ( cc[ j ].label == k ) cc[ j ].label = current_label;   \
            }                                                              \
          }                                                                \
        }                                                                  \
      } else {                                                             \
        /*                                                                               \
         * No neighbors :                                                                \
         * we use a new label for this point.                                            \
         * This is the only case of error in the whole procedure.                        \
         * This could be a little bit improved as follows:                               \
         * 1. we re-label the potential equivalence classes (without conditions of       \
         *    validity, i.e. without testing cc[ i ].highValuePoints and cc[ i ].lowValuePoints) to get       \
         *    the effective number of equivalence classes at this time.                  \
         * 2. using this re-labeling, we change the image labels up to the current       \
         *    point.                                                                     \
         * 3. we re-build the equivalence table to have only one entry per equivalence   \
         *    classe.                                                                    \
         */                                                                              \
        current_label = ++used_labels;                                                   \
        if ( used_labels > (_EQUIVALENCE_ARRAY_SIZE_ - 1) ) {                         \
          /* on essaie de faire quelque chose \
           */                                 \
          tmpUsed = used_labels-1; \
          tmpCc = _LabelsOverflowManagement( theIm, cc, &tmpUsed, x, y, z, \
                                             local_connexite, par->min_size ); \
          if ( (tmpCc == (_my_cc *)NULL) || (tmpUsed >= used_labels-1 ) ) {   \
            VT_Error( "Too much used labels for connected components computation", proc ); \
            if ( tmpCc != (_my_cc *)NULL ) free( tmpCc );             \
            return( -1 );                   \
          }                                 \
          else {                            \
            used_labels = tmpUsed;                \
            current_label = ++used_labels;  \
            *theCc = tmpCc;                 \
            free( cc );                     \
            cc = tmpCc;                     \
          }                                 \
          /*                                  \
          VT_Error( "Too much used labels for connected components computation", proc ); \
          return( -1 );                                                            \
          */                                  \
        }                                                                                \
        cc[ current_label ].label = current_label;                                       \
        cc[ current_label ].lowValuePoints  = 0;                                                   \
        cc[ current_label ].highValuePoints  = 0;                                            \
      }                                                                                  \
}

  /*
   * Generic tests of all the neighbors
   * we make sure that they belong to the image.
   * We check if the neighbor has a label (value > 0)
   * We are looking for the equivalence classe (i.e. cc[ value ].label) of the neighbors
   */
#define _GENERIC_TEST {                                                                  \
    for ( i = 0; i < nb_offsets; i ++ )                                                  \
      if ( *(theBuf + offsets[i]) > 0 )                                                  \
        label_neighbors[ nb_neighbors++ ] = cc[ (int)( *(theBuf + offsets[i]) ) ].label; \
}

  /*
   * Specific tests for the upper row of the neighborhood of a point
   * We check if the neighbor is connected to the current point
   *          if the neighbor belongs to the image
   *          if the neighbor has a label
   */
#define _UPPER_ROW_TESTS {                                                                              \
    if ( y > 0 ) {                                                                                      \
      for ( i = -1; i <= 1; i ++ ) {                                                                    \
        if ( array_offset[1+i][0][1] == 0 ) continue;                                                   \
        if ( (x+i < 0) || (x+i >= dimx) )   continue;                                                   \
        if ( *(theBuf + array_offset[1+i][0][1]) > 0 )                                                  \
          label_neighbors[ nb_neighbors++ ] = cc[ (int)( *(theBuf + array_offset[1+i][0][1]) ) ].label; \
      }                                                                                                 \
    }                                                                                                   \
}

  /*
   * Specific test for the left neighbor of a point
   * We should test if (array_offset[0][1][1] != 0) but we
   * do not because the point (x-1,y,z) always belongs to the
   * neighborhoods we use.
   */
#define _LEFT_NEIGHBOR_TEST {                                                   \
    if ( x > 0 )                                                                \
      if ( *(theBuf - 1) > 0 )                                                  \
        label_neighbors[ nb_neighbors++ ] = cc[ (int)( *(theBuf - 1) ) ].label; \
}

  /*
   * additional informations :
   * - the value of the current label is given to the image buffer
   * - the connect component size is incremented
   * - the validity of the connect component is checked (with respect 
   *   to the current value)
   */
#define _ADDITIONAL_INFORMATIONS {                                     \
    *theBuf = (u16)current_label;                                      \
    cc[ current_label ].lowValuePoints ++;                                      \
    if ( current_value >= _hig_value_ ) cc[ current_label ].highValuePoints ++; \
}

  /*
   * Computation of all points.
   *
   * We will attribute to each point a value (a label). According to this
   * value and to the equivalence table, we know the equivalence classe 
   * of the point (i.e. cc[ value ].label).
   * 
   * The computation is divided into two parts : the first slice
   * and the others slices. This allows to avoid some tests and
   * to spare some time.
   *
   * The principle of the computation is the following :
   * For each point, we
   * - check if t may belong to a connected component
   *   => ( current_value >= _low_value_)
   * - check if tests are needed for the neighbors, i.e.
   *   if the point is on the image border. This point
   *   is different for the first slice and the other ones.
   *   first slice  -> if ( y == 0 ) _INSIDEY_ = 0;
   *   other slices -> if ( (y == 0) || (y == dimy1) ) _INSIDEY_ = 0;
   *   => ( _INSIDEY_ == 1 ) && ( x > 0 ) && ( x < dimx1 )
   * - depending on the above checking, 
   *   we compute the number of neighbors (nb_neighbors) and
   *   extract all the neighbors' values (label_neighbors[]),
   *   either without additionnal tests (_GENERIC_TEST),
   *   or with additionnal tests for the neighbors in the upper slice,
   *   in the upper row (_UPPER_ROW_TESTS) or the left neighbor
   *   (_LEFT_NEIGHBOR_TEST).
   *   The two offsets structures (offsets[] and array_offset[][][])
   *   are necessary because of this choice.
   * - Once we have the number of neighbors and their values, we can
   *   compute the point's label (_LABEL_COMPUTATION)
   *   - if the point have neighbors, its label will be the minimum
   *     of all labels, and we report the equivalence between this
   *     minimum label and the other labels in the neighborhood inside
   *     the equivalence table.
   *   - if the point does not have any neighbor, we use a new label.
   *     we increment the number of used label (++used_labels) and
   *     we initialize a new label. 
   *     WARNING: an error may occur if such initialization is not possible
   *     (it depends on the type used for the image).
   * - Once the point's label is computed, we report this value in the
   *   image, increment the connected component size and test if 
   *   connected component will be valid (current_value >= _hig_value_)
   */

  /*
   * first slice
   */
  local_connexite = VT_CheckConnectivity( par->type_connexite, (int)1 );
  _INIT_CONNECTIVITY( local_connexite )
  z = 0;
  for ( y = 0; y < dimy; y++ ) {
    _INSIDEY_ = 1;
    if ( y == 0 ) _INSIDEY_ = 0;
    for ( x = 0; x < dimx; x++, theBuf++ ) {
      current_value = *theBuf;
      if ( current_value < _low_value_ ) continue;
      nb_neighbors = 0;
      if ( ( _INSIDEY_ == 1 ) && ( x > 0 ) && ( x < dimx1 ) ) {
        _GENERIC_TEST
      } else {
        _UPPER_ROW_TESTS
        _LEFT_NEIGHBOR_TEST
      }
      _LABEL_COMPUTATION
      _ADDITIONAL_INFORMATIONS
    }
  }

  /*
   * other slices
   */
  local_connexite = VT_CheckConnectivity( par->type_connexite, (int)(theIm->dim.z) );
  _INIT_CONNECTIVITY( local_connexite )  
  for ( z = 1; z < (int)theIm->dim.z; z ++ ) {
    for ( y = 0; y < dimy; y++ ) {
      _INSIDEY_ = 1;
      if ( (y == 0) || (y == dimy1) ) _INSIDEY_ = 0;
      for ( x = 0; x < dimx; x++, theBuf++ ) {
        current_value = *theBuf;
        if ( current_value < _low_value_ ) continue;
        nb_neighbors = 0;
        if ( ( _INSIDEY_ == 1 ) && ( x > 0 ) && ( x < dimx1 ) ) {
          _GENERIC_TEST
        } else {
          /*
           * upper slice tests
           */
          for ( j = -1; j <= 1; j ++ ) {
            if ( (y+j < 0) || (y+j >= dimy) )       continue;
            for ( i = -1; i <= 1; i ++ ) {
              if ( (x+i < 0) || (x+i >= dimx) )     continue;
              if ( array_offset[1+i][1+j][0] == 0 ) continue;
              if ( *(theBuf + array_offset[1+i][1+j][0]) > 0 )
                label_neighbors[ nb_neighbors++ ] = cc[ (int)( *(theBuf + array_offset[1+i][1+j][0]) ) ].label;
            }
          }
          _UPPER_ROW_TESTS
          _LEFT_NEIGHBOR_TEST
        }
        _LABEL_COMPUTATION
        _ADDITIONAL_INFORMATIONS
      }
    }
  }
  
  /*
   * At this point, all the image's points have been processed.
   */

  if ( _VT_DEBUG_ > 0 ) {
    char message[256];
    sprintf( message, "number of used labels: %5d", used_labels );
    VT_Message( message, proc );
  }

  if ( used_labels == 0 ) return( 1 );

  /*
   * output: only the largest connected component
   */
  if ( par->max_nbcc == 1 ) {
    /*
     * looking for the largest valid connected component
     */
    cc[ 0 ].label = 0;
    cc[ 0 ].lowValuePoints  = 0;
    for ( i = 1; i <= used_labels; i++ ) {  
      if ( cc[ i ].label != i )            continue;
      if ( cc[ i ].highValuePoints < 1 )      continue;
      if ( cc[ i ].lowValuePoints  < par->min_size ) continue;
      if ( cc[ i ].lowValuePoints  <= cc[ 0 ].lowValuePoints ) continue;
      cc[ 0 ].lowValuePoints  = cc[ i ].lowValuePoints;
      cc[ 0 ].label = i;
    }
    /*
     * re-labeling the whole image
     */
    for ( i = 1; i <= used_labels; i++ ) {
      if ( cc[ i ].label == i ) { 
        if ( cc[ 0 ].label == i )
          cc[ i ].label = _bin_value_;
        else
          cc[ i ].label = 0;
      } else {
        j = cc[ i ].label;
        cc[ i ].label = cc[ j ].label;
      }
    }
    
    if ( cc[ 0 ].label == 0 ) {
      if ( (_VT_VERBOSE_ > 0) || (_VT_DEBUG_ > 0) )
        VT_Message( "No largest valid connected component found" , proc );
    } else {
      if ( (_VT_VERBOSE_ > 0) || (_VT_DEBUG_ > 0) )
        VT_Message( "One largest valid connected component found" , proc );
    }
    cc[ 0 ].label = 0;
    return( 1 );
  }

  /*
   * counting and re-labeling all the valid connected components
   * this is not optimal, but more elegant because it may be used
   * in several further cases.
   *
   * The representative label of a equivalence's classe
   * (i.e. a connected component) is characterized by
   * ( cc[ i ].label == i ).
   * We have to check if this connected component is valid i.e.
   * - if it contains some points with (value >= _hig_value_),
   *   => (cc[ i ].highValuePoints >= 1)
   * - if its size is large enough
   *   => (cc[ i ].lowValuePoints >= par->min_size) )
   * if yes, we increment the number of valid connected components
   * (++valid_labels) and give this value as a new label for the
   * connected component. 
   * if no, we give 0 as a new label and as a new size.
   *
   * If a label is not representative of its equivalence's classe
   * (i.e. cc[ i ].label < i), we give to it the new value
   * of its representative label (which may be 0 if the connected
   * component is not valid). Recall that the representative label
   * of an equivalence's classe is always parsed before the other
   * labels of the same equivalence's classe. Id. for the size.
   *
   * To keep recognizing the valid classes, the representative labels
   * have the flag highValuePoints equal to 1, while the others not.
   */

  k = 0;
  for ( i = 1; i <= used_labels; i++ ) {
    if ( cc[ i ].label == i ) {
      k ++;
      if ( (cc[ i ].highValuePoints >= 1) && (cc[ i ].lowValuePoints >= par->min_size) )
        cc[ i ].label = ++valid_labels;
      else {
        cc[ i ].label = cc[ i ].lowValuePoints = 0;
        cc[ i ].highValuePoints  = 0;
      }
    }
    else {
      j = cc[ i ].label; 
      cc[ i ].label = cc[ j ].label;
      cc[ i ].lowValuePoints = cc[ j ].lowValuePoints;
      cc[ i ].highValuePoints  = 0;
    }
  }

  /*
   * At this point, all valid components have been renumbered,
   * and have the right size.
   */

  if ( (_VT_VERBOSE_ > 0) || (_VT_DEBUG_ > 0) ) {
    char message[256];
    sprintf( message, "number of valid connected components: %5d (out of %d)", valid_labels, k );
    VT_Message( message, proc );
  }

  /*
   * output: all the valid connected components if specified
   */
  if ( (par->max_nbcc <= 0) ) {
    switch ( par->type_output ) {
    case VT_BINAIRE :
      for ( i = 1; i <= used_labels; i++ ) {
        if ( cc[ i ].label > 0  )
          cc[ i ].label = _bin_value_;
      }
      break;
    case VT_SIZE :
      for ( i = 1; i <= used_labels; i++ ) {
        if ( cc[ i ].label > 0  )
          cc[ i ].label = cc[ i ].lowValuePoints;
      }
      break;
    }
    return( 1 );
  }
  
  /*
   * output: the n largest connected component
   *         par->max_nbcc > 1 
   */
  if ( par->max_nbcc > 1 ) {
    _my_cc *validComp = (_my_cc*)NULL;
    validComp = (_my_cc *)VT_Malloc( (unsigned int)((int)(valid_labels + 1) * sizeof(_my_cc)) );
    if ( validComp == (_my_cc*)NULL ) {
       VT_Error( "allocation failed for auxiliary array (to sort connected components)", proc );
       return( -1 );
    }
    /* store all the valid connected components in an array */
    cc[ 0 ].label = cc[ 0 ].lowValuePoints = cc[ 0 ].highValuePoints = 0;
    j = 0;
    for ( i = 1; i <= used_labels; i++ ) {
      if ( cc[ i ].highValuePoints >= 1 ) validComp[ ++j ] = cc[i];
    }
    /* sort them */
    _Sort_CC( validComp, (int)1, valid_labels );

    /* exchange label and index:
       for the max_nbcc first components store the index in highValuePoints
       for the following store 0
       after that, give label the value of highValuePoints
     */
    for ( i = 1; (i <= par->max_nbcc) && (i <= valid_labels); i ++ )
      validComp[ validComp[i].label ].highValuePoints = i;
    /* if par->max_nbcc >= valid_labels, the following does not matter */
    for ( i = par->max_nbcc + 1; i <= valid_labels; i++ )
      validComp[ validComp[i].label ].highValuePoints = 0;
    
    for ( i = 1; i <= valid_labels; i++ )
      validComp[ i ].label = validComp[ i ].highValuePoints;

    /* report the change of value in the complete array */
    for ( i = 1; i <= used_labels; i++ ) {
      if ( cc[i].label == 0 ) continue;
      j =  cc[i].label;
      cc[i].label = validComp[ j ].label;
    }
    
    switch ( par->type_output ) {
    case VT_BINAIRE :
      for ( i = 1; i <= used_labels; i++ ) {
        if ( cc[ i ].label > 0  )
          cc[ i ].label = _bin_value_;
      }
      break;
    case VT_SIZE :
      for ( i = 1; i <= used_labels; i++ ) {
        if ( cc[ i ].label > 0  )
          cc[ i ].label = cc[ i ].lowValuePoints;
      }
      break;
    }
    
    VT_Free( (void**)(&validComp) );
    return( 1 );
  }
  
  return( 0 );
}






















#ifdef _UNUSED_
static _my_cc * VT_RelabelLabelsImage( vt_image *theIm, _my_cc *cc,
                                  int *used_labels,
                                  int xlim, int ylim, int zlim )
{
  char *proc="VT_RelabelLabelsImage";
  u16 *theBuf = (u16 *)(theIm->buf);
  int valid_labels = 0;
  int i, j;
  _my_cc *components = (_my_cc *)NULL;

  
  components = (_my_cc *)VT_Malloc( (unsigned int)((int)_EQUIVALENCE_ARRAY_SIZE_ * sizeof(_my_cc)) );
  if ( components == (_my_cc *)NULL ) {
    VT_Error( "allocation failed for equivalence array", proc );
    return( (_my_cc*)NULL );
  }
  components[0].label = 0;

  for ( i = 1; i <= (*used_labels); i++ ) {
    if ( cc[ i ].label == i ) {
      cc[ i ].label = ++valid_labels; 
      components[ valid_labels ] = cc[i];
    } else {
      j = cc[ i ].label; 
      cc[ i ].label = cc[ j ].label;
    }
  }
  
  if ( (_VT_VERBOSE_ > 0) || (_VT_DEBUG_ > 0) ) {
    char message[256];
    sprintf( message, "number of classes: %5d (out of %d labels) -> pt(%d,%d,%d)", 
             valid_labels, *used_labels, xlim, ylim, zlim );
    VT_Message( message, proc );
  }

  /* renumerote jusqu'au point courant (inclus)
   */
  j = zlim * theIm->dim.y * theIm->dim.x + ylim * theIm->dim.x + xlim;
  for ( i = 0; i <= j; i ++, theBuf++ ) {
    *theBuf = (u16)( cc[ (int)(*theBuf) ].label );
  }
  
  *used_labels = valid_labels;
  return( components );
}
#endif






/* gestion du depassement des labels utilises
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */




static _my_cc * _LabelsOverflowManagement( vt_image *theIm, _my_cc *cc,
                                           int *used_labels,
                                           int x, int y, int z,
                                           int connexite,
                                           int minCcSize )
{
  char *proc="_LabelsOverflowManagement";
  int local_connexite = VT_CheckConnectivity( connexite, z+1 );
  int xlast = x;
  int ylast = y;
  int zlast = z;
  int xnum, ynum, znum;

  u16 *theBuf = (u16 *)(theIm->buf);
  int valid_labels = 0;
  int valid_comps = 0;
  int binary_label = -1;
  int i, j;
  _my_cc *components = (_my_cc *)NULL;


  /* determination du dernier point traite
   */
  if ( xlast < 0 || ylast < 0 || zlast < 0 ) {
    VT_Error( "bad (negative) last point coordinate(s)", proc );
    return( (_my_cc*)NULL );
  }
  
  if ( xlast > 0 ) {
    xlast --;
  }
  else if ( ylast > 0 ) {
    xlast = theIm->dim.x - 1;
    ylast --;
  } 
  else if ( zlast > 0 ) {
    xlast = theIm->dim.x - 1;
    ylast = theIm->dim.y - 1;
    zlast --;
  } else {
    VT_Error( "all last point coordinates are zeros", proc );
    return( (_my_cc*)NULL );
  }

  /* pour certaines composantes (classes d'equivalence)
     on sait deja si il faut les garder ou non,
     ce sont celles qui n'ont plus de voisins
     dans les points qui restent a parcourir.
   *
     Determination du dernier point pouvant
     appartenir a l'une de ces composantes
   *
   * On ne regarde que les cas non-degeneres,
   * c-a-d ou le dernier point traite n'appartient pas a un bord.
   */
  znum = ynum = xnum = -1;


  switch ( local_connexite ) {
  case N04 :
    if ( ylast > 0 ) {
      xnum = xlast;
      ynum = ylast - 1;
      znum = zlast;
    }
    break;
  case N08 :
    if ( ylast > 0 && xlast > 0 ) {
      xnum = xlast - 1;
      ynum = ylast - 1;
      znum = zlast;
    }
    break;
  case N06 :
  case N10 :
    if ( zlast > 0 ) {
      xnum = xlast;
      ynum = ylast;
      znum = zlast - 1;
    }
    break;
  case N18 :
    if ( zlast > 0 && ylast > 0 && xlast > 0 ) {
      xnum = xlast - 1;
      ynum = ylast - 1;
      znum = zlast - 1;
    }
    break;
  case N26 :
  default :
    if ( zlast > 0 && ylast > 0 && xlast > 0 ) {
      xnum = xlast - 1;
      ynum = ylast - 1;
      znum = zlast - 1;
    }
  }


  /* on regarde les composantes si on peur les eliminer
     1. si on a trouve un point jusqu'auquel on peut
        'eliminer' les composantes
        -> on les marque toutes pour l'elimination
        -> on rattrape celles qui peuvent se propager
           (de M_num (non inclus) a M_last (inclus))
     2. sinon on les marque toutes pour les garder.
   */
  if ( xnum >= 0 && ynum >= 0 && znum >= 0 ) {
    for ( i = 1; i <= (*used_labels); i++ )
      cc[i].inside = 1;
    j = zlast * theIm->dim.y * theIm->dim.x + ylast * theIm->dim.x + xlast;
    i = znum  * theIm->dim.y * theIm->dim.x + ynum  * theIm->dim.x + xnum;
    for ( i +=1; i <= j; i++ ) {
      if ( theBuf[i] > 0 ) {
        cc[ cc[ (int)theBuf[i] ].label ].inside = 0;
      }
    }
    
  } else {
    for ( i = 1; i <= (*used_labels); i++ )
      cc[i].inside = 0;
  }
  
  /* allocation d'un nouveau tableau d'equivalence
   */
  components = (_my_cc *)VT_Malloc( (unsigned int)((int)_EQUIVALENCE_ARRAY_SIZE_ * sizeof(_my_cc)) );
  if ( components == (_my_cc *)NULL ) {
    VT_Error( "allocation failed for equivalence array", proc );
    return( (_my_cc*)NULL );
  }
  components[0].label = 0;


  /* on regarde les labels et on cherche
     les representants des classes d'equivalences (labels[i] = i)
     1. si la composante peut encore grandir 
        (inside = 0) => on la garde
     2. sinon, on teste sa validite
  *
     si l'output est binaire, on garde la valeur de 1 
     pour toutes les composantes valides
  */

  for ( i = 1; i <= (*used_labels); i++ ) {
    /* 
     * c'est le label representant la classe d'equivalence
     */
    if ( cc[ i ].label == i ) {
      /*
       * la composante peut encore grandir => on la garde telle quelle
       */
      if ( cc[ i ].inside == 0 ) {
        cc[ i ].label = ++valid_labels;
        components[ valid_labels ] = cc[i];
      }
      /*
       * on a deja tous les points;
       */
      else {
        if ( (cc[ i ].highValuePoints >= 1)
             && (cc[ i ].lowValuePoints >= minCcSize) ) {
          valid_comps ++;
          /*
           * mode binaire, on recupere le numero de la premiere composante
           * complete
           */
          if ( _binary_output_ == 1 ) {
            if ( valid_comps == 1 ) {
              cc[ i ].label = binary_label = ++valid_labels;
              components[ valid_labels ] = cc[i];
            }
            cc[ i ].label = binary_label;
          }
          /*
           * on garde la composante telle quelle
           */
          else {
            cc[ i ].label = ++valid_labels;
            components[ valid_labels ] = cc[i];
          }
        }
        /*
         * on peut ne plus considerer la composante
         */
        else {
          cc[ i ].label = 0;
        }
      }
    } 
    /* 
     * ce n'est pas le label representant la classe d'equivalence
     */
    else {
      j = cc[ i ].label; 
      cc[ i ].label = cc[ j ].label;
    }
  }
  
  if ( (_VT_VERBOSE_ > 0) || (_VT_DEBUG_ > 0) ) {
    char message[256];
    sprintf( message, "previous number of used labels: %d\n\
\t new number of valid labels: %d\n\
\t number of already valid components: %d",
             *used_labels, valid_labels, valid_comps );
    VT_Message( message, proc );
    if ( _VT_DEBUG_ > 0 ) {
      sprintf( message, "last point for valid component: %3d %3d %3d\n\
\t last processed point:           %3d %3d %3d\n\
\t current point:                  %3d %3d %3d", 
               xnum, ynum, znum, xlast, ylast, zlast, x, y, z );
      VT_Message( message, proc );
      if ( _binary_output_ == 1 ) {
        sprintf( message, "mode binary: label for valid components is %d", binary_label );
        VT_Message( message, proc );
      }
    }
  }
  
  /* renumerote jusqu'au dernier point calcule 
   */
  j = zlast * theIm->dim.y * theIm->dim.x + ylast * theIm->dim.x + xlast;
  for ( i = 0; i <= j; i ++, theBuf++ ) {
    *theBuf = (u16)( cc[ (int)(*theBuf) ].label );
  }
  *used_labels = valid_labels;
  return( components );
}









/* Check the connectivity,
   eventually change it, according to the Z dimension
 */ 
  
int VT_CheckConnectivity( Neighborhood connexite, int dimz )
{
    /*--- default ---*/
    Neighborhood local_connexite = N26;
    
    /*--- copy of parameter, or default initialization ---*/
    switch ( connexite ) {
    case N18 :
    case N06 :
    case N10 :
    case N08 :
    case N04 :
    case N26 :
        local_connexite = connexite;
        break;
    default :
        local_connexite = N26;
    }

    /*--- special case: 2D image ---*/
    if ( dimz == 1 ) {
        switch ( local_connexite ) {
        case N10 :
        case N18 :
        case N26 :
        case N08 :
            local_connexite = N08;
            break;
        case N06 :
        case N04 :
            local_connexite = N04;
        }
    }
    
    return( local_connexite );
}

















/* Sorting an array of _my_cc structures with respect to size
 */
static void _Sort_CC( _my_cc *tab, int left, int right )
{
  int i,last;
  _my_cc tmp;

  if ( left >= right ) return;

  tmp = tab[left];   tab[left] = tab[(left+right)/2];   tab[(left+right)/2] = tmp;
  
  last = left;
  for ( i = left+1; i <= right; i++ )       
    if ( tab[i].lowValuePoints > tab[left].lowValuePoints ) {
      tmp = tab[++last];   tab[last] = tab[i];   tab[i] = tmp;
    }

  tmp = tab[left];   tab[left] = tab[last];   tab[last] = tmp;
  
  _Sort_CC( tab, left, last-1 );
  _Sort_CC( tab, last+1, right );
}














/* Initialization of the structure parameters.
 */

void VT_Connexe( vt_connexe *par /* parameters */ )
{
    par->min_size = 1;
    par->max_nbcc = -1;
    par->type_connexite = N26;
    par->type_output = VT_GREY;
    par->dim = VT_3D;
    par->verbose = 0;
}







static void _SetOutputToBinary()
{
  _binary_output_ = 1;
}

static void _UnsetOutputToBinary()
{
  _binary_output_ = 0;
}
