#include <ef/tuiWindow.h>
#include <ef/tuiRoot.h>
#include <ef/tuiLabel.h>
#include <ef/tuiButton.h>

#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/err.h>

tui_s* tui_window_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height){
	tuiWindow_s* win = mem_new(tuiWindow_s);
	if( !win ) err_fail("malloc");

	tui_s* tr = tui_new(parent, id, name, border, r, c, width, height, win);
	
	tr->free = free;
	tr->eventFocus = tui_default_event_focus;
	tr->eventKey = tui_default_event_key;
	tr->type = TUI_TYPE_WINDOW;

	return tr;
}

__private int msgbox_key(tui_s* tui, termKey_s key){
	if(key.escape == TERM_KEY_LEFT) key.escape = TERM_KEY_RIGHT;
	else if( key.escape == TERM_KEY_RIGHT) key.escape = TERM_KEY_LEFT;
	return tui_default_event_key(tui,key);
}

__private int msgbox_press(tui_s* tui, __unused int press){
	int* id = tui_button_userdata(tui);
	*id = tui->id;
	return TUI_EVENT_RETURN_EXIT;
}

int tui_window_msgbox(tui_s* root, int id, utf8_t* name, int border, int r, int c, int w, int h, utf8_t* text, utf8_t** buttonName, unsigned buttonCount){
	tui_s* msgbox = tui_window_new(root, id, name, border, r, c, w, h);	
	if( border ) msgbox->focusBorder = 1;
	tuiPosition_s pos = tui_area_position(msgbox);
	tuiSize_s siz = tui_area_size(msgbox);
	
	tui_s* lbl = tui_label_new(msgbox, id+1000, NULL, 0, pos.r, pos.c, siz.width, siz.height);	
	lbl->eventKey = msgbox_key;
	tui_label_set(lbl, text);
	
	int ret = -1;
	unsigned const br = pos.r + siz.height - 1;
	unsigned bc = siz.width + pos.c;
	tui_s* btn;
	for( size_t i = 0; i < buttonCount; ++i){
		unsigned bw = utf_width(buttonName[i]);
		bc -= (bw + 1);
		btn = tui_button_new(msgbox, i, NULL, 0, br, bc, bw, 1);
		btn->eventKey = msgbox_key;
		tui_button_set(btn, buttonName[i]);
		tui_button_onpress_set(btn, msgbox_press, &ret);
	}	

	tui_root_wait(msgbox, btn);
	
	return ret;
}
