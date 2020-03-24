#include "test.h"

/*@test -e --err 'test error'*/

/*@fn*/
void test_err(__unused const char* argA, __unused const char* argB){
	err_enable();

	err_pushf("test %d\n",1);
	const char* str = err_descript();
	TESTF("err1", strcmp(str, "test 1\n") );
	err_pushf("test %d\n",2);
	str = err_descript();
	TESTF("err2", strcmp(str, "test 1\ntest 2\n") );
	err_clear();
	str = err_descript();
	TESTF("err3", *str );

	err_restore();
}

