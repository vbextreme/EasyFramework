#ifndef __EF_GUI_IMAGE_H__
#define __EF_GUI_IMAGE_H__

#include <ef/type.h>
#include <ef/image.h>
#include <ef/imageGif.h>
#include <ef/media.h>

typedef struct guiImage guiImage_s;
typedef struct gui gui_s;
typedef void (*guiImageFN_f)(gui_s* gui, guiImage_s**, void* generic);
typedef void (*guiImageFree_f)(void* generic);

typedef enum { GUI_IMAGE_COLOR, GUI_IMAGE_IMG, GUI_IMAGE_GIF, GUI_IMAGE_VIDEO, GUI_IMAGE_FN, GUI_IMAGE_CUSTOM } guiImageType_e;

#define GUI_IMAGE_FLAGS_ALPHA 0x01
#define GUI_IMAGE_FLAGS_PLAY  0x02
#define GUI_IMAGE_FLAGS_LOOP  0x04

typedef struct guiImage{
	char* res;
	union{
		g2dColor_t color;
		g2dImage_s* img;
		gif_s* gif;
		media_s* video;
		guiImageFN_f fn;
	};
	g2dCoord_s pos;
	g2dCoord_s src;
	guiImageType_e type;
	void* data;
	guiImageFree_f free;
	unsigned frameid;
	unsigned flags;
}guiImage_s;

typedef struct guiComposite{
	guiImage_s** img;
	unsigned flags;
}guiComposite_s;

guiImage_s* gui_image_color_new(g2dColor_t color, unsigned width, unsigned height, unsigned flags);
guiImage_s* gui_image_fn_new(guiImageFN_f fn, void* data, guiImageFree_f freefn, unsigned width, unsigned height, unsigned flags);
guiImage_s* gui_image_custom_new(g2dImage_s* g2d, unsigned flags);
guiImage_s* gui_image_new(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio);
guiImage_s* gui_image_load(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio);
void gui_image_xy_set(guiImage_s* img, unsigned x, unsigned y);
void gui_image_src_xy_set(guiImage_s* img, unsigned x, unsigned y);
void gui_image_wh_set(guiImage_s* img, unsigned w, unsigned h);
void gui_image_redraw(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count);
void gui_image_free(guiImage_s* img);
void gui_image_resize(gui_s* gui, guiImage_s* img, unsigned width, unsigned height, int ratio);

guiComposite_s* gui_composite_new(unsigned count);
void gui_composite_free(guiComposite_s* cmp);
guiComposite_s* gui_composite_add(guiComposite_s* cmp, guiImage_s* img);
void gui_composite_redraw(gui_s* gui, guiComposite_s* cmp);
void gui_composite_resize(gui_s* gui, guiComposite_s* cmp, unsigned width, unsigned height);












#endif
