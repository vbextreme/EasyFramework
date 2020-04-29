#ifndef __EF_GUI_RESOURCES_H__
#define __EF_GUI_RESOURCES_H__

#include <ef/type.h>
#include <ef/image.h>
#include <ef/imageGif.h>
#include <ef/guiImage.h>
#include <ef/media.h>
#include <ef/ft.h>
#include <ef/utf8.h>

typedef enum {
	GUI_RESOURCE_LONG,
	GUI_RESOURCE_DOUBLE,
	GUI_RESOURCE_UTF,
	GUI_RESOURCE_TEXT,
	GUI_RESOURCE_COLOR,
	GUI_RESOURCE_POSITION,
	GUI_RESOURCE_IMG,
	GUI_RESOURCE_GIF,
	GUI_RESOURCE_MEDIA,
	GUI_RESOURCE_FONTS
}guiResource_e;

typedef struct guiResource{
	union{
		long l;
		double d;
		utf_t utf;
		utf8_t* text;
		g2dColor_t color;
		g2dCoord_s position;
		g2dImage_s* img;
		gif_s* gif;
		media_s* media;
		guiImage_s* image;
		ftFonts_s* fonts;
	};
	guiResource_e type;
	unsigned reference;
}guiResource_s;

/** init resource*/
void gui_resources_init(void);

/** free resource*/
void gui_resources_free(void);

void gui_resource_long_new(const char* name, long value);

void gui_resource_double_new(const char* name, double value);

void gui_resource_utf_new(const char* name, utf_t value);

void gui_resource_text_new(const char* name, const utf8_t* value);

void gui_resource_color_new(const char* name, g2dColor_t value);

void gui_resource_position_new(const char* name, g2dCoord_s* value);

void gui_resource_img_new(const char* name, g2dImage_s* value);

void gui_resource_gif_new(const char* name, gif_s* value);

void gui_resource_media_new(const char* name, media_s* value);

void gui_resource_fonts_new(const char* name, ftFonts_s* value);

#define gui_resource_new(NAME, TYPE) _Generic((TYPE),\
	long         : gui_resource_long_new,\
	double       : gui_resource_double_new,\
	utf8_t*      : gui_resource_text_new,\
	const utf8_t*: gui_resource_text_new,\
	g2dColor_t   : gui_resource_color_new,\
	g2dCoord_s   : gui_resource_position_new,\
	g2dImage_s*  : gui_resource_img_new,\
	gif_s*       : gui_resource_gif_new,\
	media_s*     : gui_resource_media_new,\
	ftFonts_s*   : gui_resource_fonts_new\
)(NAME, TYPE)

guiResource_s* gui_resource(const char* name);

void gui_resource_release(const char* name);

#endif
