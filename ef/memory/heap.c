#include <ef/memory.h>
#include <ef/err.h>
#include <ef/mth.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

void* mem_heap_alloc(size_t* size) {
	iassert(size);
	iassert(*size > 0);
	*size = ROUND_UP(*size, getpagesize());
	void* heap = mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if( heap == (void*)-1 ){
		err_pushno("mmap");
		return NULL;
	}
	return heap;
}

void mem_heap_close(void* mem, size_t size){
	if( mem == NULL ){
		dbg_warning("shmem already closed");
		return;
	}

	if( munmap(mem, size) < 0 ){
		err_pushno("munmap");
	}
}
