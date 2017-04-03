/*************************************************************************
 * ccparameters.c - extraction de parametres sur des parties numerotees
 *
 * $Id: ccparameters.c,v 1.2 2001/04/03 10:27:21 greg Exp $
 *
 * DESCRIPTION: 
 *
 *
 *
 * AUTHOR:
 * Gregoire Malandain
 *
 * CREATION DATE:
 * Sun Feb 11 10:45:53 MET 2001
 *
 * Copyright Gregoire Malandain, INRIA
 *
 *
 * ADDITIONS, CHANGES:
 *
 *
 */


#include <vt_common.h>
#include <ccparameters.h>




static int _verbose_ = 0;

void _VerboseInCcParameters()
{
  _verbose_ = 1;
}
void _NoVerboseInCcParameters()
{
  _verbose_ = 0;
}





void _print_one_component( FILE *f, int label,
			   typeImageParameter *par1,
			   typeImageParameter *par2 )
{
  int i;
  typeParameter *par = &(par1->thePar[label]);

  fprintf( f, "LABEL=%d VOL=%d N=%d\n", label, par->volume, par->n_components );
  fprintf( f, "   CORNER=[%d %d %d]",
	   par->iptmin[0], par->iptmin[1], par->iptmin[2] );
  fprintf( f, " DIM=[%d %d %d]", 
	   par->iptmax[0] - par->iptmin[0] + 1,
	   par->iptmax[1] - par->iptmin[1] + 1,
	   par->iptmax[2] - par->iptmin[2] + 1 );
  fprintf( f, " PT=[%d %d %d]\n", 
	   par->intersection[0], par->intersection[1], par->intersection[2] );

  for ( i=0; i<par->n_components; i++ ) {
    fprintf( f, "   NEIGHBOR=%d VOL=%d PT=[%d %d %d]\n",
	     par->i_component[i],
	     par2->thePar[ par->i_component[i] ].volume,
	     par2->thePar[ par->i_component[i] ].intersection[0],
	     par2->thePar[ par->i_component[i] ].intersection[1],
	     par2->thePar[ par->i_component[i] ].intersection[2] );
  }
}


void _print_same_components( FILE *f, enumConfiguration cfg,
			    typeImageParameter *par1,
			    typeImageParameter *par2 )
{
  int l;
  fprintf( f, "# image1='%s' image2='%s'\n", par1->name, par2->name );
  fprintf( f, "\n" );
  for (l=1; l<=par1->n_labels; l++ ) {
    if ( par1->thePar[l].configuration != cfg ) continue;
    _print_one_component( f, l, par1, par2 );
  }
}





void _compute_common_box( int *ptmin1, int *ptmax1,
			  int *ptmin2, int *ptmax2,
			  int *ptmin,  int *ptmax )
{
  ptmin[0] = ptmin1[0] < ptmin2[0] ? ptmin1[0] : ptmin2[0];
  ptmin[1] = ptmin1[1] < ptmin2[1] ? ptmin1[1] : ptmin2[1];
  ptmin[2] = ptmin1[2] < ptmin2[2] ? ptmin1[2] : ptmin2[2];

  ptmax[0] = ptmax1[0] > ptmax2[0] ? ptmax1[0] : ptmax2[0];
  ptmax[1] = ptmax1[1] > ptmax2[1] ? ptmax1[1] : ptmax2[1];
  ptmax[2] = ptmax1[2] > ptmax2[2] ? ptmax1[2] : ptmax2[2];
}



void _printGraphOfComponents( FILE *f, int label1,
				typeImageParameter *par1,
				int label2,
				typeImageParameter *par2,
				int spaces )
{
  int i, s=spaces;
  
  if ( label1 <= 0 || label1 > par1->n_labels ) return;

  if ( label2 <= 0 || label2 > par2->n_labels ) {
    fprintf( f, "- " );
    s = 0;
  }
  else {
    fprintf( f, "  " );
    for ( i=0; i<s; i++ ) fprintf( f, "    " );
    fprintf( f, "<-> " );
    s++;
  }
  
  fprintf( f, "cc #%2d in '%s'", label1, par1->name );

  switch( par1->thePar[label1].configuration ) {
  default :         fprintf( f, " [???] " ); break;
  case _INCONNU_ :  fprintf( f, " [inconnue] " ); break;
  case _ISOLE_ :    fprintf( f, " [isolee] " ); break;
  case _UNIVOQUE_ : fprintf( f, " [univoque] " ); break;
  case _ZIGZAG_ :   fprintf( f, " [zigzag] " ); break;
  case _SPLIT_ :    fprintf( f, " [split] " ); break;
  case _MERGE_ :    fprintf( f, " [merge] " ); break;
  }

  if ( par1->thePar[label1].is_printed ) {
    fprintf( f, "(already printed)\n" );
    return;
  }

  fprintf( f, "\n" );

  for ( i=0; i<par1->thePar[label1].n_components; i++ ) {
    if ( par1->thePar[label1].i_component[i] != label2 )
      _printGraphOfComponents( f, par1->thePar[label1].i_component[i], 
				 par2, label1, par1, s );
  }

  par1->thePar[label1].is_printed = 1;

}



