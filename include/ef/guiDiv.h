#ifndef __EF_GUI_DIV_H__
#define __EF_GUI_DIV_H__

#include <ef/gui.h>

typedef enum {
	GUI_DIV_NONE,
	GUI_DIV_VERTICAL,
	GUI_DIV_HORIZONTAL,
	GUI_DIV_GRID
}guiDivMode_e;

typedef struct guiDiv{
	guiDivMode_e mode;
	g2dPoint_s sep;
	g2dPoint_s scroll;
	g2dPoint_s lastElement;
}guiDiv_s;

#define GUI_DIV_DEFAULT_X 5
#define GUI_DIV_DEFAULT_Y 5

/** create new div*/
guiDiv_s* gui_div_new(guiDivMode_e mode);

/** attach div to gui*/
gui_s* gui_div_attach(gui_s* gui, guiDiv_s* div);

/** free div*/
void gui_div_free(guiDiv_s* div);

/** realign div*/
void gui_div_align(gui_s* gui, guiDiv_s* div);

/** event free*/
int gui_div_event_free(gui_s* gui, __unused xorgEvent_s* ev);

/** event key*/
int gui_div_event_key(gui_s* gui, xorgEvent_s* event);


#endif
