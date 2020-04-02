/* fork of*/
// MIT licensed.
// Copyright (c) 2015 Titus Wormer <tituswormer@gmail.com>

#include <ef/fzs.h>
#include <ef/memory.h>

// Returns a size_t, depicting the difference between `a` and `b`.
// See <https://en.wikipedia.org/wiki/Levenshtein_distance> for more information.
size_t fzs_levenshtein(const char *a, size_t lena, const char *b, size_t lenb){
	if( lena == 0 ) lena = strlen(a);
	if( lenb == 0 ) lenb = strlen(b);
	if( a == b ) return 0;
	if( lena == 0 ) return lenb;
	if (lenb == 0 ) return lena;
	
	__mem_free size_t* cache = mem_many(size_t, lena);
	size_t index = 0;
	size_t bIndex = 0;
	size_t distance;
	size_t bDistance;
	size_t result;
	char code;

	while( index < lena ){
	    cache[index] = index + 1;
		index++;
	}

	while( bIndex < lenb ){
		code = b[bIndex];
		result = distance = bIndex++;
		index = SIZE_MAX;
	    while( ++index < lena ){
			bDistance = code == a[index] ? distance : distance + 1;
			distance = cache[index];

			cache[index] = result = distance > result
				? bDistance > result
					? result + 1
					: bDistance
				: bDistance > distance
					? distance + 1
					: bDistance;
		}
	}
	
	scan_build_unknown_cleanup(cache);
	return result;
}

ssize_t fzs_vector_find(char** v, const char* str, size_t lens){
	if( lens == 0 ) lens = strlen(str);
	size_t min = -1UL;
	ssize_t index = -1;
	vector_foreach(v, i){
		size_t distance = fzs_levenshtein(v[i], strlen(v[i]), str, lens);
		if( distance < min ){
		   	min =  distance;
			index = i;
		}
	}
	return index;
}

__private int fzs_cmp(const void* A, const void* B){
	const fzsElement_s* a = (const fzsElement_s*)A;
	const fzsElement_s* b = (const fzsElement_s*)B;
	return a->distance - b->distance;
}

void fzs_qsort(fzsElement_s* fzse, const char* str, size_t lens){
	iassert(fzse);
	iassert(str);

	if( lens == 0 ) lens = strlen(str);
	vector_foreach(fzse, i){
		fzse[i].distance = fzs_levenshtein(fzse[i].str, fzse[i].len, str, lens);
	}
	vector_qsort(fzse, fzs_cmp);
}




