#ifndef __EF_GUI_DIV_H__
#define __EF_GUI_DIV_H__

#include <ef/gui.h>

typedef enum {
	GUI_DIV_GRID,
	GUI_DIV_HORI,
	GUI_DIV_VERT
}guiDivMode_e;

typedef struct guiDiv{
	guiDivMode_e mode;
	g2dPoint_s sep;
}guiDiv_s;

#define GUI_DIV_DEFAULT_X 5
#define GUI_DIV_DEFAULT_Y 5

guiDiv_s* gui_div_new(guiDivMode_e mode);
gui_s* gui_div_attach(gui_s* gui, guiDiv_s* div);
void gui_div_free(guiDiv_s* div);

int gui_div_event_free(gui_s* gui, __unused xorgEvent_s* ev);



#endif
