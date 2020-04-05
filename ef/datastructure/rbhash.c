#include <ef/type.h>
#include <ef/memory.h>
#include <ef/rbhash.h>
#include <ef/mth.h>
#include <ef/str.h>
#include <ef/err.h>

#define rbhash_slot(HASH,SIZE) FAST_MOD_POW_TWO(HASH, SIZE)
#define rbhash_slot_next(SLOT, SIZE) (rbhash_slot((SLOT)+1,SIZE))
#define rbhash_element_slot(TABLE, ESZ, SLOT) ((rbhashElement_s*)(ADDR(TABLE) + ((ESZ) * (SLOT))))
#define rbhash_element_next(TABLE, ESZ) ((rbhashElement_s*)(ADDR(TABLE) + (ESZ)))

rbhash_s* rbhash_new(size_t size, size_t min, size_t keysize, rbhash_f hashing, rbhashfree_f del){
	rbhash_s* rbh = mem_new(rbhash_s);
	rbh->size = ROUND_UP_POW_TWO32(size);
	rbh->min = min;
	rbh->hashing = hashing;
	rbh->del = del;
	rbh->count = 0;
	rbh->maxdistance = 0;
	rbh->keySize = keysize;
	rbh->elementSize = ROUND_UP(sizeof(rbhashElement_s), sizeof(void*));
	rbh->elementSize = ROUND_UP(rbh->elementSize + keysize, sizeof(void*)); 
	iassert((rbh->elementSize % sizeof(void*)) == 0);
	rbh->table = malloc( rbh->elementSize * rbh->size );
	if( rbh->table == NULL ){
		err_pushno("malloc");
		free(rbh);
		return NULL;
	}

	rbhashElement_s* table = rbh->table;
	for( size_t i = 0; i < rbh->size; ++i, table = rbhash_element_next(table,rbh->elementSize)){
		table->len = 0;
	}

	return rbh;
}

void rbhash_free(rbhash_s* rbh){
	rbhashElement_s* table = rbh->table;
	for( size_t i = 0; i < rbh->size; ++i, table = rbhash_element_next(table,rbh->elementSize) ){
		if( table->len == 0 ) continue;
		if( rbh->del && table->data){
			rbh->del(table->hash, table->key, table->data);
		}
		--rbh->count;
	}
	free(rbh->table);
	iassert(rbh->count == 0);
	free(rbh);
}

void rbhash_free_auto(rbhash_s** rbh){
	if( *rbh ){
		rbhash_free(*rbh);
	}
}

__private void rbhash_swapdown(rbhashElement_s* table, const size_t size, const size_t esize, size_t* maxdistance, rbhashElement_s* nw){
	uint32_t bucket = rbhash_slot(nw->hash, size);

	rbhashElement_s* tbl = rbhash_element_slot(table, esize, bucket);
	size_t i = 0;
	while( i < size && tbl->len != 0 ){
		if(  nw->distance > tbl->distance ){
			SWAP(tbl->data, nw->data);
			SWAP(tbl->distance, nw->distance);
			SWAP(tbl->hash, nw->hash);
			SWAP(tbl->len, nw->len);
			mem_swap(tbl->key, tbl->len, nw->key, nw->len);
		}
		++nw->distance;
		bucket = rbhash_slot_next(bucket, size);
		tbl = rbhash_element_slot(table, esize, bucket);
		++i;
		if( nw->distance > *maxdistance ) *maxdistance = nw->distance;
	}
	if( tbl->len != 0 ){
		err_fail("hash lose element, the hash table is to small");
	}
	memcpy(tbl,nw,esize);
}

__private err_t rbhash_upsize(rbhash_s* rbh){
	const size_t p = ((rbh->size * rbh->min) / 100) + 1;
	const size_t pm = rbh->count + p;
	if( rbh->size > pm ) return 0;
	
	const size_t newsize = ROUND_UP_POW_TWO32(pm);
	
	rbhashElement_s* newtable =  malloc(rbh->elementSize * newsize);
	if( newtable == NULL ){
		err_pushno("malloc");
		return -1;
	}
	rbhashElement_s* table = newtable;
	for( size_t i = 0; i < rbh->size; ++i, table = rbhash_element_next(table,rbh->elementSize)){
		table->len = 0;
	}

	rbh->maxdistance = 0;
	table = rbh->table;
	for(size_t i = 0; i < rbh->size; ++i, table = rbhash_element_next(table,rbh->elementSize)){
		if( table->len == 0 ) continue;
		table->distance = 0;
		rbhash_swapdown(newtable, newsize, rbh->elementSize, &rbh->maxdistance, table);
	}
	free(rbh->table);
	rbh->table = newtable;
	rbh->size = newsize;
	return 0;
}

err_t rbhash_add_hash(rbhash_s* rbh, uint32_t hash, void* key, size_t len, void* data){
	if( len == 0 ) len = strlen(key);
	if( len > rbh->keySize ){
		err_push("key to long");
		errno = EFBIG;
		return -1;
	}
	
	__mem_free rbhashElement_s* el = malloc(rbh->elementSize);
	el->data = data;
	el->distance = 0;
	el->hash = hash;
	el->len = len;
	memcpy(el->key, key, len);
	rbhash_swapdown(rbh->table, rbh->size, rbh->elementSize, &rbh->maxdistance, el);
	++rbh->count;
	return rbh->min ? rbhash_upsize(rbh) : 0;
}

