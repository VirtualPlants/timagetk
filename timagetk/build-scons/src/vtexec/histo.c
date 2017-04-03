/*************************************************************************
 * histo.c -
 *
 * $Id: histo.c,v 1.16 2002/12/11 12:04:03 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * 
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <string.h>

#include <histogram.h>

#include <vt_common.h>
#include <vt_histo-tools.h>


typedef enum {
  _HISTOGRAM_,
  _CUMULATIVE_,
  _PDF_HISTOGRAM_,
  _PDF_CUMULATIVE_,
  _ICAL_,
  _ICAL_ECT_
} enumComputation;




typedef struct local_par {
  vt_names names;
  enumComputation typeComputation;
  enumHistogramFile typeOutput;

  float binlength;
  char *description;

  char *real_transformation_name;
  char *voxel_transformation_name;

} local_par;



/*-------Definition des fonctions statiques----------*/
static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );

static int _readMatrice( char *name, double *mat );
static void _changeMatFromRealUnitToVoxelUnit( double *origVoxelSize,
					       double *destVoxelSize,
					       double *mat );



static char *usage = "[image-in] [file-out] [-mask %s]\n\
 [-transformation |-trsf %s] [-voxel-transformation |-voxel-trsf %s]\n\
 [-ical | -ical-ect] [-histo | -cumul] [-pdf]\n\
 [-matlab | -scilab | -image | -txt %s] [-stderr | -stdout]\n\
 [-desc %s]\n\
 [-bin %f]\n\
 [-inv] [-swap] [-v] [-D] [-help]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\n\
 $Revision: 1.16 $ $Date: 2002/12/11 12:04:03 $ $Author: greg $\n";


static char program[STRINGLENGTH];






