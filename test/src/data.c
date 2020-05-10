#include "test.h"

__private char* DATA[] = {
	"vbextreme",
	"vbx",
	"miao",
	"ciao",
	"andrea",
	"hello world",
	"any word",
	"\x1B[1F",
	NULL
};

__private char** data_builtin(void){
	char** ret = vector_new(char*, 8, free);
	for( size_t i = 0; DATA[i]; ++i){
		vector_push_back(ret, DATA[i]);
	}
	return ret;
}

char** data_word(const char* pdata){
	if( pdata == NULL ){
		return data_builtin();
	}
	
	__mem_free char* path = path_resolve(pdata);
	__stream_close stream_s* dataf = stream_open(path, "r", 0, STREAM_CHUNK);

	printf("data word:%s\n",path);
   	if( !dataf ){
		err_print();
		exit(1);
	}
	
	char** ret = vector_new(char*, 256, free);

	char* line;
	size_t total = 0;
	while( stream_inp(dataf, &line, '\n', 0) > 0 && line != NULL ){
		//dbg_info("%s", line);
		vector_push_back(ret, line);
		++total;
		if( !(total % 100) ){
			printf("\rdata loading: %lu", total);
			fflush(stdout);
		}
	}
	printf("\rdata loading: %lu\n", total);

	return ret;
}

void data_word_free(char** v, const char* pdata){
	if( !pdata ) return;
	vector_free(v);
}








