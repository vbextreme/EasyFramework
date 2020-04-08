#include <ef/image.h>
#include <ef/memory.h>
#include <ef/file.h>
#include <ef/err.h>

#include "qdbmp.h"

g2dImage_s* g2d_load_bmp(char const* path){	
	BMP* raw = BMP_ReadFile(path);
	BMP_STATUS ret = BMP_GetError();
	if( !raw || ret != BMP_OK ){
		if( raw ) BMP_Free(raw);
		if( ret == BMP_FILE_INVALID ){
			errno = 666;
			return NULL;
		}
		err_push("bmp(%d):%s", ret, BMP_GetErrorDescription());
		return NULL;
	}

	g2dImage_s* img = g2d_new(raw->Header.Width, raw->Header.Height, -1);
	if( !img ) err_fail("wtf");

	const unsigned bpp = raw->Header.BitsPerPixel >> 3;
	const unsigned strider = raw->Header.ImageDataSize / raw->Header.Height;
	const unsigned char* bmppix = raw->Data;
	const int palette = raw->Header.BitsPerPixel == 8;

	for( size_t y = 0; y < img->h; ++y ){
		const unsigned row = g2d_row(img, y);
		g2dColor_t* pix = g2d_color(img, row, 0);
		const unsigned bmprow = ((raw->Header.Height - y) - 1) * strider;
		for( size_t x = 0; x < img->w; ++x){
			const unsigned bmpx = x * bpp;
			if( palette ){
				unsigned char* paddr = raw->Palette + bmppix[bmpx] * 4;
				pix[x] = g2d_color_make(img, 255, paddr[2], paddr[1], paddr[0]);
			}
			else{
				pix[x] = g2d_color_make(img, 255, bmppix[bmprow+bmpx+2], bmppix[bmprow+bmpx+1], bmppix[bmprow+bmpx]);
			}
		}
	}	

	BMP_Free(raw);
	return img;
}

