#include "test.h"

/*@test -v --vector 'test vector'*/

/*@fn*/
void test_vector(__unused const char* argA, __unused const char* argB){
	err_enable();

	int* v = vector_new(int, 3, NULL);
	vector_push_back(v,1);
	vector_push_back(v,2);
	vector_push_back(v,3);
	vector_push_back(v,4);
	dbg_info("vector.count:%lu",vector_count(v));
	
	int* k = vector_get_push_back(v);
	*k = 5;

	dbg_info("remove");
	vector_remove(v, 2);
	vector_foreach(v, i){
		dbg_info("%d",v[i]);
	}
	dbg_info("vector.count:%lu",vector_count(v));

	dbg_info("add");
	vector_add(v,2,3);
	vector_foreach(v, i){
		dbg_info("%d",v[i]);
	}
	dbg_info("vector.count:%lu",vector_count(v));

	vector_free(v);

	char** str = vector_new(char*, 3, free);
	vector_push_back(str, str_dup("ciao",0));
	vector_push_back(str, str_dup("mondo",0));
	vector_push_back(str, str_dup("ok",0));
	vector_push_back(str, str_dup("va",0));

	vector_foreach(str, i){
		dbg_info("%s", str[i]);
	}

	vector_free(str);

	err_restore();
}

