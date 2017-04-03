/****************************************************
 * watershed.c -
 *
 * $Id: watershed.c,v 1.5 2008/02/26 17:42:51 greg Exp $
 *
 * Copyright (c) INRIA 2001
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Wed May  9 13:29:19 MEST 2001
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */


#include <chunks.h>
#include <watershed.h>



/*
 * static global variables
 * verbose,
 * management of ambiguous cases
 * memory management
 * 
 */



static int _verbose_ = 1;


/* to change some of the global variables
 */

void setVerboseInWatershed( int v )
{
  _verbose_ = v;
}

void incrementVerboseInWatershed(  )
{
  _verbose_ ++;
}

void decrementVerboseInWatershed(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}





static enumWatershedLabelChoice _choice_ = _FIRST_ENCOUNTERED_NEIGHBOR_;
/* static enumWatershedLabelChoice _choice_ = _MOST_REPRESENTED_; */

void watershed_setlabelchoice( enumWatershedLabelChoice choice )
{
   _choice_ = choice;
}



static int _nPointsToBeAllocated_ = 100;
static int _nPointsToBeAllocated_was_set_ = 0;

void watershed_setNumberOfPointsForAllocation( int n )
{
  if ( n > 0 ) {
    _nPointsToBeAllocated_ = n;
    _nPointsToBeAllocated_was_set_ = 1;
  }
}

int watershed_getNumberOfPointsForAllocation( )
{
  return( _nPointsToBeAllocated_ );
}

void watershed_automatedSettingNumberOfPointsForAllocation( int *theDim, int glevels )
{
  char *proc = "AutomatedSettingNumberOfPointsForAllocation";
  int s;
  if ( _nPointsToBeAllocated_was_set_ == 0 ) {
    /* rule of thumb */
    s = (int)( (double)theDim[0]*theDim[1]*theDim[2] / (glevels*50) );
    if ( s > _nPointsToBeAllocated_ ) {
      if ( _verbose_  ) {
        fprintf( stderr, "%s: bunch of allocated points, change %d for %d\n",
                 proc, _nPointsToBeAllocated_, s );
      }
      _nPointsToBeAllocated_ = s;
    }
    
  }
}



static int _max_iterations_ = -1;

void watershed_setMaxNumberOfIterations( int n )
{
  _max_iterations_ = n;
}







/*************************************************************
 *
 * static structures and functions: list management
 *
 ************************************************************/



typedef struct {
  int x;
  int y;
  int z;
  int i; /* x+y*dimx+z*dimx*dimy */
  int l; /* label du voisin qui l'a fait mettre dans la liste */
} typePoint;

typedef struct {
  int nMaxPoints;
  int nPoints;
  int nAllocatedPoints;
  typePoint *pt;
} typePointList;



static void init_PointList( typePointList *l );
static void free_PointList( typePointList *l );



typedef struct {
  int x;
  int y;
  int z;
  int i; /* x+y*dimx+z*dimx*dimy */
  int l; /* label du voisin qui l'a fait mettre dans la liste */
  int labels[6];
  int nlabels;
  int labeltobeset;
} typeExtendedPoint;

typedef struct {
  int nMaxPoints;
  int nPoints;
  int nAllocatedPoints;
  typeExtendedPoint *pt;
} typeExtendedPointList;



static void init_ExtendedPointList( typeExtendedPointList *l );
static void free_ExtendedPointList( typeExtendedPointList *l );






static int addPointToList( typePointList *l, 
                           int x, int y, int z, int i, int label );
static int copyPointListToExtendedPointList( typeExtendedPointList *el, 
                                             typePointList *l );






/*************************************************************
 *
 * static structures and functions: tools
 *
 ************************************************************/



static int find_maximal_value( void *theGradient, 
                               bufferType theGradientType, int *theDim );

static int count_gradient_values( typePointList *thePointList, 
                                  int maxGradientValue, u16 *theLabels,
                                  void *theGradient,
                                  bufferType theGradientType, int *theDim );

static int initialize_label_array( u16 *theLabels, void *theLabelsInput, 
                                   bufferType theLabelsType, int *theDim,
                                   int *maxLabel, int *nLabel  );

static int add_label_neighbors_to_lists( typePointList *l, u16 *theLabels, 
                                   int _ISINQUEUE_,
                                   void *theGradient,
                                   bufferType theGradientType, int *theDim );

static int add_neighbors_to_lists( typeExtendedPointList *theExtPointList,
                                   typePointList *l, u16 *theLabels,
                                   int _ISINQUEUE_,
                                   void *theGradient,
                                   bufferType theGradientType, int *theDim );





/*************************************************************
 *
 * some useful functions
 *
 *************************************************************/

