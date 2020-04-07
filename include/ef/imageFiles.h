#ifndef __EF_IMAGE_FILES_H__
#define __EF_IMAGE_FILES_H__

#include <ef/image.h>

//TODO
//gif
//svg
//bmp

/** load image, supported png, jpg
 * @param path absolute path
 * @param width if 0 no scaling
 * @param height if 0 no scaling
 * @return new image or null for error
 */
g2dImage_s* g2d_load(char const* path, unsigned width, unsigned height);



#endif 
