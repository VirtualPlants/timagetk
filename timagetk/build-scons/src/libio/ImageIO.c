
#ifdef WIN32
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <ImageIO.h>

/* formats actuellement lus 

   format     |   extension(s)   |  lecture  | ecriture 
   INRIMAGE   |  .inr[.gz]       |     X     |     X     -> + .gradient[.gz] + .gradient_direction[.gz]
   GIS        |  .dim, .ima[.gz] |     X     |     X
   ANALYZE    |  .hdr, .img[.gz] |     X     |     X
   PNM        |  .ppm, .pgm      |     X     |     X
   GIF        |  .gif            |     X     |     
   BMP        |  .bmp            |     X     |
   RAW        |  .raw            |           |     X
*/
#include "inr.h"
#include "gif.h"
#include "pnm.h"
#include "bmp.h"
#include "iris.h"
#include "analyze.h"
#include "raw.h"
#ifdef MINC_FILES
#  include "mincio.h"
#endif


static int _ImageIO_debug_ = 0;
static int _ImageIO_verbose_ = 0;


/** the first file format is initialized to null */
static PTRIMAGE_FORMAT firstFormat=NULL;

/** the Inrimage file format (default format) is initialized to null */
static PTRIMAGE_FORMAT InrimageFormat=NULL;




void _SetVerboseInImageIO( int v )
{
  _ImageIO_verbose_ = v;
}

void _IncrementVerboseInImageIO(  )
{
  _ImageIO_verbose_ ++;
}

void _SetDebugInImageIO( int d )
{
  _ImageIO_debug_ = d;
}

void _IncrementDebugInImageIO(  )
{
  _ImageIO_debug_ ++;
}




/*--------------------------------------------------
 *
 * mimics standard routines
 *
 --------------------------------------------------*/




/* default allocation routine */
static void *(*allocRoutine)(size_t) = 0;
/* default deallocation routine */
static void (*deleteRoutine)(void *) = 0;

void *ImageIO_alloc(size_t s) {
  if(!allocRoutine) allocRoutine = malloc;
  return ( (*allocRoutine)(s) );
}
/* call allocation routine */
void ImageIO_free(void *m) {
  if(!deleteRoutine) deleteRoutine = free;
  (*deleteRoutine)(m);
}
/* call deallocation routine */



/* mimics fwrite() function.
   According to _openWriteImage(), openMode will has one 
   of the following value:
   - OM_STD (for stdout)
   - OM_GZ 
   - OM_FILE
*/
size_t ImageIO_write(const _image *im, const void *buf, size_t len) 
{
  char *proc = "ImageIO_write";
  size_t to_be_written = len;
  size_t stepWriteMax = INT_MAX - 1;
  size_t l;
  char *b = (char*)buf;

  size_t stepWrite = (stepWriteMax < to_be_written) ? stepWriteMax : to_be_written;  

  if ( _ImageIO_debug_ )
    fprintf( stderr, "%s: length to be written = %lu ; stepWrite =  %lu bytes ; \n", proc, to_be_written, stepWrite );

  switch(im->openMode) {
  default :
  case OM_CLOSE :
    return 0;
  case OM_STD :
#ifdef ZLIB
    while ( (to_be_written > 0) && ((l = gzwrite(im->fd, (void *) b, stepWrite)) > 0) ) {
      to_be_written -= l;
      stepWrite = (stepWriteMax < to_be_written) ? stepWriteMax : to_be_written;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: gzwrite has written %lu, remains %lu, step=%lu\n", proc, l, to_be_written, stepWrite );
      b += l;
    }
#else 
    while ( (to_be_written > 0) && ((l = fwrite(b, 1, stepWrite, (FILE*)im->fd)) > 0) ) {
      to_be_written -= l;
      stepWrite = (stepWriteMax < to_be_written) ? stepWriteMax : to_be_written;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: fwrite has written %lu, remains %lu, step=%lu\n", proc, l, to_be_written, stepWrite );
      b += l;
    }
#endif
    return ( len - to_be_written );
    break;
#ifdef ZLIB
  case OM_GZ :
    while ( (to_be_written > 0) && ((l = gzwrite(im->fd, (void *) b, stepWrite)) > 0) ) {
      to_be_written -= l;
      stepWrite = (stepWriteMax < to_be_written) ? stepWriteMax : to_be_written;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: gzwrite has written %lu, remains %lu, step=%lu\n", proc, l, to_be_written, stepWrite );
      b += l;
    }
    return ( len - to_be_written );
    break;
#endif
  case OM_FILE:
    while ( (to_be_written > 0) && ((l = fwrite(b, 1, stepWrite, (FILE*)im->fd)) > 0) ) {
      to_be_written -= l;
      stepWrite = (stepWriteMax < to_be_written) ? stepWriteMax : to_be_written;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: fwrite has written %lu, remains %lu, step=%lu\n", proc, l, to_be_written, stepWrite );
      b += l;
    }
    return ( len - to_be_written );
    break;
  }
  return 0;
}