static void _sort_array( int *tab, int N )
{
  int i, j, k, imin;
  for ( i=0; i<N-1; i++ ) {
    imin = i;
    for ( j=i+1; j<N; j++ ) {
      if ( tab[j] < tab[imin] ) imin = j; 
    }
    k         = tab[i];
    tab[i]    = tab[imin];
    tab[imin] = k;
  }  
}

static int _main_representative( int *tab, int N )
{
  int i, l, r=0, nr=0;

  /* Remark: the array has to be sorted
  */
 
  for (i=0; i<N;) {
    /* count */
    l = i+1;
    while ( l<N && tab[l]==tab[i] ) l++;
    /* keep if largest */
    if ( i==0 ) {
      nr = l-i;
      r = i;
    } else {
      if ( l-i > nr ) {
        nr = l-i;
        r = i;
      }
    }
    /* skip */
    i = l;
  }
  return( r );
}

static int _is_in_array( int l, int *tab, int N )
{
  int i, r=0;
  for ( i=0;i<N;i++ ) {
    if ( tab[i] == l ) r++;
  }
  return( r );
}



static void _process_worklist( typeExtendedPointList *theExtPointList,
                              u16 *theLabels,  int *theDim, int maxLabel,
                              int first, int last )
{
  int i, j;

  int x, y, z;
  int dimx  = theDim[0];
  int dimy  = theDim[1];
  int dimz  = theDim[2];
  int dimxy = theDim[0] * theDim[1];

  typeExtendedPoint *ept = NULL;

  int n_original_neighbor;
  int mainLabel;

  /* attribution d'etiquettes aux points de la sous-liste de travail
   */
  for ( j = first; j <=last; j++ ) {

    ept = &(theExtPointList->pt[j]);
    x = ept->x;
    y = ept->y;
    z = ept->z;
    i = ept->i;
    
    
    /* on recupere les etiquettes dans le 6-voisinage
     */
    if ( x < dimx-1 && theLabels[i+1] > 0 && theLabels[i+1] <= maxLabel )
      ept->labels[ ept->nlabels ++ ] = theLabels[i+1];
    if ( x > 0      && theLabels[i-1] > 0 && theLabels[i-1] <= maxLabel )
      ept->labels[ ept->nlabels ++ ] = theLabels[i-1];
    if ( y < dimy-1 && theLabels[i+dimx] > 0 && theLabels[i+dimx] <= maxLabel ) 
      ept->labels[ ept->nlabels ++ ] = theLabels[i+dimx];
    if ( y > 0      && theLabels[i-dimx] > 0 && theLabels[i-dimx] <= maxLabel )
      ept->labels[ ept->nlabels ++ ] = theLabels[i-dimx];
    if ( z < dimz-1 && theLabels[i+dimxy] > 0 && theLabels[i+dimxy] <= maxLabel )
      ept->labels[ ept->nlabels ++ ] = theLabels[i+dimxy];
    if ( z > 0      && theLabels[i-dimxy] > 0 && theLabels[i-dimxy] <= maxLabel )
      ept->labels[ ept->nlabels ++ ] = theLabels[i-dimxy];
    
    
    if ( ept->nlabels == 0 ) {
      
      /* pas de voisins etiquetes dans le voisinage ... tres surprenant
         on lui donne neanmoins l'etiquette de celui qui l'a conduit ici
      */
      
      if ( _verbose_ ) {
        fprintf( stderr, "no neighbors for (%d,%d,%d) = %d (max=%d)\n",
                 x, y, z, theLabels[ i ], maxLabel );
      }
      ept->labeltobeset = ept->l;
    }
    
    else if ( ept->nlabels == 1 ) {
      
      /* un seul voisin : cas ideal
       */
      
      if ( ept->labels[ 0 ] != ept->l ) {
        if ( _verbose_ )
          fprintf( stderr, "only one neighbor but conflicting labels for (%d,%d,%d) = %d (max=%d)\n",
                   x, y, z, theLabels[ i ], maxLabel );
      }
      ept->labeltobeset = ept->l;
    }
    
    else {
      
      /* plusieurs voisins etiquetes : cas embetant
       */
      
      if ( (n_original_neighbor = _is_in_array( ept->l, ept->labels, ept->nlabels )) == 0 ) {
        if ( _verbose_ )
          fprintf( stderr, "original neighbor (label=%d) not in neighborhood for (%d,%d,%d) = %d (max=%d)\n",
                   ept->l, x, y, z, theLabels[ i ], maxLabel );
      }
      
      /* on trie (par ordre croissant) les labels des voisins
       */
      _sort_array( ept->labels, ept->nlabels );
      
      /* Attribution de l'etiquette
       */
      if ( ept->labels[0] == ept->labels[ ept->nlabels-1 ] ) {

        /* cas simple : une seule etiquette dans le voisinage
         */
        ept->labeltobeset = ept->labels[ 0 ];
      }
      else {

        /* cas difficile, plusieurs etiquettes dans le voisinage
         */

        switch( _choice_ ) {
        default :
        case _FIRST_ENCOUNTERED_NEIGHBOR_ :
          /* premier 6-voisin rencontre lors de la mise dans la liste
             comportement historique par defaut
             NOTE : j'avais aussi essaye le premier label rencontre dans le
             voisinage (ie labels[0] avant tri), mais j'avais note que cela ne
             marchait pas.
          */
          ept->labeltobeset = ept->l;
          break;

        case _MIN_LABEL_ :
          /* plus petit label
           */
          ept->labeltobeset = ept->labels[0];
          break;

        case _MOST_REPRESENTED_ :
          /* label avec le plus de representants
             On compare avec le nombre de representants du label original,
             et on choisit ce dernier si les nombres de representants sont egaux
          */
          mainLabel =  _main_representative( ept->labels, ept->nlabels );
          if ( _is_in_array( ept->labels[mainLabel], ept->labels, ept->nlabels ) > n_original_neighbor )
            ept->labeltobeset = ept->labels[mainLabel];
          else
            ept->labeltobeset = ept->l;
          break;
        }
      }
    }
  }

}





