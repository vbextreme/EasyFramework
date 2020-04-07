#ifndef __EF_IMAGE_JPEG_H__
#define __EF_IMAGE_JPEG_H__

#include <ef/image.h>

/** load jpeg image, set errno to 666 if not a jpeg image*/
g2dImage_s* g2d_load_jpeg(char const* path);

#endif 