err_t rbhash_add(rbhash_s* rbh, void* key, size_t len, void* data){
	if( len == 0 ) len = strlen(key);
	return rbhash_add_hash(rbh, rbh->hashing(key, len), key, len, data);
}

err_t rbhash_add_unique(rbhash_s* rbh, void* key, size_t len, void* data){
	err_disable();
	if( rbhash_find(rbh, key, len) ){
		err_restore();
		return -1;
	}
	err_restore();
	return rbhash_add(rbh, key, len, data);
}

__private long rbhash_find_bucket(rbhash_s* rbh, uint32_t hash, void* key, size_t len){
	uint32_t slot = rbhash_slot(hash, rbh->size);
	rbhashElement_s* table = rbhash_element_slot(rbh->table, rbh->elementSize, slot);

	size_t maxscan = rbh->maxdistance + 1;
	while( maxscan-->0 ){
		if( table->len == len && table->hash == hash && !memcmp(key, table->key, len) ){
			return slot;
		}
		slot = rbhash_slot_next(slot, rbh->size);
		table = rbhash_element_slot(rbh->table, rbh->elementSize, slot);
	}
	err_push("not find key");
	errno = ESRCH;
	return -1;
}

rbhashElement_s* rbhash_find_hash_raw(rbhash_s* rbh, uint32_t hash, void* key, size_t len){
	if( len == 0 ) len=strlen(key);
	long bucket;
	if( (bucket = rbhash_find_bucket(rbh, hash, key, len)) == -1 ) return NULL;
	return rbhash_element_slot(rbh->table, rbh->elementSize, bucket);
}

void* rbhash_find_hash(rbhash_s* rbh, uint32_t hash, void* key, size_t len){
	if( len == 0 ) len = strlen(key);
	rbhashElement_s* el = rbhash_find_hash_raw(rbh, hash, key, len);
	return el ? el->data : NULL;
}

void* rbhash_find(rbhash_s* rbh, void* key, size_t len){
	if( len == 0 ) len = strlen(key);
	uint32_t hash = rbh->hashing(key, len);
	return rbhash_find_hash(rbh, hash, key, len);
}

__private void rbhash_swapup(rbhashElement_s* table, size_t size, size_t esize, size_t bucket){
	size_t bucketfit = bucket;
	bucket = rbhash_slot_next(bucket, size);
	rbhashElement_s* tbl = rbhash_element_slot(table, esize, bucket);

	while( tbl->len != 0 && tbl->distance ){
		rbhashElement_s* tblfit = rbhash_element_slot(table, esize, bucketfit);
		memcpy(tblfit, tbl, esize);
		tbl->len = 0;
		tbl->data = NULL;
		bucketfit = bucket;
		bucket = rbhash_slot_next(bucket, size);
		table = rbhash_element_slot(table, esize, bucket);
	}
}

err_t rbhash_remove_hash(rbhash_s* rbh, uint32_t hash, void* key, size_t len){
	long bucket = rbhash_find_bucket(rbh, hash, key, len);
	if( bucket == -1 ) return -1;
	rbhashElement_s* el = rbhash_element_slot(rbh->table, rbh->elementSize, bucket);

	if( rbh->del && el->data){
		rbh->del(el->hash, el->key, el->data);
	}
	
	el->len = 0;
	el->data = 0;
	rbhash_swapup(rbh->table, rbh->size, rbh->elementSize, bucket);
	--rbh->count;
	return 0;	
}

err_t rbhash_remove(rbhash_s* ht, void* key, size_t len){
	return rbhash_remove_hash(ht, ht->hashing(key, len), key, len);
}

size_t rbhash_mem_total(rbhash_s* rbh){
	size_t ram = sizeof(rbhash_s);
	ram += rbh->elementSize * rbh->size;
	return ram;
}

size_t rbhash_bucket_used(rbhash_s* rbh){
	return rbh->count;
}

size_t rbhash_collision(rbhash_s* rbh){
	rbhashElement_s* tbl = rbh->table;
	size_t collision = 0;
	for(size_t i =0; i < rbh->size; ++i, tbl = rbhash_element_next(tbl,rbh->elementSize)){
		if( tbl->len != 0 && tbl->distance != 0 ) ++collision;
	}
	return collision;
}

size_t rbhash_distance_max(rbhash_s* rbh){
	return rbh->maxdistance;
}

#if DEBUG_ENABLE > 0
void rbhash_dbg_print(rbhash_s* rbh, size_t begin, size_t end){
	dbg_info("max.d:%lu",rbh->maxdistance);
	dbg_info("count:%lu",rbh->count);
	dbg_info("size :%lu",rbh->size);
	dbg_info("esize:%lu",rbh->elementSize);
	dbg_info("ksize:%lu",rbh->keySize);
	dbg_info("slot ] distance|              string             |     hash    | hash %% size |");

	rbhashElement_s* table;
	for( size_t i = begin; i < end; ++i ){
		table = rbhash_element_slot(rbh->table,rbh->elementSize,i);
		dbg_info("%7lu] %8u| %32.*s| %12u| %12lu|", i, table->distance, table->len, table->key, table->hash, FAST_MOD_POW_TWO(table->hash, rbh->size));
	}	

}
#endif