/************************************************************
 *
 *
 *
 ************************************************************/

int watershed( void *theGradient, bufferType theGradientType,
               void *theLabelsInput, void *theLabelsOutput,
               bufferType theLabelsType,
               int *theDim )
{
  char *proc = "watershed";
  
  int i, j;
  int v = theDim[0]*theDim[1]*theDim[2];
 
  int maxGradientValue = 0;
  typePointList *thePointList = NULL;
 
  typeExtendedPointList theExtPointList;

  u16 *theLabels = NULL;
  int _ISINQUEUE_;
  int maxLabel = 0;
  int nLabel;

  int nPoints;
  int n, iteration;

  typeChunks chunks;
  int nchunks = 100;
  int nchunks_threshold = nchunks * 25;

  

  initChunks( &chunks );
  if ( allocChunks( &chunks, nchunks ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: error when allocating chunks\n", proc );
    return( -1 );
  }



  /* here, I assume the type of gradient image is UCHAR
   */
  watershed_automatedSettingNumberOfPointsForAllocation( theDim, 256 );
  
  if ( _verbose_  >= 2 ) 
    fprintf( stderr, "%s: init: compute gradient maximum value\n", proc );
  

  
  /* maximum value of the "gradient" image ?
  */  
  maxGradientValue = find_maximal_value( theGradient, theGradientType, theDim );
  if ( maxGradientValue <= 0 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: NULL gradient image?\n", proc );
    }
    freeChunks( &chunks );
    return( -1 );
  }



  if ( _verbose_ >= 2 ) 
    fprintf( stderr, "%s: init: list structures (1); max Grad = %d\n", proc, maxGradientValue );



  /* structure allocation and initialization
  */
  thePointList = (typePointList *)malloc( (maxGradientValue+1)*sizeof(typePointList) );
  if ( thePointList == (typePointList*)NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate auxiliary array\n", proc );
    freeChunks( &chunks );
    return( -1 );
  }
  
  for ( i=0; i<=maxGradientValue; i++ )
    init_PointList( &thePointList[i] );

  
  init_ExtendedPointList( &theExtPointList );







  if ( _verbose_ >= 2 ) 
    fprintf( stderr, "%s: init: auxiliary label image\n", proc );



  /* labels preparation
   */
  theLabels = (u16*)malloc( v * sizeof(u16) );
  if ( theLabels == NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to auxiliary label image\n", proc );
    free( thePointList );
    freeChunks( &chunks );
    return( -1 );
  }



   /* calcul du maximum des labels
     on remplit l'image de travail des labels
   */
  
  
  if ( initialize_label_array( theLabels, theLabelsInput, theLabelsType, theDim, &maxLabel, &nLabel ) < 0 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to fill auxiliary label image\n", proc );
    free( theLabels );
    free( thePointList );
    freeChunks( &chunks );
    return( -1 );
  }

  if ( maxLabel <= 1 || nLabel == v ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: weird label image", proc ); 
      if ( maxLabel <= 1 ) fprintf( stderr, ", one or no label?" );
      if ( nLabel == v ) fprintf( stderr, ", as many labels as voxels?" );
      fprintf( stderr, "\n");
    }
    free( theLabels );
    free( thePointList );
    freeChunks( &chunks );
    return( -1 );
  }

  if ( maxLabel == 65535 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: too many labels\n", proc );
    free( theLabels );
    free( thePointList );
    freeChunks( &chunks );
    return( -1 );
  }





  if ( _verbose_ >= 2 ) 
    fprintf( stderr, "%s: init: list structures (2)\n", proc );


  /* count the number of points per gradient value
  */
  theExtPointList.nMaxPoints = count_gradient_values( thePointList, maxGradientValue,
                                                      theLabels, theGradient, theGradientType, theDim );

  if ( theExtPointList.nMaxPoints <= 0 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to count gradient values\n", proc );
    free( theLabels );
    free( thePointList );
    freeChunks( &chunks );
    return( -1 );
  }




  
  if ( _verbose_ >= 2 ) 
    fprintf( stderr, "%s: init: queue\n", proc );



  /* labels supplementaires pour le calcul
   */
  _ISINQUEUE_ = maxLabel+1;

  /* queue initialisation
   */
  if ( add_label_neighbors_to_lists( thePointList, theLabels, _ISINQUEUE_,
          theGradient, theGradientType, theDim ) <= 0 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to initialize point lists\n", proc );
    free( theLabels );
    for ( i=0; i<=maxGradientValue; i++ )
      free_PointList( &thePointList[i] );
    free( thePointList );
    freeChunks( &chunks );
    return( -1 );
  }




  /* Fin de l'initialisation
     * le buffer des labels contient
       - les labels des graines
       - des points a _ISINQUEUE_ qui n'ont qu'un seul 6-voisin 
         qui est une graine
     * ces points sont ranges dans la liste thePts
   */


  
  /* on compte les points dans la liste
   */
  for ( nPoints=0,i=0; i<=maxGradientValue; i++ ) 
    nPoints += thePointList[i].nPoints;



  if ( _verbose_ >= 3 ) {
    fprintf( stderr, "\n--------------------------------------------------\n" );
    fprintf( stderr, "%s: end of initialization\n", proc );
    fprintf( stderr, "\t #points = %d\n", nPoints );
    for ( i=0; i<=maxGradientValue; i++ ) {
      if ( thePointList[i].nPoints != 0 ) {
        fprintf( stderr, "\t #points( grad=%5d ) = %5d\n", i, thePointList[i].nPoints );
      }
    }
    fprintf( stderr, "--------------------------------------------------\n\n" );
  }



  /* traitement de la liste : on itere tant qu'il y a des points
   */
  n = v - nLabel;
  iteration = 0;


  while ( nPoints > 0  && ((iteration < _max_iterations_) || (_max_iterations_ < 0)) ) {

    iteration ++;

    /* on cherche la plus petite valeur de gradient pour laquelle il y
           a des points dans la liste
    */
    for (i=0; i<=maxGradientValue && thePointList[i].nPoints == 0; i++ ) 
      ;

    if ( _verbose_ >= 2 ) {
      fprintf( stderr, "#i=%5d (alt=%5d) points proc./list./rema. %12d/%12d/%12d   \r", 
               iteration, i, thePointList[i].nPoints, nPoints, n );
    }
    n -= thePointList[i].nPoints;



    /* on recopie les points dans la sous-liste de travail
    */
    if ( copyPointListToExtendedPointList( &theExtPointList, 
                                           &thePointList[i] ) <= 0 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to copy point list\n", proc );
      free( theLabels );
      for ( i=0; i<=maxGradientValue; i++ )
        free_PointList( &thePointList[i] );
      free( thePointList );
      free_ExtendedPointList( &theExtPointList );
      freeChunks( &chunks );
      return( -1 );
    }
    
    /* RQ: on peut, ici, decrementer thePointList[i].nMaxPoints de thePointList[i].nPoints
       thePointList[i].nMaxPoints -= thePointList[i].nPoints;
     */
    thePointList[i].nMaxPoints -= thePointList[i].nPoints;
    thePointList[i].nPoints = 0;

    /* better memory management
       desallocation of empty lists (all lists with a gradient value less than i)
       that will remain empty (all points have already been processed)
     */
    for ( j = 0; j <= i; j++ ) {
      if ( thePointList[j].nMaxPoints == 0 && thePointList[j].nAllocatedPoints > 0 ) {
        if ( _verbose_ >= 3 ) {
          fprintf( stderr, "\n%s: free list #%d\n", proc, j );
        }
        free_PointList( &thePointList[j] );
        continue;
      }
      /* here we could do something better, 
         ie some re-allocation if thePointList[j].nMaxPoints <<  thePointList[j].nAllocatedPoints
      */
    }




    /* attribution d'etiquettes aux points de la sous-liste de travail
    */
    if ( theExtPointList.nPoints <= nchunks_threshold ) {
      _process_worklist( &theExtPointList, theLabels, theDim, maxLabel, 
                         0, theExtPointList.nPoints-1 );
    }
    else {
#ifdef _OPENMP
      if ( buildEqualChunks( &(chunks.data[0]), 0, theExtPointList.nPoints-1, nchunks ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when calculating chunks\n", proc );
        free( theLabels );
        for ( i=0; i<=maxGradientValue; i++ )
          free_PointList( &thePointList[i] );
        free( thePointList );
        free_ExtendedPointList( &theExtPointList );
        freeChunks( &chunks );
        return( -1 );
      }

#pragma omp parallel for schedule(dynamic)
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
        _process_worklist( &theExtPointList, theLabels, theDim, maxLabel,
                           (int)(chunks.data[i].first), (int)(chunks.data[i].last)  );
      }
      
#else
      _process_worklist( &theExtPointList, theLabels, theDim, maxLabel, 
                         0, theExtPointList.nPoints-1 );
#endif      
    }
 
    
    /* on ecrit les etiquettes dans l'image de labels
       on le fait apres avoir attribue les etiquettes pour que 
       cela n'interfere pas (sinon l'ordre de traitement des points 
       a une importance) 
    */
#ifdef _OPENMP
#pragma omp parallel for 
#endif 
    for ( j = 0; j < theExtPointList.nPoints; j++ ) {
      theLabels[ theExtPointList.pt[j].i ] = theExtPointList.pt[j].labeltobeset;
    }



    /* on recupere les voisins des points traites
    */

    if ( add_neighbors_to_lists( &theExtPointList, thePointList, theLabels, 
                                 _ISINQUEUE_,
                                 theGradient, theGradientType, theDim ) <= 0 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to add neighbors to point lists\n", proc );
      free( theLabels );
      for ( i=0; i<=maxGradientValue; i++ )
        free_PointList( &thePointList[i] );
      free( thePointList );
      free_ExtendedPointList( &theExtPointList );
      freeChunks( &chunks );
      return( -1 );
    }



    /* on compte les points dans la liste
     */
    for ( nPoints=0,i=0; i<=maxGradientValue; i++ ) 
      nPoints += thePointList[i].nPoints;

  }

  /* fin du traitement de la liste 
   */
  if ( _verbose_ >= 2 ) {
    fprintf( stderr, "\n" );
  }


  /* no more interesting,
     lists have already been desallocated when possible
     (better memory management)
  */
  if ( 0 && _verbose_ >= 3 ) {
    for ( i=0, j=0; i<=maxGradientValue; i++ ) {
      if ( _verbose_ >= 4 )
        fprintf( stderr, "#%3d : max=%5d pts=%5d all=%5d\n",
                 i, thePointList[i].nMaxPoints, thePointList[i].nPoints,
                 thePointList[i].nAllocatedPoints );
      j += thePointList[i].nAllocatedPoints;
    }
    fprintf( stderr, "%s: %d allocated points (instead of %d)\n", proc, j, v );
    fprintf( stderr, "\t %d allocated extended points (instead of %d)\n", 
            theExtPointList.nAllocatedPoints, theExtPointList.nMaxPoints  );
  }
  
  for ( i=0; i<=maxGradientValue; i++ )
    free_PointList( &thePointList[i] );
  free( thePointList );
  free_ExtendedPointList( &theExtPointList );


  switch( theLabelsType ) {
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such label type not handled yet\n", proc );
    }
    free( theLabels );
    freeChunks( &chunks );
    return( -1 );
  case UCHAR :
    {
      u8 *buf = theLabelsOutput;
      for ( i=0; i<v; i++ )
        buf[i] = theLabels[i];
    }
    break;
  case USHORT :
    {
      u16 *buf = theLabelsOutput;
      for ( i=0; i<v; i++ )
        buf[i] = theLabels[i];
    }
  case SSHORT :
    {
      s16 *buf = theLabelsOutput;
      for ( i=0; i<v; i++ )
        buf[i] = theLabels[i];
    }
    break;
  }

  free( theLabels );
  freeChunks( &chunks );
    
  return( 1 );

}