/* mimics fread() function.
   According to _openReadImage(), openMode will has one 
   of the following value:
   - OM_STD (for stdin)
   - OM_GZ *or* OM_FILE
*/
size_t ImageIO_read(const _image *im, void *buf, size_t len) 
{
  char *proc = "ImageIO_read";
  size_t to_be_read = len;
  size_t stepReadMax = INT_MAX - 1;
  size_t l;
  char *b = (char*)buf;
  
  size_t stepRead = (stepReadMax < to_be_read) ? stepReadMax : to_be_read;

  if ( _ImageIO_debug_ )
    fprintf( stderr, "%s: length to be read = %lu ; stepRead =  %lu bytes ; \n", proc, to_be_read, stepRead );

  switch(im->openMode) {
  default :
  case OM_CLOSE :
    return 0;
  case OM_STD :
#ifdef ZLIB
    while ( (to_be_read > 0) && ((l = gzread(im->fd, (void *) b, stepRead)) > 0) ) {
      to_be_read -= l;
      stepRead = (stepReadMax < to_be_read) ? stepReadMax : to_be_read;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: gzread has read %lu, remains %lu, step=%lu\n", proc, l, to_be_read, stepRead );
      b += l;
    }
#else 
    while ( (to_be_read > 0) && ((l = fread( b, 1, stepRead, im->fd )) > 0) ) {
      to_be_read -= l;
      stepRead = (stepReadMax < to_be_read) ? stepReadMax : to_be_read;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: fread has read %lu, remains %lu, step=%lu\n", proc, l, to_be_read, stepRead );
      b += l;
    }
#endif
    return ( len - to_be_read );
    break;
#ifdef ZLIB
  case OM_GZ :
    while ( (to_be_read > 0) && ((l = gzread(im->fd, (void *) b, stepRead)) > 0) ) {
      to_be_read -= l;
      stepRead = (stepReadMax < to_be_read) ? stepReadMax : to_be_read;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: gzread has read %lu, remains %lu, step=%lu\n", proc, l, to_be_read, stepRead );
      b += l;
    }
    return ( len - to_be_read );
    break;
#else
  case OM_FILE :
    while ( (to_be_read > 0) && ((l = fread( b, 1, stepRead, im->fd )) > 0) ) {
      to_be_read -= l;
      stepRead = (stepReadMax < to_be_read) ? stepReadMax : to_be_read;
      if ( _ImageIO_debug_ )
        fprintf( stderr, "%s: fread has read %lu, remains %lu, step=%lu\n", proc, l, to_be_read, stepRead );
      b += l;
    }
    return ( len - to_be_read );
    break;
#endif
  }

  return 0;
}



/* mimics fgets() function.
   According to _openReadImage(), openMode will has one 
   of the following value:
   - OM_STD (for stdout)
   - OM_GZ *or* OM_FILE
*/
char *ImageIO_gets( const _image *im, char *str, int size )
{
  char *ret = NULL;
  switch(im->openMode) {
  default :
  case OM_CLOSE :
    return NULL;
  case OM_STD :
#ifdef ZLIB
    ret = (char *) gzgets(im->fd, str, size );
#else
    ret = fgets(str, size, im->fd);
#endif
    break;
#ifdef ZLIB
  case OM_GZ :
    ret = (char *) gzgets(im->fd, str, size);
    break;
#else
  case OM_FILE :
    ret = fgets(str, size, im->fd);
    break;
#endif
  }
  return ret;
}



int ImageIO_seek( const _image *im, long offset, int whence ) {
  switch(im->openMode) {
  case OM_CLOSE :
  default :
    return -1;
#ifdef ZLIB
  case OM_GZ:
    return gzseek(im->fd, offset, whence );
#endif
  case OM_FILE:
    return fseek( (FILE*)im->fd, offset, whence );
  }
}

/* return non 0 in case of error
 */
int ImageIO_error( const _image *im )
{
  static int errnum;
  switch(im->openMode) {
  case OM_CLOSE :
  default :
    return 0;
#ifdef ZLIB
  case OM_GZ :
    (void)gzerror(im->fd, &errnum);
    return( (errnum != Z_OK) || gzeof(im->fd) );
#endif
  case OM_FILE :
    return( ferror( (FILE*)im->fd ) || feof( (FILE*)im->fd ) );
  }
  return 0;
}



/* Upon successful completion 0 is returned.
   
   Closing the standard output with gzclose()
   is necessary as it will flush the pending output.
 */
int ImageIO_close( _image* im )
{
  int ret=0;
  
  switch ( im->openMode ) {
  default :
  case OM_CLOSE :
    break;
#ifdef ZLIB
  case OM_GZ :
  case OM_STD :
    ret = gzclose( im->fd );
    break;
#else 
  case OM_STD :
    break;
#endif
  case OM_FILE :
    ret = fclose( (FILE*)im->fd );
  }
  im->fd = NULL;
  im->openMode = OM_CLOSE;
  
  return ret;
}







