#ifndef __EF_TUI_WINDOW_H__
#define __EF_TUI_WINDOW_H__

#include <ef/tui.h>

typedef struct tuiWindow{
	int unused;
}tuiWindow_s;

tui_s* tui_window_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height);

int tui_window_msgbox(tui_s* root, int id, utf8_t* name, int border, int r, int c, int w, int h, utf8_t* text, utf8_t** buttonName, unsigned buttonCount);



#endif
