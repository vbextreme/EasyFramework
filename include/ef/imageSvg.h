#ifndef __EF_IMAGE_SVG_H__
#define __EF_IMAGE_SVG_H__

#include <ef/image.h>

typedef struct svg{
	size_t size;
	unsigned char* memblock;
	void* rsvgHandle;
	unsigned dimW;
	unsigned dimH;
}svg_s;

/** load svg image*/
svg_s* svg_load(char const* path);

/** render svg*/
g2dImage_s* svg_render(svg_s* svg, unsigned width, unsigned height);

/** free svg*/
void svg_free(svg_s* svg);


#endif 