/*************************************************************
 *
 * static structures and functions: list management
 *
 ************************************************************/



static void init_PointList( typePointList *l )
{
  l->nMaxPoints = 0;
  l->nPoints = 0;
  l->nAllocatedPoints = 0;
  l->pt = NULL;
}

static void free_PointList( typePointList *l )
{
  if ( l->pt != NULL ) free ( l-> pt );
  init_PointList( l );
}



static void init_ExtendedPointList( typeExtendedPointList *l )
{
  l->nMaxPoints = 0;
  l->nPoints = 0;
  l->nAllocatedPoints = 0;
  l->pt = NULL;
}

static void free_ExtendedPointList( typeExtendedPointList *l )
{
  if ( l->pt != NULL ) free ( l-> pt );
  init_ExtendedPointList( l );
}






static int addPointToList( typePointList *l, 
                           int x, int y, int z, int i, int label)
{
  char *proc = "addPointToList";
  int n = l->nPoints;
  int newn;
  typePoint *pt = NULL;

  if ( n == l->nAllocatedPoints ) {

    newn = l->nAllocatedPoints + _nPointsToBeAllocated_;
    if ( newn > l->nMaxPoints ) newn = l->nMaxPoints;
    if ( newn <= 0 || newn <= l->nAllocatedPoints ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: weird size for point list\n", proc );
        fprintf( stderr, "\t failed to add (%d,%d,%d) in list", x, y, z );
      }
      return( -1 );
    }

    pt = (typePoint *)malloc( newn * sizeof( typePoint ) );
    if ( pt == NULL ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: can not reallocate point list\n", proc );
        fprintf( stderr, "\t failed to add (%d,%d,%d) in list", x, y, z );
      }
      return( -1 );
    }

    if ( l->pt != NULL ) {
      (void)memcpy( (void*)pt, (void*)l->pt,  l->nAllocatedPoints * sizeof( typePoint ) );
      free( l->pt );
    }
    
    l->pt = pt;
    l->nAllocatedPoints = newn;
    
  }

  l->pt[ n ].x = x;
  l->pt[ n ].y = y;
  l->pt[ n ].z = z;
  l->pt[ n ].i = i;
  l->pt[ n ].l = label;

  l->nPoints ++;

  return( l->nPoints );
}





