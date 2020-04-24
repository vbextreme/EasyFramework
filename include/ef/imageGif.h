#ifndef __EF_IMAGE_GIF_H__
#define __EF_IMAGE_GIF_H__

#include <ef/image.h>

typedef struct gifFrame{
	g2dCoord_s pos;
	g2dImage_s* img;  /**< image to display*/
	unsigned delay;   /**< delay in ms*/
	unsigned dispose;
	int transindex;
}gifFrame_s;

typedef struct gif{
	unsigned width;
	unsigned height;
	gifFrame_s* frames; /**< vector of gifFrame_s*/
}gif_s;

gif_s* g2d_load_gif(char const* path);

void g2d_gif_free(gif_s* gif);

void g2d_gif_resize(gif_s* gif, unsigned width, unsigned height, int ratio);

#endif 