/* given an initialized file descriptor and a file name, 
   open file from stdin (if name == NULL, or name == "-", or name == "<"), 
   or a standard/gzipped file otherwise (gzipped files are handled assuming
   that it is compiled and linked with zlib). 
   openMode will have one of the following value:
   - OM_STD (for stdin)
   - OM_GZ *or* OM_FILE
*/
void _openReadImage(_image* im, const char *name) {
  if(im->openMode == OM_CLOSE) {

    /* open from stdin */
    if( name == NULL || name[0] == '\0' 
        || (name[0] == '-' && name[1] == '\0')
        || (name[0] == '<' && name[1] == '\0') ) {
#ifdef ZLIB      
      im->fd = gzdopen(fileno(stdin), "rb");
#else
      im->fd = fdopen(fileno(stdin), "rb");
#endif
      im->openMode = OM_STD;
    }

    else {
#ifdef ZLIB     
      im->fd = gzopen(name, "rb");
      if(im->fd) im->openMode = OM_GZ;
#else
      im->fd = fopen(name, "rb");
      if(im->fd) im->openMode = OM_FILE;
#endif

    }

  }
}





/* given an initialized file descriptor and a file name, 
   open file from stdout (if name == NULL, or name == "-", or name == ">"), 
   a gzipped pipe (if name got the extension ".gz")
   or a standard file otherwise.
   openMode will have one of the following value:
   - OM_STD (for stdout)
   - OM_GZ 
   - OM_FILE
*/
void _openWriteImage(_image* im, const char *name) 
{
  im->openMode = OM_CLOSE;

  if( name == NULL || name[0] == '\0' 
      || (name[0] == '-' && name[1] == '\0') 
      || (name[0] == '>' && name[1] == '\0') ) {

#ifdef ZLIB
#if (defined _LINUX_) || (defined _SOLARIS_) || (defined _SGI_)
    im->fd = gzdopen(1, "wb");
#else
    im->fd = gzdopen(fileno(stdout), "wb");
#endif
#else    
    im->fd = (_ImageIO_file) stdout;
#endif
    im->openMode = OM_STD;
  }

  else{
#ifdef ZLIB      
    
    /* from gzopen() doc:
       ... The mode parameter is as in fopen ("rb" or "wb") but can
       also include a compression level ("wb9") or a strategy: 'f' for
       filtered data as in "wb6f", 'h' for Huffman only compression as
       in "wb1h" ...
       However, a small .gz header will be written ... thus gz(d)open can not
       be used for regular files.
    */
    
    if( !strncmp(name+strlen(name)-3, ".gz", 3) )
      {
#ifdef _MSC_VER
        int ffd=_open(name,_O_RDWR | _O_CREAT| _O_TRUNC | _O_BINARY, _S_IREAD|_S_IWRITE);
        im->fd = gzdopen( ffd, "wb" );
#else
        im->fd = gzopen( name, "wb" );
#endif
        im->openMode = OM_GZ;
      }
    else 
#endif
    {
      im->fd = (_ImageIO_file) fopen(name, "wb");
      im->openMode = OM_FILE;
    }
  }
}



/* set allocation and deallocation routines */
void setImageIOAllocationRoutines(ALLOCATION_FUNCTION alloc,
                                  DEALLOCATION_FUNCTION del) {
  if(alloc != NULL) allocRoutine = alloc;
  if(del != NULL) deleteRoutine = del;
}





/*--------------------------------------------------
 *
 *
 *
 --------------------------------------------------*/





ENDIANNESS _getEndianness()
{
  union {
    unsigned char uc[2];
    unsigned short us;
  } twobytes;
  twobytes.us = 255;
  /* on linux or dec
   */
  if ( twobytes.uc[1] == 0 ) return( END_LITTLE );
  /* on solaris or sgi
   */
  return( END_BIG );
}







/* Allocates and initializes an image descriptor */
_image *_initImage() {
  _image *im;

  im = (_image *) ImageIO_alloc(sizeof(_image));
  if ( im == NULL ) return( im );

  /* default image size is 1*1*1 */
  im->xdim = im->ydim = im->zdim = im->vdim = 1;
  /* default image voxel size is 1.0*1.0*1.0 */
  im->vx = im->vy = im->vz = 1.0;

  /* default image center  is 0 0 0 */
  im->cx = im->cy = im->cz = 0;

  /* default image offset  is 0 0 0 */
  im->tx = im->ty = im->tz = 0.0;

  /* default image rotation  is 0 0 0 */
  im->rx = im->ry = im->rz = 0.0;

  /* no data yet */
  im->data = NULL;

  /* no file associated to image */
  im->fd = NULL;
  im->openMode = OM_CLOSE;
  im->endianness = END_UNKNOWN;

  /* unknown data kind
     default is binary
   */
  im->dataMode = DM_BINARY;

  /* no user string */
  im->user = NULL;
  im->nuser = 0;

  /* unknown word kind */
  im->wdim = 0;
  im->wordKind = WK_UNKNOWN;
  im->vectMode = VM_SCALAR;
  im->sign = SGN_UNKNOWN;
  im->imageFormat = NULL;

  /** eventually initializes the supported file formats */
  if (firstFormat==NULL)
    initSupportedFileFormat();
  /* return image descriptor */
  return im;
}



