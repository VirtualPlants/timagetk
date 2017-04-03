#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <chamfercoefficients.h>

#include <vt_common.h>




static char *options = "\
  [-dim %d] [-mask-size|-mask|-size %d] [-error %f] [-vectors %d]\n\
  [-prebuild|-pb %d] [-build-only|-bo] [-y %f] [-z %f]\n\
  [-f %d] [-l %d]\n\
  [-res %s] [-matlab %s] [-latex %s] [-imask %d] [-txt %s] [-image %s]\n\
  [-help|--help] [-examples|-ex]\n\
  [-v [%d]] [-trace [%d]] [-debug]";





static char *details = "\n\
\n\
 =====  mask construction =====\n\
-dim %d : space dimension (3 or 2 (default))\n\
-mask-size|-mask|-size %d : mask size\n\
 3 (default) -> mask 3x3 or 3x3x3\n\
 5 ->  mask 5x5 or 5x5x5\n\
 etc.\n\
-vectors %d : maximal number of vectors in the initial triangulation\n\
   (ie after the symmetrisation due to the anisotropy is done)\n\
   E.g.: '-dim 2 -vectors 4' generates 4 vectors in the sector [(1,0),(1,1)]\n\
         and a mask with 24 values, while '-dim 2 -vectors 4 -y 1.2' generates\n\
         4 vectors in the two sectors [(1,0),(1,1)] and [(1,1),(0,1)] and then\n\
         a mask with 16 values.\n\
   if -mask-size is also specified (after in the command line)\n\
   the vectors are restricted in the given neighborhood\n\
-error %f : maximal sector error\n\
   0.01 means 1%\n\
   if -mask-size is also specified (after in the command line)\n\
   the vectors are restricted in the given neighborhood\n\
   -vectors %d specifies a maximal number of vectors\n\
-prebuild|-pb %d : prebuilt masks (assume dim=3)\n\
   admissible values:\n\
   3, 333 -> mask 3x3x3\n\
   5, 555 -> mask 5x5x5\n\
   7, 777 -> mask 7x7x7\n\
   335\n\
   355\n\
   this was implemented for comparison with (Fouard, IVC, 2005)\n\
-x %f : pixel or voxel dimension along X\n\
-y %f : pixel or voxel dimension along Y\n\
-z %f : voxel dimension along z\n\
-build-only|-bo: do not compute weights\n\
\n\
 =====  weight computing  =====\n\
-f %d : first weight value (default=1)\n\
-l %d : last weight value (default=20)\n\
\n\
 ===== output controlling =====\n\
-res %s : specify output text file (default is stdout)\n\
-matlab %s : specify output matlab file\n\
   nice plot of mask errors\n\
-latex %s : specify output latex file\n\
   latex table of computed weights\n\
-imask %d : choose the mask (index) for output\n\
   the first one has a 0 index\n\
-txt %s : specify text output files\n\
   %s contains the mask values (in text)\n\
   %s.info contains some information\n\
   the chosen mask is the last one (the one with largest values)\n\
   except if it is specifies with -imask\n\
-image %s : specify image output files\n\
   %s contains the mask values\n\
   %s.info contains some information\n\
   the chosen mask is the last one (the one with largest values)\n\
   except if it is specifies with -imask\n\
\n\
 =====        misc        =====\n\
-help : print this help\n\
-examples|-ex : print some command lines examples\n\
-v [%d] : be verbose [indicates the level of verbosity]\n\
-trace [%d] : write some traces [indicates the level of verbosity]\n\
   (I guess you want to use it)\n\
-debug [%d] : write some other stuff [indicates the level of verbosity]\n\
   (I guess you want to use it)\n\
=====        in progress    ======\n\
to be used at your own risk ...\n\
-online : try to produce natural triangulations by correcting concavities\n\
   (by swapping edegs between adjacent sectors) while building the mask\n\
-offline : try to produce natural triangulations by correcting concavities\n\
   (by swapping edegs between adjacent sectors) after building the mask\n\
\n";





