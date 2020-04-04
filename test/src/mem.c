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

	{
		size_t a = ~(size_t)0;
		size_t b = 0;
		mem_swap(&a, sizeof a, &b, sizeof b);
		TESTT("mem_swap size_t", a == 0 && b == ~(size_t)0);
	}
	{
		int a = -7777;
		int b = 5555;
		mem_swap(&a, sizeof a, &b, sizeof b);
		TESTT("mem_swap int", a == 5555 && b == -7777);
	}
	{
		char va[128] = "hello";
		char vb[128] = "world";
		mem_swap(va, strlen(va) + 1, vb, strlen(vb) + 1);
		TESTT("mem_swap str ==", !strcmp(va, "world") && !strcmp(vb, "hello"));
	}
	{
		char va[128] = "va";
		char vb[128] = "le of vb";
		mem_swap(va, strlen(va) + 1, vb, strlen(vb) + 1);
		TESTT("mem_swap str <", !strcmp(va, "le of vb") && !strcmp(vb, "va"));
	}
	{
		char va[128] = "va gr of";
		char vb[128] = "vb";
		mem_swap(va, strlen(va) + 1, vb, strlen(vb) + 1);
		TESTT("mem_swap str >", !strcmp(va, "vb") && !strcmp(vb, "va gr of"));
	}
}