/* Free an image descriptor */
void _freeImage(_image *im) {
  unsigned int i;

  if ( im == NULL ) return;

  /* close image if opened */
  if(im->openMode != OM_CLOSE) ImageIO_close(im);

  /* free data if any */
  if(im->data != NULL) ImageIO_free(im->data);
  im->data = NULL;

  /* free user string array if any */
  if( (im->nuser > 0) && (im->user != NULL) ) {
    for(i = 0; i < im->nuser; i++)
      if ( im->user[i] != NULL ) ImageIO_free(im->user[i]);
    ImageIO_free(im->user);
  }
  im->nuser = 0;
  im->user = NULL;

  /* free given descriptor */
  ImageIO_free(im);
}





/* Reads an image from a file and returns an image descriptor or NULL if
   reading failed.
   Reads from stdin if image name is NULL. */
_image* _readImage(const char *name) {
  _image *im;


  /* read header */
  im = _readImageHeader( name );

  if(im != NULL && im->openMode != OM_CLOSE) {
    /* read body */
    /* switch from generic reader to specific ones
       if(_readImageData(im) < 0) {
       GM Lun  6 mai 2013 12:19:28 CEST */
    if ( (*(im->imageFormat)->readImageData)(im) < 0 ) {
      if ( _ImageIO_verbose_ || _ImageIO_debug_ )
        fprintf(stderr, "_readImage: error: invalid data encountered in \'%s\'\n",
                name);
      _freeImage(im);
      return NULL;
    }
    ImageIO_close(im);
  }

  return im;
}



/* Reads an image from a file and returns an image descriptor or NULL if<br>
   reading failed.<br>
   Reads from stdin if image name is NULL.
   If the image is vectorial, it is uninterlaced. */
_image* _readNonInterlacedImage(const char *name) {
  _image *im;

  /* read header */
  im = _readImageHeader(name);

  if(im && im->openMode != OM_CLOSE) {
    /* read scalar image body */
    if(im->vdim == 1) {
      if(_readImageData(im) < 0) {
        if ( _ImageIO_verbose_ || _ImageIO_debug_ )
          fprintf(stderr, "_readImage: error: invalid data encountered in \'%s\'\n",
                  name);
        _freeImage(im);
        return NULL;
      }
    }
    /* read vectorial image body */
    else {
      im->vectMode = VM_NON_INTERLACED;
      if(_readNonInterlacedImageData(im) < 0) {
        if ( _ImageIO_verbose_ || _ImageIO_debug_ )
          fprintf(stderr, "_readImage: error: invalid data encountered in \'%s\'\n",
                  name);
        _freeImage(im);
        return NULL;
      }
    }
    ImageIO_close(im);
   }

  return im;
}











/* Write inrimage given in inr in file name. If file name's suffix is
   .gz, the image is gziped. If file name's suffix is .hdr, the image
   is written in ANALYZE format. If file name is NULL, image is written
   on stdout */
