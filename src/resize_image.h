#ifndef __RESIZE_IMAGE_h
#define __RESIZE_IMAGE_h 1

#include <FreeImage.h>
int resize_jpeg(char *inFileName, BYTE **mem_buffer, DWORD *size_in_bytes);
void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message);

#endif
