#include <ef/type.h>
#include <ef/regex.h>
#include <ef/vector.h>
#include <ef/str.h>
#include <ef/err.h>

void regex_init(regex_s* rx){
	rx->rx = NULL;
	rx->match = NULL;
	rx->offv = NULL;
	rx->offc = 0;
	rx->regex = NULL;
	rx->text = NULL;
	rx->lent = 0;
	rx->newline = 0;
	rx->utf8 = -1;
	rx->crlf = -1;
}

void regex_set(regex_s* rx, const char* match){
	rx->regex = match;
}

void regex_text(regex_s* rx, const char* txt, size_t len){
	rx->text = txt;
	rx->lent = len ? len : strlen(txt);
}

void regex_match_delete(regex_s* rx){
	pcre2_match_data_free(rx->match);
}

void regex_delete(regex_s* rx){
	regex_match_delete(rx);
	pcre2_code_free(rx->rx);
}

int regex_build(regex_s* rx){
	int err;
	PCRE2_SIZE errof;
	rx->rx = pcre2_compile((PCRE2_SPTR)rx->regex, PCRE2_ZERO_TERMINATED, 0, &err, &errof, NULL);
	if( rx->rx == NULL ){
		PCRE2_UCHAR buffer[256];
		pcre2_get_error_message(err, buffer, sizeof(buffer));
		err_push("regex compilation failed: %s", buffer);
		err_pushf("%s\n", (char*)rx->match);
		size_t off = errof;
		while( off-->0 ) err_pushf(" ");
		err_pushf("^");	
		return -1;
	}
	return 0;
}

int regex_match(regex_s* rx){
	rx->match = pcre2_match_data_create_from_pattern(rx->rx, NULL);
	
	int rc = pcre2_match( rx->rx, (PCRE2_SPTR)rx->text, rx->lent, 0, 0, rx->match, NULL);

	if (rc < 0){
		switch(rc){
			case PCRE2_ERROR_NOMATCH: break;
			default: err_push("pcre2 match"); break;
		}
		return -1;
	}
	rx->offc = rc;
	
	rx->offv = pcre2_get_ovector_pointer(rx->match);
	if( rx->offv[0] > rx->offv[1] ){
		err_push("pcre2 \\K");
		return -1;
	}

	return 0;
}

__private void regex_match_before_continue(regex_s* rx){
	uint32_t obits;
	pcre2_pattern_info(rx->rx, PCRE2_INFO_ALLOPTIONS, &obits);
	rx->utf8 = (obits & PCRE2_UTF) != 0;
	pcre2_pattern_info(rx->rx, PCRE2_INFO_NEWLINE, &rx->newline);
	rx->crlf = rx->newline == PCRE2_NEWLINE_ANY || rx->newline == PCRE2_NEWLINE_CRLF || rx->newline == PCRE2_NEWLINE_ANYCRLF;
}

int regex_match_continue(regex_s* rx){
	if( rx->utf8 == 1 && rx->crlf == -1 )
		regex_match_before_continue(rx);

	for(;;){
		uint32_t options = 0;
		PCRE2_SIZE start_offset = rx->offv[1];
		
		if( rx->offv[0] == rx->offv[1]){
			if( rx->offv[0] == rx->lent) return -1;
			options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
		}
		else{
			PCRE2_SIZE startchar = pcre2_get_startchar(rx->match);
			if( start_offset <= startchar ){
				if (startchar >= rx->lent) return -1;
				start_offset = startchar + 1;
				if( rx->utf8){
				for (; start_offset < rx->lent; start_offset++)
					if ((rx->text[start_offset] & 0xc0) != 0x80) return -1;
				}
			}
		}

		int rc = pcre2_match(rx->rx, (PCRE2_SPTR)rx->text, rx->lent, start_offset, options, rx->match, NULL);

		if( rc == PCRE2_ERROR_NOMATCH){
		    if (options == 0) return -1;                    
			rx->offv[1] = start_offset + 1;
		    if( rx->crlf && start_offset < rx->lent - 1 && rx->text[start_offset] == '\r' && rx->text[start_offset + 1] == '\n'){
				rx->offv[1] += 1;
			}
			else if( rx->utf8 ){
				while( rx->offv[1] < rx->lent){
					if( (rx->text[rx->offv[1]] & 0xc0) != 0x80) return -1;
					rx->offv[1] += 1;
				}
			}
			continue;
		}

		if (rc < 0){
			err_push("pcre2 match");
		    return -1;
		}
		
		rx->offc = rc;
		if( rx->offv[0] > rx->offv[1] ){
			err_push("pcre2 \\K");
			return -1;
		}
		break;
	}

	return 0;
}

size_t regex_match_count(regex_s* rx){
	return rx->offc;
}

const char* regex_match_get(size_t* lenout, regex_s* rx, size_t index){
	iassert(index < rx->offc);
	const char* start = rx->text + rx->offv[2*index];
	if( lenout ) *lenout = rx->offv[2*index+1] - rx->offv[2*index];
	return start;
}

char** str_regex(const char* str, const char* regex, int global){
	regex_s rx = REGEX_NEW();
	
	regex_set(&rx, regex);
	if( regex_build(&rx) ){
		return NULL;
	}

	regex_text(&rx, str, strlen(str));
	if( regex_match(&rx) ){
		regex_delete(&rx);
		return NULL;
	}

	size_t count = regex_match_count(&rx);
	char** capture = vector_new(char*, count, free);
	if( !capture ) return NULL;

	for(size_t i = 0; i < count; ++i){
		size_t len;
		const char* match = regex_match_get(&len, &rx, i);
		vector_push_back(capture, str_dup(match, len));	
	}
	
	if( global ){
		while( !regex_match_continue(&rx) ){
			count = regex_match_count(&rx);
			for(size_t i = 0; i < count; ++i){
				size_t len;
				const char* match = regex_match_get(&len, &rx, i);
				vector_push_back(capture, str_dup(match, len));	
			}
		}
	}

	regex_delete(&rx);

	return capture;
}

