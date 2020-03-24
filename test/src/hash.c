#include "test.h"
#include <ef/rbhash.h>

/*@test -H --hash 'test function hash, use arg-a for file data, without a use internal data'*/

__private uint32_t* calcolate_hash(char** strs, rbhash_f fn, const char* prompt){
	uint32_t* hash = vector_new(uint32_t, vector_count(strs), vector_minimal(strs));
	if( !hash ) return NULL;

	vector_foreach(strs, i){
		uint32_t ahash = fn(strs[i], strlen(strs[i]));
		vector_push_back(hash, ahash);
		if( !(i%100) ){
			printf("\r%s.hashing::%lu%%", prompt, (i*100) / vector_count(strs));
			fflush(stdout);
		}
	}
	return hash;
}

__private int hscmp(const void* A, const void* B){
	uint32_t* a = (uint32_t*)A;
	uint32_t* b = (uint32_t*)B;
	return *b - *a;
}

__private int linec = 91;

__private void hash_distribuition(uint32_t* hs, const char* prompt){
	size_t totalCollision = 0;
	size_t elementCollision = 0;
	size_t elementMaxCollision = 0;
	size_t dist[12] = {0};

	printf("\r%s.sorting::",prompt);
	fflush(stdout);
	vector_qsort(hs, hscmp);
	printf("ok");
	fflush(stdout);
	
	size_t i = 0;
	while( i < vector_count(hs) - 1 ){
		uint32_t hash = hs[i];
		elementCollision = 0;
		size_t j = i+1;
		while( j < vector_count(hs) && hash == hs[j] ){
			++elementCollision;
			++j;
		}
		totalCollision += elementCollision;
		if( elementCollision ) ++totalCollision;
		if( elementCollision > 10 ){
			++dist[11];
		}
		else{
			++dist[elementCollision];
		}
		if( elementCollision > elementMaxCollision ) elementMaxCollision = elementCollision;
		if( !(i%100) ){
			printf("\r%s.distribuition::%lu%%", prompt, (i*100) / vector_count(hs));
			fflush(stdout);
		}
		i=j;
	}
	if( hs[vector_count(hs)-1] != hs[vector_count(hs)-2]) ++dist[0];
	++linec;
	if( linec > 97 ) linec = 92;
	printf("\r\033[%dm", linec);
	printf("%s.result     :: t(%6.2f%%) m(%3lu) ",
			prompt, 
			(100.0*totalCollision)/vector_count(hs), 
			elementMaxCollision);

	for( size_t i = 0; i < 11; ++i){
		printf("[%ld]%6.3f%% ", i, (dist[i] * 100.0) / vector_count(hs));
	}
	printf(">%6.3f%% ", (dist[11] * 100.0) / vector_count(hs));

}

static unsigned cmpmod;
__private int hscmpm(const void* A, const void* B){
	uint32_t* a = (uint32_t*)A;
	uint32_t* b = (uint32_t*)B;
	return (*b%cmpmod) - (*a%cmpmod);
}

__private void hash_mod(uint32_t* hs, const char* prompt, unsigned mod){
	size_t totalCollision = 0;
	size_t elementCollision = 0;
	size_t elementMaxCollision = 0;
	size_t dist[12] = {0};

	cmpmod = mod;
	printf("\r%s.sorting::",prompt);
	fflush(stdout);
	vector_qsort(hs, hscmpm);
	printf("ok");
	fflush(stdout);

	size_t i = 0;
	while( i < vector_count(hs)-1 ){
		uint32_t hash = hs[i] % mod;
		elementCollision = 0;
		size_t j = i+1;
		while( j < vector_count(hs) && hash == hs[j] % mod ){
			++elementCollision;
			++j;
		}
		totalCollision += elementCollision;
		if( elementCollision ) ++totalCollision;
		if( elementCollision > 10 ){
			++dist[11];
		}
		else{
			++dist[elementCollision];
		}
		if( elementCollision > elementMaxCollision ) elementMaxCollision = elementCollision;
		if( !(i%100) ){
			printf("\r%s.mod::%lu%%", prompt, (i*100) / vector_count(hs));
			fflush(stdout);
		}
		i=j;
	}
	if( hs[vector_count(hs)-1] != hs[vector_count(hs)-2] ) ++dist[0];

	printf("\r\033[%dm", linec);
	printf("%s.result.%3.1fx:: t(%6.2f%%) m(%3lu) ",
			prompt, (double)mod/(double)vector_count(hs), 
			(100.0*totalCollision)/vector_count(hs), 
			elementMaxCollision);

	for( size_t i = 0; i < 11; ++i){
		printf("[%ld]%6.3f%% ", i, (dist[i] * 100.0) / vector_count(hs));
	}
	printf(">%6.3f%% ", (dist[11] * 100.0) / vector_count(hs));

	printf("\033[m\n");
}

uint32_t hash_vbx(const char* str, size_t len){
	size_t hash = 0;
	for( size_t i = 0; i < len; ++i){
		hash += (hash<<7) + (str[i] + len);
	}
	return hash;
}

/*@fn*/
void test_hash(const char* a, __unused const char*b){
	err_enable();

	struct map{
		char* prompt;
		rbhash_f fn;
	}mymap[] = {
		{ "oneattime ", hash_one_at_a_time},
		{ "fasthash  ", hash_fasthash},
		{ "kr        ", hash_kr},
		{ "sedgewick ", hash_sedgewicks},
		{ "sobel     ", hash_sobel},
		{ "weinberger", hash_weinberger},
		{ "elf       ", hash_elf},
		{ "sdbm      ", hash_sdbm},
		{ "bernstein ", hash_bernstein},
		{ "knuth     ", hash_knuth},
		{ "partow    ", hash_partow},
		{ "vbx       ", hash_vbx},
		{ NULL, NULL }
	};
	
	char** edata = data_word(a);
	if( !edata ){
		err_print();
		return;
	}

	for( size_t i = 0; mymap[i].prompt; ++i){
		size_t st = time_cpu_ms();
		__vector_free uint32_t* ehash = calcolate_hash(edata, mymap[i].fn, mymap[i].prompt);
		size_t en = time_cpu_ms();
		hash_distribuition(ehash, mymap[i].prompt);
		printf("time(%lu)", en-st);
		printf("\033[m\n");
		hash_mod(ehash, mymap[i].prompt, vector_count(edata));
		hash_mod(ehash, mymap[i].prompt, vector_count(edata)*2);
	}
	
	data_word_free(edata, a);

	err_restore();
}

