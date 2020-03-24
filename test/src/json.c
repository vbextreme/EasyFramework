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
