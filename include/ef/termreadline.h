#ifndef __EF_TERM_READ_LINE_H__
#define __EF_TERM_READ_LINE_H__

#include <ef/type.h>
#include <ef/utf8.h>

#define TERM_READLINE_MODE_INSERT         0x0001
#define TERM_READLINE_MODE_REPLACE        0x0002
#define TERM_READLINE_MODE_SCROLL_COL     0x0004
#define TERM_READLINE_MODE_AUTOSCROLL_COL 0x0008


typedef struct termRLArea{
	unsigned col;
	int row;
	unsigned width;
	unsigned height;
	unsigned rowLast;
	unsigned colLast;
}termRLArea_s;

typedef struct termRLText{
	utf8_t* str;
	size_t len;
	size_t size;
}termRLText_s;

typedef struct termRLCursor{
	unsigned row;
	unsigned col;
	unsigned scrollcol;
	unsigned mode;
}termRLCursor_s;

typedef struct termRLUtfPrivate{
	char** value;
	size_t len;
	size_t size;
}termRLUtfPrivate_s;

typedef struct termReadLine{
	termRLArea_s position;
	termRLText_s prompt;
	termRLText_s text;
	utf8Iterator_s it;
	termRLCursor_s cursor;
	termRLUtfPrivate_s attribute;
}termReadLine_s;

termReadLine_s* term_readline_new(utf8_t* prompt, int r, int c, int w, int h);

void term_readline_free(termReadLine_s* rl);

void term_readline_draw(termReadLine_s* rl);

utf_t term_readline_attribute_new(termReadLine_s* rl, char* att);

err_t term_readline_attribute_change(termReadLine_s* rl, utf_t utf, char* att);

void term_readline_mode(termReadLine_s* rl, int mode);

const utf8_t* term_readline_str_raw(termReadLine_s* rl);

const utf8_t* term_readline_str_raw(termReadLine_s* rl);

void term_readline_put(termReadLine_s* rl, utf_t utf);

void term_readline_puts(termReadLine_s* rl, utf8_t* str);

void term_readline_del(termReadLine_s* rl);

void term_readline_backspace(termReadLine_s* rl);

void term_readline_cursor_next(termReadLine_s* rl);

void term_readline_cursor_prev(termReadLine_s* rl);

void term_readline_cursor_end(termReadLine_s* rl);

void term_readline_cursor_home(termReadLine_s* rl);

void term_readline_cursor_scroll_left(termReadLine_s* rl);

void term_readline_cursor_scroll_right(termReadLine_s* rl);

#endif 
