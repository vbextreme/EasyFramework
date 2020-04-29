#ifndef __EF_GUI_BAR_H__
#define __EF_GUI_BAR_H__

#include <ef/gui.h>
#include <ef/guiLabel.h>

#define GUI_THEMES_BAR_COLOR "bar.color"
#define GUI_THEMES_BAR_MODE  "bar.mode"

#define GUI_BAR_HORIZONTAL   0x0001
#define GUI_BAR_VERTICAL     0x0002
#define GUI_BAR_CIRCLE       0x0004
#define GUI_BAR_SHOW_CURRENT 0x0008
#define GUI_BAR_SHOW_MIN     0x0010
#define GUI_BAR_SHOW_MAX     0x0020
#define GUI_BAR_SHOW_PERCENT 0x0040

typedef struct guiBar{
	guiCaption_s* caption;
	guiImage_s* fill;
	utf8_t* textdescript;
	double min;
	double max;
	double current;
	unsigned flags;	
}guiBar_s;

/** create new bar*/
guiBar_s* gui_bar_new(guiCaption_s* caption, guiImage_s* fill, double min, double max, double start, unsigned flags);

/** attach bar to gui*/
gui_s* gui_bar_attach(gui_s* gui, guiBar_s* bar);

/** free bar*/
void gui_bar_free(guiBar_s* bar);

/** set bar text*/
void gui_bar_text_set(gui_s* gui, const utf8_t* text);

/** set current value*/
void gui_bar_current_set(gui_s* gui, double current);

/** set max value*/
void gui_bar_max_set(gui_s* gui, double max);

/** bar redraw */
void gui_bar_redraw(gui_s* gui);

/** bar free event*/
int gui_bar_event_free(gui_s* gui, __unused xorgEvent_s* ev);

/** bar redraw event*/
int gui_bar_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

/** button event theme*/
//int gui_bar_event_themes(gui_s* gui, xorgEvent_s* ev);

#endif
