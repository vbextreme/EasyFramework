#ifdef _APP
#include <stdio.h>
#include <stdlib.h>
#include "easyalloc.h"
#include <easybenchmark.h>
#include <easythread.h>
#include <easymath.h>

#define NLOOP 100000
#define NTEST 100

VOID* subm;

VOID* testm(VOID* arg)
{
	THREAD_START(arg,FLOAT64*,tel);
	
	INT32* ptest0;
	INT32* ptest1;
	INT32* ptest2;
	INT32* ptest3;
	INT32* ptest4;
	INT32* ptest5;
	INT32* ptest6;
	INT32* ptest7;
	INT32* ptest8;
	INT32* ptest9;
	INT32 rtest[NTEST];
	
	INT32 i,k=0;
	for ( i = 0; i < NLOOP; ++i )
	{
		ptest0 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest0 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest0 = mth_random(9999);
		ptest1 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest1 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest1 = mth_random(9999);
		ptest2 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest2 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest2 = mth_random(9999);
		ptest3 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest3 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest3 = mth_random(9999);
		ptest4 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest4 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest4 = mth_random(9999);
		ptest5 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest5 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest5 = mth_random(9999);
		ptest6 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest6 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest6 = mth_random(9999);
		ptest7 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest7 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest7 = mth_random(9999);
		ptest8 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest8 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest8 = mth_random(9999);
		ptest9 = mal_malloc(subm,sizeof(INT32));
			if ( !ptest9 ) {puts("Fail.0"); mal_dbg_mem(subm); exit(0);}
		rtest[k] = *ptest9 = mth_random(9999);
	
		mal_free(ptest0);
		mal_free(ptest1);
		mal_free(ptest2);
		mal_free(ptest3);
		mal_free(ptest4);
		mal_free(ptest5);
		mal_free(ptest6);
		mal_free(ptest7);
		mal_free(ptest8);
		mal_free(ptest9);
	}
	
	THREAD_END(NULL);
	
}

VOID* mtest(VOID* arg)
{
	THREAD_START(arg,FLOAT64*,tel);
	
	INT32* ptest0;
	INT32* ptest1;
	INT32* ptest2;
	INT32* ptest3;
	INT32* ptest4;
	INT32* ptest5;
	INT32* ptest6;
	INT32* ptest7;
	INT32* ptest8;
	INT32* ptest9;
	INT32 rtest[NTEST];
	
	INT32 i,k=0;
	for ( i = 0; i < NLOOP; ++i )
	{
		ptest0 = malloc(sizeof(INT32));
			if ( !ptest0 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest0 = mth_random(9999);
		ptest1 = malloc(sizeof(INT32));
			if ( !ptest1 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest1 = mth_random(9999);
		ptest2 = malloc(sizeof(INT32));
			if ( !ptest2 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest2 = mth_random(9999);
		ptest3 = malloc(sizeof(INT32));
			if ( !ptest3 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest3 = mth_random(9999);
		ptest4 = malloc(sizeof(INT32));
			if ( !ptest4 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest4 = mth_random(9999);
		ptest5 = malloc(sizeof(INT32));
			if ( !ptest5 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest5 = mth_random(9999);
		ptest6 = malloc(sizeof(INT32));
			if ( !ptest6 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest6 = mth_random(9999);
		ptest7 = malloc(sizeof(INT32));
			if ( !ptest7 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest7 = mth_random(9999);
		ptest8 = malloc(sizeof(INT32));
			if ( !ptest8 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest8 = mth_random(9999);
		ptest9 = malloc(sizeof(INT32));
			if ( !ptest9 ) {puts("Fail.0"); exit(0);}
		rtest[k] = *ptest9 = mth_random(9999);
	
		free(ptest0);
		free(ptest1);
		free(ptest2);
		free(ptest3);
		free(ptest4);
		free(ptest5);
		free(ptest6);
		free(ptest7);
		free(ptest8);
		free(ptest9);
	}
	
	THREAD_END(NULL);
}

int main(int argc,char** argv)
{
	mth_initrandom();
	
	INT32 sz = (NTEST * (8 + sizeof(INT32))) * 5 ;
	printf("sz:%d\n",sz);
	
	subm = malloc( sz );
	mal_init(subm, sz );
	
	FLOAT64 te0;
	
	THR t1 = thr_new(testm,0,0,1);
	THR t2 = thr_new(testm,0,0,2);
	THR t3 = thr_new(testm,0,0,3);
	THR t4 = thr_new(testm,0,0,4);
	
	te0 = bch_get();
	thr_run(t1,NULL);
	thr_run(t2,NULL);
	thr_run(t3,NULL);
	thr_run(t4,NULL);
	
	
	thr_waitthr(t1);
	thr_waitthr(t2);
	thr_waitthr(t3);
	thr_waitthr(t4);
	
	te0 = bch_get() - te0;
	
	printf("mt malmalloc:%f\n",te0);
	thr_free(t1);
	thr_free(t2);
	thr_free(t3);
	thr_free(t4);
	free(subm);
	
	
	t1 = thr_new(mtest,0,0,1);
	t2 = thr_new(mtest,0,0,2);
	t3 = thr_new(mtest,0,0,3);
	t4 = thr_new(mtest,0,0,4);
	
	te0 = bch_get();
	thr_run(t1,NULL);
	thr_run(t2,NULL);
	thr_run(t3,NULL);
	thr_run(t4,NULL);
	
	
	thr_waitthr(t1);
	thr_waitthr(t2);
	thr_waitthr(t3);
	thr_waitthr(t4);
	
	te0 = bch_get() - te0;
	
	printf("mt stdmalloc:%f\n",te0);
	thr_free(t1);
	thr_free(t2);
	thr_free(t3);
	thr_free(t4);
	
	
	VOID* submem = malloc(1024);
		mal_init(submem,1024);
		
	puts("INIT");
		//mal_dbg_mem(submem);
	
	
	puts("MALLOC A");
		INT32* a = mal_malloc(submem,sizeof(INT32));
		//mal_dbg_mem(submem);
	
	puts("MALLOC B");
		INT32* b = mal_malloc(submem,sizeof(INT32));
		//mal_dbg_mem(submem);
	
	puts("MALLOC C");
		INT32* c = mal_malloc(submem,sizeof(INT32));
		//mal_dbg_mem(submem);
	
	puts("FREE B");
		mal_free(b);
		//mal_dbg_mem(submem);
	
	puts("RE MALLOC B * 3");
		b = mal_malloc(submem,sizeof(INT32) * 3);
		//mal_dbg_mem(submem);
	
	
	puts("FREE C");
		mal_free(c);
		//mal_dbg_mem(submem);
	
	
	puts("FREE A");
		mal_free(a);
		//mal_dbg_mem(submem);
	
	puts("FREE B");
		mal_free(b);
		//mal_dbg_mem(submem);
	//return 0;
	
	
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
	 submem = malloc(10000);
	submem = mal_init(submem,10000);
	
	st = bch_get();
	for ( i = 0; i < 1000000; ++i )
	{
		INT32* test1 = mal_malloc(submem, sizeof(INT32) * 512 );
		INT32* test2 = mal_malloc(submem, sizeof(INT32) * 512 );
		INT32* test3 = mal_malloc(submem, sizeof(INT32) * 512 );
		INT32* test4 = mal_malloc(submem, sizeof(INT32) * 512 );
		mal_free(test1);
		mal_free(test2);
		mal_free(test3);
		mal_free(test4);
	}
	en = bch_get() - st;
	
	printf("mal_malloc:%f\n",en);
	free(submem);
    
    return 0;
    
}
#endif
