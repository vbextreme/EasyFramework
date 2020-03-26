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
	delay_ms(1000);
	term_clear(TERM_CLEAR);
	puts("bye bye");
	delay_ms(1000);	

	//term_escapef(cap_exit_ca_mode);

	term_ca_mode(0);	

	term_end();
	err_restore();
}

