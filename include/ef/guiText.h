#ifndef __EF_GUI_TEXT_H__
#define __EF_GUI_TEXT_H__

#include <ef/gui.h>

#define GUI_TEXT_REND_TEXT    0x000001
#define GUI_TEXT_REND_CURSOR  0x000008
#define GUI_TEXT_REND_SCROLL  0x000010
#define GUI_TEXT_REND_CURON   0x000020

#define GUI_TEXT_INSERT		  0x000100
#define GUI_TEXT_SCROLL_X     0x000200
#define GUI_TEXT_SCROLL_Y     0x000400
#define GUI_TEXT_CUR_VISIBLE  0x000800
#define GUI_TEXT_SEL          0x001000

#define GUI_TEXT_FLAGS_END    24
#define GUI_TEXT_FLAGS_MASK   0xFFFFFF

#define GUI_TEXT_CURSOR_THIN      (0<<GUI_TEXT_FLAGS_END)
#define GUI_TEXT_CURSOR_LIGHT     (1<<GUI_TEXT_FLAGS_END)
#define GUI_TEXT_CURSOR_PLENTIFUL (2<<GUI_TEXT_FLAGS_END)
#define GUI_TEXT_CURSOR_FAT       (3<<GUI_TEXT_FLAGS_END)

#define GUI_TEXT_WORD_SEP " \n\t`~!@#$%^&*()+{}|[]\\;':\"<>?,./"

#define GUI_THEME_TEXT_BLINK        "text.cursor.blink"
#define GUI_THEME_TEXT_CURSOR       "text.cursor.mode"
#define GUI_THEME_TEXT_CURSOR_COLOR "text.cursor.color"
#define GUI_THEME_TEXT_SEL_COLOR    "text.sel.color"
#define GUI_THEME_TEXT_TAB          "text.tab"
#define GUI_THEME_TEXT_FOREGROUND   "text.foreground"
#define GUI_THEME_TEXT_SCROLL_X     "text.scroll.x"
#define GUI_THEME_TEXT_SCROLL_Y     "text.scroll.y"

typedef struct guiText{
	guiImage_s* render;
	g2dPoint_s scroll;
	g2dCoord_s cursor;
	int cursorOffsetX;
	int cursorOffsetY;
	guiTimer_s* blink;
	unsigned blinktime;

	utf8_t* text;
	size_t len;
	size_t size;
	size_t resize;
	utf8Iterator_s it;
	utf8_t* selStart;
	utf8_t* selEnd;
	utf8_t* clipmem;	

	ftFonts_s* fonts;
	g2dColor_t foreground;
	g2dColor_t select;
	g2dColor_t colCursor;
	unsigned flags;
	unsigned tabspace;
	unsigned spacesize;
}guiText_s;

/** create new text*/
guiText_s* gui_text_new(ftFonts_s* font, g2dColor_t foreground, g2dColor_t select, g2dColor_t colCursor, unsigned tabspace, unsigned blinktime, unsigned flags);

/** attach text to gui*/
gui_s* gui_text_attach(gui_s* gui, guiText_s* txt);

/** free text*/
void gui_text_free(guiText_s* txt);

/** sets flags*/
void gui_text_flags_set(gui_s* gui, unsigned flags);

/** insert/replace*/
void gui_text_ir_toggle(gui_s* gui);

/** get raw str*/
const utf8_t* gui_text_str_raw(gui_s* gui);

/** get text string, remember to free*/
utf8_t* gui_text_str(gui_s* gui);

/** write a string*/
void gui_text_print(gui_s* gui, const utf8_t* text);

/** sel text*/
void gui_text_sel(gui_s* gui);

/** unsel text*/
void gui_text_unsel(gui_s* gui);

/** delete selection*/
void gui_text_sel_del(gui_s* gui);

/** get text selection, remember to free*/
utf8_t* gui_text_sel_get(gui_s* gui);

/** right len*/
size_t gui_text_line_right_len(gui_s* gui);

/** left len*/
size_t gui_text_line_left_len(gui_s* gui);

/** back one line*/
utf8_t* gui_text_back_line_ptr(gui_s* gui);

/** next ona line*/
utf8_t* gui_text_next_line_ptr(gui_s* gui);

/** put char*/
void gui_text_put(gui_s* gui, utf_t utf);

/** del char*/
void gui_text_del(gui_s* gui);

/** backspace*/
void gui_text_backspace(gui_s* gui);

/** change type of cursaor*/
void gui_text_cursor_change(gui_s* gui, unsigned modeflags);

/** cursor next*/
void gui_text_cursor_next(gui_s* gui);

/** cursor prev*/
void gui_text_cursor_prev(gui_s* gui);

/** cursor to end*/
void gui_text_cursor_end(gui_s* gui);

/** cursor to home*/
void gui_text_cursor_home(gui_s* gui);

/** scroll left*/
void gui_text_cursor_scroll_left(gui_s* gui);

/** scroll right*/
void gui_text_cursor_scroll_right(gui_s* gui);

/** cursor up */
err_t gui_text_cursor_up(gui_s* gui);

/** cursor down*/
err_t gui_text_cursor_down(gui_s* gui);

/**cursor pag down*/
void gui_text_cursor_pagdn(gui_s* gui);

/** cursors pagup*/
void gui_text_cursor_pagup(gui_s* gui);

/** set cursor on coordinate*/
void gui_text_cursor_on_position(gui_s* gui, unsigned x, unsigned y);

/** render cursor*/
void gui_text_render_cursor(gui_s* gui, __unused guiImage_s** img, __unused void* ud);

/** render text*/
void gui_text_render_text(gui_s* gui, int partial);

/** render scroll*/
void gui_text_render_scroll(gui_s* gui);

/** redraw text*/
void gui_text_redraw(gui_s* gui, int partial);

/** event on key*/
int gui_text_event_key(gui_s* gui, xorgEvent_s* event);

/** event redraw*/
int gui_text_event_redraw(gui_s* gui, __unused xorgEvent_s* ev);

/** event clipboard*/
int gui_text_event_clipboard(gui_s* gui, xorgEvent_s* ev);

/** event free*/
int gui_text_event_free(gui_s* gui, __unused xorgEvent_s* ev);

int gui_text_event_themes(__unused gui_s* gui, xorgEvent_s* ev);

/** event timer blink*/
int gui_text_timer_blink(guiTimer_s* timer);

/** event focus*/
int gui_text_event_focus(gui_s* gui, xorgEvent_s* ev);

/** event mouse*/
int gui_text_event_mouse(gui_s* gui, xorgEvent_s* event);

int gui_text_event_move(gui_s* gui, xorgEvent_s* event);

#endif
