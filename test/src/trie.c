#include "test.h"
#include <ef/trie.h>

/*@test -T --trie 'test trie, use arg-a for file data, without use internal data set'*/

/*@fn*/
void test_trie(const char* a, __unused const char* b){
	int engage = 1;
	int ret = 1;

	char** edata = data_word(a);
	if( !edata ){
		err_print();
		return;
	}

	__trie_free trie_s* tr = trie_new(NULL);
	
	vector_foreach(&edata, i){
		if( trie_insert(tr, edata[i], &engage) ){
			ret = 0;
			break;
		}
		if( !(i%100) ){
			printf("\radd::%lu%%", (i*100) / vector_count(edata));
			fflush(stdout);
		}
	}
	printf("\r");
	TESTT("trie insert", ret == 1);
	if( !ret ){
		data_word_free(edata, a);
		return;
	}
	ret = 1;

	vector_foreach(&edata, i){
		if( !trie_search(tr, edata[i]) ){
			err_push("fail to search %s in trie", edata[i] );
			ret = 0;
			break;
		}
		if( !(i%100) ){
			printf("\rsearch::%lu%%", (i*100) / vector_count(edata));
			fflush(stdout);
		}
	}
	printf("\r");
	TESTT("trie search", ret == 1);

	data_word_free(edata, a);
}

