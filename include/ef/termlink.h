#ifndef __EF_TERM_LINK_INFO_H__
#define __EF_TERM_LINK_INFO_H__

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
	TERM_CLEAR_COUTN          /**< private not used*/
} termClearMode_e;

/** clear line or screen
 * @see termClearMode_e
 * @param mode mode to clear
 */
void term_clear(termClearMode_e mode);

#endif 
