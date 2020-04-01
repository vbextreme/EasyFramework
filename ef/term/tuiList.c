#include <ef/tuiList.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>


__private void list_free(void* ud){
	tuiList_s* lst = ud;
	vector_foreach(lst->elements, i){
		free(lst->elements[i].name);
	}
	vector_free(lst->elements);
	free(ud);
}

__private void print_type(tuiList_s* lst, size_t i){
	switch(lst->type){
		case TUI_LIST_CHECK:
			term_print(lst->elements[i].val ? TUI_LIST_CHECKED_TRUE : TUI_LIST_CHECKED_FALSE);
		break;

		case TUI_LIST_OPTION:
			term_print(lst->elements[i].val ? TUI_LIST_OPTION_TRUE : TUI_LIST_OPTION_FALSE);
		break;

		case TUI_LIST_NORMAL: 
		break;
	}	
}

void tui_list_event_draw(tui_s* tui){
	tuiList_s* lst = tui->usrdata;

	const tuiPosition_s pos = tui_area_position(tui);
	const tuiSize_s size = tui_area_size(tui);
	
	int c = pos.c;
	int r = pos.r;
	int we = 0;
	const int wt = lst->type == TUI_LIST_NORMAL ? 0 : 1;

	vector_foreach(lst->elements, i){
		if( (lst->mode == TUI_LIST_VERTICAL ) && i < lst->scrollRow) continue;
		if( (lst->mode == TUI_LIST_HORIZONTAL || lst->mode == TUI_LIST_GRID) && c >= size.width + pos.c ) continue;
		if( r >= pos.r + size.height ) break;

		term_gotorc(pos.r,pos.c);
		tui_attribute_print(tui);
		lst->elements[i].r = r;
		lst->elements[i].c = c;
		
		switch( lst->mode ){
			case TUI_LIST_HORIZONTAL:
				we += utf_width(lst->elements[i].name) + wt;
				if( we + c < size.width + pos.c ){
					term_gotorc(pos.r,pos.c);
					tui_attribute_print(tui);
					print_type(lst, i);
					term_print(lst->elements[i].name);
					c += we + 1;
				}
				else{
					c = INT_MAX;
				}
			break;
			
			case TUI_LIST_GRID:
				we += utf_width(lst->elements[i].name) + wt;
				if( we + c < size.width + pos.c ){
					term_gotorc(pos.r,pos.c);
					tui_attribute_print(tui);
					print_type(lst, i);
					term_print(lst->elements[i].name);
					c += we + 1;
				}
				else{
					++r;
					if( r < pos.r + size.height ){
						c = pos.c;
						term_gotorc(pos.r,pos.c);
						tui_attribute_print(tui);
						print_type(lst, i);
						term_print(lst->elements[i].name);
						c += we + 1;
					}
					else{
						c = INT_MAX;
					}
				}
			break;

			case TUI_LIST_VERTICAL:
				term_gotorc(pos.r,pos.c);
				tui_attribute_print(tui);
				print_type(lst, i);
				term_print(lst->elements[i].name);
				term_print(lst->elements[i].name);
				++r;
			break;
		}
	}
}

__private void list_set_cursor(tui_s* tui){
	tuiList_s* lst = tui->usrdata;
	term_gotorc(lst->elements[lst->sel+lst->scrollRow].r, lst->elements[lst->sel+lst->scrollRow].c);
}

int tui_list_event_key(tui_s* tui, termKey_s key){
	tuiList_s* lst = tui->usrdata;

	switch( key.ch ){
		case TERM_INPUT_CHAR_ESC:
		return TUI_EVENT_RETURN_FOCUS_PARENT;
		
		case '\n':
			if( lst->type == TUI_LIST_CHECK ){
				lst->elements[lst->sel+lst->scrollRow].val = !lst->elements[lst->sel+lst->scrollRow].val;
			}
			else if( lst->type == TUI_LIST_OPTION ){
				vector_foreach(lst->elements, i) lst->elements[i].val = 0;
				lst->elements[lst->sel+lst->scrollRow].val = 1;
			}
			tui_draw(tui);
			list_set_cursor(tui);
			term_flush();
		return 0;
	}

	switch( key.escape ){
		case TERM_KEY_UP:
			if( lst->mode == TUI_LIST_HORIZONTAL ) return TUI_EVENT_RETURN_FOCUS_PARENT;
			if( lst->sel == 0 ){
				if( lst->scrollRow ){
					--lst->scrollRow;
				}
				else{
					lst->sel = vector_count(lst->elements) - 1;
					lst->scrollRow = lst->sel - tui_area_size(tui).height;
					lst->sel -= lst->scrollRow;
				}
			}
			else{
				--lst->sel;
			}
			list_set_cursor(tui);
		return 0;
		
		case TERM_KEY_RIGHT:
		return TUI_EVENT_RETURN_FOCUS_NEXT;

		case TERM_KEY_LEFT:
		return TUI_EVENT_RETURN_FOCUS_PREV;

		case TERM_KEY_DOWN:
			if( lst->mode == TUI_LIST_HORIZONTAL ) return TUI_EVENT_RETURN_FOCUS_NEXT;
			if( (int)lst->sel >= tui_area_size(tui).height ){
			   	if( lst->scrollRow + 1 + lst->sel < vector_count(lst->elements) - 1 ){
					++lst->scrollRow;
				}
				else{
					lst->sel = 0;
					lst->scrollRow = 0;
				}
			}
			else{
				++lst->sel;
			}
			list_set_cursor(tui);
		return 0;
	}

	return 0;
}

int tui_list_event_focus(tui_s* tui, int enable){
	//tuiList_s* lst = tui->usrdata;
	tui_default_event_focus(tui,enable);
	if( enable ) list_set_cursor(tui);
	return 0;
}

tui_s* tui_list_new(tui_s* parent, int id, utf8_t* name, int border, int r, int c, int width, int height){
	tuiList_s* lst = mem_new(tuiList_s);
	if( !lst ) err_fail("malloc");
	
	tui_s* tr = tui_new(parent, id, name, border, r, c, width, height, lst);
	
	tr->free = list_free;
	tr->draw = tui_list_event_draw;
	tr->eventKey = tui_list_event_key;
	tr->eventFocus = tui_default_event_focus;
	tr->type = TUI_TYPE_LIST;
	
	lst->mode = TUI_LIST_VERTICAL;
	lst->type = TUI_LIST_NORMAL;
	lst->elements = vector_new(tuiListElement_s, 8, 32);
	lst->sel = 0;

	return tr;
}

void tui_list_add(tui_s* tui, const utf8_t* name, int val, void* userdata){
	tuiList_s* lst = tui->usrdata;
	tuiListElement_s* el = vector_get_push_back(lst->elements);
	el->name = U8(str_dup((const char*)name, 0));
	el->val = val;
	el->usrdata = userdata;
}

void tui_list_clear(tui_s* tui){
	tuiList_s* lst = tui->usrdata;
	vector_foreach(lst->elements, i){
		free(lst->elements[i].name);
	}
}

void tui_list_option(tui_s* tui, tuiListMode_e mode, tuiListType_e type){
	tuiList_s* lst = tui->usrdata;
	lst->mode = mode;
	lst->type = type;
}
