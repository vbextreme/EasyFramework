#ifndef __EF_IMAGE_PNG_H__
#define __EF_IMAGE_PNG_H__

#include <ef/image.h>

/** load png image, set errno to 666 if not a png image*/
g2dImage_s* g2d_load_png(char const* path);

#endif 