void _print_UnknownComponents( FILE *f,
			       typeImageParameter *par1,
			       typeImageParameter *par2 )
{
  int l;

  for (l=1; l<=par1->n_labels; l++ )
    par1->thePar[l].is_printed = 0;
  for (l=1; l<=par2->n_labels; l++ )
    par2->thePar[l].is_printed = 0;

  for (l=1; l<=par1->n_labels; l++ ) {
    if ( par1->thePar[l].configuration != _INCONNU_ ) continue;
    _printGraphOfComponents( f, l, par1, -1, par2, 0 );
    fprintf( f, "\n" );
  }

  for (l=1; l<=par2->n_labels; l++ ) {
    if ( par2->thePar[l].configuration != _INCONNU_ ) continue;
    _printGraphOfComponents( f, l, par2, -1, par1, 0 );
    fprintf( f, "\n" );
  }
  
}



void _print_AllComponents( FILE *f,
			       typeImageParameter *par1,
			       typeImageParameter *par2 )
{
  int l;

  for (l=1; l<=par1->n_labels; l++ )
    par1->thePar[l].is_printed = 0;
  for (l=1; l<=par2->n_labels; l++ )
    par2->thePar[l].is_printed = 0;

  for (l=1; l<=par1->n_labels; l++ ) {
    _printGraphOfComponents( f, l, par1, -1, par2, 0 );
    fprintf( f, "\n" );
  }

  for (l=1; l<=par2->n_labels; l++ ) {
    _printGraphOfComponents( f, l, par2, -1, par1, 0 );
    fprintf( f, "\n" );
  }
  
}



void _get_Statistics( int *stats, typeImageParameter *par )
{
  int i, l;
  for (i=0; i<_CONFIGURATIONS_; i++ ) stats[i] = 0;
  for (l=1; l<=par->n_labels; l++ ) {
    if ( 0 ) printf( "CFG #%2d = %d\n", l, par->thePar[l].configuration );
    stats[ par->thePar[l].configuration ] ++;
  }
}



void _print_Statistics( FILE *f, typeImageParameter *par )
{
  int s[_CONFIGURATIONS_];
  _get_Statistics( s, par );

  fprintf( f, "#\n" );
  fprintf( f, "# fichier %s\n", par->name );

  if ( s[_INCONNU_] > 0 ) 
    fprintf( f, "#    config(s) inconnue(s)  : %d\n", s[_INCONNU_] );
  if ( s[_ISOLE_] > 0 ) 
    fprintf( f, "#    comp(s) isolee(s)      : %d\n", s[_ISOLE_] );
  if ( s[_UNIVOQUE_] > 0 ) 
    fprintf( f, "#    config(s) univoques(s) : %d\n", s[_UNIVOQUE_] );
  if ( s[_ZIGZAG_] > 0 ) 
    fprintf( f, "#    config(s) zigzag(s)    : %d\n", s[_ZIGZAG_] );
  if ( s[_SPLIT_] > 0 ) 
    fprintf( f, "#    comp(s) se splittant   : %d\n", s[_SPLIT_] );
  if ( s[_MERGE_] > 0 ) 
    fprintf( f, "#    comp(s) mergeant       : %d\n", s[_MERGE_] );
  fprintf( f, "#\n" );
 
}



