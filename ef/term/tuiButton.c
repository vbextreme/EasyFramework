#include <ef/tuiButton.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

__private void button_free(void* ud){
	tuiButton_s* btn = ud;
	if( btn->str ) free(btn->str);
	free(ud);
}

void tui_button_event_draw(tui_s* tui){
	dbg_info("draw(%d)", tui->id);

	tuiButton_s* btn = tui->usrdata;
	
	if( !btn->str ) return;
	const tuiPosition_s pos = tui_area_position(tui);
	const tuiSize_s size = tui_area_size(tui);
	
	term_gotorc(pos.r,pos.c);
	tui_attribute_print(tui);
	
	utf8Iterator_s it = utf8_iterator(btn->str, 0);
	utf_t u;
	size_t weight = utf_width(btn->str);
	int c = (int)weight < size.width ? (size.width - weight) / 2 : 0;
	int r = 0;

	while( (u=utf8_iterator_next(&it)) ){
		if( u != '\n' ) term_print(u);
		++c;
		if( c >= size.width || u == '\n' ){
			c = 0;
			++r;
			if( r >= size.height ) break;
			term_gotorc(r+pos.r,c+pos.c);
		}
	}
}

int tui_button_event_key(tui_s* tui, termKey_s key){
	tuiButton_s* btn = tui->usrdata;
	if( key.ch == '\n' ){
		if( btn->onpress ) return btn->onpress(tui, 1);
		return 0;
	}
	return tui_default_event_key(tui, key);
}

int tui_button_event_mouse(tui_s* tui, __unused termMouse_s mouse){
	iassert(tui->type == TUI_TYPE_BUTTON);
	tuiButton_s* btn = tui->usrdata;
	if( btn->onpress ) return btn->onpress(tui, 1);
	return 0;
}

tui_s* tui_button_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height){
	tuiButton_s* btn = mem_new(tuiButton_s);
	if( !btn ) err_fail("malloc");
	
	tui_s* tr = tui_new(parent, id, name, border, r, c, width, height, btn);
	
	tr->free = button_free;
	tr->draw = tui_button_event_draw;
	tr->eventKey = tui_button_event_key;
	tr->eventFocus = tui_default_event_focus;
	tr->eventMouse = tui_button_event_mouse;
	tr->type = TUI_TYPE_BUTTON;
	
	btn->str = NULL;
	btn->onpress = NULL;
	btn->usrdata = NULL;
	
	return tr;
}

void tui_button_set(tui_s* tl, const utf8_t* str){
	iassert(tl->type == TUI_TYPE_BUTTON);
	tuiButton_s* btn = tl->usrdata;
	if( btn->str ) free(btn->str);
	btn->str = U8(str_dup((char*)str,0));
}

void tui_button_onpress_set(tui_s* tl, tuiEventInt_f fn, void* userdata){
	tuiButton_s* btn = tl->usrdata;
	btn->onpress = fn;
	btn->usrdata = userdata;
}

void* tui_button_userdata(tui_s* tui){
	tuiButton_s* btn = tui->usrdata;
	return btn->usrdata;
}

