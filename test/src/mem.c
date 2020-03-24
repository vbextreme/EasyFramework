#include "test.h"

/*@test -m --mem 'test memory'*/

/*@fn*/
void test_mem(__unused const char* argA, __unused const char* argB){
	/*memory.c*/
	int* mem;
	TESTT("mem_new", mem = mem_new(int));
	mem_free(mem);
	TESTT("mem_many", mem = mem_many(int, 100));
	memset(mem, 1,sizeof(int)*100);
	mem_clear_many(int, mem, 100);
	for(size_t i = 0; i < 100; ++i ) 
		mem[0] += mem[i];	
	TESTF("mem_clear_many", mem[0]);
	mem_free_safe(mem);
	TESTF("mem_free_safe", mem);
	/*shared.c*/
	void* ptr;
	TESTT("mem_shaerd_create/open", ptr = mem_shared_create_or_map("/name", 0666, 4096));
	mem_shared_close("/name",4096);
	TESTF("mem_shared_delete", mem_shared_delete("/name"));
	/*heap.c*/
	size_t size=4096;
	TESTT("mem_heap_alloc", mem=mem_heap_alloc(&size));
	TESTT("mem_heap_alloc size", size);
	mem_heap_close(mem, size);
}

