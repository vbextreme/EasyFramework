#include <ef/tuiLabel.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

__private void label_free(void* ud){
	tuiLabel_s* lbl = ud;
	if( lbl->str ) free(lbl->str);
	free(ud);
}

void tui_label_event_draw(tui_s* tui){
	dbg_info("draw(%d)", tui->id);
	tuiLabel_s* lbl = tui->usrdata;
	if( !lbl->str ) return;
	const tuiPosition_s pos = tui_area_position(tui);
	const tuiSize_s size = tui_area_size(tui);
	
	term_gotorc(pos.r,pos.c);
	tui_attribute_print(tui);

	utf8Iterator_s it = utf8_iterator(lbl->str, 0);
	utf_t u;
	int c = 0;
	int r = 0;
	
	while( (u=utf8_iterator_next(&it)) ){
		if( u != '\n' ) term_print_u8(u);
		++c;
		if( c >= size.width || u == '\n' ){
			c = 0;
			++r;
			if( r >= size.height ) break;
			term_gotorc(r+pos.r,c+pos.c);
		}
	}
}

tui_s* tui_label_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height){
	tuiLabel_s* lbl = mem_new(tuiLabel_s);
	if( !lbl ) err_fail("malloc");
	
	tui_s* tr = tui_new(parent, id, name, border, r, c, width, height, lbl);
	
	tr->free = label_free;
	tr->draw = tui_label_event_draw;
	tr->eventFocus = tui_default_event_focus;
	tr->eventKey = tui_default_event_key;
	tr->type = TUI_TYPE_LABEL;
	lbl->str = NULL;

	return tr;
}

void tui_label_set(tui_s* tl, const utf8_t* str){
	iassert(tl->type == TUI_TYPE_LABEL);
	tuiLabel_s* lbl = tl->usrdata;
	if( lbl->str ) free(lbl->str);
	lbl->str = U8(str_dup((char*)str,0));
}


