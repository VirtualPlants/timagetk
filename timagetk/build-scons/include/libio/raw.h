
#ifndef RAW_H
#define RAW_H


#include <ImageIO.h>


#ifdef __cplusplus
extern "C" {
#endif



/** Writes the given image body in an already opened file.
    @param im image descriptor */
int _writeRawData(const _image *im);

/** test if an image file has the raw format
    @return -1 */
int testRawHeader(char *magic,const char *filename);

/** calls _writeInrimageHeader and _writeInrimageData
    @param basename the file name without any extension 
    @param im structure
    @return same as  _writeInrimageHeader*/
int writeRaw(char *basename,_image *im);

/** creates an return the file format structure associated with the Inrimage file format */
PTRIMAGE_FORMAT createRawFormat();

#ifdef __cplusplus
}
#endif

#endif
