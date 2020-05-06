#ifndef __EF_GUI_BAR_H__
#define __EF_GUI_BAR_H__

#include <ef/gui.h>
#include <ef/guiCaption.h>

#define GUI_THEMES_BAR_COLOR        "bar.color"
#define GUI_THEMES_BAR_MODE         "bar.mode"
#define GUI_THEMES_BAR_DESCRIPT     "bar.descript"
#define GUI_THEMES_BAR_CUR_DESCRIPT "bar.descriptCurrent"

#define GUI_BAR_HORIZONTAL   0x0001
#define GUI_BAR_VERTICAL     0x0002
#define GUI_BAR_CIRCLE       0x0004
#define GUI_BAR_MIN_ANGLE    0x0008
#define GUI_BAR_SHOW_CURRENT 0x0010
#define GUI_BAR_SHOW_MIN     0x0020
#define GUI_BAR_SHOW_MAX     0x0040
#define GUI_BAR_SHOW_PERCENT 0x0080

typedef struct guiBar{
	guiCaption_s* caption;
	guiImage_s* fill;
	utf8_t* textdescript;
	utf8_t* currentdescript;
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

void gui_bar_flags_set(gui_s* gui, unsigned flags);

/** set bar text*/
void gui_bar_text_set(gui_s* gui, const utf8_t* text, const utf8_t* aftercurrent);

void gui_bar_circle_fn(gui_s* gui, __unused guiImage_s* img, void* generic);

/** set current value*/
void gui_bar_current_set(gui_s* gui, double current);
double gui_bar_current(gui_s* gui);
void gui_bar_max_set(gui_s* gui, double max);
double gui_bar_max(gui_s* gui);
void gui_bar_min_set(gui_s* gui, double min);
double gui_bar_min(gui_s* gui);

/** set max value*/
void gui_bar_max_set(gui_s* gui, double max);

void gui_bar_mode_horizontal(gui_s* gui, guiImage_s* fill);
void gui_bar_mode_vertical(gui_s* gui, guiImage_s* fill);
void gui_bar_mode_circle(gui_s* gui, g2dColor_t color);

/** bar redraw */
void gui_bar_redraw(gui_s* gui);

/** bar free event*/
int gui_bar_event_free(gui_s* gui, __unused xorgEvent_s* ev);

/** bar redraw event*/
int gui_bar_event_redraw(gui_s* gui, __unused xorgEvent_s* unset);

int gui_bar_event_move(gui_s* gui, xorgEvent_s* event);


/** button event theme*/
int gui_bar_event_themes(gui_s* gui, xorgEvent_s* ev);

#endif
