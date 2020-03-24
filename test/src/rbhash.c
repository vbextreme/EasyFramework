#include "test.h"
#include <ef/rbhash.h>

/*@test -R --rb-hash 'test rbhash, use arg-a for file data, without a use internal data, use arg-b for hash function, default vbx, all for test all'*/

uint32_t hash_vbx(const char* str, size_t len);

__private struct map{
		const char* name;
		rbhash_f fn;
}mymap[] = {
	{ "oneattime", hash_one_at_a_time},
	{ "fasthash", hash_fasthash},
	{ "kr", hash_kr},
	{ "sedgewick", hash_sedgewicks},
	{ "sobel", hash_sobel},
	{ "weinberger", hash_weinberger},
	{ "elf", hash_elf},
	{ "sdbm", hash_sdbm},
	{ "bernstein", hash_bernstein},
	{ "knuth", hash_knuth},
	{ "partow", hash_partow},
	{ "vbx", hash_vbx},
	{ NULL, NULL }
};

__private rbhash_f hs_ntof(const char* name){
	for( size_t i = 0; mymap[i].name; ++i){
		if( !strcmp(name, mymap[i].name) ) return mymap[i].fn;
	}
	err_fail("no hash function with name %s", name);
	return NULL;
}

__private void rbhashtable(char** edata, const char* fname){
	int onlyforcheck = 1;
	
	printf("test hash:%s\n", fname);

	err_disable();

	size_t ts = time_cpu_us();
	__rbhash_free rbhash_s* rbh = rbhash_new(1000000, 30, 128, hs_ntof(fname), NULL);
	size_t te = time_cpu_us();
	printf("init time: %fs %lums %luus\n", (te-ts)/1000000.0, (te-ts)/1000, te-ts);  

	ts = time_cpu_us();
	vector_foreach(edata, i){
		char* k = edata[i];
		if( rbhash_add_unique(rbh, k, 0, &onlyforcheck) ){
			rbhashElement_s* el = rbhash_find_hash_raw(rbh, rbh->hashing(k, strlen(k)), k, strlen(k));
			err_fail("(%lu)'%s' is not unique key, find %s with %u hash", i, k, el->key, el->hash);
		}
		if( !(i%100) ){
			printf("\radd::%lu%%", (i*100) / vector_count(edata));
			fflush(stdout);
		}
	}
	te = time_cpu_us();
	printf("\radd time: %fs %lums %luus\n", (te-ts)/1000000.0, (te-ts)/1000, te-ts);  

	ts = time_cpu_us();
	vector_foreach(edata, i){
		char* k = edata[i];
		if( !rbhash_find(rbh, k, 0) ){
			unsigned h = rbh->hashing(k, strlen(k));
			//unsigned b = FAST_MOD_POW_TWO(h, rbh.size);
			//unsigned s = b > 4 ? b - 5 : 0;
			//unsigned e = b + rbh.maxdistance < rbh.size ? b+rbh.maxdistance+5:b+1;
			puts("");
			//dbg_error("ops");
			//rbhash_dbg_print(&rbh, s, e);
			err_fail("'%s'(%lu|%u) is not exists", k, FAST_MOD_POW_TWO(h,rbh->size), h);
		}
		if( !(i%100) ){
			printf("\rsearch::%lu%%", (i*100) / vector_count(edata));
			fflush(stdout);
		}
	}
	te = time_cpu_us();
	printf("\rsearch time: %fs %lums %luus\n", (te-ts)/1000000.0, (te-ts)/1000, te-ts);  

	printf("total ram usage:%lub %fMiB\n", rbhash_mem_total(rbh), rbhash_mem_total(rbh)/(1024.0*1024.0));
	printf("total bucket   :%lu\n", rbhash_bucket_used(rbh));
	printf("total collision:%lu\n", rbhash_collision(rbh));
	printf("max distance   :%lu\n", rbhash_distance_max(rbh));

	err_restore();
}


/*@fn*/
void test_hashtable(const char* a, const char* b){
	err_enable();
	
	if( b == NULL ) b = "vbx";

	char** edata = data_word(a);
	if( !edata ){
		err_print();
		return;
	}

	if( !str_equal(b,strlen(b),"all",strlen("all")) ){
		for( size_t i = 0; mymap[i].name; ++i){
			rbhashtable(edata, mymap[i].name);
		}
	}
	else{
		rbhashtable(edata, b);
	}

	data_word_free(edata, a);

	err_restore();
}

