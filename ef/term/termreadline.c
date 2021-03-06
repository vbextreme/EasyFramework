#include <ef/termreadline.h>
#include <ef/terminfo.h>
#include <ef/termlink.h>
#include <ef/termmode.h>
#include <ef/termkey.h>
#include <ef/str.h>
#include <ef/memory.h>
#include <ef/err.h>

#define TERM_READLINE_PRIVATE_UTF UTF_PRIVATE0_START
#define TERM_READLINE_ATTRIBUTE_SIZE 80

__private void term_readline_line_clear(termReadLine_s* rl, int r, int c){
	unsigned nc = (c - rl->position.col) + 1;
	nc = rl->position.width - nc;
	while( nc-->0 ) putchar(' ');
	term_gotorc(r,c);
}
	
termReadLine_s* term_readline_new(utf8_t* prompt, int r, int c, int w, int h){
	termReadLine_s* rl = mem_new(termReadLine_s);
	if( !rl ){
		err_pushno("malloc");
		return NULL;
	}
	
	if( r < 0 || c < 0 ){
		int y = 0;
		int x = 0;
		term_cursor_position(&y, &x);
		if( r < 0 ) r = y;
		if( c < 0 ) c = x;
	}
	rl->position.col = c;
	rl->position.row = r;

	if( w < 0 || h < 0 ){
		unsigned sw = 80;
		unsigned sh = 40;
		term_screen_size_get(&sh, &sw);
		if( w < 0 ) w = sw;
		if( h < 0 ) h = sh;
	}
	rl->position.width = w;
	rl->position.height = h;
	rl->position.colLast = 0;
	rl->position.rowLast = 0;

	rl->prompt.str = prompt;
	rl->prompt.len = prompt ? utf_width(prompt) : 0;
	rl->prompt.size = 0;

	rl->text.size = rl->position.width * rl->position.height + 1;
	rl->text.str = mem_many(utf8_t, rl->text.size);
	if( !rl->text.str ){
		free(rl);
		return NULL;
	}
	rl->text.str[0] = 0;
	rl->text.len = 0;
	rl->it = utf8_iterator(rl->text.str, 0);
	
	rl->cursor.col = rl->position.col + rl->prompt.len;
	rl->cursor.row = rl->position.row;
	rl->cursor.mode = TERM_READLINE_MODE_INSERT;
	rl->cursor.scrollcol = 0;
	rl->cursor.scrollrow = 0;

	/*rl->attribute.size = TERM_READLINE_ATTRIBUTE_SIZE;
	rl->attribute.value = mem_many(char*, rl->attribute.size);
	if( !rl->attribute.value ){
		free(rl->text.str);
		free(rl);
		return NULL;
	}
	rl->attribute.len = 0;
	*/
	return rl;
}

void term_readline_free(termReadLine_s* rl){
	/*for( size_t i = 0; i < rl->attribute.len; ++i){
		free(rl->attribute.value[i]);
	}
	free(rl->attribute.value);
	*/
	free(rl->text.str);
	free(rl);
}

size_t term_readline_line_left_width(termReadLine_s* rl){
	utf8Iterator_s it = rl->it;
	size_t width = 0;
	utf_t utf;
	while( (utf=utf8_iterator_prev(&it)) && utf != '\n' ) ++width;
	return (int)rl->cursor.row == rl->position.row ? width + rl->prompt.len : width;
}

size_t term_readline_line_right_width(termReadLine_s* rl){
	utf8Iterator_s it = rl->it;
	size_t width = 0;
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) && utf != '\n' ) ++width;
	return width;
}

/*
__private void term_readline_attribute_print(termReadLine_s* rl, utf_t att){
	size_t id = att - TERM_READLINE_PRIVATE_UTF - 1;
	if( id >= rl->attribute.len ){
		err_fail("attribute 0x%X with offset %lu not exists", att, id);
	}
	
	term_print(rl->attribute.value[id]);
}
*/

