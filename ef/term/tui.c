#include <ef/tui.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/tuiRoot.h>
#include <ef/str.h>
#include <ef/err.h>

#define TUI_CHILD_INITIAL 2
#define TUI_CHILD_RESIZE  2

/*

/-----T---7
|     |   |
F-----+---3   
|     |   |
L-----1---j

(---------)
{---------}

.:

*/
__private utf8_t* tuiBorder[] = {
	U8("-|/7LjFT+13"),
	U8("─│┌┐└┘├┬┼┴┤"),
	U8("━┃┏┓┗┛┣┳╋┻┫"),
	U8("═║╔╗╚╝╠╦╬╩╣"),
    U8("▀▍▛▜▙▟▚▞"),
	U8("╭╮╰╯"),
	U8("┍┎┝ ┑┠╁┰╀┱┡┒┕┖┙┚┞┟┢┥┦┧┨┩┪┭┮┯┲┵┶┷┸┹┺┽┾┿╂╃╄╅╆╇╈╉╊╼╽╾╿"),
	U8("╒╓╕╖╘╙╛╜╞╟╡╢╤╥╧╨╪╫"),
	U8("╌┆┅┇┈┊┉┋╌╎╍╏")
};

__private utf_t tuiAtt[TUI_ATT_COUNT];

__private void tui_att_init(void){
	char tmp[256];
	for( size_t i = 0; i < 16; ++i ){
		/*scanbuild get error because term_escapemk macro not working on clang*/
		int c = i < 8 ? i : i - 8 + 60;
		term_escapemk(tmp, "color16_fg", c);
		tuiAtt[i] = term_utf_custom(0, tmp);
		term_escapemk(tmp, "color16_bk", c);
		tuiAtt[i+16] = term_utf_custom(0, tmp);
	}
	term_escapemk(tmp, "color_reset");
	tuiAtt[32] = term_utf_custom(0, tmp);
	term_escapemk(tmp, cap_enter_bold_mode);
	tuiAtt[33] = term_utf_custom(0, tmp);
	term_escapemk(tmp, cap_enter_italics_mode);
	tuiAtt[34] = term_utf_custom(0, tmp);
	term_escapemk(tmp, cap_enter_underline_mode);
	tuiAtt[35] = term_utf_custom(0, tmp);
	term_escapemk(tmp, cap_exit_attribute_mode);
	tuiAtt[36] = term_utf_custom(0, tmp);
}

void tui_begin(void){
	term_begin();
	term_load(NULL, term_name());
	term_load(NULL, term_name_extend());
	term_load(NULL, term_name_ef());
	term_update_key();
	term_endon_sigint();
	term_screen_size_enable();
	term_input_enable();
	tui_att_init();
	term_ca_mode(1);
	term_mouse(1);
	term_flush();
}

void tui_end(){
	term_mouse(0);
	term_color_reset();
	term_font_attribute(TERM_FONT_RESET);
	term_ca_mode(0);
	term_flush();
	term_input_disable();
	term_buff_end();
	term_end();
}

utf_t tui_att_get(tuiAttributes_s att){
	if( att < 0 || att >= TUI_ATT_COUNT ) return 0;
	return tuiAtt[att];	
}

utf_t tui_border_cast(int weight, char rappresentation){
	if( weight < 1 || weight > 3 ) return 0;
	char* pch = strchr((char*)tuiBorder[0], rappresentation);
	if( !pch ) return 0;
	size_t id = pch - (char*)tuiBorder[0];
	//dbg_info("cast \"%s\" [%lu:%c]", tuiBorder[id], id
	utf8Iterator_s it = utf8_iterator(tuiBorder[weight], 0);
	utf_t utf;
	while( id-->0 ) utf8_iterator_next(&it);
	utf = utf8_iterator_next(&it);
	return utf;
}

void tui_draw_hline(utf_t ch, unsigned count){
	while(count-->0){
		term_print(ch);
	}
}

void tui_draw_vline(tuiPosition_s st, utf_t ch, unsigned count){
	const unsigned en = st.r + count;
	for(size_t y = st.r; y < en; ++y){
		term_gotorc(y, st.c);
		term_print(ch);
	}
}