int _writeImage(_image *im, const char *name_to_be_written ) {

  int r = ImageIO_NO_ERROR;
  int length = 0;
  char *name = NULL;
  char *baseName = NULL;

  if ( im == NULL ) return -1;

  /* different conventions for the standard input
   */
  if ( name_to_be_written == NULL || name_to_be_written[0] == '\0' 
       || (name_to_be_written[0] == '-' && name_to_be_written[1] == '\0') 
       || (name_to_be_written[0] == '>' && name_to_be_written[1] == '\0') ) {
    name = NULL;
  }
  else {
    name = strdup( name_to_be_written );
  }

  initSupportedFileFormat();

  /* what is the wanted format
   */
  if ( name == NULL ) {
    im->imageFormat = InrimageFormat;
  } else {
    int i,extLength;
    PTRIMAGE_FORMAT f;
    char ext[IMAGE_FORMAT_NAME_LENGTH];
    char *ptr;
    
    
    /* scan all formats; */
    im->imageFormat=NULL;
    length=strlen(name);

    for(f=firstFormat;(f!=NULL)&& (im->imageFormat==NULL);f=f->next) {
      /* scan all extensions for that format */
      ptr=&f->fileExtension[0];

      do {
        /* get next file extension */
        i=0;
        for(i=0;((*ptr)!=',' && (*ptr)!='\0');i++,ptr++) {
          ext[i]=(*ptr);
        }
        if ((*ptr)==',') {
          ext[i]='\0';
          ptr++;
        }
        else {
          ext[i]='\0';
        }
        extLength=strlen(ext);

        /* test if the tail of name matches the extension */
        if ( (length > extLength) && (!strcmp( name + length - extLength, ext)) ) {
          im->imageFormat=f;
          /* copy original name and removes extension */
          baseName=strdup(name);
          for(i= length - extLength;i<length;++i)
            baseName[i]='\0';
        }

      } while (((*ptr)!='\0') && (im->imageFormat==NULL));
    }

    if (!im->imageFormat) { 
      if ( _ImageIO_verbose_ || _ImageIO_debug_ )
        fprintf(stderr, "_writeImage: warning : unknown extension in %s = assuming Inrimage\n",name);
      im->imageFormat=InrimageFormat;
      baseName=strdup(name);
    }
    else {
        if ( _ImageIO_verbose_ || _ImageIO_debug_ )
          fprintf(stderr, "_writeImage: uses '%s' format\n", im->imageFormat->realName );
    }
  }


  /* open file descriptor */
  /* _openWriteImage( im, name ) ; 
  
  

  if(!im->fd) {
     fprintf(stderr, "_writeImage: error: open failed\n");
     if ( name != NULL ) free( name );
     if ( baseName != NULL ) free( baseName );
     return ImageIO_OPENING;
  }
  */

  if (im->imageFormat) {

    if (im->imageFormat->writeImage==NULL) {
      im->imageFormat=InrimageFormat;
    }

    if ( 0 ) {
      if ( _ImageIO_verbose_ || _ImageIO_debug_ )
        fprintf(stderr, "_writeImage: will write '%s' with '%s' format\n",
                name, im->imageFormat->realName );
    }

    if ((*im->imageFormat->writeImage)(name, im)<0) {
      if ( _ImageIO_verbose_ || _ImageIO_debug_ )
        fprintf(stderr, "_writeImage: error: unable to write \'%s\'\n",
                name);
      r = ImageIO_WRITING_HEADER;
    } 
    
  }
  


  /* close file descriptor */
  ImageIO_close( im );

  im->fd = NULL;
  im->openMode = OM_CLOSE;

  if ( baseName != NULL ) free( baseName );
  if ( name != NULL ) free( name );
 
  return r;
}












/* read header from an image file 
   
   if standard input, it's an inrimage
   if not, get a magic string
   and try to find the good format

   if data are in a separate file,
   the header reading procedure will open
   the data file.

   error:
   0  success
   -1 unknown image type
   -2 error while opening
   -3 error while reading header
   -4 error while reading header or data
 */
_image *_readImageHeader( const char *name ) {
  int error = 0;
  return( _readImageHeaderAndGetError( name, &error ) );
}



_image *_readImageHeaderAndGetError( const char *name_to_be_read, int *error )
{
  char *proc = "_readImageHeaderAndGetError";
  _image *im;
  char magic[5];
  char *name = NULL;
  PTRIMAGE_FORMAT f;
  int res;

  *error = ImageIO_NO_ERROR;
  
  /* open image file */
  im = _initImage();
  if ( name_to_be_read == NULL || name_to_be_read[0] == '\0' 
       || (name_to_be_read[0] == '-' && name_to_be_read[1] == '\0') 
       || (name_to_be_read[0] == '<' && name_to_be_read[1] == '\0') ) {
    name = NULL;
  }
  else {
    name = malloc( strlen(name_to_be_read) + 1 );
    if ( name == (char*)NULL ) {
      fprintf( stderr, "%s: allocation error\n", proc );
      return NULL;
    }
    strncpy( name, name_to_be_read,  strlen(name_to_be_read) );
    name[ strlen(name_to_be_read) ] = '\0';
  }


  _openReadImage(im, name);

  if(!im->fd) {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr, "_readImageHeaderAndGetError: error: unable to open file \'%s\'\n", name);
    _freeImage(im);
    *error = ImageIO_OPENING;
    if ( name != NULL ) free( name );
    return NULL;
  }

  initSupportedFileFormat();
  
  /* what is the wanted format ?
     assume that stdin is inrimage
   */
  if(im->openMode == OM_STD) {
    im->imageFormat=InrimageFormat;
  }
  else {
   /* get magic string for disk files
    */
    ImageIO_read(im, magic, 4);
    magic[4] = '\0';
    ImageIO_seek(im, 0L, SEEK_SET);
    /** test each format */
    for(f=firstFormat;(f!=NULL)&& (im->imageFormat==NULL);f=f->next) {
        /* test whether 'testImageFormat' exists
         */
        if ( ! f->testImageFormat )
            continue;
        /* test if it is the correct format based on magic and file extension
         */
        if (((*f->testImageFormat)(magic, name)) >=0) {
          im->imageFormat=f;
      }
    }
  }

  if ( im->imageFormat == NULL ) {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr, "_readImageHeaderAndGetError: does not find image format for \'%s\'\n", name);
    ImageIO_close( im );
    _freeImage(im);
    *error = ImageIO_UNKNOWN_TYPE;
    if ( name != NULL ) free( name );
    return NULL;
  }

  /* now tests if the header can be read correctly */

  res=(*(im->imageFormat)->readImageHeader)(name,im);
  /* could read header only */ 
  if (res == 0) {
    if ( name != NULL ) free( name );
    return( im );
  } 
  /* could read header and data */
  else if ( res > 0 ) {
    ImageIO_close(im);
    if ( name != NULL ) free( name );
    return im;
  }  

  /* could not read error : throw error */
  if ( _ImageIO_verbose_ || _ImageIO_debug_ )
    fprintf(stderr, "_readImageHeaderAndGetError: an error occurs when reading image\n" );
  if ( name == NULL || im->openMode == OM_STD) {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr, "\t from \'standard input\'" );
  }
  else {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr, "\t from file \'%s\'", name );
  }
  if ( _ImageIO_verbose_ || _ImageIO_debug_ )
    fprintf(stderr, " using format \'%s\'\n", (im->imageFormat)->realName );
  ImageIO_close( im );
  _freeImage(im);
  *error = ImageIO_READING_HEADER;
  if ( name != NULL ) free( name );
  return NULL;

}