__private void term_readline_print(termReadLine_s* rl, utf8_t* str, int* r, unsigned* c){
	const unsigned scrollx = rl->cursor.mode & TERM_READLINE_MODE_SCROLL_COL;
	const unsigned scrolly = rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW;

	utf8Iterator_s it = utf8_iterator(str, 0);
	utf_t utf;

	while( *r < 0 ){
		utf_t utf = utf8_iterator_next(&it);
		if( utf == 0 ) return;
		if( utf >= TERM_READLINE_PRIVATE_UTF ){
			term_print(utf);
			continue;
		}
		++*c;
		if( utf == '\n' || (!scrollx && *c - rl->position.col >= rl->position.width) ){
			++*r;
			*c = rl->position.col;
		}
	}

	if( scrolly ){
		unsigned offsety = rl->cursor.scrollrow;
	   	unsigned width = scrollx ? UINT_MAX : rl->position.width - rl->prompt.len;
		while( offsety > 0 && (utf=utf8_iterator_next(&it)) ){
			if( utf >= TERM_READLINE_PRIVATE_UTF ){
				term_print(utf);
				continue;
			}
			--width;
			if( width == 0 || utf == '\n' ){
				--offsety;
				width = scrollx ? UINT_MAX : rl->position.width;
			}
		}
	}

	if( it.begin == rl->text.str && scrollx){
		utf_t utf;
		unsigned offsetx = rl->cursor.scrollcol;
		while( offsetx>0 && (utf=utf8_iterator_next(&it)) && utf != '\n' ){
			if( utf >= TERM_READLINE_PRIVATE_UTF ){
				term_print(utf);
				continue;
			}
			--offsetx;
		}
		if( offsetx && !utf ) return;
		if( offsetx && utf == '\n' ) utf8_iterator_prev(&it);
	}

	term_readline_line_clear(rl, *r, *c);
	
	while( *r - rl->position.row < (int)rl->position.height && (utf=utf8_iterator_next(&it)) ){
		if( utf > TERM_READLINE_PRIVATE_UTF ){
			term_print(utf);
		}
		else{
			term_print(utf);
			++(*c);
			//dbg_warning("c:%d col:%u w:%u", *c, rl->position.col, rl->position.width);
			//dbg_warning("r:%d row:%u h:%u", *r, rl->position.row, rl->position.height);

			if( utf == '\n' || *c - rl->position.col >= rl->position.width ){
				++(*r);
				*c = rl->position.col;
				if( !scrolly && *r >= (int)rl->position.height ){
					dbg_error("r:%d row:%u h:%u", *r, rl->position.row, rl->position.height);
					--(*r);
					--rl->position.row;
					putchar(' ');
				}
				if( scrollx ){
					if( utf != '\n' ){ 
						while( (utf=utf8_iterator_next(&it)) && utf != '\n' ) {
							if( utf > TERM_READLINE_PRIVATE_UTF ){
								term_print(utf);
							}
						}
					}
					if( !utf ) return;
					unsigned offsetx = rl->cursor.scrollcol;
					while( offsetx > 0 && (utf=utf8_iterator_next(&it)) && utf != '\n' ){
						if( utf > TERM_READLINE_PRIVATE_UTF ){
							term_print(utf);
							continue;
						}
						--offsetx;
					}
					if( offsetx && !utf ) return;
					if( offsetx && utf == '\n' ) utf8_iterator_prev(&it);
				}
				term_gotorc(*r, *c);
				term_readline_line_clear(rl, *r, *c);
			}
		}
	}
}

__private void term_readline_downsize_clear(termReadLine_s* rl, unsigned r, unsigned c){
	term_color_reset();
	term_font_attribute(TERM_FONT_RESET);

	if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW ){
		for(; r < rl->position.height + rl->position.row; ++r){
			term_gotorc(r, c);
			for(; c  < rl->position.width + rl->position.col; ++c ){
				putchar(' ');
			}
			c = rl->position.col;
		}
		if( r == rl->position.height ){
			for(; c < rl->position.width + rl->position.col; ++c ){
				putchar(' ');
			}
		}
	}
	else{
		for(; r < rl->position.rowLast; ++r){
			term_gotorc(r, c);
			for(; c  < rl->position.width + rl->position.col; ++c ){
				putchar(' ');
			}
			c = rl->position.col;
		}
		if( r == rl->position.rowLast ){
			for(; c < rl->position.colLast; ++c ){
				putchar(' ');
			}
		}
	}
}

