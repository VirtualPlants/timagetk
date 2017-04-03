#include "inr.h"

#include <string.h>


static int _debug_ = 0;







/* Writes the given image body in an already opened file.*/
int _writeRawData(const _image *im) {
  char *proc = "_writeRawData";
  unsigned long size, nbv, nwrt, i;
  unsigned int v;
  unsigned char **vp;
  
  if(im->openMode != OM_CLOSE) {

    /* scalar or interlaced vectors */
    if(im->vectMode != VM_NON_INTERLACED) {
      size = im->xdim * im->ydim * im->zdim * im->vdim * im->wdim;
      if ( _debug_ ) 
        fprintf( stderr, "%s: write %lu bytes\n", proc, size );
      nwrt = ImageIO_write(im, im->data, size);
      if(nwrt != size) return -1;
      else return 1;
    }

    /* non interlaced vectors: interlace for saving */
    else {
      nbv = im->xdim * im->ydim * im->zdim;
      size = im->xdim * im->ydim * im->zdim * im->wdim;
      vp = (unsigned char **) ImageIO_alloc(im->vdim * sizeof(unsigned char *));
      for(v = 0; v < im->vdim; v++)
        vp[v] = (unsigned char *) im->data + v * size;
      for(i = 0; i < nbv; i++)
        for(v = 0; v < im->vdim; v++) {
          if(ImageIO_write(im, (const void *) vp[v], im->wdim) != im->wdim)
            return -1;
          vp[v] += im->wdim;
        }
      ImageIO_free(vp);
      return 1;
    }
  }
  else return -1;
}






int testRawHeader( char *magic __attribute__ ((unused)),
                   const char *name __attribute__ ((unused)) )
{
    return -1;
}


int writeRaw(char *name,_image *im) {
  int res;

  _openWriteImage( im, name );

  if(!im->fd) {
    fprintf(stderr, "writeRaw: error: unable to open file \'%s\'\n", name );
    return ImageIO_OPENING;
  }

  
  res = _writeRawData( im );
  if (res < 0) {
    fprintf(stderr, "writeRaw: error: unable to write data of \'%s\'\n",
            name);
    ImageIO_close( im );
    im->fd = NULL;
    im->openMode = OM_CLOSE;
    return( res );
  }

  ImageIO_close( im );
  im->fd = NULL;
  im->openMode = OM_CLOSE;

  return ( res );  
}

PTRIMAGE_FORMAT createRawFormat() {
  PTRIMAGE_FORMAT f=(PTRIMAGE_FORMAT) ImageIO_alloc(sizeof(IMAGE_FORMAT));

  f->testImageFormat=&testRawHeader;
  f->readImageHeader=0;
  f->readImageData=0;
  f->writeImage=&writeRaw;
  strcpy(f->fileExtension,".raw");
  strcpy(f->realName,"RawData");
  return f;
}