tui_s* tui_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height, void* userdata){
	tui_s* tui = mem_new(tui_s);
	if( !tui ) err_fail("malloc");
	tui->id = id;
	tui->name = name ? U8(str_dup((char*)name,0)):NULL;

	tui->position.r = r;
	tui->position.c = c;
	tui->size.width = width;
	tui->size.height = height;
	tui->usrdata = userdata;
	
	tui->parent = parent;
	tui->childs = vector_new(tui_s*, TUI_CHILD_INITIAL, TUI_CHILD_RESIZE);
	if(parent){
		tui_child_add(parent, tui);
	}
	tui->free = NULL;
	tui->draw = NULL;
	tui->eventKey = NULL;
	tui->eventFocus = NULL;
	tui->eventMouse = NULL;

	tui->attribute[0] = vector_new(utf_t, 2, 2);
	tui->attribute[1] = vector_new(utf_t, 2, 2);
	tui_attribute_add(tui, 0, tuiAtt[TUI_COLOR_RESET]);
	tui_attribute_add(tui, 1, tuiAtt[TUI_COLOR_RESET]);

	tui->clearchar = ' ';
	tui->border = border;
	//tui->visible = 0;
	tui->focused = 0;
	tui->focusBorder = 0;

	return tui;
}

void tui_free(tui_s* tui){
	if( tui->parent ){
		tui_child_remove(tui->parent, tui);
	}
	vector_foreach(tui->childs, i){
		tui_free(tui->childs[i]);
	}
	vector_free(tui->childs);

	if( tui->free ) tui->free(tui->usrdata);
	if( tui->name ) free(tui->name);
	vector_free(tui->attribute[0]);
	vector_free(tui->attribute[1]);
	free(tui);
}

void tui_name_set(tui_s* tui, utf8_t* name){
	if( tui->name ) free(tui->name);
	tui->name = U8(str_dup((char*)name, 0));
	if( !tui->name ) err_fail("create name");
}

void tui_attribute_add(tui_s* tui, int focus, utf_t att){
	iassert( focus >= 0 && focus <= 1);
	vector_push_back(tui->attribute[focus], att);
}

void tui_attribute_clear(tui_s* tui){
	vector_clear(tui->attribute[0]);
	vector_clear(tui->attribute[1]);
}

void tui_attribute_print(tui_s* tui){
	vector_foreach(tui->attribute[tui->focused], i){
		term_print(tui->attribute[tui->focused][i]);
	}
}

void tui_child_add(tui_s* parent, tui_s* child){
	vector_push_back(parent->childs, child);
}

tui_s* tui_child_remove(tui_s* parent, tui_s* child){
	vector_foreach(parent->childs, i){
		if(parent->childs[i] == child){
			tui_s* ret = parent->childs[i];
			vector_remove(parent->childs, i);
			return ret;
		}
	}
	return NULL;
}

tui_s* tui_child_find(tui_s* parent, utf8_t* name, int id){
	vector_foreach(parent->childs, i){
		if( name ){
			if( !strcmp((char*)name, (char*)parent->childs[i]->name) ){
				return parent->childs[i];
			}
		}
		else if( id == parent->childs[i]->id ){
			return parent->childs[i];
		}
	}
	return NULL;
}

ssize_t tui_child_index(tui_s* parent, tui_s* child){
	dbg_info("search %d from %d", child->id, parent->id);
	vector_foreach(parent->childs, i){
		if(parent->childs[i] == child){
			dbg_info("index:%lu", i);
			return i;
		}
	}
	dbg_error("not find child");
	return -1;
}

tuiPosition_s tui_area_position(tui_s* tui){
	tuiPosition_s ret = tui->position;
	if( tui->border ){
		++ret.c;
		++ret.r;
	}
	return ret;
}

tuiSize_s tui_area_size(tui_s* tui){
	tuiSize_s ret = tui->size;
	if( tui->border ){
		ret.width -= 2;
		ret.height -= 2;
	}
	return ret;
}

void tui_area_goto(tui_s* tui){
	tuiPosition_s pos = tui_area_position(tui);
	term_gotorc(pos.r, pos.c);
}

void tui_clear_area(tui_s* tui){
	const tuiPosition_s pos = tui_area_position(tui);
	const tuiSize_s size = tui_area_size(tui);
	tui_attribute_print(tui);

	for(int y = pos.r; y < size.height + pos.r; ++y){
		term_gotorc(y , pos.c);
		for( int x = pos.c; x < pos.c + size.width; ++x){
			term_print(tui->clearchar);
		}
	}

	term_gotorc(pos.r, pos.c);
}

void tui_clear(tui_s* tui){
	const unsigned xs = tui->position.c;
	const unsigned ys = tui->position.r;	
	const unsigned h = tui->size.height + ys;
	const unsigned w = tui->size.width + xs;
	
	dbg_info("tui(%d).%s: clear r:%u c:%u h:%u w:%u", tui->id, tui->name,ys,xs,h,w);
	tui_attribute_print(tui);
	for(unsigned y = ys; y < h; ++y){
		term_gotorc(y,xs);
		for( unsigned x = xs; x < w; ++x){
			term_print(tui->clearchar);
		}
	}
	term_gotorc(ys, xs);
}

