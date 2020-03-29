#include "test.h"
#include <ef/term.h>
#include <ef/memory.h>
#include <ef/file.h>

/*@test -E --term 'test terminal'*/

/*@fn*/
void test_term(__unused const char* argA, __unused const char* argB){
	err_enable();
	term_begin();
	term_screen_size_enable();
	term_input_enable();
		
	__mem_free char* lcex = path_resolve("../../build/" TERM_EF_EXTEND);
	printf("../../build::%s\n",lcex);

	if( term_load(NULL, term_name()) ){
		dbg_error("ops load default term");
		err_print();
		term_input_disable();
		term_end();
		exit(1);
	}
	if( term_load(lcex, term_name_ef()) ){
		dbg_error("ops load ef term");
		err_print();
		term_input_disable();
		term_end();
		exit(1);
	}
	term_update_key();
	term_flush();
	

	termReadLine_s* rl = term_readline_new(U8("inp: "), -1, -1, -1, -1);
	term_readline_draw(rl);
	
	char escColRes[256];
	char escColLGreen[256];
	char escColLRed[256];

	term_escapemk(escColRes, "color_reset");
	term_escapemk(escColLGreen, "color16_fg", TERM_COLOR_LIGHT_GREEN);
	term_escapemk(escColLRed, "color16_fg", TERM_COLOR_LIGHT_RED);
	utf_t ecolreset = term_readline_attribute_new(rl, escColRes);
	utf_t ecolgreen = term_readline_attribute_new(rl, escColLGreen);
	utf_t ecolred   = term_readline_attribute_new(rl, escColLRed);


	term_flush();

	int rlMode = TERM_READLINE_MODE_INSERT | TERM_READLINE_MODE_SCROLL_COL;// | TERM_READLINE_MODE_AUTOSCROLL_COL;
	term_readline_mode(rl, rlMode);

	while(1){
		termKey_s key = term_input_extend();
		if( key.ch == 0 && key.escape == 0 ) break;
		if( key.ch == TERM_INPUT_CHAR_ESC ) break;
		dbg_info("ch:%d es:%d", key.ch, key.escape);
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

			case TERM_KEY_SHIFT_LEFT:
				term_readline_cursor_scroll_left(rl);
			break;

			case TERM_KEY_SHIFT_RIGHT:
				term_readline_cursor_scroll_right(rl);
			break;

			case TERM_KEY_FIND:
				term_readline_cursor_home(rl);
			break;

			case TERM_KEY_SELECT:
				term_readline_cursor_end(rl);
			break;

			case TERM_KEY_F5:
				if( rlMode & TERM_READLINE_MODE_SCROLL_COL ){
					rlMode &= ~TERM_READLINE_MODE_SCROLL_COL;
				}
				else{
					rlMode |= TERM_READLINE_MODE_SCROLL_COL;
				}
				term_readline_mode(rl, rlMode);
			break;

			case TERM_KEY_F4:
				term_readline_put(rl,ecolred);
			break;
	
			case TERM_KEY_F3:
				term_readline_put(rl,ecolgreen);
			break;
			
			case TERM_KEY_F2:
				term_readline_put(rl, ecolreset);
			break;

			case TERM_KEY_IC:
				if( rlMode & TERM_READLINE_MODE_INSERT ){
					rlMode &= ~TERM_READLINE_MODE_INSERT;
					rlMode |= TERM_READLINE_MODE_REPLACE;
				}
				else if( rlMode & TERM_READLINE_MODE_REPLACE){
					rlMode &= ~TERM_READLINE_MODE_REPLACE;
				   	rlMode |= TERM_READLINE_MODE_INSERT;
				}
				term_readline_mode(rl, rlMode);
			break;

			case 0:
				if( key.ch ){
					term_readline_put(rl,key.ch);
				}
			break;
		}
		term_readline_draw(rl);
		term_flush();
	}

	term_readline_free(rl);
	puts("");
	term_flush();


/*
	term_ca_mode(1);
	term_gotorc(0,0);
	puts("hello");
	term_cursor_visible(0);
	term_flush();
	delay_ms(1000);
	term_cursor_visible(1);
	term_clear(TERM_CLEAR);
	term_ca_mode(0);

	term_clear(TERM_CLEAR);
	term_flush();

	int x,y;
	if( term_cursor_position(&y, &x) ){
		err_fail("cursor position");
	}
	printf("y:%d, x:%d\n", y, x);
	puts("next try in raw mode");
	term_flush();
	delay_ms(1000);
	term_clear(TERM_CLEAR);
	term_raw_enable();
	printf("raw mode, goto to x=4 y=3");
	term_gotorc(3,4);
	term_flush();
	if( term_cursor_position(&y, &x) ){
		term_raw_disable();
		err_fail("cursor position");
	}
	printf("y:%d, x:%d\n", y, x);
	delay_ms(1000);
	term_raw_disable();

	term_color16_bk(TERM_COLOR_WHYTE);
	term_color16_fg(TERM_COLOR_BLACK);
	term_print("black");
	term_color_reset();
	term_print(" ");


	char* colorn[] = {
		"black",
		"red",
		"green",
		"yellow",
		"blue",
		"magenta",
		"cyan",
		"gray", 
		NULL
	};
	
	for( size_t i = 0; colorn[i]; ++i){
		term_color16_fg(i);
		term_print(colorn[i]);
		term_color_reset();
		term_print("\n");		
	}
	for( size_t i = 0; colorn[i]; ++i){
		term_color16_fg(60+i);
		term_print("light ");
		term_print(colorn[i]);
		term_color_reset();
		term_print("\n");		
	}

	puts("");
	
	term_print("insert: ");
	term_flush();

	utf8_t inp[80] = {0};
	utf8_t* ins = inp;
	*ins = 0;
	termKey_s key;

	while( (key=term_input_extend()).ch ){
		if( key.ch == '\n' ) break;
		utf_putch(ins, key.ch);
		//printf("%s",ins);
		utf_t prev;
		ins = (utf8_t*)utf_next(&prev, ins);
		utf8_fputchar(stdout, prev);
		term_flush();
	}
	utf_putch(ins, 0);
	puts("");
	
	if( key.escape ){
		printf("escape  :%d\n", key.escape);
	}
	if( inp[0] ){
		printf("inserted:'%s'\n", inp);
	}
	//term_escapef("term_move", 50,50);

*/

	term_input_disable();
	term_end();
	err_restore();
}

