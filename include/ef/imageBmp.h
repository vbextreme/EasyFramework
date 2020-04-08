#ifndef __EF_IMAGE_BMP_H__
#define __EF_IMAGE_BMP_H__

#include <ef/image.h>

/** load bmp image, set errno to 666 if not a bmp image*/
g2dImage_s* g2d_load_bmp(char const* path);

#endif 
