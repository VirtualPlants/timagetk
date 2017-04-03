#ifndef PNM_H
#define PNM_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

#include <ImageIO.h>


int readPnmImage(const char *name,_image *im);
int writePnmImage(char *name,  _image *im);
int testPnmHeader(char *magic,const char *name);
PTRIMAGE_FORMAT createPnmFormat();

#ifdef __cplusplus
}
#endif

#endif
