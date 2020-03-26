#include <ef/type.h>
#include <ef/memory.h>
#include <ef/file.h>
#include <ef/str.h>
#include <ef/delay.h>
#include <ef/err.h>
#include <ef/terminfo.h>
#include <ef/termcapstr.h>
#include <ef/termlink.h>

void term_ca_mode(int ee){
	static tiData_s* em[2] = {0};

	if( ee == -1 || !em[0] ){
		em[0] = term_info(cap_enter_ca_mode); 
		em[1] = term_info(cap_exit_ca_mode);
		if( !em[0] || !em[1] ){
			err_fail("%s || %s", cap_enter_ca_mode, cap_exit_ca_mode);
		}
		if( ee == -1 ) return;
	}
	
	tvariable_s var[10] = { [1].type = 0, [1].l = 0 };
	term_escape_make_print(em[!ee]->str, var);
}

void term_gotorc(int r, int c){
	static tiData_s* em = NULL;

	if( r < 0 || c < 0 || !em ){
		em = term_info(cap_cursor_address); 
		if( !em ) err_fail("%s", cap_cursor_address);
		if( r < 0 || c < 0 ) return;
	}
	
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = r,
		[2].type = 0, [2].l = c,
   	};
	term_escape_make_print(em->str, var);
}

void term_clear(termClearMode_e mode){
	static tiData_s* em[TERM_CLEAR_COUNT] = {0};

	if( mode == TERM_CLEAR_RELOAD || !em[TERM_CLEAR] ){
		char* capn[] = {
			[TERM_CLEAR] = cap_clear_screen,
			[TERM_CLEAR_BEGIN_LINE]= cap_clr_bol,
			[TERM_CLEAR_END_OF_LINE]= cap_clr_eol,
			[TERM_CLEAR_END_OF_SCREEN]= cap_clr_eos,
			NULL
		};
		for( size_t i = 0; capn[i]; ++i){
			em[i] = term_info(capn[i]); 
			if( !em[i] ) err_fail("%s", capn[i]);
		}
	}
	if( mode <= TERM_CLEAR_RELOAD || mode >= TERM_CLEAR_COUNT ) return;
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = 0
   	};
	term_escape_make_print(em[mode]->str, var);
}

void term_cursor(termCursor_e mode){
	static tiData_s* em[TERM_CURSOR_COUNT] = {0};

	if( mode == TERM_CURSOR_RELOAD || !em[TERM_CURSOR_LEFT] ){
		char* capn[] = {
			[TERM_CURSOR_LEFT] = cap_cursor_left,
			[TERM_CURSOR_RIGHT]= cap_cursor_right,
			[TERM_CURSOR_DOWN] = cap_cursor_down,
			[TERM_CURSOR_UP]   = cap_cursor_up,
			[TERM_CURSOR_HOME] = cap_cursor_home,
			[TERM_CURSOR_END]  = cap_cursor_to_ll,
			NULL
		};
		for( size_t i = 0; capn[i]; ++i){
			em[i] = term_info(capn[i]); 
			if( !em[i] ) err_fail("%s", capn[i]);
		}
	}
	if( mode <= TERM_CURSOR_RELOAD || mode >= TERM_CURSOR_COUNT ) return;
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = 0
   	};
	term_escape_make_print(em[mode]->str, var);

}

void term_cursor_visible(int v){
	static tiData_s* em[2] = {0};

	if( v == -1 || !em[0] ){
		em[0] = term_info(cap_cursor_visible); 
		em[1] = term_info(cap_cursor_invisible);
		if( !em[0] || !em[1] ){
			err_fail("%s || %s", cap_cursor_invisible, cap_cursor_visible);
		}
		if( v < 0 ) return;
	}
	
	tvariable_s var[10] = { [1].type = 0, [1].l = 0 };
	term_escape_make_print(em[!v]->str, var);
}

void term_color16_bk(termColor_e color){
	static tiData_s* em = NULL;

	if( color < 0 || !em ){
		em = term_info("color16_bk"); 
		if( !em ) err_fail("color16_bk");
		if( color < 0 ) return;
	}
	
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = color,
   	};
	term_escape_make_print(em->str, var);
}

void term_color16_fg(termColor_e color){
	static tiData_s* em = NULL;

	if( color < 0 || !em ){
		em = term_info("color16_fg"); 
		if( !em ) err_fail("color16_fg");
		if( color < 0 ) return;
	}
	
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = color,
   	};
	term_escape_make_print(em->str, var);
}

void term_color_reset(void){
	static tiData_s* em = NULL;

	if( !em ){
		em = term_info("color_reset"); 
		if( !em ) err_fail("color_reset");
	}
	
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = 0,
   	};
	term_escape_make_print(em->str, var);
}



