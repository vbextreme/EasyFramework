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

	termReadLine_s* rl = term_readline_new(U8("inp: "), -1, -1, -1, -1);
	term_readline_draw(rl);
	term_flush();

	while(1){
		termKey_s key = term_input_extend();
		if( key.ch == 0 && key.escape == 0 ) break;
		if( key.ch == TERM_INPUT_CHAR_ESC ) break;
		if( key.ch > 0 ){
			term_readline_put(rl,key.ch);
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

