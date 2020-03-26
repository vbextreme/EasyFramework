#include "test.h"
#include <ef/terminfo.h>
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




	term_end();
	err_restore();
}

