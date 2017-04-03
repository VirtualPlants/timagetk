
#include <vt_histo-seuil.h>

/*--- ---*/
int VT_SeuilHisto( vt_histo *histo, int choix, int *seuil )
{
    e_Histo h;
    int i, min, max;

    *seuil = 0;

    E_H_InitHisto( &h );
    switch ( histo->type ) {
    case UCHAR :
	h.type = TYPEIM_256;
	break;
    case USHORT :
	h.type = TYPEIM_16B;
	break;
    case SSHORT :
	h.type = TYPEIM_S16B;
	break;
    default :
	VT_Error( "unknown histogram type", "VT_SeuilHisto" );
	return( 0 );
    }
    
    E_H_AllocHisto( &h );
    for ( i = 0; i < histo->size; i ++ ) 
	h.buf[i] = (int)histo->buf[i];
    E_H_HistoMinMax( &h, &min, &max );

    if ( E_H_HistoSeuil( &h, choix, seuil ) != TRUE ) {
	E_H_FreeHisto( &h );
	VT_Error( "unable to compute threshold", "VT_SeuilHisto" );
	return( 0 );
    }
    E_H_FreeHisto( &h );
    if ( (*seuil < histo->min) || (*seuil > histo->max) ) {
	VT_Error( "threshold outside bounds", "VT_SeuilHisto" );
	*seuil = 0;
	return( 0 );
    }
    
    return( 1 );
}
