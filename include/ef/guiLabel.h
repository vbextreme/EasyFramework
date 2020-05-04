#ifndef __EF_GUI_LABEL_H__
#define __EF_GUI_LABEL_H__

#include <ef/gui.h>
#include <ef/guiCaption.h>

typedef struct guiLabel{
	guiCaption_s* caption;
}guiLabel_s;

/** create new label*/
guiLabel_s* gui_label_new(guiCaption_s* caption);

/** attach label to gui*/
gui_s* gui_label_attach(gui_s* gui, guiLabel_s* lbl);

/** label free*/
void gui_label_free(guiLabel_s* lbl);

/** label set caption*/
void gui_label_text_set(gui_s* gui, const utf8_t* text);

/** label redraw event*/
void gui_label_redraw(gui_s* gui);

/** label scroll*/
void gui_label_scroll(gui_s* gui, unsigned x, unsigned y);

/** event free*/
int gui_label_event_free(gui_s* gui, __unused xorgEvent_s* ev);

/** event redraw*/
int gui_label_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

/** event themes*/
int gui_label_event_themes(gui_s* gui, xorgEvent_s* ev);

int gui_label_event_move(gui_s* gui, xorgEvent_s* event);

#endif
