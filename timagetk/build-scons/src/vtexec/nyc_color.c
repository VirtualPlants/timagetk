#include <vt_common.h>

#define _MULT_DIFF_WITH_FIRST_ 1
#define _MULT_DIFF_WITH_LAST_ 2
#define _ABSOLUTE_DIFF_WITH_FIRST_ 3
#define _ABSOLUTE_DIFF_WITH_LAST_ 4


typedef struct local_par {
  vt_names names;
  char name_mask[STRINGLENGTH];
  int is_mask;
  char name_ref[STRINGLENGTH];
  int is_ref;
  char name_tc[STRINGLENGTH];
  int is_tc;
  int max;
  int type_computation;
  double coeff;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [image-out] [-mask image-mask] [-ref image-ref]\n\
\t [-tc image-tc] [-mult | {-abs|-absolute}] [-last | -first] [-max %d]\n\
\t [-inv] [-swap] [-v] [-D] [-help] [options-de-type]";

static char *detail = "\
\t si 'image-in' est '-', on prendra stdin\n\
\t si 'image-out' est absent, on prendra stdout\n\
\t si les deux sont absents, on prendra stdin et stdout\n\
\t -inv : inverse 'image-in'\n\
\t -swap : swap 'image-in' (si elle est codee sur 2 octets)\n\
\t -v : mode verbose\n\
\t -D : mode debug\n\
\t options-de-type : -o 1    : unsigned char\n\
\t                   -o 2    : unsigned short int\n\
\t                   -o 2 -s : short int\n\
\t                   -o 4 -s : int\n\
\t                   -r      : float\n\
\t si aucune de ces options n'est presente, on prend le type de 'image-in'\n";
static char program[STRINGLENGTH];



int main( int argc, char *argv[] )
{
  local_par par;
  vt_image *image, imred, imblue, imgreen, *itc, *imask = (vt_image *)NULL, *iref = (vt_image *)NULL;
  
  register int x, y, z, c;
  register u8 ***in;
  register u8 ***red;
  register u8 ***gre;
  register u8 ***blu;
  register u8 ***mask = (u8 ***)NULL;
  register u8 ***ref = (u8 ***)NULL;
  register u8 ***tc;
  register int diff;
  
  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /* tc ? */
  if ( par.is_tc != 1 )
    VT_ErrorParse("no color table image\n", 0);
  
  itc = _VT_Inrimage( par.name_tc );
  if ( itc == (vt_image*)NULL ) {
    VT_ErrorParse("unable to read tc image\n", 0);
  }
  if ( (itc->type != UCHAR) && (itc->dim.x != 256) && (itc->dim.y != 3) ) {
    VT_ErrorParse("unable to deal with such ref image type\n", 0);
  }

  /*--- lecture de l'image d'entree ---*/
  image = _VT_Inrimage( par.names.in );
  if ( image == (vt_image*)NULL ) 
    VT_ErrorParse("unable to read input image\n", 0);
  
  if ( image->type != UCHAR ) {
    VT_ErrorParse("unable to deal with such image type\n", 0);
  }
  
  /* mask ? */
  if ( par.is_mask == 1 ) {
    imask = _VT_Inrimage( par.name_mask );
    if ( imask == (vt_image*)NULL ) {
      VT_ErrorParse("unable to read mask image\n", 0);
    }
    if ( imask->type != UCHAR ) {
      VT_ErrorParse("unable to deal with such mask image type\n", 0);
    }
  }

  /* ref ? */
  if ( par.is_ref == 1 ) {
    iref = _VT_Inrimage( par.name_ref );
    if ( iref == (vt_image*)NULL ) {
      VT_ErrorParse("unable to read ref image\n", 0);
    }
    if ( iref->type != UCHAR ) {
      VT_ErrorParse("unable to deal with such ref image type\n", 0);
    }
  }

  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.inv == 1 )  VT_InverseImage( image );
  if ( par.names.swap == 1 ) VT_SwapImage( image );
  
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &imred );
  VT_InitImage( &imred, strcat( strdup(par.names.out), ".red") , image->dim.x, image->dim.y, image->dim.z, UCHAR );
  if ( VT_AllocImage( &imred ) != 1 ) {
    VT_ErrorParse("unable to allocate red output image\n", 0);
  }
  VT_Image( &imblue );
  VT_InitImage( &imblue, strcat( strdup(par.names.out), ".blue") , image->dim.x, image->dim.y, image->dim.z, UCHAR );
  if ( VT_AllocImage( &imblue ) != 1 ) {
    VT_ErrorParse("unable to allocate blue output image\n", 0);
  }
  VT_Image( &imgreen );
  VT_InitImage( &imgreen, strcat( strdup(par.names.out), ".green") , image->dim.x, image->dim.y, image->dim.z, UCHAR );
  if ( VT_AllocImage( &imgreen ) != 1 ) {
    VT_ErrorParse("unable to allocate green output image\n", 0);
  }
  
  /* --- */
  in = (u8 ***)(image->array);
  tc = (u8 ***)(itc->array);
  red = (u8 ***)(imred.array);
  gre = (u8 ***)(imgreen.array);
  blu = (u8 ***)(imblue.array);

  if ( par.is_ref == 1 ) {
    ref = (u8 ***)(iref->array);
    if ( par.is_mask == 1 ) {
      mask = (u8 ***)(imask->array);
    }

    switch ( par.type_computation ) {
    case _MULT_DIFF_WITH_FIRST_ :
    default :
      for ( z =0; z < image->dim.z; z ++ ) 
      for ( y =0; y < image->dim.y; y ++ ) 
      for ( x =0; x < image->dim.x; x ++ ) {
	red[z][y][x] = gre[z][y][x] = blu[z][y][x] = ref[0][y][x];
	if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	diff = (int)in[z][y][x] - (int)ref[0][y][x];
	c = (int)((double)diff * 128.0 / (double)par.max + 0.5 ) + 127;
	if (c < 0) c = 0;
	if (c > 255) c = 255;
	if (diff < 0.0 ) diff *= -1.0;
	red[z][y][x] += (int)((double)diff * (double)tc[0][0][c] / (double)255.0 + 0.5);
	gre[z][y][x] += (int)((double)diff * (double)tc[0][1][c] / (double)255.0 + 0.5);
	blu[z][y][x] += (int)((double)diff * (double)tc[0][2][c] / (double)255.0 + 0.5);
      }
      break;
    case _MULT_DIFF_WITH_LAST_ :
      for ( z =0; z < image->dim.z; z ++ ) 
      for ( y =0; y < image->dim.y; y ++ ) 
      for ( x =0; x < image->dim.x; x ++ ) {
	red[z][y][x] = gre[z][y][x] = blu[z][y][x] = in[z][y][x];
	if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	diff = (int)ref[0][y][x] - (int)in[z][y][x];
	c = (int)((double)diff * 128.0 / (double)par.max + 0.5 ) + 127;
	if (c < 0) c = 0;
	if (c > 255) c = 255;
	if (diff < 0.0 ) diff *= -1.0;
	red[z][y][x] += (int)((double)diff * (double)tc[0][0][c] / (double)255.0 + 0.5);
	gre[z][y][x] += (int)((double)diff * (double)tc[0][1][c] / (double)255.0 + 0.5);
	blu[z][y][x] += (int)((double)diff * (double)tc[0][2][c] / (double)255.0 + 0.5);
      }
      break;
    case _ABSOLUTE_DIFF_WITH_FIRST_ :
      for ( z =0; z < image->dim.z; z ++ ) 
      for ( y =0; y < image->dim.y; y ++ ) 
      for ( x =0; x < image->dim.x; x ++ ) {
	red[z][y][x] = gre[z][y][x] = blu[z][y][x] = in[z][y][x];
	if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	diff = (int)in[z][y][x] - (int)ref[0][y][x];
	c = (int)((double)diff * 128.0 / (double)par.max + 0.5 ) + 127;
	if (c < 0) c = 0;
	if (c > 255) c = 255;
	if ( (tc[0][0][c] == 0) && (tc[0][1][c] == 0) && (tc[0][2][c] == 0) ) continue;
	red[z][y][x] = tc[0][0][c];
	gre[z][y][x] = tc[0][1][c];
	blu[z][y][x] = tc[0][2][c];
      }
      break;
    case _ABSOLUTE_DIFF_WITH_LAST_ :
      for ( z =0; z < image->dim.z; z ++ ) 
      for ( y =0; y < image->dim.y; y ++ ) 
      for ( x =0; x < image->dim.x; x ++ ) {
	red[z][y][x] = gre[z][y][x] = blu[z][y][x] = in[z][y][x];
	if ( (par.is_mask == 1) && (mask[0][y][x] == 0) ) continue;
	diff = (int)ref[0][y][x] - (int)in[z][y][x];
	c = (int)((double)diff * 128.0 / (double)par.max + 0.5 ) + 127;
	if (c < 0) c = 0;
	if (c > 255) c = 255;
	if ( (tc[0][0][c] == 0) && (tc[0][1][c] == 0) && (tc[0][2][c] == 0) ) continue;
	red[z][y][x] = tc[0][0][c];
	gre[z][y][x] = tc[0][1][c];
	blu[z][y][x] = tc[0][2][c];
      }
    }

  }

  /*--- ecriture de l'image resultat ---*/
  (void)VT_WriteInrimage( &imred );
  (void)VT_WriteInrimage( &imgreen );
  (void)VT_WriteInrimage( &imblue );
  
  return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  char text[STRINGLENGTH];
  int is_mult = 1;
  int is_first = 1;
  
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
      /*--- arguments generaux ---*/
      else if ( strcmp ( argv[i], "-help" ) == 0 ) {
	VT_ErrorParse("\n", 1);
      }
      else if ( strcmp ( argv[i], "-v" ) == 0 ) {
	_VT_VERBOSE_ = 1;
      }
      else if ( strcmp ( argv[i], "-D" ) == 0 ) {
	_VT_DEBUG_ = 1;
      }
      /*--- mask ---*/
      else if ( strcmp ( argv[i], "-mask" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -mask...\n", 0 );
	strncpy( par->name_mask, argv[i], STRINGLENGTH );  
	par->is_mask = 1;
      }
      /*--- ref ---*/
      else if ( strcmp ( argv[i], "-ref" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -ref...\n", 0 );
	strncpy( par->name_ref, argv[i], STRINGLENGTH );  
	par->is_ref = 1;
      }
      /*--- tc ---*/
      else if ( strcmp ( argv[i], "-tc" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -tc...\n", 0 );
	strncpy( par->name_tc, argv[i], STRINGLENGTH );  
	par->is_tc = 1;
      }
      /*--- ---*/
      else if ( strcmp ( argv[i], "-mult" ) == 0 ) {
	is_mult = 1;
      }
      else if ( strcmp ( argv[i], "-abs" ) == 0 ) {
	is_mult = 0;
      }
      else if ( strcmp ( argv[i], "-absolute" ) == 0 ) {
	is_mult = 0;
      }
      else if ( strcmp ( argv[i], "-first" ) == 0 ) {
	is_first = 1;
      }
      else if ( strcmp ( argv[i], "-last" ) == 0 ) {
	is_first = 0;
      }
      /*--- coeff ---*/
      else if ( strcmp ( argv[i], "-coeff" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -coeff...\n", 0 );
	status = sscanf( argv[i],"%lf",&(par->coeff) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -coeff...\n", 0 );
      }
      /*--- max ---*/
      else if ( strcmp ( argv[i], "-max" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -max...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->max) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -max...\n", 0 );
      }
      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }
      /*--- option inconnue ---*/
      else {
	sprintf(text,"unknown option %s\n",argv[i]);
	VT_ErrorParse(text, 0);
      }
    }
    /*--- saisie des noms d'images ---*/
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
  
  /* type de calcul */
  if ( is_first == 1 )
    par->type_computation = ( is_mult == 1 ) ? _MULT_DIFF_WITH_FIRST_ : _ABSOLUTE_DIFF_WITH_FIRST_;
  else
    par->type_computation = ( is_mult == 1 ) ? _MULT_DIFF_WITH_LAST_ : _ABSOLUTE_DIFF_WITH_LAST_;
  
  /*--- s'il n'y a pas assez de noms ... ---*/
  if (nb == 0) {
    strcpy( par->names.in,  "<" );  /* standart input */
    strcpy( par->names.out, ">" );  /* standart output */
  }
  if (nb == 1)
    strcpy( par->names.out, ">" );  /* standart output */
  
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
  par->type_computation = TYPE_UNKNOWN;
  par->is_mask = 0;
  par->is_ref = 0;
  par->is_tc = 0;
  par->max = 128;
  par->coeff = 0.0;
}
