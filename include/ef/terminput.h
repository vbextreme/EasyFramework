#ifndef __EF_TERM_INPUT_H__
#define __EF_TERM_INPUT_H__

#include <ef/type.h>
#include <ef/utf8.h>

#define TERM_INPUT_CHAR_SCREEN -2
#define TERM_INPUT_CHAR_ESC    0x1B

#define TERM_INPUT_EXTEND_SCREEN 256
#define TERM_INPUT_EXTEND_OFFSET 1024

typedef struct termKey{
	utf_t ch;
	int escape;
}termKey_s;

/** return numbers of rows and cols
 * @param rows if not null return rows
 * @param cols if not null return cols
 * @return 0 successfull -1 error
 */
err_t term_screen_size_get(unsigned* rows, unsigned* cols);

/** enable screen update size */
void term_screen_size_enable(void);

/** flush stdin */
void term_flushin(void);

/** enable terminal input */
void term_input_enable(void);

/** disable terminal input */
void term_input_disable(void);

/** return data available on stdin */
int term_kbhit(void);

/** ungetch key*/
void term_input_ungetch(termKey_s key);

/** return a key, screen resize */
termKey_s term_input_utf8(void);

/** return term_input_utf8 or an escape */
termKey_s term_input_extend(void);

/*
#define TERM_KEY_ESC (0x1B)
#define TERM_CNT_SCREEN_RESIZE "screen_resize"
#define TERM_MOUSE_LEFT         0
#define TERM_MOUSE_MID          1
#define TERM_MOUSE_RIGHT        2
#define TERM_MOUSE_RELEASED     3
#define TERM_MOUSE_SCROLL_UP    4
#define TERM_MOUSE_SCROLL_DOWN  5
#define TERM_MOUSE_SCROLL_LEFT  6
#define TERM_MOUSE_SCROLL_RIGHT 7
#define TERM_MOUSE_MOVE         99

typedef struct termKey{
	char* cnt;
	utf_t utf;
}termKey_s;

typedef struct termMouse{
	char button;
	char shift;
	char meta;
	char control;
	int c;
	int r;
}termMouse_s;

err_t term_screen_size_get(unsigned* rows, unsigned* cols);
void term_screen_size_enable(void);
void term_flushin(void);
int term_screen_size_update(void);
int term_kbhit(void);
termMouse_s term_mouse_get(void);
int term_get_ch(void);
utf_t term_get_utf(void);
termKey_s term_get_ex(termInfo_s* ti);
*/


#endif 
