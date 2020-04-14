#ifndef __EF_GUI_LABEL_H__
#define __EF_GUI_LABEL_H__

#include <ef/gui.h>

#define GUI_LABEL_RENDERING 0x01
#define GUI_LABEL_CENTER_X  0x02
#define GUI_LABEL_CENTER_Y  0x04

typedef struct guiLabel{
	g2dImage_s* render;
	unsigned flags;
	g2dPoint_s scroll;
	utf8_t* text;
	ftFonts_s* fonts;
	int autowrap;
	g2dColor_t foreground;
}guiLabel_s;

guiLabel_s* gui_label_new(ftFonts_s* font, int autowrap, g2dColor_t foreground, unsigned flags);
gui_s* gui_label_attach(gui_s* gui, guiLabel_s* lbl);
void gui_label_free(guiLabel_s* lbl);
void gui_label_text_set(__unused gui_s* gui, guiLabel_s* lbl, const utf8_t* text);
void gui_label_redraw(gui_s* gui, guiBackground_s* bkg, guiLabel_s* lbl);
void gui_label_scroll(gui_s* gui, guiLabel_s* lbl, unsigned x, unsigned y);

int gui_label_event_free(gui_s* gui, __unused xorgEvent_s* ev);
int gui_label_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

#endif
