#include <ef/memory.h>
#include <ef/err.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef MEM_UNDECLARE_PKEY
pkey_t mem_pkey_new(__ef_unused unsigned int mode){
	return 0;
}
#else
pkey_t mem_pkey_new(unsigned int mode){
	return pkey_alloc(0 ,mode);
}
#endif

err_t mem_protect(pkey_t* key, void* addr, size_t size, unsigned int mode){
	int prot = PROT_READ | PROT_WRITE;
#ifdef MEM_PKEY_DISABLE
	*key = -1;
	switch( mode ){
		case MEM_PROTECT_WRITE: prot = PROT_READ; break;
		case MEM_PROTECT_RW: prot = PROT_NONE; break;
	}	
#else
	#ifndef MEM_UNDECLARE_PKEY 
	if( (*key = pkey_alloc(0, mode)) < 0 ){
		err_pushno("pkey_alloc");
		return -1;
	}
	#endif
#endif

#ifdef MEM_UNDECLARE_PKEY
	if( mprotect(addr, size, prot) < 0 ){
		err_pushno("mprotect");
		return -1;
	}
#else
	if( pkey_mprotect(addr, size, prot, *key) < 0 ){
		err_pushno("pkey_mprotect");
		return -1;
	}
#endif

#ifdef MEM_PKEY_DISABLE
	*key = size;
#endif

	return 0;
}

err_t mem_protect_change(pkey_t key, unsigned int mode,
#ifdef EF_MEM_PKEY_DISABLE
 void* addr){
	switch( mode ){
		case MEM_PROTECT_DISABLED: mode = PROT_READ | PROT_WRITE; break;
		case MEM_PROTECT_WRITE: mode = PROT_READ; break;
		case MEM_PROTECT_RW: mode = PROT_NONE; break;
	}
	#ifdef MEM_UNDECLARE_PKEY
	if( mprotect(addr, key, mode) < 0){
		err_pushno("pkey_mprotect");
		return -1;
	}
	#else
	if( pkey_mprotect(addr, key, mode, -1) < 0 ){
		err_pushno("pkey_mprotect");
		return -1;
	}
	#endif
	return 0;
}
#else
 __unused void* addr){
	return pkey_set(key ,mode);
 }
#endif
