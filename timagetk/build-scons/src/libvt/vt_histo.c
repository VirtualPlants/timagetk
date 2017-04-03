
#include <vt_histo.h>

void VT_Histo( vt_histo *histo /* histogram to be initialized */ )
{
    register int i;
    for (i=0; i<STRINGLENGTH; i++) histo->name[i] = '\0';
    histo->type = 0;
    histo->size = 0;
    histo->offset = 0;
    histo->buf = (u64*)NULL;
    histo->min = histo->max = 0;
    histo->moy = 0.0;
}





int VT_AllocHisto( vt_histo *histo /* histogram to be allocated */ )
{
    register int i;
    
    switch ( histo->type ) {
    case SCHAR :
	histo->offset = -128;
	break;
    case SSHORT :
	histo->offset = -32768;
	break;
    default :
	histo->offset = 0;
    }

   switch ( histo->type ) {
    case UCHAR :
    case SCHAR :
	histo->size = 256;
	break;
    case USHORT :
    case SSHORT :
	histo->size = 65536;
	break;
    default :
	histo->offset = histo->size = 0;
	VT_Error( "unknown histogram type", "VT_AllocHisto" );
	return( -1 );
    }

    histo->buf = (u64*)VT_Malloc( (unsigned int)(histo->size * sizeof(u64)) );
    if (histo->buf == (u64*)NULL) {
	VT_Error( "buffer allocation failed", "VT_AllocHisto" );
	return( -1 );
    }

    for ( i = 0; i < histo->size; i ++ )
	histo->buf[i] = 0;
    
    return( 1 );
    
}





void VT_FreeHisto( vt_histo *histo )
{
	if ( histo->buf == (u64*)NULL ) return;
	VT_Free( (void**)&(histo->buf) );
	VT_Histo( histo );
}





void VT_PrintHisto( vt_histo *histo )
{
  int i;
  u64 *h = (u64*)(histo->buf);
  
  for ( i =0; i<histo->size; i ++ ) {
    if ( h[i] != 0 ) 
      printf("HISTOGRAM[%6d] = %8ld\n", i+histo->offset, h[i] );
  }
}





int VT_ComputeHisto( vt_histo *histo /* histogram to be filled */,
		     vt_image *image /* input image */ )
{
    register int i, o, vol;
    register u64* h;
    
    /*--- tests ---*/
    if ( VT_Test1Image( image, "VT_ComputeHisto" ) == -1 ) {
	VT_Error( "Image not correct", "VT_ComputeHisto" );
	return( -1 );
    }    
    if ( histo->buf == (u64*)NULL ) {
	VT_Error( "Histogram not allocated", "VT_ComputeHisto" );
	return( -1 );
    }    
    if ( histo->type != image->type ) {
	VT_Error( "Histogram and image have different type", "VT_ComputeHisto" );
	return( -1 );
    }    

    /*--- ---*/
    vol = image->dim.x * image->dim.y * image->dim.z * image->dim.v;
    
    /*--- initialisation ---*/
    h = (u64*)(histo->buf);
    for ( i = 0; i < histo->size; i ++ )
	h[i] = 0;

    /*--- calcul ---*/
    switch ( image->type ) {
    case SCHAR :
        {
	s8 *b;
	b = (s8 *)(image->buf);
	histo->offset = o = -128;
	for ( i = 0; i < vol; i ++ ) 
	    h[ ((int)(*b++)) - o ] ++;
	}
	break;
    case UCHAR :
        {
	u8 *b;
	b = (u8 *)(image->buf);
	histo->offset = o = 0;
	for ( i = 0; i < vol; i ++ ) 
	    h[ ((int)(*b++)) ] ++;
	}
	break;
    case SSHORT :
        {
	s16 *b;
	b = (s16 *)(image->buf);
	histo->offset = o = -32768;
	for ( i = 0; i < vol; i ++ ) 
	    h[ ((int)(*b++)) - o ] ++;
	}
	break;
    case USHORT :
        {
	u16 *b;
	b = (u16 *)(image->buf);
	histo->offset = o = 0;
	for ( i = 0; i < vol; i ++ ) 
	    h[ ((int)(*b++)) ] ++;
	}
	break;
    default :
	VT_Error( "image type unknown or not supported", "VT_ComputeHisto" );
	return( -1 );
    }
    
    /*--- min, moy et max ---*/
    if ( VT_MMMHisto( histo ) != 1 ) {
	VT_Error( "Unable to comput min, moy and max of histogram", "VT_ComputeHisto" );
	return( -1 );
    }
    
    return( 1 );
}





