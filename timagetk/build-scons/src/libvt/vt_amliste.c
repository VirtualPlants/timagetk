
#include <math.h>

#include <vt_amliste.h>

#define VT_F2I( F ) ( (F) >= 0.0 ? ((int)((F)+0.5)) : ((int)((F)-0.5)) )

/* Ces procedures creent des listes de points.
    
   Ce sont les points qui sont a VT_DELETABLE
   dans l'image d'entree.

   La seconde procedure conserve en plus la valeur de ces
   points, et les trie par ordre croissant.
   */

typedef struct vt_pt_amincir_list {
  int n;
  int s;
  vt_pt_amincir **l;
} vt_pt_amincir_list;

typedef struct vt_vpt_amincir_list {
  int n;
  int s;
  vt_vpt_amincir **l;
} vt_vpt_amincir_list;

/*------- Definition des fonctions statiques ----------*/

static void _VT_FreePtList( vt_pt_amincir_list *l );
static int  _VT_AddPtList( vt_pt_amincir_list *l, vt_pt_amincir *p );

static void _VT_FreeVPtList( vt_vpt_amincir_list *l );
static int  _VT_AddVPtList( vt_vpt_amincir_list *l, vt_vpt_amincir *p );
static void _VT_SortVPtList( vt_vpt_amincir* list, int right, int left );
static int  _VT_CompareVPts( vt_vpt_amincir* pt1, vt_vpt_amincir* pt2 );





vt_pt_amincir *_VT_ThinPtList( vt_image *image, int dim, int *nb )
{
  size_t x, y, z;
  int v;
  unsigned char *b;
  long int n;
  vt_pt_amincir_list list;
  vt_pt_amincir *lp, p;

  *nb = 0;
  if ( VT_Test1Image( image, "_VT_ThinPtList" ) == -1 )
                return( (vt_pt_amincir*)NULL );
  if ( image->type != UCHAR ) {
    VT_Error( "incorrect image type", "_VT_ThinPtList" );
    return( (vt_pt_amincir*)NULL );
  }

  /*--- initialisation de la liste ---*/
  list.n = list.s = 0;
  list.l = (vt_pt_amincir**)NULL;
  n = image->dim.x * image->dim.y * image->dim.z;
  list.s = (int)sqrt( (double)n );
  if ( list.s * list.s < n ) list.s ++;
  if ( list.s <= 0 ) {
    VT_Error( "unable to compute list of points", "_VT_ThinPtList" );
    return( (vt_pt_amincir*)NULL );
  }

  /*--- transformation de l'image ---*/
  b = (unsigned char*)image->buf;
  for ( z = 0; z < image->dim.z; z ++ )
  for ( y = 0; y < image->dim.y; y ++ )
  for ( x = 0; x < image->dim.x; x ++ ) {
    v = (int)(*b++);
    if ( v != VT_DELETABLE ) continue;
    p.pt.x = x;   p.pt.y = y;   p.pt.z = z;
    p.status = VT_DELETABLE;
    p.inside = 1;
    if ( (x==0) || (y==0) || (x+1==image->dim.x) || (y+1==image->dim.y) )
      p.inside = 0;
    if ( ( dim == VT_3D ) && ( (z==0) || (z+1==image->dim.z) ) )
      p.inside = 0;

    if ( _VT_AddPtList( &list, &p ) == -1 ) {
      _VT_FreePtList( &list );
      VT_Error( "unable to compute list of points", "_VT_ThinPtList" );
      return( (vt_pt_amincir*)NULL );
    }
  }

  /*--- copie de la liste ---*/
  if ( list.n <= 0 ) {
    VT_Error( "unable to compute list of points", "_VT_ThinPtList" );
    return( (vt_pt_amincir*)NULL );
  }
  lp = (vt_pt_amincir*)VT_Malloc( (unsigned int)(list.n * sizeof(vt_pt_amincir)) );
  if ( lp == (vt_pt_amincir*)NULL ) {
    _VT_FreePtList( &list );
    VT_Error( "allocation failed", "_VT_ThinPtList" );
    return( (vt_pt_amincir*)NULL );
  }
  *nb = list.n;
  for (v=0; v<list.n; v++) {
    x = v / (list.s);
    y = v % (list.s);
    lp[v] = list.l[x][y];
  }
  _VT_FreePtList( &list );
  return( lp );
}





