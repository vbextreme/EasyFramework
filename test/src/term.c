#include "test.h"
#include <ef/terminfo.h>
#include <ef/termcapstr.h>
#include <ef/termlink.h>

#include <ef/memory.h>
#include <ef/file.h>

/*@test -E --term 'test terminal'*/

/*@fn*/
void test_term(__unused const char* argA, __unused const char* argB){
	err_enable();
	term_begin();
	__mem_free char* lcex = path_resolve("../../build/" TERM_EF_EXTEND);
	if( term_load(NULL, term_name()) ){
		dbg_error("ops");
		err_print();
	}
	if( term_load(lcex, term_name_ef()) ){
		dbg_error("ops");
		err_print();
	}

	term_ca_mode(1);
	term_gotorc(0,0);
	puts("hello");
	term_cursor_visible(0);
	term_flush();
	delay_ms(1000);
	term_cursor_visible(1);
	term_clear(TERM_CLEAR);
	term_ca_mode(0);

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

	term_escapef("term_move", 50,50);



	term_end();
	err_restore();
}

