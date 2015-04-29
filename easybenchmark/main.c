#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "easybenchmark.h"

BCH_PERF_GLOBAL;


void test1()
{
	BCH_PERF_START;
	
	INT32 i;
	UINT32 ni = 0;
	for ( i = 0; i < 1000000; ++i )
		ni += i;
	
	BCH_PERF_STOP;
}

void test2()
{
	BCH_PERF_START;
	
	INT32 i;
	UINT32 ni = 0;
	for ( i = 0; i < 1000000; ++i )
		ni += i;
	
	BCH_PERF_STOP;
}


int main()
{
    
    BCH_PERF_INIT;
    
    BCH_PERF_START;
    
    test1();
    
    
    BCH_PERF_PAUSE;
    INT32 i,ni;
    for ( i = 0; i < 1000000; ++i )
		ni += i;
	BCH_PERF_START;
        
    test2();
    
    BCH_PERF_STOP;
    
    bch_perf(stdout,__perf__);
    
    return 0;
}

#endif
