#include "test.h"
#include <ef/json.h>
#include <ef/file.h>

/*@test -j --json 'test json, if argA is setted print json error, if argB is setted exit after first error'*/

__private size_t test_count(const char* path){
	__dir_close dir_s* dir = dir_open(path);
	if( dir == NULL ){
		err_fail("open %s", path);
	}
	size_t count = 0;
	dir_foreach(dir, file){
		if( dirent_currentback(file) ) continue;
		if( dirent_type(file) == DT_REG ) ++count;
	}
	return count;
}

__private int js_integer(__unused json_s* js, const char* value, size_t len){
	if( json_long_validation(NULL, value,len) ) return JSON_ERR_NUMBER;
	return 0;
}

__private int js_double(__unused json_s* js, const char* value, size_t len){
	if( json_float_validation(NULL, value, len) ) return JSON_ERR_NUMBER;	
	return 0;
}

__private void jinit(json_s* js){
	js->valueFloat = js_double;
	js->valueInteger = js_integer;
}


/*@fn*/
void test_json(const char* argA, const char* argB){
	err_enable();
	json_begin();

struct atest{
	int* a;
	long* b;
	char* str;
	int** v;
};

//char* btest = "{ \"a\": 123, \"b\": 456 }";
//char* btest = "{ \"a\": 123, \"b\": 456, \"str\": \"hello\"}";
//char* btest = "{ \"a\": 123, \"b\": 456, \"str\": \"hello\", \"v\": [ 1, 2, 3] }";
char* btest = "[{ \"a\": 123, \"k\": 5, \"b\": 456, \"str\": \"hello\", \"v\": [ 7, 8, 9] },{ \"a\": 101112, \"b\": 131415, \"str\": \"world\", \"v\": [ 16, 17, 18] }]";


//jsonDef_s* jd = json_parse_new_object(sizeof(struct atest));
jsonDef_s* jvv = json_parse_new_vector();
jsonDef_s* jd = json_parse_declare_object(jvv, "atest", 0, sizeof(struct atest));

json_parse_declare_int(jd, "a", offsetof(struct atest, a));
json_parse_declare_long(jd, "b", offsetof(struct atest, b));
json_parse_declare_string(jd, "str", offsetof(struct atest, str));
jsonDef_s* jv = json_parse_declare_vector(jd, "v", offsetof(struct atest, v));
json_parse_declare_int(jv, "v.int", 0);

/*
struct atest* foo = json_parse(jd, btest);
if( !foo ){
	dbg_error("No foo");
	err_fail("foo");
	return;
}

json_def_free(jd);

dbg_info("dump");
dbg_info("&foo: %p", foo);
dbg_info("&foo->a: %p", foo->a);
dbg_info("foo->a: %d", *foo->a);
dbg_info("&foo->b: %p", foo->b);
dbg_info("foo->b: %ld", *foo->b);
dbg_info("&foo->str: %p", foo->str);
dbg_info("foo->str: %s", foo->str);
dbg_info("&foo->v: %p", foo->v);
dbg_info("foo->v.count: %lu", vector_count(foo->v));
vector_foreach(foo->v, i){
	dbg_info("foo->v[%lu]::%d", i, *foo->v[i]);
}
*/

struct atest** foo = json_parse(jvv, btest);
if( !foo ){
	dbg_error("No foo");
	err_fail("foo");
	return;
}

json_def_free(jvv);

dbg_info("dump %lu", vector_count(foo));
vector_foreach(foo,i){
	if( foo[i]->a ) dbg_info("foo->a: %d", *foo[i]->a);
	if( foo[i]->b ) dbg_info("foo->b: %ld", *foo[i]->b);
	if( foo[i]->str ) dbg_info("foo->str: %s", foo[i]->str);
	if( foo[i]->v ){
		dbg_info("foo->v.count: %lu", vector_count(foo[i]->v));
		vector_foreach(foo[i]->v, j){
			dbg_info("foo->v[%lu]::%d", j, *(foo[i]->v[j]));
		}
	}
}

return;
	char* ts = "hello world \\n \\tok\\n\\u2764 love it";
	char* test = json_unescape(NULL, ts, strlen(ts));
	if( test ){
		printf("<TEST>\n%s\n</TEST>\n", test);
	}

	const char* pathValid = "../jsontest/valid";
	const char* pathInvalid = "../jsontest/invalid";
	__mem_free char* pathValidR = path_resolve(pathValid);
	__mem_free char* pathInvalidR = path_resolve(pathInvalid);
	size_t ntest = test_count(pathValid) + test_count(pathInvalid);
	size_t count = 0;	
	{
		__dir_close dir_s* dir = dir_open(pathValidR);
		if( dir == NULL ){
			err_fail("open %s", pathValidR);
		}
		dir_foreach(dir, file){
			if( dirent_currentback(file) ) continue;
			++count;
			printf("[%3lu/%3lu]%s:", count, ntest, dirent_name(file));
			//fflush(stdout);
			char jfile[PATH_MAX];
			sprintf(jfile, "%s/%s", pathValidR, dirent_name(file));
			//printf("%s\n", jfile);
			json_s js = {0};
			jinit(&js);
			__fd_close int fd = fd_open(jfile, "r", 0);
			if( fd < 0 ){
				err_fail("open %s", jfile);
			}
			__mem_free char* data = fd_slurp(NULL, fd, 4096, 1);
			if( data == NULL ){
				err_fail("on slurp %s", jfile);
			}

			if( json_lexer(&js, data) ){
				puts("fail");
				if( argA != NULL ) json_error(&js);
				if( argB != NULL ) err_fail("user interrupt");
			}
			else{
				puts("ok");
			}
		}
	}

	{
		__dir_close dir_s* dir = dir_open(pathInvalidR);
		if( dir == NULL ){
			err_fail("open %s", pathInvalidR);
		}
		dir_foreach(dir, file){
			if( dirent_currentback(file) ) continue;
			++count;
			printf("[%3lu/%3lu]%s:", count, ntest, dirent_name(file));
			char jfile[PATH_MAX];
			sprintf(jfile, "%s/%s", pathInvalidR, dirent_name(file));
			//printf("%s\n", jfile);
			json_s js = {0};
			jinit(&js);
			__fd_close int fd = fd_open(jfile, "r", 0);
			if( fd < 0 ){
				err_fail("open %s", jfile);
			}
			__mem_free char* data = fd_slurp(NULL, fd, 4096, 1);
			if( data == NULL ){
				err_fail("on slurp %s", jfile);
			}
			//dbg_info("<SLURP>%s<SLURP>", data);

			if( !json_lexer(&js, data) ){
				puts("fail");
				if( argB != NULL ) err_fail("user interrupt");
			}
			else{
				puts("ok");
			}
		}
	}

	//json_s js = {0};
	//char* data = "\"a\"\"b\"";
	//json_lexer(&js, data);
	//json_error(&js);	
	json_end();
	err_restore();
}