static void _VT_FreePtList( vt_pt_amincir_list *l )
{
  int i, nb;
  if ( l->n == 0 ) return;
  if ( l->l == (vt_pt_amincir**)NULL ) return;
  nb = l->n / l->s;
  if ( (l->n % l->s) > 0 ) nb ++;
  for (i = 0; i<nb; i++) {
    if ( l->l[i] != (vt_pt_amincir*)NULL )
      VT_Free( (void**)&(l->l[i]) );
  }
  VT_Free( (void**)&(l->l) );
  l->l = (vt_pt_amincir**)NULL;
}





static int _VT_AddPtList( vt_pt_amincir_list *l, vt_pt_amincir *p )
{
  int i, j, k;
  /*--- premier point ---*/
  if ( l->n == 0 ) {
    l->l = (vt_pt_amincir**)VT_Malloc( (unsigned int)(l->s * sizeof(vt_pt_amincir*)) );
    if (l->l == (vt_pt_amincir**)NULL) {
      VT_Error( "unable to allocate principal list", "_VT_AddPtList");
      return( -1 );
    }
    for (k=0; k<l->s; k++) l->l[k] = (vt_pt_amincir*)NULL;
  }
  /*--- coordonnees du point ---*/
  i = (l->n) / (l->s);
  j = (l->n) % (l->s);
  /*--- doit-on allouer une nouvelle liste ? ---*/
  if ( j == 0 )  {
    l->l[i] = (vt_pt_amincir*)VT_Malloc( (unsigned int)(l->s * sizeof(vt_pt_amincir)) );
    if (l->l[i] == (vt_pt_amincir*)NULL) {
      VT_Error( "unable to allocate secondary list", "_VT_AddPtList");
      return( -1 );
    }
  }
  l->l[i][j] = *p;
  l->n ++;
  return( 1 );
}





