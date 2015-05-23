#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "imgur_config.h"
#include "resize_image.h"
#include "dbg.h"

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
{
	if(fif != FIF_UNKNOWN) {
	//	printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
		log_err("%s Format", FreeImage_GetFormatFromFIF(fif));
	}
//	printf(message);
	log_err("Image error: %s", message);
}

// const char *getExt (const char *fspec) {
// 	char *e = strrchr(fspec, '.');
// 	if (e == NULL)
// 		e = "";
// 	return e;
// }

/*
 * resizes a picture so that it fits in a square of size IMAGE_SQUARE_SIZE x IMAGE_SQUARE_SIZE
 */

int resize_jpeg(char *inFileName, BYTE **ret_buffer, DWORD *size_in_bytes)
{
	BYTE *mem_buffer;
	FIBITMAP *dib, *new_dib;
	FREE_IMAGE_FORMAT fif;
	FIMEMORY *hmem = NULL;
	int new_width, new_height, orig_width, orig_height, flags;
	double ratio;

	log_info("Begining resample for %s", inFileName);

	fif = FreeImage_GetFileType(inFileName, 0);
	switch(fif) {
		case FIF_JPEG:
			flags = JPEG_EXIFROTATE | JPEG_ACCURATE;
			break;
		case FIF_BMP:
			flags = BMP_DEFAULT;
			break;
		case FIF_PNG:
			flags = PNG_DEFAULT;
			break;
		default:
			flags = 0;
	}

	dib = FreeImage_Load(fif, inFileName, flags);
	throw_error(!dib, IMAGE_NOT_LOADED, "Could not load image %s", inFileName);

	orig_width = FreeImage_GetWidth(dib);
	orig_height = FreeImage_GetHeight(dib);
	ratio = (double) orig_width / orig_height;

	if(orig_width <= IMAGE_SQUARE_SIZE && orig_height <= IMAGE_SQUARE_SIZE) {
		new_width = orig_width;
		new_height = orig_height;
	} else if(orig_width > orig_height) {
		new_width = IMAGE_SQUARE_SIZE;
		new_height = floor(IMAGE_SQUARE_SIZE / ratio);
	} else {
		new_height = IMAGE_SQUARE_SIZE;
		new_width = floor(IMAGE_SQUARE_SIZE * ratio);
	}

	new_dib = FreeImage_Rescale(dib, new_width, new_height, FILTER_BILINEAR);
	FreeImage_Unload(dib);
	hmem = FreeImage_OpenMemory(0, 0);
	FreeImage_SaveToMemory(FIF_PNG, new_dib, hmem, PNG_DEFAULT);
	FreeImage_Unload(new_dib);

	mem_buffer = NULL;
	FreeImage_AcquireMemory(hmem, &mem_buffer, size_in_bytes);
	throw_error(!(*ret_buffer = (BYTE*) malloc(*size_in_bytes * sizeof(BYTE))), OUT_OF_MEMORY, "Out of memory");

	memcpy(*ret_buffer, mem_buffer, *size_in_bytes * sizeof(BYTE));

	log_info("Resampled %s => MEMORY (%d bytes)", inFileName, *size_in_bytes);

	// TALVEZ ESSA CHAMADA ABAIXO SEJA UM PROBLEMA POIS PODE DAR PAU NO mem_buffer. VERIFICAR SE ISSO É UM PROBLEMA MESMO
	// FreeImage_CloseMemory(hmem);

	// log_info("RESAMPLED width %d, width %d\n", FreeImage_GetWidth(new_dib),FreeImage_GetHeight(new_dib));

	// FreeImage_SeekMemory(hmem, 0L, SEEK_SET);

	// FAZER ISSSO DO OUTRO LADO!!! FreeImage_CloseMemory(hmem);
	FreeImage_CloseMemory(hmem);
	return 1;
error:
	return 0;
}

