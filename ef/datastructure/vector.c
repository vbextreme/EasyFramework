#include <ef/vector.h>
#include <ef/memory.h>
#include <ef/mth.h>
#include <ef/err.h>

void* vector_new_raw(size_t sof, size_t size, size_t minimal){
	//vector_s* v = malloc(sizeof(vector_s) + sof * size);
	vector_s* v = mem_flexible_structure_new(vector_s, sof, size);
	if( v == NULL ){
		err_pushno("malloc");
		return NULL;
	}
	v->count = 0;
	v->minimal = minimal;
	v->sof = sof;
	v->size = size;
	return v->element;	
}

void vector_free(void* v){
	if( v ) free(VECTOR(v));
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
	if( v == NULL ){
		err_pushno("realloc");
		return NULL;
	}
	v->size = size;

	return v->element;
}

err_t vector_upsize(void* ptrmem, size_t count){
	void** m = ptrmem;
	vector_s* v = VECTOR(*m);

	if( count + v->count > v->size ){
		size_t rsz = v->size + ROUND_UP((count + v->count) - v->size, v->minimal);
		void* e = vector_resize(*m, rsz);
		if( e == NULL ) return -1;
		*m = e;
	}
	return 0;	
}

void vector_remove(void* m, const size_t index){
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
}

err_t vector_add_raw(void* ptrmem, const size_t index){
	void** m = ptrmem;

	vector_s* v = VECTOR(*m);

	if( v->count == 0 || index >= v->count){
		dbg_warning("index out of bound");	
		return -1;
	}
	
	if( v->count + 1 > v->size ){
		if( vector_upsize(m, 1) ) return -1;
		v = VECTOR(*m);
	}

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