vt_vpt_amincir *_VT_ThinVPtList( vt_image *image, vt_image *value, int dim, int *nb )
{
  size_t x, y, z;
  int v, val;
  u8 *b;
  int n;
  vt_vpt_amincir_list list;
  vt_vpt_amincir *lp, p;

  *nb = 0;

  if ( VT_Test2Image( image, value, "_VT_ThinVPtList" ) == -1 ) return( (vt_vpt_amincir*)NULL );
  if ( value->type != USHORT ) {
    VT_Error( "incorrect image type", "_VT_ThinVPtList" );
                return( (vt_vpt_amincir*)NULL );
  }

  /*--- initialisation de la liste ---*/
  list.n = list.s = 0;
  list.l = (vt_vpt_amincir**)NULL;
  n = image->dim.x * image->dim.y * image->dim.z;
  list.s = (int)sqrt( (double)n );
  if ( list.s * list.s < n ) list.s ++;
  if ( list.s <= 0 ) {
    VT_Error( "unable to compute list of points", "_VT_ThinVPtList" );
    return( (vt_vpt_amincir*)NULL );
  }

  /*--- transformation de l'image ---*/
  b = (u8*)image->buf;

  switch ( value->type ) {
  case UCHAR :
      {
      u8 *vb;
      vb = (u8*)value->buf;
      for ( z = 0; z < image->dim.z; z ++ )
      for ( y = 0; y < image->dim.y; y ++ )
      for ( x = 0; x < image->dim.x; x ++ ) {

    v = (int)(*b++);   val = (int)(*vb++);
    if ( v != VT_DELETABLE ) continue;

    p.pt.x = x;   p.pt.y = y;   p.pt.z = z;   p.pt.v = val;
    p.status = VT_DELETABLE;
    p.inside = 1;
    if ( (x==0) || (y==0) || (x+1==image->dim.x) || (y+1==image->dim.y) )
        p.inside = 0;
    if ( ( dim == VT_3D ) && ( (z==0) || (z+1==image->dim.z) ) )
        p.inside = 0;

    if ( _VT_AddVPtList( &list, &p ) == -1 ) {
        _VT_FreeVPtList( &list );
        VT_Error( "unable to compute list of points", "_VT_ThinVPtList" );
        return( (vt_vpt_amincir*)NULL );
    }
      }}
      break;
  case USHORT :
      {
      u16 *vb;
      vb = (u16*)value->buf;
      for ( z = 0; z < image->dim.z; z ++ )
      for ( y = 0; y < image->dim.y; y ++ )
      for ( x = 0; x < image->dim.x; x ++ ) {

    v = (int)(*b++);   val = (int)(*vb++);
    if ( v != VT_DELETABLE ) continue;

    p.pt.x = x;   p.pt.y = y;   p.pt.z = z;   p.pt.v = val;
    p.status = VT_DELETABLE;
    p.inside = 1;
    if ( (x==0) || (y==0) || (x+1==image->dim.x) || (y+1==image->dim.y) )
        p.inside = 0;
    if ( ( dim == VT_3D ) && ( (z==0) || (z+1==image->dim.z) ) )
        p.inside = 0;

    if ( _VT_AddVPtList( &list, &p ) == -1 ) {
        _VT_FreeVPtList( &list );
        VT_Error( "unable to compute list of points", "_VT_ThinVPtList" );
        return( (vt_vpt_amincir*)NULL );
    }
      }}
      break;
  default :
      for ( z = 0; z < image->dim.z; z ++ )
      for ( y = 0; y < image->dim.y; y ++ )
      for ( x = 0; x < image->dim.x; x ++ ) {

    v = (int)(*b++);   val = VT_F2I( VT_GetXYZvalue( value, x, y, z ) );
    if ( v != VT_DELETABLE ) continue;

    p.pt.x = x;   p.pt.y = y;   p.pt.z = z;   p.pt.v = val;
    p.status = VT_DELETABLE;
    p.inside = 1;
    if ( (x==0) || (y==0) || (x+1==image->dim.x) || (y+1==image->dim.y) )
        p.inside = 0;
    if ( ( dim == VT_3D ) && ( (z==0) || (z+1==image->dim.z) ) )
        p.inside = 0;

    if ( _VT_AddVPtList( &list, &p ) == -1 ) {
        _VT_FreeVPtList( &list );
        VT_Error( "unable to compute list of points", "_VT_ThinVPtList" );
        return( (vt_vpt_amincir*)NULL );
    }
      }
  }

  /*--- copie de la liste ---*/
  if ( list.n <= 0 ) {
    VT_Error( "unable to compute list of points", "_VT_ThinVPtList" );
    return( (vt_vpt_amincir*)NULL );
  }
  lp = (vt_vpt_amincir*)VT_Malloc( (unsigned int)(list.n * sizeof(vt_vpt_amincir)) );
  if ( lp == (vt_vpt_amincir*)NULL ) {
    _VT_FreeVPtList( &list );
    VT_Error( "allocation failed", "_VT_ThinVPtList" );
    return( (vt_vpt_amincir*)NULL );
  }
  *nb = list.n;
  for (v=0; v<list.n; v++) {
    x = v / (list.s);
    y = v % (list.s);
    lp[v] = list.l[x][y];
  }
  _VT_FreeVPtList( &list );

  /*--- tri de la liste ---*/
  _VT_SortVPtList( lp, (int)0, (*nb) - 1 );

  /*--- a enlever plus tard ---*/
  /*
  fprintf( stderr, "APRES TRI\n");
  for ( v = 0; v < *nb; v++ ) {
      fprintf( stderr, "pt # %6d : V %4d Z %4d Y %4d X %4d\n", v, lp[v].pt.v, lp[v].pt.z, lp[v].pt.y, lp[v].pt.x );
  }
  */
  return( lp );
}





