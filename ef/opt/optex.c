#include <ef/optex.h>

#ifndef OPT_ERR_LINE_MAX 
	#define OPT_ERR_LINE_MAX 80
#endif

typedef enum{ OPT_ERR_OK, OPT_ERR_UNKNOW_ARG, OPT_ERR_ARG_REQUIRE_VALUE, OPT_ERR_NOT_SIGNED_VALUE, OPT_ERR_NOT_UNSIGNED_VALUE, OPT_ERR_NOT_DOUBLE_VALUE, OPT_ERR_COUNT } opterr_e;

__private int errno_argc;
__private int errno_subarg;
__private opterr_e errno_info;
__private char* errno_str[] = {
	"no error",
	"unknow argument",
	"argument required value",
	"argument is not signed",
	"argument is not unsigned",
	"argument is not double"
};

__private void opt_reset(argdef_s* args){
	size_t el = 0;
	for(; args[el].vshort; ++el){
		args[el].hasset = 0;
	}
}

__private int opt_type(char* opt){
	if( opt[0] == '-' ){
		if( opt[1] == '-' ){
			return 2;
		}
		return 1;
	}
	return 0;
}

__private int opt_find_long(argdef_s* args, char* opt){
	int el = 0;
	for(; args[el].vshort; ++el){
		if( !strcmp(args[el].vlong, opt) ){
			return el;
		}
	}
	return -1;
}

__private int opt_find_short(argdef_s* args, char opt){
	int el = 0;
	for(; args[el].vshort; ++el){
		if( args[el].vshort == opt ){
			return el;
		}
	}
	return -1;
}

__private int opt_as_signed(long* ptr, char* argv){
	char* en;
	*ptr = strtol(argv, &en, 10);
	return ( *en != 0 ) ? -OPT_ERR_NOT_SIGNED_VALUE : 1;
}

__private int opt_as_unsigned(unsigned long* ptr, char* argv){
	char* en;
	*ptr = strtoul(argv, &en, 10);
	return ( *en != 0 ) ? -OPT_ERR_NOT_UNSIGNED_VALUE : 1;
}

__private int opt_as_double(double* ptr, char* argv){
	char* en;
	*ptr = strtod(argv, &en);
	return ( *en != 0 ) ? -OPT_ERR_NOT_DOUBLE_VALUE : 1;
}

__private int opt_arg_set(argdef_s* arg, int itarg, char** argv, int argc){
	arg->hasset = 1;

	switch( arg->typeParam ){
		case ARGDEF_NOARG:
		return 0;
		
		case ARGDEF_SIGNED:
			if( itarg >= argc ) return -OPT_ERR_ARG_REQUIRE_VALUE;
		return opt_as_signed(arg->autoset, argv[itarg]);
		
		case ARGDEF_UNSIGNED:
			if( itarg >= argc ) return -OPT_ERR_ARG_REQUIRE_VALUE;
		return opt_as_unsigned(arg->autoset, argv[itarg]);
		
		case ARGDEF_DOUBLE:
			if( itarg >= argc ) return -OPT_ERR_ARG_REQUIRE_VALUE;
		return opt_as_double(arg->autoset, argv[itarg]);

		case ARGDEF_STR:
			if( itarg >= argc ) return -OPT_ERR_ARG_REQUIRE_VALUE;
			arg->autoset = argv[itarg];
		return 1;
	}

	dbg_fail("internal error");
	return -1;
}

__private int opt_parse_short(argdef_s* args, int itarg, char** argv, int argc){
	int skipper = 1;
	char* opt = argv[itarg];
	for(++opt; *opt; ++opt ){
		int el = opt_find_short(args, *opt);
		if( el < 0 ){
			errno_argc = itarg;
			errno_subarg = opt - argv[itarg];
			errno_info = OPT_ERR_UNKNOW_ARG;
			return -1;
		}

		int ns = opt_arg_set(&args[el], itarg + skipper, argv, argc);
		if( ns < 0 ){
			errno_info = -ns;
			if( errno_info == OPT_ERR_ARG_REQUIRE_VALUE ){
				errno_argc = itarg;
				errno_subarg = opt - argv[itarg];
			}
			else{
				errno_argc = itarg + skipper;
				errno_subarg = -1;
			}
			return -1;
		}

		skipper += ns;
	}
	return skipper;
}