int _find_intersection( vt_image *im1,
			typeParameter *par1,
			int color1,
			vt_image *im2,
			typeParameter *par2,
			int color2 )
{
  char *proc = "_find_intersection";
  int n, i, j, x, y, z;
  int ptmin[3], ptmax[3];
  double bx, by, bz;
  double od, d;
  int ix = -1;
  int iy = -1;
  int iz = -1;
  
  par1->intersection[0] = -1;
  par1->intersection[1] = -1;
  par1->intersection[2] = -1;

  par2->intersection[0] = -1;
  par2->intersection[1] = -1;
  par2->intersection[2] = -1;

  if ( im1->type != im2->type ) {
    fprintf( stderr, "%s: images must have the same type\n", proc );
    return( 0 );
  }

  _compute_common_box( par1->iptmin, par1->iptmax,
		       par2->iptmin, par2->iptmax,
		       ptmin, ptmax );

  par1->iptmin[0] = ptmin[0];
  par1->iptmin[1] = ptmin[1];
  par1->iptmin[2] = ptmin[2];

  par1->iptmax[0] = ptmax[0];
  par1->iptmax[1] = ptmax[1];
  par1->iptmax[2] = ptmax[2];

  par2->iptmin[0] = ptmin[0];
  par2->iptmin[1] = ptmin[1];
  par2->iptmin[2] = ptmin[2];

  par2->iptmax[0] = ptmax[0];
  par2->iptmax[1] = ptmax[1];
  par2->iptmax[2] = ptmax[2];


  switch ( im1->type ) {
  default :
    fprintf( stderr, "%s: type not handled in switch\n", proc );
    return( 0 );
  case UCHAR :
    {
      unsigned char *buf1 = (unsigned char*)im1->buf;
      unsigned char *buf2 = (unsigned char*)im2->buf;
      
      n = 0;
      bx = by = bz = 0;

      for ( z=ptmin[2]; 
      z <= ptmax[2] && z < (int)im1->dim.z && z < (int)im2->dim.z;
	    z ++ )
      for ( y = ptmin[1]; 
      y <= ptmax[1] && y < (int)im1->dim.y && y < (int)im2->dim.y;
	    y ++ ) {
	i = z * im1->dim.y*im1->dim.x + y*im1->dim.x + ptmin[0];
	j = z * im2->dim.y*im2->dim.x + y*im2->dim.x + ptmin[0];
	for ( x = ptmin[0]; 
        x <= ptmax[0] && x < (int)im1->dim.x && x < (int)im2->dim.x;
	      x ++, i ++, j ++ ) {
	  if ( buf1[i] != color1 ) continue;
	  if ( buf2[j] != color2 ) continue;
	  bx += x;
	  by += y;
	  bz += z;
	  n ++;
	}
      }

      if ( n == 0 ) {
	fprintf( stderr, "%s: empty intersection\n", proc );
	return( 0 );
      }

      bx /= n;
      by /= n;
      bz /= n;

      od = im1->dim.x*im1->dim.x + im1->dim.y*im1->dim.y 
	+ im1->dim.z*im1->dim.z;

      for ( z=ptmin[2]; 
      z <= ptmax[2] && z < (int)im1->dim.z && z < (int)im2->dim.z;
	    z ++ )
      for ( y = ptmin[1]; 
      y <= ptmax[1] && y < (int)im1->dim.y && y < (int)im2->dim.y;
	    y ++ ) {
	i = z * im1->dim.y*im1->dim.x + y*im1->dim.x + ptmin[0];
	j = z * im2->dim.y*im2->dim.x + y*im2->dim.x + ptmin[0];
	for ( x = ptmin[0]; 
        x <= ptmax[0] && x < (int)im1->dim.x && x < (int)im2->dim.x;
	      x ++, i ++, j ++ ) {
	  if ( buf1[i] != color1 ) continue;
	  if ( buf2[j] != color2 ) continue;
	  d = (bx-x)*(bx-x) + (by-y)*(by-y) + (bz-z)*(bz-z);
	  if ( od > d ) {
	    ix = x;   iy = y;   iz = z;
	    od = d;
	  }
	}
      }

    }
    break;
  }
  
  par1->intersection[0] = ix;
  par1->intersection[1] = iy;
  par1->intersection[2] = iz;

  par2->intersection[0] = ix;
  par2->intersection[1] = iy;
  par2->intersection[2] = iz;

  return( 1 );
}
  













int _is_UNIVOQUE_( int label1,
		   vt_image *im1,
		   typeImageParameter *par1,
		   vt_image *im2,
		   typeImageParameter *par2 )
{
  char *proc = "_is_UNIVOQUE_";
  int label2;

  label2 = par1->thePar[label1].i_component[0];

  if ( par2->thePar[label2].n_components > 1 ) return( 0 );

  if ( par2->thePar[label2].n_components == 0 ) {
    fprintf( stderr, "%s: strange case #1\n", proc );
    return( -1 );
  }

  if ( par2->thePar[label2].configuration != _INCONNU_ ) {
    fprintf( stderr, "%s: strange case #2\n", proc );
    return( -1 );
  }
  
  if ( _find_intersection( im1, &(par1->thePar[label1]), label1,
			    im2, &(par2->thePar[label2]), label2 ) != 1 ) {
    fprintf( stderr, "%s: unable to find intersection\n", proc );
    return( 0 );
  }

  par1->thePar[label1].configuration = _UNIVOQUE_;
  par2->thePar[label2].configuration = _UNIVOQUE_;

  par1->thePar[label1].status = _VALIDE_;
  par2->thePar[label2].status = _VALIDE_;

  return( 1 );
}



