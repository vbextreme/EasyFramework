#include <ef/memory.h>
#include <ef/err.h>
#include <ef/mth.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

void* malloc_or_die(size_t sz){
	void* mem = malloc(sz);
	if( mem == NULL ){
		err_pushno("malloc");
		err_fail("malloc");
	}
	return mem;
}

void mem_matrix_free(void* mem, size_t y){
	void** b = (void**)mem;
    size_t i;
    for (i=0; i < y; ++i)
        free(b[i]);

    free(b);
}

void* mem_matrix_new(size_t y, size_t sz){
    void **b;
    b = mem_many(void*, y);
	if( !b ){
		err_pushno("malloc");	
		return NULL;
	}

    for(size_t i = 0; i < y; ++i){
        b[i] = (void*)malloc(sz);
        if( !b[i] ){
            for (size_t j = 0; j < i; ++j)
                mem_free(b[j]);
            mem_free(b);
            return NULL;
        }
    }
    return b;
}

void mem_free_auto(void* mem){
	void** mug = (void**)mem;
	free(*mug);
	*mug = NULL;
}

void* mem_many_aligned_raw(size_t* size, size_t aligneto){
	*size = ROUND_UP(*size, aligneto);
	return aligned_alloc(aligneto, *size);
}

err_t mem_resize_raw(void** mem, const size_t size){
	void* raw = realloc(*mem, size);
	if( raw == NULL ){
		err_pushno("realloc");
		return -1;
	}
	*mem = raw;
	return 0;
}
