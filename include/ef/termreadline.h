#ifndef __EF_TERM_READ_LINE_H__
#define __EF_TERM_READ_LINE_H__

#include <ef/type.h>
#include <ef/utf8.h>
#include <ef/terminput.h>

/** enable insetr mode*/
#define TERM_READLINE_MODE_INSERT         0x01
/** enable replace mode*/
#define TERM_READLINE_MODE_REPLACE        0x02
/** enable columns scroll*/
#define TERM_READLINE_MODE_SCROLL_COL     0x04
/** enable rows scroll*/
#define TERM_READLINE_MODE_SCROLL_ROW     0x08

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
	unsigned scrollrow;
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

/** create new readline
 * @param prompt text before input
 * @param r row position, if -1 get current row
 * @param c col position, if -1 get current col
 * @param w width, if -1 get screen width
 * @param h height, if -1 get screen height
 * @return new readline or NULL for error
 */
termReadLine_s* term_readline_new(utf8_t* prompt, int r, int c, int w, int h);

/** free readline */
void term_readline_free(termReadLine_s* rl);

/** change prompt*/
void term_readline_prompt_change(termReadLine_s* rl, utf8_t* prompt);

/** get number of glyph before cursor*/
size_t term_readline_line_left_width(termReadLine_s* rl);

/** get number of glyph after cursor*/
size_t term_readline_line_right_width(termReadLine_s* rl);

/** draw readline*/
void term_readline_draw(termReadLine_s* rl);

/** create new attribute
 * @param rl readline obj
 * @param att string attribute, the attribute can't display any glyph
 * @return unicode associated to attribute
 */
utf_t term_readline_attribute_new(termReadLine_s* rl, char* att);

/** change attribute to previus unicode
 * @param rl readline
 * @param utf previus unicode
 * @param att new attribute
 * @return 0 successfull, -1 error
 */
err_t term_readline_attribute_change(termReadLine_s* rl, utf_t utf, char* att);

/** set readline mode
 * @param readline
 * @param mode any of TERM_READLINE_MODE_
 */
void term_readline_mode(termReadLine_s* rl, int mode);

/** return a unicode string with contains a readline text, remember to free return value*/
utf8_t* term_readline_str(termReadLine_s* rl);

/** same term_readline_str but return utf attribute in text and not need to free value returned*/
const utf8_t* term_readline_str_raw(termReadLine_s* rl);

/** write a unicode to readline*/
void term_readline_put(termReadLine_s* rl, utf_t utf);

/** write a string*/
void term_readline_puts(termReadLine_s* rl, utf8_t* str);

/** delete a glyph*/
void term_readline_del(termReadLine_s* rl);

/** backspace */
void term_readline_backspace(termReadLine_s* rl);

/** move cursor to next */
void term_readline_cursor_next(termReadLine_s* rl);

/** move cursor prev*/
void term_readline_cursor_prev(termReadLine_s* rl);

/** move cursor to end line*/
void term_readline_cursor_end(termReadLine_s* rl);

/** move cursor to begin line*/
void term_readline_cursor_home(termReadLine_s* rl);

/** scroll left*/
void term_readline_cursor_scroll_left(termReadLine_s* rl);

/** scroll right*/
void term_readline_cursor_scroll_right(termReadLine_s* rl);

/** move cursor up*/
void term_readline_cursor_up(termReadLine_s* rl);

/** move cursor down*/
void term_readline_cursor_down(termReadLine_s* rl);

/** pagup */
void term_readline_cursor_pagdn(termReadLine_s* rl);

/** pagdw */
void term_readline_cursor_pagup(termReadLine_s* rl);

/** toggle input/replace mode*/
void term_readline_mode_ir(termReadLine_s* rl);

/** after read key, process key with this functiona and after this draw and flush*/
void term_readline_process_key(termReadLine_s* rl, termKey_s key);

#endif 