int _is_SPLIT_( int label1,
		vt_image *im1,
		typeImageParameter *par1,
		vt_image *im2,
		typeImageParameter *par2 )
{
  char *proc = "_is_SPLIT_";
  int l;
  
  for (l=0; l<par1->thePar[label1].n_components; l++ ) {
    if ( par2->thePar[ par1->thePar[label1].i_component[l] ].n_components != 1 ) {
      return( 0 );
    }
  }

  for (l=0; l<par1->thePar[label1].n_components; l++ ) { 
    if ( _find_intersection( im1, &(par1->thePar[label1]), label1,
			     im2, &(par2->thePar[par1->thePar[label1].i_component[l]]), 
			     par1->thePar[label1].i_component[l] ) != 1 ) {
      fprintf( stderr, "%s: unable to find intersection\n", proc );
      return( 0 );
    }
  }

  par1->thePar[label1].configuration = _SPLIT_;
  par1->thePar[label1].status = _VALIDE_;

  for (l=0; l<par1->thePar[label1].n_components; l++ ) { 
    par2->thePar[par1->thePar[label1].i_component[l]].configuration = _MERGE_;
    par2->thePar[par1->thePar[label1].i_component[l]].status = _VALIDE_;
  }

  return( 1 );
}



int _is_ZIGZAG_( int label1_2,
		vt_image *im1,
		typeImageParameter *par1,
		vt_image *im2,
		typeImageParameter *par2 )
{
  char *proc = "_is_ZIGZAG_";
  int i1, i2;
  int label2_2, label2_1, label1_1;

  i1 = par1->thePar[label1_2].i_component[0];
  i2 = par1->thePar[label1_2].i_component[1];

  if ( par2->thePar[ i1 ].n_components == 2 ) {
    if ( par2->thePar[ i2 ].n_components == 1 ) {
      label2_2 = i1;
      label2_1 = i2;
    }
    else {
      return( 0 );
    }
  }
  else if ( par2->thePar[ i1 ].n_components == 1 ) {
    if ( par2->thePar[ i2 ].n_components == 2 ) {
      label2_2 = i2;
      label2_1 = i1;
    }
    else {
      return( 0 );
    }
  }
  else {
    return( 0 );
  }


  if ( par2->thePar[ label2_2 ].i_component[0] == label1_2 ) 
    label1_1 = par2->thePar[ label2_2 ].i_component[1];
  else
    label1_1 = par2->thePar[ label2_2 ].i_component[0];

  if ( par1->thePar[ label1_1 ].n_components != 1 )
    return( 0 );


  /* test d'appartenance des barycentres ?
   */
  
  par2->thePar[ label2_2 ].i_component[0] = label1_1;
  par2->thePar[ label2_2 ].n_components = 1;

  par1->thePar[ label1_2 ].i_component[0] = label2_1;
  par1->thePar[ label1_2 ].n_components = 1;

  if ( _find_intersection( im1, &(par1->thePar[label1_2]), label1_2,
			   im2, &(par2->thePar[label2_1]), label2_1 ) != 1 ) {
    fprintf( stderr, "%s: unable to find intersection #1\n", proc );
    return( 0 );
  }

  if ( _find_intersection( im1, &(par1->thePar[label1_1]), label1_1,
			   im2, &(par2->thePar[label2_2]), label2_2 ) != 1 ) {
    fprintf( stderr, "%s: unable to find intersection #2\n", proc );
    return( 0 );
  }

  par1->thePar[label1_1].configuration = _ZIGZAG_;
  par1->thePar[label1_1].status = _VALIDE_;

  par1->thePar[label1_2].configuration = _ZIGZAG_;
  par1->thePar[label1_2].status = _VALIDE_;

  par2->thePar[label2_1].configuration = _ZIGZAG_;
  par2->thePar[label2_1].status = _VALIDE_;

  par2->thePar[label2_2].configuration = _ZIGZAG_;
  par2->thePar[label2_2].status = _VALIDE_;

  return( 1 );
}



