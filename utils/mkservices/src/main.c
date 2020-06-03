#include "gencode.h"
#include <ef/optex.h>

typedef void(*test_f)(const char* a, const char* b);

typedef enum{
	ARG_HELP,
	ARG_SERVICE,
	ARG_OBJECT,
	ARG_INTROSPECT,
	ARG_MATCH,
	ARG_INCLUDE,
	ARG_HEADER,
	ARG_C,
	ARG_VERBOSE,
	ARG_MAKE,
	ARG_LIST,
	ARG_ACTIVABLE,
	ARG_ACQUIRED
}optarg_e;

argdef_s args[] = {
	{0, 'h', "help",       ARGDEF_NOARG, NULL, "help/usage"},
	{0, 's', "service",    ARGDEF_STR,   NULL, "service name"},
	{0, 'o', "object",     ARGDEF_STR,   NULL, "object name, if not set obj=service with replace . with /"},
	{0, 'i', "introspect", ARGDEF_NOARG, NULL, "introspect service, use with -s -o"},
	{0, 't', "match",      ARGDEF_NOARG, NULL, "generate code only with interface is equal with service name"},
	{0, 'I', "include",    ARGDEF_STR,   NULL, "dir include"},
	{0, 'H', "header",     ARGDEF_STR,   NULL, "header name"},
	{0, 'C', "c",          ARGDEF_STR,   NULL, ".c filename"},
	{0, 'v', "verbose",    ARGDEF_NOARG, NULL, "verbose make on stdout"},
	{0, 'm', "make",       ARGDEF_NOARG, NULL, "generate code from introspect service, use with -s -o -t -H -C -v"},
	{0, 'l', "ls",         ARGDEF_NOARG, NULL, "list services, can use with -a -A"},
	{0, 'a', "activable",  ARGDEF_NOARG, NULL, "disable show activable on ls"},
	{0, 'A', "acquired",   ARGDEF_NOARG, NULL, "disable show acquired on ls"},
	{0, 0, NULL, ARGDEF_NOARG, NULL, NULL}
};

int main(int argc, char* argv[]){
	err_begin();
	err_enable();

	const char* service = NULL;
	__mem_free char* object = NULL;
	__mem_free char* include = NULL;
	__mem_free char* header = NULL;
	__mem_free char* hfilename = NULL;
	int activable = 1;
	int acquired = 1;
	int onlyThisInterface = 0;
	int verbose = 0;
	FILE* fh = stdout;
	FILE* fc = stdout;

	int unparsed = opt_parse(args, argv, argc);
	
	if( unparsed < 0 ){
		opt_error(argc, argv);
		return 1;
	}

	if( opt_enabled(args, ARG_HELP) ){
		opt_usage(args, argv[0]);
		return 0;
	}

	if( opt_enabled(args, ARG_SERVICE) )   service   = opt_arg_str(args, ARG_SERVICE);
	if( opt_enabled(args, ARG_OBJECT) )    object    = str_dup(opt_arg_str(args, ARG_OBJECT), 0);
	if( opt_enabled(args, ARG_ACTIVABLE) ) activable = !activable;
	if( opt_enabled(args, ARG_ACQUIRED) )  acquired  = !acquired;
	if( opt_enabled(args, ARG_MATCH) )     onlyThisInterface = !onlyThisInterface;
	if( opt_enabled(args, ARG_VERBOSE) )   verbose = !verbose;

	if( !object && service ){
		object = str_printf("/%s", service);
		char* rp = object;
		while( (rp=strchr(rp, '.')) ){
			*rp = '/';
			++rp;
		}
	}

	if( opt_enabled(args, ARG_INCLUDE) ){
		include = path_resolve(opt_arg_str(args, ARG_INCLUDE));
	}
	else{
		include = path_resolve("./");
	}

	if( opt_enabled(args, ARG_HEADER) ){
		header  = str_dup(opt_arg_str(args, ARG_HEADER), 0);
		hfilename = str_printf("%s/%s", include, header);
		fh = fopen(hfilename, "w");
		if( !fh ){
			err_pushno("open");
			err_fail("open file %s", hfilename);
		}
	}

	if( opt_enabled(args, ARG_C) ){
		__mem_free char* path = path_resolve(opt_arg_str(args, ARG_C));
		fc = fopen(path, "w");
		if( !fc ){
			err_pushno("open");
			err_fail("open file %s", path);
		}
	}

	if( opt_enabled(args, ARG_LIST) ){
		services_list(acquired, activable);
	}

	if( opt_enabled(args, ARG_INTROSPECT) ){
		if( service == NULL ) err_fail("introspect required service name");
		service_introspect(service, object);
	}

	if( opt_enabled(args, ARG_MAKE) ){
		if( service == NULL ) err_fail("make required service name");
		if( header == NULL ) err_fail("make required header name");
		service_make(service, object, onlyThisInterface, fh, fc, header, verbose);
	}

	err_end();
	return 0;
}