__private void term_readline_cursor_update(termReadLine_s* rl, int promptr, unsigned promptc){
	const unsigned scrollx = rl->cursor.mode & TERM_READLINE_MODE_SCROLL_COL;
	const unsigned scrolly = rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW;
	int r = promptr;
	unsigned c = promptc;
	utf8Iterator_s it = utf8_iterator(rl->text.str, 0);

	if( scrolly ){
		//dbg_warning("skip row:%u", rl->cursor.scrollrow);
		unsigned offsety = rl->cursor.scrollrow;
	   	unsigned width = scrollx ? UINT_MAX : rl->position.width - rl->prompt.len;
		utf_t utf;
		while( offsety > 0 && (utf=utf8_iterator_next(&it)) ){
			if( utf >= TERM_READLINE_PRIVATE_UTF ){
				continue;
			}
			--width;
			if( width == 0 || utf == '\n' ){
				--offsety;
				width = scrollx ? UINT_MAX : rl->position.width;
			}
		}
	}

	if( scrollx ){
		utf_t utf;
		unsigned offsetx = rl->cursor.scrollcol;
		while( offsetx > 0 && (utf=utf8_iterator_next(&it)) && utf != '\n' ) --offsetx;
		if( offsetx && !utf ){
			rl->cursor.col = c;
			rl->cursor.row = r;
			return;
		}
		else if( offsetx && utf == '\n' ){
			utf8_iterator_prev(&it);
		}
	}

	while( rl->it.str != it.str ){
		utf_t utf = utf8_iterator_next(&it);
		if( !utf ){
			return;
		}	
		if( utf >= TERM_READLINE_PRIVATE_UTF ) continue;
		++c;
		if( utf == '\n' || c - rl->position.col >= rl->position.width ){
			c = rl->position.col;
			if( scrollx ){
				if( utf != '\n' ) while( (utf=utf8_iterator_next(&it)) && utf != '\n' );
				if( !utf ){
					return;
				}
				unsigned offsetx = rl->cursor.scrollcol;
				while( offsetx > 0 && (utf=utf8_iterator_next(&it)) && utf != '\n'){
					--offsetx;
				}
				if( offsetx && !utf ){
					return;
				}
				else if( offsetx && utf == '\n' ){
					utf8_iterator_prev(&it);
				}
			}
			++r;
		}
	}
	rl->cursor.col = c;
	rl->cursor.row = r;
}

void term_readline_draw(termReadLine_s* rl){
	int r = rl->position.row;
	unsigned c = rl->position.col;
	int pr;
	unsigned pc;

	term_gotorc(r, c);
	if( r >= 0 && rl->cursor.scrollrow == 0 ){
		if( rl->prompt.str ) term_readline_print(rl, rl->prompt.str, &r, &c);
		pr = r;
		pc = c;
	}
	else{
		pr = r;
		pc = c;
	}
	term_readline_print(rl, rl->text.str, &r, &c);

	term_readline_downsize_clear(rl, r, c);
	rl->position.colLast = c;
	rl->position.rowLast = r;
	
	term_readline_cursor_update(rl, pr, pc);
	term_gotorc(rl->cursor.row, rl->cursor.col);
}
/*
utf_t term_readline_attribute_new(termReadLine_s* rl, char* att){
	if( rl->attribute.len >= rl->attribute.size ){
		rl->attribute.size += TERM_READLINE_ATTRIBUTE_SIZE;
		if( mem_resize(rl->attribute.value, char*, rl->attribute.size) ){
			err_fail("on resize attribute");
		}
	}
	size_t id = rl->attribute.len++;
	rl->attribute.value[id] = str_dup(att, 0);
	utf_t ret = id + TERM_READLINE_PRIVATE_UTF + 1;
	dbg_info("create attribute id:%lu utf:0x%X", id, ret);
	return ret;
}

err_t term_readline_attribute_change(termReadLine_s* rl, utf_t utf, char* att){
	size_t id = utf - TERM_READLINE_PRIVATE_UTF - 1;
	if( id >= rl->attribute.len ){
		err_push("attribute 0x%X with offset %lu not exists", utf, id);
		return -1;
	}

	free(rl->attribute.value[id]);
	rl->attribute.value[id] = att;

	return 0;
}
*/
void term_readline_mode(termReadLine_s* rl, int mode){
	rl->cursor.mode = mode;
}

const utf8_t* term_readline_str_raw(termReadLine_s* rl){
	return rl->text.str;
}

utf8_t* term_readline_str(termReadLine_s* rl){
	utf8_t* str = mem_many(utf8_t, rl->text.len + 1);
	utf8Iterator_s itsrc = utf8_iterator(rl->text.str, 0);
	utf8Iterator_s itdst = utf8_iterator(str, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&itsrc)) ){
		if( utf > TERM_READLINE_PRIVATE_UTF ){
			continue;
		}
		utf8_iterator_replace(&itdst, utf);
	}
	utf8_iterator_replace(&itdst, 0);
	return str;
}