static int copyPointListToExtendedPointList( typeExtendedPointList *el, 
                                             typePointList *l )
{
  char *proc = "copyPointListToExtendedPointList";
  int i;
  int newn;
  typeExtendedPoint *pt;
  
  if ( el->nAllocatedPoints < l->nPoints ) {
    newn = el->nAllocatedPoints;
    while ( newn < l->nPoints )
      newn  += _nPointsToBeAllocated_;
    if ( newn > el->nMaxPoints ) newn = el->nMaxPoints;
    if ( newn < l->nPoints ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: weird size for point list\n", proc );
        if ( _verbose_ >= 2 ) {
          fprintf( stderr, "\t allocated = %d increment = %d max = %d\n",
                   el->nAllocatedPoints, _nPointsToBeAllocated_, el->nMaxPoints );
          fprintf( stderr, "\t #points in list = %d\n", l->nPoints );
        }
      }
      return( -1 );
    }
    
    pt = (typeExtendedPoint *)malloc( newn * sizeof( typeExtendedPoint ) );
    if ( pt == NULL ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: can not reallocate point list\n", proc );
      return( -1 );
    }
    
    if ( el->pt != NULL ) {
      free( el->pt );
    }
    
    el->pt = pt;
    el->nAllocatedPoints = newn;
  }
  
