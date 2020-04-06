#ifndef __EF_IMAGE_FILES_H__
#define __EF_IMAGE_FILES_H__

#include <ef/image.h>

/** load image, supported png
 * @param path absolute path
 * @return new image or null for error
 */
g2dImage_s* g2d_load(char const* path);


#endif 
