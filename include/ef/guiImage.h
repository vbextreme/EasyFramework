#ifndef __EF_GUI_IMAGE_H__
#define __EF_GUI_IMAGE_H__

#include <ef/type.h>
#include <ef/image.h>
#include <ef/imageSvg.h>
#include <ef/imageGif.h>
#include <ef/media.h>

typedef struct guiLayer guiLayer_s;
typedef struct gui gui_s;
typedef void (*guiLayerFN_f)(gui_s* gui, guiLayer_s**, void* generic);
typedef void (*guiLayerFree_f)(void* generic);

typedef enum { GUI_LAYER_COLOR, GUI_LAYER_IMG, GUI_LAYER_SVG, GUI_LAYER_GIF, GUI_LAYER_VIDEO, GUI_LAYER_FN, GUI_LAYER_CUSTOM } guiLayerType_e;

#define GUI_LAYER_FLAGS_ALPHA 0x01
#define GUI_LAYER_FLAGS_PLAY  0x02
#define GUI_LAYER_FLAGS_LOOP  0x04
#define GUI_LAYER_FLAGS_PERC  0x08

typedef struct guiLayerPercentage{
	double x;
	double y;
	double w;
	double h;
}guiLayerPercentage_s;

typedef struct guiLayer{
	char* res;
	union{
		g2dColor_t color;
		g2dImage_s* img;
		gif_s* gif;
		media_s* video;
		guiLayerFN_f fn;
	};
	g2dCoord_s pos;
	g2dCoord_s src;
	guiLayerPercentage_s per;
	guiLayerType_e type;
	void* data;
	guiLayerFree_f free;
	unsigned frameid;
	unsigned flags;
}guiLayer_s;

typedef struct guiComposite{
	guiLayer_s** layers;
	unsigned flags;
}guiComposite_s;

guiLayer_s* gui_layer_color_new(g2dColor_t color, unsigned width, unsigned height, unsigned flags);
guiLayer_s* gui_layer_fn_new(guiLayerFN_f fn, void* data, guiLayerFree_f freefn, unsigned width, unsigned height, unsigned flags);
guiLayer_s* gui_layer_custom_new(g2dImage_s* g2d, unsigned flags);
guiLayer_s* gui_layer_new(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio);
guiLayer_s* gui_layer_load(g2dColor_t color, const char* pathRelative, unsigned width, unsigned height, unsigned flags, int ratio);
void gui_layer_xy_set(guiLayer_s* img, unsigned x, unsigned y);
void gui_layer_src_xy_set(guiLayer_s* img, unsigned x, unsigned y);
void gui_layer_perc_set(guiLayer_s* img, double x, double y, double w, double h);
void gui_layer_wh_set(guiLayer_s* img, unsigned w, unsigned h);
void gui_layer_redraw(gui_s* gui, guiComposite_s* cmp, unsigned id, unsigned count);
void gui_layer_free(guiLayer_s* img);
void gui_layer_resize(gui_s* gui, guiLayer_s* img, unsigned width, unsigned height, int ratio);

guiComposite_s* gui_composite_new(unsigned count);
void gui_composite_free(guiComposite_s* cmp);
guiComposite_s* gui_composite_add(guiComposite_s* cmp, guiLayer_s* img);
void gui_composite_remove(guiComposite_s* cmp, size_t id);
void gui_composite_replace(guiComposite_s* cmp, guiLayer_s* oldl, guiLayer_s* newl);
void gui_composite_redraw(gui_s* gui, guiComposite_s* cmp);
void gui_composite_resize(gui_s* gui, guiComposite_s* cmp, unsigned width, unsigned height);












#endif
