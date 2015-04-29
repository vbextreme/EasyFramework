#ifndef EASYOPT_H_INCLUDED
#define EASYOPT_H_INCLUDED

#include <easytype.h>

typedef struct __MYOPT* MYOPT;

#define OPTLONG 1000

MYOPT opt_new(CHAR* format);
VOID opt_free(MYOPT opt);
VOID opt_usage(CHAR* name, MYOPT opt);
INT32 opt_parse(CHAR** carg, MYOPT opt, INT32 argc, CHAR** argv);
const CHAR* opt_fromc(MYOPT opt, INT32 c);

#endif // EASYSTRING_H_INCLUDED
