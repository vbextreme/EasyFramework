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

void mem_swap(void* restrict a, size_t sizeA, void* restrict b, size_t sizeB){
	if( sizeA >= sizeof(size_t) && sizeB >= sizeof(size_t) ){
		const size_t la = sizeA - (sizeA % sizeof(size_t));
		const size_t lb = sizeB - (sizeB % sizeof(size_t));
		const size_t bcount = MTH_MIN(la, lb);
		const size_t count = bcount / sizeof(size_t);
		size_t* memA = a;
		size_t* memB = b;

		for( size_t i = 0; i < count; ++i ){
			SWAP(memA[i], memB[i]);
		}

		sizeA -= bcount;
		sizeB -= bcount;
		a = &memA[count];
		b = &memB[count];
	}
	
	if( sizeA >= sizeof(unsigned) && sizeB >= sizeof(unsigned) ){
		const size_t la = sizeA - (sizeA % sizeof(unsigned));
		const size_t lb = sizeB - (sizeB % sizeof(unsigned));
		const size_t bcount = MTH_MIN(la, lb);
		const size_t count = bcount / sizeof(unsigned);
		unsigned* memA = a;
		unsigned* memB = b;

		for( size_t i = 0; i < count; ++i ){
			SWAP(memA[i], memB[i]);
		}

		sizeA -= bcount;
		sizeB -= bcount;
		a = &memA[count];
		b = &memB[count];
	}

	if( sizeA >= sizeof(unsigned short) && sizeB >= sizeof(unsigned short) ){
		const size_t la = sizeA - (sizeA % sizeof(unsigned short));
		const size_t lb = sizeB - (sizeB % sizeof(unsigned short));
		const size_t bcount = MTH_MIN(la, lb);
		const size_t count = bcount / sizeof(unsigned short);
		unsigned* memA = a;
		unsigned* memB = b;

		for( size_t i = 0; i < count; ++i ){
			SWAP(memA[i], memB[i]);
		}

		sizeA -= bcount;
		sizeB -= bcount;
		a = &memA[count];
		b = &memB[count];
	}

	{
		const size_t count = MTH_MIN(sizeA, sizeB);
		char* memA = a;
		char* memB = b;
	
		for(size_t i = 0; i < count; ++i){
			SWAP(memA[i], memB[i]);
		}
		a = &memA[count];
		b = &memB[count];
		sizeA -= count;
		sizeB -= count;
	}

	iassert( sizeA == 0 || sizeB == 0);

	if( sizeA == 0 && sizeB == 0 ) return;

	if( sizeA == 0 ){
		memcpy(a, b, sizeB);	
	}
	else if( sizeB == 0 ){
		memcpy(b, a, sizeA);
	}
}