static void _swapImageData( _image *im ) 
{
  unsigned char *ptr1, *ptr2, b[8];
  unsigned short int si, *ptr3, *ptr4;
  unsigned int        i, *ptr5, *ptr6;
  unsigned int long size, length;
  
  if( _getEndianness() != im->endianness) {

    size = im->xdim * im->ydim * im->zdim * im->vdim * im->wdim;
    if ( size <= 0 ) return;

    length = size / im->wdim;
    ptr1 = ptr2 = (unsigned char *) im->data;
    
    /* 2 bytes swap */
    if(im->wdim == 2) {
      /*
        while(length--) {
        b[0] = *ptr1++;
        b[1] = *ptr1++;
        *ptr2++ = b[1];
        *ptr2++ = b[0];
        }
      */
      ptr3 = ptr4 = (unsigned short int *) im->data;
      while( length-- ) {
        si = *ptr3++;
        *ptr4++ = ((si >> 8) & 0xff) | (si << 8);
      }
    }
    
    /* 4 bytes swap */
    else if(im->wdim == 4) {
      /*
        while(length--) {
        b[0] = *ptr1++;
        b[1] = *ptr1++;
        b[2] = *ptr1++;
        b[3] = *ptr1++;
        *ptr2++ = b[3];
        *ptr2++ = b[2];
        *ptr2++ = b[1];
        *ptr2++ = b[0];
        }
      */
      ptr5 = ptr6 = (unsigned int *) im->data;
      while( length-- ) {
        i = *ptr5++;
        *ptr6++ =  (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | ((i >> 24) & 0xff);
      }
    }
    /* 8 bytes swap */
    else if(im->wdim == 8) {
      while(length--) {
        b[0] = *ptr1++;
        b[1] = *ptr1++;
        b[2] = *ptr1++;
        b[3] = *ptr1++;
        b[4] = *ptr1++;
        b[5] = *ptr1++;
        b[6] = *ptr1++;
        b[7] = *ptr1++;
        *ptr2++ = b[7];
        *ptr2++ = b[6];
        *ptr2++ = b[5];
        *ptr2++ = b[4];
        *ptr2++ = b[3];
        *ptr2++ = b[2];
        *ptr2++ = b[1];
        *ptr2++ = b[0];
      }
    }
  }
}






/* Read data of an inrimage.
   If im->data is not NULL, assume that the buffer was previously allocated
   Swap bytes depending on the endianness and the current architecture  */
int _readImageData(_image *im) {
  char *proc = "_readImageData";
  size_t size, nread;

  if(im->openMode != OM_CLOSE) {
    if ( _ImageIO_debug_ ) {
      fprintf( stderr, "%s: %lu * %lu * %lu * %d *%d\n", proc, im->xdim, im->ydim, im->zdim, im->vdim, im->wdim );
      }
    size = (size_t)im->xdim * (size_t)im->ydim * (size_t)im->zdim * (size_t)im->vdim * (size_t)im->wdim;
    
    if ( size <= 0 ) return -3;

    if(!im->data) {
      im->data = (unsigned char *) ImageIO_alloc(size);
      if ( _ImageIO_debug_ ) {
        fprintf( stderr, "%s: allocate %lu bytes at %p\n", proc, size, im->data );
      }

      if(!im->data) return -2;
    }

    nread = ImageIO_read(im, im->data, size);
    if(nread != size) return -1;


    /* architecture is big endian and data little endian 
       length = nb of points
     */
    _swapImageData( im );


  }

  return 1;
}





/* Read data of a vectorial inrimage, making the resulting buffer non-
   inerlaced.
   If im->data is not NULL, assume that the buffer was previously allocated
   Swap bytes depending on the endianness and the current architecture. */
int _readNonInterlacedImageData(_image *im) {
  unsigned long size, nread;
  unsigned char **vp, *buf;
  unsigned int i, j, k, v, w;

  if(im->vdim == 1) return _readImageData(im);
  
  if(im->openMode != OM_CLOSE) {
    size = im->xdim * im->ydim * im->zdim * im->vdim * im->wdim;

    if ( size <= 0 ) return -3;

    if(!im->data) {
      im->data = (unsigned char *) ImageIO_alloc(size);
      if(!im->data) return -2;
    }

    vp = (unsigned char **) ImageIO_alloc(im->vdim * sizeof(unsigned char *));
    buf = (unsigned char *) ImageIO_alloc(im->vdim * im->wdim);
    size = im->xdim * im->ydim * im->zdim * im->wdim;
    for(v = 0; v < im->vdim; v++)
      vp[v] = (unsigned char *) im->data + v * size;

    for(k = 0; k < im->zdim; k++) {
      for(j = 0; j < im->ydim; j++) {
        for(i = 0; i < im->xdim; i++) {
          nread = ImageIO_read(im, buf, im->vdim * im->wdim);
          if(nread != im->vdim * im->wdim) return -1;
          for(v = 0; v < im->vdim; v++)
            for(w = 0; w < im->wdim; w++)
              *vp[v]++ = *buf++;
          buf -= im->vdim * im->wdim;
        }
      }
    }

    ImageIO_free(buf);
    ImageIO_free(vp);

    /* architecture is big endian and data little endian */
    _swapImageData( im );

    
    /* reorder lines */
    /* no non-interlaced data for ANALYZE. But if ever... */
/*     if( im->imageFormat == IF_ANALYZE ) { */
/*        int v ; */
/*        int vdim = im->vdim ; */
/*        int lineSize = im->wdim * im->xdim ; */
/*        int vsize = lineSize * im->ydim * im->zdim ; */
/*        char* swapped = ImageIO_alloc(lineSize) ; */
/*        for( v = 0 ; v < vdim ; ++v ) */
/*        { */
/*           char* buf1 = (char*)im->data + v*vsize ; */
/*           char* buf2 = buf1 + vsize - lineSize ; */

/*           while( buf1 < buf2 ) */
/*           { */
/*              memcpy( swapped, buf1, lineSize ) ; */
/*              memcpy( buf1, buf2, lineSize ) ; */
/*              memcpy( buf2, swapped, lineSize ) ; */
/*              buf1 += lineSize ; */
/*              buf2 -= lineSize ; */
/*           } */

/*           ImageIO_free( swapped ) ; */
/*        } */
/*     } */
  }

  return 1;
}


/* Reads body from a non-interlaced vectorial inrimage whose header has
   been read by _readImageHeader. The image buffer is interlaced. */
int _readNonInterlacedFileData(_image *im) {
  unsigned long size, nread;
  unsigned char *ptr1, *vp, *buf;
  unsigned int i, j, k, v, w;

  if(im->vdim == 1) return _readImageData(im);

  if(im->openMode != OM_CLOSE) {
    size = im->xdim * im->ydim * im->zdim * im->vdim * im->wdim;

    if ( size <= 0 ) return -3;

    if(!im->data) {
      im->data = (unsigned char *) ImageIO_alloc(size);
      if(!im->data) return -2;
    }

    size = im->xdim * im->ydim * im->zdim * im->wdim;
    buf = ptr1 = (unsigned char *) ImageIO_alloc(size);

    for(v = 0; v < im->vdim; v++) {
      buf = ptr1;
      nread = ImageIO_read(im, buf, size);
      if(nread != size) return -1;
      vp = (unsigned char *) im->data + (v * im->wdim);
      for(k = 0; k < im->zdim; k++) {
        for(j = 0; j < im->ydim; j++) {
          for(i = 0; i < im->xdim; i++) {
            for(w = 0; w < im->wdim; w++) *vp++ = *buf++;
            vp += (im->vdim - 1) * im->wdim;
          }
        }
      }
    }

    ImageIO_free(buf);

    /* architecture is big endian and data little endian */
    _swapImageData( im );
    
    
    /* reorder lines */
    /* no non-interlaced data for ANALYZE. But if ever... */
/*     if( im->imageFormat == IF_ANALYZE ) { */
/*        int v ; */
/*        int vdim = im->vdim ; */
/*        int lineSize = im->wdim * im->xdim ; */
/*        int vsize = lineSize * im->ydim * im->zdim ; */
/*        char* swapped = ImageIO_alloc(lineSize) ; */
/*        for( v = 0 ; v < vdim ; ++v ) */
/*        { */
/*           char* buf1 = (char*)im->data + v*vsize ; */
/*           char* buf2 = buf1 + vsize - lineSize ; */

/*           while( buf1 < buf2 ) */
/*           { */
/*              memcpy( swapped, buf1, lineSize ) ; */
/*              memcpy( buf1, buf2, lineSize ) ; */
/*              memcpy( buf2, swapped, lineSize ) ; */
/*              buf1 += lineSize ; */
/*              buf2 -= lineSize ; */
/*           } */

/*           ImageIO_free( swapped ) ; */
/*        } */
/*     } */
  }

  return 1;  
}










/*--------------------------------------------------
 *
 * ?????
 *
 --------------------------------------------------*/





/* check the type of image in fileName */
PTRIMAGE_FORMAT imageType(const char *fileName) {
  _ImageIO_file f;
  char magic[5];
  PTRIMAGE_FORMAT format;

  if(!fileName) {
#ifdef ZLIB    
    f = gzdopen(fileno(stdin), "rb");
#else
    f = fdopen(fileno(stdin), "rb");
#endif
  }
  else {
#ifdef ZLIB      
    f = gzopen(fileName, "rb");
#else
    f = fopen(fileName, "rb");
#endif
  }

  if(!f) return NULL;
  
#ifdef ZLIB
  gzread( f, (void *) magic, 4);
#else
  fread( (void *) magic, 1, 4, f );
#endif


  magic[4] = '\0';

#ifdef ZLIB 
  gzclose( f );
#else
  if(fileName) fclose( f );
#endif

  if (firstFormat==NULL)
    initSupportedFileFormat();

  for(format=firstFormat;(format!=NULL);format=format->next) {
    /* test if it is the correct header based on magic and file extension */
    if (((*format->testImageFormat)(magic,fileName)) >=0) {
      return format;
    }
  }
  return 0;

}





/*--------------------------------------------------
 *
 * Image Format Management
 *
 --------------------------------------------------*/





/** adds a format at the beginning of the list of image formats. 
    Test if all mandatory fields have been filled */
int addImageFormat( PTRIMAGE_FORMAT format) 
{

  if ( ! format->testImageFormat ) {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr,"addImageFormat: no test format procedure in %s\n",
        format->realName);
    return -1;
  }

  if ( format->writeImage ||
       ( (format->readImageHeader)
         && (strlen(format->fileExtension)>0)
         && (strlen(format->realName)>0) ) ) {

    format->next=firstFormat;
    firstFormat=format;
    
    return 0;

  } 
  else {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr,"addImageFormat: information missing in file format %s\n",
              format->realName);
    return -1;
  }
}

