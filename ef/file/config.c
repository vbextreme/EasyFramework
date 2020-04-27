#include <ef/config.h>
#include <ef/memory.h>
#include <ef/err.h>

__private char* config_value_string(stream_s* sm){
	char* str = NULL;
	int ch;
	if( stream_inp(sm, &ch) < 1 ) return NULL;
	
	if( ch == '\'' || ch == '"' ){
		if( stream_inp(sm, &str, ch, 0) < 1 ) return NULL;
		stream_skip_line(sm);
	}
	else{
		stream_rollback(sm, &ch, 1);
		ssize_t leni;
		if( (leni=stream_inp_toanyof(sm, &str, " \t\n", 1)) < 1 ) return NULL;
		if( str[leni-1] != '\n' ){
			//dbg_info("end without n");
			stream_skip_line(sm);
		}
		else{
			//dbg_info("end with n");
		}

		str[leni-1] = 0;
	}
	return str;
}

int config_parse_line(trie_s* tr, stream_s* sm){
	if( stream_kbhit(sm) < 1 ) return 0;
	
	stream_skip_h(sm);
	trieElement_s* search = &tr->root;
	configTrie_s* out;

	/*search key*/
	char ch;
	ssize_t ret;
	trieStep_e step;

	while( (ret=stream_read(sm, &ch, 1)) == 1 && ch != ' ' && ch != '\t' && ch != '=' && ch != '\n' ){
		//dbg_info("read:%d(%c)", ch, ch);
		step = trie_step((void**)&out, &search, ch);
		if( step == TRIE_STEP_ERROR ){
			//dbg_warning("trie error");
			stream_skip_line(sm);
			return 1;
		}
	}
	//dbg_info("remaining:%ld", stream_kbhit(sm));
	//dbg_info("ret %ld ch %d step %d", ret, ch, step);
	
	if( ret == -1 ) return -1;
	step = trie_step((void**)&out, &search, 1); 
	if( step != TRIE_STEP_END_NODE ){
		//dbg_info("not end node:%s", ch == '\n' ? "empty line": "end read");	
		return ch == '\n' ? 1 : 0;
	}
	
	if( ch == ' ' || ch == '\t' ){
		stream_skip_h(sm);
		//dbg_info("remaining:%ld", stream_kbhit(sm));
		if( (ret=stream_read(sm, &ch, 1)) != 1 ){
			//dbg_warning("after key space");
			return ret;
		}
	}

	if( ch == '\n' ){
		//dbg_info("call fn without value");
		if( out->fn(out->type, NULL, out->userdata) & CONFF_ERROR ) return -1;
		return 1;
	}
	
	if( ch != '=' ){
		//dbg_warning("config unaspected char:%d(%c)", ch, ch);
		err_push("config unaspected char:%d(%c)", ch, ch);
		return -1;
	}

	stream_skip_h(sm);
	//dbg_info("remaining:%ld", stream_kbhit(sm));

	char* value = config_value_string(sm);
	//dbg_info("remaining:%ld", stream_kbhit(sm));
	if( value == NULL ){
		dbg_error("no value");
		return -1;
	}
	
	//dbg_info("call fn with:%d %s", out->type, value);
	ret = out->fn(out->type, &value, out->userdata);
	if( value ) free(value);
	if( ret & CONFF_ERROR ){
		//dbg_info("conf return error");
		return -1;
	}
	//dbg_info("next conf");
	return 1;
}

err_t config_parse(trie_s* tr, stream_s* sm){
	int ret;
	while( (ret=config_parse_line(tr, sm)) == 1 );
	return ret;
}

