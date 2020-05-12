#ifndef __EF_GUI_BUTTON_H__
#define __EF_GUI_BUTTON_H__

#include <ef/gui.h>
#include <ef/guiCaption.h>

#define GUI_BUTTON_STATE_NORMAL 0
#define GUI_BUTTON_STATE_PRESS  1
#define GUI_BUTTON_STATE_HOVER  2
#define GUI_BUTTON_STATE_COUNT  3

#define GUI_THEME_BUTTON_PRESS "press"
#define GUI_THEME_BUTTON_HOVER "hover"
#define GUI_THEME_BUTTON_HOVER_ENABLE "hoverEnable"

#define GUI_BUTTON_FLAGS_HOVER 0x01


typedef struct guiButton{
	guiCaption_s* caption;
	guiComposite_s* state[GUI_BUTTON_STATE_COUNT];
	guiEvent_f onclick;
	guiEvent_f parentKey;
	int flags;
}guiButton_s;

/** create new button*/
guiButton_s* gui_button_new(guiCaption_s* caption, guiComposite_s* press, guiComposite_s* hover, guiEvent_f onclick, int flags);

/** attach button to gui*/
gui_s* gui_button_attach(gui_s* gui, guiButton_s* btn);

/** free button*/
void gui_button_free(guiButton_s* btn);

/** get button set text */
void gui_button_text_set(gui_s* gui, const utf8_t* text);

/** button redraw */
void gui_button_redraw(gui_s* gui, unsigned normalPressHover);

/** button free event*/
int gui_button_event_free(gui_s* gui, __unused xorgEvent_s* ev);

/** button redraw event*/
int gui_button_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

/** button event key*/
int gui_button_event_key(gui_s* gui, xorgEvent_s* event);

/** button event mouse*/
int gui_button_event_mouse(gui_s* gui, xorgEvent_s* event);

int gui_button_event_move(gui_s* gui, xorgEvent_s* event);

/** button event theme*/
int gui_button_event_themes(gui_s* gui, xorgEvent_s* ev);

#endif
