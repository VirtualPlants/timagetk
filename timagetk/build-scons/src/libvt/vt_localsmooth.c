#include <vt_localsmooth.h>

static int _Operation_Uchar( vt_image *theIm, 
			     vt_image *resIm, 
			     Neighborhood conn, 
			     int iterations );
static int _Operation_Schar( vt_image *theIm, 
			     vt_image *resIm, 
			     Neighborhood conn, 
			     int iterations );
static int _Operation_Ushort( vt_image *theIm, 
			      vt_image *resIm, 
			      Neighborhood conn, 
			      int iterations );
static int _Operation_Sshort( vt_image *theIm, 
			      vt_image *resIm, 
			      Neighborhood conn, 
			      int iterations );
static int _Operation_Int( vt_image *theIm, 
			   vt_image *resIm, 
			   Neighborhood conn, 
			   int iterations );
static int _Operation_Float( vt_image *theIm, 
			     vt_image *resIm, 
			     Neighborhood conn, 
			     int iterations );





int VT_06LocalSmoothing( vt_image *theIm, vt_image *resIm, int iterations )
{
  Neighborhood c = C_06;
  return( VT_SmoothingWithConnectivity( theIm, resIm, c, iterations ) );
}





int VT_26LocalSmoothing( vt_image *theIm, vt_image *resIm, int iterations )
{
  Neighborhood c = C_26;
  return( VT_SmoothingWithConnectivity( theIm, resIm, c, iterations ) );
}





int VT_SmoothingWithConnectivity( vt_image *theIm, vt_image *resIm, Neighborhood conn, int iterations )
{
  char *proc="VT_SmoothingWithConnectivity";
  Neighborhood c = conn;

  if ( VT_Test2Image( resIm, theIm, proc ) == -1 ) return( -1 );
  if ( theIm->type != resIm->type ) {
    VT_Error("images have different types",proc);
    return( -1 );
  }
  if ( ((theIm->dim.x <= 2) && (theIm->dim.y <= 2)) || (iterations <= 0) )
    if ( VT_CopyImage( theIm, resIm ) != 1 ) {
      VT_Error( "error while copying", proc );
      return( -1 );
    }
  
  if ( theIm->dim.z == 1 ) {
    switch ( c ) {
    case C_10 :
    case C_18 :
    case C_26 :
      c = C_08; break;
    case C_06 :
      c = C_04; break;
    case C_04 :
    case C_08 :
      break;
    }
  }

  switch ( theIm->type ) {
  case UCHAR :
    return( _Operation_Uchar( theIm, resIm, c, iterations ) );
  case SCHAR :
    return( _Operation_Schar( theIm, resIm, c, iterations ) );
  case USHORT :
    return( _Operation_Ushort( theIm, resIm, c, iterations ) );
  case SSHORT :
    return( _Operation_Sshort( theIm, resIm, c, iterations ) );
  case SINT :
    return( _Operation_Int( theIm, resIm, c, iterations ) );
  case FLOAT :
    return( _Operation_Float( theIm, resIm, c, iterations ) );
  default :
    VT_Error( "image type unknown or not supported", proc );
    return( -1 );
  }
  
  return( 0 );
}





/* basic function for local averaging for one given type

 */
static int _Operation_Uchar( vt_image *theIm, vt_image *resIm, Neighborhood conn, int iterations )
{
  char *proc="_Operation_Uchar";
  vt_image *input = (vt_image*)NULL;
  void *localBuf = (double*)NULL;
  char *localTab[5];
  register double coefCorner = 0.0;
  register double coefVertex = 0.0;
  register double coefFace = 0.0;
  register double coefVolume = 0.0;
  int dx, dy, dz, dx1, dy1, dz1;
  int dxy;
  int sliceSize;
  register int x, y, z;
  int iter;

  /* we suppose that tests on images and connexite have already be made and successful */
  /* initialization */
  dx = theIm->dim.x;   dx1 = dx - 1;
  dy = theIm->dim.y;   dy1 = dy - 1;
  dz = theIm->dim.z;   dz1 = dz - 1;
  dxy = dx * dy;

  /* allocation of the auxiliary buffer 
   */
  sliceSize = dx * dy * sizeof(u8);
  localBuf = (void*)VT_Malloc( (unsigned int)(5 * dxy * sizeof(double)) );
  if ( localBuf == (void*)NULL ) {
    VT_Error( "allocation failed for auxiliary buffer", proc );
    return( -1 );
  }
  for ( x = 0; x < 5; x ++ ) {
    localTab[x] = localBuf;
    localTab[x] += x * dxy * sizeof( double );
  }

  /* iterative processing */
  input = theIm;
  for ( iter = 0; iter < iterations; iter ++ ) {
    switch ( conn ) {

    case C_04 :
      {
	register u8 *cur = (u8*)(input->buf);
	u8 *resBuf = (u8*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	register u8 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register u8 *res;
	py -= dx;   px -= 1; 
	ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)3.0;
	coefVertex = (double)1.0 / (double)4.0;
	coefFace   = (double)1.0 / (double)5.0;

	for ( z = 0; z < dz; z ++ ) {
	  res = (u8 *)auxCurr;
	  /* first row */
          *res = (*cur + *nx + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, res++ ) {
            *res = (*cur + *nx + *ny + *px) * coefVertex;
	  }
          *res = (*cur + *px + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  py += dx;
	  /* middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, res++ ) {
              *res = (*cur + *nx + *px + *ny + *py) * coefFace;
	    }
            *res = (*cur + *px + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	  }
	  /* last row */
          *res = (*cur + *nx + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, res++ ) {
            *res = (*cur + *px + *nx + *py) * coefVertex;
	  }
          *res = (*cur + *px + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  ny += dx;
	  /* copy */
	  memcpy( (void*)resBuf, auxCurr, sliceSize );
	  resBuf += dxy;
	}
      }
      break;

    case C_08 :
      {
	register u8 *cur = (u8*)(input->buf);
	u8 *resBuf = (u8*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	register u8 *px = cur;
	register double *cy, *py, *ny;
	register double *res;
	px -= 1;
        coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)6.0;
	coefFace   = (double)1.0 / (double)9.0;

	for ( z = 0; z < dz; z ++ ) {
	  /* computing along X */
	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, ny++ )
            *resBuf = (*cy + *ny) * coefVertex;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) {
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
   	    for ( x = 1; x < dx1; x++, cy++, ny++, py++, resBuf++ ) {
              *resBuf = (*cy + *py + *ny) * coefFace;
	    }
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
	  }
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, py++ )
            *resBuf = (*cy + *py) * coefVertex;
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	}
      }
      break;

    case C_06 :
      {
	register u8 *cur = (u8*)(input->buf);
	u8 *resBuf = (u8*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	void *auxPrev = (void*)localTab[1];
	void *tmp;
	register u8 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register u8 *pz = cur, *nz = cur; 
	register u8 *res;
	pz -= dxy;   py -= dx;   px -= 1; 
	nz += dxy;   ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)5.0;
	coefFace   = (double)1.0 / (double)6.0;
	coefVolume = (double)1.0 / (double)7.0;

	/* first slice */
	res = (u8*)auxCurr;
	/* first slice, first row */
        *res = (*cur + *nx + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *nz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	py += dx;
	/* first slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	}
	/* first slice, last row */
        *res = (*cur + *nx + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *nz) * coefVertex;
	}
        *res = (*cur + *px + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	ny += dx; pz += dxy;
	tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	
	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  res = (u8*)auxCurr;
	  /* middle slice, first row */
          *res = (*cur + *nx + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *ny + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  py += dx;
	  /* middle slice, middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, pz++, res++ ) {
              *res = (*cur + *px + *nx + *py + *ny + *pz + *nz) * coefVolume;
	    }
            *res = (*cur + *px + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	  }
	  /* middle slice, last row */
          *res = (*cur + *nx + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *py + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  ny += dx;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	}
	/* last slice */
	res = (u8*)auxCurr;
	/* last slice, first row */
        *res = (*cur + *nx + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *pz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	py += dx;
	/* last slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, pz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *pz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	}
	/* last slice, last row */
        *res = (*cur + *nx + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *pz) * coefVertex;
	}
        *res = (*cur + *px + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_26 :
      {
	register u8 *cur = (u8*)(input->buf);
	u8 *resBuf = (u8*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxNext = (void*)localTab[2];
	void *auxPrev = (void*)localTab[3];
	void *tmp;
	register u8 *px = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	coefCorner = (double)1.0 / (double)8.0;
	coefVertex = (double)1.0 / (double)12.0;
	coefFace   = (double)1.0 / (double)18.0;
	coefVolume = (double)1.0 / (double)27.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;

	/* second slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxNext;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)auxNext;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	
	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  res = (double*)auxNext;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  
          /* middle slice: result */
          pz = (double*)auxPrev;
	  cz = (double*)auxCurr;   nz = (double*)auxNext;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}

	/* last slice */
	cz = (double*)auxCurr;   pz = (double*)auxPrev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    case C_10 :
      {
	register u8 *cur = (u8*)(input->buf);
	u8 *resBuf = (u8*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxPrev = (void*)localTab[2];
	void *tmp;
	register u8 *px = cur;
	register double *cy, *py, *ny;
	register u8 *pz = cur, *nz = cur;
	register double *res;
        register u8 *r;
	px -= 1;
	pz -= dxy;   nz += dxy;
	coefCorner = (double)1.0 / (double)5.0;
	coefVertex = (double)1.0 / (double)7.0;
	coefFace   = (double)1.0 / (double)8.0;
	coefVolume = (double)1.0 / (double)11.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (u8*)auxCurr;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, res++, nz++ )
            *r = (*res + *nz) * coefFace;
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
        }
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        pz += dxy;
	tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;

	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* current slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
 	  res = (double*)auxCurr;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* computing along Z */
          res = (double*)auxCurr;  r = (u8*)auxCurr;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
            for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
              *r = (*res + *pz + *nz) * coefVolume;
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
          }
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;
	}

	/* last slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (u8*)auxCurr;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
          for ( x = 1; x < dx1; x++, r++, res++, pz++ )
            *r = (*res + *pz) * coefFace;
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
        }
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_18 :
      {
	register u8 *cur = (u8*)(input->buf);
	u8 *resBuf = (u8*)(resIm->buf);
	void *aux4Prev = (void*)localTab[0];
	void *aux4Curr = (void*)localTab[1];
	void *aux4Next = (void*)localTab[2];
	void *auxCurr = (void*)localTab[3];
	void *auxNext = (void*)localTab[4];
	void *tmp;
	register u8 *px = cur;
	register u8 *ppy = cur, *nny = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	ppy -= dx;   nny += dx;
	coefCorner = (double)1.0 / (double)7.0;
	coefVertex = (double)1.0 / (double)10.0;
	coefFace   = (double)1.0 / (double)14.0;
	coefVolume = (double)1.0 / (double)19.0;

	/* first slice: computing along X */
	res = (double*)aux4Curr;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
 	/* first slice: 8-connectivity: computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)aux4Curr;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
 	/* first slice: 4-connectivity: computing along Y */
	res = (double*)aux4Curr;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;

	/* second slice: computing along X */
	res = (double*)aux4Next;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* second slice: 8-connectivity: computing along Y */
	res = (double *)auxNext;
	py = cy = ny = (double *)aux4Next;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* second slice: 4-connectivity: computing along Y */
	res = (double*)aux4Next;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)aux4Next;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;

	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: computing along X */
	  res = (double*)aux4Next;
	  for ( y = 0; y < dy; y ++ ) {
	    *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
	      *res = *cur + *px + px[2];
	    }
	    *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* next slice: 8-connectivity: computing along Y */
	  res = (double *)auxNext;
	  py = cy = ny = (double *)aux4Next;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	    for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	      *res = *py + *cy + *ny;
	    }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* next slice: 4-connectivity: computing along Y */
	  res = (double*)aux4Next;
	  for ( x = 0; x < dx; x++, res++, nny++ )
	    *res += *nny;
	  ppy += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	    *res += *nny + *ppy;
	  }
	  for ( x = 0; x < dx; x++, res++, ppy++ ) 
	    *res += *ppy;
	  nny += dx;
	  /* current slice: result */
          pz = (double*)aux4Prev;
	  cz = (double*)auxCurr;   nz = (double*)aux4Next;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;

	  tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	  tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}
	
	/* last slice: result */
	cz = (double*)auxCurr;   pz = (double*)aux4Prev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    default :
      VT_Free( (void**)(&localBuf) );
      VT_Error( "unknown (or not supported) connectivity", proc );
      return( -1 );
    }
    input = resIm;
  }
  /* end */
  VT_Free( (void**)&localBuf );
  return( 1 );
}