/** adds a format at the end of the list of image formats. 
    Test if all mandatory fields have been filled */
int addImageFormatAtEnd( PTRIMAGE_FORMAT format) 
{
  PTRIMAGE_FORMAT f;

  if ( ! format->testImageFormat ) {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr,"addImageFormatAtEnd: no test format procedure in %s\n",
        format->realName);
    return -1;
  }

  if ( format->writeImage ||
       ( (format->readImageHeader)
         && (strlen(format->fileExtension)>0)
         && (strlen(format->realName)>0) ) ) {

    format->next = NULL;
    
    if (firstFormat == NULL) {
      firstFormat=format;
    }
    else {
      for(f=firstFormat;(f->next!=NULL);f=f->next)
        ;
      f->next=format;    
    }
    
    return 0;

  } 
  else {
    if ( _ImageIO_verbose_ || _ImageIO_debug_ )
      fprintf(stderr,"addImageFormatAtEnd: information missing in file format %s\n",
              format->realName);
    return -1;
  }
}


/** creates supported image formats */
void initSupportedFileFormat() 
{
  PTRIMAGE_FORMAT f;
  if ( InrimageFormat == NULL ) {
    f = createAnalyzeFormat();
    addImageFormatAtEnd( f );
    f = createBMPFormat();
    addImageFormatAtEnd( f );
    f = createGifFormat();
    addImageFormatAtEnd( f );
    f = createIrisFormat();
    addImageFormatAtEnd( f );
    f = createPnmFormat();
    addImageFormatAtEnd( f );
    f = createRawFormat();
    addImageFormatAtEnd( f );
    InrimageFormat = createInrimageFormat();
    addImageFormat( InrimageFormat );
  }
}



PTRIMAGE_FORMAT firstImageFormat() {
  return firstFormat;
}



/** prints supported image formats */
void printSupportedFileFormat() {    
  PTRIMAGE_FORMAT f;
  int i;

  initSupportedFileFormat();

  for(i=0, f=firstFormat;(f!=NULL);i++, f=f->next) {
    if ( (f->testImageFormat) &&
         (f->readImageHeader) &&
         (strlen(f->fileExtension)>0) &&
         (strlen(f->realName)>0)) {
      fprintf( stderr, "#%2d: format name ='%s', extensions='%s'",
              i, f->realName, f->fileExtension );
      if (f->readImageHeader) 
        fprintf( stderr, ", read" );
      if (f->writeImage) 
        fprintf( stderr, ", write" );
      fprintf( stderr, "\n" );
    }
  }
}


/** remove supported image formats */
void removeSupportedFileFormat() {    
  PTRIMAGE_FORMAT f=firstFormat;
  
  while( f != NULL) {
    PTRIMAGE_FORMAT f_old = f;
    f = f->next;
    ImageIO_free( f_old);
  }
  InrimageFormat=NULL;

}

