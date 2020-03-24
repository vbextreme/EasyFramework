#ifndef __TEST_H__
#define __TEST_H__

#include <ef/type.h>
#include <ef/memory.h>
#include <ef/err.h>
#include <ef/optex.h>
#include <ef/mth.h>
#include <ef/vector.h>
#include <ef/file.h>
#include <ef/delay.h>
#include <ef/str.h>

#define TESTF(STR,F) do{\
	printf("test:%s::",STR);\
	if( (F) ){\
		printf("error\n");\
		err_print();\
	}\
	else{\
		printf("ok\n");\
	}\
}while(0)

#define TESTT(STR,F) do{\
	printf("test:%s::",STR);\
	if( !(F) ){\
		printf("error\n");\
		err_print();\
	}\
	else{\
		printf("ok\n");\
	}\
}while(0)

char** data_word(const char* pdata);
void data_word_free(char** v, const char* pdata);

#endif
