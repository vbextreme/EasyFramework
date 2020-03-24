#include "test.h"
#include <ef/fzs.h>

/*@test -F --fuzzy-search 'test fuzzy search, use arg-a for data, use arg-b for search string, default is vbextreme'*/

__private void fuzzyadd(fzsElement_s** v, const char* name){
	fzsElement_s tmp = {
		.str = name,
		.len = strlen(name),
		.distance = fzs_levenshtein(name, 0, name, 0)
	};
	vector_push_back(*v, tmp);
}

/*@fn*/
void test_fuzzysearch(const char* a, const char* b){
	err_enable();

	if( b == NULL ) b = "vbextreme";
	
	char** edata = data_word(a);
	if( !edata ){
		err_print();
		return;
	}

	__vector_free fzsElement_s* fzd = vector_new(fzsElement_s, 32, 32);
	vector_foreach(edata, i){
		fuzzyadd(&fzd, edata[i]);
	}

	fzs_qsort(fzd, b, 0);
	
	vector_foreach(fzd, i){
		printf("%s:%ld\n", fzd[i].str, fzd[i].distance);
	}

	data_word_free(edata, a);

	err_restore();
}