void tui_clear_all(tui_s* tui){
	vector_foreach(tui->childs, i){
		tui_clear_all(tui->childs[i]);
	}
	tui_clear(tui);
}

void tui_draw_border(tui_s* tui){
	dbg_info("tui[%d]:%s.border",tui->id, tui->name);
	if( !tui->border ) return;
	tui_attribute_print(tui);

	utf_t h = tui_border_cast(tui->border, '-');
	utf_t v = tui_border_cast(tui->border, '|');
	
	term_gotorc(tui->position.r, tui->position.c);
	term_print(tui_border_cast(tui->border, '/'));
	if( tui->name && utf_width(tui->name)+5 < tui->size.width ){
		size_t n = utf_width(tui->name) + 4;
		term_print(tui_border_cast(tui->border, '3'));
		term_print(tui->name);
		term_print(tui_border_cast(tui->border, 'F'));
		tui_attribute_print(tui);
		tui_draw_hline(h, tui->size.width - n);
	}
	else{
		tui_draw_hline(h, tui->size.width - 2);
	}
	term_print(tui_border_cast(tui->border, '7'));

	term_gotorc(tui->position.r + tui->size.height-1, tui->position.c);
	term_print(tui_border_cast(tui->border, 'L'));
	tui_draw_hline(h, tui->size.width - 2);
	term_print(tui_border_cast(tui->border, 'j'));

	tui_draw_vline((tuiPosition_s){.r = tui->position.r + 1, .c = tui->position.c}, v, tui->size.height - 2);
	tui_draw_vline((tuiPosition_s){.r = tui->position.r + 1, .c = tui->position.c + tui->size.width - 1}, v, tui->size.height - 2);
}

void tui_draw(tui_s* tui){
	tui_clear(tui);
	dbg_info("tui[%d]:%s.draw",tui->id, tui->name);
	if( tui->border ) tui_draw_border(tui) ;
	if( tui->draw ) tui->draw(tui);
	vector_foreach(tui->childs,i){
		tui_draw(tui->childs[i]);
	}
}

__private void rmove(tui_s* tui, int r, int c){
	vector_foreach(tui->childs, i){
		rmove(tui->childs[i], r, c);
	}
	tui->position.c += c;
	tui->position.r += r;
}

void tui_move(tui_s* tui, int r, int c){
	tui_clear(tui);
	rmove(tui, r - tui->position.r, c - tui->position.c);
	tui_draw(tui);
}

int tui_default_event_key(tui_s* tui, termKey_s key){
	dbg_info("key(%d)", tui->id);

	switch( key.ch ){
		case TERM_INPUT_CHAR_ESC:
		return TUI_EVENT_RETURN_EXIT;
	}

	switch( key.escape ){
		case TERM_KEY_UP:
		return TUI_EVENT_RETURN_FOCUS_PARENT;
		
		case TERM_KEY_RIGHT:
		return TUI_EVENT_RETURN_FOCUS_NEXT;

		case TERM_KEY_LEFT:
		return TUI_EVENT_RETURN_FOCUS_PREV;

		case TERM_KEY_DOWN:
		return TUI_EVENT_RETURN_FOCUS_CHILD;

		case TERM_KEY_MOUSE:{
			termMouse_s mouse = term_mouse_event();
			tui_s* sel = tui_root_getin(tui_root_get(tui), mouse.r, mouse.c);
			if( !sel ) return TUI_EVENT_RETURN_EXIT;
			if( sel->eventMouse ) {
				return sel->eventMouse(sel, mouse);
			}
			else{
				tui_root_focus_set(tui_root_get(tui), sel);
			}
		}
		return 0;
	}

	return 0;
}

int tui_default_event_focus(tui_s* tui, int enable){
	dbg_info("focus(%d) %d", enable, tui->id);

	if( enable ){
		tui->focused = 1;
		if( tui->border && tui->focusBorder ){
			tui->border = 2;
		}
		dbg_info("set focus on r:%d c:%d", tui->position.r, tui->position.c);
		tui_draw(tui);
		term_gotorc(tui->position.r, tui->position.c);
		term_flush();
	}
	else{
		tui->focused = 0;
		if( tui->border && tui->focusBorder ) tui->border = 1;
		tui_draw(tui);
		term_flush();
	}
	return 0;
}

