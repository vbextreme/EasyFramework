#ifdef _APP
#include <stdio.h>
#include <stdlib.h>
#include "easyalloc.h"
#include <easybenchmark.h>

int main(int argc,char** argv)
{
	INT32 i;
	FLOAT64 st = bch_get();
	for ( i = 0; i < 1000000; ++i )
	{
		INT32* test1 = malloc( sizeof(INT32) * 512 );
		INT32* test2 = malloc( sizeof(INT32) * 512 );
		INT32* test3 = malloc( sizeof(INT32) * 512 );
		INT32* test4 = malloc( sizeof(INT32) * 512 );
		test1[0] = 0;
		test2[1] = 7;
		test3[100] = 8;
		test4[500] = 9;
		free(test1);
		free(test2);
		free(test3);
		free(test4);
	}
	FLOAT64 en = bch_get() - st;
	
	printf("std_malloc:%f\n",en);
	fflush(stdout);
	VOID* submem = malloc(10000);
	submem = mal_init(submem,10000);
	
	st = bch_get();
	for ( i = 0; i < 1000000; ++i )
	{
		INT32* test1 = mal_malloc(submem, sizeof(INT32) * 512 );
		INT32* test2 = mal_malloc(submem, sizeof(INT32) * 512 );
		INT32* test3 = mal_malloc(submem, sizeof(INT32) * 512 );
		INT32* test4 = mal_malloc(submem, sizeof(INT32) * 512 );
		mal_free(submem,test1);
		mal_free(submem,test2);
		mal_free(submem,test3);
		mal_free(submem,test4);
	}
	en = bch_get() - st;
	
	printf("mal_malloc:%f\n",en);
	free(submem);
    
    return 0;
}
#endif
