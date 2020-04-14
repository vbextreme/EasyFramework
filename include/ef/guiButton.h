#ifndef __EF_GUI_BUTTON_H__
#define __EF_GUI_BUTTON_H__

#include <ef/gui.h>
#include <ef/guiLabel.h>

#define GUI_BUTTON_BACKGROUND_PRESS 1
#define GUI_BUTTON_BACKGROUND_HOVER 2

typedef struct guiButton{
	guiLabel_s* label;
	guiEvent_f onclick;
	guiEvent_f parentKey;
}guiButton_s;

guiButton_s* gui_button_new(guiLabel_s* lbl, guiEvent_f onclick);
gui_s* gui_button_attach(gui_s* gui, guiButton_s* btn, guiBackground_s* press, guiBackground_s* hover);
void gui_button_free(guiButton_s* btn);
guiLabel_s* gui_button_label(guiButton_s* button);
void gui_button_redraw(gui_s* gui, guiButton_s* btn, int press);

int gui_button_event_free(gui_s* gui, __unused xorgEvent_s* ev);
int gui_button_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);
int gui_button_event_key(gui_s* gui, xorgEvent_s* event);
int gui_button_event_mouse(gui_s* gui, xorgEvent_s* event);


#endif
