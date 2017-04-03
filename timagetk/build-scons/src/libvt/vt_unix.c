/*************************************************************************
 * vt_unix.c - interfaces avec les standards unix
 *
 * $Id: vt_unix.c,v 1.4 2000/09/12 18:15:05 greg Exp $
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



#include <vt_unix.h>

/*
static int _verbose_ = 1;
*/
static int _debug_ = 0;


/* Fonction d'allocation.

   Duplication de la fonction malloc standard. On teste
   si la taille est strictement positive.

ERROR 
   Les memes que malloc.

RETURN
   (void*)NULL en cas d'erreur.
*/

void *VT_Malloc( unsigned long int size /* size of the allocation in bytes */ )
{
  void *ret =(void*)NULL;
  if ( size <= 0 ) {
    VT_Error("size less than 0","VT_Malloc");
    return( (void*)NULL );
  }
  ret = (void*)malloc( size );

  if ( _debug_ ) {
    fprintf( stderr, "VT_Malloc: allocate %lu bytes at %p\n", size, ret );
  }

  if (ret == (void*)NULL ) {
    char message[60];
    sprintf(message, "allocation failed (%lu bytes were requested)", size );
    VT_Error(message,"VT_Malloc");
    return( (void*)NULL );
  }


  return( ret );
}






/* Fonction de liberation.

   Duplication de la fonction free standard. On passe
   l'adresse du pointeur. Si celui-ci est different
   de (void*)NULL, la memoire est liberee est le 
   pointeur est mis a (void*)NULL.

   FreeType est defini dans vt_unix.h :
   pour _dec_ et _alpha_ c'est void*
   sinon c'est char*.
*/

void VT_Free( void** pt /* address of the pointer to be released */ )
{
  if ( *pt == (void*)NULL ) {
    VT_Warning("pointer already null","VT_Free");
    return;
  }
  free( (FreeType)*pt );
  *pt = (void*)NULL;
}






/* Fonction de calcul de longueur d'une chaine de caracteres.

   Duplication de la fonction strlen standard.
*/

int VT_Strlen( char *s /* pointer to the string */ )
{
  if ( s == (char*)NULL ) return( 0 );
  return( (int)strlen( s ) );
}



/* Fonction de copie de n caracteres d'une chaine dans une autre.

   Duplication de la fonction strncpy standard.
*/

void VT_Strncpy( char *s1 /* string where the copy is to be done */,
		char *s2 /* string to be copied */,
		int n /* maximum number of chars to be copied */ )
{
	char *c;
	if (n <= 0) return;
	c = strncpy( s1, s2, n );
}



/* Fonction de copie d'une chaine dans une autre.

   Duplication de la fonction strcpy standard.
*/

void VT_Strcpy( char *s1 /* string where the copy is to be done */,
		char *s2 /* string to be copied */ )
{
	char *c;
	c = strcpy( s1, s2 );
}



/* Fonction de comparaison de n caracteres de deux chaines.

   Duplication de la fonction standard strncmp.
*/

int VT_Strncmp( char *s1 /* first string */,
		char *s2  /* second string */,
		int n /* maximum number of chars to be compared */ )
{
	if (n <= 0) return( -1 );
	return( strncmp( s1, s2, n ) );
}



/* Fonction d'ouverture en lecture d'un file descriptor.

   Duplication reduite de la fonction standard open.
   Si name="<", retourne stdin.
   Sinon, on  ouvre avec 

RETURN
   Retourne -1 en cas d'erreur.

ERROR
   Le type du mode est different selon les architectures.
   Il est fixe a mode_t pour alpha (_alpha_) et sun (_sun_),
   a int pour dec (_dec_) et a int par defaut (si aucun
   des trois define (_sun, _alpha_ ou _dec_) n'a ete fait.
*/

int VT_ROpen( char *name /* file name to be opened */ )
{
	if( (name[0] == '<') && (name[1] == '\0') )
		return( 0 ); /*--- standard input ---*/
	return( open( name, (int)O_RDONLY, 0644 ) );
}



/* Fonction d'ouverture en ecriture d'un file descriptor.

   Duplication reduite de la fonction standard open.
   Si name=">", retourne stdout.
   Sinon, on  ouvre avec 

RETURN
   Retourne -1 en cas d'erreur.

ERROR
   Le type du mode est different selon les architectures.
   Il est fixe a mode_t pour alpha (_alpha_) et sun (_sun_),
   a int pour dec (_dec_) et a int par defaut (si aucun
   des trois define (_sun, _alpha_ ou _dec_) n'a ete fait.
*/
int VT_WOpen( char *name /* file name to be opened */ )
{
  if( ( name == (char*)NULL ) ||
      ( name[0] == '\0' ) ||
      ((name[0] == '>') && (name[1] == '\0')) )
		return( 1 ); /*--- standard output ---*/
	return( open( name, O_WRONLY | O_CREAT | O_TRUNC, 0644 ) );
}



/* Fonction de fermeture d'un file descriptor.

   Duplication de la fonction standard close.
   Si le file descriptor est 0, 1 ou 2 (stdin, stdout
   ou stderr), pas d'action.
*/
void VT_Close( int fd /* file descriptor to be closed */ )
{
	if ( (fd == 0) || (fd == 1) || (fd == 2) ) return;
	close( fd );
}



/* Fonction de lecture dans un file descriptor.

   Duplication de la fonction standard read.
*/
int VT_Read( int fd /* file descriptor */,
	    char *buf /* buffer to be filled */,
	    int nbytes /* number of bytes */ )
{
	int nbread, toberead;
	char *b;
	
	b = buf;
	toberead = nbytes;
	while ( (toberead > 0) && ((nbread = read( fd, b, toberead )) > 0) ) {
		toberead -= nbread;
		b += nbread;
	}

	if ( toberead > 0 ) {
	    char texte[80];
	    sprintf( texte, "%d bytes have been read instead of %d", (nbytes - toberead), nbytes );
	    VT_Error( texte, "VT_Read");
	}
	
	return( (nbytes - toberead) );
}



/* Fonction d'ecriture dans un file descriptor.

   Duplication de la fonction standard write.
*/
int VT_Write( int fd /* file descriptor */,
	    char *buf /* buffer to be written */,
	    int nbytes /* number of bytes */ )
{
	if (nbytes <= 0) return( 0 );
	return( write(fd, buf, nbytes) );
}





/* Fonction de copie de n caracteres d'une zone de memoire dans une autre.

   Duplication de la fonction memcpy standard.
*/

void VT_Memcpy( char *s1 /* memory area where the copy is to be done */,
		char *s2 /* memory area to be copied */,
		int n /* maximum number of chars to be copied */ )
{
	FreeType c;
	if (n <= 0) return;
	c = memcpy( (FreeType)s1, (FreeType)s2, n );
}
