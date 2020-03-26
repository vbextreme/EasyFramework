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
	static tiData_s* em[TERM_CLEAR_COUTN] = {0};

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
	if( mode <= TERM_CLEAR_RELOAD || mode >= TERM_CLEAR_COUTN ) return;
	tvariable_s var[10] = { 
		[1].type = 0, [1].l = 0
   	};
	term_escape_make_print(em[mode]->str, var);
}








