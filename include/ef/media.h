#ifndef __EF_MEDIA_H__
#define __EF_MEDIA_H__

#include <ef/image.h>

typedef struct media media_s;

void media_free(media_s* media);

media_s* media_load(const char* path);

void media_resize_set(media_s* media, g2dImage_s* img);

int media_decode(media_s* media);

g2dImage_s* media_frame_get(media_s* media);

void media_sleep(media_s* media);

long media_delay_get(media_s* media);

#endif 
