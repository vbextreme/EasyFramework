#include <ef/memory.h>
#include <ef/err.h>
#include <ef/mth.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


void* mem_shared_create(const char* name, int priv, size_t size){
	int fm = shm_open( name, O_CREAT | O_EXCL | O_RDWR, priv);
	if( fm == -1 ){
		err_pushno("shm_open");
		return NULL;
	}
	if( ftruncate(fm, size) == -1 ){
		err_pushno("ftruncate");
		close(fm);
		return NULL;
	}
	
	void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fm, 0);
	close(fm);
	
	if( mem == (void*)-1 ){
		err_pushno("mmap");
		return NULL;
	}

	return mem;
}

void* mem_shared_map(const char* name){
	int fm = shm_open( name, O_RDWR, 0);
	if( fm == -1 ){
		err_pushno("shm_open");
		return NULL;
	}
	struct stat ss;
	if( fstat(fm, &ss) == -1 ){
		err_pushno("stat");
		close(fm);
		return NULL;
	}
	void* mem = mmap(NULL, ss.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fm, 0);
	close(fm);
	
	if( mem == (void*)-1 ){
		err_pushno("mmap");
		return NULL;
	}

	return mem;
}

void* mem_shared_create_or_map(const char* name, int priv, size_t size){
	err_disable();
	void* mem = mem_shared_create(name, priv, size);
	err_restore();
	if( mem ) return mem;
	return mem_shared_map(name);
}
	
size_t mem_shared_size(const char* name){
	int fm = shm_open( name, O_RDWR, 0);
	if( fm == -1 ){
		err_pushno("shm_open");
		return 0;
	}
		
	struct stat ss;
	if( fstat(fm, &ss) == -1 ){
		err_pushno("stat");
		close(fm);
		return 0;
	}
	close(fm);
	return ss.st_size;
}

void mem_shared_close(void* mem, size_t size){
	munmap(mem, size);
}

int mem_shared_delete(const char* name){
	if( shm_unlink(name) == -1 ){
		err_pushno("shm_unlink");
		return -1;
	}
	return 0;
}

void* mem_shared_alloc_ptr(void** mem, size_t size){
	void* ret = *mem;
	size = ROUND_UP(size, sizeof(void*));
	*mem += size;
	return ret;
}
