#include <ef/rbuffer.h>
#include <ef/memory.h>
#include <ef/mth.h>
#include <ef/err.h>

#define CB_W_NEXT(CB) ((CB)->w + 1)

rbuffer_s* rbuffer_new(size_t sof, size_t size){
	size = ROUND_UP_POW_TWO32(size);
	//rbuffer_s* cb = malloc(sizeof(rbuffer_s) + sof * size );
	rbuffer_s* cb = mem_flexible_structure_new(rbuffer_s, sof, size);
	if( cb == NULL ){
		err_pushno("malloc");
		return NULL;
	}
	cb->sof = sof;
	cb->size = size;
	cb->r = 0;
	cb->w = 0;
	return cb;
}

void rbuffer_free(rbuffer_s* cb){
	if( cb ) free(cb);
}

void rbuffer_free_auto(rbuffer_s** cb){
	rbuffer_free(*cb);
}

int rbuffer_isempty(rbuffer_s* cb){
	return cb->w == cb->r;
}

__private inline size_t rbuffer_w_next(rbuffer_s* cb){
	return FAST_MOD_POW_TWO((cb->w + 1), cb->size);
}

int rbuffer_isfull(rbuffer_s* cb){
	return rbuffer_w_next(cb) == cb->r;
}	

err_t rbuffer_write(rbuffer_s* cb, void* data){
	size_t next = rbuffer_w_next(cb);
	if( next == cb->r 	) return -1;
	memcpy( ADDR(cb->element) + (cb->w * cb->sof), data, cb->sof);
	cb->w = next;
	return 0;	
}

__private inline size_t rbuffer_r_next(rbuffer_s* cb){
	return FAST_MOD_POW_TWO((cb->r + 1), cb->size);
}

err_t rbuffer_read(rbuffer_s* cb, void* out){
	if( cb->r == cb->w ) return -1;
	memcpy( out, ADDR(cb->element) + (cb->r * cb->sof), cb->sof);
	cb->r = rbuffer_r_next(cb);
	return 0;
}

size_t rbuffer_available_write(rbuffer_s* cb){
	size_t size;
	if( cb->w > cb->r ){
		size = cb->size - cb->w;
		size += cb->r;
	}
	else if( cb->w < cb->r) {
		size = (cb->r - cb->w);
	}
	else{
		size = cb->size;
	}
	return size - 1;
}

size_t rbuffer_available_read(rbuffer_s* cb){
	return (cb->size-1) - rbuffer_available_write(cb);
}

size_t rbuffer_available_linear_write(rbuffer_s* cb){
	size_t size;
	if( cb->w >= cb->r ){
		size = (cb->size - cb->w) - 1;
	}
	else{
		size = (cb->r - cb->w) - 1;
	}
	//dbg_info("w(%lu) sz(%lu) av(%lu)", cb->w, cb->size, size);
	return size;
}

size_t rbuffer_available_linear_read(rbuffer_s* cb){
	size_t size;
	if( cb->r > cb->w ){
		size = cb->size - cb->r;
	}
	else{
		size = cb->w - cb->r;
	}
	return size;
}

void rbuffer_sync_write(rbuffer_s* cb, size_t n){
	cb->w = FAST_MOD_POW_TWO(cb->w+n, cb->size);
}

void rbuffer_sync_read(rbuffer_s* cb, size_t n){
	cb->r = FAST_MOD_POW_TWO(cb->r+n, cb->size);
}

__private inline size_t rbuffer_r_prev(rbuffer_s* cb){
	return FAST_MOD_POW_TWO((cb->r - 1), cb->size);
}

__private inline size_t rbuffer_w_prev(rbuffer_s* cb){
	return FAST_MOD_POW_TWO((cb->w - 1), cb->size);
}

err_t rbuffer_unread(rbuffer_s* cb){
	size_t prev = rbuffer_r_prev(cb);
	if( prev == cb->w ) return -1;
	cb->r = prev;
	return 0;
}

err_t rbuffer_unwrite(rbuffer_s* cb){
	size_t prev = rbuffer_w_prev(cb);
	if( prev == cb->r ) return -1;
	cb->w = prev;
	return 0;
}

void* rbuffer_addr_r(rbuffer_s* cb){
	return ADDR(cb->element) + cb->r * cb->sof;
}

void* rbuffer_addr_w(rbuffer_s* cb){
	return ADDR(cb->element) + cb->r * cb->sof;
}

void rbuffer_clear(rbuffer_s* cb){
	cb->w = cb->r;	
}