/* basic function for local averaging for one given type

 */
static int _Operation_Schar( vt_image *theIm, vt_image *resIm, Neighborhood conn, int iterations )
{
  char *proc="_Operation_Schar";
  vt_image *input = (vt_image*)NULL;
  void *localBuf = (double*)NULL;
  char *localTab[5];
  register double coefCorner = 0.0;
  register double coefVertex = 0.0;
  register double coefFace = 0.0;
  register double coefVolume = 0.0;
  int dx, dy, dz, dx1, dy1, dz1;
  int dxy;
  int sliceSize;
  register int x, y, z;
  int iter;

  /* we suppose that tests on images and connexite have already be made and successful */
  /* initialization */
  dx = theIm->dim.x;   dx1 = dx - 1;
  dy = theIm->dim.y;   dy1 = dy - 1;
  dz = theIm->dim.z;   dz1 = dz - 1;
  dxy = dx * dy;

  /* allocation of the auxiliary buffer 
   */
  sliceSize = dx * dy * sizeof(s8);
  localBuf = (void*)VT_Malloc( (unsigned int)(5 * dxy * sizeof(double)) );
  if ( localBuf == (void*)NULL ) {
    VT_Error( "allocation failed for auxiliary buffer", proc );
    return( -1 );
  }
  for ( x = 0; x < 5; x ++ ) {
    localTab[x] = localBuf;
    localTab[x] += x * dxy * sizeof( double );
  }

  /* iterative processing */
  input = theIm;
  for ( iter = 0; iter < iterations; iter ++ ) {
    switch ( conn ) {

    case C_04 :
      {
	register s8 *cur = (s8*)(input->buf);
	s8 *resBuf = (s8*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	register s8 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register s8 *res;
	py -= dx;   px -= 1; 
	ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)3.0;
	coefVertex = (double)1.0 / (double)4.0;
	coefFace   = (double)1.0 / (double)5.0;

	for ( z = 0; z < dz; z ++ ) {
	  res = (s8 *)auxCurr;
	  /* first row */
          *res = (*cur + *nx + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, res++ ) {
            *res = (*cur + *nx + *ny + *px) * coefVertex;
	  }
          *res = (*cur + *px + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  py += dx;
	  /* middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, res++ ) {
              *res = (*cur + *nx + *px + *ny + *py) * coefFace;
	    }
            *res = (*cur + *px + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	  }
	  /* last row */
          *res = (*cur + *nx + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, res++ ) {
            *res = (*cur + *px + *nx + *py) * coefVertex;
	  }
          *res = (*cur + *px + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  ny += dx;
	  /* copy */
	  memcpy( (void*)resBuf, auxCurr, sliceSize );
	  resBuf += dxy;
	}
      }
      break;

    case C_08 :
      {
	register s8 *cur = (s8*)(input->buf);
	s8 *resBuf = (s8*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	register s8 *px = cur;
	register double *cy, *py, *ny;
	register double *res;
	px -= 1;
        coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)6.0;
	coefFace   = (double)1.0 / (double)9.0;

	for ( z = 0; z < dz; z ++ ) {
	  /* computing along X */
	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, ny++ )
            *resBuf = (*cy + *ny) * coefVertex;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) {
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
   	    for ( x = 1; x < dx1; x++, cy++, ny++, py++, resBuf++ ) {
              *resBuf = (*cy + *py + *ny) * coefFace;
	    }
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
	  }
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, py++ )
            *resBuf = (*cy + *py) * coefVertex;
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	}
      }
      break;

    case C_06 :
      {
	register s8 *cur = (s8*)(input->buf);
	s8 *resBuf = (s8*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	void *auxPrev = (void*)localTab[1];
	void *tmp;
	register s8 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register s8 *pz = cur, *nz = cur; 
	register s8 *res;
	pz -= dxy;   py -= dx;   px -= 1; 
	nz += dxy;   ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)5.0;
	coefFace   = (double)1.0 / (double)6.0;
	coefVolume = (double)1.0 / (double)7.0;

	/* first slice */
	res = (s8*)auxCurr;
	/* first slice, first row */
        *res = (*cur + *nx + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *nz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	py += dx;
	/* first slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	}
	/* first slice, last row */
        *res = (*cur + *nx + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *nz) * coefVertex;
	}
        *res = (*cur + *px + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	ny += dx; pz += dxy;
	tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	
	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  res = (s8*)auxCurr;
	  /* middle slice, first row */
          *res = (*cur + *nx + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *ny + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  py += dx;
	  /* middle slice, middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, pz++, res++ ) {
              *res = (*cur + *px + *nx + *py + *ny + *pz + *nz) * coefVolume;
	    }
            *res = (*cur + *px + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	  }
	  /* middle slice, last row */
          *res = (*cur + *nx + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *py + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  ny += dx;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	}
	/* last slice */
	res = (s8*)auxCurr;
	/* last slice, first row */
        *res = (*cur + *nx + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *pz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	py += dx;
	/* last slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, pz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *pz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	}
	/* last slice, last row */
        *res = (*cur + *nx + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *pz) * coefVertex;
	}
        *res = (*cur + *px + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_26 :
      {
	register s8 *cur = (s8*)(input->buf);
	s8 *resBuf = (s8*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxNext = (void*)localTab[2];
	void *auxPrev = (void*)localTab[3];
	void *tmp;
	register s8 *px = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	coefCorner = (double)1.0 / (double)8.0;
	coefVertex = (double)1.0 / (double)12.0;
	coefFace   = (double)1.0 / (double)18.0;
	coefVolume = (double)1.0 / (double)27.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;

	/* second slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxNext;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)auxNext;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	
	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  res = (double*)auxNext;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  
          /* middle slice: result */
          pz = (double*)auxPrev;
	  cz = (double*)auxCurr;   nz = (double*)auxNext;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}

	/* last slice */
	cz = (double*)auxCurr;   pz = (double*)auxPrev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    case C_10 :
      {
	register s8 *cur = (s8*)(input->buf);
	s8 *resBuf = (s8*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxPrev = (void*)localTab[2];
	void *tmp;
	register s8 *px = cur;
	register double *cy, *py, *ny;
	register s8 *pz = cur, *nz = cur;
	register double *res;
        register s8 *r;
	px -= 1;
	pz -= dxy;   nz += dxy;
	coefCorner = (double)1.0 / (double)5.0;
	coefVertex = (double)1.0 / (double)7.0;
	coefFace   = (double)1.0 / (double)8.0;
	coefVolume = (double)1.0 / (double)11.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (s8*)auxCurr;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, res++, nz++ )
            *r = (*res + *nz) * coefFace;
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
        }
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        pz += dxy;
	tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;

	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* current slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
 	  res = (double*)auxCurr;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* computing along Z */
          res = (double*)auxCurr;  r = (s8*)auxCurr;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
            for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
              *r = (*res + *pz + *nz) * coefVolume;
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
          }
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;
	}

	/* last slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (s8*)auxCurr;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
          for ( x = 1; x < dx1; x++, r++, res++, pz++ )
            *r = (*res + *pz) * coefFace;
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
        }
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_18 :
      {
	register s8 *cur = (s8*)(input->buf);
	s8 *resBuf = (s8*)(resIm->buf);
	void *aux4Prev = (void*)localTab[0];
	void *aux4Curr = (void*)localTab[1];
	void *aux4Next = (void*)localTab[2];
	void *auxCurr = (void*)localTab[3];
	void *auxNext = (void*)localTab[4];
	void *tmp;
	register s8 *px = cur;
	register s8 *ppy = cur, *nny = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	ppy -= dx;   nny += dx;
	coefCorner = (double)1.0 / (double)7.0;
	coefVertex = (double)1.0 / (double)10.0;
	coefFace   = (double)1.0 / (double)14.0;
	coefVolume = (double)1.0 / (double)19.0;

	/* first slice: computing along X */
	res = (double*)aux4Curr;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
 	/* first slice: 8-connectivity: computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)aux4Curr;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
 	/* first slice: 4-connectivity: computing along Y */
	res = (double*)aux4Curr;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;

	/* second slice: computing along X */
	res = (double*)aux4Next;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* second slice: 8-connectivity: computing along Y */
	res = (double *)auxNext;
	py = cy = ny = (double *)aux4Next;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* second slice: 4-connectivity: computing along Y */
	res = (double*)aux4Next;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)aux4Next;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;

	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: computing along X */
	  res = (double*)aux4Next;
	  for ( y = 0; y < dy; y ++ ) {
	    *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
	      *res = *cur + *px + px[2];
	    }
	    *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* next slice: 8-connectivity: computing along Y */
	  res = (double *)auxNext;
	  py = cy = ny = (double *)aux4Next;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	    for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	      *res = *py + *cy + *ny;
	    }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* next slice: 4-connectivity: computing along Y */
	  res = (double*)aux4Next;
	  for ( x = 0; x < dx; x++, res++, nny++ )
	    *res += *nny;
	  ppy += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	    *res += *nny + *ppy;
	  }
	  for ( x = 0; x < dx; x++, res++, ppy++ ) 
	    *res += *ppy;
	  nny += dx;
	  /* current slice: result */
          pz = (double*)aux4Prev;
	  cz = (double*)auxCurr;   nz = (double*)aux4Next;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;

	  tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	  tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}
	
	/* last slice: result */
	cz = (double*)auxCurr;   pz = (double*)aux4Prev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    default :
      VT_Free( (void**)(&localBuf) );
      VT_Error( "unknown (or not supported) connectivity", proc );
      return( -1 );
    }
    input = resIm;
  }
  /* end */
  VT_Free( (void**)&localBuf );
  return( 1 );
}





