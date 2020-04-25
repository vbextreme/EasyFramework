#ifndef __EF_MEDIA_H__
#define __EF_MEDIA_H__

#include <ef/image.h>

typedef struct media media_s;

/** free media*/
void media_free(media_s* media);

/** load media*/
media_s* media_load(const char* path);

/** set resize media*/
void media_resize_set(media_s* media, g2dImage_s* img);

/** decode a frame*/
int media_decode(media_s* media);

/** get frame*/
g2dImage_s* media_frame_get(media_s* media);

/** sleep for sync*/
void media_sleep(media_s* media);

/** get time for sync*/
long media_delay_get(media_s* media);

/** get width*/
unsigned media_width(media_s* media);

/** get height */
unsigned media_height(media_s* media);

/** get durate in ms */
double media_duration(media_s* media);

/** get fps*/
double media_fps(media_s* media);

/** seeking to ms from start*/
void media_seek_to(media_s* media, double s);

/** seeking from current to +- ms*/
void media_seek(media_s* media, long timems);

#endif 