__private int opt_parse_long(argdef_s* args, int itarg, char** argv, int argc){
	int skipper = 1;

	int el = opt_find_long(args, &argv[itarg][2]);
	if( el < 0 ){
		errno_argc = itarg;
		errno_subarg = -1;
		errno_info = OPT_ERR_UNKNOW_ARG;
		return -1;
	}

	int ns = opt_arg_set(&args[el], itarg + skipper, argv, argc);
	if( ns < 0 ){
		errno_info = -ns;
		if( errno_info == OPT_ERR_ARG_REQUIRE_VALUE ){
			errno_argc = itarg;
			errno_subarg = -1;
		}
		else{
			errno_argc = itarg + skipper;
			errno_subarg = -1;
		}
		return -1;
	}

	skipper += ns;
	return skipper;
}

int opt_parse(argdef_s* args, char** argv, int argc){
	int it;
	errno_argc = 0;
	errno_subarg = -1;
	errno_info = 0;

	opt_reset(args);
	
	it = 1;
	while( it < argc ){
		int sk;

		switch( opt_type(argv[it]) ){
			case 0: return it;
			
			case 1:
				sk = opt_parse_short(args, it, argv, argc);
			break;

			case 2:
				sk = opt_parse_long(args, it, argv, argc);
			break;

			default: 
				dbg_fail("internal error");
			return -1;
		}

		if( sk < 0 ) return -1;
		it += sk;
	}

	return it;
}

void opt_errno(int* ac, int* sub, int* info){
	*ac = errno_argc;
	*sub = errno_subarg;
	*info = errno_info;
}

void opt_error(int argc, char** argv){
	iassert( errno_info >= 0 && errno_info < OPT_ERR_COUNT );

	fprintf(stderr, "argument error(%d):%s\n", errno_info, errno_str[errno_info] );

	if( errno_argc > 0 ){		
		int offsetarg = 0;
		size_t lenE = strlen(argv[errno_argc]);

		if( lenE >= OPT_ERR_LINE_MAX-1 ){
			fprintf(stderr,"%.*s", OPT_ERR_LINE_MAX, argv[errno_argc]);
		}
		else{
			int nch = OPT_ERR_LINE_MAX - 1;
			nch -= lenE;
			int offsetp = errno_argc;
			int offsetn = errno_argc;
			int offsetP = 1;
			int offsetN = 1;

			while( nch > 0 && offsetP && offsetN ){
				if( offsetP && offsetp > 0 ){
					int len =  strlen(argv[offsetp-1]);
					if( nch - len >= 0 ){
						--offsetp;
						nch -= len;
						offsetarg +=len+1;
					}
					else{
						offsetP = 0;
					}
				}
				else{
					offsetP = 0;
				}
				if( offsetN && offsetn < argc-1 ){
					int len = strlen(argv[offsetn+1]);
					if( nch - len >= 0 ){
						++offsetn;
						nch -= len;
					}
					else{
						offsetN = 0;
					}
				}
				else{
					offsetN = 0;
				}
			}

			//nch = OPT_ERR_LINE_MAX - 1;
			if( offsetp > 0 ) fprintf(stderr, "... ");
			for( int i = offsetp; i < errno_argc; ++i ){
				fprintf(stderr, "%s ", argv[i]);
			}
			
			fprintf(stderr, "%s ", argv[errno_argc]);
			if( offsetn > errno_argc ){
				for( int i = offsetn; i < argc; ++i ){
					fprintf(stderr, "%s ", argv[i]);
				}
			}
			if( offsetn < argc - 1) fprintf(stderr, "...");
		}
		fputc('\n',stderr);

		if( errno_subarg > -1 ) offsetarg += errno_subarg;
		if( offsetarg > OPT_ERR_LINE_MAX + 4 ) offsetarg = 0;
		while( offsetarg-->0 ) fputc(' ', stderr);
		fputc('^', stderr);
		fputc('\n',stderr);
	}
}

void opt_usage(argdef_s* args, char* argv0){
	printf("usage %s\n", argv0);
	
	size_t el;
	for(el = 0; args[el].vshort; ++el){
		printf("\t-%c --%s", args[el].vshort, args[el].vlong);
		if( args[el].typeParam > ARGDEF_NOARG && args[el].typeParam < ARGDEF_STR ){
			fputs(" <int>\n\t\t", stdout);
		}
		else if( args[el].typeParam == ARGDEF_STR ){
			fputs(" <str>\n\t\t", stdout);
		}
		else{
			fputs("\n\t\t", stdout);
		}
		puts(args[el].descript);
	}
}