/* basic function for local averaging for one given type

 */
static int _Operation_Ushort( vt_image *theIm, vt_image *resIm, Neighborhood conn, int iterations )
{
  char *proc="_Operation_Ushort";
  vt_image *input = (vt_image*)NULL;
  void *localBuf = (double*)NULL;
  char *localTab[5];
  register double coefCorner = 0.0;
  register double coefVertex = 0.0;
  register double coefFace = 0.0;
  register double coefVolume = 0.0;
  int dx, dy, dz, dx1, dy1, dz1;
  int dxy;
  int sliceSize;
  register int x, y, z;
  int iter;

  /* we suppose that tests on images and connexite have already be made and successful */
  /* initialization */
  dx = theIm->dim.x;   dx1 = dx - 1;
  dy = theIm->dim.y;   dy1 = dy - 1;
  dz = theIm->dim.z;   dz1 = dz - 1;
  dxy = dx * dy;

  /* allocation of the auxiliary buffer 
   */
  sliceSize = dx * dy * sizeof(u16);
  localBuf = (void*)VT_Malloc( (unsigned int)(5 * dxy * sizeof(double)) );
  if ( localBuf == (void*)NULL ) {
    VT_Error( "allocation failed for auxiliary buffer", proc );
    return( -1 );
  }
  for ( x = 0; x < 5; x ++ ) {
    localTab[x] = localBuf;
    localTab[x] += x * dxy * sizeof( double );
  }

  /* iterative processing */
  input = theIm;
  for ( iter = 0; iter < iterations; iter ++ ) {
    switch ( conn ) {

    case C_04 :
      {
	register u16 *cur = (u16*)(input->buf);
	u16 *resBuf = (u16*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	register u16 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register u16 *res;
	py -= dx;   px -= 1; 
	ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)3.0;
	coefVertex = (double)1.0 / (double)4.0;
	coefFace   = (double)1.0 / (double)5.0;

	for ( z = 0; z < dz; z ++ ) {
	  res = (u16 *)auxCurr;
	  /* first row */
          *res = (*cur + *nx + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, res++ ) {
            *res = (*cur + *nx + *ny + *px) * coefVertex;
	  }
          *res = (*cur + *px + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  py += dx;
	  /* middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, res++ ) {
              *res = (*cur + *nx + *px + *ny + *py) * coefFace;
	    }
            *res = (*cur + *px + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	  }
	  /* last row */
          *res = (*cur + *nx + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, res++ ) {
            *res = (*cur + *px + *nx + *py) * coefVertex;
	  }
          *res = (*cur + *px + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  ny += dx;
	  /* copy */
	  memcpy( (void*)resBuf, auxCurr, sliceSize );
	  resBuf += dxy;
	}
      }
      break;

    case C_08 :
      {
	register u16 *cur = (u16*)(input->buf);
	u16 *resBuf = (u16*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	register u16 *px = cur;
	register double *cy, *py, *ny;
	register double *res;
	px -= 1;
        coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)6.0;
	coefFace   = (double)1.0 / (double)9.0;

	for ( z = 0; z < dz; z ++ ) {
	  /* computing along X */
	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, ny++ )
            *resBuf = (*cy + *ny) * coefVertex;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) {
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
   	    for ( x = 1; x < dx1; x++, cy++, ny++, py++, resBuf++ ) {
              *resBuf = (*cy + *py + *ny) * coefFace;
	    }
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
	  }
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, py++ )
            *resBuf = (*cy + *py) * coefVertex;
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	}
      }
      break;

    case C_06 :
      {
	register u16 *cur = (u16*)(input->buf);
	u16 *resBuf = (u16*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	void *auxPrev = (void*)localTab[1];
	void *tmp;
	register u16 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register u16 *pz = cur, *nz = cur; 
	register u16 *res;
	pz -= dxy;   py -= dx;   px -= 1; 
	nz += dxy;   ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)5.0;
	coefFace   = (double)1.0 / (double)6.0;
	coefVolume = (double)1.0 / (double)7.0;

	/* first slice */
	res = (u16*)auxCurr;
	/* first slice, first row */
        *res = (*cur + *nx + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *nz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	py += dx;
	/* first slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	}
	/* first slice, last row */
        *res = (*cur + *nx + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *nz) * coefVertex;
	}
        *res = (*cur + *px + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	ny += dx; pz += dxy;
	tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	
	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  res = (u16*)auxCurr;
	  /* middle slice, first row */
          *res = (*cur + *nx + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *ny + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  py += dx;
	  /* middle slice, middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, pz++, res++ ) {
              *res = (*cur + *px + *nx + *py + *ny + *pz + *nz) * coefVolume;
	    }
            *res = (*cur + *px + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	  }
	  /* middle slice, last row */
          *res = (*cur + *nx + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *py + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  ny += dx;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	}
	/* last slice */
	res = (u16*)auxCurr;
	/* last slice, first row */
        *res = (*cur + *nx + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *pz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	py += dx;
	/* last slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, pz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *pz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	}
	/* last slice, last row */
        *res = (*cur + *nx + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *pz) * coefVertex;
	}
        *res = (*cur + *px + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_26 :
      {
	register u16 *cur = (u16*)(input->buf);
	u16 *resBuf = (u16*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxNext = (void*)localTab[2];
	void *auxPrev = (void*)localTab[3];
	void *tmp;
	register u16 *px = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	coefCorner = (double)1.0 / (double)8.0;
	coefVertex = (double)1.0 / (double)12.0;
	coefFace   = (double)1.0 / (double)18.0;
	coefVolume = (double)1.0 / (double)27.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;

	/* second slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxNext;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)auxNext;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	
	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  res = (double*)auxNext;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  
          /* middle slice: result */
          pz = (double*)auxPrev;
	  cz = (double*)auxCurr;   nz = (double*)auxNext;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}

	/* last slice */
	cz = (double*)auxCurr;   pz = (double*)auxPrev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    case C_10 :
      {
	register u16 *cur = (u16*)(input->buf);
	u16 *resBuf = (u16*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxPrev = (void*)localTab[2];
	void *tmp;
	register u16 *px = cur;
	register double *cy, *py, *ny;
	register u16 *pz = cur, *nz = cur;
	register double *res;
        register u16 *r;
	px -= 1;
	pz -= dxy;   nz += dxy;
	coefCorner = (double)1.0 / (double)5.0;
	coefVertex = (double)1.0 / (double)7.0;
	coefFace   = (double)1.0 / (double)8.0;
	coefVolume = (double)1.0 / (double)11.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (u16*)auxCurr;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, res++, nz++ )
            *r = (*res + *nz) * coefFace;
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
        }
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        pz += dxy;
	tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;

	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* current slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
 	  res = (double*)auxCurr;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* computing along Z */
          res = (double*)auxCurr;  r = (u16*)auxCurr;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
            for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
              *r = (*res + *pz + *nz) * coefVolume;
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
          }
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;
	}

	/* last slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (u16*)auxCurr;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
          for ( x = 1; x < dx1; x++, r++, res++, pz++ )
            *r = (*res + *pz) * coefFace;
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
        }
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_18 :
      {
	register u16 *cur = (u16*)(input->buf);
	u16 *resBuf = (u16*)(resIm->buf);
	void *aux4Prev = (void*)localTab[0];
	void *aux4Curr = (void*)localTab[1];
	void *aux4Next = (void*)localTab[2];
	void *auxCurr = (void*)localTab[3];
	void *auxNext = (void*)localTab[4];
	void *tmp;
	register u16 *px = cur;
	register u16 *ppy = cur, *nny = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	ppy -= dx;   nny += dx;
	coefCorner = (double)1.0 / (double)7.0;
	coefVertex = (double)1.0 / (double)10.0;
	coefFace   = (double)1.0 / (double)14.0;
	coefVolume = (double)1.0 / (double)19.0;

	/* first slice: computing along X */
	res = (double*)aux4Curr;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
 	/* first slice: 8-connectivity: computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)aux4Curr;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
 	/* first slice: 4-connectivity: computing along Y */
	res = (double*)aux4Curr;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;

	/* second slice: computing along X */
	res = (double*)aux4Next;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* second slice: 8-connectivity: computing along Y */
	res = (double *)auxNext;
	py = cy = ny = (double *)aux4Next;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* second slice: 4-connectivity: computing along Y */
	res = (double*)aux4Next;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)aux4Next;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;

	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: computing along X */
	  res = (double*)aux4Next;
	  for ( y = 0; y < dy; y ++ ) {
	    *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
	      *res = *cur + *px + px[2];
	    }
	    *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* next slice: 8-connectivity: computing along Y */
	  res = (double *)auxNext;
	  py = cy = ny = (double *)aux4Next;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	    for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	      *res = *py + *cy + *ny;
	    }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* next slice: 4-connectivity: computing along Y */
	  res = (double*)aux4Next;
	  for ( x = 0; x < dx; x++, res++, nny++ )
	    *res += *nny;
	  ppy += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	    *res += *nny + *ppy;
	  }
	  for ( x = 0; x < dx; x++, res++, ppy++ ) 
	    *res += *ppy;
	  nny += dx;
	  /* current slice: result */
          pz = (double*)aux4Prev;
	  cz = (double*)auxCurr;   nz = (double*)aux4Next;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;

	  tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	  tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}
	
	/* last slice: result */
	cz = (double*)auxCurr;   pz = (double*)aux4Prev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    default :
      VT_Free( (void**)(&localBuf) );
      VT_Error( "unknown (or not supported) connectivity", proc );
      return( -1 );
    }
    input = resIm;
  }
  /* end */
  VT_Free( (void**)&localBuf );
  return( 1 );
}