void _NameComponents( vt_image *im1,
		      typeImageParameter *par1,
		      vt_image *im2,
		      typeImageParameter *par2 )
{
  int l;

  for ( l=1; l<=par1->n_labels; l++ ) {

    if ( par1->thePar[l].configuration != _INCONNU_ ) continue;

    if ( par1->thePar[l].n_components == 0 ) {
      par1->thePar[l].configuration = _ISOLE_;
      par1->thePar[l].status = _VALIDE_;
      continue;
    }

    if ( par1->thePar[l].n_components > 1 ) {

      if ( par1->thePar[l].n_components == 2 && 
	   _is_ZIGZAG_( l, im1, par1, im2, par2 ) == 1 ) continue;
      
      if ( _is_SPLIT_( l, im1, par1, im2, par2 ) == 1 ) continue;
      
      continue;
    }

    if ( _is_UNIVOQUE_( l, im1, par1, im2, par2 ) == 1 ) continue;

  }

  
}










static void _searchCorrespondingComponents( vt_image *im1,
					    typeImageParameter *par1,
					    vt_image *im2 )
{
  char *proc = "_searchCorrespondingComponents";
  int l, n, x, y, z, i, j;
  
  if ( im1->type != im2->type ) {
    fprintf( stderr, "%s: images must have the same type\n", proc );
    return;
  }
  
  switch ( im1->type ) {
  default :
    fprintf( stderr, "%s: type not handled in switch\n", proc );
    return;
  case UCHAR :
    {
      unsigned char *buf1 = (unsigned char*)im1->buf;
      unsigned char *buf2 = (unsigned char*)im2->buf;
      
      for ( l=1; l<=par1->n_labels; l++ ) {
	
	for ( z = par1->thePar[l].ptmin[2]; 
        z <= par1->thePar[l].ptmax[2] && z < (int)im1->dim.z && z < (int)im2->dim.z;
	      z ++ )
	for ( y = par1->thePar[l].ptmin[1]; 
        y <= par1->thePar[l].ptmax[1] && y < (int)im1->dim.y && y < (int)im2->dim.y;
	      y ++ ) {
	  i = z * im1->dim.y*im1->dim.x + y*im1->dim.x + par1->thePar[l].ptmin[0];
	  j = z * im2->dim.y*im2->dim.x + y*im2->dim.x + par1->thePar[l].ptmin[0];
	  for ( x = par1->thePar[l].ptmin[0]; 
    x <= par1->thePar[l].ptmax[0] && x < (int)im1->dim.x && x < (int)im2->dim.x;
		x ++, i ++, j ++ ) {
	    
	    if ( buf1[i] != l ) continue;
	    if ( buf2[j] == 0 ) continue;
	    
	    for ( n=0; n<par1->thePar[l].n_components &&
		    par1->thePar[l].i_component[ n ] != buf2[j]; n++ ) 
	      ;
	    if ( n != par1->thePar[l].n_components ) continue;
	    
	    if ( par1->thePar[l].n_components == _COMPONENTS_ ) {
	      fprintf( stderr, "%s: too many corresponding components for cc #%d in '%s'\n", proc, l, par1->name );
	      continue;
	    }
	    
	    par1->thePar[l].i_component[ par1->thePar[l].n_components ] = buf2[j];
	    par1->thePar[l].n_components ++;
	    
	  }
	}
      }
    }
    break;
  case USHORT :
    {
      unsigned short int *buf1 = (unsigned short int*)im1->buf;
      unsigned short int *buf2 = (unsigned short int*)im2->buf;
      
      for ( l=1; l<=par1->n_labels; l++ ) {
	
	for ( z = par1->thePar[l].ptmin[2]; 
        z <= par1->thePar[l].ptmax[2] && z < (int)im1->dim.z && z < (int)im2->dim.z;
	      z ++ )
	for ( y = par1->thePar[l].ptmin[1]; 
        y <= par1->thePar[l].ptmax[1] && y < (int)im1->dim.y && y < (int)im2->dim.y;
	      y ++ ) {
	  i = z * im1->dim.y*im1->dim.x + y*im1->dim.x + par1->thePar[l].ptmin[0];
	  j = z * im2->dim.y*im2->dim.x + y*im2->dim.x + par1->thePar[l].ptmin[0];
	  for ( x = par1->thePar[l].ptmin[0]; 
    x <= par1->thePar[l].ptmax[0] && x < (int)im1->dim.x && x < (int)im2->dim.x;
		x ++, i ++, j ++ ) {
	    
	    if ( buf1[i] != l ) continue;
	    if ( buf2[j] == 0 ) continue;
	    
	    for ( n=0; n<par1->thePar[l].n_components &&
		    par1->thePar[l].i_component[ n ] != buf2[j]; n++ ) 
	      ;
	    if ( n != par1->thePar[l].n_components ) continue;
	    
	    if ( par1->thePar[l].n_components == _COMPONENTS_ ) {
	      fprintf( stderr, "%s: too many corresponding components for cc #%d in '%s'\n", proc, l, par1->name );
	      continue;
	    }
	    
	    par1->thePar[l].i_component[ par1->thePar[l].n_components ] = buf2[j];
	    par1->thePar[l].n_components ++;
	    
	  }
	}
      }
    }
    break;
  }

}




