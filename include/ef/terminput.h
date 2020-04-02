#ifndef __EF_TERM_INPUT_H__
#define __EF_TERM_INPUT_H__

#include <ef/type.h>
#include <ef/utf8.h>

#define TERM_INPUT_CHAR_SCREEN -2
#define TERM_INPUT_CHAR_ESC    0x1B

#define TERM_INPUT_EXTEND_SCREEN 256
#define TERM_INPUT_EXTEND_OFFSET 1024

typedef struct termMouse_s{
	int r;
	int c;
	int button;
	int meta;
	int shift;
	int control;
}termMouse_s;

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

/** when receved TERM_KEY_MOUSE, call this function to get mouse position */
termMouse_s term_input_mouse(void);

/** if use term_input_extend use this for reading mouse value, not term_input_mouse */
termMouse_s term_mouse_event(void);

#endif 