/* basic function for local averaging for one given type

 */
static int _Operation_Sshort( vt_image *theIm, vt_image *resIm, Neighborhood conn, int iterations )
{
  char *proc="_Operation_Sshort";
  vt_image *input = (vt_image*)NULL;
  void *localBuf = (double*)NULL;
  char *localTab[5];
  register double coefCorner = 0.0;
  register double coefVertex = 0.0;
  register double coefFace = 0.0;
  register double coefVolume = 0.0;
  int dx, dy, dz, dx1, dy1, dz1;
  int dxy;
  int sliceSize;
  register int x, y, z;
  int iter;

  /* we suppose that tests on images and connexite have already be made and successful */
  /* initialization */
  dx = theIm->dim.x;   dx1 = dx - 1;
  dy = theIm->dim.y;   dy1 = dy - 1;
  dz = theIm->dim.z;   dz1 = dz - 1;
  dxy = dx * dy;

  /* allocation of the auxiliary buffer 
   */
  sliceSize = dx * dy * sizeof(s16);
  localBuf = (void*)VT_Malloc( (unsigned int)(5 * dxy * sizeof(double)) );
  if ( localBuf == (void*)NULL ) {
    VT_Error( "allocation failed for auxiliary buffer", proc );
    return( -1 );
  }
  for ( x = 0; x < 5; x ++ ) {
    localTab[x] = localBuf;
    localTab[x] += x * dxy * sizeof( double );
  }

  /* iterative processing */
  input = theIm;
  for ( iter = 0; iter < iterations; iter ++ ) {
    switch ( conn ) {

    case C_04 :
      {
	register s16 *cur = (s16*)(input->buf);
	s16 *resBuf = (s16*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	register s16 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register s16 *res;
	py -= dx;   px -= 1; 
	ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)3.0;
	coefVertex = (double)1.0 / (double)4.0;
	coefFace   = (double)1.0 / (double)5.0;

	for ( z = 0; z < dz; z ++ ) {
	  res = (s16 *)auxCurr;
	  /* first row */
          *res = (*cur + *nx + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, res++ ) {
            *res = (*cur + *nx + *ny + *px) * coefVertex;
	  }
          *res = (*cur + *px + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  py += dx;
	  /* middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, res++ ) {
              *res = (*cur + *nx + *px + *ny + *py) * coefFace;
	    }
            *res = (*cur + *px + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	  }
	  /* last row */
          *res = (*cur + *nx + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, res++ ) {
            *res = (*cur + *px + *nx + *py) * coefVertex;
	  }
          *res = (*cur + *px + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  ny += dx;
	  /* copy */
	  memcpy( (void*)resBuf, auxCurr, sliceSize );
	  resBuf += dxy;
	}
      }
      break;

    case C_08 :
      {
	register s16 *cur = (s16*)(input->buf);
	s16 *resBuf = (s16*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	register s16 *px = cur;
	register double *cy, *py, *ny;
	register double *res;
	px -= 1;
        coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)6.0;
	coefFace   = (double)1.0 / (double)9.0;

	for ( z = 0; z < dz; z ++ ) {
	  /* computing along X */
	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, ny++ )
            *resBuf = (*cy + *ny) * coefVertex;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) {
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
   	    for ( x = 1; x < dx1; x++, cy++, ny++, py++, resBuf++ ) {
              *resBuf = (*cy + *py + *ny) * coefFace;
	    }
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
	  }
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, py++ )
            *resBuf = (*cy + *py) * coefVertex;
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	}
      }
      break;

    case C_06 :
      {
	register s16 *cur = (s16*)(input->buf);
	s16 *resBuf = (s16*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	void *auxPrev = (void*)localTab[1];
	void *tmp;
	register s16 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register s16 *pz = cur, *nz = cur; 
	register s16 *res;
	pz -= dxy;   py -= dx;   px -= 1; 
	nz += dxy;   ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)5.0;
	coefFace   = (double)1.0 / (double)6.0;
	coefVolume = (double)1.0 / (double)7.0;

	/* first slice */
	res = (s16*)auxCurr;
	/* first slice, first row */
        *res = (*cur + *nx + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *nz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	py += dx;
	/* first slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	}
	/* first slice, last row */
        *res = (*cur + *nx + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *nz) * coefVertex;
	}
        *res = (*cur + *px + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	ny += dx; pz += dxy;
	tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	
	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  res = (s16*)auxCurr;
	  /* middle slice, first row */
          *res = (*cur + *nx + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *ny + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  py += dx;
	  /* middle slice, middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, pz++, res++ ) {
              *res = (*cur + *px + *nx + *py + *ny + *pz + *nz) * coefVolume;
	    }
            *res = (*cur + *px + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	  }
	  /* middle slice, last row */
          *res = (*cur + *nx + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *py + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  ny += dx;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	}
	/* last slice */
	res = (s16*)auxCurr;
	/* last slice, first row */
        *res = (*cur + *nx + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *pz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	py += dx;
	/* last slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, pz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *pz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	}
	/* last slice, last row */
        *res = (*cur + *nx + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *pz) * coefVertex;
	}
        *res = (*cur + *px + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_26 :
      {
	register s16 *cur = (s16*)(input->buf);
	s16 *resBuf = (s16*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxNext = (void*)localTab[2];
	void *auxPrev = (void*)localTab[3];
	void *tmp;
	register s16 *px = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	coefCorner = (double)1.0 / (double)8.0;
	coefVertex = (double)1.0 / (double)12.0;
	coefFace   = (double)1.0 / (double)18.0;
	coefVolume = (double)1.0 / (double)27.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;

	/* second slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxNext;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)auxNext;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	
	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  res = (double*)auxNext;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  
          /* middle slice: result */
          pz = (double*)auxPrev;
	  cz = (double*)auxCurr;   nz = (double*)auxNext;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}

	/* last slice */
	cz = (double*)auxCurr;   pz = (double*)auxPrev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    case C_10 :
      {
	register s16 *cur = (s16*)(input->buf);
	s16 *resBuf = (s16*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxPrev = (void*)localTab[2];
	void *tmp;
	register s16 *px = cur;
	register double *cy, *py, *ny;
	register s16 *pz = cur, *nz = cur;
	register double *res;
        register s16 *r;
	px -= 1;
	pz -= dxy;   nz += dxy;
	coefCorner = (double)1.0 / (double)5.0;
	coefVertex = (double)1.0 / (double)7.0;
	coefFace   = (double)1.0 / (double)8.0;
	coefVolume = (double)1.0 / (double)11.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (s16*)auxCurr;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, res++, nz++ )
            *r = (*res + *nz) * coefFace;
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
        }
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        pz += dxy;
	tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;

	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* current slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
 	  res = (double*)auxCurr;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* computing along Z */
          res = (double*)auxCurr;  r = (s16*)auxCurr;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
            for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
              *r = (*res + *pz + *nz) * coefVolume;
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
          }
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;
	}

	/* last slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (s16*)auxCurr;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
          for ( x = 1; x < dx1; x++, r++, res++, pz++ )
            *r = (*res + *pz) * coefFace;
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
        }
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_18 :
      {
	register s16 *cur = (s16*)(input->buf);
	s16 *resBuf = (s16*)(resIm->buf);
	void *aux4Prev = (void*)localTab[0];
	void *aux4Curr = (void*)localTab[1];
	void *aux4Next = (void*)localTab[2];
	void *auxCurr = (void*)localTab[3];
	void *auxNext = (void*)localTab[4];
	void *tmp;
	register s16 *px = cur;
	register s16 *ppy = cur, *nny = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	ppy -= dx;   nny += dx;
	coefCorner = (double)1.0 / (double)7.0;
	coefVertex = (double)1.0 / (double)10.0;
	coefFace   = (double)1.0 / (double)14.0;
	coefVolume = (double)1.0 / (double)19.0;

	/* first slice: computing along X */
	res = (double*)aux4Curr;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
 	/* first slice: 8-connectivity: computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)aux4Curr;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
 	/* first slice: 4-connectivity: computing along Y */
	res = (double*)aux4Curr;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;

	/* second slice: computing along X */
	res = (double*)aux4Next;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* second slice: 8-connectivity: computing along Y */
	res = (double *)auxNext;
	py = cy = ny = (double *)aux4Next;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* second slice: 4-connectivity: computing along Y */
	res = (double*)aux4Next;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)aux4Next;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;

	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: computing along X */
	  res = (double*)aux4Next;
	  for ( y = 0; y < dy; y ++ ) {
	    *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
	      *res = *cur + *px + px[2];
	    }
	    *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* next slice: 8-connectivity: computing along Y */
	  res = (double *)auxNext;
	  py = cy = ny = (double *)aux4Next;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	    for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	      *res = *py + *cy + *ny;
	    }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* next slice: 4-connectivity: computing along Y */
	  res = (double*)aux4Next;
	  for ( x = 0; x < dx; x++, res++, nny++ )
	    *res += *nny;
	  ppy += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	    *res += *nny + *ppy;
	  }
	  for ( x = 0; x < dx; x++, res++, ppy++ ) 
	    *res += *ppy;
	  nny += dx;
	  /* current slice: result */
          pz = (double*)aux4Prev;
	  cz = (double*)auxCurr;   nz = (double*)aux4Next;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;

	  tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	  tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}
	
	/* last slice: result */
	cz = (double*)auxCurr;   pz = (double*)aux4Prev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    default :
      VT_Free( (void**)(&localBuf) );
      VT_Error( "unknown (or not supported) connectivity", proc );
      return( -1 );
    }
    input = resIm;
  }
  /* end */
  VT_Free( (void**)&localBuf );
  return( 1 );
}