void _FollowsComponents( vt_image *im1,
			 typeImageParameter *par1,
			 vt_image *im2,
			 typeImageParameter *par2 )
{
  char *proc = "_FollowsComponents";

  if ( im1->type != im2->type ) {
    fprintf( stderr, "%s: images must have the same type\n", proc );
    return;
  }

  _searchCorrespondingComponents( im1, par1, im2 );
  _searchCorrespondingComponents( im2, par2, im1 );
  
  
}
			 
			 


















static void _initParameter( typeParameter *par,
			    int dimx, int dimy, int dimz )
{
  int i;
  
  par->ptmin[0] = dimx-1;
  par->ptmin[1] = dimy-1;
  par->ptmin[2] = dimz-1;

  par->ptmax[0] = 0;
  par->ptmax[1] = 0;
  par->ptmax[2] = 0;

  par->volume = 0;
  
  par->dbary[0] = 0.0;
  par->dbary[1] = 0.0;
  par->dbary[2] = 0.0;

  par->ibary[0] = 0;
  par->ibary[1] = 0;
  par->ibary[2] = 0;

  par->n_components = 0;
  
  for (i=0; i<_COMPONENTS_; i++ ) 
    par->i_component[i] = 0;

  par->configuration = _INCONNU_;
  par->status = _INVALIDE_;

  par->intersection[0] = 0;
  par->intersection[1] = 0;
  par->intersection[2] = 0;


  par->is_printed = 0;
}




static void _FillStructure( vt_image *theIm,
			    typeParameter *par,
			    int color )

