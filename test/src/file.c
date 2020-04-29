#include "test.h"
#include <ef/file.h>
#include <ef/config.h>
#include <ef/trie.h>

/*@test -f --file 'test file use argva for get config file, default ../config.test' */

int printconf(int type, char** value, __unused void* userdata){
	printf("config(%d):%s\n", type, value ? *value : "null");
	return 0;
}

configTrie_s* conf_new(int type){
	configTrie_s* ct = mem_new(configTrie_s);
	iassert(ct);
	ct->fn = printconf;
	ct->type = type;
	ct->userdata = NULL;
	return ct;
}

/*@fn*/
void test_file(const char* argA, __unused const char* argB){
	const char* confname[]={
		"POWER_SUPPLY_VOLTAGE_MIN_DESIGN",
		"POWER_SUPPLY_VOLTAGE_NOW",
		"POWER_SUPPLY_ENERGY_FULL",
		"POWER_SUPPLY_ENERGY_NOW",
		"POWER_SUPPLY_POWER_NOW",
		"POWER_SUPPLY_CAPACITY",
		"POWER_SUPPLY_STATUS",
		NULL
	};

	if( argA == NULL ) argA = "../config.test";
	__mem_free char* path = path_resolve(argA);	

	__trie_free trie_s* tr = trie_new(NULL);
	if( !tr ){
		err_print();
		return;
	}
	for( size_t i = 0; confname[i]; ++i){
		TESTF("insert config in trie", trie_insert(tr, confname[i], conf_new(i)) );
	}

	__stream_close stream_s* sm = stream_open(path, "r", 0, 4096);
   	if( !sm ){
		err_fail("stream open");
	}
	
	config_parse(tr, sm);

	const char* testpath[] = {
		"./",
		"./home",
		"../",
		"../yessa",
		"../../",
		"../../hello",
		"~/",
		"~/.config"
		"/home",
		"home/hello",
		"home/./help",
		"home/../ok",
		"home/ji/../run",
		NULL
	};

	for( size_t i = 0; testpath[i]; ++i){
		__mem_free char* pr = path_resolve(testpath[i]);
		printf("'%s' -> '%s'\n", testpath[i], pr);
	}
}