/* basic function for local averaging for one given type

 */
static int _Operation_Int( vt_image *theIm, vt_image *resIm, Neighborhood conn, int iterations )
{
  char *proc="_Operation_Int";
  vt_image *input = (vt_image*)NULL;
  void *localBuf = (double*)NULL;
  char *localTab[5];
  register double coefCorner = 0.0;
  register double coefVertex = 0.0;
  register double coefFace = 0.0;
  register double coefVolume = 0.0;
  int dx, dy, dz, dx1, dy1, dz1;
  int dxy;
  int sliceSize;
  register int x, y, z;
  int iter;

  /* we suppose that tests on images and connexite have already be made and successful */
  /* initialization */
  dx = theIm->dim.x;   dx1 = dx - 1;
  dy = theIm->dim.y;   dy1 = dy - 1;
  dz = theIm->dim.z;   dz1 = dz - 1;
  dxy = dx * dy;

  /* allocation of the auxiliary buffer 
   */
  sliceSize = dx * dy * sizeof(i32);
  localBuf = (void*)VT_Malloc( (unsigned int)(5 * dxy * sizeof(double)) );
  if ( localBuf == (void*)NULL ) {
    VT_Error( "allocation failed for auxiliary buffer", proc );
    return( -1 );
  }
  for ( x = 0; x < 5; x ++ ) {
    localTab[x] = localBuf;
    localTab[x] += x * dxy * sizeof( double );
  }

  /* iterative processing */
  input = theIm;
  for ( iter = 0; iter < iterations; iter ++ ) {
    switch ( conn ) {

    case C_04 :
      {
	register i32 *cur = (i32*)(input->buf);
	i32 *resBuf = (i32*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	register i32 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register i32 *res;
	py -= dx;   px -= 1; 
	ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)3.0;
	coefVertex = (double)1.0 / (double)4.0;
	coefFace   = (double)1.0 / (double)5.0;

	for ( z = 0; z < dz; z ++ ) {
	  res = (i32 *)auxCurr;
	  /* first row */
          *res = (*cur + *nx + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, res++ ) {
            *res = (*cur + *nx + *ny + *px) * coefVertex;
	  }
          *res = (*cur + *px + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  py += dx;
	  /* middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, res++ ) {
              *res = (*cur + *nx + *px + *ny + *py) * coefFace;
	    }
            *res = (*cur + *px + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	  }
	  /* last row */
          *res = (*cur + *nx + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, res++ ) {
            *res = (*cur + *px + *nx + *py) * coefVertex;
	  }
          *res = (*cur + *px + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  ny += dx;
	  /* copy */
	  memcpy( (void*)resBuf, auxCurr, sliceSize );
	  resBuf += dxy;
	}
      }
      break;

    case C_08 :
      {
	register i32 *cur = (i32*)(input->buf);
	i32 *resBuf = (i32*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	register i32 *px = cur;
	register double *cy, *py, *ny;
	register double *res;
	px -= 1;
        coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)6.0;
	coefFace   = (double)1.0 / (double)9.0;

	for ( z = 0; z < dz; z ++ ) {
	  /* computing along X */
	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, ny++ )
            *resBuf = (*cy + *ny) * coefVertex;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) {
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
   	    for ( x = 1; x < dx1; x++, cy++, ny++, py++, resBuf++ ) {
              *resBuf = (*cy + *py + *ny) * coefFace;
	    }
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
	  }
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, py++ )
            *resBuf = (*cy + *py) * coefVertex;
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	}
      }
      break;

    case C_06 :
      {
	register i32 *cur = (i32*)(input->buf);
	i32 *resBuf = (i32*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	void *auxPrev = (void*)localTab[1];
	void *tmp;
	register i32 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register i32 *pz = cur, *nz = cur; 
	register i32 *res;
	pz -= dxy;   py -= dx;   px -= 1; 
	nz += dxy;   ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)5.0;
	coefFace   = (double)1.0 / (double)6.0;
	coefVolume = (double)1.0 / (double)7.0;

	/* first slice */
	res = (i32*)auxCurr;
	/* first slice, first row */
        *res = (*cur + *nx + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *nz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	py += dx;
	/* first slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	}
	/* first slice, last row */
        *res = (*cur + *nx + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *nz) * coefVertex;
	}
        *res = (*cur + *px + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	ny += dx; pz += dxy;
	tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	
	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  res = (i32*)auxCurr;
	  /* middle slice, first row */
          *res = (*cur + *nx + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *ny + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  py += dx;
	  /* middle slice, middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, pz++, res++ ) {
              *res = (*cur + *px + *nx + *py + *ny + *pz + *nz) * coefVolume;
	    }
            *res = (*cur + *px + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	  }
	  /* middle slice, last row */
          *res = (*cur + *nx + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *py + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  ny += dx;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	}
	/* last slice */
	res = (i32*)auxCurr;
	/* last slice, first row */
        *res = (*cur + *nx + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *pz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	py += dx;
	/* last slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, pz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *pz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	}
	/* last slice, last row */
        *res = (*cur + *nx + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *pz) * coefVertex;
	}
        *res = (*cur + *px + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_26 :
      {
	register i32 *cur = (i32*)(input->buf);
	i32 *resBuf = (i32*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxNext = (void*)localTab[2];
	void *auxPrev = (void*)localTab[3];
	void *tmp;
	register i32 *px = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	coefCorner = (double)1.0 / (double)8.0;
	coefVertex = (double)1.0 / (double)12.0;
	coefFace   = (double)1.0 / (double)18.0;
	coefVolume = (double)1.0 / (double)27.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;

	/* second slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxNext;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)auxNext;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	
	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  res = (double*)auxNext;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  
          /* middle slice: result */
          pz = (double*)auxPrev;
	  cz = (double*)auxCurr;   nz = (double*)auxNext;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}

	/* last slice */
	cz = (double*)auxCurr;   pz = (double*)auxPrev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    case C_10 :
      {
	register i32 *cur = (i32*)(input->buf);
	i32 *resBuf = (i32*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxPrev = (void*)localTab[2];
	void *tmp;
	register i32 *px = cur;
	register double *cy, *py, *ny;
	register i32 *pz = cur, *nz = cur;
	register double *res;
        register i32 *r;
	px -= 1;
	pz -= dxy;   nz += dxy;
	coefCorner = (double)1.0 / (double)5.0;
	coefVertex = (double)1.0 / (double)7.0;
	coefFace   = (double)1.0 / (double)8.0;
	coefVolume = (double)1.0 / (double)11.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (i32*)auxCurr;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, res++, nz++ )
            *r = (*res + *nz) * coefFace;
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
        }
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        pz += dxy;
	tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;

	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* current slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
 	  res = (double*)auxCurr;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* computing along Z */
          res = (double*)auxCurr;  r = (i32*)auxCurr;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
            for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
              *r = (*res + *pz + *nz) * coefVolume;
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
          }
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;
	}

	/* last slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (i32*)auxCurr;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
          for ( x = 1; x < dx1; x++, r++, res++, pz++ )
            *r = (*res + *pz) * coefFace;
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
        }
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_18 :
      {
	register i32 *cur = (i32*)(input->buf);
	i32 *resBuf = (i32*)(resIm->buf);
	void *aux4Prev = (void*)localTab[0];
	void *aux4Curr = (void*)localTab[1];
	void *aux4Next = (void*)localTab[2];
	void *auxCurr = (void*)localTab[3];
	void *auxNext = (void*)localTab[4];
	void *tmp;
	register i32 *px = cur;
	register i32 *ppy = cur, *nny = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	ppy -= dx;   nny += dx;
	coefCorner = (double)1.0 / (double)7.0;
	coefVertex = (double)1.0 / (double)10.0;
	coefFace   = (double)1.0 / (double)14.0;
	coefVolume = (double)1.0 / (double)19.0;

	/* first slice: computing along X */
	res = (double*)aux4Curr;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
 	/* first slice: 8-connectivity: computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)aux4Curr;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
 	/* first slice: 4-connectivity: computing along Y */
	res = (double*)aux4Curr;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;

	/* second slice: computing along X */
	res = (double*)aux4Next;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* second slice: 8-connectivity: computing along Y */
	res = (double *)auxNext;
	py = cy = ny = (double *)aux4Next;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* second slice: 4-connectivity: computing along Y */
	res = (double*)aux4Next;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)aux4Next;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;

	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: computing along X */
	  res = (double*)aux4Next;
	  for ( y = 0; y < dy; y ++ ) {
	    *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
	      *res = *cur + *px + px[2];
	    }
	    *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* next slice: 8-connectivity: computing along Y */
	  res = (double *)auxNext;
	  py = cy = ny = (double *)aux4Next;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	    for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	      *res = *py + *cy + *ny;
	    }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* next slice: 4-connectivity: computing along Y */
	  res = (double*)aux4Next;
	  for ( x = 0; x < dx; x++, res++, nny++ )
	    *res += *nny;
	  ppy += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	    *res += *nny + *ppy;
	  }
	  for ( x = 0; x < dx; x++, res++, ppy++ ) 
	    *res += *ppy;
	  nny += dx;
	  /* current slice: result */
          pz = (double*)aux4Prev;
	  cz = (double*)auxCurr;   nz = (double*)aux4Next;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;

	  tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	  tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}
	
	/* last slice: result */
	cz = (double*)auxCurr;   pz = (double*)aux4Prev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    default :
      VT_Free( (void**)(&localBuf) );
      VT_Error( "unknown (or not supported) connectivity", proc );
      return( -1 );
    }
    input = resIm;
  }
  /* end */
  VT_Free( (void**)&localBuf );
  return( 1 );
}





