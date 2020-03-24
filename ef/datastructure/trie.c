#include <ef/trie.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/err.h>

trie_s* trie_new(trieFree_f freefnc){
	trie_s* tr = mem_new(trie_s);
	if( tr == NULL ){
		err_pushno("malloc");
		return NULL;
	}
	tr->root.charset = NULL;
	tr->root.endnode = NULL;
	tr->root.next = NULL;
	tr->free = freefnc;
	return tr;
}

__private void trie_rec_free(trieFree_f fnc, trieElement_s* el){
	if( el->charset == NULL ) return;
	for( size_t i = 0; el->charset[i] ; ++i){
		if( fnc && el->endnode[i] ) fnc(el->endnode[i]);
		trie_rec_free(fnc, &el->next[i]);
	}
	free(el->charset);
	free(el->endnode);
	free(el->next);
}

void trie_free(trie_s* tr){
	if( tr ){
		trie_rec_free(tr->free, &tr->root);
		free(tr);
	}
}

void trie_free_auto(trie_s** tr){
	trie_free(*tr);
}

__private trieElement_s* trie_add_charset(trieElement_s* node, char ch, void* endData){
	size_t size = 0;
	if( node->charset ){
		size = strlen(node->charset);
	}
	if( mem_resize(node->charset, char, size + 2) ) return NULL;
	node->charset[size    ] = ch;
	node->charset[size + 1] = 0;

	if( mem_resize(node->endnode, void*, size + 1) ){
		node->charset[size] = 0;
		return NULL;
	}
	node->endnode[size] = endData;

	if( mem_resize(node->next, trieElement_s, size + 1) ){
		node->charset[size] = 0;
		node->endnode[size] = 0;
		return NULL;
	}
	node->next[size].charset = NULL;
	node->next[size].endnode = NULL;
	node->next[size].next    = NULL;

	return &node->next[size];
}

__private inline ssize_t trie_search_charset(char* cset, char ch){
	if( cset == NULL ) return -1;
	char* set = strchr(cset, ch);
	if( set ) return set - cset;
	return -1;
}

trieElement_s* trie_add(trieElement_s* node, char ch, void* data){
	ssize_t chs = trie_search_charset(node->charset, ch);
	if( chs < 0 ){
		//dbg_info("charset.add::%d(%c)",ch,ch);
		return trie_add_charset(node, ch, data);
	}
	if( data ){
		if( node->endnode[chs] ){
			//dbg_error("charset.collision::%d(%c)",ch,ch);
			err_push("node collision ch::%d(%c)", ch, ch);
			return NULL;
		}
		//dbg_info("charset.endnode::%d(%c)",ch,ch);
		node->endnode[chs] = data;
	}
	return &node->next[chs];
}

err_t trie_insert(trie_s* trie, const char* str, void* data){
	//dbg_info("add %s", str);
	const char* begin = str;
	trieElement_s* el = &trie->root;
	while( *str ){
		if( (el = trie_add(el, *str, NULL)) == NULL ){
			err_fail("wtf, %s in in trie", begin); 
			return -1;
		}
		++str;
	}
	if( trie_add(el, 1, data) == NULL ){
		//dbg_error("insert.last");
		err_push("on add %s in trie", begin); 
		return -1;
	}
	return 0;
}

trieStep_e trie_step(void** out, trieElement_s** el, char ch){
	int chs = trie_search_charset((*el)->charset, ch);
	if( chs < 0 ) return TRIE_STEP_ERROR;
	trieStep_e ret = (*el)->endnode[chs] ? TRIE_STEP_END_NODE: TRIE_STEP_NEXT;
	*out = (*el)->endnode[chs];
	*el = &(*el)->next[chs];
	return ret;
}

void* trie_search(trie_s* trie, const char* str){
	trieElement_s* el = &trie->root;
	trieStep_e step = TRIE_STEP_ERROR;
	void* ret = NULL;
	while( *str && (step=trie_step(&ret, &el, *str)) == TRIE_STEP_NEXT ){
		++str;
	}
	if( step == TRIE_STEP_ERROR ) return NULL;
	switch( trie_step(&ret, &el, 1) ){
		case TRIE_STEP_ERROR: return NULL;
		case TRIE_STEP_NEXT: return NULL;
		case TRIE_STEP_END_NODE: return ret;
	}
	return NULL;
}










