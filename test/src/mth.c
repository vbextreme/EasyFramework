#include "test.h"

/*@test -M --math 'test math'*/

/*@fn*/
void test_math(__unused const char* argA, __unused const char* argB){
	mth_random_begin();

	TESTT("max", 7 == MTH_MAX(7,3));
	TESTT("max", 7 == MTH_MAX(3,7));
	TESTT("max3", 5 == MTH_3MAX(5,3,1));
	TESTT("max3", 5 == MTH_3MAX(3,5,1));
	TESTT("max3", 5 == MTH_3MAX(1,3,5));
	TESTT("min", 3 == MTH_MIN(7,3));
	TESTT("min", 3 == MTH_MIN(3,7));
	TESTT("min3", 1 == MTH_3MIN(5,3,1));
	TESTT("min3", 1 == MTH_3MIN(3,5,1));
	TESTT("min3", 1 == MTH_3MIN(1,3,5));
	TESTT("roundup", 55 == ROUND_UP(32,55));
	TESTT("roundup^2", 64 == ROUND_UP_POW_TWO32(58));
	TESTT("froundup^2", 128 == mth_round_up_power_two(120));
	TESTT("fastbitcount", 3 == FAST_BIT_COUNT(7));
	
	const long channel = 2000;
	const long alpha = MM_ALPHA(channel,100);
	const long ahpla = MM_AHPLA(channel,alpha);
	long med = 1280;
	for( size_t i = 0; i < 200; ++i){
		med = MM_NEXT(channel,alpha, ahpla, 204, med);
	}
	TESTT("mm_alpha", med == 204);

	TESTT("gtor", MTH_DOUBLE_CMP(6.283, mth_gtor(360.0), 0.005) );
	TESTT("gtor", MTH_DOUBLE_CMP(3.141, mth_gtor(180.0), 0.005) );
	TESTT("gtor", MTH_DOUBLE_CMP(1.570, mth_gtor(90.0), 0.005) );
	TESTT("gtor", MTH_DOUBLE_CMP(0.015, mth_gtor(1.0), 0.005) );

	const size_t ntest = 10000000;
	size_t count = 0;
	
	for( size_t i = 0; i < ntest; ++i ){
		int r = mth_random(100);
		if( r < 0 || r >= 100 ) ++count;
	}
	TESTF("random", count);

	count = 0;
	for( size_t i = 0; i < ntest; ++i ){
		int r = mth_random_range(100, 110);
		if( r < 100 || r > 110 ) ++count;
	}
	TESTF("random_range", count);

	count = 0;
	for( size_t i = 0; i < ntest; ++i ){
		double r = mth_random_f01();
		if( r < 0.0 || r > 1.0 ) ++count;
	}
	TESTF("random_f01", count);

	count = 0;
	for( size_t i = 0; i < ntest; ++i ){
		double r = mth_random_gauss(128.0, 10.0);
		if( r > 127.0 && r < 129.0 ) ++count;
	}
	TESTT("gauss", count > ((ntest*7)/100));

	
	char* e64 = base64_encode("ABC", 4);
	char* d64 = base64_decode(NULL,e64);
	printf("A:%s:%s\n", e64, d64);

	printf("round up pow of two 16: %u\n",ROUND_UP_POW_TWO32(16));
	printf("round down pow of two 16: %u\n",ROUND_DOWN_POW_TWO32(16));
	printf("round down pow of two 17: %u\n",ROUND_DOWN_POW_TWO32(17));

}

