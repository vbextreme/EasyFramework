#include "test.h"
#include "autogen.h"
#include <ef/utf8.h>

typedef void(*test_f)(const char* a, const char* b);

AUTOGEN_PROTO

__private test_f tfn[] = {
	AUTOGEN_FN
	NULL
};


typedef enum{
	ARG_HELP,
	ARG_A,
	ARG_B,
	ARG_AUTO
}optarg_e;

argdef_s args[] = {
	{0, 'h', "help", ARGDEF_NOARG, NULL, "help/usage"},
	{0, 'a', "arg-a", ARGDEF_STR,NULL,"arg a"},
	{0, 'b', "arg-b", ARGDEF_STR,NULL,"arg a"},
	AUTOGEN_OPT
	{0, 0, NULL, ARGDEF_NOARG, NULL, NULL}
};

int main(int argc, char* argv[]){
	err_begin();
	err_enable();
	utf_begin();

	int unparsed = opt_parse(args, argv, argc);
	
	if( unparsed < 0 ){
		opt_error(argc, argv);
		return 1;
	}

	if( opt_enabled(args, ARG_HELP) ){
		opt_usage(args, argv[0]);
		return 0;
	}

	const char* a = opt_enabled(args,ARG_A) ? opt_arg_str(args,ARG_A) : NULL;
	const char* b = opt_enabled(args,ARG_B) ? opt_arg_str(args,ARG_B) : NULL;

	for( size_t i = 0; tfn[i]; ++i){
		if( opt_enabled(args, ARG_AUTO+i) ){
			dbg_info("start");
			tfn[i](a,b);
			dbg_info("end");
		}
	}
	dbg_info("end");
	err_end();
	return 0;
}