int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image  = (vt_image*)NULL;
  vt_image *immask = (vt_image*)NULL;

  double mat[16];
  double voxelsize[3];
  double *theMat = (double*)NULL;

  vt_3m m;
  typeHistogram histo, pdfhisto, cumul;
  typeHistogram *theHisto;
  FILE *f;
  
  int theDim[3];

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );
  

  /* is there a transformation ?
   */
  if ( par.real_transformation_name != (char*)NULL ) {
    if ( _readMatrice( par.real_transformation_name, mat ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read real matrice\n", 0);
    }
    voxelsize[0] = image->siz.x;
    voxelsize[1] = image->siz.y;
    voxelsize[2] = image->siz.z;
    /* assume that the result image has the same geometry
       (voxel size and dimensions) than the input image
    */
    _changeMatFromRealUnitToVoxelUnit( voxelsize, voxelsize, mat );
    theMat = mat;
  }
  else if ( par.voxel_transformation_name != (char*)NULL ) {
    if ( _readMatrice( par.voxel_transformation_name, mat ) != 1 ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read voxel matrice\n", 0);
    }
    theMat = mat;
  }



  /* is there a mask ?
   */
  if ( par.names.ext[0] != '\0' ) {
    immask = _VT_Inrimage( par.names.ext );
    if ( immask == (vt_image*)NULL ) {
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to read mask image\n", 0);
    }
  }



  switch ( par.typeComputation ) {
  default :
    VT_FreeImage( image );
    VT_Free( (void**)&image );
    VT_ErrorParse( "computation type not handled yet\n", 0);

  case _ICAL_ :
  case _ICAL_ECT_ :

    if ( VT_3m( image, immask, &m ) == -1 ) {
      if ( immask != (vt_image*)NULL ) {
	VT_FreeImage( immask );
	VT_Free( (void**)&immask );
      }
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse("unable to compute minimum, mean and maximum\n", 0);
    }

    switch ( par.typeOutput ) {
    default :
      if ( immask != (vt_image*)NULL ) {
	VT_FreeImage( immask );
	VT_Free( (void**)&immask );
      }
      VT_FreeImage( image );
      VT_Free( (void**)&image );
      VT_ErrorParse( "output type not handled yet\n", 0);
    case _TXT_ :
      f = fopen( par.names.out, "w" );
      if ( f == (FILE*)NULL ) {
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to open output file\n", 0);
      }
    case _STDERR_ :
      f = stderr;
      break;
    case _STDOUT_ :
      f = stdout;
      break;
    }
    
    fprintf( f, "%17.6f %17.6f", m.min, m.moy );
    if ( par.typeComputation == _ICAL_ECT_ ) 
      fprintf( f, " +/- %-17.6f", m.ect );
    fprintf( f, " %17.6f\n", m.max );

    if ( par.typeOutput == _TXT_ )
      fclose( f );

    break;

  case _HISTOGRAM_ :
  case _PDF_HISTOGRAM_ :
  case _CUMULATIVE_ :
  case _PDF_CUMULATIVE_ :

    theDim[0] = image->dim.x * image->dim.v;
    theDim[1] = image->dim.y;
    theDim[2] = image->dim.z;

    if ( _VT_DEBUG_ )
      fprintf( stderr, "%s: histogram allocation and filling\n", program );

    initHistogram( &histo );
    if ( immask == (vt_image*)NULL ) {
      if ( alloc1DHistogramFromImageAndBinLength( &histo, image->buf, image->type, 
						  (void*)NULL, TYPE_UNKNOWN,
						  theMat,
						  theDim, 
						  par.binlength ) != 1 ) {
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "error when calculating histogram\n", 0);
      }
    }
    else {
      if ( alloc1DHistogramFromImageAndBinLength( &histo, image->buf, image->type,
						  immask->buf, immask->type,
						  theMat,
						  theDim, 
						  par.binlength ) != 1 ) {
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	  VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "error when calculating histogram\n", 0);
      }
    }

    theHisto = &histo;

    if ( par.typeComputation == _PDF_HISTOGRAM_ || par.typeComputation == _PDF_CUMULATIVE_ ) {

      if ( _VT_DEBUG_ )
	fprintf( stderr, "%s: pdf allocation\n", program );
      
      initHistogram( &pdfhisto );
      if ( allocHistogramFromHistogram( &pdfhisto, &histo, FLOAT ) != 1 ) {
	freeHistogram( &histo );
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	  VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to allocate pdf histogram\n", 0);
      }

      if ( _VT_DEBUG_ )
	fprintf( stderr, "%s: pdf filling\n", program );

      if ( pdfHistogram( &pdfhisto, &histo ) != 1 ) {
	freeHistogram( &pdfhisto );
	freeHistogram( &histo );
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	  VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to compute pdf histogram\n", 0);
      }
      freeHistogram( theHisto );
      theHisto = &pdfhisto;

    }
    
    if ( par.typeComputation == _CUMULATIVE_ || par.typeComputation == _PDF_CUMULATIVE_ ) {

      if ( _VT_DEBUG_ )
	fprintf( stderr, "%s: cumulative histogram allocation\n", program );

      initHistogram( &cumul );
      if ( allocHistogramFromHistogram( &cumul, theHisto, FLOAT ) != 1 ) {
	freeHistogram( theHisto );
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	  VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to allocate cumulative histogram\n", 0);
      }

      if ( _VT_DEBUG_ )
	fprintf( stderr, "%s: cumulative histogram filling\n", program );

      if ( cumulative1DHistogram( &cumul, theHisto) != 1 ) {
	freeHistogram( &cumul );
	freeHistogram( theHisto );
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	  VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to compute cumulative histogram\n", 0);
      }
      freeHistogram( theHisto );
      theHisto = &cumul;

    }


    switch ( par.typeOutput ) {
    default :
      if ( par.names.out != (char*)NULL && par.names.out[0] != '\0' ) {
	if ( VT_WriteHistogram( par.names.out, theHisto, par.description ) != 1 ) {
	  freeHistogram( theHisto );
	  if ( immask != (vt_image*)NULL ) {
	    VT_FreeImage( immask );
	    VT_Free( (void**)&immask );
	  }
	  VT_FreeImage( image );
	  VT_Free( (void**)&image );
	  VT_ErrorParse( "unable to write generic output\n", 0);
	}
      }
      else {
	freeHistogram( theHisto );
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	  VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "output type not handled yet\n", 0);
      }
      break;
    case _IMAGE_ :
      if ( VT_WriteHistogramAsImage( par.names.out, theHisto ) != 1 ) {
	freeHistogram( theHisto );
	if ( immask != (vt_image*)NULL ) {
	  VT_FreeImage( immask );
	  VT_Free( (void**)&immask );
	}
	VT_FreeImage( image );
	VT_Free( (void**)&image );
	VT_ErrorParse( "unable to write image output\n", 0);
      }
      break;
    case _MATLAB_ :
      writeHistogramMatlab( par.names.out, theHisto, par.description );
      break;
    case _SCILAB_ :
      writeHistogramScilab( par.names.out, theHisto, par.description );
      break;
    case _TXT_ :
      write1DHistogramTxt( par.names.out, theHisto );
      break;
    case _STDERR_ :
      fprintf1DHistogram( stderr, theHisto );
      break;
    case _STDOUT_ :
      fprintf1DHistogram( stdout, theHisto );
      break;
    }
    freeHistogram( theHisto );
    break;
  }

  if ( immask != (vt_image*)NULL ) {
    VT_FreeImage( immask );
    VT_Free( (void**)&immask );
  }
  VT_FreeImage( image );
  VT_Free( (void**)&image ); 

  return( 1 );
}