void term_readline_prompt_change(termReadLine_s* rl, utf8_t* prompt){
	rl->prompt.str = prompt;
	rl->prompt.len = prompt ? utf_width(prompt) : 0;
}

void term_readline_put(termReadLine_s* rl, utf_t utf){
	if( rl->it.str - rl->it.begin + 4 >= (long)rl->text.size - 1 ){
		rl->text.size += rl->position.width + 5;
		if( mem_resize(rl->text.str, utf8_t, rl->text.size) ){
			err_fail("eom");
		}
		size_t offset = rl->it.str - rl->it.begin;
		rl->it.begin = rl->text.str;
		rl->it.str =  rl->it.begin + offset;
	}
	if( rl->cursor.mode & TERM_READLINE_MODE_INSERT ){
		dbg_info("insert 0x%X", utf);
		utf8_iterator_insert(&rl->it, utf);
	}
	else if( rl->cursor.mode & TERM_READLINE_MODE_REPLACE ){
		dbg_info("replace 0x%X", utf);
		utf8_iterator_replace(&rl->it, utf);
	}

	if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_COL ){
		if( utf == '\n' ){
			rl->cursor.scrollcol = 0;
		}
		else{
			int lw = term_readline_line_left_width(rl);
			if( lw - (int)rl->cursor.scrollcol >= (int)rl->position.width ){
				++rl->cursor.scrollcol;
			}
		}
	}
	
	if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW && (int)rl->cursor.row - rl->position.row + 1 > (int)rl->position.height ){
		dbg_info("row:%d curr:%u n:%d h:%d", rl->position.row, rl->cursor.row, (int)rl->cursor.row - rl->position.row, rl->position.height);
		++rl->cursor.scrollrow;
	}
}

void term_readline_puts(termReadLine_s* rl, utf8_t* str){
	utf8Iterator_s it = utf8_iterator(str, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) ){
		term_readline_put(rl, utf);
	}
}

void term_readline_del(termReadLine_s* rl){
	utf8_iterator_delete(&rl->it);
}

void term_readline_backspace(termReadLine_s* rl){
	if( utf8_iterator_prev(&rl->it) )
		utf8_iterator_delete(&rl->it);
}

__private void dbg_cursor(termReadLine_s* rl){
	fprintf(stderr, "%s\n", rl->it.begin);
	utf8Iterator_s c = utf8_iterator(rl->it.begin, 0);
	
	while( c.str != rl->it.str ){
		utf8_iterator_next(&c);
		fprintf(stderr, " ");
	}
	fprintf(stderr,"^\n");
}

void term_readline_cursor_next(termReadLine_s* rl){
	utf_t u;
	while( (u=utf8_iterator_next(&rl->it)) >= TERM_READLINE_PRIVATE_UTF );
	if( u ){	
		if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_COL ){//
			int lw = term_readline_line_left_width(rl);
			dbg_info("lw:%d scrol:%u", lw, rl->cursor.scrollcol);
			if( lw - (int)rl->cursor.scrollcol >= (int)rl->position.width ){
				++rl->cursor.scrollcol;
			}
		}
	}
	dbg_cursor(rl);
}

void term_readline_cursor_prev(termReadLine_s* rl){
	while( utf8_iterator_prev(&rl->it) >= TERM_READLINE_PRIVATE_UTF );

	if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_COL && rl->cursor.scrollcol > 0 ){//
		int lw = term_readline_line_left_width(rl);
		if( rl->position.row == (int)rl->cursor.row ) lw -= rl->prompt.len;
		dbg_info("prev lw:%d scrol:%u", lw, rl->cursor.scrollcol);
	   	if( lw - (int)rl->cursor.scrollcol < 0 ){
			dbg_info("dec scroll");
			--rl->cursor.scrollcol;
		}
	}
	dbg_cursor(rl);
}

void term_readline_cursor_end(termReadLine_s* rl){
	while( *rl->it.str && *rl->it.str != '\n' ){
		term_readline_cursor_next(rl);
	}
}

void term_readline_cursor_home(termReadLine_s* rl){
	utf_t utf;
	while( (utf=utf8_iterator_prev(&rl->it)) && utf != '\n' );
	if( utf ) utf8_iterator_next(&rl->it);
	rl->cursor.scrollcol = 0;
}