#ifdef _OPENMP
#pragma omp parallel for 
#endif 
  for (i=0; i<l->nPoints; i++ ) {
    el->pt[i].x = l->pt[i].x;
    el->pt[i].y = l->pt[i].y;
    el->pt[i].z = l->pt[i].z;
    el->pt[i].i = l->pt[i].i;
    el->pt[i].l = l->pt[i].l;
    el->pt[i].nlabels = 0;
    el->pt[i].labeltobeset = -1; 
  }
  el->nPoints = l->nPoints;
  return( 1 );
}





/*************************************************************
 *
 * static structures and functions: tools
 *
 ************************************************************/



/************************************************************
 *
 * maximal value of the "gradient" image
 */

static int find_maximal_value( void *theGradient, 
                               bufferType theGradientType, int *theDim )
{
  char *proc = "find_maximal_value";
  int error_value = -1;
  int i;
  int v = theDim[0]*theDim[1]*theDim[2];
  int maximal_value = 0;

  switch( theGradientType ) {

  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such gradient type not handled yet\n", proc );
    }
    return( error_value );

  case UCHAR :
    {
      u8 *buf = theGradient;
      maximal_value = buf[0];
      for (i=1;i<v;i++)
        if ( maximal_value < buf[i] )  maximal_value = buf[i];
    }
    break;

  case USHORT :
    {
      u16 *buf = theGradient;
      maximal_value = buf[0];
      for (i=1;i<v;i++)
        if ( maximal_value < buf[i] )  maximal_value = buf[i];
    }
    break;

  case SSHORT :
    {
      u16 *buf = theGradient;
      maximal_value = buf[0];
      for (i=1;i<v;i++)
        if ( maximal_value < buf[i] )  maximal_value = buf[i];
    }
    break;

  }

  return( maximal_value );
}






/************************************************************
 *
 * Count the number of point per gradient value
 * return the maximal number 
 */