static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb;
  int status;
  char text[STRINGLENGTH];
  
  if ( VT_CopyName( program, argv[0] ) != 1 )
    VT_Error("Error while copying program name", (char*)NULL);
  if ( argc == 1 ) VT_ErrorParse("\n", 0 );
  
  /*--- lecture des parametres ---*/
  i = 1; nb = 0;
  while ( i < argc ) {
    if ( argv[i][0] == '-' ) {
      if ( argv[i][1] == '\0' ) {
	if ( nb == 0 ) {
	  /*--- standart input ---*/
	  strcpy( par->names.in, "<" );
	  nb += 1;
	}
      }
      else if ( strcmp ( argv[i], "-help" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
	_VT_VERBOSE_ = 1;
	incrementVerboseInHistogram( );
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 ) {
	if ( _VT_DEBUG_ <= 0 ) 
	  _VT_DEBUG_ = 1;
	else
	  _VT_DEBUG_ ++;
      }

      /*--- type output ---*/

      else if ( strcmp ( argv[i], "-ical" ) == 0 ) {
	par->typeComputation = _ICAL_;
	par->typeOutput = _STDOUT_;
      }

      else if ( strcmp ( argv[i], "-ical-ect" ) == 0 ) {
	par->typeComputation = _ICAL_ECT_;
	par->typeOutput = _STDOUT_;
      }
      else if ( strcmp ( argv[i], "-histo" ) == 0 ) {
	if ( par->typeComputation == _PDF_CUMULATIVE_ )
	  par->typeComputation = _PDF_HISTOGRAM_;
	else
	  par->typeComputation = _HISTOGRAM_;
      }

      else if ( strcmp ( argv[i], "-cumul" ) == 0 ) {
	if ( par->typeComputation == _PDF_HISTOGRAM_ )
	  par->typeComputation = _PDF_CUMULATIVE_;
	else
	  par->typeComputation = _CUMULATIVE_;
      }

      else if ( strcmp ( argv[i], "-pdf" ) == 0 ) {
	if ( par->typeComputation == _CUMULATIVE_ )
	  par->typeComputation = _PDF_CUMULATIVE_;
	else
	  par->typeComputation = _PDF_HISTOGRAM_;
      }


      else if ( strcmp ( argv[i], "-matlab" ) == 0 ) {
	par->typeOutput = _MATLAB_;
      }
      else if ( strcmp ( argv[i], "-scilab" ) == 0 ) {
	par->typeOutput = _SCILAB_;
      }
      else if ( strcmp ( argv[i], "-image" ) == 0 ) {
	par->typeOutput = _IMAGE_;
      }
      else if ( strcmp ( argv[i], "-stdout" ) == 0 ) {
	par->typeOutput = _STDOUT_;
      }
      else if ( strcmp ( argv[i], "-stderr" ) == 0 ) {
	par->typeOutput = _STDERR_;
      }
      else if ( strcmp ( argv[i], "-txt" ) == 0 ) {
	par->typeOutput = _TXT_;
      }

      else if ( strcmp ( argv[i], "-desc" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -desc...\n", 0 );
	par->description = argv[i];
      }

      else if ( strcmp ( argv[i], "-bin" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -bin...\n", 0 );
	status = sscanf( argv[i], "%f", &(par->binlength) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -bin...\n", 0 );
      }
      
      /* image related stuff
       */

      else if ( strcmp ( argv[i], "-mask" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -mask...\n", 0 );
	strncpy( par->names.ext, argv[i], STRINGLENGTH );
      }

      else if ( strcmp ( argv[i], "-transformation") == 0
		|| (strcmp ( argv[i], "-trsf") == 0 && argv[i][5] == '\0') ) {
	i++;
	if ( i >= argc) VT_ErrorParse( "parsing -transformation", 0 );
	par->real_transformation_name = argv[i];
      }
      
      else if ( strcmp ( argv[i], "-voxel-transformation") == 0
		|| strcmp ( argv[i], "-voxel-trsf") == 0 ) {
	i++;
	if ( i >= argc) VT_ErrorParse( "parsing -voxel-transformation", 0 );
	par->voxel_transformation_name = argv[i];
      }



      else {
	sprintf(text,"unknown option %s\n",argv[i]);
	VT_ErrorParse(text, 0);
      }
    }

    else if ( argv[i][0] != 0 ) {
      if ( nb == 0 ) { 
	strncpy( par->names.in, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else if ( nb == 1 ) {
	strncpy( par->names.out, argv[i], STRINGLENGTH );  
	nb += 1;
      }
      else 
	VT_ErrorParse("too much file names when parsing\n", 0 );
    }
    i += 1;
  }
  if (nb == 0) {
    strcpy( par->names.in,  "<" );  /* standart input */
  }
}




static void VT_ErrorParse( char *str, int flag )
{
  (void)fprintf(stderr,"Usage : %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",detail);
  (void)fprintf(stderr,"Erreur : %s",str);
  exit(0);
}




static void VT_InitParam( local_par *par )
{
  VT_Names( &(par->names) );
  par->typeComputation = _HISTOGRAM_;
  par->typeOutput = _UNDEFINED_HISTOGRAM_FILE_;

  par->binlength = 1.0;
  par->description = (char*)NULL;

  par->real_transformation_name = NULL;
  par->voxel_transformation_name = NULL;
}








/************************************************************
 *
 ************************************************************/

/* from reech3d.c
 */
static int _readMatrice( char *name, double *mat )
{
  FILE *fopen(), *fp;
  char text[STRINGLENGTH];
  int i, nbelts = 0;
  int status;
  
  /* lecture de 4 double par ligne
     On prevoit le cas ou la ligne commence par "O8 xxxxx ...
     */

  fp = fopen( name, "r" );
  if ( fp == NULL ) return( 0 );
  
  while ( (nbelts < 16) && (fgets( text, STRINGLENGTH, fp ) != NULL) ) {
    if ( (text[0] == 'O') && (text[1] == '8') ) {
      status = sscanf( &(text[2]), "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    } else {
      status = sscanf( text, "%lf %lf %lf %lf", 
		       &mat[nbelts+0], &mat[nbelts+1],
		       &mat[nbelts+2], &mat[nbelts+3] );
    }
    if ( _VT_DEBUG_ ) {
      fprintf( stderr, "read %d elements:", status );
      for (i=0; i<status; i++ )
	fprintf( stderr, " %lf", mat[nbelts+i] );
      fprintf( stderr, "\n" );
    }
    if ( status == 4 ) nbelts += 4;
  }
  fclose( fp );

  if ( _VT_DEBUG_ == 1 ) {
    fprintf( stderr, " lecture de la matrice %s\n", name );
    fprintf( stderr, " %d elements lus\n", nbelts );
    fprintf( stderr,"   %f %f %f %f\n", mat[0], mat[1], mat[2], mat[3] );
    fprintf( stderr,"   %f %f %f %f\n", mat[4], mat[5], mat[6], mat[7] );
    fprintf( stderr,"   %f %f %f %f\n", mat[8], mat[9], mat[10], mat[11] );
    fprintf( stderr,"   %f %f %f %f\n", mat[12], mat[13], mat[14], mat[15] );
  }
  if ( nbelts == 16 ) return ( 1 );
  return( 0 );
}



/* theTrsf allows to resample 'orig' into 'dest' thus goes
   from 'dest' to 'orig'
*/
static void _changeMatFromRealUnitToVoxelUnit( double *origVoxelSize,
					       double *destVoxelSize,
					       double *mat )
{
  mat[ 0] = mat[ 0] * destVoxelSize[0] / origVoxelSize[0];
  mat[ 1] = mat[ 1] * destVoxelSize[1] / origVoxelSize[0];
  mat[ 2] = mat[ 2] * destVoxelSize[2] / origVoxelSize[0];
  mat[ 3] = mat[ 3]                    / origVoxelSize[0];
  
  mat[ 4] = mat[ 4] * destVoxelSize[0] / origVoxelSize[1];
  mat[ 5] = mat[ 5] * destVoxelSize[1] / origVoxelSize[1];
  mat[ 6] = mat[ 6] * destVoxelSize[2] / origVoxelSize[1];
  mat[ 7] = mat[ 7]                    / origVoxelSize[1];
  
  mat[ 8] = mat[ 8] * destVoxelSize[0] / origVoxelSize[2];
  mat[ 9] = mat[ 9] * destVoxelSize[1] / origVoxelSize[2];
  mat[10] = mat[10] * destVoxelSize[2] / origVoxelSize[2];
  mat[11] = mat[11]                    / origVoxelSize[2];
}