/* basic function for local averaging for one given type

 */
static int _Operation_Float( vt_image *theIm, vt_image *resIm, Neighborhood conn, int iterations )
{
  char *proc="_Operation_Float";
  vt_image *input = (vt_image*)NULL;
  void *localBuf = (double*)NULL;
  char *localTab[5];
  register double coefCorner = 0.0;
  register double coefVertex = 0.0;
  register double coefFace = 0.0;
  register double coefVolume = 0.0;
  int dx, dy, dz, dx1, dy1, dz1;
  int dxy;
  int sliceSize;
  register int x, y, z;
  int iter;

  /* we suppose that tests on images and connexite have already be made and successful */
  /* initialization */
  dx = theIm->dim.x;   dx1 = dx - 1;
  dy = theIm->dim.y;   dy1 = dy - 1;
  dz = theIm->dim.z;   dz1 = dz - 1;
  dxy = dx * dy;

  /* allocation of the auxiliary buffer 
   */
  sliceSize = dx * dy * sizeof(r32);
  localBuf = (void*)VT_Malloc( (unsigned int)(5 * dxy * sizeof(double)) );
  if ( localBuf == (void*)NULL ) {
    VT_Error( "allocation failed for auxiliary buffer", proc );
    return( -1 );
  }
  for ( x = 0; x < 5; x ++ ) {
    localTab[x] = localBuf;
    localTab[x] += x * dxy * sizeof( double );
  }

  /* iterative processing */
  input = theIm;
  for ( iter = 0; iter < iterations; iter ++ ) {
    switch ( conn ) {

    case C_04 :
      {
	register r32 *cur = (r32*)(input->buf);
	r32 *resBuf = (r32*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	register r32 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register r32 *res;
	py -= dx;   px -= 1; 
	ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)3.0;
	coefVertex = (double)1.0 / (double)4.0;
	coefFace   = (double)1.0 / (double)5.0;

	for ( z = 0; z < dz; z ++ ) {
	  res = (r32 *)auxCurr;
	  /* first row */
          *res = (*cur + *nx + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, res++ ) {
            *res = (*cur + *nx + *ny + *px) * coefVertex;
	  }
          *res = (*cur + *px + *ny) * coefCorner;
	  cur++; px++; nx++; ny++; res++;
	  py += dx;
	  /* middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, res++ ) {
              *res = (*cur + *nx + *px + *ny + *py) * coefFace;
	    }
            *res = (*cur + *px + *ny + *py) * coefVertex;
	    cur++; px++; nx++; py++; ny++; res++;
	  }
	  /* last row */
          *res = (*cur + *nx + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, res++ ) {
            *res = (*cur + *px + *nx + *py) * coefVertex;
	  }
          *res = (*cur + *px + *py) * coefCorner;
	  cur++; px++; nx++; py++; res++;
	  ny += dx;
	  /* copy */
	  memcpy( (void*)resBuf, auxCurr, sliceSize );
	  resBuf += dxy;
	}
      }
      break;

    case C_08 :
      {
	register r32 *cur = (r32*)(input->buf);
	r32 *resBuf = (r32*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	register r32 *px = cur;
	register double *cy, *py, *ny;
	register double *res;
	px -= 1;
        coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)6.0;
	coefFace   = (double)1.0 / (double)9.0;

	for ( z = 0; z < dz; z ++ ) {
	  /* computing along X */
	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, ny++ )
            *resBuf = (*cy + *ny) * coefVertex;
          *resBuf = (*cy + *ny) * coefCorner;
          resBuf++; cy++; ny++;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) {
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
   	    for ( x = 1; x < dx1; x++, cy++, ny++, py++, resBuf++ ) {
              *resBuf = (*cy + *py + *ny) * coefFace;
	    }
            *resBuf = (*cy + *py + *ny) * coefVertex;
            resBuf++; cy++; ny++; py++;
	  }
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	  for ( x = 1; x < dx1; x++, resBuf++, cy++, py++ )
            *resBuf = (*cy + *py) * coefVertex;
          *resBuf = (*cy + *py) * coefCorner;
          resBuf++; cy++; py++;
	}
      }
      break;

    case C_06 :
      {
	register r32 *cur = (r32*)(input->buf);
	r32 *resBuf = (r32*)(resIm->buf);
	void *auxCurr = (void*)localTab[0];
	void *auxPrev = (void*)localTab[1];
	void *tmp;
	register r32 *px = cur, *nx = cur, *py = cur, *ny = cur;
	register r32 *pz = cur, *nz = cur; 
	register r32 *res;
	pz -= dxy;   py -= dx;   px -= 1; 
	nz += dxy;   ny += dx;   nx += 1; 
	coefCorner = (double)1.0 / (double)4.0;
	coefVertex = (double)1.0 / (double)5.0;
	coefFace   = (double)1.0 / (double)6.0;
	coefVolume = (double)1.0 / (double)7.0;

	/* first slice */
	res = (r32*)auxCurr;
	/* first slice, first row */
        *res = (*cur + *nx + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *nz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *nz) * coefCorner;
	cur++; px++; nx++; ny++; nz++; res++;
	py += dx;
	/* first slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *nz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; nz++; res++;
	}
	/* first slice, last row */
        *res = (*cur + *nx + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *nz) * coefVertex;
	}
        *res = (*cur + *px + *py + *nz) * coefCorner;
	cur++; px++; nx++; py++; nz++; res++;
	ny += dx; pz += dxy;
	tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	
	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  res = (r32*)auxCurr;
	  /* middle slice, first row */
          *res = (*cur + *nx + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *ny + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *ny + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; ny++; nz++; pz++; res++;
	  py += dx;
	  /* middle slice, middle rows */
	  for ( y = 1; y < dy1; y ++ ) {
            *res = (*cur + *nx + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, nz++, pz++, res++ ) {
              *res = (*cur + *px + *nx + *py + *ny + *pz + *nz) * coefVolume;
	    }
            *res = (*cur + *px + *py + *ny + *pz + *nz) * coefFace;
	    cur++; px++; nx++; py++; ny++; nz++; pz++; res++;
	  }
	  /* middle slice, last row */
          *res = (*cur + *nx + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, nz++, pz++, res++ ) {
            *res = (*cur + *px + *nx + *py + *pz + *nz) * coefFace;
	  }
          *res = (*cur + *px + *py + *pz + *nz) * coefVertex;
	  cur++; px++; nx++; py++; nz++; pz++; res++;
	  ny += dx;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev;   auxPrev = auxCurr;   auxCurr = tmp;
	}
	/* last slice */
	res = (r32*)auxCurr;
	/* last slice, first row */
        *res = (*cur + *nx + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, ny++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *ny + *pz) * coefVertex;
	}
        *res = (*cur + *px + *ny + *pz) * coefCorner;
	cur++; px++; nx++; ny++; pz++; res++;
	py += dx;
	/* last slice, middle rows */
	for ( y = 1; y < dy1; y ++ ) {
          *res = (*cur + *nx + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, ny++, pz++, res++ ) {
            *res = (*cur + *nx + *px + *py + *ny + *pz) * coefFace;
	  }
          *res = (*cur + *px + *py + *ny + *pz) * coefVertex;
	  cur++; px++; nx++; py++; ny++; pz++; res++;
	}
	/* last slice, last row */
        *res = (*cur + *nx + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	for ( x = 1; x < dx1; x++, cur++, px++, nx++, py++, pz++, res++ ) {
          *res = (*cur + *px + *nx + *py + *pz) * coefVertex;
	}
        *res = (*cur + *px + *py + *pz) * coefCorner;
	cur++; px++; nx++; py++; pz++; res++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_26 :
      {
	register r32 *cur = (r32*)(input->buf);
	r32 *resBuf = (r32*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxNext = (void*)localTab[2];
	void *auxPrev = (void*)localTab[3];
	void *tmp;
	register r32 *px = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	coefCorner = (double)1.0 / (double)8.0;
	coefVertex = (double)1.0 / (double)12.0;
	coefFace   = (double)1.0 / (double)18.0;
	coefVolume = (double)1.0 / (double)27.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;

	/* second slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxNext;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)auxNext;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	
	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
	  res = (double*)auxNext;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  
          /* middle slice: result */
          pz = (double*)auxPrev;
	  cz = (double*)auxCurr;   nz = (double*)auxNext;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}

	/* last slice */
	cz = (double*)auxCurr;   pz = (double*)auxPrev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    case C_10 :
      {
	register r32 *cur = (r32*)(input->buf);
	r32 *resBuf = (r32*)(resIm->buf);
	void *auxTmp = (void*)localTab[0];
	void *auxCurr = (void*)localTab[1];
	void *auxPrev = (void*)localTab[2];
	void *tmp;
	register r32 *px = cur;
	register double *cy, *py, *ny;
	register r32 *pz = cur, *nz = cur;
	register double *res;
        register r32 *r;
	px -= 1;
	pz -= dxy;   nz += dxy;
	coefCorner = (double)1.0 / (double)5.0;
	coefVertex = (double)1.0 / (double)7.0;
	coefFace   = (double)1.0 / (double)8.0;
	coefVolume = (double)1.0 / (double)11.0;

	/* first slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (r32*)auxCurr;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, res++, nz++ )
            *r = (*res + *nz) * coefFace;
          *r = (*res + *nz) * coefVertex;
          r++; res++; nz++;
        }
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        for ( x = 1; x < dx1; x++, r++, res++, nz++ )
          *r = (*res + *nz) * coefVertex;
        *r = (*res + *nz) * coefCorner;
        r++; res++; nz++;
        pz += dxy;
	tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;

	/* middle slices */
	for ( z = 1; z < dz1; z ++ ) {
	  /* current slice: 8-connectivity */
	  /* computing along X */
  	  res = (double*)auxTmp;
	  for ( y = 0; y < dy; y ++ ) {
            *res = *cur + px[2];
            cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
              *res = *cur + *px + px[2];
	    }
            *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* computing along Y */
 	  res = (double*)auxCurr;
	  py = cy = ny = (double*)auxTmp;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	    *res = *py + *cy + *ny;
	  }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* computing along Z */
          res = (double*)auxCurr;  r = (r32*)auxCurr;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
            for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
              *r = (*res + *pz + *nz) * coefVolume;
            *r = (*res + *pz + *nz) * coefFace;
            r++; pz++; res++; nz++;
          }
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
          for ( x = 1; x < dx1; x++, r++, pz++, res++, nz++ )
            *r = (*res + *pz + *nz) * coefFace;
          *r = (*res + *pz + *nz) * coefVertex;
          r++; pz++; res++; nz++;
	  /* copy of previous slice */
	  memcpy( (void*)resBuf, auxPrev, sliceSize );
	  resBuf += dxy;
	  tmp = auxPrev; auxPrev = auxCurr; auxCurr = tmp;
	}

	/* last slice: 8-connectivity */
	/* computing along X */
	res = (double*)auxTmp;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)auxTmp;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* computing along Z */
	res = (double*)auxCurr;   r = (r32*)auxCurr;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
          for ( x = 1; x < dx1; x++, r++, res++, pz++ )
            *r = (*res + *pz) * coefFace;
          *r = (*res + *pz) * coefVertex;
          r++; res++; pz++;
        }
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
        for ( x = 1; x < dx1; x++, r++, res++, pz++ )
          *r = (*res + *pz) * coefVertex;
        *r = (*res + *pz) * coefCorner;
        r++; res++; pz++;
	/* copy of previous slice */
	memcpy( (void*)resBuf, auxPrev, sliceSize );
	resBuf += dxy;
	/* copy of current slice */
	memcpy( (void*)resBuf, auxCurr, sliceSize );
      }
      break;

    case C_18 :
      {
	register r32 *cur = (r32*)(input->buf);
	r32 *resBuf = (r32*)(resIm->buf);
	void *aux4Prev = (void*)localTab[0];
	void *aux4Curr = (void*)localTab[1];
	void *aux4Next = (void*)localTab[2];
	void *auxCurr = (void*)localTab[3];
	void *auxNext = (void*)localTab[4];
	void *tmp;
	register r32 *px = cur;
	register r32 *ppy = cur, *nny = cur;
	register double *cy, *py, *ny;
	register double *cz, *pz, *nz;
	register double *res;
	px -= 1;
	ppy -= dx;   nny += dx;
	coefCorner = (double)1.0 / (double)7.0;
	coefVertex = (double)1.0 / (double)10.0;
	coefFace   = (double)1.0 / (double)14.0;
	coefVolume = (double)1.0 / (double)19.0;

	/* first slice: computing along X */
	res = (double*)aux4Curr;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
 	/* first slice: 8-connectivity: computing along Y */
	res = (double*)auxCurr;
	py = cy = ny = (double*)aux4Curr;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
 	/* first slice: 4-connectivity: computing along Y */
	res = (double*)aux4Curr;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;

	/* second slice: computing along X */
	res = (double*)aux4Next;
	for ( y = 0; y < dy; y ++ ) {
          *res = *cur + px[2];
          cur++; px++; res++;
	  for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
            *res = *cur + *px + px[2];
	  }
          *res = *cur + *px;
	  cur++; px++; res++;
	}
	/* second slice: 8-connectivity: computing along Y */
	res = (double *)auxNext;
	py = cy = ny = (double *)aux4Next;
	py -= dx; ny += dx;
	for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	  *res = *cy + *ny;
	py += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	  *res = *py + *cy + *ny;
	}
	for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	  *res = *cy + *py;
	/* second slice: 4-connectivity: computing along Y */
	res = (double*)aux4Next;
	for ( x = 0; x < dx; x++, res++, nny++ )
	  *res += *nny;
	ppy += dx;
	for ( y = 1; y < dy1; y ++ ) 
	for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	  *res += *nny + *ppy;
	}
	for ( x = 0; x < dx; x++, res++, ppy++ ) 
	  *res += *ppy;
	nny += dx;
	
	/* first slice: result */
	cz = (double*)auxCurr;   nz = (double*)aux4Next;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
            *resBuf = (*cz + *nz) * coefFace;
          *resBuf = (*cz + *nz) * coefVertex;
          resBuf++; cz++; nz++;
        }
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, nz++ )
          *resBuf = (*cz + *nz) * coefVertex;
        *resBuf = (*cz + *nz) * coefCorner;
        resBuf++; cz++; nz++;

	tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;

	/* middle slice */
	for ( z = 1; z < dz1; z ++ ) {
	  /* next slice: computing along X */
	  res = (double*)aux4Next;
	  for ( y = 0; y < dy; y ++ ) {
	    *res = *cur + px[2];
	    cur++; px++; res++;
	    for ( x = 1; x < dx1; x++, cur++, px++, res++ ) {
	      *res = *cur + *px + px[2];
	    }
	    *res = *cur + *px;
	    cur++; px++; res++;
	  }
	  /* next slice: 8-connectivity: computing along Y */
	  res = (double *)auxNext;
	  py = cy = ny = (double *)aux4Next;
	  py -= dx; ny += dx;
	  for ( x = 0; x < dx; x++, res++, cy++, ny++ )
	    *res = *cy + *ny;
	  py += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	    for ( x = 0; x < dx; x++, cy++, ny++, py++, res++ ) {
	      *res = *py + *cy + *ny;
	    }
	  for ( x = 0; x < dx; x++, res++, cy++, py++ ) 
	    *res = *cy + *py;
	  /* next slice: 4-connectivity: computing along Y */
	  res = (double*)aux4Next;
	  for ( x = 0; x < dx; x++, res++, nny++ )
	    *res += *nny;
	  ppy += dx;
	  for ( y = 1; y < dy1; y ++ ) 
	  for ( x = 0; x < dx; x++, nny++, ppy++, res++ ) {
	    *res += *nny + *ppy;
	  }
	  for ( x = 0; x < dx; x++, res++, ppy++ ) 
	    *res += *ppy;
	  nny += dx;
	  /* current slice: result */
          pz = (double*)aux4Prev;
	  cz = (double*)auxCurr;   nz = (double*)aux4Next;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( y = 1; y < dy1; y++ ) {
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
            for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
              *resBuf = (*cz + *pz + *nz) * coefVolume;
            *resBuf = (*cz + *pz + *nz) * coefFace;
            resBuf++; pz++; cz++; nz++;
          }
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;
          for ( x = 1; x < dx1; x++, resBuf++, pz++, cz++, nz++ )
            *resBuf = (*cz + *pz + *nz) * coefFace;
          *resBuf = (*cz + *pz + *nz) * coefVertex;
          resBuf++; pz++; cz++; nz++;

	  tmp = aux4Prev; aux4Prev = aux4Curr; aux4Curr = aux4Next; aux4Next = tmp;
	  tmp = auxCurr; auxCurr = auxNext; auxNext = tmp;
	}
	
	/* last slice: result */
	cz = (double*)auxCurr;   pz = (double*)aux4Prev;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( y = 1; y < dy1; y++ ) {
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
          for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
            *resBuf = (*cz + *pz) * coefFace;
          *resBuf = (*cz + *pz) * coefVertex;
          resBuf++; cz++; pz++;
        }
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
        for ( x = 1; x < dx1; x++, resBuf++, cz++, pz++ )
          *resBuf = (*cz + *pz) * coefVertex;
        *resBuf = (*cz + *pz) * coefCorner;
        resBuf++; cz++; pz++;
      }
      break;

    default :
      VT_Free( (void**)(&localBuf) );
      VT_Error( "unknown (or not supported) connectivity", proc );
      return( -1 );
    }
    input = resIm;
  }
  /* end */
  VT_Free( (void**)&localBuf );
  return( 1 );
}