void term_readline_cursor_scroll_left(termReadLine_s* rl){
	if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_COL && rl->cursor.scrollcol > 0 ) rl->cursor.scrollcol -= 1;
}

void term_readline_cursor_scroll_right(termReadLine_s* rl){
	if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_COL ) ++rl->cursor.scrollcol;
	dbg_info("cursor.scroll: %u", rl->cursor.scrollcol);
}

void term_readline_cursor_up(termReadLine_s* rl){
	int col = (int)term_readline_line_left_width(rl) - (int)rl->cursor.scrollcol;
	int wid = rl->position.width;
	utf_t utf;
	while( wid > 0 && (utf=utf8_iterator_prev(&rl->it)) ){
		if( utf >= TERM_READLINE_PRIVATE_UTF ) continue;
		if( utf == '\n' ){
			int cl = (int)term_readline_line_left_width(rl) - (int)rl->cursor.scrollcol;
			wid = cl - col;
		}
		else{
			--wid;
		}
	}
	if( rl->cursor.scrollrow > 0 && (int)rl->cursor.row - rl->position.row <= 0 ){
		--rl->cursor.scrollrow;
	}	
}

void term_readline_cursor_down(termReadLine_s* rl){
	int col = (int)term_readline_line_left_width(rl) - (int)rl->cursor.scrollcol;
	int wid = rl->position.width;
	utf_t utf;
	while( wid > 0 && (utf=utf8_iterator_next(&rl->it)) ){
		if( utf >= TERM_READLINE_PRIVATE_UTF ) continue;
		if( utf == '\n' ){
			int cl = (int)term_readline_line_left_width(rl) - (int)rl->cursor.scrollcol;
			wid = col - cl;
		}
		else{
			--wid;
		}
	}
	if( (int)rl->cursor.row - rl->position.row + 1 >= (int)rl->position.height ){
		++rl->cursor.scrollrow;
	}
}

void term_readline_cursor_pagdn(termReadLine_s* rl){
	unsigned hei = rl->position.height;
	while( hei-->0 ){
		term_readline_cursor_down(rl);
	}
}

void term_readline_cursor_pagup(termReadLine_s* rl){
	unsigned hei = rl->position.height;
	while( hei-->0 ){
		term_readline_cursor_up(rl);
	}
}

void term_readline_mode_ir_toggle(termReadLine_s* rl){
	if( rl->cursor.mode & TERM_READLINE_MODE_INSERT ){
		rl->cursor.mode &= ~TERM_READLINE_MODE_INSERT;
		rl->cursor.mode |= TERM_READLINE_MODE_REPLACE;
	}
	else if( rl->cursor.mode & TERM_READLINE_MODE_REPLACE){
		rl->cursor.mode &= ~TERM_READLINE_MODE_REPLACE;
		rl->cursor.mode |= TERM_READLINE_MODE_INSERT;
	}
}

void term_readline_process_key(termReadLine_s* rl, termKey_s key){
	switch( key.escape ){
		case TERM_KEY_BACKSPACE:
			term_readline_backspace(rl);
		break;

		case TERM_KEY_DC:
			term_readline_del(rl);
		break;

		case TERM_KEY_LEFT:
			term_readline_cursor_prev(rl);
		break;

		case TERM_KEY_RIGHT:
			term_readline_cursor_next(rl);
		break;

		case TERM_KEY_CTRL_LEFT:
			term_readline_cursor_scroll_left(rl);
		break;

		case TERM_KEY_SHIFT_RIGHT:
			term_readline_cursor_scroll_right(rl);
		break;

		case TERM_KEY_UP:
			if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW) term_readline_cursor_up(rl);
		break;

		case TERM_KEY_DOWN:
			if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW) term_readline_cursor_down(rl);
		break;

		case TERM_KEY_PPAGE:
			if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW) term_readline_cursor_pagup(rl);
		break;

		case TERM_KEY_NPAGE:
			if( rl->cursor.mode & TERM_READLINE_MODE_SCROLL_ROW) term_readline_cursor_pagdn(rl);
		break;

		case TERM_KEY_FIND:
			term_readline_cursor_home(rl);
		break;

		case TERM_KEY_SELECT:
			term_readline_cursor_end(rl);
		break;

		case TERM_KEY_IC:
			term_readline_mode_ir_toggle(rl);
		break;

		case 0:
			if( key.ch ) term_readline_put(rl,key.ch);
		break;
	}
}
