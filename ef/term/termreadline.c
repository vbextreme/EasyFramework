#include <ef/termreadline.h>
#include <ef/terminfo.h>
#include <ef/termlink.h>
#include <ef/termmode.h>
#include <ef/terminput.h>
#include <ef/str.h>
#include <ef/memory.h>
#include <ef/err.h>

#define TERM_READLINE_PRIVATE_UTF UTF_PRIVATE0_START
#define TERM_READLINE_ATTRIBUTE_SIZE 80

#define TERM_READLINE_INSERT_MODE  0x0001
#define TERM_READLINE_REPLACE_MODE 0x0002


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
	
	rl->attribute.size = TERM_READLINE_ATTRIBUTE_SIZE;
	rl->attribute.value = mem_many(char*, rl->attribute.size);
	if( !rl->attribute.value ){
		free(rl->text.str);
		free(rl);
		return NULL;
	}
	rl->attribute.len = 0;

	return rl;
}

void term_readline_free(termReadLine_s* rl){
	for( size_t i = 0; i < rl->attribute.len; ++i){
		free(rl->attribute.value[i]);
	}
	free(rl->attribute.value);

	free(rl->text.str);
	free(rl);
}

__private void term_readline_attribute_print(termReadLine_s* rl, utf_t att){
	size_t id = att - TERM_READLINE_PRIVATE_UTF;
	if( id >= rl->attribute.len ){
		err_fail("attribute 0x%X with offset %lu not exists", att, id);
	}
	
	term_print(rl->attribute.value[id]);
}

__private void term_readline_print(termReadLine_s* rl, utf8_t* str, unsigned* r, unsigned* c){
	utf8Iterator_s it = utf8_iterator(str, 0);
	utf_t utf;
	while( (utf=utf8_iterator_next(&it)) ){
		if( rl->it.str == it.str ){
			rl->cursor.col = *c;
			rl->cursor.row = *r;
		}
		if( utf > TERM_READLINE_PRIVATE_UTF ){
			term_readline_attribute_print(rl, utf);
		}
		else{
			utf8_fputchar(stdout, utf);
			++(*c);
			if( *c >= rl->position.width ){
				++(*r);
				*c = rl->position.col;
				if( *r >= rl->position.height ){
					--(*r);
					--rl->position.row;
				}
				term_gotorc(*r, *c);
			}
		}
	}
}

__private void term_readline_downsize_clear(termReadLine_s* rl, unsigned r, unsigned c){
	term_color_reset();
	term_font_attribute(TERM_FONT_RESET);
	for(; r < rl->position.rowLast; ++r){
		term_gotorc(r, c);
		for(; c  < rl->position.width; ++c ){
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

void term_readline_draw(termReadLine_s* rl){
	unsigned r = rl->position.row;
	unsigned c = rl->position.col;

	term_gotorc(r, c);
	if( rl->prompt.str ) term_readline_print(rl, rl->prompt.str, &r, &c);
	term_readline_print(rl, rl->text.str, &r, &c);

	term_readline_downsize_clear(rl, r, c);
	rl->position.colLast = c;
	rl->position.rowLast = r;
	
	term_gotorc(rl->cursor.row, rl->cursor.col);
}

utf_t term_readline_attribute_new(termReadLine_s* rl, char* att){
	if( rl->attribute.len >= rl->attribute.size ){
		rl->attribute.size += TERM_READLINE_ATTRIBUTE_SIZE;
		if( mem_resize(rl->attribute.value, char*, rl->attribute.size) ){
			err_fail("on resize attribute");
		}
	}
	size_t id = rl->attribute.len++;
	rl->attribute.value[id] = str_dup(att, 0);
	return id + TERM_READLINE_PRIVATE_UTF;
}

err_t term_readline_attribute_change(termReadLine_s* rl, utf_t utf, char* att){
	size_t id = utf - TERM_READLINE_PRIVATE_UTF;
	if( id >= rl->attribute.len ){
		err_push("attribute 0x%X with offset %lu not exists", utf, id);
		return -1;
	}

	free(rl->attribute.value[id]);
	rl->attribute.value[id] = att;

	return 0;
}

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
	if( rl->it.str - rl->it.begin + 1 >= (long)rl->text.size - 1 ){
		rl->text.size += rl->position.width + 1;
		if( mem_resize(rl->text.str, utf8_t, rl->text.size) ){
			err_fail("eom");
		}
		size_t offset = rl->it.str - rl->it.begin;
		rl->it.begin = rl->text.str;
		rl->it.str =  rl->it.begin + offset;
	}
	if( rl->cursor.mode & TERM_READLINE_INSERT_MODE ){
		utf8_iterator_insert(&rl->it, utf);
	}
	else if( rl->cursor.mode & TERM_READLINE_REPLACE_MODE ){
		utf8_iterator_replace(&rl->it, utf);
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

void term_readline_cursor_next(termReadLine_s* rl){
	utf8_iterator_next(&rl->it);
}

void term_readline_cursor_prev(termReadLine_s* rl){
	utf8_iterator_prev(&rl->it);
}

void term_readline_cursor_end(termReadLine_s* rl){
	while( utf8_iterator_next(&rl->it) );
}

void term_readline_cursor_home(termReadLine_s* rl){
	while( utf8_iterator_prev(&rl->it) );
}

