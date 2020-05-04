#ifndef __EF_GUI_CAPTION_H__
#define __EF_GUI_CAPTION_H__

#include <ef/gui.h>

#define GUI_CAPTION_RENDERING 0x01
#define GUI_CAPTION_CENTER_X  0x02
#define GUI_CAPTION_CENTER_Y  0x04
#define GUI_CAPTION_AUTOWRAP  0x08

#define GUI_THEME_CAPTION                 "caption"
#define GUI_THEME_CAPTION_AUTOWRAP        "caption.autowrap"
#define GUI_THEME_CAPTION_CENTER_X        "caption.center.x"
#define GUI_THEME_CAPTION_CENTER_Y        "caption.center.y"
#define GUI_THEME_FOREGROUND              "foreground"

typedef struct guiCaption{
	guiImage_s* render;
	unsigned flags;
	g2dPoint_s scroll;
	utf8_t* text;
	unsigned textWidth;
	unsigned textHeight;
	ftFonts_s* fonts;
	g2dColor_t foreground;
}guiCaption_s;

/** create new label*/
guiCaption_s* gui_caption_new(ftFonts_s* font, g2dColor_t foreground, unsigned flags);

/** label free*/
void gui_caption_free(guiCaption_s* lbl);

/** caption set caption*/
void gui_caption_text_set(gui_s* gui, guiCaption_s* cap, const utf8_t* text);

/** label render text*/
void gui_caption_render(gui_s* gui, guiCaption_s* cap);

/** caption scroll*/
void gui_caption_scroll(gui_s* gui, guiCaption_s* cap, unsigned x, unsigned y);

/** set caption theme*/
int gui_caption_themes(gui_s* gui, guiCaption_s* cap, const char* name);

#endif
