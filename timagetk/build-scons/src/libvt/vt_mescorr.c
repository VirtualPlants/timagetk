
#include <math.h>

#include <vt_mescorr.h>

#define VT_MIN(A,B) ( (A) < (B) ? (A) : (B) )

/*------- Definition des fonctions statiques ----------*/





/* Fonction d'initialisation de la structure vt_mescorr.

   Tout est mis a 0;
*/

void VT_Mescorr( vt_mescorr *s /* structure to be initialized */ )
{
	int i;
	s->min.x = s->min.y = s->min.z = 0;
	s->max.x = s->max.y = s->max.z = 0;
	s->a = s->b = (double)0.0;
	s->si1 = s->si2 = (double)0.0;
	s->si12 = s->si22 = s->si1i2 = (double)0.0;
	s->mean = s->sigma = (double)0.0;
	for (i=0; i<STRINGLENGTH; i++)
		s->name[i] = '\0';
}





/* Fonction de mesure de correlation entre 2 images.

   On cherche une fonction lineaire entre les intensites
   des 2 images :
   (intensite image 2) = a * (intensite image 1) + b
   a et b sont des champs de la structure parametre.
   On dispose des mesures d'erreurs suivantes :
   - moyenne des distances entre un couple d'intensites (i1,i2) 
     et la droite d'interpolation : mean;
   - moyenne des distances au carre entre un couple d'intensites 
     (i1,i2) et la droite d'interpolation : sigma;

RETURN
   Retourne 1 si tout se passe bien.
*/