int VT_ComputeMaskedHisto( vt_histo *histo /* histogram to be filled */,
			   vt_image *image /* input image */,
			   vt_image *mask )
{
    register int i, o, vol;
    register u64* h;
    
    

    unsigned char *buf;

    if ( mask == NULL )
      return( VT_ComputeHisto( histo, mask ) );
    

    buf = (unsigned char*)(mask->buf);
    /*--- tests ---*/
    if ( VT_Test1Image( image, "VT_ComputeHisto" ) == -1 ) {
	VT_Error( "Image not correct", "VT_ComputeHisto" );
	return( -1 );
    }    
    if ( mask->type != UCHAR ) {
      VT_Error( "Mask type not correct", "VT_ComputeHisto" );
      return( -1 );
    }
    if ( VT_Test2Image( image, mask, "VT_ComputeHisto" ) == -1 ) return( -1 );

    if ( histo->buf == (u64*)NULL ) {
	VT_Error( "Histogram not allocated", "VT_ComputeHisto" );
	return( -1 );
    }    
    if ( histo->type != image->type ) {
	VT_Error( "Histogram and image have different type", "VT_ComputeHisto" );
	return( -1 );
    }    


    /*--- ---*/
    vol = image->dim.x * image->dim.y * image->dim.z * image->dim.v;
    
    /*--- initialisation ---*/
    h = (u64*)(histo->buf);
    for ( i = 0; i < histo->size; i ++ )
	h[i] = 0;

    /*--- calcul ---*/
    switch ( image->type ) {
    case SCHAR :
        {
	s8 *b;
	b = (s8 *)(image->buf);
	histo->offset = o = -128;
	for ( i = 0; i < vol; i ++, b ++, buf ++ ) 
	  if ( *buf > 0 ) h[ ((int)(*b)) - o ] ++;
	}
	break;
    case UCHAR :
        {
	u8 *b;
	b = (u8 *)(image->buf);
	histo->offset = o = 0;
	for ( i = 0; i < vol; i ++, b ++, buf ++ ) 
	    if ( *buf > 0 ) h[ ((int)(*b)) ] ++;
	}
	break;
    case SSHORT :
        {
	s16 *b;
	b = (s16 *)(image->buf);
	histo->offset = o = -32768;
	for ( i = 0; i < vol; i ++, b ++, buf ++ ) 
	    if ( *buf > 0 ) h[ ((int)(*b)) - o ] ++;
	}
	break;
    case USHORT :
        {
	u16 *b;
	b = (u16 *)(image->buf);
	histo->offset = o = 0;
	for ( i = 0; i < vol; i ++, b ++, buf ++ ) 
	    if ( *buf > 0 ) h[ ((int)(*b)) ] ++;
	}
	break;
    default :
	VT_Error( "image type unknown or not supported", "VT_ComputeHisto" );
	return( -1 );
    }
    
    /*--- min, moy et max ---*/
    if ( VT_MMMHisto( histo ) != 1 ) {
	VT_Error( "Unable to comput min, moy and max of histogram", "VT_ComputeHisto" );
	return( -1 );
    }
    
    return( 1 );
}





int VT_MMMHisto( vt_histo *histo )
{
    register int i, min, max;
    register u64* h;
    register u64 moy, sum;

    if ( histo->buf == (u64*)NULL ) {
	VT_Error( "Histogram not allocated", "VT_MMMHisto" );
	return( -1 );
    }    

    switch ( histo->type ) {
    case SCHAR :
    case UCHAR :
    case SSHORT :
    case USHORT :
	break;
    default :
	VT_Error( "histogram type unknown or not supported", "VT_MMMHisto" );
	return( -1 );
    }

    h = (u64*)(histo->buf);
    
    /*--- min ---*/
    i = 0;
    while ( (i < histo->size) && (h[i] == 0) ) i++ ;
    
    /*--- histogramme nul ---*/
    if ( i == histo->size ) {
	histo->min = histo->max = 0;
	histo->moy = 0.0;
	return( 1 );
    }
    
    /*--- min (continued) ---*/
    min = i;

    /*--- max ---*/
    i = histo->size - 1;
    while ( (i >= 0) && (h[i] == 0) ) i-- ;
    max = i;

    /*--- moy ---*/
    moy = sum = 0;
    for ( i = min; i <= max; i++ ) {
	moy += (u64)i * h[i];
	sum += h[i];
    }
    
    /*--- retours ---*/
    histo->min = min + histo->offset;
    histo->max = max + histo->offset;
    histo->moy = (double)moy / (double)sum + (double)histo->offset;
    
    return( 1 );
}





int VT_CumulativeHisto( vt_histo *cum, vt_histo *histo )
{
    register int i;
    register u64 *h, *c;

    if ( histo->buf == (u64*)NULL ) {
	VT_Error( "Histogram not allocated", "VT_CumulativeHisto" );
	return( -1 );
    }    
    if ( cum->buf == (u64*)NULL ) {
	VT_Error( "Cumulative histogram not allocated", "VT_CumulativeHisto" );
	return( -1 );
    }    
    if ( cum->type != histo->type ) {
	VT_Error( "Histogram and cumulative histogram have different type", "VT_CumulativeHisto" );
	return( -1 );
    }    
    
    switch ( histo->type ) {
    case SCHAR :
    case UCHAR :
    case SSHORT :
    case USHORT :
	break;
    default :
	VT_Error( "histogram type unknown or not supported", "VT_CumulativeHisto" );
	return( -1 );
    }

    cum->offset = histo->offset;
    cum->size   = histo->size;
    cum->min    = histo->min;
    cum->max    = histo->max;
    cum->moy    = histo->moy;
    h = (u64*)(histo->buf);
    c = (u64*)(cum->buf);
    
    c[0] = h[0];
    for ( i = 1; i < cum->size; i ++ )
	c[i] = c[i-1] + h[i];
    
    return( 1 );
}