static int count_gradient_values( typePointList *thePointList, 
                                  int maxGradientValue, u16 *theLabels, void *theGradient,
                                  bufferType theGradientType, int *theDim )
{
  char *proc = "count_gradient_values";
  int error_value = -1;
  int i;
  int v = theDim[0]*theDim[1]*theDim[2];
  int maximal_value = 0;

  switch( theGradientType ) {

  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such gradient type not handled yet\n", proc );
    }
    return( error_value );

  case UCHAR :
    {
      u8 *buf = theGradient;
      for (i=0;i<v;i++) {
        if ( theLabels[i] == 0 )
          thePointList[  buf[i] ].nMaxPoints ++;
      }
    }
    break;
 
  case USHORT :
    {
      u16 *buf = theGradient;
      for (i=0;i<v;i++) {
        if ( theLabels[i] == 0 )
          thePointList[  buf[i] ].nMaxPoints ++;
      }
    }
    break;

  case SSHORT :
    {
      s16 *buf = theGradient;
      for (i=0;i<v;i++) {
        if ( theLabels[i] == 0 )
          thePointList[  buf[i] ].nMaxPoints ++;
      }
    }
    break;
  }

  maximal_value = 0;
  for ( i=0; i<=maxGradientValue; i++ ) 
    if ( maximal_value < thePointList[ i ].nMaxPoints ) 
      maximal_value = thePointList[ i ].nMaxPoints;

  return( maximal_value );
}






/************************************************************
 *
 * Fill the auxiliary array of labels with input labels
 * return the maximal value
 *
 */

static int initialize_label_array( u16 *theLabels, void *theLabelsInput, 
                                   bufferType theLabelsType, int *theDim,
                                   int *maxLabel, int *nLabel )
{
  char *proc = "initialize_label_array";
  int error_value = -1;
  int i;
  int v = theDim[0]*theDim[1]*theDim[2];
  int n = 0;
  int m = 0;
  int neg = 0;

  switch( theLabelsType ) {

  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such label type not handled yet\n", proc );
    }
    return( error_value );
  
  case UCHAR :
    {
      u8 *buf = theLabelsInput;
      for ( i=0; i<v; i++ ) {
        theLabels[i] = buf[i];
        if ( theLabels[i] == 0 ) continue;
        if ( m < theLabels[i] ) m = theLabels[i];
        n ++;
      }
    }
    break;
  case USHORT :
    {
      u16 *buf = theLabelsInput;
      for ( i=0; i<v; i++ ) {
        theLabels[i] = buf[i];
        if ( theLabels[i] == 0 ) continue;
        if ( m < theLabels[i] ) m = theLabels[i];
        n ++;
      }
    }
    break;
  case SSHORT :
    {
      s16 *buf = theLabelsInput;
      for ( i=0; i<v; i++ ) {
        if ( buf[i] < 0 ) {
          theLabels[i] = 0;
          neg ++;
          continue;
        }
        theLabels[i] = buf[i];
        if ( theLabels[i] == 0 ) continue;
        if ( m < theLabels[i] ) m = theLabels[i];
        n ++;
      }
    }
    if ( neg > 0 ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: negative labels will be ignored\n", proc );
      }
    }
    break;
  }

  *maxLabel = m;
  *nLabel = n;

  return( 1 );
}





/************************************************************
 *
 * Fill the points lists with the neighbors of the 
 * auxiliary label array
 *
 */

