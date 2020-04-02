#ifndef __EF_TUI_ROOT_H__
#define __EF_TUI_ROOT_H__

#include <ef/tui.h>

typedef struct tuiRoot{
	tui_s* focus;
}tuiRoot_s;

/** create new root tui, is always need for all tui application*/
tui_s* tui_root_new(void);

/** get root from any tui*/
tui_s* tui_root_get(tui_s* any);

/** get tui focused*/
tui_s* tui_root_focused(tui_s* tr);

/** set focus to tui*/
void tui_root_focus_set(tui_s* tr, tui_s* focus);

/** get tui in position r,c*/
tui_s* tui_root_getin(tui_s* tr, int r, int c);

/** loop key*/
void tui_root_loop(tui_s* tr);

/** create a loop key and wait it*/
void tui_root_wait(tui_s* tui, tui_s* setFocus);

#endif
