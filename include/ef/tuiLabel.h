#ifndef __EF_TUI_LABEL_H__
#define __EF_TUI_LABEL_H__

#include <ef/tui.h>

typedef struct tuiLabel{
	utf8_t* str;
}tuiLabel_s;

/** event label draw*/
void tui_label_event_draw(tui_s* tui);

/** create new label*/
tui_s* tui_label_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height);

/** set label caption*/
void tui_label_set(tui_s* tl, const utf8_t* str);


#endif