int VT_MesureCorrelation( vt_image *im1 /* first image */,
			 vt_image *im2 /* second image */,
			 vt_mescorr *par /* parameters */ )
{
	register int x, y, z;
	int xmin, ymin, zmin, xmax, ymax, zmax;
	int d1x, d1y, d1xy, d2x, d2y, d2xy;
	double inb, inp, n, si1, si2, si12, si22, si1i2;
	double a, b, mean, sigma;
	register double i1, i2, ssi1, ssi2, ssi12, ssi22, ssi1i2, d, smean;

	if ( VT_Test1Image( im1, "VT_MesureCorrelation" ) == -1 ) 
                return( 0 );
	if ( VT_Test1Image( im2, "VT_MesureCorrelation" ) == -1 ) 
                return( 0 );
	
	d1x = im1->dim.x;   d1y = im1->dim.y;
	d2x = im2->dim.x;   d2y = im2->dim.y;
	d1xy = d1x * d1y;   d2xy = d2x * d2y;

	/*--- verification des parametres de calcul ---*/
	xmin = ymin = zmin = 0;
	xmax = VT_MIN( (im1->dim.x - 1), (im2->dim.x - 1) );
	ymax = VT_MIN( (im1->dim.y - 1), (im2->dim.y - 1) );
	zmax = VT_MIN( (im1->dim.z - 1), (im2->dim.z - 1) );

	if ( (par->min.x < 0) || (par->min.x > xmax) || (par->min.x > par->max.x) )
		VT_Warning( "Coordinate X of the first corner has changed", "VT_MesureCorrelation");
	else xmin = par->min.x;
	if ( (par->min.y < 0) || (par->min.y > ymax) || (par->min.y > par->max.y) )
		VT_Warning( "Coordinate Y of the first corner has changed", "VT_MesureCorrelation");
	else ymin = par->min.y;
	if ( (par->min.z < 0) || (par->min.z > zmax) || (par->min.z > par->max.z) )
		VT_Warning( "Coordinate Z of the first corner has changed", "VT_MesureCorrelation");
	else zmin = par->min.z;
	if ( (par->max.x < 0) || (par->max.x > xmax) )
		VT_Warning( "Coordinate X of the second corner has changed", "VT_MesureCorrelation");
	else xmax = par->max.x;
	if ( (par->max.y < 0) || (par->max.y > ymax) )
		VT_Warning( "Coordinate Y of the second corner has changed", "VT_MesureCorrelation");
	else ymax = par->max.y;
	if ( (par->max.z < 0) || (par->max.z > zmax) )
		VT_Warning( "Coordinate Z of the second corner has changed", "VT_MesureCorrelation");
	else zmax = par->max.z;
	
	/*--- initialisation ---*/
	inb = ((double)1.0) / ((double)( (xmax-xmin+1) * (ymax-ymin+1) ));
	inp = ((double)1.0) / ((double)( (zmax-zmin+1) ));
	n = (double)( (xmax-xmin+1) * (ymax-ymin+1) * (zmax-zmin+1) );
	si1 = si2 = si12 = si22 = si1i2 = (double)0.0;
	mean = sigma = (double)0.0;
	a = b = (double)0.0;
	
	switch ( im1->type ) {
	case UCHAR :
		{
		unsigned char *buf1;
		buf1 = (unsigned char*)(im1->buf);
		switch ( im2->type ) {
			case UCHAR :
				{
				unsigned char *buf2;
				buf2 = (unsigned char*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SSHORT :
				{
				short int *buf2;
				buf2 = (short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case USHORT :
				{
				unsigned short int *buf2;
				buf2 = (unsigned short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SINT :
				{
				int *buf2;
				buf2 = (int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case FLOAT :
				{
				float *buf2;
				buf2 = (float*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			default :
				VT_Error( "image type unknown or not supported", "VT_MesureCorrelation" );
				return( -1 );
		}}
		break;
	case SSHORT :
		{
		short int *buf1;
		buf1 = (short int*)(im1->buf);
		switch ( im2->type ) {
			case UCHAR :
				{
				unsigned char *buf2;
				buf2 = (unsigned char*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SSHORT :
				{
				short int *buf2;
				buf2 = (short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case USHORT :
				{
				unsigned short int *buf2;
				buf2 = (unsigned short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SINT :
				{
				int *buf2;
				buf2 = (int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case FLOAT :
				{
				float *buf2;
				buf2 = (float*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			default :
				VT_Error( "image type unknown or not supported", "VT_MesureCorrelation" );
				return( -1 );
		}}
		break;
	case USHORT :
		{
		unsigned short int *buf1;
		buf1 = (unsigned short int*)(im1->buf);
		switch ( im2->type ) {
			case UCHAR :
				{
				unsigned char *buf2;
				buf2 = (unsigned char*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SSHORT :
				{
				short int *buf2;
				buf2 = (short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case USHORT :
				{
				unsigned short int *buf2;
				buf2 = (unsigned short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SINT :
				{
				int *buf2;
				buf2 = (int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case FLOAT :
				{
				float *buf2;
				buf2 = (float*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			default :
				VT_Error( "image type unknown or not supported", "VT_MesureCorrelation" );
				return( -1 );
		}}
		break;
	case SINT :
		{
		int *buf1;
		buf1 = (int*)(im1->buf);
		switch ( im2->type ) {
			case UCHAR :
				{
				unsigned char *buf2;
				buf2 = (unsigned char*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SSHORT :
				{
				short int *buf2;
				buf2 = (short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case USHORT :
				{
				unsigned short int *buf2;
				buf2 = (unsigned short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SINT :
				{
				int *buf2;
				buf2 = (int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case FLOAT :
				{
				float *buf2;
				buf2 = (float*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			default :
				VT_Error( "image type unknown or not supported", "VT_MesureCorrelation" );
				return( -1 );
		}}
		break;
	case FLOAT :
		{
		float *buf1;
		buf1 = (float*)(im1->buf);
		switch ( im2->type ) {
			case UCHAR :
				{
				unsigned char *buf2;
				buf2 = (unsigned char*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SSHORT :
				{
				short int *buf2;
				buf2 = (short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case USHORT :
				{
				unsigned short int *buf2;
				buf2 = (unsigned short int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case SINT :
				{
				int *buf2;
				buf2 = (int*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			case FLOAT :
				{
				float *buf2;
				buf2 = (float*)(im2->buf);
				/*--- calcul des diverses sommes ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					ssi1 = ssi2 = ssi12 = ssi22 = ssi1i2 = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						ssi1 += i1;   ssi2 += i2;
						ssi12 += (i1 * i1);   ssi22 += (i2 * i2);  
						ssi1i2 += (i1 * i2);
					}
					si1 += ( ssi1 * inb );    si2 += ( ssi2 * inb ); 
					si12 += ( ssi12 * inb );    si22 += ( ssi22 * inb );
					si1i2 += ( ssi1i2 * inb );
				}
				si1 *= inp;    si2 *= inp; 
				si12 *= inp;    si22 *= inp;   si1i2 *= inp;
				/*--- calcul des parametres de l'approximation ---*/
				a = ( si1i2 - n * si1 * si2 ) / ( si12 - n * si1 * si1 );
				b = n * ( si2 * si12 - si1 * si1i2 ) / ( si12 - n * si1 * si1 );
				sigma = si22 + a * a * si12 + b * b - 2.0 * a * si1i2;
				sigma += - 2.0 * b * si2 + 2.0 * a * b * si1;
				/*--- calcul de la distance moyenne ---*/
				for ( z = zmin; z <= zmax; z ++ ) {
					smean = (double)0.0;
					for ( y = ymin; y <= ymax; y ++ )
					for ( x = xmin; x <= xmax; x ++ ) {
						i1 = (double)(buf1[x + y*d1x + z*d1xy]);
						i2 = (double)(buf2[x + y*d2x + z*d2xy]);
						d = i2 - a * i1 - b;
						if ( d < 0.0 ) smean -= d;
						else smean += d;
					}
					mean += ( smean * inb );
				}
				mean /= sqrt( 1.0 + a * a);
				}
				break;
			default :
				VT_Error( "image type unknown or not supported", "VT_MesureCorrelation" );
				return( -1 );
		}}
		break;
	default :
		VT_Error( "image type unknown or not supported", "VT_MesureCorrelation" );
		return( -1 );
	}

	par->si1 = si1;
	par->si2 = si2;
	par->si12 = si12;
	par->si22 = si22;
	par->si1i2 = si1i2;
	par->a = a;
	par->b = b;
	par->mean = mean;
	par->sigma = sigma;

	return( 1 );
}


