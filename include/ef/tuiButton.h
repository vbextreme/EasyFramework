#ifndef __EF_TUI_BUTTON_H__
#define __EF_TUI_BUTTON_H__

#include <ef/tui.h>

typedef struct tuiButton{
	utf8_t* str;
	tuiEventInt_f onpress;
	void* usrdata;
}tuiButton_s;

void tui_button_event_draw(tui_s* tui);

int tui_button_event_focus(tui_s* tui, int focus);

int tui_button_event_key(tui_s* tui, termKey_s key);

int tui_button_event_mouse(tui_s* tui, __unused termMouse_s mouse);

tui_s* tui_button_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height);

void tui_button_set(tui_s* tl, const utf8_t* str);

void tui_button_onpress_set(tui_s* tl, tuiEventInt_f fn, void* userdata);

void* tui_button_userdata(tui_s* tui);

#endif
