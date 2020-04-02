#ifndef __EF_TUI_ROOT_H__
#define __EF_TUI_ROOT_H__

#include <ef/tui.h>

typedef struct tuiRoot{
	tui_s* focus;
}tuiRoot_s;

tui_s* tui_root_new(void);

tui_s* tui_root_get(tui_s* any);

tui_s* tui_root_focused(tui_s* tr);

void tui_root_focus_set(tui_s* tr, tui_s* focus);

tui_s* tui_root_getin(tui_s* tr, int r, int c);

void tui_root_loop(tui_s* tr);

void tui_root_wait(tui_s* tui, tui_s* setFocus);

#endif