static void _VT_FreeVPtList( vt_vpt_amincir_list *l )
{
  int i, nb;
  if ( l->n == 0 ) return;
  if ( l->l == (vt_vpt_amincir**)NULL ) return;
  nb = l->n / l->s;
  if ( (l->n % l->s) > 0 ) nb ++;
  for (i = 0; i<nb; i++) {
    if ( l->l[i] != (vt_vpt_amincir*)NULL )
      VT_Free( (void**)&(l->l[i]) );
  }
  VT_Free( (void**)&(l->l) );
  l->l = (vt_vpt_amincir**)NULL;
}





static int _VT_AddVPtList( vt_vpt_amincir_list *l, vt_vpt_amincir *p )
{
  int i, j, k;
  /*--- premier point ---*/
  if ( l->n == 0 ) {
    l->l = (vt_vpt_amincir**)VT_Malloc( (unsigned int)(l->s * sizeof(vt_vpt_amincir*)) );
    if (l->l == (vt_vpt_amincir**)NULL) {
      VT_Error( "unable to allocate principal list", "_VT_AddVPtList");
      return( -1 );
    }
    for (k=0; k<l->s; k++) l->l[k] = (vt_vpt_amincir*)NULL;
  }
  /*--- coordonnees du point ---*/
  i = (l->n) / (l->s);
  j = (l->n) % (l->s);
  /*--- doit-on allouer une nouvelle liste ? ---*/
  if ( j == 0 )  {
    l->l[i] = (vt_vpt_amincir*)VT_Malloc( (unsigned int)(l->s * sizeof(vt_vpt_amincir)) );
    if (l->l[i] == (vt_vpt_amincir*)NULL) {
      VT_Error( "unable to allocate secondary list", "_VT_AddVPtList");
      return( -1 );
    }
  }
  l->l[i][j] = *p;
  l->n ++;
  return( 1 );
}






static void _VT_SortVPtList( vt_vpt_amincir* list, int left, int right )
{
    int i, last;
    vt_vpt_amincir aux;

    if ( left >= right ) return;

    /*--- _VT_SwapVPts( &(list[left]), &(list[(left+right)/2]) ); ---*/
    aux = list[left];   list[left] = list[(left+right)/2];   list[(left+right)/2] = aux;

    last = left;
    for ( i = left+1; i <= right; i++ )
  if ( _VT_CompareVPts( &(list[i]), &(list[left]) ) < 0 ) {
      /*--- _VT_SwapVPts( &(list[++last]), &(list[i]) ); ---*/
      last ++;
      aux = list[last];   list[last] = list[i];   list[i] = aux;
  }
    /*--- _VT_SwapVPts( &(list[left]), &(list[last]) ); ---*/
    aux = list[left];   list[left] = list[last];   list[last] = aux;

    _VT_SortVPtList( list, left, last-1 );
    _VT_SortVPtList( list, last+1, right );
}





static int _VT_CompareVPts( vt_vpt_amincir* pt1, vt_vpt_amincir* pt2 )
{
    if ( pt1->pt.v < pt2->pt.v ) return( (int)-1 );
    if ( pt1->pt.v > pt2->pt.v ) return( (int)1 );

    if ( pt1->pt.z < pt2->pt.z ) return( (int)-1 );
    if ( pt1->pt.z > pt2->pt.z ) return( (int)1 );

    if ( pt1->pt.y < pt2->pt.y ) return( (int)-1 );
    if ( pt1->pt.y > pt2->pt.y ) return( (int)1 );

    if ( pt1->pt.x < pt2->pt.x ) return( (int)-1 );
    if ( pt1->pt.x > pt2->pt.x ) return( (int)1 );    

    return( 0 );
}
