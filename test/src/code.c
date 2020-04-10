#include "test.h"
#include <ef/file.h>
#include <ef/str.h>
#include <ef/regex.h>
#include <ef/terminput.h>

/*@test -Z --code 'private code generation'*/

/*@fn*/
void test_code(__unused const char* argA, __unused const char* argB){
	err_enable();

	__mem_free char* path = path_resolve("../../ef/term/titostring.h");
	__stream_close stream_s* f = stream_open(path, "r", 0, 4096);
	__mem_free char* pathdest = path_resolve("../../include/ef/termkey.h");
	__stream_close stream_s* d = stream_open(pathdest, "w", 0660, 4096);
	if( !f || !d ){
		err_fail("open files");
		return;
	}
	
	stream_out(d, "#ifndef __EF_TERM_KEY_H__\n", 0);
	stream_out(d, "#define __EF_TERM_KEY_H__\n\n", 0);

	char* line;
	while( stream_inp_string(f, &line, '\n', 1) > 0 && str_ancmp(line, "__private char* termKeyToStr") ){
		free(line);
	}
	if( !line ){
		dbg_fail("point code");
	}
	free(line);
	
	size_t id = TERM_INPUT_EXTEND_OFFSET;
	while( stream_inp_string(f, &line, '\n', 0) > 0 && strcmp(line, "};") ){
		__vector_free char** capture = str_regex(line, "[ \t]*\"([^\"]+)\"", 0);
		if( !capture ){
			dbg_fail("ops: '%s'", line);
			continue;
		}
		if( vector_count(capture) == 2 ){
			__mem_free char* toup = str_dup(capture[1], 0);
			str_toupper(toup, capture[1]);
			if( !str_ancmp(capture[1], "key") ){
				stream_printf(d, "#define TERM_%s %lu\n", toup, id++);
			}
			else{
				stream_printf(d, "#define TERM_KEY_%s %lu\n", toup, id++);
			}
		}
		vector_foreach(capture, i) free(capture[i]);
		free(line);
	}
	free(line);

	stream_out(d, "\n#endif\n", 0);

	err_restore();
}

