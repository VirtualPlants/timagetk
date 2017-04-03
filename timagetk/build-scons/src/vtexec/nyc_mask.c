#include <vt_common.h>

typedef struct local_par {
  vt_names names;
  vt_ipt dim;
  vt_ipt som;
  vt_ipt pt1;
  vt_ipt pt2;
  int type;
} local_par;

/*------- Definition des fonctions statiques ----------*/

static void VT_Parse( int argc, char *argv[], local_par *par );
static void VT_ErrorParse( char *str, int l );
static void VT_InitParam( local_par *par );



static char *usage = "[image-in] [-x %d] [-y %d]\n\
\t [-som %d %d] [-pt1 %d %d] [-pt2 %d %d]\n\
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
  vt_image image;
  register int x, y;
  register u8 ***buf;
  vt_ipt v1, v2, v;
  int n1, n2, n;
  double c12, c;

  /*--- initialisation des parametres ---*/
  VT_InitParam( &par );
  
  /*--- lecture des parametres ---*/
  VT_Parse( argc, argv, &par );
  
  /*--- initialisation de l'image resultat ---*/
  VT_Image( &image );
  VT_InitImage( &image, par.names.in, par.dim.x, par.dim.y, (int)1, (int)UCHAR );
  /* if ( par.type != TYPE_UNKNOWN ) image.type = par.type; */
  if ( VT_AllocImage( &image ) != 1 ) {
    VT_ErrorParse("unable to allocate output image\n", 0);
  }

  /* */
  buf = (u8 ***)(image.array);
  v1.x = par.pt1.x - par.som.x;
  v1.y = par.pt1.y - par.som.y;
  v2.x = par.pt2.x - par.som.x;
  v2.y = par.pt2.y - par.som.y;
  n1 = v1.x * v1.x + v1.y * v1.y;
  n2 = v2.x * v2.x + v2.y * v2.y;
  c12 = 0.0;
  if ( (n1 > 0) && (n2 > 0) )
    c12 = (v1.x * v2.x + v1.y * v2.y) / (sqrt( (double)n1 ) * sqrt( (double)n2 ));
  for ( y=0; y<image.dim.y; y++ )
  for ( x=0; x<image.dim.x; x++ ) {
    buf[0][y][x] = 0;
    if ( (n1 == 0) || (n2 == 0) ) continue;
    v.x = x - par.som.x;
    v.y = y - par.som.y;
    n = v.x * v.x + v.y * v.y;
    if ( n == 0 ) continue;
    if ( (n > n1) && (n > n2) ) continue;
    c = (v1.x * v.x + v1.y * v.y) / (sqrt( (double)n1 ) * sqrt( (double)n ));
    if ( c < c12 ) continue;
    c = (v.x * v2.x + v.y * v2.y) / (sqrt( (double)n ) * sqrt( (double)n2 ));
    if ( c < c12 ) continue;
    buf[0][y][x] = 255;
  }
  
  /*--- operations eventuelles sur l'image d'entree ---*/
  if ( par.names.inv == 1 )  VT_InverseImage( &image );
  
  /*--- ecriture de l'image resultat ---*/
  if ( VT_WriteInrimage( &image ) == -1 ) {
    VT_FreeImage( &image );
    VT_ErrorParse("unable to write output image\n", 0);
  }
  
  /*--- liberations memoires ---*/
  VT_FreeImage( &image );
  return( 1 );
}



static void VT_Parse( int argc, char *argv[], local_par *par )
{
  int i, nb, status;
  int o=0, s=0, r=0;
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
      /*--- traitement eventuel de l'image d'entree ---*/
      else if ( strcmp ( argv[i], "-inv" ) == 0 ) {
	par->names.inv = 1;
      }
      else if ( strcmp ( argv[i], "-swap" ) == 0 ) {
	par->names.swap = 1;
      }
      /*--- dimension ---*/
      else if ( strcmp ( argv[i], "-x" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -x...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->dim.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -x...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-y" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -y...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->dim.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -y...\n", 0 );
      }
      /*--- args ---*/
      else if ( strcmp ( argv[i], "-som" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -som...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->som.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -som...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -som...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->som.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -som...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-pt1" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -pt1...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->pt1.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -pt1...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -pt1...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->pt1.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -pt1...\n", 0 );
      }
      else if ( strcmp ( argv[i], "-pt2" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -pt2...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->pt2.x) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -pt2...\n", 0 );
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -pt2...\n", 0 );
	status = sscanf( argv[i],"%d",&(par->pt2.y) );
	if ( status <= 0 ) VT_ErrorParse( "parsing -pt2...\n", 0 );
      }
      /*--- lecture du type de l'image de sortie ---*/
      else if ( strcmp ( argv[i], "-r" ) == 0 ) {
	r = 1;
      }
      else if ( strcmp ( argv[i], "-s" ) == 0 ) {
	s = 1;
      }
      else if ( strcmp ( argv[i], "-o" ) == 0 ) {
	i += 1;
	if ( i >= argc)    VT_ErrorParse( "parsing -o...\n", 0 );
	status = sscanf( argv[i],"%d",&o );
	if ( status <= 0 ) VT_ErrorParse( "parsing -o...\n", 0 );
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
      else 
	VT_ErrorParse("too much file names when parsing\n", 0 );
    }
    i += 1;
  }
  
  /*--- s'il n'y a pas assez de noms ... ---*/
  if (nb == 0) {
    strcpy( par->names.in,  "<" );  /* standart input */
    strcpy( par->names.out, ">" );  /* standart output */
  }
  if (nb == 1)
    strcpy( par->names.out, ">" );  /* standart output */
  
  /*--- type de l'image resultat ---*/
  if ( (o == 1) && (s == 1) && (r == 0) )  par->type = SCHAR;
  if ( (o == 1) && (s == 0) && (r == 0) ) par->type = UCHAR;
  if ( (o == 2) && (s == 0) && (r == 0) ) par->type = USHORT;
  if ( (o == 2) && (s == 1) && (r == 0) )  par->type = SSHORT;
  if ( (o == 4) && (s == 1) && (r == 0) )  par->type = SINT;
  if ( (o == 0) && (s == 0) && (r == 1) )  par->type = FLOAT;
  /* if ( par->type == TYPE_UNKNOWN ) VT_Warning("no specified type", program); */
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
  par->type = TYPE_UNKNOWN;
  par->dim.x = par->dim.y = par->dim.z = 0;
  par->som.x = par->som.y = par->som.z = 0;
  par->pt1.x = par->pt1.y = par->pt1.z = 0;
  par->pt2.x = par->pt2.y = par->pt2.z = 0;
}
