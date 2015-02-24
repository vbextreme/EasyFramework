#ifndef EASYOPT_H_INCLUDED
#define EASYOPT_H_INCLUDED

#include <easytype.h>
#include <getopt.h>

typedef struct __MYOPT* MYOPT;

//format :"Argv[:|::],arGv[:[::]],argv"

MYOPT opt_new(INT32 narg, CHAR* format);
VOID opt_free(MYOPT opt);
VOID opt_usage(CHAR* name, MYOPT opt);
INT32 opt_parse(CHAR** carg, MYOPT opt, INT32 argc, CHAR** argv);

#endif // EASYSTRING_H_INCLUDED
