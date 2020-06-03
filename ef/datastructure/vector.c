#include <ef/vector.h>
#include <ef/memory.h>
#include <ef/mth.h>
#include <ef/err.h>

void* vector_new_raw(size_t sof, size_t size, vfree_f fn){
	vector_s* v = mem_flexible_structure_new(vector_s, sof, size);
	if( v == NULL ) err_fail("eom");
	v->count = 0;
	v->sof = sof;
	v->size = size;
	v->fn = fn;
	return v->element;	
}

void vector_clear(void* m){
	if( !m ) return;
	vector_s* v = VECTOR(m);
	if( v->fn ){
		for( size_t i = 0; i < v->count; ++i){
			void** src = (void**)(ADDRTO(m, v->sof, i));
			v->fn(*src);
		}
	}
	v->count = 0;
}

void vector_free(void* m){
	if( !m ) return;
	vector_clear(m);
	vector_s* v = VECTOR(m);
	free(v);
}

void vector_free_auto(void* v){
	vector_free(*((void**)v));
}

void* vector_resize(void* mem, size_t size){
	vector_s* v = VECTOR(mem);
	
	if( v->size == size ) return mem;

#ifdef __clang__
	/** scanbuild think the realloc use sizeof on void*, this disable error on scanbuild*/
	dbg_fail("do not run this");
	v = realloc(v, ROUND_UP(size * v->sof + sizeof(vector_s), 8));
#else
	v = realloc(v, ROUND_UP(size * v->sof + sizeof(vector_s), sizeof(void*)));
#endif
	if( v == NULL ) err_fail("realloc");

	v->size = size;

	return v->element;
}

void vector_upsize(void* ptrmem, size_t count){
	void** m = ptrmem;
	vector_s* v = VECTOR(*m);

	if( count + v->count > v->size ){
		size_t nwsz = v->size * 2;
		size_t rsz = ROUND_UP(v->size + count, nwsz);
		void* e = vector_resize(*m, rsz);
		*m = e;
	}
}

void vector_downsize(void* ptrmem){
	void** m = ptrmem;
	vector_s* v = VECTOR(*m);

	if( v->count  / 4 > v->size ){
		size_t rsz = v->size / 2;
		void* e = vector_resize(*m, rsz);
		*m = e;
	}
}

void vector_remove_raw(void** ptrmem, const size_t index){
	void* m = *(void**)ptrmem;
	vector_s* v = VECTOR(m);

	if( v->count == 0 || index >= v->count){
		dbg_warning("index out of bound");	
		return;
	}

	if( index < v->count - 1 ){
		void* dst = ADDR(m) + index * v->sof;
		const void* src = ADDR(m) + ((index+1) * v->sof);
		const size_t mem = (v->count - index) * v->sof;
		memmove(dst, src, mem);
	}
	--v->count;

	vector_downsize(ptrmem);
}

err_t vector_add_raw(void* ptrmem, const size_t index){
	void** m = ptrmem;
	vector_s* v = VECTOR(*m);

	if( v->count == 0 || index >= v->count){
		dbg_warning("index out of bound");	
		return -1;
	}
	
	vector_upsize(m, 1);
	v = VECTOR(*m);

	void* dst = ADDR(*m) + ((index + 1) * v->sof);
	const void* src = ADDR(*m) + (index * v->sof);
	const size_t mem = (v->count - index) * v->sof;
	memmove(dst, src, mem);
	++v->count;
	return 0;
}

err_t vector_fitting(void* ptrmem){
	void** m = ptrmem;
	void* nm = vector_resize(*m, vector_count(*m));
	if( !nm ) return -1;
	*m = nm;
	return 0;
}

void vector_shuffle(void* m, size_t begin, size_t end){
	vector_s* v = VECTOR(m);

	const size_t count = (end - begin) + 1;
	for( size_t i = begin; i <= end; ++i ){
		size_t j = begin + mth_random(count);
		if( j != i ){
			mem_swap(ADDR(m) + (i * v->sof) , v->sof, ADDR(m) + (j * v->sof), v->sof);
		}
	}
}