{
  char *proc = "_FillStructure";
  int i, x, y, z;
  double od, d;

  int *minCorner = par->ptmin;
  int *maxCorner = par->ptmax;

  par->dbary[0] = 0;
  par->dbary[1] = 0;
  par->dbary[2] = 0;

  par->ibary[0] = 0;
  par->ibary[1] = 0;
  par->ibary[2] = 0;

  switch ( theIm->type ) {
  default :
    fprintf( stderr, "%s: type not handled in switch\n", proc );
    return;
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char*)theIm->buf;

      for ( z=minCorner[2]; z<=maxCorner[2] && z<(int)theIm->dim.z; z ++ )
      for ( y=minCorner[1]; y<=maxCorner[1] && y<(int)theIm->dim.y; y ++ ) {
	i = z * theIm->dim.y*theIm->dim.x + y * theIm->dim.x + minCorner[0];
  for ( x=minCorner[0]; x<=maxCorner[0] && x<(int)theIm->dim.x; x ++, i++ ) {
	  
	  if ( theBuf[i] != color ) continue;
	
	  par->dbary[0] += x;
	  par->dbary[1] += y;
	  par->dbary[2] += z;
	}
      }

      par->dbary[0] /= par->volume;
      par->dbary[1] /= par->volume;
      par->dbary[2] /= par->volume;

      od = theIm->dim.x*theIm->dim.x + theIm->dim.y*theIm->dim.y 
	+ theIm->dim.z*theIm->dim.z;

      for ( z=minCorner[2]; z<=maxCorner[2] && z<(int)theIm->dim.z; z ++ )
      for ( y=minCorner[1]; y<=maxCorner[1] && y<(int)theIm->dim.y; y ++ ) {
	i = z * theIm->dim.y*theIm->dim.x + y * theIm->dim.x + minCorner[0];
  for ( x=minCorner[0]; x<=maxCorner[0] && x<(int)theIm->dim.x; x ++, i++ ) {

	  if ( theBuf[i] != color ) continue;
	
	  d = (par->dbary[0] - x )*(par->dbary[0] - x ) +
	    (par->dbary[1] - y )*(par->dbary[1] - y ) +
	    (par->dbary[2] - z )*(par->dbary[2] - z );
	
	  if ( d < od ) {
	    par->ibary[0] = x;
	    par->ibary[1] = y;
	    par->ibary[2] = z;
	    od = d;
	  }
	}
      }
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int *)theIm->buf;

      for ( z=minCorner[2]; z<=maxCorner[2] && z<(int)theIm->dim.z; z ++ )
      for ( y=minCorner[1]; y<=maxCorner[1] && y<(int)theIm->dim.y; y ++ ) {
	i = z * theIm->dim.y*theIm->dim.x + y * theIm->dim.x + minCorner[0];
  for ( x=minCorner[0]; x<=maxCorner[0] && x<(int)theIm->dim.x; x ++, i++ ) {
	  
	  if ( theBuf[i] != color ) continue;
	
	  par->dbary[0] += x;
	  par->dbary[1] += y;
	  par->dbary[2] += z;
	}
      }

      par->dbary[0] /= par->volume;
      par->dbary[1] /= par->volume;
      par->dbary[2] /= par->volume;

      od = theIm->dim.x*theIm->dim.x + theIm->dim.y*theIm->dim.y 
	+ theIm->dim.z*theIm->dim.z;

      for ( z=minCorner[2]; z<=maxCorner[2] && z<(int)theIm->dim.z; z ++ )
      for ( y=minCorner[1]; y<=maxCorner[1] && y<(int)theIm->dim.y; y ++ ) {
	i = z * theIm->dim.y*theIm->dim.x + y * theIm->dim.x + minCorner[0];
  for ( x=minCorner[0]; x<=maxCorner[0] && x<(int)theIm->dim.x; x ++, i++ ) {

	  if ( theBuf[i] != color ) continue;
	
	  d = (par->dbary[0] - x )*(par->dbary[0] - x ) +
	    (par->dbary[1] - y )*(par->dbary[1] - y ) +
	    (par->dbary[2] - z )*(par->dbary[2] - z );
	
	  if ( d < od ) {
	    par->ibary[0] = x;
	    par->ibary[1] = y;
	    par->ibary[2] = z;
	    od = d;
	  }
	}
      }
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int *)theIm->buf;

      for ( z=minCorner[2]; z<=maxCorner[2] && z<(int)theIm->dim.z; z ++ )
      for ( y=minCorner[1]; y<=maxCorner[1] && y<(int)theIm->dim.y; y ++ ) {
	i = z * theIm->dim.y*theIm->dim.x + y * theIm->dim.x + minCorner[0];
  for ( x=minCorner[0]; x<=maxCorner[0] && x<(int)theIm->dim.x; x ++, i++ ) {
	  
	  if ( theBuf[i] != color ) continue;
	
	  par->dbary[0] += x;
	  par->dbary[1] += y;
	  par->dbary[2] += z;
	}
      }

      par->dbary[0] /= par->volume;
      par->dbary[1] /= par->volume;
      par->dbary[2] /= par->volume;

      od = theIm->dim.x*theIm->dim.x + theIm->dim.y*theIm->dim.y 
	+ theIm->dim.z*theIm->dim.z;

      for ( z=minCorner[2]; z<=maxCorner[2] && z<(int)theIm->dim.z; z ++ )
      for ( y=minCorner[1]; y<=maxCorner[1] && y<(int)theIm->dim.y; y ++ ) {
	i = z * theIm->dim.y*theIm->dim.x + y * theIm->dim.x + minCorner[0];
  for ( x=minCorner[0]; x<=maxCorner[0] && x<(int)theIm->dim.x; x ++, i++ ) {

	  if ( theBuf[i] != color ) continue;
	
	  d = (par->dbary[0] - x )*(par->dbary[0] - x ) +
	    (par->dbary[1] - y )*(par->dbary[1] - y ) +
	    (par->dbary[2] - z )*(par->dbary[2] - z );
	
	  if ( d < od ) {
	    par->ibary[0] = x;
	    par->ibary[1] = y;
	    par->ibary[2] = z;
	    od = d;
	  }
	}
      }
    }
    break;
   }

  par->intersection[0] = par->ibary[0];
  par->intersection[1] = par->ibary[1];
  par->intersection[2] = par->ibary[2];

  return;
  
}