static void usage( char *prg, char *str, int flag )
{ 
  fprintf( stderr, "\n" );
  fprintf( stderr, "%s:\n%s\n\n", prg, options );
  if ( flag ) 
    fprintf( stderr, "%s\n", details );
  if ( str != NULL ) 
    fprintf( stderr, "Error: %s\n", str );
}





const char *examples[] = {
"-dim 3 -size 5 -matlab ... -latex ...",
"-dim 3 -pb 777 -matlab ... -latex ...",
"-dim 3 -size 7 -matlab ... -latex ...",
"-dim 3 -size 3 -y 1.2 -z 2 -matlab ... -latex ...",
"-dim 3 -pb 335 -y 1.2 -z 2 -matlab ... -latex ...",
"-dim 3 -pb 355 -y 1.2 -z 2 -matlab ... -latex ...",
"-dim 3 -size 5 -y 1.2 -z 2 -matlab ... -latex ...",
"-dim 3 -vectors 10 -y 1.2 -z 2 -matlab ... -latex ...",
"-dim 3 -vectors 16 -size 5 -y 1.2 -z 2 -matlab ... -latex ...",
"-dim 3 -vectors 16 -y 1.2 -z 2 -matlab ... -latex ...",
"-dim 3 -vectors 19 -y 1.2 -z 2 -matlab ... -latex ...",
"" };

static void print_examples( char *prg )
{
  int i;
  for ( i=0; strlen( examples[i] ) > 0; i ++ ) 
    fprintf( stdout, "  %s %s\n", prg, examples[i] );
}

















typedef enum {
  _PREBUILD_,
  _SIZE_,
  _ERROR_
} BuildingCriteriumType;











