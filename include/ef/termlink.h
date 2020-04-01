#ifndef __EF_TERM_LINK_INFO_H__
#define __EF_TERM_LINK_INFO_H__

// TODO att common attribute for printing

#include <ef/type.h>

/** alternate mode, 1 enter, 0 exit -1 reload cap*/
void term_ca_mode(int ee);

/** move cursor, origin 0,0, if r || c  < 0 reload cap */
void term_gotorc(int r, int c);

typedef enum { 
	TERM_CLEAR_RELOAD = -1,   /**< reload cap*/ 
	TERM_CLEAR,               /**< clear all screen */
   	TERM_CLEAR_BEGIN_LINE,    /**< clear to begin line*/
   	TERM_CLEAR_END_OF_LINE,   /**< clear to end of line*/
	TERM_CLEAR_END_OF_SCREEN, /**< clear to end of screen*/
	TERM_CLEAR_COUNT          /**< private not used*/
} termClearMode_e;

/** clear line or screen
 * @see termClearMode_e
 * @param mode mode to clear
 */
void term_clear(termClearMode_e mode);

typedef enum {
	TERM_CURSOR_RELOAD = -1,
	TERM_CURSOR_LEFT,
	TERM_CURSOR_RIGHT,
	TERM_CURSOR_DOWN,
	TERM_CURSOR_UP,
	TERM_CURSOR_HOME,
	TERM_CURSOR_END,
	TERM_CURSOR_COUNT
}termCursor_e;

/** move cursor */
void term_cursor(termCursor_e mode);

/** set visible or invisible cursor */
void term_cursor_visible(int v);

/** 1 store / 0 load cursor */
void term_cursor_mem(int store);

typedef enum{
	TERM_COLOR_RELOAD = -1,
	TERM_COLOR_BLACK,
	TERM_COLOR_RED,
	TERM_COLOR_GREEN,
	TERM_COLOR_YELLOW,
	TERM_COLOR_BLUE,
	TERM_COLOR_MAGENTA,
	TERM_COLOR_CYAN,
	TERM_COLOR_GRAY,
	TERM_COLOR_LIGHT_GRAY = 60,
	TERM_COLOR_LIGHT_RED,
	TERM_COLOR_LIGHT_GREEN,
	TERM_COLOR_LIGHT_YELLOW,
	TERM_COLOR_LIGHT_BLUE,
	TERM_COLOR_LIGHT_MAGENTA,
	TERM_COLOR_LIGHT_CYAN,
	TERM_COLOR_WHYTE
} termColor_e;

/** set background color 16 */
void term_color16_bk(termColor_e color);

/** set foreground color 16 */
void term_color16_fg(termColor_e color);

/** set background color 256 */
void term_color256_bk(int color);

/** set foreground color 256 */
void term_color256_fg(int color);

/** set background color 24k */
void term_color24_bk(unsigned char r, unsigned char g, unsigned char b);

/** set foreground color 24k */
void term_color24_fg(unsigned char r, unsigned char g, unsigned char b);

/** reset to default color */
void term_color_reset(void);

typedef enum {
	TERM_FONT_RELOAD = -1,
	TERM_FONT_BOLT,
	TERM_FONT_ITALIC,
	TERM_FONT_UNDERLINE,
	TERM_FONT_RESET,
	TERM_FONT_COUNT
}termFontAttribute_e;

/** set font attribute */
void term_font_attribute(termFontAttribute_e att);

/** change scroll region */
void term_change_scroll_region(int startRow, int endRow);

/** resize terminal */
void term_resize(int w, int h);

/** enable mouse event */
void term_mouse(int enable);

/** enable mouse move report */
void term_mouse_move(int enable);

/** enable mouse focus report */
void term_mouse_focus(int enable);

/** read cursor position, origin 0,0
 * @param r row
 * @param c columns
 * @return -1 error 0 successfull
 */
err_t term_cursor_position(int* r, int* c);

/** call this function intercept sigint and restore all terminal attribute before exit(1) */
void term_endon_sigint(void);

#endif 
