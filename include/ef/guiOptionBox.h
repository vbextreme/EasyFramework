#ifndef __EF_GUI_OPTIONBOX_H__
#define __EF_GUI_OPTIONBOX_H__

#include <ef/gui.h>
#include <ef/guiCaption.h>

#define GUI_OPTION_STATE_OFF       0
#define GUI_OPTION_STATE_ON        1
#define GUI_OPTION_STATE_HOVER_OFF 2
#define GUI_OPTION_STATE_HOVER_ON  3
#define GUI_OPTION_STATE_COUNT     4

#define GUI_OPTION_FLAGS_UNIQUE        0x01
#define GUI_OPTION_FLAGS_ACTIVE        0x02
#define GUI_OPTION_FLAGS_HOVER         0x04
#define GUI_OPTION_FLAGS_HOVER_ENABLE  0x08

#define GUI_THEME_OPTION_ON        "option.on"
#define GUI_THEME_OPTION_HOVER_ON  "option.hover.on"
#define GUI_THEME_OPTION_HOVER_OFF "option.hover.off"
#define GUI_THEME_OPTION_HOVER     "option.hover"

typedef struct guiOption{
	guiCaption_s* caption;
	guiComposite_s* state[GUI_OPTION_STATE_COUNT];
	guiEvent_f parentKey;
	int flags;
}guiOption_s;

/** create new option*/
guiOption_s* gui_option_new(guiCaption_s* caption, guiComposite_s* on, guiComposite_s* hoveroff, guiComposite_s* hoveron, int flags);

/** attach option to gui*/
gui_s* gui_option_attach(gui_s* gui, guiOption_s* opt);

/** free option*/
void gui_option_free(guiOption_s* opt);

/** option set text */
void gui_option_text_set(gui_s* gui, const utf8_t* text);

/** set on/off */
void gui_option_active(gui_s* gui, int value);

/** return 0 off or positive value for on */
int gui_option_activated(gui_s* gui);

/** invert */
void gui_option_toggle(gui_s* gui);

/** option redraw */
void gui_option_redraw(gui_s* gui);

/** option free event*/
int gui_option_event_free(gui_s* gui, __unused xorgEvent_s* ev);

/** option redraw event*/
int gui_option_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

/** option event key*/
int gui_option_event_key(gui_s* gui, xorgEvent_s* event);

/** option event mouse*/
int gui_option_event_mouse(gui_s* gui, xorgEvent_s* event);

/** option event move*/
int gui_option_event_move(gui_s* gui, xorgEvent_s* event);

/** option event theme*/
int gui_option_event_themes(gui_s* gui, xorgEvent_s* ev);

#endif
