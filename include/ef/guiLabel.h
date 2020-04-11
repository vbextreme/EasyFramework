#ifndef __EF_GUI_LABEL_H__
#define __EF_GUI_LABEL_H__

#include <ef/gui.h>

typedef struct guiLabel{
	utf8_t* text;
	size_t weight;
	ftFonts_s* fonts;
	int autowrap;
	g2dColor_t foreground;
	g2dCoord_s position;
}guiLabel_s;


void label_free(guiLabel_s* lbl);
int gui_label_event_free(gui_s* gui, __unused xorgEvent_s* ev);
guiLabel_s* label_new(ftFonts_s* font, int autowrap, g2dColor_t foreground);
gui_s* gui_label_attach(gui_s* gui, guiLabel_s* lbl);
void gui_label_text_set(gui_s* gui, const utf8_t* text);
void gui_label_position_set(gui_s* gui, unsigned x, unsigned y);
int gui_label_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

#endif