static int add_label_neighbors_to_lists( typePointList *l, u16 *theLabels, 
                                         int _ISINQUEUE_,
                                         void *theGradient,
                                         bufferType theGradientType, int *theDim )
{
  char *proc = "add_label_neighbors_to_lists";
  int error_value = -1;
  int i;
  int x, y, z;
  int dimx  = theDim[0];
  int dimy  = theDim[1];
  int dimz  = theDim[2];
  int dimx1 = theDim[0] - 1;
  int dimy1 = theDim[1] - 1;
  int dimz1 = theDim[2] - 1;
  int dimxy = theDim[0] * theDim[1];

#define _ADD_LABEL_NEIGHBORS_TO_LISTS \
      for ( i=0, z=0; z<dimz; z++ ) \
      for ( y=0; y<dimy; y++ ) \
      for ( x=0; x<dimx; x++, i++ ) { \
        if ( theLabels[i] == 0 || theLabels[i] == _ISINQUEUE_ ) continue; \
        if ( x < dimx1 ) { \
          if ( theLabels[i+1] == 0  ) { \
            if ( addPointToList( &(l[buf[i+1]]), x+1, y, z, i+1, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i+1] = _ISINQUEUE_; \
          } \
        } \
        if ( x > 0 ) { \
          if ( theLabels[i-1] == 0  ) { \
            if ( addPointToList( &(l[buf[i-1]]), x-1, y, z, i-1, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i-1] = _ISINQUEUE_; \
          } \
        } \
        if ( y < dimy1 ) { \
          if ( theLabels[i+dimx] == 0  ) { \
            if ( addPointToList( &(l[buf[i+dimx]]), x, y+1, z, i+dimx, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i+dimx] = _ISINQUEUE_; \
          } \
        } \
        if ( y > 0 ) { \
          if ( theLabels[i-dimx] == 0  ) { \
            if ( addPointToList( &(l[buf[i-dimx]]), x, y-1, z, i-dimx, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i-dimx] = _ISINQUEUE_; \
          } \
        } \
        if ( z < dimz1 ) { \
          if ( theLabels[i+dimxy] == 0  ) { \
            if ( addPointToList( &(l[buf[i+dimxy]]), x, y, z+1, i+dimxy, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i+dimxy] = _ISINQUEUE_; \
          } \
        } \
        if ( z > 0 ) { \
          if ( theLabels[i-dimxy] == 0  ) { \
            if ( addPointToList( &(l[buf[i-dimxy]]), x, y, z-1, i-dimxy, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i-dimxy] = _ISINQUEUE_; \
          } \
        } \
      }

  switch( theGradientType ) {

  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such gradient type not handled yet\n", proc );
    }
    return( error_value );



  case UCHAR :
    {
      u8 *buf = theGradient;
      _ADD_LABEL_NEIGHBORS_TO_LISTS
    }
    
    break;
    
  case USHORT :
    {
      u16 *buf = theGradient;
      _ADD_LABEL_NEIGHBORS_TO_LISTS
    }
    break;

  case SSHORT :
    {
      s16 *buf = theGradient;
      _ADD_LABEL_NEIGHBORS_TO_LISTS
    }
    break;
  }
  

  return( 1 );

}






/************************************************************
 *
 * Fill the points lists with the neighbors of the 
 * auxiliary list
 *
 */

static int add_neighbors_to_lists( typeExtendedPointList *theExtPointList,
                                   typePointList *l, u16 *theLabels,
                                   int _ISINQUEUE_,
                                   void *theGradient,
                                   bufferType theGradientType, int *theDim )
{
  char *proc = "add_neighbors_to_lists";
  int error_value = -1;
  int j;
  int x, y, z, i;
  int dimx  = theDim[0];
  int dimx1 = theDim[0] - 1;
  int dimy1 = theDim[1] - 1;
  int dimz1 = theDim[2] - 1;
  int dimxy = theDim[0] * theDim[1];

#define _ADD_NEIGHBORS_TO_LISTS  \
      for ( j = 0; j < theExtPointList->nPoints; j++ ) { \
        x = theExtPointList->pt[j].x; \
        y = theExtPointList->pt[j].y; \
        z = theExtPointList->pt[j].z; \
        i = theExtPointList->pt[j].i; \
        if ( x < dimx1 ) { \
          if ( theLabels[i+1] == 0  ) { \
            if ( addPointToList( &(l[buf[i+1]]), x+1, y, z, i+1, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i+1] = _ISINQUEUE_; \
          } \
        } \
        if ( x > 0 ) { \
          if ( theLabels[i-1] == 0  ) { \
            if ( addPointToList( &(l[buf[i-1]]), x-1, y, z, i-1, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i-1] = _ISINQUEUE_; \
          } \
        } \
        if ( y < dimy1 ) { \
          if ( theLabels[i+dimx] == 0  ) { \
            if ( addPointToList( &(l[buf[i+dimx]]), x, y+1, z, i+dimx, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i+dimx] = _ISINQUEUE_; \
          } \
        } \
        if ( y > 0 ) { \
          if ( theLabels[i-dimx] == 0  ) { \
            if ( addPointToList( &(l[buf[i-dimx]]), x, y-1, z, i-dimx, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i-dimx] = _ISINQUEUE_; \
          } \
        } \
        if ( z < dimz1 ) { \
          if ( theLabels[i+dimxy] == 0  ) { \
            if ( addPointToList( &(l[buf[i+dimxy]]), x, y, z+1, i+dimxy, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i+dimxy] = _ISINQUEUE_; \
          } \
        } \
        if ( z > 0 ) { \
          if ( theLabels[i-dimxy] == 0  ) { \
            if ( addPointToList( &(l[buf[i-dimxy]]), x, y, z-1, i-dimxy, (int)theLabels[i] ) <= 0 ) \
              return( -1 ); \
            theLabels[i-dimxy] = _ISINQUEUE_; \
          } \
        } \
      }

  switch( theGradientType ) {

  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such gradient type not handled yet\n", proc );
    }
    return( error_value );

  case UCHAR :
    {
      u8 *buf = theGradient;
      _ADD_NEIGHBORS_TO_LISTS
    }
    break;
    
  case USHORT :
    {
      u16 *buf = theGradient;
      _ADD_NEIGHBORS_TO_LISTS
    }
    break;

  case SSHORT :
    {
      s16 *buf = theGradient;
      _ADD_NEIGHBORS_TO_LISTS
    }
    break;

  }
  

  return( 1 );

}