int main( int argc, char *argv[] )
{
  int i, t;
  int status;
  char text[256];
  
  int first = 1;
  int last = 20;

  int verbose = 0;
  int debug = 0;
  int trace = 0;
  
  FILE *fout = stdout;
  char *nout = NULL;

  FILE *fmatlab = NULL;
  char *nmatlab = NULL;

  FILE *flatex = NULL;
  char *nlatex = NULL;

  FILE *ftxt = NULL;
  char *ntxt = NULL;

  char *nimage = NULL;

  FILE *finfo;

  int  dims[3];
  int  *btxt = NULL;

  vt_image imres;



  char nfile[1024];

  int imask = -1;

  int anisotropy_x = 0;
  int anisotropy_y = 0;
  int anisotropy_z = 0;

  int build_only = 0;
  int prebuild = 0;

  double voxel_size[3] = { 1.0, 1.0, 1.0 };
  double original_voxel_size[3] = { 1.0, 1.0, 1.0 };
  int dim = 2;

  int mask_size = 3;
  double mask_error = 0.0001;
  int nb_vectors = 1000;

  VectorList vl;
  ConeList cl;

  WeightsList wl;

  BuildingCriteriumType criterium = _SIZE_;

  int online_convexity_correction = 0;
  int offline_convexity_correction = 0;





  /*----------------------------------------
    parsing args
    ----------------------------------------*/
  if ( argc == 1 ) {
    usage( argv[0], NULL, 0 );
    return( 0 );
  }
    
  for ( i=1; i<argc; i++ ) {

    if ( argv[i][0] == '-' ) {

      /* general stuff
       */

      if ( strcmp ( argv[i], "-help" ) == 0  || 
	   strcmp ( argv[i], "--help" ) == 0 ) {
	usage( argv[0], NULL, 1 );
	return( 0 );
      }

      if ( strcmp ( argv[i], "-examples" ) == 0  || 
	   strcmp ( argv[i], "-ex" ) == 0 ) {
	print_examples( argv[0] );
	return( 0 );
      }

      else if ( strcmp ( argv[i], "-verbose" ) == 0 || 
		(strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
	i += 1;
	if ( i >= argc) {
	  if ( verbose <= 0 ) 
	    verbose = 1;
	  else
	    verbose ++;
	  _set_chamfer_verbose( verbose );
	  i -= 1;
	}
	else {
	  status = sscanf( argv[i],"%d", &t );
	  if ( status <= 0 ) {
	    if ( verbose <= 0 ) 
	      verbose = 1;
	    else
	      verbose ++;
	    _set_chamfer_verbose( verbose );
	    i -= 1;
	  }
	  else {
	    verbose = t;
	    _set_chamfer_verbose( verbose );
	  }
	}
      }

      else if ( strcmp ( argv[i], "-trace" ) == 0  ) {
	i += 1;
	if ( i >= argc) {
	  trace ++;
	  _set_chamfer_trace( trace );
	  i -= 1;
	}
	else {
	  status = sscanf( argv[i],"%d", &t );
	  if ( status <= 0 ) {
	    trace ++;
	    _set_chamfer_trace( trace );
	    i -= 1;
	  }
	  else {
	    trace = t;
	    _set_chamfer_trace( trace );
	  }
	}
      }
      
      else if ( strcmp ( argv[i], "-debug" ) == 0  ) {
	i += 1;
	if ( i >= argc) {
	  debug ++;
	  _set_chamfer_debug( debug );
	  i -= 1;
	}
	else {
	  status = sscanf( argv[i],"%d", &t );
	  if ( status <= 0 ) {
	    debug ++;
	    _set_chamfer_debug( debug );
	    i -= 1;
	  }
	  else {
	    debug = t;
	    _set_chamfer_debug( debug );
	  }
	}
      }
      
      /* building masks
       */

      else if ( strcmp ( argv[i], "-dim" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -dim", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%d", &dim );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -dim", 0 );
	  return( 1 );
	}
	if ( dim != 2 && dim != 3 ) {
	  usage( argv[0], "dimension not handled yet (-dim)", 0 );
	  return( 1 );
	}
	  
      }
      
      else if ( strcmp ( argv[i], "-mask-size" ) == 0 
		|| strcmp ( argv[i], "-mask" ) == 0
		|| strcmp ( argv[i], "-size" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -mask-size", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%d", &mask_size );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -mask-size", 0 );
	  return( 1 );
	}
      }
      
      else if ( strcmp ( argv[i], "-vectors" ) == 0 
		|| strcmp ( argv[i], "-vector" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -vectors", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%d", &nb_vectors );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -vectors", 0 );
	  return( 1 );
	}
	criterium = _ERROR_;
	mask_size = 101;
      }

      else if ( strcmp ( argv[i], "-error" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -error", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%lf", &mask_error );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -error", 0 );
	  return( 1 );
	}
	criterium = _ERROR_;
	mask_size = 101;
      }
      
      else if ( strcmp ( argv[i], "-prebuild" ) == 0 
		|| strcmp ( argv[i], "-pb" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -prebuild", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%d", &prebuild );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -prebuild", 0 );
	  return( 1 );
	}
	switch ( prebuild ) {
	case 3 :
	case 333 :
	case 5 :
	case 555 :
	case 5552 :
	case 5553 :
	case 7 :
	case 777 :
	case 335 :
	case 3352 :
	case 3353 :
	case 355 :
	  break;
	default :
	  usage( argv[0], "unknown mask (-prebuild)", 0 );
	}
	criterium = _PREBUILD_;
	dim = 3;
      }
      
      else if ( strcmp ( argv[i], "-build-only" ) == 0 
		|| strcmp ( argv[i], "-bo" ) == 0 ) {
	build_only = 1;
	if ( verbose <= 0 ) {
	  verbose = 1;
	  _set_chamfer_verbose( verbose );
	}
      }
      
      else if ( strcmp ( argv[i], "-x" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -x", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%lf", &(original_voxel_size[0]) );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -x", 0 );
	  return( 1 );
	}
	anisotropy_x = 1;
      }
      
      else if ( strcmp ( argv[i], "-y" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -y", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%lf", &(original_voxel_size[1]) );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -y", 0 );
	  return( 1 );
	}
	anisotropy_y = 1;
      }
      
      else if ( strcmp ( argv[i], "-z" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -z", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%lf", &(original_voxel_size[2]) );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -z", 0 );
	  return( 1 );
	}
	anisotropy_z = 1;
      }

      else if ( strcmp ( argv[i], "-online" ) == 0 ) {
	online_convexity_correction = 1;
      }
      else if ( strcmp ( argv[i], "-offline" ) == 0 ) {
	offline_convexity_correction = 1;
      }

      
      /* weights computation
       */
      
      else if ( strcmp ( argv[i], "-f" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -f", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%d", &first );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -f", 0 );
	  return( 1 );
	}
      }
      
      else if ( strcmp ( argv[i], "-l" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -l", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%d", &last );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -l", 0 );
	  return( 1 );
	}
      }

      /* output parameters
       */
      
      else if ( strcmp ( argv[i], "-res" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -res", 0 );
	  return( 1 );
	}
	nout = argv[i];
      }
      
      else if ( strcmp ( argv[i], "-matlab" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -matlab", 0 );
	  return( 1 );
	}
	nmatlab = argv[i];
      }
      
      else if ( strcmp ( argv[i], "-latex" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -latex", 0 );
	  return( 1 );
	}
	nlatex = argv[i];
      }
      
      else if ( strcmp ( argv[i], "-imask" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -imask", 0 );
	  return( 1 );
	}
	status = sscanf( argv[i],"%d", &imask );
	if ( status <= 0 ) {
	  usage( argv[0], "error when parsing -imask", 0 );
	  return( 1 );
	}
      }
      
      else if ( strcmp ( argv[i], "-txt" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -txt", 0 );
	  return( 1 );
	}
	ntxt = argv[i];
      }
      
      else if ( strcmp ( argv[i], "-image" ) == 0 ) {
	i += 1;
	if ( i >= argc) {
	  usage( argv[0], "missing arg to -image", 0 );
	  return( 1 );
	}
	nimage = argv[i];
      }
      
      /* unknown options
       */

      else {
	sprintf(text,"unknown option %s\n",argv[i]);	
	usage( argv[0], text, 0 );
      }
    }

    else {
      sprintf(text,"unknown option %s\n",argv[i]);	
      usage( argv[0], text, 0 );
    }
  }





  /*----------------------------------------

    ----------------------------------------*/

  if ( nout != NULL ) {
    fout = fopen( nout, "w" );
    if ( fout == NULL ) {
      usage( argv[0], "error when opening result file\n", 0 );
      return( 1 );
    }
    fprintf( fout, "\n" );
    fprintf( fout, "%%" );
    for (i=0; i<argc; i++ )
      fprintf( fout, " %s", argv[i] );
    fprintf( fout, "\n\n" );
  }

  _init_vector_list( &vl );
  _init_cone_list( &cl );


  /* anisotropies
   */
  voxel_size[0] = original_voxel_size[0];
  voxel_size[1] = original_voxel_size[1];
  voxel_size[2] = original_voxel_size[2];

  if ( anisotropy_x ) {
    voxel_size[0] = 1.0;
    voxel_size[1] /= original_voxel_size[0];
    anisotropy_y = 1;
    if ( dim == 3 ) {
      voxel_size[2] /= original_voxel_size[0];
      anisotropy_z = 1;
    }
  }
  
  /* Mask Initialization
   */
  switch( dim ) {
  case 2 :
  default :
    _init_2D_mask( &vl, &cl, voxel_size );
    break;
  case 3 :
    _init_3D_mask( fout, &vl, &cl, voxel_size ); 
    break;
  }

  

  /* Symmetrisation, if required
   */
  switch( criterium ) {
  case _ERROR_ :
    switch( dim ) {
    case 2 :
    default :
      _symmetrise_2D_mask( &vl, &cl, voxel_size, anisotropy_y );
      break;
    case 3 :
      _symmetrise_3D_mask( fout, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );
      break;
    }
    break;
  default :
    break;
  }





  /* Mask Computation
   */
  switch( criterium ) {
    
  case _PREBUILD_ :
    switch ( prebuild ) {
    case 3 :
    case 333 :
      break;
    case 5 :
    case 555 :
      _build_3D_mask_5x5x5( fout, &vl, &cl, voxel_size );
      break;
    case 5552 :
      _build_3D_mask_5x5x5_2( fout, &vl, &cl, voxel_size );
      break;
    case 5553 :
      _build_3D_mask_5x5x5_3( fout, &vl, &cl, voxel_size );
      break;
    case 7 :
    case 777 :
      _build_3D_mask_7x7x7( fout, &vl, &cl, voxel_size );
      break;
    case 335 :
      _build_3D_mask_3x3x5( fout, &vl, &cl, voxel_size );
      break;
    case 3352 :
      _build_3D_mask_3x3x5_2( fout, &vl, &cl, voxel_size );
      break;
    case 3353 :
      _build_3D_mask_3x3x5_3( fout, &vl, &cl, voxel_size );
      break;
    case 355 :
      _build_3D_mask_3x5x5( fout, &vl, &cl, voxel_size );
      break;
    }
    break;

  case _SIZE_ :
    switch( dim ) {
    case 2 :
    default :
      _build_2D_mask( &vl, &cl, voxel_size, mask_size );
      break;
    case 3 :
      _build_3D_mask( fout, &vl, &cl, voxel_size, mask_size, online_convexity_correction );
      break;
    }
    break;

  case _ERROR_ :
    switch( dim ) {
    case 2 :
    default :
      _build_2D_mask_wrt_error( fout, &vl, &cl, voxel_size, 
				mask_size, nb_vectors, mask_error );
      break;
    case 3 :
      _build_3D_mask_wrt_error( fout, &vl, &cl, voxel_size, 
				mask_size,  nb_vectors, mask_error, online_convexity_correction );
      break;
    }
    break;
  }


  /* Symmetrisation, if required
   */
  switch( criterium ) {
    
  case _PREBUILD_ :
    _symmetrise_3D_mask( fout, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );
    break;

  case _SIZE_ :
    switch( dim ) {
    case 2 :
    default :
      _symmetrise_2D_mask( &vl, &cl, voxel_size, anisotropy_y );
      break;
    case 3 :
      _symmetrise_3D_mask( fout, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );
      break;
    }
    
    break;
  default :
    break;
  }

  if ( offline_convexity_correction ) {
    _correct_3D_mask_wrt_naturalconvexity( fout, &vl, &cl, voxel_size );
  }



  /* Print Stuff
   */
  switch( dim ) {
  case 2 :
  default :
     if ( verbose >= 1 )
       _print_2D_vector_list( fout, &vl );
    break;
  case 3 :
     if ( verbose >= 1 )
       _print_3D_vector_list( fout, &vl );
    _check_3D_mask_naturalconvexity( fout, &vl, &cl, voxel_size );
    break;
  }


  if ( verbose >= 1 ) {
    if ( 1 ) fprintf( fout, "MASK OLD COMPL. = %d\n", _old_compute_mask_vector_number( &vl ) );
    fprintf( fout, "MASK COMPLEXITY = %d\n", _compute_mask_vector_number( &vl, &cl, voxel_size, anisotropy_y, anisotropy_z ) );
    _print_cone_list( fout, &vl, &cl );
  }






  switch( dim ) {
  case 2 :
  default :
    break;
  case 3 :
    _check_3D_mask_fareyregularity( fout, &vl, &cl );
    break;
  }

  /* print Matlab stuff, if required
   */

  if ( nmatlab != NULL ) {
    fmatlab = fopen( nmatlab, "w" );
    if ( fmatlab == NULL ) {
      usage( argv[0], "error when opening matlab file\n", 0 );
      return( 1 );
    }
    switch( dim ) {
    case 2 :
    default :
      _print_2D_mask_matlab( fmatlab, &vl, &cl );
      break;
    case 3 :
      _print_3D_mask_matlab( fmatlab, &vl, &cl );
      break;
    }
    fclose( fmatlab );
  }







  /* compute weights
   */




  _init_weights_list( &wl );
  wl.error_optimal = cl.allocatedCones[0].error;

  if ( build_only == 0 ) {

    /* compute weights
     */

    switch( dim ) {
    case 2 :
    default :
      (void) _compute_2D_weights( fout, &wl, &vl, &cl, first, last );
      break;
    case 3 :
      (void) _compute_3D_weights( fout, &wl, &vl, &cl, voxel_size, first, last );
      break;
    }
  }



  
  if ( wl.n_weights > 0 ) {

    _print_weights_list( fout, &wl, &vl );
    
    if ( nlatex != NULL ) {
      flatex = fopen( nlatex, "w" );
      if ( flatex == NULL ) {
	usage( argv[0], "error when opening latex file\n", 0 );
	return( 1 );
      }
      _print_weights_list_in_latex( flatex, &wl, &vl );
      fclose( flatex );
    }
    
    if ( imask < 0 || imask >= wl.n_weights )
      imask = wl.n_weights - 1;
    
    for ( i=0; i<vl.n_vectors; i++ )
      vl.allocatedVectors[i].w = wl.allocatedWeights[imask].w[i];
    btxt = _build_buffer_mask( dims, &vl, &cl, voxel_size, anisotropy_y, anisotropy_z );

    
    if ( ntxt != NULL ) {
      sprintf( nfile, "%s.info", ntxt );
    }
    if ( nimage != NULL ) {
      sprintf( nfile, "%s.info", nimage );
    }
    
    if ( ntxt != NULL || nimage != NULL ) {
      finfo = fopen( nfile, "w" );
      if ( finfo == NULL ) {
	usage( argv[0], "error when opening info file\n", 0 );
	return( 1 );
      }
      fprintf( finfo, "epsilon = %9.6f (integer distance has to be divided by this value)\n", 
	       wl.allocatedWeights[imask].epsilon );
      fprintf( finfo, "theoretical maximal error = %9.6f %%\n", 
	       wl.allocatedWeights[imask].error * 100.0 );
    }

    if ( ntxt != NULL ) {
      ftxt = fopen( ntxt, "w" );
      if ( ftxt == NULL ) {
	usage( argv[0], "error when opening text file\n", 0 );
	return( 1 );
      }
      _print_mask( ftxt, btxt, dims );
      fclose( ftxt );
    }
   
    if ( nimage != NULL ) {
      int max = 0;
      
      for ( i=0; i<dims[0]*dims[1]*dims[2]; i++ )
	if ( max < btxt[i] ) max = btxt[i];
      if ( max <= 255 ) 
	VT_InitImage( &imres, nimage, dims[0], dims[1], dims[2], (int)UCHAR );
      else if ( max <= 65535 )
	VT_InitImage( &imres, nimage, dims[0], dims[1], dims[2], (int)USHORT );
      else 
	VT_InitImage( &imres, nimage, dims[0], dims[1], dims[2], (int)INT );
      if ( VT_AllocImage( &imres ) != 1 ) {
	usage( argv[0], "error when allocating output image\n", 0 );
	return( 1 );
      }
      switch ( imres.type ) {
      default :
	usage( argv[0], "such image type not handled in switch\n", 0 );
	return( 1 );
      case UCHAR :
	{
	  u8 *buf = (u8*)imres.buf;
	  for ( i=0; i<dims[0]*dims[1]*dims[2]; i++ )
	    buf[i] = btxt[i];
	}
	break;
      case USHORT :
	{
	  u16 *buf = (u16*)imres.buf;
	  for ( i=0; i<dims[0]*dims[1]*dims[2]; i++ )
	    buf[i] = btxt[i];
	}
	break;
      case SINT :
	{
	  s32 *buf = (s32*)imres.buf;
	  for ( i=0; i<dims[0]*dims[1]*dims[2]; i++ )
	    buf[i] = btxt[i];
	}
	break;
      }
       if ( VT_WriteInrimage( &imres ) == -1 ) {
	 usage( argv[0], "unable to write output image\n", 0 );
	 return( 1 );
       }
       VT_FreeImage( &imres );
    }
    

 
    if ( btxt != NULL ) free( btxt );
    
  }
  

  _free_cone_list( &cl );
  _free_vector_list( &vl );
  _free_weights_list( &wl );

  if ( nout != NULL ) {
    fclose( fout );
  }

  return( 0 );
  }
