
#include <vt_error.h>

/*------- Definition des fonctions statiques ----------*/

typeBoolean _VT_DEBUG_ = False;
typeBoolean _VT_VERBOSE_ = False;





/* Fonction d'ecriture des messages d'erreurs.

   Si la variable globale _VT_DEBUG_ est a 1,
   cette fonction ecrit le message mess en specifiant
   le nom de la procedure proc sur la sortie standard
   d'erreur (stderr).
*/

void VT_Error( char *mess /* message to be printed */,
              char *proc /* name of the calling procedure */ )
{
    if ( _VT_DEBUG_ == False ) return;
    if ( fprintf(stderr,"Error: %s in %s.\n", mess, proc) == EOF ) {
	if ( fprintf(stderr,"Error when writing on stderr.\n") == EOF ) {
	    if ( fprintf(stdout,"Unable to write on stderr.\n") == EOF ) {
		; /* What can I do ? */
	    }
	}
    }
}





/* Fonction d'ecriture des messages de warning.
   
   Si la variable globale _VT_DEBUG_ est a 1,
   cette fonction ecrit le message mess en specifiant
   le nom de la procedure proc sur la sortie standard
   d'erreur (stderr).
*/

void VT_Warning( char *mess /* message to be printed */,
		char *proc /* name of the calling procedure */ )
{
    if ( _VT_DEBUG_ == False ) return;
    if ( fprintf(stderr,"Warning: %s in %s.\n", mess, proc) == EOF ) {
	if ( fprintf(stderr,"Error when writing on stderr.\n") == EOF ) {
	    if ( fprintf(stdout,"Unable to write on stderr.\n") == EOF ) {
		; /* What can I do ? */
	    }
	}
    }
}





/* Fonction d'ecriture de messages d'information.

   Si la variable globale _VT_DEBUG_ est a 1,
   cette fonction ecrit le message mess en specifiant
   le nom de la procedure proc sur la sortie standard
   d'erreur (stderr).
*/

void VT_Message( char *mess /* message to be printed */,
		char *proc /* name of the calling procedure */ )
{
    if ( (_VT_DEBUG_ == False) && (_VT_VERBOSE_ == False) ) return;
    if ( fprintf(stderr,"Message: %s in %s.\n", mess, proc) == EOF ) {
	if ( fprintf(stderr,"Error when writing on stderr.\n") == EOF ) {
	    if ( fprintf(stdout,"Unable to write on stderr.\n") == EOF ) {
		; /* What can I do ? */
	    }
	}
    }
}