typeParameter *ComputeParameterFromLabels( vt_image *theIm,
					   int *ncc )
{
  char *proc = "ComputeParameterFromLabels";
  typeParameter *thePar = NULL;
  int n_labels = 0;
  int x, y, z, i, n;

  switch ( theIm->type ) {
  default :
    fprintf( stderr, "%s: type not handled in switch\n", proc );
    return( NULL );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char*)theIm->buf;
      for ( i=theIm->dim.x*theIm->dim.y*theIm->dim.z-1; i>=0; i-- )
	if ( n_labels < theBuf[i] ) n_labels = theBuf[i];
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int*)theIm->buf;
      for ( i=theIm->dim.x*theIm->dim.y*theIm->dim.z-1; i>=0; i-- )
	if ( n_labels < theBuf[i] ) n_labels = theBuf[i];
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int*)theIm->buf;
      for ( i=theIm->dim.x*theIm->dim.y*theIm->dim.z-1; i>=0; i-- )
	if ( n_labels < theBuf[i] ) n_labels = theBuf[i];
    }
    break;
  }

  *ncc = n_labels;
  if ( n_labels <= 0 ) return( NULL );

  /* first allocation
   */
  thePar = (typeParameter *)malloc( (n_labels+1)*sizeof(typeParameter) );
  if ( thePar == NULL ) return( NULL );
  
  

  /* initialisations 
   */
  for ( n=0; n<=n_labels; n++ ) 
    _initParameter( &(thePar[n]), theIm->dim.x, theIm->dim.y, theIm->dim.z );




  /* first parameters
   */
  switch ( theIm->type ) {
  default :
    fprintf( stderr, "%s: type not handled in switch\n", proc );
    return( NULL );
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char*)theIm->buf;
      for ( i=0, z=0; z<(int)theIm->dim.z; z++ )
      for ( y=0; y<(int)theIm->dim.y; y++ )
      for ( x=0; x<(int)theIm->dim.x; x++, i++ ) {

	if ( theBuf[i] == 0 )       continue;
	if ( theBuf[i] > n_labels ) continue;
    
	n = theBuf[i];
	
	thePar[n].volume ++;
	if ( x < thePar[n].ptmin[0] ) thePar[n].ptmin[0] = x;
	if ( y < thePar[n].ptmin[1] ) thePar[n].ptmin[1] = y;
	if ( z < thePar[n].ptmin[2] ) thePar[n].ptmin[2] = z;
	if ( x > thePar[n].ptmax[0] ) thePar[n].ptmax[0] = x;
	if ( y > thePar[n].ptmax[1] ) thePar[n].ptmax[1] = y;
	if ( z > thePar[n].ptmax[2] ) thePar[n].ptmax[2] = z;
      }
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int*)theIm->buf;
      for ( i=0, z=0; z<(int)theIm->dim.z; z++ )
      for ( y=0; y<(int)theIm->dim.y; y++ )
      for ( x=0; x<(int)theIm->dim.x; x++, i++ ) {

	if ( theBuf[i] == 0 )       continue;
	if ( theBuf[i] > n_labels ) continue;
    
	n = theBuf[i];
	
	thePar[n].volume ++;
	if ( x < thePar[n].ptmin[0] ) thePar[n].ptmin[0] = x;
	if ( y < thePar[n].ptmin[1] ) thePar[n].ptmin[1] = y;
	if ( z < thePar[n].ptmin[2] ) thePar[n].ptmin[2] = z;
	if ( x > thePar[n].ptmax[0] ) thePar[n].ptmax[0] = x;
	if ( y > thePar[n].ptmax[1] ) thePar[n].ptmax[1] = y;
	if ( z > thePar[n].ptmax[2] ) thePar[n].ptmax[2] = z;
      }
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int*)theIm->buf;
      for ( i=0, z=0; z<(int)theIm->dim.z; z++ )
      for ( y=0; y<(int)theIm->dim.y; y++ )
      for ( x=0; x<(int)theIm->dim.x; x++, i++ ) {

	if ( theBuf[i] <= 0 )       continue;
	if ( theBuf[i] > n_labels ) continue;
    
	n = theBuf[i];
	
	thePar[n].volume ++;
	if ( x < thePar[n].ptmin[0] ) thePar[n].ptmin[0] = x;
	if ( y < thePar[n].ptmin[1] ) thePar[n].ptmin[1] = y;
	if ( z < thePar[n].ptmin[2] ) thePar[n].ptmin[2] = z;
	if ( x > thePar[n].ptmax[0] ) thePar[n].ptmax[0] = x;
	if ( y > thePar[n].ptmax[1] ) thePar[n].ptmax[1] = y;
	if ( z > thePar[n].ptmax[2] ) thePar[n].ptmax[2] = z;
      }
    }
    break;
  }





  /* more parameters
   */
  for ( n=1; n<=n_labels; n++ ) {

    thePar[n].iptmin[0] = thePar[n].ptmin[0];
    thePar[n].iptmin[1] = thePar[n].ptmin[1];
    thePar[n].iptmin[2] = thePar[n].ptmin[2];

    thePar[n].iptmax[0] = thePar[n].ptmax[0];
    thePar[n].iptmax[1] = thePar[n].ptmax[1];
    thePar[n].iptmax[2] = thePar[n].ptmax[2];

    _FillStructure( theIm, &thePar[n], n );
  }


  return( thePar );
}

